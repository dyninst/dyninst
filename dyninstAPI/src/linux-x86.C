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

// $Id: linux-x86.C,v 1.45 2004/03/12 23:18:04 legendre Exp $

#include <fstream>

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
#include <elf.h>
#include <libelf.h>

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
#include "dyninstAPI/src/dyn_thread.h"

#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#include "dyninstAPI/src/dyn_lwp.h"
#include <sstream>

#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
//#include "saveSharedLibrary.h" 

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

// The following were defined in process.C
extern unsigned enable_pd_attach_detach_debug;

#if ENABLE_DEBUG_CERR == 1
#define attach_cerr if (enable_pd_attach_detach_debug) cerr
#else
#define attach_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_inferior_rpc_debug;

#if ENABLE_DEBUG_CERR == 1
#define inferiorrpc_cerr if (enable_pd_inferior_rpc_debug) cerr
#else
#define inferiorrpc_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_shm_sampling_debug;

#if ENABLE_DEBUG_CERR == 1
#define shmsample_cerr if (enable_pd_shm_sampling_debug) cerr
#else
#define shmsample_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_fork_exec_debug;

#if ENABLE_DEBUG_CERR == 1
#define forkexec_cerr if (enable_pd_fork_exec_debug) cerr
#else
#define forkexec_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_signal_debug;

#if ENABLE_DEBUG_CERR == 1
#define signal_cerr if (enable_pd_signal_debug) cerr
#else
#define signal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

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

/* ********************************************************************** */

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs) {
   // Cycle through all registers, reading each from the
   // process user space with ptrace(PTRACE_PEEKUSER ...
   int error;
   bool errorFlag = false;
   assert(get_lwp_id() != 0);
   error = P_ptrace(PTRACE_GETREGS, get_lwp_id(), 0, (int)&(regs->gprs) );
   if( error ) {
      perror("dyn_lwp::getRegisters PTRACE_GETREGS" );
      errorFlag = true;
   } else {
      error = P_ptrace(PTRACE_GETFPREGS, get_lwp_id(), 0, (int)&(regs->fprs));
      if( error ) {
         perror("dyn_lwp::getRegisters PTRACE_GETFPREGS" );
         errorFlag = true;
      }
   }
   
   if( errorFlag )
      return false;
   else
      return true;
}

bool dyn_lwp::changePC(Address loc,
                       struct dyn_saved_regs */*ignored registers*/)
{
   Address regaddr = EIP * INTREGSIZE;
   assert(get_lwp_id() != 0);
   if (0 != P_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, loc )) {
      perror( "dyn_lwp::changePC - PTRACE_POKEUSER" );
      return false;
   }
   
   return true;
}

bool dyn_lwp::clearOPC() {
   Address regaddr = ORIG_EAX * INTREGSIZE;
   assert(get_lwp_id() != 0);
   if (0 != P_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, -1UL)) {
      perror( "dyn_lwp::changePC - PTRACE_POKEUSER" );
      return false;
   }
   return true;
}

static bool changeBP(int pid, Address loc) {
   Address regaddr = EBP * INTREGSIZE;
   if (0 != P_ptrace(PTRACE_POKEUSER, pid, regaddr, loc )) {
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

bool dyn_lwp::executingSystemCall() 
{
  return false;
}

bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs) {
   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

   bool retVal = true;
   
   assert(get_lwp_id() != 0);
   if( P_ptrace( PTRACE_SETREGS, get_lwp_id(), 0,(int)&(regs.gprs) ) )
   {
      perror("dyn_lwp::restoreRegisters PTRACE_SETREGS" );
      retVal = false;
   }
   
   if( P_ptrace( PTRACE_SETFPREGS, get_lwp_id(), 0, (int)&(regs.fprs)))
   {
      perror("dyn_lwp::restoreRegisters PTRACE_SETFPREGS" );
      retVal = false;
   }

   return retVal;
}

// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
   if(status() == running) {
      cerr << "    performance problem in call to dyn_lwp::getActiveFrame\n"
           << "       successive pauses and continues with ptrace calls\n";
   }

   Address pc, fp, sp;
   fp = deliverPtraceReturn(PTRACE_PEEKUSER, 0 + EBP * INTREGSIZE, 0);
   if (errno) return Frame();

   pc = deliverPtraceReturn(PTRACE_PEEKUSER, 0 + EIP * INTREGSIZE, 0);
   if (errno) return Frame();

   sp = deliverPtraceReturn(PTRACE_PEEKUSER, 0 + UESP * INTREGSIZE, 0);
   if (errno) return Frame();

   return Frame(pc, fp, sp, proc_->getPid(), NULL, this, true);
}

