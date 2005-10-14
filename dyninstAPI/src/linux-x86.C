/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: linux-x86.C,v 1.81 2005/10/14 16:37:48 legendre Exp $

#include <fstream>

#include "dyninstAPI/src/process.h"

#include <sys/ptrace.h>
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

#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/symtab.h"
//#include "dyninstAPI/src/func-reloc.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/InstrucIter.h"

#include "dyninstAPI/src/mapped_object.h" 

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

const char DL_OPEN_FUNC_NAME[] = "do_dlopen";

const char libc_version_symname[] = "__libc_version";


#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

#define NUM_REGS (17 /*+ NUM_FREGS*/)
#define NUM_FREGS 8
#define FP0_REGNUM NUM_REGS
#define FP7_REGNUM (FP0_REGNUM+7)
#define INTREGSIZE (sizeof(long))
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
   error = P_ptrace(PTRACE_GETREGS, get_lwp_id(), 0, (long)&(regs->gprs) );
   if( error ) {
      perror("dyn_lwp::getRegisters PTRACE_GETREGS" );
      errorFlag = true;
   } else {
      error = P_ptrace(PTRACE_GETFPREGS, get_lwp_id(), 0, (long)&(regs->fprs));
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
   Address regaddr = offsetof(struct user_regs_struct, PTRACE_REG_IP);
   assert(get_lwp_id() != 0);
   if (0 != P_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, loc )) {
      perror( "dyn_lwp::changePC - PTRACE_POKEUSER" );
      return false;
   }
   
   return true;
}

bool dyn_lwp::clearOPC() {
   Address regaddr = offsetof(struct user_regs_struct, PTRACE_REG_ORIG_AX);
   assert(get_lwp_id() != 0);
   if (0 != P_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, -1UL)) {
      perror( "dyn_lwp::changePC - PTRACE_POKEUSER" );
      return false;
   }
   return true;
}

static bool changeBP(int pid, Address loc) {
   Address regaddr = offsetof(struct user_regs_struct, PTRACE_REG_BP);
   if (0 != P_ptrace(PTRACE_POKEUSER, pid, regaddr, loc )) {
      perror( "process::changeBP - PTRACE_POKEUSER" );
      return false;
   }

   return true;
}

#define REG_STR(x)	REG_STR_(x)
#define REG_STR_(x)	#x
void printRegs( void *save ) {
    user_regs_struct *regs = (user_regs_struct*)save;
    cerr
	<< REG_STR( PTRACE_REG_AX ) ": " << (void*)regs->PTRACE_REG_AX
	<< REG_STR( PTRACE_REG_BX ) ": " << (void*)regs->PTRACE_REG_BX
	<< REG_STR( PTRACE_REG_CX ) ": " << (void*)regs->PTRACE_REG_CX
	<< REG_STR( PTRACE_REG_DX ) ": " << (void*)regs->PTRACE_REG_DX << endl
	<< REG_STR( PTRACE_REG_DI ) ": " << (void*)regs->PTRACE_REG_DI
	<< REG_STR( PTRACE_REG_SI ) ": " << (void*)regs->PTRACE_REG_SI << endl
	<< REG_STR( PTRACE_REG_CS ) ": " << (void*)regs->PTRACE_REG_CS
	<< REG_STR( PTRACE_REG_DS ) ": " << (void*)regs->PTRACE_REG_DS
	<< REG_STR( PTRACE_REG_ES ) ": " << (void*)regs->PTRACE_REG_ES
	<< REG_STR( PTRACE_REG_FS ) ": " << (void*)regs->PTRACE_REG_FS
	<< REG_STR( PTRACE_REG_GS ) ": " << (void*)regs->PTRACE_REG_GS
	<< REG_STR( PTRACE_REG_SS ) ": " << (void*)regs->PTRACE_REG_SS << endl
	<< REG_STR( PTRACE_REG_IP ) ": " << (void*)regs->PTRACE_REG_IP
	<< REG_STR( PTRACE_REG_SP ) ": " << (void*)regs->PTRACE_REG_SP
	<< REG_STR( PTRACE_REG_BP ) ": " << (void*)regs->PTRACE_REG_BP << endl
	<< REG_STR( PTRACE_REG_ORIG_AX ) ": " << (void*)regs->PTRACE_REG_ORIG_AX
	<< REG_STR( PTRACE_REG_FLAGS ) ": " << (void*)regs->PTRACE_REG_FLAGS << endl;
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
   if( P_ptrace( PTRACE_SETREGS, get_lwp_id(), 0, (long)&(regs.gprs)))
   {
      perror("dyn_lwp::restoreRegisters PTRACE_SETREGS" );
      retVal = false;
   }
   
   if( P_ptrace( PTRACE_SETFPREGS, get_lwp_id(), 0, (long)&(regs.fprs)))
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

   errno = 0;
   fp = deliverPtraceReturn(PTRACE_PEEKUSER, offsetof(struct user_regs_struct, PTRACE_REG_BP), 0);
   if (errno) return Frame();

   pc = deliverPtraceReturn(PTRACE_PEEKUSER, offsetof(struct user_regs_struct, PTRACE_REG_IP), 0);
   if (errno) return Frame();

   sp = deliverPtraceReturn(PTRACE_PEEKUSER, offsetof(struct user_regs_struct, PTRACE_REG_SP), 0);
   if (errno) return Frame();

   return Frame(pc, fp, sp, proc_->getPid(), proc_, NULL, this, true);
}

