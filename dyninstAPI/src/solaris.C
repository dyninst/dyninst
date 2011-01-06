/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: solaris.C,v 1.224 2008/06/19 22:13:43 jaw Exp $

#include "dyninstAPI/src/symtab.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include <string>
#include "dyninstAPI/src/debug.h"
#include "common/h/pathName.h" // concat_pathname_components()
#include "common/h/debugOstream.h"
#include "common/h/solarisKludges.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/editSharedLibrary.h" //ccw 11 mar 2005
#include "mapped_module.h"
#include "mapped_object.h"
#include "dynamiclinking.h"
#include "dyninstAPI/h/BPatch.h"

#include "symtabAPI/src/Object.h" //TODO: Remove this

#if defined (sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"
#else
#include "dyninstAPI/src/inst-x86.h"
#endif

#include "signalgenerator.h"

#include "function.h"

#include "instPoint.h"
#include "baseTramp.h"
#include "miniTramp.h"

#include <procfs.h>
#include <stropts.h>
#include <link.h>
#include <dlfcn.h>
#include <strings.h> //ccw 11 mar 2005

#include "dyn_lwp.h"

#include "ast.h"
#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

extern "C" {
extern long sysconf(int);
};

/*
   Define the indices of some registers to be used with pr_reg.
   These values are different on sparc and x86 platforms.
   RETVAL_REG: the registers that holds the return value of calls ($o0 on sparc,
               %eax on x86).
   PC_REG: program counter
   FP_REG: frame pointer (%i7 on sparc, %ebp on x86) 
*/
#ifdef sparc_sun_solaris2_4
#define PARAM1_REG (R_O0)
#define RETVAL_REG (R_O0)
#define PC_REG (R_PC)
#define FP_REG (R_O6)
#endif
#ifdef i386_unknown_solaris2_5
#define RETVAL_REG (EAX)
#define SP_REG (UESP)
#define PC_REG (EIP)
#define FP_REG (EBP)
#endif


// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  //This is the POSIX-compliant way of disconnecting from the terminal 
  setpgrp();
}

/*
 * The set operations (set_entry_syscalls and set_exit_syscalls) are defined
 * in sol_proc.C
 */


// Compatibility for /proc
bool process::get_entry_syscalls(sysset_t *entry)
{
    pstatus_t status;
    if (!get_status(&status)) return false;
    
    memcpy(entry, &(status.pr_sysentry), sizeof(sysset_t));    
    return true;
}

bool process::get_exit_syscalls(sysset_t *exit)
{
    pstatus_t status;
    if (!get_status(&status)) return false;

    memcpy(exit, &(status.pr_sysexit), sizeof(sysset_t));
    return true;
}    

bool process::dumpImage(std::string imageFileName) 
{
    int newFd;
    string command;

    string origFile = getAOut()->fileName();
   
    // first copy the entire image file
    command = "cp ";
    command += origFile;
    command += " ";
    command += imageFileName.c_str();
    system(command.c_str());

    // now open the copy
    newFd = open(imageFileName.c_str(), O_RDWR, 0);
    if (newFd < 0) {
	// log error
	return false;
    }

    Elf_X elf(newFd, ELF_C_READ);
    if (!elf.isValid()) return false;

    Elf_X_Shdr shstrscn = elf.get_shdr( elf.e_shstrndx() );
    Elf_X_Data shstrdata = shstrscn.get_data();
    const char* shnames = (const char *) shstrdata.get_string();

    Address baseAddr = 0;
    int length = 0;
    int offset = 0;
    for (int i = 0; i < elf.e_shnum(); ++i) {
	Elf_X_Shdr shdr = elf.get_shdr(i);
	const char *name = (const char *) &shnames[shdr.sh_name()];

	if (!P_strcmp(name, ".text")) {
	    offset = shdr.sh_offset();
	    length = shdr.sh_size();
	    baseAddr = shdr.sh_addr();
	    break;
	}
    }

    char *tempCode = new char[length];
    bool ret = readTextSpace((void *) baseAddr, length, tempCode);
    if (!ret) {
	// log error

	delete[] tempCode;
	elf.end();
	P_close(newFd);

	return false;
    }

    lseek(newFd, offset, SEEK_SET);
    write(newFd, tempCode, length);

    // Cleanup
    delete[] tempCode;
    elf.end();
    P_close(newFd);

    return true;
}