// MT problem FIXME

Address getPC(int pid) {
   Address regaddr = EIP * INTREGSIZE;
   int res;
   res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   if(errno == ESRCH) { //ccw 6 sep 2002
      //pause and try again, let the mutatee have time
      //to ptrace(TRACEME)
      sleep(2);
      res = P_ptrace (PTRACE_PEEKUSER, pid, regaddr, 0);
   }
   if( errno ) {
      perror( "getPC" );
      return 0; // Shut up the compiler
   } else {
      assert(res);
      return (Address)res;
   }
}

bool process::loadDYNINSTlibCleanup()
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

  dyn_lwp *lwp_to_use = NULL;

  if(process::IndependentLwpControl() && getRepresentativeLWP() == NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();
  
  // restore registers
  assert(savedRegs != NULL);
  lwp_to_use->restoreRegisters(*savedRegs);

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
  if (!writeDataSpace ((void*)(theEBP-6*sizeof(int)),6*sizeof(int),savedStackFrame)) return false;

  if( isRunning_() )
	  cerr << "WARNING -- process is running at trap from dlopenDYNINSTlib" << endl;

  delete savedRegs;
  savedRegs = NULL;
  return true;
}

bool process::handleTrapAtEntryPointOfMain()
{
    assert(main_brk_addr);
    // restore original instruction 
    if (!writeDataSpace((void *)main_brk_addr, 2, (char *)savedCodeBuffer))
        return false;
    main_brk_addr = 0;
    return true;
}

bool process::insertTrapAtEntryPointOfMain()
{
  function_base *f_main = 0;
  pdvector<pd_Function *> *pdfv=NULL;
 
  // first check a.out for function symbol
  if (NULL == (pdfv = symbols->findFuncVectorByPretty("main")) || !pdfv->size()) {
      // we can't instrument main - naim
      // Paradyn: this pops up an error window which requires user input
      //showErrorCallback(108,"main() uninstrumentable");
      return false;
  }
  
  if (pdfv->size() > 1) {
      cerr << __FILE__ << __LINE__ << ": found more than one main! using the first" << endl;
  }
  f_main = (function_base *) (*pdfv)[0];
  assert(f_main);
  Address addr = f_main->addr();
  
  // save original instruction first
  if (!readDataSpace((void *)addr, 2, savedCodeBuffer, true))
      return false;

  // and now, insert trap
  instruction insnTrap;
  generateBreakPoint(insnTrap);

  // x86. have to use SIGILL instead of SIGTRAP
  if (!writeDataSpace((void *)addr, 2, insnTrap.ptr()))
      return false;

  main_brk_addr = addr;
  return true;
}

void emitCallRel32(unsigned disp32, unsigned char *&insn);


#define UNKNOWN          0x0  //Don't know
#define VSYSCALL_PAGE    0x1  //PC is in the vsyscall page
#define SIG_HANDLER      0x2  //Current function is a signal handler
#define ALLOCATES_FRAME  0x3  //Function allocates a frame
#define SAVES_FP_NOFRAME 0x4  //Function doesn't allocate a frame but does
                              // save the FP (result of gcc's 
                              // -fomit-frame-pointer)
#define NO_USE_FP        0x5  //Function doesn't allocate a frame and doesn't
                              // use the frame pointer (also result of gcc's
                              // -fomit-frame-pointer)