// MT problem FIXME

Address getPC(int pid) {
   Address regaddr = offsetof(struct user_regs_struct, PTRACE_REG_IP);
   long res;
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

bool process::loadDYNINSTlibCleanup(dyn_lwp *trappingLWP)
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);

  Address codeBase = findFunctionToHijack(this);
  assert(codeBase);

  writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer);

  // restore registers
  assert(savedRegs != NULL);
  trappingLWP->restoreRegisters(*savedRegs);

  // restore the stack frame of _start()
  user_regs_struct *theIntRegs = (user_regs_struct *)savedRegs;
  RegValue theBP = theIntRegs->PTRACE_REG_BP;

  if( !theBP )
  {
	  theBP = theIntRegs->PTRACE_REG_SP;
  }

  assert (theBP);
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  if (!writeDataSpace ((void*)(theBP-6*sizeof(int)),6*sizeof(int),savedStackFrame)) return false;

  delete savedRegs;
  savedRegs = NULL;
  return true;
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp *trappingLWP)
{
    assert(main_brk_addr);
    assert(trappingLWP);
    // restore original instruction 
    // Use this for the size -- make sure that we're using the same
    // insn in both places. Or give savedCodeBuffer a size twin.

    if (!writeDataSpace((void *)main_brk_addr, sizeof(savedCodeBuffer), (char *)savedCodeBuffer))
        return false;

    if (! trappingLWP->changePC(main_brk_addr,NULL))
        {
            logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
            assert(0);
        }
    
    main_brk_addr = 0;
    return true;
}

bool process::insertTrapAtEntryPointOfMain()
{
    int_function *f_main = 0;
    pdvector<int_function *> funcs;
    
    //first check a.out for function symbol   
    bool res = findFuncsByPretty("main", funcs);
    if (!res)
    {
        logLine( "a.out has no main function. checking for PLT entry\n" );
        //we have not found a "main" check if we have a plt entry
	res = findFuncsByPretty( "DYNINST_pltMain", funcs );
 
	if (!res) {
            logLine( "no PLT entry for main found\n" );
            return false;
	  }       
    }
    
    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one main! using the first" << endl;
    }
    f_main = funcs[0];
    assert(f_main);
    Address addr = f_main->getAddress();

    // and now, insert trap
    // For some reason, using a trap breaks but using an illegal works. Anyone 
    // have any idea?
    // It looks like the trap PC is actual PC + 1...
    // This is ugly, but we'll have the "check if this was entry of main" function
    // check main_brk_addr or main_brk_addr + 1...
    startup_printf("Saving %d bytes from entry of main...\n", 
                   sizeof(savedCodeBuffer));

    // save original instruction first
    if (!readDataSpace((void *)addr, sizeof(savedCodeBuffer), savedCodeBuffer, true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
        fprintf(stderr, "Failed to read at address 0x%lx\n", addr);
        return false;
    }

    codeGen gen(1);
    instruction::generateTrap(gen);
    
    if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr()))
        return false;

    
    main_brk_addr = addr;

    signal_printf("Added trap at entry of main, address 0x%x\n", main_brk_addr);
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
#define TRAMP            0x6  //Trampoline

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
  size_t dynstr = 0, shstr;

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
	    // Aggravating... FC3 has these as offsets from the entry
	    // of the vsyscall page. Others have these as absolutes.
	    // We hates them, my precioussss....
	    Address signal_addr = syms[i].st_value;
	    if (signal_addr < p->getVsyscallStart()) {
	      p->addSignalHandler(syms[i].st_value + p->getVsyscallStart(), 4);
	    }
	    else 
	      p->addSignalHandler(syms[i].st_value, 4);
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

void calcVSyscallFrame(process *p)
{
  void *result;
  unsigned dso_size;
  char *buffer;

  /**
   * If we've already calculated and cached the DSO information then 
   * just return.
   **/
  if (p->getVsyscallStart() != 0x0) {
    return; 
  }

#if defined(arch_x86_64)
  // FIXME: HACK to disable vsyscall page for AMD64, for now
  p->setVsyscallRange(0x1000, 0x0);
  p->setVsyscallData(NULL);
  return;
#endif

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
     //Solution: (Bad hack) Making an RPC isn't safe, it could restart the
     // process.  Instead we'll read the DSO out of paradynd's
     // address space.  On Linux 2.6.0-2.6.2 the DSO doesn't change
     // within kernel versions, so one process' DSO should be as good
     // as any others.
     memcpy(buffer, (void *) p->getVsyscallStart(), dso_size);
  }
  getVSyscallSignalSyms(buffer, dso_size, p);
  result = parseVsyscallPage(buffer, dso_size, p);
  p->setVsyscallData(result);
  
  return;
}

static int getFrameStatus(process *p, unsigned long pc)
{
   codeRange *range;

   int_function *func = NULL;
   miniTrampInstance *mini = NULL;
   multiTramp *multi = NULL;
   baseTrampInstance *base = NULL;

   calcVSyscallFrame(p);

   if (p->isInSignalHandler(pc))
      return SIG_HANDLER;
   else if (pc >= p->getVsyscallStart() && pc < p->getVsyscallEnd())
      return VSYSCALL_PAGE;

   range = p->findCodeRangeByAddress(pc);
   func = range->is_function();
   multi = range->is_multitramp();
   mini = range->is_minitramp();
   if (multi)
       base = multi->getBaseTrampInstanceByAddr(pc);

   if (base) {
       if (base->isInInstru(pc))
           return TRAMP;
       else
           func = base->multiT->func();
   }
   else if (multi) {
       // Not in base tramp instrumented... we're effectively in the func
       func = multi->func();
   }
   else if (mini) {
       return TRAMP;
   }
   
   if (func == NULL)
      return UNKNOWN;
   else if (!func->hasNoStackFrame()) 
      return ALLOCATES_FRAME;   
   else if (func->savesFramePointer())
     return SAVES_FP_NOFRAME;
   else
     return NO_USE_FP;
}