/* Auxiliary function */
bool checkAllThreadsForBreakpoint(process *proc, Address break_addr)
{
   pdvector<Frame> activeFrames;
   if (!proc->getAllActiveFrames(activeFrames)) {
      fprintf(stderr, "%s[%d]:  getAllActiveFrames failed\n", FILE__, __LINE__);
      return false;
   }
   for(unsigned frame_iter = 0; frame_iter < activeFrames.size(); frame_iter++)
   {
      if (activeFrames[frame_iter].getPC() == break_addr) {
         return true;
      }
   }
   
   return false;
}

bool process::trapAtEntryPointOfMain(dyn_lwp *, Address)
{
    if (main_brk_addr == 0x0) return false;
    return checkAllThreadsForBreakpoint(this, main_brk_addr);
}

bool process::handleTrapAtEntryPointOfMain(dyn_lwp *)
{
    assert(main_brk_addr);
    
  // restore original instruction 
    writeDataSpace((void *)main_brk_addr, sizeof(savedCodeBuffer), 
                   (char *)savedCodeBuffer);
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

    codeGen gen(instruction::size());
    insnCodeGen::generateTrap(gen);

    // save original instruction first
    readDataSpace((void *)addr, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    writeDataSpace((void *)addr, gen.used(), gen.start_ptr());

    main_brk_addr = addr;
    
    return true;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); return false; }
bool process::instrumentLibcStartMain() { assert(0); return false; }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); return false; }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); return false; }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); return 0; }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); return 0; }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); return 0; }
bool process::isMmapSysCall(int) { assert(0); return false; }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); return 0;}
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); return 0;}

bool AddressSpace::getDyninstRTLibName() {
   if (dyninstRT_name.length() == 0) {
      // Get env variable
      if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
         dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
      }
      else {
         std::string msg = std::string("Environment variable ") +
                        std::string("DYNINSTAPI_RT_LIB") +
                        std::string(" has not been defined.");
         showErrorCallback(101, msg);
         return false;
      }
   }
   
    // Automatically choose the .a version or the .so version
    const char *name = dyninstRT_name.c_str();

    const char *split = P_strrchr(name, '/');
    if ( !split ) split = name;
    split = P_strchr(split, '.');
    if ( !split || P_strlen(split) <= 1 ) {
        // We should probably print some error here.
        // Then, of course, the user will find out soon enough.
        startup_printf("Invalid Dyninst RT lib name: %s\n", 
                dyninstRT_name.c_str());
        return false;
    }

    const char *suffix = split;
    if( getAOut()->isStaticExec() ) {
        suffix = ".a";
    }else{
        if( P_strncmp(suffix, ".a", 2) == 0 ) {
            // This will be incorrect if the RT library's version changes
            suffix = ".so";
        }
    }

    dyninstRT_name = std::string(name, split - name) +
                     std::string(suffix);

    startup_printf("Dyninst RT Library name set to '%s'\n",
            dyninstRT_name.c_str());

   // Check to see if the library given exists.
   if (access(dyninstRT_name.c_str(), R_OK)) {
      std::string msg = std::string("Runtime library ") + dyninstRT_name +
                     std::string(" does not exist or cannot be accessed!");
      showErrorCallback(101, msg);
      return false;
   }
   return true;
}