extern bool isFramePush(instruction &i);
void *parseVsyscallPage(char *buffer, unsigned dso_size, process *p);
extern Address getRegValueAtFrame(void *ehf, Address pc, int reg, 
				  int *reg_map, process *p, bool *error);

#define VSYS_DEFAULT_START 0xffffe000
#define VSYS_DEFAULT_END   0xfffff000

/**
 * Signal handler return points can be found in the vsyscall page.
 * this function reads the symbol information that describes these
 * points out of the vsyscall page.
 **/
#define VSYS_SIGRETURN_NAME "_sigreturn"
static void getVSyscallSignalSyms(char *buffer, unsigned dso_size, process *p)
{
  Elf_Scn *sec;
  Elf32_Shdr *shdr;
  Elf32_Ehdr *ehdr;
  Elf32_Sym *syms;
  unsigned i;
  size_t dynstr, shstr;

  Elf *elf = elf_memory(buffer, dso_size);
  if (elf == NULL)
    goto err_handler;
  ehdr = elf32_getehdr(elf);

  //Get string section indexes
  shstr = ehdr->e_shstrndx;
  for (i = 0; i < ehdr->e_shnum; i++)
  {
    shdr = elf32_getshdr(elf_getscn(elf, i));
    if (shdr != NULL && shdr->sh_type == SHT_STRTAB && 	
	strcmp(elf_strptr(elf, shstr, shdr->sh_name), ".dynstr") == 0)
    {
      dynstr = i;
      break;
    }
  }

  //For each section..
  for (sec = elf_nextscn(elf, NULL); sec != NULL; sec = elf_nextscn(elf, sec))      
  {
    shdr = elf32_getshdr(sec);
    if (shdr == NULL) goto err_handler;
    if (shdr->sh_type == SHT_DYNSYM)
    {
      syms = (Elf32_Sym *) elf_getdata(sec, NULL)->d_buf;
      //For each symbol ..
      for (i = 0; i < (shdr->sh_size / sizeof(Elf32_Sym)); i++)
      {
	if ((syms[i].st_info & 0xf) == STT_FUNC)
	{
	  //Check if this is a function symbol
	  char *name = elf_strptr(elf, dynstr, syms[i].st_name);
	  if (strstr(name, VSYS_SIGRETURN_NAME) != NULL)
	  {	    
	    p->addSignalHandlerAddr(syms[i].st_value);
	  } 
	}
      }
    }
  }
  
  elf_end(elf);
  return;

err_handler:
  if (elf != NULL)
    elf_end(elf);
}

static void calcVSyscallFrame(process *p)
{
  void *result;
  unsigned dso_size;
  char *buffer;

  /**
   * If we've already calculated and cached the DSO information then 
   * just return.
   **/
  if (p->getVsyscallStart() != 0x0)
    return; 

  /**
   * Read the location of the vsyscall page from /proc/.
   **/
  if (!p->readAuxvInfo())
  {
    //If we couldn't read from auxv, we're probably on linux 2.4.
    // It should be safe to assume the following defaults:
    p->setVsyscallRange(VSYS_DEFAULT_START, VSYS_DEFAULT_END);
    p->setVsyscallData(NULL);
    return;
  }
  
  /**
   * Read the vsyscall page out of process memory.
   **/
  dso_size = p->getVsyscallEnd() - p->getVsyscallStart();
  buffer = (char *) calloc(1, dso_size);
  assert(buffer);
  if (!p->readDataSpace((caddr_t) p->getVsyscallStart(), dso_size, buffer,
			false))
  {
    //Linux 2.6.0 - Linux 2.6.2 has a  bug where ptrace 
    // can't read from the DSO.  The process can read the memory, 
    // it's just ptrace that's acting stubborn.  
    //Solution: Upgrade to 2.6.3
    pdstring msg = pdstring("Could not read from vsyscall page.\n");
    showErrorCallback(129, msg);
    goto err_handler;
  }

  getVSyscallSignalSyms(buffer, dso_size, p);
  result = parseVsyscallPage(buffer, dso_size, p);
  p->setVsyscallData(result);
  
  return;

err_handler:
  if (buffer != NULL)
    free(buffer);
  p->setVsyscallData(NULL);
  return;
}