static bool isPrevInstrACall(Address addr, process *p, int_function **callee)
{
   codeRange *range = p->findCodeRangeByAddress(addr);
   pdvector<instPoint *> callsites;

   if (range == NULL)
     return false;
   
   int_function *func_ptr = range->is_function();

   if (func_ptr != NULL)
     callsites = func_ptr->funcCalls();
   else
     return false;
      
   for (unsigned i = 0; i < callsites.size(); i++)
   {
     instPoint *site = callsites[i];

     // Argh. We need to check for each call site in each
     // instantiation of the function.
     if (site->match(addr - site->insn().size())) {
	 *callee = site->findCallee();
	 return true;
     }
   }   
   
   return false; 
}

/**
 * Sometimes a function that uses a stack frame may not yet have put up its
 * frame or have already taken it down by the time we reach it during a stack
 * walk.  This function returns true in this case.  offset is the distance
 * from the top of the stack to the return value for the caller.
 **/
static bool hasAllocatedFrame(Address addr, process *proc, int &offset)
{
    int frameSizeDontCare;
    codeRange *range = proc->findCodeRangeByAddress(addr);

    if (range &&
        range->is_basicBlockInstance()) {
        InstrucIter ii(range->get_address_cr(),
                       range->get_size_cr(),
                       proc);
        ii.setCurrentAddress(addr);
        if (ii.isAReturnInstruction() ||
            ii.isStackFramePreamble(frameSizeDontCare))
            {
                offset = 0;
                return false;
            }
        if (ii.isFrameSetup())
            {
                offset = proc->getAddressWidth();
                return false;
            }
    }
    return true;       
}

/**
 * If frame 'f' is a trampoline frame, this function returns true
 * if the trampoline was called by function entry or exit
 * instrumentation.
 **/
static bool isInEntryExitInstrumentation(Frame f)
{
   codeRange *range = f.getRange();
   miniTrampInstance *miniTI = range->is_minitramp();
   multiTramp *multi = range->is_multitramp();
   baseTrampInstance *baseTI = NULL;
   if (multi) baseTI = multi->getBaseTrampInstanceByAddr(f.getPC());

   if (baseTI == NULL)
   {
      if (miniTI == NULL)
         return false;
      baseTI = miniTI->baseTI;
   }
   const instPoint *preInst = baseTI->baseT->preInstP;
   if (preInst && 
       (preInst->getPointType() == functionEntry ||
        preInst->getPointType() == functionExit)) 
       // Not sure yet if function exit will be preInst or postInst...
       return true;
   const instPoint *postInst = baseTI->baseT->postInstP;
   if (postInst &&
       postInst->getPointType() == functionExit) 
       return true;

   return false;
}

extern int tramp_pre_frame_size_32;
extern int tramp_pre_frame_size_64;

//The estimated maximum frame size when having to do an 
// exhaustive search for a frame.
#define MAX_STACK_FRAME_SIZE 8192

// x86_64 uses a different stack address than standard x86.
#define MAX_STACK_FRAME_ADDR_64 0x0000007fbfffffff
#define MAX_STACK_FRAME_ADDR_32 0xbfffffff

//Constant values used for the registers in the vsyscall page.
#define DW_CFA  0
#define DW_ESP 4
#define DW_EBP 5
#define DW_PC  8
#define MAX_DW_VALUE 8

// constants for walking out of a signal handler
// these are offsets from the stack pointer in the signal
// tramp (__restore or __restore_rt) into the sigcontext
// struct that is placed on the sigtramp's frame
// TODO: obtain these from the appropriate header files
#define SIG_HANDLER_FP_OFFSET_32 28
#define SIG_HANDLER_PC_OFFSET_32 60
#define SIG_HANDLER_FRAME_SIZE_32 64
#define SIG_HANDLER_FP_OFFSET_64 120
#define SIG_HANDLER_PC_OFFSET_64 168
#define SIG_HANDLER_FRAME_SIZE_64 576


