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

// $Id: linux.C,v 1.14 1999/05/28 01:49:40 nash Exp $

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

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/instPoint.h"
#include "util/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "util/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "util/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;
extern debug_ostream signal_cerr;

extern bool isValidAddress(process *proc, Address where);
extern void generateBreakPoint(instruction &insn);

const char DYNINST_LOAD_HIJACK_FUNCTIONS[][15] = {
  "main",
  "_start",
  "_init"
};
const int N_DYNINST_LOAD_HIJACK_FUNCTIONS = 3;

const char DL_OPEN_FUNC_NAME[] = "_dl_open";

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

/* ********************************************************************** */

/* this table must line up with REGISTER_NAMES */
/* symbols like 'EAX' come from <sys/reg.h> */
static int regmap[] = 
{
  EAX, ECX, EDX, EBX,
  UESP, EBP, ESI, EDI,
  EIP, EFL, CS, SS,
  DS, ES, FS, GS,
  ORIG_EAX
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

const int GENREGS_STRUCT_SIZE = sizeof( user::regs );
const int FPREGS_STRUCT_SIZE = sizeof( user::i387 );

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

class ptraceKludge {
private:
  static bool haltProcess(process *p);
  static void continueProcess(process *p, const bool halted);

public:
  static bool deliverPtrace(process *p, int req, int addr, int data);
  static int deliverPtraceReturn(process *p, int req, int addr, int data);
};

bool ptraceKludge::haltProcess(process *p) {
  bool wasStopped = (p->status() == stopped);
  if (p->status() != neonatal && !wasStopped) {
    if (!p->loopUntilStopped()) {
      cerr << "error in loopUntilStopped\n";
      assert(0);
    }
  }  return wasStopped;
}

void ptraceKludge::continueProcess(process *p, const bool wasStopped) {
  // First handle the cases where we shouldn't issue a PTRACE_CONT:
  if (p->status() == neonatal) return;
  if (wasStopped) return;

  // Choose either one of the following methods to continue a process.
  // The choice must be consistent with that in process::continueProc_ and stop_

#ifndef PTRACE_ATTACH_DETACH
  if (P_ptrace(PTRACE_CONT, p->pid, 1, SIGCONT) == -1) {
#else
  if (P_ptrace(PTRACE_DETACH, p->pid, 1, SIGCONT) == -1) {
#endif
      perror("error in continueProcess");
      assert(0);
  }
}

bool ptraceKludge::deliverPtrace(process *p, int req, int addr,
				 int data ) {
  bool halted = true;

  if (req != PTRACE_DETACH)
     halted = haltProcess(p);

  bool ret = (P_ptrace(req, p->getPid(), addr, data) != -1);

  if (req != PTRACE_DETACH)
     continueProcess(p, halted);

  return ret;
}


// Some ptrace requests in Linux return the value rather than storing at the address in data
// (especially PEEK requests)
// - nash
int ptraceKludge::deliverPtraceReturn(process *p, int req, int addr,
				 int data ) {
  bool halted = true;

  if (req != PTRACE_DETACH)
     halted = haltProcess(p);

  int ret = P_ptrace(req, p->getPid(), addr, data);

  if (req != PTRACE_DETACH)
     continueProcess(p, halted);

  return ret;
}

/* ********************************************************************** */

void *process::getRegisters() {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, reading each from the
   // process user space with ptrace(PTRACE_PEEKUSER ...

#ifdef PTRACE_GETREGS
   char *buffer = new char [ GENREGS_STRUCT_SIZE + FPREGS_STRUCT_SIZE ];
   if( P_ptrace( PTRACE_GETREGS, pid, 0, (int)(buffer) ) )
   {
	   perror("process::getRegisters PTRACE_GETREGS" );
   }
   //printf( "ORIG_EAX %#.8x\n", *(((int*)buffer)+ORIG_EAX) );

   if( P_ptrace( PTRACE_GETFPREGS, pid, 0, (int)(buffer + GENREGS_STRUCT_SIZE) ) )
   {
	   perror("process::getRegisters PTRACE_GETFPREGS" );
   }
#else
   int *buffer = new int[ REGS_INTS ];
   int regno;
   Address regaddr;

   for (regno = 0; regno < NUM_REGS; regno++) {
     regaddr = register_addr (regno);
     buffer[ regno ] = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
     if( errno ) {
       perror("process::getRegisters PTRACE_PEEKUSER");
       return NULL;
     }
   }

   int baddr = register_addr ( FP0_REGNUM );
   int eaddr = register_addr ( FP7_REGNUM ) + REGISTER_RAW_SIZE( FP7_REGNUM );
   int count;
   for (regaddr = baddr, count = NUM_REGS; regaddr < eaddr; regaddr += sizeof(int), count++ ) {
     buffer[ count ] = P_ptrace( PTRACE_PEEKUSER, pid, regaddr, 0);
     if( errno ) {
       perror("process::getRegisters PTRACE_PEEKUSER fp");
       return NULL;
     }
   }
#endif

   return (void*)buffer;
}

static bool changePC(int pid, Address loc) {
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

 Address getPC(int pid) {
   Address regaddr = EIP * INTREGSIZE;
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if( errno ) {
     perror( "getPC" );
     exit(-1);
     return 0; // Shut up the compiler
   } else
     return (Address)res;
 }

void printStackWalk( process *p ) {
	Frame theFrame(p);
	while (true) {
		// do we have a match?
		const Address framePC = theFrame.getPC();
		inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
		
		if (theFrame.isLastFrame())
			// well, we've gone as far as we can, with no match.
			break;
		
		// else, backtrace 1 more level
		theFrame = theFrame.getPreviousStackFrameInfo(p);
	}
}
 
void printRegs( void *save ) {
	user_regs_struct *regs = (user_regs_struct*)save;
	inferiorrpc_cerr
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

bool process::executingSystemCall() {
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

bool process::changePC(Address loc, const void * /* savedRegs */ ) {
  assert(status_ == stopped);

  return ::changePC(pid, loc);
}

bool process::changePC(Address loc) {
  assert(status_ == stopped);

  return ::changePC(pid, loc);
}


bool process::restoreRegisters(void *buffer) {
   // assumes the process is stopped (ptrace requires it)
   assert(status_ == stopped);

   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

#ifdef PTRACE_GETREGS
   char *buf = (char*)buffer;

   if( P_ptrace( PTRACE_SETREGS, pid, 0, (int)(buf) ) )
   {
	   perror("process::restoreRegisters PTRACE_SETREGS" );
   }

   if( P_ptrace( PTRACE_SETFPREGS, pid, 0, (int)(buf + GENREGS_STRUCT_SIZE) ) )
   {
	   perror("process::restoreRegisters PTRACE_SETFPREGS" );
   }
#else
   int *buf = (int*)buffer;
   int regno;
   Address regaddr;

   for (regno = 0; regno < NUM_REGS-1; regno++) {
     regaddr = register_addr (regno);
     if( P_ptrace (PTRACE_POKEUSER, pid, regaddr, buf[regno] ) ) {
       perror("process::restoreRegisters PTRACE_POKEUSER");
	   fprintf( stderr, "PID %d, reg %d, address %#.8x, value %#.8x\n",
				pid, regno, regaddr, buf[regno] );
       return false;
     }
   }
   return true;

   // Cycle through all 20 words making up the 8 fp registers
   int baddr = register_addr ( FP0_REGNUM );
   int eaddr = register_addr ( FP7_REGNUM ) + REGISTER_RAW_SIZE( FP7_REGNUM );
   int count;
   for (regaddr=baddr, count=NUM_REGS; regaddr<eaddr; regaddr+= sizeof(int), count++ ) {
     if( P_ptrace (PTRACE_POKEUSER, pid, regaddr, buf[count] ) ) {
       perror("process::restoreRegisters PTRACE_POKEUSER fp");
	   fprintf( stderr, "PID %d, fp word %d, address %#.8x, value %#.8x\n",
				pid, count, regaddr, buf[count] );
       return false;
     }
   }
#endif

   return true;
}

bool process::getActiveFrame(Address *fp, Address *pc)
{
  *fp = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, 0 + EBP * INTREGSIZE, 0);
  if( errno )
    return false;

  *pc = ptraceKludge::deliverPtraceReturn(this, PTRACE_PEEKUSER, 0 + EIP * INTREGSIZE, 0);
  if( errno )
    return false;

  return true;
}

// TODO: implement this
bool process::needToAddALeafFrame(Frame,Address &){
    return false;
}

// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL);
  P_close (ttyfd);
}

bool process::stop_() {
	return (P_kill(getPid(), SIGSTOP) != -1); 
}

bool process::continueWithForwardSignal(int sig) {
   // formerly OS::osForwardSignal
   return (P_ptrace(PTRACE_CONT, pid, 1, sig) != -1);
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

process *findProcess(int);  // In process.C

// wait for a process to terminate or stop
// We only want to catch SIGSTOP and SIGILL
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block) {
  int options = 0;
  if( !block )
    options |= WNOHANG;
#else
int process::waitProcs(int *status) {
  int options = WNOHANG;
#endif
  int result, sig;
  bool ignore;
  do {
    ignore = false;
    result = waitpid( -1, status, options );

	// Check for TRAP at the end of a syscall, then
    // if the signal's not SIGSTOP or SIGILL,
    // send the signal back and wait for another.
    if( result > 0 && WIFSTOPPED(*status) ) {
		process *p = findProcess( result );
		sig = WSTOPSIG(*status);
		if( sig == SIGTRAP && ( !p->reachedVeryFirstTrap || p->inExec ) )
			; // Report it
/*
#ifdef CHECK_SYSTEM_CALLS
// This code is needed if we are using PTRACE_SYSCALL to wait for the
// end of a system call.  However, that's not what we do, we set our own
// illegal instruction in the user code at the return from the system call.
		else if( sig == SIGTRAP && p->isRPCwaitingForSysCallToComplete() )
		{
			inferiorrpc_cerr << "Catching SIGTRAP for RPCwaitingForSysCallToComplete" << endl;
			assert( !p->isRunning_() );
		}
#endif
*/
		else if( sig != SIGSTOP && sig != SIGILL ) {
			ignore = true;
			if( sig != SIGTRAP )
			{
//#ifdef notdef
				Address pc;
				pc = getPC( result );
				signal_cerr << "process::waitProcs -- Signal #" << sig << " in " << result << "@" << (void*)pc << ", resignalling the process" << endl;
//#endif
			}
			if( P_ptrace(PTRACE_CONT, result, 1, sig) == -1 ) {
				if( errno == ESRCH ) {
					cerr << "WARNING -- process does not exist, constructing exit code" << endl;
					*status = W_EXITCODE(0,sig);
					ignore = false;
				} else
					cerr << "ERROR -- process::waitProcs forwarding signal " << sig << " -- " << sys_errlist[errno] << endl;
			}
#ifdef notdef
			else
				signal_cerr << "Signal " << sig << " in " << result << endl;
#endif
		}
    }
	
  } while ( ignore );

  if( result > 0 ) {
#if defined(USES_LIBDYNINSTRT_SO)
	  if( WIFSTOPPED(*status) ) {
		  process *curr = findProcess( result );
		  if (!curr->dyninstLibAlreadyLoaded() && curr->wasCreatedViaAttach())
		  {
			  // make sure we are stopped in the eyes of paradynd - naim
			  bool wasRunning = (curr->status() == running);
			  if (curr->status() != stopped)
				  curr->Stopped();   
			  if(curr->isDynamicallyLinked()) {
				  curr->handleIfDueToSharedObjectMapping();
			  }
			  if (curr->trapDueToDyninstLib()) {
				  // we need to load libdyninstRT.so.1 - naim
				  curr->handleIfDueToDyninstLib();
			  }
			  if (wasRunning) 
				  if (!curr->continueProc()) assert(0);
		  }
	  }
#endif
#ifdef SIGNAL_DEBUG
	  if( WIFSIGNALED(*status) )
	  {
		  Address pc;
		  pc = getPC( result );
		  signal_cerr << "process::waitProcs -- Exit on signal #" << sig << " in " << result << "@" << (void*)pc << ", resignalling the process" << endl;
	  }
	  else if( WIFEXITED(*status) )
	  {
		  signal_cerr << "process::waitProcs -- Exit from " << result << endl;
	  }
#endif
  }// else if( errno )
    //perror( "process::waitProcs - waitpid" );
  return result;
}

// attach to an inferior process.
bool process::attach() {
  char procName[128];

  bool running;
  if( createdViaAttach )
    running = isRunning_();

  proc_fd = -1;

  // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
  // to paradynd in all cases?  How about when we are attaching to an
  // already-running process?  (Seems that in the latter case, no SIGTRAP
  // is automatically generated)

  // step 1) /proc open: attach to the inferior process memory file
  sprintf(procName,"/proc/%d/mem", (int)getPid());
  attach_cerr << "Opening memory space for process #" << getPid() << " at " << procName << endl;
  int fd = P_open(procName, O_RDWR, 0);
  if (fd < 0 ) {
    fprintf(stderr, "attach: open failed on %s: %s\n", procName, sys_errlist[errno]);
    return false;
  }

  proc_fd = fd;

  // Only if we are really attaching rather than spawning the inferior
  // process ourselves do we need to call PTRACE_ATTACH
  if( createdViaAttach || createdViaFork ) {
	  attach_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
    if( 0 != P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) )
	{
		perror( "process::attach - PTRACE_ATTACH" );
		return false;
	}
  }

  if( createdViaAttach )
  {
    // If the process was running, it will need to be restarted, as
    // PTRACE_ATTACH kills it
    // Actually, the attach process contructor assumes that the process is
    // running.  While this is foolish, let's play along for now.
	if( status_ != running || !isRunning_() ) {
		if( 0 != P_ptrace(PTRACE_CONT, getPid(), 0, 0) )
		{
			perror( "process::attach - continue" );
		}
    }
  }

#ifdef notdef
  if( status_ != running && isRunning_() )
  {
	  attach_cerr << "fixing status_ => running" << endl;
	  status_ = running;
  }
  else if( status_ == running && !isRunning_() )
  {
	  attach_cerr << "fixing status_ => stopped" << endl;
	  status_ = stopped;
  }
#endif

  return true;
}

bool process::attach_() {
  return false; // (P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) != -1);
}

#if defined(USES_LIBDYNINSTRT_SO)
bool process::trapAtEntryPointOfMain()
{
  // is the trap instr at main_brk_addr?
  if( getPC(getPid()) == (Address)main_brk_addr)
    return(true);
  else
    return(false);
}

bool process::trapDueToDyninstLib()
{
  // is the trap instr at dyninstlib_brk_addr?
  if( getPC(getPid()) == (Address)dyninstlib_brk_addr)
    return(true);
  else
    return(false);
}

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
  Address theEBP = theIntRegs->ebp;

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

  delete[] savedRegs;
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
  function_base *f_main = findOneFunction("main");
  if (!f_main) {
    // we can't instrument main - naim
    showErrorCallback(108,"");
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
  //  Address codeBase = (this->findOneFunctionFromAll(DYNINST_LOAD_HIJACK_FUNCTION))->getAddress(this);
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

  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  vector<AstNode*> dlopenAstArgs(2);

  unsigned count = 0;

  AstNode *dlopenAst;

  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList[];
  extern int deadListSize;
  registerSpace *dlopenRegSpace = new registerSpace(deadListSize/sizeof(int), deadList, 0, NULL);
  dlopenRegSpace->resetSpace();

  // we need to make 1 call to dlopen, to load libdyninst.so.1 - nash

  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)0);
  // library name. We use a scratch value first. We will update this parameter
  // later, once we determine the offset to find the string - naim
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
  dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);

  Address dyninst_count = 0;
  dlopenAst->generateCode(this, dlopenRegSpace, (char *)scratchCodeBuffer,
			  dyninst_count, true, true);
  writeDataSpace((void *)(codeBase+count), dyninst_count, (char *)scratchCodeBuffer);
  count += dyninst_count;

  instruction insnTrap;
  generateBreakPoint(insnTrap);
  writeDataSpace((void *)(codeBase + count), 2, insnTrap.ptr());
  dyninstlib_brk_addr = codeBase + count;
  count += 2;

  char libname[256];
#ifdef BPATCH_LIBRARY  /* dyninst API loads a different run-time library */
  if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
    strcpy((char*)libname,(char*)getenv("DYNINSTAPI_RT_LIB"));
  } else {
    string msg = string("Environment variable DYNINSTAPI_RT_LIB is not defined,"
        " should be set to the pathname of the dyninstAPI_RT runtime library.");
    showErrorCallback(101, msg);
    return false;
  }