static int getFrameStatus(process *p, unsigned pc)
{
   pd_Function *this_func;
   
   calcVSyscallFrame(p);

   if (p->isInSignalHandler(pc))
      return SIG_HANDLER;
   else if (pc >= p->getVsyscallStart() && pc < p->getVsyscallEnd())
      return VSYSCALL_PAGE;

   this_func = p->findFuncByAddr(pc);   
   if (this_func == NULL)
      return UNKNOWN;
   else if (!this_func->hasNoStackFrame())
      return ALLOCATES_FRAME;   
   else if (this_func->savesFramePointer())
     return SAVES_FP_NOFRAME;
   else
     return NO_USE_FP;
}

static bool isPrevInstrACall(Address addr, process *p, pd_Function **callee)
{
   codeRange *range = p->findCodeRangeByAddress(addr);
   pdvector<instPoint *> callsites;

   if (range == NULL)
     return false;

   if (range->reloc_ptr != NULL)
      callsites = range->reloc_ptr->funcCallSites();
   else if (range->function_ptr != NULL)
      callsites = range->function_ptr->funcCalls(NULL);
   else
     return false;
      
   for (unsigned i = 0; i < callsites.size(); i++)
   {
     instPoint *site = callsites[i];
     if (site->absPointAddr(p) + site->insnAtPoint().size() == addr)
     {
       *callee = site->getCallee();
       return true;
     }
   }   

   return false; 
}

//The estimated maximum frame size when having to do an 
// exhaustive search for a frame.
#define MAX_STACK_FRAME 8192

//Constant values used for the registers in the vsyscall page.
#define DW_CFA  0
#define DW_ESP 4
#define DW_EBP 5
#define DW_PC  8
#define MAX_DW_VALUE 8