Frame Frame::getCallerFrame()
{
   /**
    * These two variables are only valid when this function is
    * called recursively.
    **/
   static Frame prevFrame;
   static bool prevFrameValid = false;

   /**
    * for the x86, the frame-pointer (EBP) points to the previous 
    * frame-pointer, and the saved return address is in EBP-4.
    **/
   struct {
      Address fp;
      Address rtn;
   } addrs;

   int status;

   Address newPC=0;
   Address newFP=0;
   Address newSP=0;
   Address newpcAddr=0;

   status = getFrameStatus(getProc(), getPC());

   if (status == VSYSCALL_PAGE)
   {
      void *vsys_data;

      if ((vsys_data = getProc()->getVsyscallData()) == NULL)
      {
        /**
         * No vsyscall stack walking data present (we're probably 
         * on Linux 2.4) we'll go ahead and treat the vsyscall page 
         * as a leaf
         **/
        if (!getProc()->readDataSpace((caddr_t) sp_, sizeof(int), 
                             (caddr_t) &addrs.rtn, true))
          return Frame();
        newFP = fp_;
        newPC = addrs.rtn;
        newSP = sp_+4;
        goto done;
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
                                 getProc(), &error);
        if (error) return Frame();

        //Calc registers values.
        newPC = getRegValueAtFrame(vsys_data, pc_, DW_PC, reg_map, getProc(), 
				     &error);
        if (error) return Frame();
        newFP = getRegValueAtFrame(vsys_data, pc_, DW_EBP, reg_map, getProc(), 
				     &error);
        if (error) return Frame();
        newSP = reg_map[DW_CFA];	
        goto done;
      }
   }
   else if (status == SIG_HANDLER)
   {
       int fp_offset, pc_offset, frame_size;
       if (getProc()->getAddressWidth() == 4) {
	   fp_offset = SIG_HANDLER_FP_OFFSET_32;
	   pc_offset = SIG_HANDLER_PC_OFFSET_32;
	   frame_size = SIG_HANDLER_FRAME_SIZE_32;
       }
       else {
	   fp_offset = SIG_HANDLER_FP_OFFSET_64;
	   pc_offset = SIG_HANDLER_PC_OFFSET_64;
	   frame_size = SIG_HANDLER_FRAME_SIZE_64;
       }

       if (!getProc()->readDataSpace((caddr_t)(sp_+fp_offset), sizeof(Address),
				     (caddr_t)&addrs.fp, true)) {
	   // FIXME
	   assert(0);
       }
       if (!getProc()->readDataSpace((caddr_t)(sp_+pc_offset), sizeof(Address),
				     (caddr_t)&addrs.rtn, true)) {
	   // FIXME
	   assert(0);
       }


      /**
       * If the current frame is for the signal handler function, then we need 
       * to read the information about the next frame from the data saved by 
       * the signal handling mechanism.
       **/
     newFP = addrs.fp;
     newPC = addrs.rtn;
     newSP = sp_ + frame_size;
      goto done;
   }   
   else if ((status == ALLOCATES_FRAME || status == TRAMP))
   {
      /**
       * The function that created this frame uses the standard 
       * prolog: push %ebp; mov %esp->ebp .  We can read the 
       * appropriate data from the frame pointer.
       **/
      int offset = 0;
      // FIXME: for tramps, we need to check if we've saved the FP yet
      if ((status != TRAMP && !hasAllocatedFrame(pc_, getProc(), offset)) || 
          (prevFrameValid && isInEntryExitInstrumentation(prevFrame)))
      {
         addrs.fp = offset + sp_;
         if (!getProc()->readDataSpace((caddr_t) addrs.fp, sizeof(Address), 
                               (caddr_t) &addrs.rtn, true))
            return Frame();
         newPC = addrs.rtn;
         newFP = fp_;
         newSP = addrs.fp + getProc()->getAddressWidth();
      }
      else
      {
         if (!fp_)
            return Frame();
         if (!getProc()->readDataSpace((caddr_t) fp_, 2*sizeof(Address), 
                                       (caddr_t) &addrs, true))
            return Frame();
         newFP = addrs.fp;
         newPC = addrs.rtn;
         newSP = fp_+ (2 * sizeof(Address));
      }
      if (status == TRAMP)
	newSP += 
	    getProc()->getAddressWidth() == 8 ? tramp_pre_frame_size_64 : tramp_pre_frame_size_32;
      goto done;
   }
   else if (status == SAVES_FP_NOFRAME || status == NO_USE_FP)
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
      Address estimated_sp;
      Address estimated_ip;
      Address estimated_fp;
      Address stack_top;
      int_function *callee;
      bool result;

      /**
       * Calculate the top of the stack.
       **/
      Address max_stack_frame_addr =
#if defined(arch_x86_64)
	  getProc()->getAddressWidth() == 8 ? MAX_STACK_FRAME_ADDR_64 : MAX_STACK_FRAME_ADDR_32;
#else
          MAX_STACK_FRAME_ADDR_32;