#else
  if (getenv("PARADYN_LIB") != NULL) {
    strcpy((char*)libname,(char*)getenv("PARADYN_LIB"));
  } else {
    string msg = string("PARADYN_LIB has not been defined for ") +
                 string("process") + string(pid);
    showErrorCallback(101, msg);
    return false;
  }
#endif

  Address dyninstlib_addr = (Address) (codeBase + count);

  writeDataSpace((void *)(codeBase + count), strlen(libname)+1,
		 (caddr_t)libname);
  count += strlen(libname)+1;
  // we have now written the name of the library after the trap - naim

  assert(count<=BYTES_TO_SAVE);

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

  // save registers
  savedRegs = getRegisters();
  assert((savedRegs!=NULL) && (savedRegs!=(void *)-1));
  // save the stack frame of _start()
  user_regs_struct *regs = (user_regs_struct*)savedRegs;
  Address theEBP = regs->ebp;

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
  if (!changePC(codeBase,savedRegs))
  {
	  logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
	  assert(0);
  }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif

  return true;
}

Address process::get_dlopen_addr() const {
  if (dyn != NULL)
    return(dyn->get_dlopen_addr());
  else 
    return(0);
}
#endif

bool process::isRunning_() const {
   // determine if a process is running by doing low-level system checks, as
   // opposed to checking the 'status_' member vrble.  May assume that attach()
   // has run, but can't assume anything else.

  char procName[64];
  char sstat[132];
  char *token = NULL;

  sprintf(procName,"/proc/%d/stat", (int)pid);
  FILE *sfile = P_fopen(procName, "r");
  fread( sstat, 128, 1, sfile );
  fclose( sfile );

  char status;
  if( !( strtok( sstat, " (" ) && strtok( NULL, " )" ) && ( token = strtok( NULL, " " ) ) ) )
    assert( false && "Shouldn't happen" );
  status = token[0];

  if( status == 'T' )
    return false;
  else
    return true;
}