bool process::loadDYNINSTlib() {
    // we will write the following into a buffer and copy it into the
    // application process's address space
    // [....LIBRARY's NAME...|code for DLOPEN]
    
    // write to the application at codeOffset. This won't work if we
    // attach to a running process.
    //Address codeBase = this->getImage()->codeOffset();
    // ...let's try "_start" instead
    int_function *_startfn;
    
    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("_start", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"_start() unfound");
        return false;
    }

    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one _start! using the first" << endl;
    }
    _startfn = funcs[0];

    Address codeBase = _startfn->getAddress();
    assert(codeBase);
    
    // Or should this be readText... it seems like they are identical
    // the remaining stuff is thanks to Marcelo's ideas - this is what 
    // he does in NT. The major change here is that we use AST's to 
    // generate code for dlopen.
    
    // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
    readDataSpace((void *)codeBase, sizeof(savedCodeBuffer), savedCodeBuffer, true);
    
    codeGen scratchCodeBuffer(BYTES_TO_SAVE);
    scratchCodeBuffer.setAddrSpace(this);
    scratchCodeBuffer.setAddr(codeBase);
    scratchCodeBuffer.setRegisterSpace(registerSpace::savedRegSpace(this));

    // First we write in the dyninst lib string. Vewy simple.
    Address dyninstlib_addr = codeBase;
    
    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

    // Were we're calling into
    Address dlopencall_addr = codeBase + scratchCodeBuffer.used();

    /*
      fprintf(stderr, "dlopen call addr at 0x%x, for codeBase of 0x%x\n",
            dlopencall_addr, codeBase);
    */


    pdvector<AstNodePtr> dlopenAstArgs(2);
    AstNodePtr dlopenAst;
    
    // We call directly into ld.so.1. This used to be handled in 
    // process::findInternalSymbols, which made it very difficult
    // to figure out what was going on.
    Address dlopen_func_addr = dyn->get_dlopen_addr();
    assert(dlopen_func_addr);

    //fprintf(stderr, "We want to call 0x%x\n", dlopen_func_addr);
    // See if we can get a function for it.

    dlopenAstArgs[0] = AstNode::operandNode(AstNode::Constant, (void *)(dyninstlib_addr));
    dlopenAstArgs[1] = AstNode::operandNode(AstNode::Constant, (void*)DLOPEN_MODE);
    dlopenAst = AstNode::funcCallNode(dlopen_func_addr, dlopenAstArgs);

    dlopenAst->generateCode(scratchCodeBuffer,
                            true);

    // Slap in a breakpoint
    dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
    insnCodeGen::generateTrap(scratchCodeBuffer);
    
    writeDataSpace((void *)codeBase, scratchCodeBuffer.used(), 
                   scratchCodeBuffer.start_ptr());
    
    //fprintf(stderr, "Breakpoint at 0x%x\n", dyninstlib_brk_addr);
    
    // save registers
    savedRegs = new dyn_saved_regs;
    bool status = getRepresentativeLWP()->getRegisters(savedRegs);
    assert(status == true);
    
    if (!getRepresentativeLWP()->changePC(dlopencall_addr, NULL)) {
        logLine("WARNING: changePC failed in loadDYNINSTlib\n");
        assert(0);
    }
    setBootstrapState(loadingRT_bs);
    return true;
}

bool process::trapDueToDyninstLib(dyn_lwp *)
{
  if (dyninstlib_brk_addr == 0) return(false);
  return checkAllThreadsForBreakpoint(this, dyninstlib_brk_addr);
}

bool process::loadDYNINSTlibCleanup(dyn_lwp *)
{
  // rewrite original instructions in the text segment we use for 
  // the inferiorRPC - naim
  unsigned count = sizeof(savedCodeBuffer);
  //Address codeBase = getImage()->codeOffset();

  int_function *_startfn;

    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("_start", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"_start() unfound");
        return false;
    }

    if( funcs.size() > 1 ) {
        cerr << __FILE__ << __LINE__ 
             << ": found more than one main! using the first" << endl;
    }
    _startfn = funcs[0];

    Address codeBase = _startfn->getAddress();
    assert(codeBase);
    if (!writeDataSpace((void *)codeBase, count, (char *)savedCodeBuffer))
        return false;

    // restore registers
    if (!getRepresentativeLWP()->restoreRegisters(*savedRegs))
     return false;
    delete savedRegs;
    savedRegs = NULL;
    return true;
}

bool SignalGeneratorCommon::getExecFileDescriptor(std::string filename,
                                    int /*pid*/,
                                    bool /*whocares*/,
                                    int &,
                                    fileDescriptor &desc)
{
    desc = fileDescriptor(filename.c_str(), 0, 0, false);
    return true;
}

#if defined(cap_dynamic_heap)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
                                        inferiorHeapType /* type */)
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}
#endif

bool process::dumpCore_(const std::string coreName) 
{
  char command[100];

  sprintf(command, "gcore %d 2> /dev/null; mv core.%d %s", getPid(), getPid(), 
          coreName.c_str());

  detach(false);
  system(command);
  attach();

  return false;
}

