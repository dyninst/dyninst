/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: linux-x86.C,v 1.5 2002/08/28 21:16:12 chadd Exp $

#include <fstream.h>

#include "dyninstAPI/src/process.h"

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <math.h> // for floor()
#include <unistd.h> // for sysconf()

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
//#include "saveSharedLibrary.h" 

#endif

#ifdef HRTIME
#include "rtinst/h/RThwtimer-linux.h"
#endif

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

extern bool isValidAddress(process *proc, Address where);
extern void generateBreakPoint(instruction &insn);


const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
  "main",
  "_init",
  "_start"
};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

const char DL_OPEN_FUNC_NAME[] = "_dl_open";

const char libc_version_symname[] = "__libc_version";


#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

#ifndef _SYS_USER_H
struct user_regs_struct
{
  long ebx;
  long ecx;
  long edx;
  long esi;
  long edi;
  long ebp;
  long eax;
  long xds;
  long xes;
  long xfs;
  long xgs;
  long orig_eax;
  long eip;
  long xcs;
  long eflags;
  long esp;
  long xss;
};
#endif

static int regmap[] = 
{
    EBX, ECX, EDX, ESI,
    EDI, EBP, EAX, DS,
    ES, FS, GS, ORIG_EAX,
    EIP, CS, EFL, UESP,
    SS
/*
  EAX, ECX, EDX, EBX,
  UESP, EBP, ESI, EDI,
  EIP, EFL, CS, SS,
  DS, ES, FS, GS,
  ORIG_EAX
*/
};

#define NUM_REGS (17 /*+ NUM_FREGS*/)
#define NUM_FREGS 8
#define FP0_REGNUM NUM_REGS
#define FP7_REGNUM (FP0_REGNUM+7)
#define INTREGSIZE (sizeof(int))
#define FPREGSIZE 10
#define MAX_REGISTER_RAW_SIZE 10

#define REGISTER_RAW_SIZE(N) (((N) < FP0_REGNUM) ? INTREGSIZE : FPREGSIZE)
#define REGS_SIZE ( NUM_REGS * REGISTER_RAW_SIZE(0) + NUM_FREGS * REGISTER_RAW_SIZE(FP0_REGNUM) )
#define REGS_INTS ( REGS_SIZE / INTREGSIZE )

const int GENREGS_STRUCT_SIZE = sizeof( user_regs_struct );
#ifdef _SYS_USER_H 
const int FPREGS_STRUCT_SIZE = sizeof( user_fpregs_struct );
#else
const int FPREGS_STRUCT_SIZE = sizeof( user_i387_struct );
#endif

int register_addr (int regno )
{
  int addr;

  if ( (regno < 0 || regno >= NUM_REGS)
       && (regno < FP0_REGNUM || regno > FP7_REGNUM) )
    {
      fprintf ( stderr, "Invalid register number %d.", regno);
      assert(0);
      return -1;
    }

  if (regno >= FP0_REGNUM && regno <= FP7_REGNUM) 
    {
      int fpstate;
      struct user *u = NULL;
      fpstate = (int)(&u->i387.st_space);
      addr = fpstate + 10 * (regno - FP0_REGNUM);
    }
  else
    addr = INTREGSIZE * regmap[regno];

  return addr;
}

/* ********************************************************************** */

// Not targeting particular LWPs yet
void *process::getRegisters(unsigned /*lwp*/)
{
  return getRegisters();
}

void *process::getRegisters() {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, reading each from the
   // process user space with ptrace(PTRACE_PEEKUSER ...

   char *buf = new char [ GENREGS_STRUCT_SIZE + FPREGS_STRUCT_SIZE ];
   int error;
   bool errorFlag = false;
   error = P_ptrace( PTRACE_GETREGS, pid, 0, (int)(buf) );
   if( error ) {
       perror("process::getRegisters PTRACE_GETREGS" );
       errorFlag = true;
   } else {
       error = P_ptrace( PTRACE_GETFPREGS, pid, 0, (int)(buf + GENREGS_STRUCT_SIZE) );
       if( error ) {
	   perror("process::getRegisters PTRACE_GETFPREGS" );
	   errorFlag = true;
       }
   }

   if( errorFlag )
       return NULL;
   else
       return (void*)buf;
}

bool changePC(int pid, Address loc) {
  Address regaddr = EIP * INTREGSIZE;
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changePC - PTRACE_POKEUSER" );
    return false;
  }

  return true;
}