#endif

      stack_top = 0;
      if (sp_ < max_stack_frame_addr && sp_ > max_stack_frame_addr - 0x200000)
      {
	  //If we're within two megs of the linux x86 default stack, we'll
	  // assume that's the one in use.
	  stack_top = max_stack_frame_addr - 3; // Points to first possible integer
						// ** SIZE ISSUE **
      }
      else if (getProc()->multithread_capable() && thread_ != NULL)
      {
         int stack_diff = thread_->get_stack_addr() - sp_;
         if (stack_diff < MAX_STACK_FRAME_SIZE && stack_diff > 0)
            stack_top = thread_->get_stack_addr();
      }
      if (stack_top == 0)
         stack_top = sp_ + MAX_STACK_FRAME_SIZE;
      assert(sp_ < stack_top);

      /**
       * Search for the correct return value.
       **/
      estimated_sp = sp_;
      for (; estimated_sp < stack_top; estimated_sp++)
      {
	  result = getProc()->readDataSpace((caddr_t) estimated_sp, sizeof(Address), 
					    (caddr_t) &estimated_ip, false);
         
         if (!result) break;

         //If the instruction that preceeds this address isn't a call
         // instruction, then we'll go ahead and look for another address.
         if (!isPrevInstrACall(estimated_ip, getProc(), &callee))
            continue;

         //Given this point for the top of our stack frame, calculate the 
         // frame pointer         
         if (status == SAVES_FP_NOFRAME)
         {
            result = getProc()->readDataSpace((caddr_t) estimated_sp-4, sizeof(int),
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
         int_function *cur_func = getProc()->findFuncByAddr(pc_);
         if (cur_func != NULL &&
             cur_func == callee)
	   {
	     newPC = estimated_ip;
	     newFP = estimated_fp;
	     newSP = estimated_sp+4;
	     goto done;
         }
         
         //Check the validity of the frame pointer.  It's possible the
         // previous frame doesn't have a valid fp, so we won't be able
         // to rely on the check in this case.
         int_function *next_func = getProc()->findFuncByAddr(estimated_ip);
         if (next_func != NULL && 
             getFrameStatus(getProc(), estimated_ip) == ALLOCATES_FRAME &&
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

         newPC = estimated_ip;
         newFP = estimated_fp;
         newSP = estimated_sp + getProc()->getAddressWidth();
         goto done;
      }
   }

   return Frame();

 done:

   Frame ret = Frame(newPC, newFP, newSP, newpcAddr, this);

   if (status == TRAMP)
   {
      /**
       * Instrumentation has its own stack frame (created in the
       * base tramp), but is really an extension of the function. 
       * We skip the function. Platform-independent code can
       * always regenerate it if desired.
       **/
      prevFrameValid = true;
      prevFrame = *this;
      ret = ret.getCallerFrame();
      prevFrameValid = false;
   }
   return ret;
}

bool Frame::setPC(Address newpc) {
    fprintf(stderr, "Implement me! Changing frame PC from %lx to %lx\n",
            pc_, newpc);
    //pc_ = newpc;
    //range_ = NULL;
    return false;
}

void process::setPrelinkCommand(char *command){
	if(command){
		struct stat buf;
		int retVal = stat(command,&buf);	
		if( retVal != -1){
			systemPrelinkCommand = new char[strlen(command)+9];

			sprintf(systemPrelinkCommand, "%s -r ",command);
		}
	}
}

bool process::prelinkSharedLibrary(pdstring originalLibNameFullPath, 
                                   char* dirName, Address baseAddr)
{
	char *newLibName = saveWorldFindNewSharedLibraryName(originalLibNameFullPath,
                                                        dirName);
	bool res = false;
	char *command = new char[originalLibNameFullPath.length() + strlen(newLibName) + 10];
	
	if(!systemPrelinkCommand){

		char * bpatchPrelink = BPatch::bpatch->getPrelinkCommand();
		if( bpatchPrelink){
			setPrelinkCommand(bpatchPrelink);
		}else{
			setPrelinkCommand("/usr/sbin/prelink");
		}
	}

	memset(command, '\0',originalLibNameFullPath.length() + strlen(newLibName) + 10);

	sprintf(command, "/bin/cp %s %s", originalLibNameFullPath.c_str(), newLibName);

	//fprintf(stderr, "RUNNING COMMAND: %s\n", command);

	//cp originalLibNameFullPath newLibName
	system(command);
	delete [] command;

	if(systemPrelinkCommand){
		char *prelinkCommand = new char[strlen(systemPrelinkCommand) + 64+strlen(newLibName) ];
		memset(prelinkCommand, '\0', strlen(systemPrelinkCommand) + 64+strlen(newLibName) );

		// /usr/sbin/prelink -r baseAddr newLibName
		sprintf(prelinkCommand,"%s 0x%x %s", systemPrelinkCommand, 
              (unsigned) baseAddr, newLibName);
		//fprintf(stderr, "RUNNING COMMAND %s\n", prelinkCommand);

		system(prelinkCommand);
		delete [] prelinkCommand;
	}
	res = true;

	return res;
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
	mapped_object *mapped_obj;

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

	// PRELINK every shared lib marked dirty or dirtycalled to the new directory:
	// Always save the API_RT lib
	for(unsigned i=0; i < mapped_objects.size() ; i++) {
		mapped_obj = mapped_objects[i];
		if((mapped_obj->isSharedLib()) && 
		   (mapped_obj->isDirty() || mapped_obj->isDirtyCalled() || strstr(mapped_obj->fileName().c_str(),"libdyninstAPI_RT") ) &&
				NULL== strstr(mapped_obj->fileName().c_str(),"ld-linux.so") &&  /* do not save some libs*/
				NULL==strstr(mapped_obj->fileName().c_str(),"libc")){ 
			/*fprintf(stderr,"\nWRITE BACK SHARED OBJ %s\n", mapped_obj->getName().c_str());*/

			if(!prelinkSharedLibrary(mapped_obj->fileName(),directoryName, mapped_obj->codeBase())){
				char *msg;
				msg = new char[mapped_obj->fileName().length() + strlen(directoryName)+128];
				sprintf(msg,"dumpPatchedImage: %s not saved to %s.\n.\nTry to use the original shared library with the mutated binary.\n",mapped_obj->fileName().c_str(),directoryName);

				BPatch_reportError(BPatchWarning,0,msg);
				delete [] msg;
			}
		}

	}

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
		for(unsigned i=0; i < mapped_objects.size() ; i++) {
			mapped_obj = mapped_objects[i];
			if((mapped_obj->isSharedLib()) && 
			   (mapped_obj->isDirty() || mapped_obj->isDirtyCalled()) && 
				/* there are a number of libraries that we cannot save even if they are
					mutated. */
				NULL==strstr(mapped_obj->fileName().c_str(),"libdyninstAPI_RT") && 
				NULL== strstr(mapped_obj->fileName().c_str(),"ld-linux.so") && 
				NULL==strstr(mapped_obj->fileName().c_str(),"libc")){ //ccw 24 jul 2003

				//bperr(" mutatedSharedObjectsIndex %d %s\n", mutatedSharedObjectsIndex,mapped_obj->fileName().c_str() ); //ccw 8 mar 2004
				memcpy(  & ( mutatedSharedObjects[mutatedSharedObjectsIndex]),
					mapped_obj->fileName().c_str(),
					strlen(mapped_obj->fileName().c_str())+1);

				mutatedSharedObjectsIndex += strlen( mapped_obj->fileName().c_str())+1;

				/* 	LINUX PROBLEM. in the link_map structure the map->l_addr field is NOT
					the load address of the dynamic object, as the documentation says.  It is the
					RELOCATED address of the object. If the object was not relocated then the
					value is ZERO.
	
					So, on LINUX we check the address of the dynamic section, map->l_ld, which is
					correct.
				*/

				Symbol info;
				pdstring dynamicSection = "_DYNAMIC";
				mapped_obj->getSymbolInfo(dynamicSection,info);
				baseAddr = mapped_obj->codeBase() + info.addr();
				//fprintf(stderr," %s DYNAMIC ADDR: %x\n",mapped_obj->fileName().c_str(), baseAddr);

				//baseAddr = mapped_obj->getBaseAddress();
				memcpy( & (mutatedSharedObjects[mutatedSharedObjectsIndex]), &baseAddr, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	


				//set flag
				tmpFlag = ((mapped_obj->isDirty() &&  NULL==strstr(mapped_obj->fileName().c_str(),"libc"))?1:0);	
				memcpy( &(mutatedSharedObjects[mutatedSharedObjectsIndex]), &tmpFlag, sizeof(unsigned int));
				mutatedSharedObjectsIndex += sizeof(unsigned int);	

			}
		}	
	}

//	dyninst_SharedLibrariesSize++; // 6 mar 2005 : we now want to include libdyninstAPI_RT.so in this section
	char *dyninst_SharedLibrariesData =saveWorldCreateSharedLibrariesSection(dyninst_SharedLibrariesSize);
	
		
	writeBackElf *newElf = new writeBackElf((const char*) mapped_objects[0]->parse_img()->file().c_str(),
						"/tmp/dyninstMutatee",errFlag);
        newElf->registerProcess(this);

	//libdyninstAPI_RT.so.  The RT lib will check to see that it is loaded
	//in the correct place when the mutated binary is run.
	Address rtlibAddr;
	for(unsigned i=0; i < mapped_objects.size() ; i++) {
		mapped_obj = mapped_objects[i];
		if( strstr(mapped_obj->fileName().c_str(),"libdyninstAPI_RT") ) {
			//rtlibAddr = mapped_obj->getBaseAddress();
			/* 	LINUX PROBLEM. in the link_map structure the map->l_addr field is NOT
				the load address of the dynamic object, as the documentation says.  It is the
				RELOCATED address of the object. If the object was not relocated then the
				value is ZERO.
	
				So, on LINUX we check the address of the dynamic section, map->l_ld, which is
				correct.
			*/

			Symbol info;
			pdstring dynamicSection = "_DYNAMIC";
			mapped_obj->getSymbolInfo(dynamicSection,info);
			rtlibAddr = mapped_obj->codeBase() + info.addr();
			//fprintf(stderr," %s DYNAMIC ADDR: %x\n",mapped_obj->fileName().c_str(), rtlibAddr);
		}
	}

	/*fprintf(stderr,"SAVING RTLIB ADDR: %x\n",rtlibAddr);*/
	newElf->addSection(0,&rtlibAddr,sizeof(Address),"rtlib_addr",false);


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
		delete newElf; //INSURE CCW
                return NULL;
	}
	delete newElf; //INSURE CCW
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

   long ret = deliverPtraceReturn(PTRACE_PEEKUSER, offsetof(struct user_regs_struct, PTRACE_REG_AX), 0);
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
        if (!readDataSpace( (void*)syscall, 2, trappedSyscall->saved_insn, true))
          fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);

        codeGen gen(1);
        instruction::generateTrap(gen);
        writeDataSpace((void *)syscall, gen.used(), gen.start_ptr());

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
      int_function *&target_pdf, Address base_addr) {

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
    /*
      // Size bad. <smack>
    sprintf(errorLine , "              size %i\n",
	    target_pdf->getSize());
    logLine(errorLine);
    */
    sprintf(errorLine , "              addr 0x%x\n",
	    (unsigned)target_pdf->getAddress());
    logLine(errorLine);

    sprintf(errorLine, "  base_addr  0x%x\n", (unsigned)base_addr);
    logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr) {

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

instruction generateTrapInstruction() {
	return instruction((const unsigned char*)"\017\013\0220\0220", ILLEGAL, 4);
	} /* end generateTrapInstruction() */


bool process::getDyninstRTLibName() {
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
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

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "_m32";
    const char *name = dyninstRT_name.c_str();

    if (getAddressWidth() != sizeof(void *) && !P_strstr(name, modifier)) {
	const char *split = P_strrchr(name, '/');

	if (!split) split = name;
	split = P_strchr(split, '.');
	if (!split) {
	    // We should probably print some error here.
	    // Then, of course, the user will find out soon enough.
	    return false;
	}

	dyninstRT_name = pdstring(name, split - name) +
			 pdstring(modifier) +
			 pdstring(split);
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
  Symbol libc_vers;
  if (getSymbolInfo(libc_version_symname, libc_vers)) {
    char libc_version[ sizeof(int)*libc_vers.size() + 1 ];
    libc_version[ sizeof(int)*libc_vers.size() ] = '\0';
    if (!readDataSpace( (void *)libc_vers.addr(), libc_vers.size(), libc_version, true ))
      fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
    if (!strncmp(libc_version, "2.0", 3))
      return loadDYNINSTlib_libc20();
  }
  return loadDYNINSTlib_libc21();
}

// Defined in inst-x86.C...
void emitPushImm(unsigned int imm, unsigned char *&insn); 

bool process::loadDYNINSTlib_libc21() {
#if false && defined(PTRACEDEBUG)
  debug_ptrace = true;
#endif
  startup_printf("**** LIBC21 dlopen for RT lib\n");
  // do_dlopen takes a struct argument. This is as follows:
  // const char *libname;
  // int mode;
  // void *result;
  // void *caller_addr
  // Now, we have to put this somewhere writable. The idea is to
  // put it on the stack....

  Address codeBase = findFunctionToHijack(this);

  if(!codeBase)
  {
      startup_cerr << "Couldn't find a point to insert dlopen call" << endl;
      return false;
  }

  startup_printf("(%d) writing in dlopen call at addr %p\n", getPid(), (void *)codeBase);

  codeGen scratchCodeBuffer(BYTES_TO_SAVE);

  // we need to make a call to dlopen to open our runtime library

  
  // Variables what we're filling in
  Address dyninstlib_str_addr = 0;
  Address dlopen_call_addr = 0;

  pdvector<int_function *> dlopen_funcs;
  if (!findFuncsByAll(DL_OPEN_FUNC_NAME, dlopen_funcs))
      assert(0);

  Address dlopen_addr = dlopen_funcs[0]->getAddress();

  assert(dyninstRT_name.length() < BYTES_TO_SAVE);
  // We now fill in the scratch code buffer with appropriate data
  startup_cerr << "Dyninst RT lib name: " << dyninstRT_name << endl;

  dyninstlib_str_addr = codeBase + scratchCodeBuffer.used();
  scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

  startup_printf("(%d) dyninst str addr at 0x%x\n", getPid(), dyninstlib_str_addr);

  startup_printf("(%d) after copy, %d used\n", getPid(), scratchCodeBuffer.used());

  // Now the real work begins...
  dlopen_call_addr = codeBase + scratchCodeBuffer.used();

#if defined(arch_x86_64)
  if (getAddressWidth() == 4) {
#endif

      // Push caller
      emitPushImm(dlopen_addr, scratchCodeBuffer);
      
      // Push hole for result
      emitPushImm(0, scratchCodeBuffer);
      
      // Push mode
      emitPushImm(DLOPEN_MODE, scratchCodeBuffer);
      
      // Push string addr
      emitPushImm(dyninstlib_str_addr, scratchCodeBuffer);
      
      // Push the addr of the struct: esp
      emitSimpleInsn(PUSHESP, scratchCodeBuffer);
      
      startup_printf("(%d): emitting call from 0x%x to 0x%x\n",
		     getPid(), codeBase + scratchCodeBuffer.used(), dlopen_addr);
      instruction::generateCall(scratchCodeBuffer, scratchCodeBuffer.used() + codeBase, dlopen_addr);
      
#if defined(arch_x86_64)
  } else {

      // Push caller
      emitMovImmToReg64(REGNUM_RAX, dlopen_addr, true, scratchCodeBuffer);
      emitSimpleInsn(0x50, scratchCodeBuffer); // push %rax

      // Push hole for result
      emitSimpleInsn(0x50, scratchCodeBuffer); // push %rax

      // Push padding and mode
      emitMovImmToReg64(REGNUM_EAX, DLOPEN_MODE, false, scratchCodeBuffer); // 32-bit mov: clears high dword
      emitSimpleInsn(0x50, scratchCodeBuffer); // push %rax

      // Push string addr
      emitMovImmToReg64(REGNUM_RAX, dyninstlib_str_addr, true, scratchCodeBuffer);
      emitSimpleInsn(0x50, scratchCodeBuffer); // push %rax
      
      // Set up the argument: the current stack pointer
      emitMovRegToReg64(REGNUM_RDI, REGNUM_RSP, true, scratchCodeBuffer);
      
      // The call (must be done through a register in order to reach)
      emitMovImmToReg64(REGNUM_RAX, dlopen_addr, true, scratchCodeBuffer);
      emitSimpleInsn(0xff, scratchCodeBuffer); // group 5
      emitSimpleInsn(0xd0, scratchCodeBuffer); // mod = 11, ext_op = 2 (call Ev), r/m = 0 (RAX)
  }
#endif

  // And the break point
  dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
  instruction::generateTrap(scratchCodeBuffer);

  startup_printf("(%d) dyninst lib string addr at 0x%x\n", getPid(), dyninstlib_str_addr);
  startup_printf("(%d) dyninst lib call addr at 0x%x\n", getPid(), dlopen_call_addr);
  startup_printf("(%d) break address is at %p\n", getPid(), (void *) dyninstlib_brk_addr);
  startup_printf("(%d) writing %d bytes\n", getPid(), scratchCodeBuffer.used());

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
  // We can tighten this up if we record how much we saved

  if (!readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true))
         fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);

  startup_printf("(%d) Writing from %p to %p\n", getPid(), (char *)scratchCodeBuffer.start_ptr(), (char *)codeBase);
  writeDataSpace((void *)(codeBase), scratchCodeBuffer.used(), scratchCodeBuffer.start_ptr());

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

  RegValue theBP = regs->PTRACE_REG_BP;
  // Under Linux, at the entry point to main, ebp is 0
  // the first thing main usually does is to push ebp and
  // move esp -> ebp, so we'll do that, too
  if( !theBP )
  {
	  theBP = regs->PTRACE_REG_SP;
	  startup_cerr << "BP at 0x0, creating fake stack frame with SP == "
				  << (void*)theBP << endl;
	  changeBP( getPid(), theBP );
  }

  assert( theBP );
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  readDataSpace((void*)(theBP-6*sizeof(int)),6*sizeof(int), savedStackFrame, true);

  lwp_to_use = NULL;

  if(process::IndependentLwpControl() && getRepresentativeLWP() ==NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();
  startup_printf("Changing PC to 0x%x\n", dlopen_call_addr);
  startup_printf("String at 0x%x\n", dyninstlib_str_addr);

  if (! lwp_to_use->changePC(dlopen_call_addr,NULL))
    {
      logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
      assert(0);
    }


#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif


  setBootstrapState(loadingRT_bs);
  return true;
}



bool process::loadDYNINSTlib_libc20() {
#if false && defined(PTRACEDEBUG)
  debug_ptrace = true;
#endif
  // we will write the following into a buffer and copy it into the
  // application process's address space
  // [....LIBRARY's NAME...|code for DLOPEN]

  startup_printf("**** LIBC20 dlopen for RT lib\n");

  // write to the application at codeOffset. This won't work if we
  // attach to a running process.

  Address codeBase = findFunctionToHijack(this);

  if(!codeBase)
  {
    startup_cerr << "Couldn't find a point to insert dlopen call" << endl;
    return false;
  }

  startup_printf("(%d) writing in dlopen call at addr %p\n", getPid(), (void *)codeBase);

  // Or should this be readText... it seems like they are identical
  // the remaining stuff is thanks to Marcelo's ideas - this is what 
  // he does in NT. The major change here is that we use AST's to 
  // generate code for dlopen.

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h

  if (!readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true))
         fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);

  codeGen scratchCodeBuffer(BYTES_TO_SAVE);
  Address dyninstlib_addr = 0;
  Address dlopencall_addr = 0;

  // Copy the dlopen string
  dyninstlib_addr = codeBase;

  scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

  pdvector<AstNode*> dlopenAstArgs( 2 );

  AstNode *dlopenAst;
  // deadList and deadListSize are defined in inst-sparc.C - naim
  extern Register deadList32[];
  extern int deadList32Size;
  registerSpace *dlopenRegSpace = new registerSpace(deadList32Size/sizeof(int), deadList32, 0, NULL);
  dlopenRegSpace->resetSpace();

  // we need to make a call to dlopen to open our runtime library

  startup_printf("(%d) using stack arguments to dlopen\n", getPid());
  dlopenAstArgs[0] = new AstNode(AstNode::Constant, (void*)dyninstlib_addr);
  // library name. We use a scratch value first. We will update this parameter
  // later, once we determine the offset to find the string - naim
  dlopenAstArgs[1] = new AstNode(AstNode::Constant, (void*)DLOPEN_MODE); // mode
  dlopenAst = new AstNode(DL_OPEN_FUNC_NAME,dlopenAstArgs);
  removeAst(dlopenAstArgs[0]);
  removeAst(dlopenAstArgs[1]);
  
  dlopencall_addr = codeBase + scratchCodeBuffer.used();
  dlopenAst->generateCode(this, dlopenRegSpace, scratchCodeBuffer,
			  true, true);
  removeAst(dlopenAst);

  dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();  
  instruction::generateTrap(scratchCodeBuffer);

  writeDataSpace((void *)codeBase, 
                 scratchCodeBuffer.used(),
                 scratchCodeBuffer.start_ptr());

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

  RegValue theBP = regs->PTRACE_REG_BP;
  // Under Linux, at the entry point to main, ebp is 0
  // the first thing main usually does is to push ebp and
  // move esp -> ebp, so we'll do that, too
  if( !theBP )
  {
	  theBP = regs->PTRACE_REG_SP;
	  startup_cerr << "BP at 0x0, creating fake stack frame with SP == "
				  << (void*)theBP << endl;
	  changeBP( getPid(), theBP );
  }

  assert( theBP );
  // this is pretty kludge. if the stack frame of _start is not the right
  // size, this would break.
  readDataSpace((void*)(theBP-6*sizeof(int)),6*sizeof(int), savedStackFrame, true);
  startup_cerr << "Changing PC to " << (void*)codeBase << endl;

  lwp_to_use = NULL;

  if(process::IndependentLwpControl() && getRepresentativeLWP() ==NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();

  if (! lwp_to_use->changePC(codeBase,NULL))
    {
      logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
      assert(0);
    }

#if false && defined(PTRACEDEBUG)
  debug_ptrace = false;
#endif


  setBootstrapState(loadingRT_bs);
  return true;
}