Frame Frame::getCallerFrame(process *p) const
{
   /**
    * for the x86, the frame-pointer (EBP) points to the previous 
    * frame-pointer, and the saved return address is in EBP-4.
    **/
   struct {
      int fp;
      int rtn;
   } addrs;

   int status;

   status = getFrameStatus(p, pc_);

   if (status == VSYSCALL_PAGE)
   {
      Frame ret(*this);
      void *vsys_data;

      ret.uppermost_ = false;

      if ((vsys_data = p->getVsyscallData()) == NULL)
      {
        /**
         * No vsyscall stack walking data present (we're probably 
         * on Linux 2.4) we'll go ahead and treat the vsyscall page 
         * as a leaf
         **/
        if (!p->readDataSpace((caddr_t) sp_, sizeof(int), 
                             (caddr_t) &addrs.rtn, true))
          return Frame();
        ret.fp_ = fp_;
        ret.pc_ = addrs.rtn;
        ret.sp_ = sp_ + 4;	
      }
      else
      {
        /**
         * We have vsyscall stack walking data, which came from
         * the .eh_frame section of the vsyscall DSO.  We'll use
         * getRegValueAtFrame to parse the data and get the correct
         * values for %esp, %ebp, and %eip
         **/
        int reg_map[MAX_DW_VALUE+1];
        bool error;

        //Set up the register values array for getRegValueAtFrame
        memset(reg_map, 0, sizeof(reg_map));
        reg_map[DW_EBP] = fp_;
        reg_map[DW_ESP] = sp_;
        reg_map[DW_PC] = pc_;

        //Calc frame start
        reg_map[DW_CFA] = getRegValueAtFrame(vsys_data, pc_, DW_CFA, reg_map, 
                                 p, &error);
        if (error) return Frame();

	//Calc registers values.
        ret.pc_ = getRegValueAtFrame(vsys_data, pc_, DW_PC, reg_map, p, 
				     &error);
        if (error) return Frame();
        ret.fp_ = getRegValueAtFrame(vsys_data, pc_, DW_EBP, reg_map, p, 
				     &error);
        if (error) return Frame();
        ret.sp_ = reg_map[DW_CFA];	
      }
      return ret;
   }
   if (status == SIG_HANDLER &&
       p->readDataSpace((caddr_t)(sp_+28), sizeof(int),
			(caddr_t)&addrs.fp, true) &&
       p->readDataSpace((caddr_t)(sp_+60), sizeof(int),
			(caddr_t)&addrs.rtn, true))

   {  
      /**
       * If the current frame is for the signal handler function, then we need 
       * to read the information about the next frame from the data saved by 
       * the signal handling mechanism.
       **/
      Frame ret(*this);
      ret.fp_ = addrs.fp;
      ret.pc_ = addrs.rtn;
      ret.sp_ = sp_ + 64;
      ret.uppermost_ = false;
      return ret;
   }   
   if (status == ALLOCATES_FRAME &&
       p->readDataSpace((caddr_t) fp_, 2*sizeof(int), 
			(caddr_t) &addrs, true))
   {
      /**
       * The function that created this frame uses the standard 
       * prolog: push %ebp; mov %esp->ebp .  We can read the 
       * appropriate data from the frame pointer.
       **/
      Frame ret(*this);
      ret.fp_ = addrs.fp;
      ret.pc_ = addrs.rtn;
      ret.sp_ = fp_+8;
      ret.uppermost_ = false;
      return ret;
   }
   if (status == SAVES_FP_NOFRAME || status == NO_USE_FP)
   {
      /**
       * The evil case.  We don't have a valid frame pointer.  We'll
       * start a search up the stack from the sp, looking for an address
       * that could qualify as the result of a return.  We'll do a few
       * things to try and keep ourselves from accidently following a 
       * constant value that looks like a return:
       *  - Make sure the address in the return follows call instruction.
       *  - See if the resulting frame pointer is part of the stack.
       *  - Peek ahead.  If the stack trace from following the address doesn't
       *     end with the top of the stack, we probably shouldn't follow it.
       **/
      Frame ret(*this);
      ret.uppermost_ = false;
      
      unsigned int estimated_sp;
      unsigned int estimated_ip;
      unsigned int estimated_fp;
      unsigned int stack_top;
      pd_Function *callee;
      bool result;

      /**
       * Calculate the top of the stack.
       **/
      stack_top = 0;
      if (sp_ < 0xbfffffff && sp_ > 0xbfffffff - 0x200000)
      {
         //If we're within two megs of the linux x86 default stack, we'll
         // assume that's the one in use.
        stack_top = 0xc0000000 - 4;
      }
      else if (p->multithread_capable() && thread_ != NULL)
      {
         int stack_diff = thread_->get_stack_addr() - sp_;
         if (stack_diff < MAX_STACK_FRAME && stack_diff > 0)
            stack_top = thread_->get_stack_addr();
      }
      if (stack_top == 0)
         stack_top = sp_ + MAX_STACK_FRAME;
      assert(sp_ < stack_top);

      /**
       * Search for the correct return value.
       **/
      estimated_sp = sp_;
      for (; estimated_sp < stack_top; estimated_sp++)
      {
         result = p->readDataSpace((caddr_t) estimated_sp, sizeof(int), 
                                   (caddr_t) &estimated_ip, false);
         
         if (!result) break;

         //If the instruction that preceeds this address isn't a call
         // instruction, then we'll go ahead and look for another address.
	 if (!isPrevInstrACall(estimated_ip, p, &callee))
	   continue;

         //Given this point for the top of our stack frame, calculate the 
         // frame pointer         
         if (status == SAVES_FP_NOFRAME)
         {
            result = p->readDataSpace((caddr_t) estimated_sp-4, sizeof(int),
                                      (caddr_t) &estimated_fp, false);
            if (!result) break;
         }
         else //status == NO_USE_FP
         {
            estimated_fp = fp_;
         }

         //If the call instruction calls into the current function, then we'll
         // just skip everything else and assume we've got the correct return
         // value (fingers crossed).
         pd_Function *cur_func = p->findFuncByAddr(pc_);
         if (cur_func != NULL && callee != NULL &&
             cur_func->match(callee))
         {
            ret.pc_ = estimated_ip;
            ret.fp_ = estimated_fp;
            ret.sp_ = estimated_sp+4;
            return ret;
         }
         
         //Check the validity of the frame pointer.  It's possible the
         // previous frame doesn't have a valid fp, so we won't be able
         // to rely on the check in this case.
         pd_Function *next_func = p->findFuncByAddr(estimated_ip);
         if (next_func != NULL && 
             getFrameStatus(p, estimated_ip) == ALLOCATES_FRAME &&
             (estimated_fp < fp_ || estimated_fp > stack_top))
         {
            continue;
         }

         //BAD HACK: The initial value of %esi when main starts sometimes
         // points to an area in the guard_setup function that may look
         // like a valid return value in some versions of libc.  Since it's
         // easy for the value of %esi to get saved on the stack somewhere,
         // we'll special case this.
         if (callee == NULL && next_func != NULL &&
             !strcmp(next_func->prettyName().c_str(), "__guard_setup"))
         {
           continue;
         }

         ret.pc_ = estimated_ip;
         ret.fp_ = estimated_fp;
         ret.sp_ = estimated_sp+4;

         return ret;
      }
   }

   return Frame();
}