static bool changeBP(int pid, Address loc) {
  Address regaddr = EBP * INTREGSIZE;
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changeBP - PTRACE_POKEUSER" );
    return false;
  }

  return true;
}

void printRegs( void *save ) {
	user_regs_struct *regs = (user_regs_struct*)save;
	cerr
		<< " eax: " << (void*)regs->eax
		<< " ebx: " << (void*)regs->ebx
		<< " ecx: " << (void*)regs->ecx
		<< " edx: " << (void*)regs->edx << endl
		<< " edi: " << (void*)regs->edi
		<< " esi: " << (void*)regs->esi << endl
		<< " xcs: " << (void*)regs->xcs
		<< " xds: " << (void*)regs->xds
		<< " xes: " << (void*)regs->xes
		<< " xfs: " << (void*)regs->xfs
		<< " xgs: " << (void*)regs->xgs
		<< " xss: " << (void*)regs->xss << endl
		<< " eip: " << (void*)regs->eip
		<< " esp: " << (void*)regs->esp
		<< " ebp: " << (void*)regs->ebp << endl
		<< " orig_eax: " << (void*)regs->orig_eax
		<< " eflags: " << (void*)regs->eflags << endl;
}

bool process::executingSystemCall(unsigned /*lwp*/) 
{
  // From the program strace, it appears that a non-negative number
  // in the ORIG_EAX register of the inferior process indicates
  // that it is in a system call, and is the number of the system
  // call. - nash
  
  Address regaddr = ORIG_EAX * INTREGSIZE;
  int res;
  res = P_ptrace ( PTRACE_PEEKUSER, getPid(), regaddr, 0 );
  if( res < 0 )
    return false;
  
  inferiorrpc_cerr << "In system call #" << res << " @ " << (void*)getPC( getPid() ) << endl;
  
  return true;
}

bool process::restoreRegisters(void *buffer, unsigned lwp)
{
  return restoreRegisters(buffer);
}

bool process::restoreRegisters(void *buffer) {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

   char *buf = (char*)buffer;
   bool retVal = true;

   if( P_ptrace( PTRACE_SETREGS, pid, 0, (int)(buf) ) )
   {
       perror("process::restoreRegisters PTRACE_SETREGS" );
       retVal = false;
   }

   if( P_ptrace( PTRACE_SETFPREGS, pid, 0, (int)(buf + GENREGS_STRUCT_SIZE) ) )
   {
       perror("process::restoreRegisters PTRACE_SETFPREGS" );
       retVal = false;
   }

   return retVal;
}

// getActiveFrame(): populate Frame object using toplevel frame
Frame process::getActiveFrame(unsigned /*lwp*/)
{
  Address pc, fp;
  fp = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, 0 + EBP * INTREGSIZE, 0);
  if (errno) Frame();

  pc = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, 0 + EIP * INTREGSIZE, 0);
  if (errno) Frame();
  return Frame(pc, fp, getPid(), NULL, 0, true);
}

process *findProcess(int);  // In process.C

Address getPC(int pid) {
   Address regaddr = EIP * INTREGSIZE;
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if( errno ) {
     perror( "getPC" );
     exit(-1);
     return 0; // Shut up the compiler
   } else {
     assert(res);
     return (Address)res;
   }   
}

#ifdef DETACH_ON_THE_FLY
/* Input P points to a buffer containing the contents of
   /proc/PID/stat.  This buffer will be modified.  Output STATUS is
   the status field (field 3), and output SIGPEND is the pending
   signals field (field 31).  As of Linux 2.2, the format of this file
   is defined in /usr/src/linux/fs/proc/array.c (get_stat). */
static void
parse_procstat(char *p, char *status, unsigned long *sigpend)
{
     int i;

     /* Advance past pid and program name */
     p = strchr(p, ')');
     assert(p);

     /* Advance to status character */
     p += 2;
     *status = *p;

     /* Advance to sigpending int */
     p = strtok(p, " ");
     assert(p);
     for (i = 0; i < 28; i++) {
	  p = strtok(NULL, " ");
	  assert(p);
     }
     *sigpend = strtoul(p, NULL, 10);
}


/* The purpose of this is to synchronize with the sigill handler in
   the inferior.  In this handler, the inferior signals us (SIG_REATTACH),
   and then it stops itself.  This routine does not return until the
   inferior stop has occurred.  In some contexts, the inferior may be
   acquire a pending SIGSTOP as or after it stops.  We clear the
   pending the SIGSTOP here, so the process doesn't inadvertently stop
   when we continue it later.

   We don't understand how the pending SIGSTOP is acquired.  It seems
   to have something to do with the process sending itself the SIGILL
   (i.e., kill(getpid(),SIGILL)).  */