Frame Frame::getCallerFrame()
{
  Address newPC=0;
  Address newFP=0;
  Address newSP=0;

  //fprintf(stderr, "Frame::getCallerFrame for %p\n", this);

  if (uppermost_) {
    codeRange *range = getRange();
    int_function *func = range->is_function();
    if (func) {
	struct dyn_saved_regs regs;
	bool status;
	if (lwp_)
	  status = lwp_->getRegisters(&regs);
	else
	  status = getProc()->getRepresentativeLWP()->getRegisters(&regs);

	assert(status == true);
      if (func->hasNoStackFrame()) { // formerly "isLeafFunc()"
	newPC = regs.theIntRegs[R_O7] + 8;
	newFP = fp_; // frame pointer unchanged
      } else {
	newPC = regs.theIntRegs[R_O7] + 8;
        if (!getProc()->readDataSpace((caddr_t)(fp_ + 56), sizeof(int), (caddr_t)&newFP, true))
           return Frame();
      }
	return Frame(newPC, newFP, newSP, 0, this);
    }
  }
  //
  // For the sparc, register %i7 is the return address - 8 and the fp is
  // register %i6. These registers can be located in %fp+14*5 and
  // %fp+14*4 respectively, but to avoid two calls to readDataSpace,
  // we bring both together (i.e. 8 bytes of memory starting at %fp+14*4
  // or %fp+56).
  // These values are copied to the stack when the application is paused,
  // so we are assuming that the application is paused at this point
  
  struct {
    Address fp;
    Address rtn;
  } addrs;
  
  if (getProc()->readDataSpace((caddr_t)(fp_ + 56), 2*sizeof(int),
			       (caddr_t)&addrs, true))
    {

      newFP = addrs.fp;
      newPC = addrs.rtn + 8;

      if (isSignalFrame()) {
         // get the value of the saved PC: this value is stored in the
         // address specified by the value in register i2 + 44. Register i2
         // must contain the address of some struct that contains, among
         // other things, the saved PC value.
         u_int reg_i2;
         if (getProc()->readDataSpace((caddr_t)(fp_+40), sizeof(u_int),
                              (caddr_t)&reg_i2,true)) {
            Address saved_pc;
            if (getProc()->readDataSpace((caddr_t) (reg_i2+44), sizeof(int),
                                 (caddr_t) &saved_pc,true)) {
               
               int_function *func = getProc()->findFuncByAddr(saved_pc);
               
               newPC = saved_pc;
               if (func && func->hasNoStackFrame())
                  newFP = fp_;
            }
         }
	 else {
	   return Frame();
	 }
      }
      

      if(getProc()->multithread_capable()) {
         // MT thread adds another copy of the start function
         // to the top of the stack... this breaks instrumentation
         // since we think we're at a function entry.
         if (newFP == 0) newPC = 0;
      }
      Frame ret = Frame(newPC, newFP, 0, 0, this);

      codeRange *range = getRange();
      // Find our way out of the minitramp, and up to the calling function
      if (range->is_minitramp()) {
         instPoint *p = getPoint();
         if (p->getPointType() != functionEntry &&
             !p->func()->hasNoStackFrame()) {
            if (!getProc()->readDataSpace((caddr_t)(newFP + 60), sizeof(int), (caddr_t)&newPC, true))
               return Frame();
            if (!getProc()->readDataSpace((caddr_t)(newFP + 56), sizeof(int), (caddr_t)&newFP, true))
               return Frame();
            ret = Frame(newPC, newFP, 0, 0, this);
         }
      }

      // If we're in a base tramp, skip this frame (return getCallerFrame)
      // as we only return minitramps
      if (range->is_multitramp()) {
          // If we're inside instrumentation only....
          multiTramp *multi = range->is_multitramp();
          baseTrampInstance *bti = multi->getBaseTrampInstanceByAddr(getPC());
          if (bti &&
              bti->isInInstru(getPC()))
              return ret.getCallerFrame();
      }
      return ret;
    }
   return Frame(); // zero frame
}

bool Frame::setPC(Address newpc) {
  fprintf(stderr, "Implement me! Changing frame PC from %x to %x\n",
	  pc_, newpc);
  return false;
}