char* process::dumpPatchedImage(pdstring imageFileName){ //ccw 7 feb 2002 

	addLibrary addLibraryElf;
	unsigned int errFlag=0;
	pdvector<imageUpdate*> compactedHighmemUpdates;
	char *mutatedSharedObjects=0;
	int mutatedSharedObjectsSize = 0, mutatedSharedObjectsIndex=0;
	char *directoryName = 0;
	unsigned int baseAddr ;
	unsigned int tmpFlag;
	shared_object *sh_obj;

	if(!collectSaveWorldData){
                BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: BPatch_thread::enableDumpPatchedImage() not called.  No mutated binary saved\n");
                return NULL;
        }

	directoryName = saveWorldFindDirectory();

	if(!directoryName){
		return NULL;
	}

	strcat(directoryName, "/");	
	char* fullName = new char[strlen(directoryName) + strlen ( (const char*)imageFileName.c_str())+1];
        strcpy(fullName, directoryName);
        strcat(fullName, (const char*)imageFileName.c_str());

	unsigned int dl_debug_statePltEntry = 0x00016574;//a pretty good guess
	unsigned int dyninst_SharedLibrariesSize = 0;
	unsigned int mutatedSharedObjectsNumb=0;

	dl_debug_statePltEntry = saveWorldSaveSharedLibs(mutatedSharedObjectsSize, 
		dyninst_SharedLibrariesSize,directoryName,mutatedSharedObjectsNumb);

	//the mutatedSO section contains a list of the shared objects that have been mutated

	//UPDATED: 24 jul 2003 to include flag.
	//the flag denotes whether the shared lib is Dirty (1) or only DirtyCalled (0)
	// This is going to be a section that looks like this:
	// string
	// addr
	// flag
	// ...
	// string
	// addr
	// flag

	if(mutatedSharedObjectsSize){
		mutatedSharedObjectsSize += mutatedSharedObjectsNumb * sizeof(unsigned int);
		mutatedSharedObjects = new char[mutatedSharedObjectsSize];
		//i ignore the dyninst RT lib here and in process::saveWorldSaveSharedLibs
		for(int i=0;shared_objects && i<(int)shared_objects->size() ; i++) {
			sh_obj = (*shared_objects)[i];
			if( (sh_obj->isDirty() || sh_obj->isDirtyCalled()) && 
				/* there are a number of libraries that we cannot save even if they are
					mutated. */
				NULL==strstr(sh_obj->getName().c_str(),"libdyninstAPI_RT") && 
				NULL== strstr(sh_obj->getName().c_str(),"ld-linux.so") && 
				NULL==strstr(sh_obj->getName().c_str(),"libc")){ //ccw 24 jul 2003

				//fprintf(stderr," mutatedSharedObjectsIndex %d %s\n", mutatedSharedObjectsIndex,sh_obj->getName().c_str() ); //ccw 8 mar 2004
				memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
					sh_obj->getName().c_str(),
					strlen(sh_obj->getName().c_str())+1);

				mutatedSharedObjectsIndex += strlen( sh_obj->getName().c_str())+1;
				baseAddr = sh_obj->getBaseAddress();
				memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]), &baseAddr, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	


				//set flag
				tmpFlag = ((sh_obj->isDirty() &&  NULL==strstr(sh_obj->getName().c_str(),"libc"))?1:0);	
				memcpy( &(mutatedSharedObjects[mutatedSharedObjectsIndex]), &tmpFlag, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	

			}
		}	
	}

	
	char *dyninst_SharedLibrariesData =saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);
		
	writeBackElf *newElf = new writeBackElf((const char*) getImage()->file().c_str(),
			                "/tmp/dyninstMutatee",errFlag);
        newElf->registerProcess(this);

        highmemUpdates.sort(imageUpdateSort);

        newElf->compactSections(highmemUpdates, compactedHighmemUpdates);

        newElf->alignHighMem(compactedHighmemUpdates);

	saveWorldCreateHighMemSections(compactedHighmemUpdates, highmemUpdates, (void*) newElf);


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
	saveWorldCreateDataSections((void*) newElf);

        newElf->createElf();

	elf_update(newElf->getElf(),ELF_C_WRITE); 
       	if(!addLibraryElf.driver(newElf->getElf(),fullName, "libdyninstAPI_RT.so.1")){
		BPatch_reportError(BPatchSerious,122,"dumpPatchedImage: addLibraryElf() failed!  No mutated binary saved\n");
		delete [] directoryName; //ccw 27 jun 2003
                return NULL;
	}
	return directoryName;	

}