static void
waitForInferiorSigillStop(int pid)
{
     int fd;
     char name[32];
     char buf[512];
     char status;
     unsigned long sigpend;

     sprintf(name, "/proc/%d/stat", pid);

     /* Keep checking the process until it is stopped */
     while (1) {
	  /* Read current contents of /proc/pid/stat */
	  fd = open(name, O_RDONLY);
	  assert(fd >= 0);
	  assert(read(fd, buf, sizeof(buf)) > 0);
	  close(fd);

	  /* Parse status and pending signals */
	  parse_procstat(buf, &status, &sigpend);

	  /* status == T iff the process is stopped */
	  if (status != 'T')
	       continue;
	  /* This is true iff SIGSTOP is set in sigpend */
	  if (0x1 & (sigpend >> (SIGSTOP-1))) {
	       /* The process is stopped, but it has another SIGSTOP
		  pending.  Continue the process to clear the SIGSTOP
		  (the process will stop again immediately). */
	       P_ptrace(PTRACE_CONT, pid, 0, 0);
	       if (0 > waitpid(pid, NULL, 0))
		    perror("waitpid");
	       continue; /* repeat to be sure we've stopped */
	  }

	  /* The process is stopped properly */
	  break;
     }
}

/* When a detached mutatee needs us to reattach and handle an event,
   it sends itself a SIGILL.  Its SIGILL handler in turn sends us
   SIG_REATTACH, which brings us here.  Here we reattach to the process and
   then help it re-execute the code that caused its SIGILL.  Having
   reattached, we receive the new SIGILL event and dispatch it as
   usual (in handleSigChild). */
static void sigill_handler(int sig, siginfo_t *si, void *unused)
{
     process *p;

     unused = 0; /* Suppress compiler warning of unused parameter */

     assert(sig == SIG_REATTACH);
     /* Determine the process that sent the signal.  On Linux (at
	least upto 2.2), we can only obtain this with the real-time
	signals, those numbered 33 or higher. */
     p = findProcess(si->si_pid);
     if (!p) {
	  fprintf(stderr, "Received SIGILL sent by unregistered or non-inferior process\n");
	  assert(0);
     }

     /* Synchronize with the SIGSTOP sent by inferior sigill handler */
     waitForInferiorSigillStop(p->getPid());

     /* Reattach, which will leave a pending SIGSTOP. */
     p->reattach();
     if (! p->isRunningIRPC())
	  /* If we got this signal when the inferior was not in an RPC,
	     then we need to reattach after we handle it.
	     FIXME: Why have we released the process for RPCs anyway? */
	  p->needsDetach = true;

     /* Resume the process.  We expect it to re-execute the code that
        generated the SIGILL.  Now that we are attached, we'll get the
        SIGILL event and handle it with handleSigChild as usual. */
     /* clear pending stop left by reattach */
     P_ptrace(PTRACE_CONT, p->getPid(), 0, 0);
     if (0 > waitpid(p->getPid(), NULL, 0))
	     perror("waitpid");
     if (!p->continueProc())
	  assert(0);
}

#endif /* DETACH_ON_THE_FLY */

void process::handleIfDueToDyninstLib() 
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  Address codeBase = 0;
  int i;

  for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
	  bool found = false;
	  Symbol s;
	  codeBase = 0;
	  found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
	  if( found )
		  codeBase = s.addr();
	  if( codeBase )
		  break;
  }
  assert( codeBase );

  writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer);

  // restore registers
  restoreRegisters(savedRegs); 

  // restore the stack frame of _start()
  user_regs_struct *theIntRegs = (user_regs_struct *)savedRegs;
  RegValue theEBP = theIntRegs->ebp;

  if( !theEBP )
  {
	  theEBP = theIntRegs->esp;
  }

  assert (theEBP);
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  writeDataSpace ((void*)(theEBP-6*sizeof(int)),6*sizeof(int),savedStackFrame);

  if( isRunning_() )
	  cerr << "WARNING -- process is running at trap from dlopenDYNINSTlib" << endl;

  delete [] (char *) savedRegs;
  savedRegs = NULL;
}