// TODO is this safe here ?
bool process::continueProc_() {
  int ret;

  if (!checkStatus()) 
    return false;

  ptraceOps++; ptraceOtherOps++;

/* choose either one of the following ptrace calls, but not both. 
 * The choice must be consistent with that in stop_ and
 * ptraceKludge::continueProcess.
 */
  ret = P_ptrace(PTRACE_CONT, getPid(), 1, 0);

  if (ret == -1)
  {
	  /*if( isRunning_() )
		  ret = 0;
		  else*/
		  perror("continueProc_()");
  }

  return ret != -1;
}

#ifdef BPATCH_LIBRARY
bool process::terminateProc_()
{
  if (!checkStatus()) 
    return false;

  if( kill( getPid(), SIGKILL ) != 0 )
    return false;
  else
    return true;
}
#endif

// TODO ??
bool process::pause_() {
  if (!checkStatus()) 
    return false;
  ptraceOps++; ptraceOtherOps++;
  bool wasStopped = (status() == stopped);
  if (status() != neonatal && !wasStopped)
    return (loopUntilStopped());
  else
    return true;
}

bool process::detach_() {
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  close( proc_fd );
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}

#ifdef BPATCH_LIBRARY
bool process::API_detach_(const bool cont) {
//  assert(cont);
  if (!checkStatus())
    return false;
  ptraceOps++; ptraceOtherOps++;
  close( proc_fd );
  if (!cont) P_kill(pid, SIGSTOP);
  return (ptraceKludge::deliverPtrace(this, PTRACE_DETACH, 1, SIGCONT));
}
#endif