// Laziness here: this func is used by the iRPC code
// to get result registers. Don't use it other than that. 

Address dyn_lwp::readRegister(Register /*reg*/) {
   // On x86, the result is always stashed in %EAX
   if(status() == running) {
      cerr << "    performance problem in call to dyn_lwp::readRegister\n"
           << "       successive pauses and continues with ptrace calls\n";
   }

   int ret = deliverPtraceReturn(PTRACE_PEEKUSER, EAX*4, 0);
   return (Address)ret;
}

syscallTrap *process::trapSyscallExitInternal(Address syscall) {
    syscallTrap *trappedSyscall = NULL;
    
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }
    if (trappedSyscall) {
        trappedSyscall->refcount++;
        return trappedSyscall;
    }
    else {
        // Add a trap at this address, please
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = syscall;
        readDataSpace( (void*)syscall, 2, trappedSyscall->saved_insn, true);

        instruction insnTrap;
        generateBreakPoint(insnTrap);
        writeDataSpace((void *)syscall, 2, insnTrap.ptr());

        syscallTraps_.push_back(trappedSyscall);
        
        return trappedSyscall;
    }
    // Should never be reached
    return NULL;
}

bool dyn_lwp::stepPastSyscallTrap() {
    assert(0 && "Unimplemented");
    return false;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    // Decrement the reference count, and if it's 0 remove the trapped
    // system call
    assert(trappedSyscall->refcount > 0);
    
    trappedSyscall->refcount--;
    if (trappedSyscall->refcount > 0)
        return true;
    
    // Erk... we hit 0. Undo the trap
    if (!writeDataSpace((void *)trappedSyscall->syscall_id, 2,
                        trappedSyscall->saved_insn))
        return false;
    // Now that we've reset the original behavior, remove this
    // entry from the vector

    pdvector<syscallTrap *> newSyscallTraps;
    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (trappedSyscall != syscallTraps_[iter])
            newSyscallTraps.push_back(syscallTraps_[iter]);
    }
    syscallTraps_ = newSyscallTraps;

    delete trappedSyscall;
    return true;
}

int dyn_lwp::hasReachedSyscallTrap() {
    if (!trappedSyscall_) return false;
    Frame active = getActiveFrame();
    return active.getPC() == trappedSyscall_->syscall_id;
}