void process::handleTrapAtEntryPointOfMain()
{
  function_base *f_main = findOneFunction("main");
  assert(f_main);
  Address addr = f_main->addr();
  // restore original instruction 
  writeDataSpace((void *)addr, 2, (char *)savedCodeBuffer);
}

void process::insertTrapAtEntryPointOfMain()
{
  function_base *f_main = 0;
  f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"main() uninstrumentable");
    extern void cleanUpAndExit(int);
    cleanUpAndExit(-1); 
    return;
  }
  assert(f_main);
  Address addr = f_main->addr();

  // save original instruction first
  readDataSpace((void *)addr, 2, savedCodeBuffer, true);

  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);

  // x86. have to use SIGILL instead of SIGTRAP
  writeDataSpace((void *)addr, 2, insnTrap.ptr());  

  main_brk_addr = addr;
}

void emitCallRel32(unsigned disp32, unsigned char *&insn);

Frame Frame::getCallerFrame(process *p) const
{
  //
  // for the x86, the frame-pointer (EBP) points to the previous frame-pointer,
  // and the saved return address is in EBP-4.
  //
  struct {
    int fp;
    int rtn;
  } addrs;

  if (p->readDataSpace((caddr_t)(fp_), 2*sizeof(int),
		       (caddr_t) &addrs, true))
  {
    Frame ret(*this);
    ret.fp_ = addrs.fp;
    ret.pc_ = addrs.rtn;
    ret.uppermost_ = false;
    return ret;
  }

  return Frame(); // zero frame
}

#ifdef BPATCH_LIBRARY

char* process::dumpPatchedImage(string imageFileName){ //ccw 7 feb 2002 

	addLibrary addLibraryElf;
	unsigned int errFlag=0;
	vector<imageUpdate*> compactedHighmemUpdates;
	char *mutatedSharedObjects=0;
	int mutatedSharedObjectsSize = 0, mutatedSharedObjectsIndex=0;
	char *directoryName = 0;
	shared_object *sh_obj;

	if(!collectSaveWorldData){
                BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::startSaveWorld() not called.  No mutated binary saved\n");
                return NULL;
        }

	directoryName = saveWorldFindDirectory();

	if(!directoryName){
		return NULL;
	}

	strcat(directoryName, "/");	
	char* fullName = new char[strlen(directoryName) + strlen ( (char*)imageFileName.c_str())+1];
        strcpy(fullName, directoryName);
        strcat(fullName, (char*)imageFileName.c_str());


	unsigned int dl_debug_statePltEntry = 0x00016574;//a pretty good guess
	unsigned int dyninst_SharedLibrariesSize = 0;
	unsigned int mutatedSharedObjectsNumb=0;

	dl_debug_statePltEntry = saveWorldSaveSharedLibs(mutatedSharedObjectsSize, 
		dyninst_SharedLibrariesSize,directoryName,mutatedSharedObjectsNumb);

	//the mutatedSO section contains a list of the shared objects that have been mutated
	if(mutatedSharedObjectsSize){
		mutatedSharedObjectsSize += mutatedSharedObjectsNumb * sizeof(unsigned int);
		mutatedSharedObjects = new char[mutatedSharedObjectsSize];
		for(int i=0;shared_objects && i<(int)shared_objects->size() ; i++) {
			sh_obj = (*shared_objects)[i];
			if(sh_obj->isDirty()){
				memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
					sh_obj->getName().c_str(),
					strlen(sh_obj->getName().c_str())+1);
				mutatedSharedObjectsIndex += strlen(
					sh_obj->getName().c_str())+1;
				unsigned int baseAddr = sh_obj->getBaseAddress();
				memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]),
					&baseAddr, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	
			}
		}	
	}

	
	char *dyninst_SharedLibrariesData =saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);
		
	writeBackElf *newElf = new writeBackElf(( char*) getImage()->file().c_str(),
			                "/tmp/dyninstMutatee",errFlag);
        newElf->registerProcess(this);

#ifdef USE_STL_VECTOR
	sort(highmemUpdates.begin(), highmemUpdates.end(), imageUpdateOrderingRelation());
#else
        highmemUpdates.sort( imageUpdate::imageUpdateSort);