bool process::dumpCore_(const string/* coreFile*/) { return false; }

bool process::writeTextWord_(caddr_t inTraced, int data) {
//  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
  return writeDataSpace_(inTraced, sizeof(int), (caddr_t) &data);
}

bool process::writeTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
//  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced << " len=" << amount << endl; cerr.flush();
  return writeDataSpace_(inTraced, amount, inSelf);
}

#ifdef BPATCH_SET_MUTATIONS_ACTIVE
bool process::readTextSpace_(void *inTraced, u_int amount, const void *inSelf) {
  return readDataSpace_( inTraced, amount, inSelf );
}
#endif

bool process::writeDataSpace_(void *inTraced, u_int amount, const void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

#if defined(PTRACEDEBUG_EXCESSIVE)
  int check;
#if !defined(PTRACEDEBUG_ALWAYS)
  if( debug_ptrace )
#endif
    fprintf( stderr, "(linux)writeDataSpace_  amount=%d  %#.8x -> %#.8x data=0x", amount, (int)inSelf, (int)inTraced );
#elif defined(PTRACEDEBUG)
#if !defined(PTRACEDEBUG_ALWAYS)
  if( debug_ptrace )
#endif
    fprintf( stderr, "(linux)writeDataSpace_  amount=%d  %#.8x -> %#.8x\n", amount, (int)inSelf, (int)inTraced );
#endif
  unsigned char buf[sizeof(int)];
  u_int count, off, addr = (int)inTraced, dat = (int)inSelf;

  off = addr % sizeof(int);
  if( off != 0 || amount < sizeof(int) ) {
    addr -= off;
    *((int*)buf) = P_ptrace( PTRACE_PEEKTEXT, getPid(), addr, 0 );
	if( errno )
	{
      char errb[150];
      sprintf( errb, "process::writeDataSpace_, ptrace( PEEKTXT, %d, %#.8x, &tmp )", getPid(), addr );
      perror( errb );
      return false;
	}
#ifdef PTRACEDEBUG_EXCESSIVE
#if !defined(PTRACEDEBUG_ALWAYS)
    if( debug_ptrace )
#endif
      fprintf( stderr, "%.8x:0x", *((int*)buf) );
#endif
  }

  for( count = 0; count < amount; count++ ) {
    //int le_off;
    //le_off = sizeof(int) - off - 1;
    /*    switch (off) {
    case 0: le_off = 0; break;
    case 1: le_off = 1; break;
    case 2: le_off = 3; break;
    case 3: le_off = 2; break;
    default: assert( false );
    } */
    buf[off] = *((unsigned char*)(dat+count));
#ifdef PTRACEDEBUG_EXCESSIVE
#if !defined(PTRACEDEBUG_ALWAYS)
    if( debug_ptrace )
#endif
      fprintf( stderr, "%.2x", 0x0000 | *((unsigned char*)(dat+count)) );
#endif
    off++; off %= sizeof(int);
    if( !off || (count == amount-1) ) {
      if( -1 == P_ptrace( PTRACE_POKETEXT, getPid(), addr, *((int*)buf) ) ) {
	perror( "process::writeDataSpace, ptrace PTRACE_POKETXT" );
	return false;
      }
      off = amount - count - 1;
#ifdef PTRACEDEBUG_EXCESSIVE
#if !defined(PTRACEDEBUG_ALWAYS)
      if( debug_ptrace ) {
#endif
	check = P_ptrace( PTRACE_PEEKTEXT, getPid(), addr, 0 );
	fprintf( stderr, ":%#.8x", check );
	if( off > 0 )
	  fprintf( stderr, " | 0x" );
#if !defined(PTRACEDEBUG_ALWAYS)
      }
#endif
#endif
      addr += sizeof(int);
      if( off < sizeof(int) && off > 0 ) {
	/*if( -1 == P_ptrace( PTRACE_PEEKTEXT, getPid(), addr, (int)buf ) ) {
	  perror( "process::writeDataSpace, ptrace PTRACE_PEEKTXT" );
	  return false;
	  }*/
	assert( proc_fd != -1 );
	if( ((lseek(proc_fd, (off_t)addr, SEEK_SET)) != (off_t)addr) ||
	    read(proc_fd, (char*)buf, sizeof(int)) != sizeof(int) ) {
		//printf("writeDataSpace_: error in read word addr = 0x%x\n",addr);
	  return false;
	}

#ifdef PTRACEDEBUG_EXCESSIVE
#if !defined(PTRACEDEBUG_ALWAYS)
	if( debug_ptrace ) {
#endif
	  check = P_ptrace( PTRACE_PEEKTEXT, getPid(), addr, 0 );
	  fprintf( stderr, "%.8x:0x", check );
#if !defined(PTRACEDEBUG_ALWAYS)
	}
#endif
#endif
      }
      off = 0;
    }
  }

#if defined(PTRACEDEBUG_EXCESSIVE)
#if !defined(PTRACEDEBUG_ALWAYS)
  if( debug_ptrace )
#endif
  fprintf( stderr, "\n" );
#endif

  return true;
}