Address dyn_lwp::getCurrentSyscall() {
    Frame active = getActiveFrame();
    return active.getPC();
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


bool process::getDyninstRTLibName() {
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            pdstring msg = pdstring("Environment variable ") +
                            pdstring("DYNINSTAPI_RT_LIB") +
                            pdstring(" has not been defined for process ")
                            + pdstring(pid);
            showErrorCallback(101, msg);
            return false;
        }
    }
    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        pdstring msg = pdstring("Runtime library ") + dyninstRT_name
        + pdstring(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }
    return true;
}

bool process::loadDYNINSTlib() {
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
  if (getSymbolInfo(libc_version_symname, libc_vers)) {
      char libc_version[ libc_vers.size() + 1 ];
      libc_version[ libc_vers.size() ] = '\0';
      readDataSpace( (void *)libc_vers.addr(), libc_vers.size(), libc_version, true );
      if (!strncmp(libc_version, "2.0", 3))
	      libc_21 = false;
  }
  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);

  unsigned char scratchCodeBuffer[BYTES_TO_SAVE];
  pdvector<AstNode*> dlopenAstArgs( 2 );

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
         function_base *func = findOnlyOneFunction(DL_OPEN_FUNC_NAME);
         if (!func) {
            std::ostringstream os(std::ios::out);
            os << "Internal error: unable to find addr of " << DL_OPEN_FUNC_NAME << endl;
            logLine(os.str().c_str());
            showErrorCallback(80, (const char *) os.str().c_str());
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

/*  if (dyninstRT_name.length()) { //ccw 28 aug 2002 
    // use the library name specified on the start-up command-line
  } else {*/
    // check the environment variable
    if (getenv(DyninstEnvVar) != NULL) {
      dyninstRT_name = getenv(DyninstEnvVar);
    } else {
      pdstring msg = pdstring("Environment variable " + pdstring(DyninstEnvVar)
                   + " has not been defined for process ") + pdstring(pid);
      showErrorCallback(101, msg);
      return false;
    }
/*  }*/
  if (access(dyninstRT_name.c_str(), R_OK)) {
    pdstring msg = pdstring("Runtime library ") + dyninstRT_name
        + pdstring(" does not exist or cannot be accessed!");
    showErrorCallback(101, msg);
    return false;
  }

  Address dyninstlib_addr = (Address) (codeBase + count);
  writeDataSpace((void *)(codeBase + count), dyninstRT_name.length()+1,
		 (caddr_t)const_cast<char*>(dyninstRT_name.c_str()));
  count += dyninstRT_name.length()+1;
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
  dyn_lwp *lwp_to_use = NULL;
  if(process::IndependentLwpControl() && getRepresentativeLWP() ==NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();

  savedRegs = new dyn_saved_regs;
  bool status = lwp_to_use->getRegisters(savedRegs);

  assert((status!=false) && (savedRegs!=(void *)-1));
  // save the stack frame of _start()
  struct dyn_saved_regs new_regs;
  memcpy(&new_regs, savedRegs, sizeof(struct dyn_saved_regs));

  user_regs_struct *regs = (user_regs_struct*) &(savedRegs->gprs);
  
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
  attach_cerr << "Changing PC to " << (void*)codeBase << endl;

  lwp_to_use = NULL;

  if(process::IndependentLwpControl() && getRepresentativeLWP() ==NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();

  if (!libc_21)
  {
      if (! lwp_to_use->changePC(codeBase,NULL))
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }
  else
  {
      user_regs_struct *reg_ptr = (user_regs_struct *)&(new_regs.gprs);
      
      reg_ptr->eip = codeBase;

      if( libc_21 ) {
          reg_ptr->eax = dyninstlib_addr;
          reg_ptr->edx = DLOPEN_MODE;
          reg_ptr->ecx = codeBase;
      }

      if(! lwp_to_use->restoreRegisters(new_regs) )
      {
          logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
          assert(0);
      }
  }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif


  setBootstrapState(loadingRT);
  return true;
}