#endif
        newElf->compactSections(highmemUpdates, compactedHighmemUpdates);

        newElf->alignHighMem(compactedHighmemUpdates);

	saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates, (void*) newElf);

	saveWorldCreateDataSections((void*) newElf);

	unsigned int k;

        for( k=0;k<imageUpdates.size();k++){
	  delete imageUpdates[k];
        }

        for( k=0;k<highmemUpdates.size();k++){
	  delete highmemUpdates[k];
        }

        for(  k=0;k<compactedHighmemUpdates.size();k++){
	  delete compactedHighmemUpdates[k];
        }
	if(mutatedSharedObjectsSize){
		newElf->addSection(0 ,mutatedSharedObjects, 
			mutatedSharedObjectsSize, "dyninstAPI_mutatedSO", false);
		delete [] mutatedSharedObjects;
	}
	//the following is for the dlopen problem
	newElf->addSection(dl_debug_statePltEntry, dyninst_SharedLibrariesData, 
	dyninst_SharedLibrariesSize, "dyninstAPI_SharedLibraries", false);
	delete [] dyninst_SharedLibrariesData;
	
	//the following reloads any shared libraries loaded into the
	//mutatee using BPatch_thread::loadLibrary
	saveWorldAddSharedLibs((void*)newElf); // ccw 14 may 2002 

        newElf->createElf();

	elf_update(newElf->getElf(),ELF_C_WRITE); 
       	addLibraryElf.driver(newElf->getElf(),fullName, "libdyninstAPI_RT.so.1");	
	return directoryName;	

}

#endif

Address process::readRegister(unsigned /*lwp*/, Register /*reg*/) {
   // On x86, the result is always stashed in %EAX
   int ret;
   ret = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, EAX*4, 0);
   return (Address)ret;
}

bool process::set_breakpoint_for_syscall_completion() {
	Address codeBase;
	codeBase = getPC( getPid() );
	readDataSpace( (void*)codeBase, 2, savedCodeBuffer, true);

	instruction insnTrap;
	generateBreakPoint(insnTrap);
	writeDataSpace((void *)codeBase, 2, insnTrap.ptr());

	inferiorrpc_cerr << "Set breakpoint at " << (void*)codeBase << endl;

	return true;
}

void process::clear_breakpoint_for_syscall_completion() {
	Address codeBase;
	codeBase = getPC( getPid() );
	writeDataSpace( (void*)codeBase, 2, savedCodeBuffer );

	inferiorrpc_cerr << "Cleared breakpoint at " << (void*)codeBase << endl;
}