bool process::readDataSpace_(const void *inTraced, u_int amount, void *inSelf) {
  ptraceOps++; ptraceBytes += amount;

#ifdef PTRACEDEBUG
#if !defined(PTRACEDEBUG_ALWAYS)
  if( debug_ptrace )
#endif
    fprintf( stderr, "(linux)readDataSpace_  amount=%d  %#.8x <- %#.8x\n", amount, (int)inSelf, (int)inTraced );
#endif

  int tries = 5;

  if( proc_fd != -1 ) {
    while (true) {
      if((lseek(proc_fd, (off_t)inTraced, SEEK_SET)) != (off_t)inTraced) {
	fprintf( stderr, "(linux)readDataSpace_: error in lseek addr = 0x%x amount = %d\n",(u_int)inTraced,amount);
	return false;
      }
      int result = read(proc_fd, inSelf, amount);
      if( result < 0 ) {
	fprintf( stderr, "(linux)readDataSpace_  amount=%d  %#.8x <- %#.8x\n", amount, (int)inSelf, (int)inTraced );
	//perror( "process::readDataSpace_ - read" );
	return false;
      } else if( result == 0 ) {
		  //if( errno )
		//perror( "process::readDataSpace_" );
		  if( tries-- == 0 )
		  {
#if defined(PDYN_DEBUG) || defined(PTRACEDEBUG)
			  fprintf( stderr, "process::readDataSpace_ -- Failed to read( /proc/*/mem ), trying ptrace\n" );
#endif
			  break;
		  }
      }
	  u_int res = result;
      assert( res <= amount );
      if( res == amount )
	return true;
      // We weren't able to read atomically, so reseek and reread.
#if defined(PDYN_DEBUG) || defined(PTRACEDEBUG)
	  fprintf( stderr, "process::readDataSpace_ -- Failed atomic read, reseeking and rereading\n" );
#endif
    }
  }

  // For some reason we couldn't or didn't open /proc/*/mem, so use ptrace
  // Should I remove this part? - nash

  // Well, since you sometimes don't seem to be able to read from the fd
  // maybe we do need to keep this - nash

  if( amount % sizeof(int) != 0 ||
      (int)inTraced % sizeof(int) != 0 ||
      (int)inSelf % sizeof(int) != 0 )
  {
	  unsigned char buf[sizeof(int)];
	  u_int count, off, addr = (int)inTraced, dat = (int)inSelf;
	  bool begin = true;

	  off = addr % sizeof(int);
	  addr -= off;

	  for( count = 0; count < amount; count++ ) {
		  if( !off || begin ) {
			  begin = false;
			  *((int*)buf) = P_ptrace( PTRACE_PEEKTEXT, getPid(), addr, 0 );
			  if( errno )
			  {
				  perror( "process::readDataSpace, ptrace PTRACE_PEEKTXT" );
				  return false;
			  }
			  addr += sizeof(int);
		  }
		  *((unsigned char*)(dat+count)) = buf[off];
		  off++; off %= sizeof(int);
	  }
	  return true;
  }

  {
	  u_int count, result;
	  int *dst = (int*)inSelf;
	  const int *addr = (const int*)inTraced;

	  for( count = 0; count < amount; count += sizeof(int), addr++, dst++ ) {
		  result = P_ptrace( PTRACE_PEEKTEXT, getPid(), (int)addr, 0 );
		  if( errno ) {
			  perror( "process::readDataSpace, ptrace PTRACE_PEEKTXT" );
			  return false;
		  }
		  *dst = result;
	  }
  
	  return true;
  }
}