void print_read_error_info(const SymtabAPI::relocationEntry entry, 
                           int_function *&target_pdf, Address base_addr) {

   sprintf(errorLine, "  entry      : target_addr 0x%lx\n",
           entry.target_addr());
   logLine(errorLine);
   sprintf(errorLine, "               rel_addr 0x%lx\n", entry.rel_addr());
   logLine(errorLine);
   sprintf(errorLine, "               name %s\n", (entry.name()).c_str());
   logLine(errorLine);
   
   if (target_pdf) {
      sprintf(errorLine, "  target_pdf : symTabName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);    
      sprintf(errorLine , "              prettyName %s\n",
              (target_pdf->symTabName()).c_str());
      logLine(errorLine);
      sprintf(errorLine , "              addr 0x%lx\n",
              target_pdf->getAddress());
      logLine(errorLine);
   }

   sprintf(errorLine, "  base_addr  0x%lx\n", base_addr);
   logLine(errorLine);
}

// hasBeenBound: returns true if the runtime linker has bound the
// function symbol corresponding to the relocation entry in at the address
// specified by entry and base_addr.  If it has been bound, then the callee 
// function is returned in "target_pdf", else it returns false.
bool process::hasBeenBound(const SymtabAPI::relocationEntry &entry, 
			   int_function *&target_pdf, Address base_addr) {

// TODO: the x86 and sparc versions should really go in seperate files 
#if defined(i386_unknown_solaris2_5)

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
        sprintf(errorLine, "read error in process::hasBeenBound "
		"addr 0x%lx, pid=%d\n (readDataSpace returns 0)",
		got_entry, pid);
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

#else
    // if the relocationEntry has not been bound yet, then the second instr 
    // in this PLT entry branches to the fist PLT entry.  If it has been   
    // bound, then second two instructions of the PLT entry have been changed 
    // by the runtime linker to jump to the address of the function.  
    // Here is an example:   
    //     before binding			after binding
    //	   --------------			-------------
    //     sethi  %hi(0x15000), %g1		sethi  %hi(0x15000), %g1
    //     b,a  <_PROCEDURE_LINKAGE_TABLE_>     sethi  %hi(0xef5eb000), %g1
    //	   nop					jmp  %g1 + 0xbc ! 0xef5eb0bc

    unsigned int insnBuf;

    Address next_insn_addr = entry.target_addr() + base_addr + instruction::size(); 
    if( !(readDataSpace((caddr_t)next_insn_addr, instruction::size(), 
                        (char *)&insnBuf, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace next_insn_addr returned 0)\n",
		next_insn_addr);
	logLine(errorLine);
	print_read_error_info(entry, target_pdf, base_addr);
    }
    instruction next_insn(insnBuf);
    // if this is a b,a instruction, then the function has not been bound
    if(((*next_insn).branch.op == FMT2op)  && ((*next_insn).branch.op2 == BICCop2) 
       && ((*next_insn).branch.anneal == 1) && ((*next_insn).branch.cond == BAcond)) {
	return false;
    } 

    // if this is a sethi instruction, then it has been bound...get target_addr
    Address third_addr = entry.target_addr() + base_addr + 8; 
    if( !(readDataSpace((caddr_t)third_addr, instruction::size(),
		       (char *)&insnBuf, true)) ) {
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%lx"
                " (readDataSpace third_addr returned 0)\n",
		third_addr);
	logLine(errorLine);
	print_read_error_info(entry,target_pdf, base_addr);
    }

    instruction third_insn(insnBuf);

    // get address of bound function, and return the corr. int_function
    if(((*next_insn).sethi.op == FMT2op) && ((*next_insn).sethi.op2 == SETHIop2)
	&& ((*third_insn).rest.op == RESTop) && ((*third_insn).rest.i == 1)
	&& ((*third_insn).rest.op3 == JMPLop3)) {
        
	Address new_target = ((*next_insn).sethi.imm22 << 10) & 0xfffffc00; 
	new_target |= (*third_insn).resti.simm13;

        target_pdf = findFuncByAddr(new_target);
	if(!target_pdf){
            return false;
	}
	return true;
    }
    // this is a messed up entry
    return false;
#endif

}