void print_read_error_info(const relocationEntry entry, 
      pd_Function *&target_pdf, Address base_addr) {

    sprintf(errorLine, "  entry      : target_addr 0x%x\n",
	    (unsigned)entry.target_addr());
    logLine(errorLine);
    sprintf(errorLine, "               rel_addr 0x%x\n", (unsigned)entry.rel_addr());
    logLine(errorLine);
    sprintf(errorLine, "               name %s\n", (entry.name()).c_str());
    logLine(errorLine);

    sprintf(errorLine, "  target_pdf : symTabName %s\n",
	    (target_pdf->symTabName()).c_str());
    logLine(errorLine);    
    sprintf(errorLine , "              prettyName %s\n",
	    (target_pdf->symTabName()).c_str());
    logLine(errorLine);
    sprintf(errorLine , "              size %i\n",
	    target_pdf->size());
    logLine(errorLine);
    sprintf(errorLine , "              addr 0x%x\n",
	    (unsigned)target_pdf->addr());
    logLine(errorLine);

    sprintf(errorLine, "  base_addr  0x%x\n", (unsigned)base_addr);
    logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry entry, 
			   pd_Function *&target_pdf, Address base_addr) {

    if (status() == exited) return false;

    // if the relocationEntry has not been bound yet, then the value
    // at rel_addr is the address of the instruction immediately following
    // the first instruction in the PLT entry (which is at the target_addr) 
    // The PLT entries are never modified, instead they use an indirrect 
    // jump to an address stored in the _GLOBAL_OFFSET_TABLE_.  When the 
    // function symbol is bound by the runtime linker, it changes the address
    // in the _GLOBAL_OFFSET_TABLE_ corresponding to the PLT entry

    Address got_entry = entry.rel_addr() + base_addr;
    Address bound_addr = 0;
    if(!readDataSpace((const void*)got_entry, sizeof(Address), 
			&bound_addr, true)){
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,pid);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
        return false;
    }

    if( !( bound_addr == (entry.target_addr()+6+base_addr)) ) {
        // the callee function has been bound by the runtime linker
	// find the function and return it
        target_pdf = findFuncByAddr(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;
}

Address getSP(int pid) {
   Address regaddr = UESP * sizeof(int);
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if( errno ) {
     perror( "getSP" );
     exit(-1);
     return 0; // Shut up the compiler
   } else {
     assert(res);
     return (Address)res;
   }
}

bool changeSP(int pid, Address loc) {
  Address regaddr = UESP * sizeof(int);
  if (0 != P_ptrace (PTRACE_POKEUSER, pid, regaddr, loc )) {
    perror( "process::changeSP - PTRACE_POKEUSER" );
    return false;
  }
  
  return true;
}

instruction generateTrapInstruction() {
	return instruction((const unsigned char*)"\017\013\0220\0220", ILLEGAL, 4);
	} /* end generateTrapInstruction() */



//ccw 29 apr 2002 : SPLIT3 i expect to need to add dlopenPARADYNlib here...
//and to edit dlopenDYNINSTlib
#if !defined(BPATCH_LIBRARY)
bool process::dlopenPARADYNlib() {
#if false && defined(PTRACEDEBUG)
  debug_ptrace = true;
#endif
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  //  Address codeBase = (this->findFuncByName(DYNINST_LOAD_HIJACK_FUNCTION))->getAddress(this);
  Address codeBase = 0;
  int i;

  for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      bool found = false;
      Symbol s;
      codeBase = 0;
      found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
      if( found )
          codeBase = s.addr();
      if( codeBase )
          break;
  }

  if( !codeBase || i >= N_DYNINST_LOAD_HIJACK_FUNCTIONS )
  {
      attach_cerr << "Couldn't find a point to insert dlopen call" << endl;
      return false;
  }

  attach_cerr << "Inserting dlopen call in " << DYNINST_LOAD_HIJACK_FUNCTIONS[i] << " at "
      << (void*)codeBase << endl;
  attach_cerr << "Process at " << (void*)getPC( getPid() ) << endl;

  bool libc_21 = true; /* inferior has glibc 2.1-2.x */
  Symbol libc_vers;
  if( !getSymbolInfo( libc_version_symname, libc_vers ) ) {
      cerr << "Couldn't find " << libc_version_symname << ", assuming glibc_2.1.x" << endl;
  } else {
      char libc_version[ libc_vers.size() + 1 ];
      libc_version[ libc_vers.size() ] = '\0';
      readDataSpace( (void *)libc_vers.addr(), libc_vers.size(), libc_version, true );
      if (strncmp(libc_version, "2", 1)) {
	  cerr << "Found " << libc_version_symname << " = \"" << libc_version
	       << "\", which doesn't match any known glibc"
	       << " assuming glibc 2.1." << endl;
      } else if (!strncmp(libc_version, "2.0", 3))
	      libc_21 = false;
  }
  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs( 2 );

  unsigned count = 0;
  Address dyninst_count = 0;

  AstNode *dlopenAst;

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(int), deadList, 0, NULL);
  dlopenRegSpace->resetSpace();

  // we need to make a call to dlopen to open our runtime library

  if( !libc_21 ) {
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0);
      // library name. We use a scratch value first. We will update this parameter
      // later, once we determine the offset to find the string - naim
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      
      dyninst_count = 0;
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
  } else {
      // In glibc 2.1.x, _dl_open is optimized for being an internal wrapper function.
      // Instead of using the stack, it passes three parameters in EAX, EDX and ECX.
      // Here we simply make a call with no normal parameters, and below we change
      // the three registers along with EIP to execute the code.
      unsigned char *code_ptr = scratchCodeBuffer;
      Address disp;
      Address addr;
      bool err;
      addr = findInternalAddress(DL_OPEN_FUNC_NAME, false, err);
      if (err) {
	  function_base *func = findOneFunction(DL_OPEN_FUNC_NAME);
	  if (!func) {
	      ostrstream os(errorLine, 1024, ios::out);
	      os << "Internal error: unable to find addr of " << DL_OPEN_FUNC_NAME << endl;
	      logLine(errorLine);
	      showErrorCallback(80, (const char *) errorLine);
	      P_abort();
	  }
	  addr = func->getAddress(0);
      }

      disp = addr - ( codeBase + 5 );
      attach_cerr << DL_OPEN_FUNC_NAME << " @ " << (void*)addr << ", displacement == "
		  << (void*)disp << endl;
      emitCallRel32( disp, code_ptr );
      dyninst_count = 5;
  }

  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  paradynlib_brk_addr = codeBase + count;
  count += 2;

  const char DyninstEnvVar[]="PARADYN_LIB";

    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
	    //dyninstName = "/home/chadd/chadd/currentDyninst/lib/i386-unknown-linux2.4/libdyninstRT.so.1";
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = (Address) (codeBase + count);

  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+1;
  // we have now written the name of the library after the trap - naim

  assert(count<=BYTES_TO_SAVE);

  if( !libc_21 ) {
      count = 0; // reset count

      // at this time, we know the offset for the library name, so we fix the
      // call to dlopen and we just write the code again! This is probably not
      // very elegant, but it is easy and it works - naim
      removeAst(dlopenAst); // to avoid leaking memory
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      dyninst_count = 0; // reset count
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
      writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
      removeAst(dlopenAst);
  }

  // save registers
  savedRegs = getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));
  // save the stack frame of _start()
  user_regs_struct *regs = (user_regs_struct*)savedRegs;
  user_regs_struct new_regs = *regs;
  RegValue theEBP = regs->ebp;

  // Under Linux, at the entry point to main, ebp is 0
  // the first thing main usually does is to push ebp and
  // move esp -> ebp, so we'll do that, too
  if( !theEBP )
  {
	  theEBP = regs->esp;
	  attach_cerr << "eBP at 0x0, creating fake stack frame with eSP == "
				  << (void*)theEBP << endl;
	  changeBP( getPid(), theEBP );
  }

  assert( theEBP );
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  readDataSpace((void*)(theEBP-6*sizeof(int)),6*sizeof(int), savedStackFrame, true);

  isLoadingParadynLib = true;  //ccw 30 apr 2002 : SPLIT3

  attach_cerr << "Changing PC to " << (void*)codeBase << endl;

  if (!libc_21)
  {
      if (!changePC(codeBase,savedRegs))
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }
  else
  {
      new_regs.eip = codeBase;

      if( libc_21 ) {
          new_regs.eax = dyninstlib_addr;
          new_regs.edx = DLOPEN_MODE;
          new_regs.ecx = codeBase;
      }

      if( !restoreRegisters( (void*)(&new_regs) ) )
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif

  return true;
}