/*
bool process::findCallee(instPoint &instr, function_base *&target){
  fprintf( stderr, "findCallee not implemented\n" );
  return false;
}
*/

bool process::readDataFromFrame(Address currentFP, Address *fp, Address *rtn, bool )
{
  bool readOK=true;
  struct {
    int fp;
    int rtn;
  } addrs;

  //
  // for the x86, the frame-pointer (EBP) points to the previous frame-pointer,
  // and the saved return address is in EBP-4.
  //

  if (readDataSpace((caddr_t) (currentFP),
                    sizeof(int)*2, (caddr_t) &addrs, true)) {
    // this is the previous frame pointer
    *fp = addrs.fp;
    // return address
    *rtn = addrs.rtn;

    // if pc==0, then we are in the outermost frame and we should stop. We
    // do this by making fp=0.

    if ( (addrs.rtn == 0) || !isValidAddress(this,(Address) addrs.rtn) ) {
      readOK=false;
    }
  }
  else {
    readOK=false;
  }

  return(readOK);
}

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
string process::tryToFindExecutable(const string & /* iprogpath */, int pid) {
  return string("/proc/") + string(pid) + "/exe";
}

/*
 * The old, ugly version that we don't need but can waste space anyhow
 * /
string process::tryToFindExecutable(const string &iprogpath, int pid) {
   // returns empty string on failure.
   // Otherwise, returns a full-path-name for the file.  Tries every
   // trick to determine the full-path-name, even though "progpath" may be
   // unspecified (empty string).
   
   // Remember, we can always return the empty string...no need to
   // go nuts writing the world's most complex algorithm.

   attach_cerr << "welcome to tryToFindExecutable; progpath=" << iprogpath << ", pid=" << pid << endl;

   const string progpath = expand_tilde_pathname(iprogpath);

   // Trivial case: if "progpath" is specified and the file exists then nothing needed
   if (exists_executable(progpath)) {
     attach_cerr << "tryToFindExecutable succeeded immediately, returning "
		 << progpath << endl;
     return progpath;
   }

   attach_cerr << "tryToFindExecutable failed on filename " << progpath << endl;

   string argv0, path, cwd;

   char buffer[128];
   sprintf(buffer, "/proc/%d/environ", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
     attach_cerr << "tryToFindExecutable failed since open of /proc/ * /environ failed" << endl;
     return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc/ * /environ okay" << endl;

   int strptr = 0;
   while( true ) {
     string env_value = extract_string( procfd, (char*)strptr );
     if( !env_value.length() )
       break;

     if (env_value.prefixed_by("PWD=") || env_value.prefixed_by("CWD=")) {
       cwd = env_value.string_of() + 4; // skip past "PWD=" or "CWD="
       attach_cerr << "get_ps_stuff: using PWD value of: " << cwd << endl;
       if( path.length() )
	 break;
     } else if (env_value.prefixed_by("PATH=")) {
       path = env_value.string_of() + 5; // skip past the "PATH="
       attach_cerr << "get_ps_stuff: using PATH value of: " << path << endl;
       if( cwd.length() )
	 break;
     }

     strptr += env_value.length() + 1;
   }

   close( procfd );

   sprintf(buffer, "/proc/%d/cmdline", pid);
   procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
     attach_cerr << "tryToFindExecutable failed since open of /proc/ * /cmdline failed" << endl;
     return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc/ * /cmdline okay" << endl;

   argv0 = extract_string( procfd, (char*)0 );
   close( procfd );

   if ( argv0.length() && path.length() && cwd.length() ) {
     // the following routine is implemented in the util lib.
     string result;
     if (executableFromArgv0AndPathAndCwd(result, argv0, path, cwd)) {
       attach_cerr << "tryToFindExecutable: returning " << result << endl;

       // I feel picky today, let's make certain that /proc agrees that
       // this is the executable by checking the inode number of
       // /proc/ * /exe against the inode number of result

       sprintf(buffer, "/proc/%d/exe", pid);
       struct stat t_stat;
       int t_inode;
       if( stat( buffer, &t_stat ) ) {
	 t_inode = t_stat.st_ino;
	 if( stat( buffer, &t_stat ) && t_inode != t_stat.st_ino )
	   cerr << "tryToFindExecutable: WARNING - found executable does not match /proc" << endl;
       }

       return result;
     }
   }

   attach_cerr << "tryToFindExecutable: giving up" << endl;

   return "";
}
*/