// findCallee: finds the function called by the instruction corresponding
// to the instPoint "instr". If the function call has been bound to an
// address, then the callee function is returned in "target" and the 
// instPoint "callee" data member is set to pt to callee's int_function.  
// If the function has not yet been bound, then "target" is set to the 
// int_function associated with the name of the target function (this is 
// obtained by the PLT and relocation entries in the image), and the instPoint
// callee is not set.  If the callee function cannot be found, (ex. function
// pointers, or other indirect calls), it returns false.
// Returns false on error (ex. process doesn't contain this instPoint).
//
int_function *instPoint::findCallee() {

   if(callee_) {
       return callee_;
   }

       if (ipType_ != callSite) {
        return NULL;
    }

    if (isDynamic()) { 
        return NULL;
    }

    // Check if we parsed an intra-module static call
    assert(img_p_);
    image_func *icallee = img_p_->getCallee();
    if (icallee) {
        // Now we have to look up our specialized version
        // Can't do module lookup because of DEFAULT_MODULE...
        const pdvector<int_function *> *possibles = func()->obj()->findFuncVectorByMangled(icallee->symTabName().c_str());
        if (!possibles) {
            return NULL;
        }
        for (unsigned i = 0; i < possibles->size(); i++) {
          if ((*possibles)[i]->ifunc() == icallee) {
                callee_ = (*possibles)[i];
                return callee_;
            }
        }
        // No match... very odd
        assert(0);
        return NULL;
    }

    
    // get the target address of this function
    Address target_addr = img_p_->callTarget();
    //    Address insn_addr = instr.pointAddr(); 
    
    if(!target_addr) {  
        // this is either not a call instruction or an indirect call instr
        // that we can't get the target address
        return NULL;
    }
    
    
    // else, get the relocation information for this image
    SymtabAPI::Symtab *obj = func()->obj()->parse_img()->getObject();
    vector<SymtabAPI::relocationEntry> fbtvector;
    if(!obj->getFuncBindingTable(fbtvector))
    	return false;	// target cannot be found...it is an indirect call.
    pdvector<SymtabAPI::relocationEntry> *fbt = new pdvector<SymtabAPI::relocationEntry>;
    for(unsigned index=0;index<fbtvector.size();index++)
    	fbt->push_back(fbtvector[index]);
    
    // find the target address in the list of relocationEntries
    Address base_addr = func()->obj()->codeBase();
    for(u_int i=0; i < fbt->size(); i++) {
        if((*fbt)[i].target_addr() == target_addr) {
            // check to see if this function has been bound yet...if the
            // PLT entry for this function has been modified by the runtime
            // linker
            int_function *target_pdf = 0;
            if(proc()->hasBeenBound((*fbt)[i], target_pdf, base_addr)) {
                callee_ = target_pdf;
                return callee_;
            } 
            else {
                // just try to find a function with the same name as entry 
                pdvector<int_function *> pdfv;
                bool found = proc()->findFuncsByMangled((*fbt)[i].name().c_str(), pdfv);
                if(found) {
                    assert(pdfv.size());
                    if(pdfv.size() > 1)
                        cerr << __FILE__ << ":" << __LINE__ 
                             << ": WARNING:  findAllFuncsByName found " 
                             << pdfv.size() << " references to function " 
                             << (*fbt)[i].name() << ".  Using the first.\n";
                    callee_ = pdfv[0];
                    return callee_;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

// vim:ts=5:

/**
 * Searches for function in order, with preference given first 
 * to libpthread, then to libc, then to the process.
 **/
static void findThreadFuncs(process *p, std::string func, 
                            pdvector<int_function *> &result)
{
   bool found = false;
   mapped_module *lpthread = p->findModule("libthread*", true);
   if (lpthread)
      found = lpthread->findFuncVectorByPretty(func, result);
   if (found) {
      return;
   }
   /*
    * Do not look in libc... there are matches, but they're singlethread versions

    mapped_module *lc = p->findModule("libc.so*", true);
    if (lc)
    found = lc->findFuncVectorByPretty(func, result);
    if (found) {
    fprintf(stderr, "found in libc.so\n");
    return;
    }
   */

   p->findFuncsByPretty(func, result);
}

bool process::initMT()
{
    unsigned i;
    bool res;
    
#if !defined(cap_threads)
    return true;
#endif

    /**
     * Instrument thread_create with calls to DYNINST_dummy_create
     **/
    pdvector<int_function *> thread_init_funcs;
    findThreadFuncs(this, "init_func", thread_init_funcs);
    if (thread_init_funcs.size() < 1) {
        //findThreadFuncs(this, "_lwp_start", thread_init_funcs);
        findThreadFuncs(this, "_thr_setup", thread_init_funcs);
        if (thread_init_funcs.size() < 1) {
          findThreadFuncs(this, "_thread_start", thread_init_funcs);
          if (thread_init_funcs.size() < 1) {
            fprintf(stderr, "%s[%d]: no copies of thread start function, expected 1\n",
                    FILE__, __LINE__);

            return false;
          }
        }
    }

    //Find DYNINST_dummy_create
   int_function *dummy_create = findOnlyOneFunction("DYNINST_dummy_create");
   if (!dummy_create) {
       fprintf(stderr, "[%s:%d] - Couldn't find DYNINST_dummy_create",
               __FILE__, __LINE__);
       return false;
   }
   //Instrument
   for (i=0; i<thread_init_funcs.size(); i++)
   {
      pdvector<AstNodePtr> args;
      AstNodePtr call_dummy_create = AstNode::funcCallNode(dummy_create, args);
      const pdvector<instPoint *> &ips = thread_init_funcs[i]->funcEntries();
      for (unsigned j=0; j<ips.size(); j++)
      {
         miniTramp *mt;
         mt = ips[j]->instrument(call_dummy_create, callPreInsn, orderFirstAtPoint, false, 
                                 false);
         if (!mt)
         {
            fprintf(stderr, "[%s:%d] - Couldn't instrument thread_create\n",
                    __FILE__, __LINE__);
         }
         //TODO: Save the mt objects for detach
      }
   }
   
   /**
    * Have dyn_pthread_self call the actual pthread_self
    **/
   //Find dyn_pthread_self
   pdvector<int_variable *> ptself_syms;
   res = findVarsByAll("DYNINST_pthread_self", ptself_syms);
   if (!res) {
       fprintf(stderr, "[%s:%d] - Couldn't find any dyn_pthread_self, expected 1\n",
               __FILE__, __LINE__);
   }
   assert(ptself_syms.size() == 1);
   Address dyn_pthread_self = ptself_syms[0]->getAddress();


   //Find pthread_self
   pdvector<int_function *> pthread_self_funcs;
   findThreadFuncs(this, "thr_self", pthread_self_funcs);   
   if (pthread_self_funcs.size() != 1) {
       fprintf(stderr, "[%s:%d] - Found %d pthread_self functions, expected 1\n",
               __FILE__, __LINE__, pthread_self_funcs.size());
       for (unsigned j=0; j<pthread_self_funcs.size(); j++) {
           int_function *ps = pthread_self_funcs[j];
           fprintf(stderr, "[%s:%u] - %s in module %s at %lx\n", __FILE__, __LINE__,
                   ps->prettyName().c_str(), ps->mod()->fullName().c_str(), 
                   ps->getAddress());
       }
       return false;
   }   
   //Replace
   res = writeFunctionPtr(this, dyn_pthread_self, pthread_self_funcs[0]);
   if (!res) {
       fprintf(stderr, "[%s:%d] - Couldn't update dyn_pthread_self\n",
               __FILE__, __LINE__);
       return false;
   }
   return true;
}

#include <sched.h>
void dyninst_yield()
{
   sched_yield();
}

bool SignalHandler::handleProcessExitPlat(EventRecord & /*ev*/, bool &) 
{
    return true;
}

bool process::hasPassedMain() 
{
   return true;
}

bool BinaryEdit::getResolvedLibraryPath(const std::string &filename, std::vector<std::string> &paths) {
    // No actual library name resolution for now
    paths.push_back(filename);
    return true;
}

// Temporary remote debugger interface.
// I assume these will be removed when procControlAPI is complete.
bool OS_isConnected(void)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_connect(BPatch_remoteHost &/*remote*/)
{
    return true;  // We're always connected to the child on this platform.
}

bool OS_getPidList(BPatch_remoteHost &/*remote*/,
                   BPatch_Vector<unsigned int> &/*tlist*/)
{
    return false;  // Not implemented.
}

bool OS_getPidInfo(BPatch_remoteHost &/*remote*/,
                   unsigned int /*pid*/, std::string &/*pidStr*/)
{
    return false;  // Not implemented.
}

bool OS_disconnect(BPatch_remoteHost &/*remote*/)
{
    return true;
}