#endif

bool process::dlopenDYNINSTlib() {
#if false && defined(PTRACEDEBUG)
  debug_ptrace = true;
#endif
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.
  //Address codeBase = this->getImage()->codeOffset();
  // ...let's try "_start" instead
  //  Address codeBase = (this->findFuncByName(DYNINST_LOAD_HIJACK_FUNCTION))->getAddress(this);
  Address codeBase = 0;
  int i;

  for( i = 0; i < N_DYNINST_LOAD_HIJACK_FUNCTIONS; i++ ) {
      bool found = false;
      Symbol s;
      codeBase = 0;
      found = symbols->symbol_info(DYNINST_LOAD_HIJACK_FUNCTIONS[i], s);
      if( found )
          codeBase = s.addr();
      if( codeBase )
          break;
  }

  if( !codeBase || i >= N_DYNINST_LOAD_HIJACK_FUNCTIONS )
  {
      attach_cerr << "Couldn't find a point to insert dlopen call" << endl;
      return false;
  }

  attach_cerr << "Inserting dlopen call in " << DYNINST_LOAD_HIJACK_FUNCTIONS[i] << " at "
      << (void*)codeBase << endl;
  attach_cerr << "Process at " << (void*)getPC( getPid() ) << endl;

  bool libc_21 = true; /* inferior has glibc 2.1-2.x */
  Symbol libc_vers;
  if( !getSymbolInfo( libc_version_symname, libc_vers ) ) {
      cerr << "Couldn't find " << libc_version_symname << ", assuming glibc_2.1.x" << endl;
  } else {
      char libc_version[ libc_vers.size() + 1 ];
      libc_version[ libc_vers.size() ] = '\0';
      readDataSpace( (void *)libc_vers.addr(), libc_vers.size(), libc_version, true );
      if (strncmp(libc_version, "2", 1)) {
	  cerr << "Found " << libc_version_symname << " = \"" << libc_version
	       << "\", which doesn't match any known glibc"
	       << " assuming glibc 2.1." << endl;
      } else if (!strncmp(libc_version, "2.0", 3))
	      libc_21 = false;
  }
  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs( 2 );

  unsigned count = 0;
  Address dyninst_count = 0;

  AstNode *dlopenAst;

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(int), deadList, 0, NULL);
  dlopenRegSpace->resetSpace();

  // we need to make a call to dlopen to open our runtime library

  if( !libc_21 ) {
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0);
      // library name. We use a scratch value first. We will update this parameter
      // later, once we determine the offset to find the string - naim
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      
      dyninst_count = 0;
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
  } else {
      // In glibc 2.1.x, _dl_open is optimized for being an internal wrapper function.
      // Instead of using the stack, it passes three parameters in EAX, EDX and ECX.
      // Here we simply make a call with no normal parameters, and below we change
      // the three registers along with EIP to execute the code.
      unsigned char *code_ptr = scratchCodeBuffer;
      Address disp;
      Address addr;
      bool err;
      addr = findInternalAddress(DL_OPEN_FUNC_NAME, false, err);
      if (err) {
	  function_base *func = findOneFunction(DL_OPEN_FUNC_NAME);
	  if (!func) {
	      ostrstream os(errorLine, 1024, ios::out);
	      os << "Internal error: unable to find addr of " << DL_OPEN_FUNC_NAME << endl;
	      logLine(errorLine);
	      showErrorCallback(80, (const char *) errorLine);
	      P_abort();
	  }
	  addr = func->getAddress(0);
      }

      disp = addr - ( codeBase + 5 );
      attach_cerr << DL_OPEN_FUNC_NAME << " @ " << (void*)addr << ", displacement == "
		  << (void*)disp << endl;
      emitCallRel32( disp, code_ptr );
      dyninst_count = 5;
  }

  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  dyninstlib_brk_addr = codeBase + count;
  count += 2;

  //ccw 29 apr 2002 : SPLIT3
  const char DyninstEnvVar[]="DYNINSTAPI_RT_LIB";