#ifdef SHM_SAMPLING
time64 process::getInferiorProcessCPUtime() /* const */ {
  /*
  rusage usage;
  time64 now;
  if( getrusage( RUSAGE_CHILDREN, &usage ) ) {
    perror( "process::getInferiorProcessCPUtime" );
    return 0;
  }
  now = usage.ru_utime.tv_sec*1000000 + usage.ru_utime.tv_usec + usage.ru_stime.tv_sec*1000000 + usage.ru_stime.tv_usec;

  return now;
  */

  time64 now = 0;
  int bufsize = 150, utime, stime;
  char buf[bufsize], *buf2;
  static int realHZ = 0;
  // Determine the number of jiffies/sec by checking the clock idle time in
  // /proc/uptime against the jiffies idle time in /proc/stat
  if( realHZ == 0 ) {
    double uptimeReal;
    int uptimeJiffies;
    FILE *tmp = P_fopen( "/proc/uptime", "r" );
    assert( tmp );
    assert( 1 == fscanf( tmp, "%*f %lf", &uptimeReal ) );
    fclose( tmp );
    tmp = P_fopen( "/proc/stat", "r" );
    assert( tmp );
    assert( 1 == fscanf( tmp, "%*s %*d %*d %*d %d", &uptimeJiffies ) );
    fclose( tmp );
    realHZ = (int)floor( (double)uptimeJiffies / uptimeReal );
#ifdef notdef
    fprintf( stderr, "Determined jiffies/sec as %d\n", realHZ );
#endif
  }

  sprintf( buf, "/proc/%d/stat", getPid() );

  int fd;

  // The reason for this complicated method of reading and sseekf-ing is
  // to ensure that we read enough of the buffer 'atomically' to make sure
  // the data is consistent.  Is this necessary?  I *think* so. - nash
  do {
    fd = P_open(buf, O_RDONLY, 0);
    if (fd < 0) {
      shmsample_cerr << "getInferiorProcessCPUtime: open failed: " << sys_errlist[errno] << endl;
      return false;
    }

    buf2 = new char[ bufsize ];

    /*size_t rsize = */P_read( fd, buf2, bufsize-1 );

    if( 2 == sscanf( buf2, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %d %d ", &utime, &stime ) ) {
      // These numbers are in 'jiffies' or clock ticks.  For now, we
      // check at the beginning the ratio between the idle seconds in
      // /proc/uptime and the idle jiffies in /proc/stat, and later we can
      // use some kind of gethrvptime for the whole thing. - nash
      // Oh, and I'm also assuming that process time includes system time
      now = ( (time64)1000000 * ( (time64)utime + (time64)stime ) ) / (time64)realHZ;
      break;
    }

    delete [] buf2;
    shmsample_cerr << "Inferior CPU time buffer expansion (" << bufsize << ")" << endl;
    bufsize = bufsize * 2;

    P_close( fd );
  } while ( true );

  delete [] buf2;
  P_close(fd);

  return now;
}
#endif // SHM_SAMPLING

bool process::loopUntilStopped() {
  int flags = WUNTRACED | WNOHANG;
  bool stopSig = false;
  int count = 0;
  /* make sure the process is stopped in the eyes of ptrace */

  while (true) {
    int waitStatus;
    int ret = P_waitpid( getPid(), &waitStatus, flags );
	if( ret == 0 ) {
	  if( !stopSig )
	  {
		  stopSig = true;
		  stop_();
	  }
	  else if( ++count > 10 && !isRunning_() )
		  break;
	} else if ((ret == -1 && errno == ECHILD) || (WIFEXITED(waitStatus))) {
      // the child is gone.
      handleProcessExit(this, WEXITSTATUS(waitStatus));
      return(false);
    } else if (WIFSIGNALED(waitStatus)) {
      handleProcessExit(this, WTERMSIG(waitStatus));
      return false;
    } else if (WIFSTOPPED(waitStatus)) {
      int sig = WSTOPSIG(waitStatus);
      if ( sig == SIGSTOP ) {
        break; // success
      } else {
        extern int handleSigChild(int, int);
        handleSigChild(pid, waitStatus);
      }
    }
    else {
      logLine("Problem stopping process\n");
      abort();
    }
  }
  return true;
}

#ifndef BPATCH_LIBRARY
bool process::dumpImage() {return false;}
#else
bool process::dumpImage(string imageFileName) 
{
    int newFd;
    image *im;
    int length = 0;
    string command;

    im = getImage();
    string origFile = im->file();


    // first copy the entire image file
    command = "cp ";
    command += origFile;
    command += " ";
    command += imageFileName;
    system(command.string_of());

    // now open the copy
    newFd = open(imageFileName.string_of(), O_RDWR, 0);
    if (newFd < 0) {
	// log error
	return false;
    }

    Elf *elfp = elf_begin(newFd, ELF_C_READ, 0);
    Elf_Scn *scn = 0;
    u_int baseAddr = 0;
    int offset = 0;

    Elf32_Ehdr*	ehdrp;
    Elf_Scn* shstrscnp  = 0;
    Elf_Data* shstrdatap = 0;
    Elf32_Shdr* shdrp;

    assert(ehdrp = elf32_getehdr(elfp));
    assert(((shstrscnp = elf_getscn(elfp, ehdrp->e_shstrndx)) != 0) &&
           ((shstrdatap = elf_getdata(shstrscnp, 0)) != 0));
    const char* shnames = (const char *) shstrdatap->d_buf;

    while ((scn = elf_nextscn(elfp, scn)) != 0) {
	const char* name;

	shdrp = elf32_getshdr(scn);
	name = (const char *) &shnames[shdrp->sh_name];
	if (!strcmp(name, ".text")) {
	    offset = shdrp->sh_offset;
	    length = shdrp->sh_size;
	    baseAddr = shdrp->sh_addr;
	    break;
	}
    }


    char tempCode[length];


    bool ret = readTextSpace_((void *) baseAddr, length, tempCode);
    if (!ret) {
       // log error
       return false;
    }

    lseek(newFd, offset, SEEK_SET);
    write(newFd, tempCode, length);
    close(newFd);

    return true;
}
#endif

#ifndef BPATCH_LIBRARY

float OS::compute_rusage_cpu() { return 0; }

float OS::compute_rusage_sys() { return 0; }

float OS::compute_rusage_min() { return 0; }

float OS::compute_rusage_maj() { return 0; }

float OS::compute_rusage_swap() { return 0; }

float OS::compute_rusage_io_in() { return 0; }

float OS::compute_rusage_io_out() { return 0; }

float OS::compute_rusage_msg_send() { return 0; }

float OS::compute_rusage_msg_recv() { return 0; }

float OS::compute_rusage_sigs() { return 0; }

float OS::compute_rusage_vol_cs() { return 0; }

float OS::compute_rusage_inv_cs() { return 0; }

#endif

int getNumberOfCPUs()
{
  return(1);
}

Address process::read_inferiorRPC_result_register(Register reg) {
   // On x86, the result is always stashed in %EAX

   int raddr = EAX * 4;
   int eaxval = ptraceKludge::deliverPtrace(this, PTRACE_PEEKUSER, raddr, 0);
   if( errno ) {
     perror( "process::read_inferiorRPC_result_register; ptrace PEEKUSER" );
     assert(false);
   }
   return eaxval;
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
    sprintf(errorLine, "               name %s\n", (entry.name()).string_of());
    logLine(errorLine);

    sprintf(errorLine, "  target_pdf : symTabName %s\n",
	    (target_pdf->symTabName()).string_of());
    logLine(errorLine);    
    sprintf(errorLine , "              prettyName %s\n",
	    (target_pdf->symTabName()).string_of());
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

// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's function_base.  
// If the function has not yet been bound, then "target" is set to the 
// function_base associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
// The assumption here is that for all processes sharing the image containing
// this instPoint they are going to bind the call target to the same function. 
// For shared objects this is always true, however this may not be true for
// dynamic executables.  Two a.outs can be identical except for how they are
// linked, so a call to fuction foo in one version of the a.out may be bound
// to function foo in libfoo.so.1, and in the other version it may be bound to 
// function foo in libfoo.so.2.  We are currently not handling this case, since
// it is unlikely to happen in practice.
bool process::findCallee(instPoint &instr, function_base *&target){
    
    if((target = (function_base *)instr.iPgetCallee())) {
 	return true; // callee already set
    }

    // find the corresponding image in this process  
    const image *owner = instr.iPgetOwner();
    bool found_image = false;
    Address base_addr = 0;
    if(symbols == owner) {  found_image = true; } 
    else if(shared_objects){
        for(u_int i=0; i < shared_objects->size(); i++){
            if(owner == ((*shared_objects)[i])->getImage()) {
		found_image = true;
		base_addr = ((*shared_objects)[i])->getBaseAddress();
		break;
            }
	}
    } 
    if(!found_image) {
        target = 0;
        return false; // image not found...this is bad
    }

    // get the target address of this function
    Address target_addr = 0;
//    Address insn_addr = instr.iPgetAddress(); 
    target_addr = instr.getTargetAddress();

    if(!target_addr) {  
	// this is either not a call instruction or an indirect call instr
	// that we can't get the target address
        target = 0;
        return false;
    }

    // see if there is a function in this image at this target address
    // if so return it
    pd_Function *pdf = 0;
    if( (pdf = owner->findFunctionInInstAndUnInst(target_addr,this)) ) {
        target = pdf;
        instr.set_callee(pdf);
	return true; // target found...target is in this image
    }

    // else, get the relocation information for this image
    const Object &obj = owner->getObject();
    const vector<relocationEntry> *fbt;
    if(!obj.get_func_binding_table_ptr(fbt)) {
	target = 0;
	return false; // target cannot be found...it is an indirect call.
    }

    // find the target address in the list of relocationEntries
    for(u_int i=0; i < fbt->size(); i++) {
	if((*fbt)[i].target_addr() == target_addr) {
	    // check to see if this function has been bound yet...if the
	    // PLT entry for this function has been modified by the runtime
	    // linker
	    pd_Function *target_pdf = 0;
	    if(hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
                target = target_pdf;
                instr.set_callee(target_pdf);
	        return true;  // target has been bound
	    } 
	    else {
		// just try to find a function with the same name as entry 
		target = findOneFunctionFromAll((*fbt)[i].name());
		if(target){
	            return true;
		}
		else {  
		    // KLUDGE: this is because we are not keeping more than
		    // one name for the same function if there is more
		    // than one.  This occurs when there are weak symbols
		    // that alias global symbols (ex. libm.so.1: "sin" 
		    // and "__sin").  In most cases the alias is the same as 
		    // the global symbol minus one or two leading underscores,
		    // so here we add one or two leading underscores to search
		    // for the name to handle the case where this string 
		    // is the name of the weak symbol...this will not fix 
		    // every case, since if the weak symbol and global symbol
		    // differ by more than leading underscores we won't find
		    // it...when we parse the image we should keep multiple
		    // names for pd_Functions

		    string s = string("_");
		    s += (*fbt)[i].name();
		    target = findOneFunctionFromAll(s);
		    if(target){
	                return true;
		    }
		    s = string("__");
		    s += (*fbt)[i].name();
		    target = findOneFunctionFromAll(s);
		    if(target){
	                return true;
		    }
		}
	    }
	    target = 0;
	    return false;
	}
    }
    target = 0;
    return false;  
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
        target_pdf = findpdFunctionIn(bound_addr);
	if(!target_pdf){
            return false;
	}
        return true;	
    }
    return false;
}