/*  if (dyninstName.length()) { //ccw 28 aug 2002 
    // use the library name specified on the start-up command-line
  } else {*/
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstName = getenv(DyninstEnvVar);
    } else {
      string msg = string("Environment variable " + string(DyninstEnvVar)
                   + " has not been defined for process ") + string(pid);
      showErrorCallback(101, msg);
      return false;
    }
/*  }*/
  if (access(dyninstName.c_str(), R_OK)) {
    string msg = string("Runtime library ") + dyninstName
        + string(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = (Address) (codeBase + count);

  writeDataSpace((void *)(codeBase + count), dyninstName.length()+1,
		 (caddr_t)const_cast<char*>(dyninstName.c_str()));
  count += dyninstName.length()+1;
  // we have now written the name of the library after the trap - naim

  assert(count<=BYTES_TO_SAVE);

  if( !libc_21 ) {
      count = 0; // reset count

      // at this time, we know the offset for the library name, so we fix the
      // call to dlopen and we just write the code again! This is probably not
      // very elegant, but it is easy and it works - naim
      removeAst(dlopenAst); // to avoid leaking memory
      dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void *)(dyninstlib_addr));
      dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE);
      dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
      removeAst(dlopenAstArgs[0]);
      removeAst(dlopenAstArgs[1]);
      dyninst_count = 0; // reset count
      dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
                              dyninst_count, true, true);
      writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
      removeAst(dlopenAst);
  }

  // save registers
  savedRegs = getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));
  // save the stack frame of _start()
  user_regs_struct *regs = (user_regs_struct*)savedRegs;
  user_regs_struct new_regs = *regs;
  RegValue theEBP = regs->ebp;

  // Under Linux, at the entry point to main, ebp is 0
  // the first thing main usually does is to push ebp and
  // move esp -> ebp, so we'll do that, too
  if( !theEBP )
  {
	  theEBP = regs->esp;
	  attach_cerr << "eBP at 0x0, creating fake stack frame with eSP == "
				  << (void*)theEBP << endl;
	  changeBP( getPid(), theEBP );
  }

  assert( theEBP );
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  readDataSpace((void*)(theEBP-6*sizeof(int)),6*sizeof(int), savedStackFrame, true);

  isLoadingDyninstLib = true;

  attach_cerr << "Changing PC to " << (void*)codeBase << endl;

  if (!libc_21)
  {
      if (!changePC(codeBase,savedRegs))
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }
  else
  {
      new_regs.eip = codeBase;

      if( libc_21 ) {
          new_regs.eax = dyninstlib_addr;
          new_regs.edx = DLOPEN_MODE;
          new_regs.ecx = codeBase;
      }

      if( !restoreRegisters( (void*)(&new_regs) ) )
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif

  return true;
}


