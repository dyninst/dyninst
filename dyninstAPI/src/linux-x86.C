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

// $Id: linux-x86.C,v 1.142 2008/08/01 17:55:12 roundy Exp $

#include <fstream>
#include <string>

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
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/instPoint.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "common/h/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/dyn_thread.h"

#include "dyninstAPI/src/mapped_object.h" 
#include "dyninstAPI/src/signalgenerator.h" 

#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/linux.h"

#include "dyninstAPI/src/registerSpace.h"

#include <sstream>

#if defined (cap_save_the_world)
#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
#endif
#include "dyninstAPI/src/debuggerinterface.h"

#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/dynamiclinking.h"
#include "dyninstAPI/src/binaryEdit.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Instruction.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

// Note: this is an internal libc flag -- it is only used
// when libc and ld.so don't have symbols
#ifndef __RTLD_DLOPEN
#define __RTLD_DLOPEN 0x80000000
#endif

const char *DL_OPEN_FUNC_USER = NULL;
const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_LIBC_FUNC_EXPORTED[] = "__libc_dlopen_mode";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";

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

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)

/* ********************************************************************** */

bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool includeFP) 
{
   // Cycle through all registers, reading each from the
   // process user space with ptrace(PTRACE_PEEKUSER ...
   int error;
   bool errorFlag = false;
   assert(get_lwp_id() != 0);
   int ptrace_errno = 0;
   error = DBI_ptrace(PTRACE_GETREGS, get_lwp_id(), 0, (long)&(regs->gprs), 
         &ptrace_errno, proc_->getAddressWidth(),  
         __FILE__, __LINE__ );
   if( error ) {
      perror("dyn_lwp::getRegisters PTRACE_GETREGS" );
      errorFlag = true;
      return false;
   }

   if (includeFP)
   {
      error = DBI_ptrace(PTRACE_GETFPREGS, get_lwp_id(), 0,
            (long)&(regs->fprs), &ptrace_errno,
            proc_->getAddressWidth(),  __FILE__, __LINE__);

      if( error ) {
         perror("dyn_lwp::getRegisters PTRACE_GETFPREGS" );
         return false;
      }
   }
   return true;
}

void dyn_lwp::dumpRegisters()
{
   dyn_saved_regs regs;
   if (!getRegisters(&regs)) {
      fprintf(stderr, "%s[%d]:  registers unavailable\n", FILE__, __LINE__);
      return;
   }

#if defined(arch_x86)
   fprintf(stderr, "eip:   %lx\n", regs.gprs.eip);
   fprintf(stderr, "eax:   %lx\n", regs.gprs.eax);
   fprintf(stderr, "ebx:   %lx\n", regs.gprs.ebx);
   fprintf(stderr, "ecx:   %lx\n", regs.gprs.ecx);
   fprintf(stderr, "esp:   %lx\n", regs.gprs.esp);
   fprintf(stderr, "xcs:   %lx\n", regs.gprs.xcs);
#endif
#if defined(arch_x86_64)
   fprintf(stderr, "eip:   %lx\n", regs.gprs.rip);
   fprintf(stderr, "eax:   %lx\n", regs.gprs.rax);
   fprintf(stderr, "ebx:   %lx\n", regs.gprs.rbx);
   fprintf(stderr, "ecx:   %lx\n", regs.gprs.rcx);
   fprintf(stderr, "esp:   %lx\n", regs.gprs.rsp);
   fprintf(stderr, "xcs:   %lx\n", regs.gprs.cs);
#endif
   //  plenty more register if we want to print em....
}

bool dyn_lwp::changePC(Address loc,
      struct dyn_saved_regs */*ignored registers*/)
{

   Address regaddr = P_offsetof(struct user_regs_struct, PTRACE_REG_IP);
   assert(get_lwp_id() != 0);
   int ptrace_errno = 0;

   // Check to see if the incoming address is valid...
   #if defined(arch_x86_64)
   if ((proc_->getAddressWidth() == 4) &&
       (sizeof(Address) == 8)) {
     assert(!(loc & 0xffffffff00000000));
   }
   #endif
   if (0 != DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, loc, 
            &ptrace_errno, proc_->getAddressWidth(),  
            __FILE__, __LINE__ )) {
      fprintf(stderr, "dyn_lwp::changePC - PTRACE_POKEUSER failure for %u",
            get_lwp_id());
      return false;
   }
   
   return true;
}

bool dyn_lwp::clearOPC() 
{
   Address regaddr = P_offsetof(struct user_regs_struct, PTRACE_REG_ORIG_AX);
   assert(get_lwp_id() != 0);
   int ptrace_errno = 0;
   if (0 != DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, -1UL, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__)) {
      perror( "dyn_lwp::changePC - PTRACE_POKEUSER" );
      return false;
   }
   return true;
}

#define REG_STR(x)	REG_STR_(x)
#define REG_STR_(x)	#x
void printRegs( void *save ) 
{
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


bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool includeFP) {
   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

   bool retVal = true;
   int ptrace_errno = 0;


   assert(get_lwp_id() != 0);
   if( DBI_ptrace( PTRACE_SETREGS, get_lwp_id(), 0,(long)&(regs.gprs), 
            &ptrace_errno, proc_->getAddressWidth(),  
            __FILE__, __LINE__ ) )
   {
      perror("dyn_lwp::restoreRegisters PTRACE_SETREGS" );
      retVal = false;
   }

   if (includeFP) {
      if( DBI_ptrace( PTRACE_SETFPREGS, get_lwp_id(), 0, (long)&(regs.fprs), 
               &ptrace_errno, proc_->getAddressWidth(),  
               __FILE__, __LINE__))
      {
         perror("dyn_lwp::restoreRegisters PTRACE_SETFPREGS" );
         retVal = false;
      }
   }
   return retVal;
}

// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
   if(status() == running) {
      fprintf(stderr, "%s[%d][%s]:  FIXME\n", __FILE__, __LINE__, 
            getThreadStr(getExecThreadID()));
      cerr << "    performance problem in call to dyn_lwp::getActiveFrame\n"
         << "       successive pauses and continues with ptrace calls\n";
   }

   Address pc, fp, sp;

   int ptrace_errno = 0;
   fp = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(), P_offsetof(struct user_regs_struct, PTRACE_REG_BP), 0, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   pc = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(), P_offsetof(struct user_regs_struct, PTRACE_REG_IP), 0, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   sp = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(), P_offsetof(struct user_regs_struct, PTRACE_REG_SP), 0, &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   dbi_printf("%s[%d]:  GET ACTIVE FRAME (pc = %p, sp = %p, fp = %p\n", 
         FILE__, __LINE__, pc, sp, fp);

   return Frame(pc, fp, sp, proc_->getPid(), proc_, NULL, this, true);
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
   startup_printf("%s[%d]: Saving %d bytes from entry of main of %d...\n", 
         FILE__, __LINE__, sizeof(savedCodeBuffer), getPid());

   // save original instruction first
   if (!readDataSpace((void *)addr, sizeof(savedCodeBuffer), savedCodeBuffer, true)) {
      fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
      fprintf(stderr, "%s[%d][%s]:  failing insertTrapAtEntryPointOfMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      fprintf(stderr, "Failed to read at address 0x%lx\n", addr);
      return false;
   }

   codeGen gen(1);
   insnCodeGen::generateTrap(gen);

   if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr())) {
      fprintf(stderr, "%s[%d][%s]:  failing insertTrapAtEntryPointOfMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      return false;
   }

   main_brk_addr = addr;

   signal_printf("Added trap at entry of main, address 0x%x\n", main_brk_addr);
   return true;
}

#if defined(arch_x86_64)

bool process::getSysCallParameters(dyn_saved_regs *regs, long *params, int numparams) 
{
   if (getAddressWidth() == 4) { // 32 bit mutatee
   } else { // 64 bit mutatee, have to use ifdef, otherwise it won't
      // compile on a 32 bit machine
      if (numparams > 0) {
         params[0] = regs->gprs.rdi;
      }
      if (numparams > 1) {
         params[1] = regs->gprs.rsi;
      }
      if (numparams > 2) {
         params[2] = regs->gprs.rdx;
      }
      if (numparams > 3) {
         params[3] = regs->gprs.r8;
      }
      if (numparams > 4) {
         params[4] = regs->gprs.r9;
      }
      if (numparams > 5) {
         params[5] = regs->gprs.r10;
      }
      for (int i=6; i < numparams; i++) {
         if (!readDataSpace((void*)regs->gprs.rsp, getAddressWidth(), 
                  (void*)(params + i * getAddressWidth()), true)) {
            return false;
         }
      }
   }
   return true;
}

int process::getSysCallNumber(dyn_saved_regs *regs) {
   return regs->gprs.orig_rax;
}

long process::getSysCallReturnValue(dyn_saved_regs *regs) {
   return regs->gprs.rax;
}

Address process::getSysCallProgramCounter(dyn_saved_regs *regs) {
   return regs->gprs.rip;
}

bool process::isMmapSysCall(int callnum) {
   if (getAddressWidth() == 4) {
      startup_printf("CALLNUM=%d\n",callnum);
   }
   return callnum == SYS_mmap;
}

Offset process::getMmapLength(int, dyn_saved_regs *regs) {
   return (Offset) regs->gprs.rsi;
}

Address process::getLibcStartMainParam(dyn_lwp *trappingLWP) {
   Address mainaddr = 0;
   dyn_saved_regs regs;
   trappingLWP->getRegisters(&regs);
   if (getAddressWidth() == 4) { // 32 bit mutatee
      if (!readDataSpace((void*)(regs.gprs.rsp + getAddressWidth()),
               getAddressWidth(), (void*)&mainaddr,true)) {
         fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
      }
   } else { // 64 bit mutatee
      mainaddr = regs.gprs.rdi;
   }
   return mainaddr;
}
// 64 bit architecture
#else 
// 32 bit architecture
bool process::getSysCallParameters(dyn_saved_regs *regs, 
      long *params, int numparams) {
   if (numparams > 0) {
      params[0] = regs->gprs.ebx;
   }
   if (numparams > 1) {
      params[1] = regs->gprs.ecx;
   }
   if (numparams > 2) {
      params[2] = regs->gprs.edx;
   }
   if (numparams > 3) {
      params[3] = regs->gprs.esi;
   }
   if (numparams > 4) {
      params[4] = regs->gprs.edi;
   }
   if (numparams > 5) {
      params[5] = regs->gprs.ebp;
   }
   for (int i=6; i < numparams; i++) {
      if (!readDataSpace((void*)regs->gprs.esp, getAddressWidth(), 
               (void*)(params + i * getAddressWidth()), true)) {
         return false;
      }
   }
   return true;
}

int process::getSysCallNumber(dyn_saved_regs *regs) 
{
   return regs->gprs.orig_eax;
}

long process::getSysCallReturnValue(dyn_saved_regs *regs) 
{
   return regs->gprs.eax;
}

Address process::getSysCallProgramCounter(dyn_saved_regs *regs) 
{
   return regs->gprs.eip;
}

bool process::isMmapSysCall(int callnum) {
   return (callnum == SYS_mmap || callnum == SYS_mmap2);
}

Offset process::getMmapLength(int callnum, dyn_saved_regs *regs) 
{
   if (callnum == SYS_mmap) {
      Offset length;
      readDataSpace((void*)(regs->gprs.ebx + getAddressWidth()),
            getAddressWidth(), (void*)&length, true);
      return length;
   }
   else {
      return (Offset) regs->gprs.ecx;
   }
}

Address process::getLibcStartMainParam(dyn_lwp *trappingLWP) 
{
   dyn_saved_regs regs;
   trappingLWP->getRegisters(&regs);
   Address mainaddr;
   if (!readDataSpace((void*)(regs.gprs.esp + getAddressWidth()),
            getAddressWidth(), (void*)&mainaddr,true)) {
      fprintf(stderr,"[%s][%d]: failed readDataSpace\n", __FILE__,__LINE__); 
   }
   return mainaddr;
} 

#endif

void process::setTraceSysCalls(bool traceSys) 
{
   traceSysCalls_ = traceSys;
}

void process::setTraceState(traceState_t state) 
{
   traceState_ = state;
}

/* For cases in which we are unable to locate the mutatee's main function
 * via our normal findMain heuristics, we resort to tracing system calls
 * looking for libc to be loaded.  Here we decode the different system
 * that we see during process startup. 
 */
bool process::decodeStartupSysCalls(EventRecord &ev) 
{
   // set up parameters & variables, which is platform dependent
   const int NUMPARAMS = 6;
   dyn_saved_regs regs;
   ev.lwp->getRegisters(&regs);
   long params[NUMPARAMS];
   int callnum = getSysCallNumber(&regs);
   long retval = getSysCallReturnValue(&regs);
   Address programCounter = getSysCallProgramCounter(&regs);
   getSysCallParameters(&regs, params, NUMPARAMS);

   int traceerr = 0;
   ev.type = evtNullEvent;
   int addrWidth=getAddressWidth();

   startup_printf("%s[%d]: decodeStartupSysCalls got tracestate=%d callnum=%d PC=0x%lx\n", 
         FILE__, __LINE__, getTraceState(), callnum, programCounter);

   // mmap syscall (there are multiple mmap syscalls on some platforms)
   // store the start and end addresses of the region so that we can later
   // determine which of the regions contains main
   if (isMmapSysCall(callnum)) {
      if (retval == -ENOSYS) { // grab the mmap region end address
         mappedRegionEnd.push_back(getMmapLength(callnum, &regs));
      } else { // the return value of mmap is the region's start address
         mappedRegionStart.push_back(retval);
         // turn the OFFSET we put into mappedRegionEnd into an absolute Address
         mappedRegionEnd[mappedRegionEnd.size()-1] 
            = mappedRegionEnd[mappedRegionEnd.size()-1] + retval;
         startup_printf("%s[%d]: traced mmap syscall for region[0x%x 0x%x]\n",
               __FILE__,__LINE__,
               (int)mappedRegionStart[mappedRegionStart.size()-1],
               (int)mappedRegionEnd[mappedRegionEnd.size()-1]);
      }
   }
   // munmap: we also keep track of memory regions that are removed
   else if (callnum==SYS_munmap && retval == -ENOSYS) { 
      Address regionStart = params[0];
      munmappedRegions.push_back(regionStart);
      startup_printf("%s[%d]: traced munmap syscall for region at 0x%x\n",
            __FILE__,__LINE__, (int)regionStart);
   }

   // switch on state, this is an automaton
   switch (getTraceState()) { 
      case libcOpenCall_ts: // wait for call to open libc.so
         if(callnum == SYS_open) { // open syscall
            char *oneword = (char*) malloc(addrWidth);
            char *pathbuffer = (char*) malloc(256);
            char *ptr = pathbuffer;
            int lastchar = addrWidth;
            Address stringAddr = params[0];
            // recover the opened path from the call's arguments
            do {
               if (!readDataSpace((void*)stringAddr, addrWidth,
                        (void*)oneword,true)) {
                  fprintf(stderr,"%s[%d]: failed readDataSpace\n",FILE__,__LINE__ );
                  return false;
               }
               strncpy(ptr, oneword, addrWidth);
               stringAddr += addrWidth; 
               ptr += addrWidth;
               for (int idx=0; idx < addrWidth; idx++) {
                  if (oneword[idx] == '\0') {
                     lastchar = idx;
                  }
               }
            }while (ptr - pathbuffer < 256 && lastchar == addrWidth);
            pathbuffer[255] = '\0';
            // if the path matches libc.so, transition state
            if (strstr(pathbuffer, "libc.so")) {
               setTraceState(libcOpenRet_ts);
            }
            free(pathbuffer);
            free(oneword);
         } // end if syscall to open
         break;
      case libcOpenRet_ts: // Save file-handle returned by open
         // syscall on libc
         if(callnum == SYS_open) { // call to open
            if (retval >= 0) {
               libcHandle_ = retval; 
               setTraceState(libcClose_ts);
            }
            else {// open syscall was unsuccessful, revert to previous state
               setTraceState(libcOpenCall_ts);
            }
         }
         break;
      case libcClose_ts:  //find close syscall matching libc open 
         if (callnum == SYS_close && retval == -ENOSYS) {
            int fd = params[0];
            if (traceerr != 0) { 
               fprintf(stderr,"[%s][%d]: ptrace err here\n", __FILE__,__LINE__); 
            }
            if (fd == libcHandle_) { // found close
               setTraceState(instrumentLibc_ts); 
               setBootstrapState(libcLoaded_bs);
               ev.type = evtLibcLoaded;
            }
         }
         break;
      case instrumentLibc_ts: // results in handling of trap in startmain 
         if (abs((long)getlibcstartmain_brk_addr() -(long)programCounter) <= 1) {
            setTraceState(done_ts);
            setTraceSysCalls(false);
            ev.type = evtLibcTrap;
         }
         else {
            return false;
         }
         break;
      default:
         fprintf(stderr,"[%s][%d] Internal error, should not reach this point\n",
               __FILE__,__LINE__);
         return false;
   }// end switch statement
   return true;
}// end decodeStartupSysCalls

/* Find libc and add it as a shared object
 * Search for __libc_start_main
 * Save old code at beginning of __libc_start_main
 * Insert trap
 * Signal thread to continue
 */
bool process::instrumentLibcStartMain() 
{
    unsigned int maps_size =0;
    map_entries *maps = getLinuxMaps(getPid(), maps_size);
    unsigned int libcIdx=0;
    while (libcIdx < maps_size &&
           ! (strstr(maps[libcIdx].path,"/libc")
              && strstr(maps[libcIdx].path,".so"))) {
       libcIdx++;
    }
    assert(libcIdx != maps_size);
    //KEVINTODO: address code and data are not always 0,0: need to fix this
    fileDescriptor libcFD = fileDescriptor(maps[libcIdx].path,0,0,true);
    mapped_object *libc = mapped_object::createMappedObject(libcFD, this);
    addASharedObject(libc);

    // find __libc_startmain
    const pdvector<int_function*> *funcs;
    funcs = libc->findFuncVectorByPretty("__libc_start_main");
    if(funcs->size() == 0 || (*funcs)[0] == NULL) {
        logLine( "Couldn't find __libc_start_main\n");
        return false;
    } else if (funcs->size() > 1) {
       startup_printf("[%s:%u] - Found %d functions called __libc_start_main, weird\n",
                      FILE__, __LINE__, funcs->size());
    }
    if (!(*funcs)[0]->isInstrumentable()) {
        logLine( "__libc_start_main is not instrumentable\n");
        return false;
    }
    Address addr = (*funcs)[0]->getAddress();
    startup_printf("%s[%d]: Instrumenting libc.so:__libc_start_main() at 0x%x\n", 
                   FILE__, __LINE__, (int)addr);

    // save original code at beginning of function
    if (!readDataSpace((void *)addr, sizeof(savedCodeBuffer),savedCodeBuffer,true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
        fprintf(stderr, "%s[%d][%s]:  failing instrumentLibcStartMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      fprintf(stderr, "Failed to read at address 0x%lx\n", addr);
      return false;
   }
   startup_printf("%s[%d]: Saved %d bytes from entry of __libc_start_main\n", 
         FILE__, __LINE__, sizeof(savedCodeBuffer));
   // insert trap
   codeGen gen(1);
   insnCodeGen::generateTrap(gen);
   if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr())) {
      fprintf(stderr, "%s[%d][%s]:  failing instrumentLibcStartMain\n",
            __FILE__, __LINE__, getThreadStr(getExecThreadID()));
      return false;
   }
   libcstartmain_brk_addr = addr;
   continueProc(); // signal process to continue
   return true;
}// end instrumentLibcStartMain


/* Read parameters to startmain including the address of main.
 * See if there's an existing mapped object that contains the address of
 *   main, which we'll call a_out, and set to be a.out for the process
 * Restore original instruction in the libc library
 * Call insertTrapAtMain
 */
bool process::handleTrapAtLibcStartMain(dyn_lwp *trappingLWP)
{
    assert(libcstartmain_brk_addr);
    assert(trappingLWP);

    // Read the parameters from the call to libcStartMain
    Address mainaddr = getLibcStartMainParam(trappingLWP);
    mapped_object *a_out = findObject(mainaddr);

    char namebuf[64];
    Address regionStart = 0;
    Address regionEnd = 0;
    // there might not be an object that corresponds to the a_out
    if (a_out == NULL) {
        // determine confines of region that encloses main, search from
        // end to get most recent mmap of an enclosing region
        int idx =  (int)mappedRegionStart.size()-1;
        while (idx >= 0  && 
               !(mappedRegionStart[idx] <= mainaddr 
                 && mainaddr <= mappedRegionEnd[idx])) {
            idx--;
        }
        if (idx < 0) {
            fprintf(stderr,"%s[%d] No valid memory region seems to contain "
                    "the address of main=0x%x\n",__FILE__,__LINE__, 
                    (int)mainaddr);
            return false;
        }
        regionStart = mappedRegionStart[idx];
        regionEnd = mappedRegionEnd[idx];
        startup_printf("main(0x%x) is in region [0x%X 0x%X]\n", 
                       (int)mainaddr, (int)regionStart, (int)regionEnd);

        // found the right region, copy its contents to a temp file
        void *regionBuf = malloc(regionEnd - regionStart);
        if (!readDataSpace((void*)regionStart, regionEnd-regionStart, 
                          regionBuf, false)) {
            fprintf(stderr, "%s[%d]: Failed to read from region [%X %X]\n",
                       __FILE__, __LINE__,(int)regionStart, 
                       (int)regionEnd);
        }
        // Make sure bytes 12-15 of magic header are set to 0 (for x86-64 bit)
        ((int*)regionBuf)[3] = 0;
        // We have no section information, so don't try to point to it
        ((int*)regionBuf)[10] = 0;

        snprintf(namebuf, 64, "/tmp/MemRegion_%X_%X", 
                 (int)regionStart, (int)regionEnd);
        namebuf[63]='\0';
        FILE *regionFD = fopen(namebuf, "w");
        assert(regionFD != NULL);
        fwrite(regionBuf, 1, regionEnd-regionStart, regionFD);
        fclose(regionFD);
        free(regionBuf);
        startup_printf("%s[%d]: Read region [%X %X] into temp file %s\n",
                       __FILE__, __LINE__,(int)regionStart, 
                       (int)regionEnd, namebuf);

        //KEVINTODO: address code and data are not always 0,0: need to fix this
        // create a fileDescriptor and a new mapped_object for the region
        // create it as though it were a shared object
        fileDescriptor fdesc = fileDescriptor(namebuf, 0, 0, true);
        a_out = mapped_object::createMappedObject(fdesc, this);

        // There is no function for adding an a.out, so we'll call
        // addASharedObject, and switch the old a.out to be this one.
        // a.out is always the first object in the mapped_objects
        // vector, our new mapped_object should be at or near the end.
        addASharedObject(a_out);
        idx = mapped_objects.size() -1;
        while (mapped_objects[idx] != a_out && idx >= 0) {
            idx--;
        }
        assert(idx >= 0);
        mapped_objects[idx] = mapped_objects[0];
        mapped_objects[0] = a_out;
        ((fileDescriptor)(a_out->getFileDesc())).setIsShared(false);
    }
    else {// (a_out != NULL)
        regionStart = a_out->getFileDesc().loadAddr();
        regionEnd = regionStart + a_out->imageSize();
    }

    // if gap parsing is on, check for a function at mainaddr, rename
    // it to "main" and set main_function to its int_function
    int_function* mainfunc = NULL;
    if (a_out->parse_img()->parseGaps()) {
        a_out->analyze();
        snprintf(namebuf,64,"gap_%lx", (long)mainaddr);
        int_function* mainfunc = findOnlyOneFunction(namebuf,a_out->fileName());
        if (mainfunc) {
            mainfunc->addSymTabName("main", true);
            mainfunc->addPrettyName("main", true);
            main_function = mainfunc;
            startup_printf("found main via gap parsing at %x in mapped_obj [%x %x]\n",
                           (int)main_function->getAddress(),
                           (int)a_out->getFileDesc().loadAddr(),
                           (int)(a_out->getFileDesc().loadAddr() + a_out->imageSize()));
        }
    }
    // parse the binary with "main" as a function location
    if (!mainfunc) {
        startup_printf("Parsing main at 0x%x in mapped object [%x %x]"
                       " which is its location according to __libc_start_main \n",
                (int)mainaddr,
                (int)a_out->getFileDesc().loadAddr(),
                (int)(a_out->getFileDesc().loadAddr() + a_out->imageSize()));
        // add function stub and parsed object
        a_out->parse_img()->addFunction(mainaddr, "main");
        a_out->analyze(); 
    }

   // Restore __libc_start_main to its original state
   if (!writeDataSpace((void *)libcstartmain_brk_addr, 
            sizeof(savedCodeBuffer), 
            (char *)savedCodeBuffer)) {
      fprintf(stderr, "%s[%d]: Failed to restore code in libcstartmain at 0x%X\n",
            __FILE__, __LINE__,(int)libcstartmain_brk_addr);
      return false;
   }
   if (! trappingLWP->changePC(libcstartmain_brk_addr,NULL)) {
      logLine("WARNING: in handleTrapAtLibcStartMain: "
            "changePC failed in handleTrapAtLibcStartmain\n");
      assert(0);
   }
   libcstartmain_brk_addr = 0;
   // now let insertTrapAtEntryPointOfMain insert the trap at main
   return insertTrapAtEntryPointOfMain();
}// handleTrapAtLibcStartmain


extern bool isFramePush(instruction &i);

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
   if (ehdr == NULL)
      goto err_handler;
   
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
      if (!p->getVsyscallText() && (shdr->sh_flags & SHF_EXECINSTR)) {
         p->setVsyscallText(shdr->sh_addr);
      }
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
                  if (signal_addr < p->getVsyscallEnd() - p->getVsyscallStart()) {
                     p->addSignalHandler(syms[i].st_value + 
                                         p->getVsyscallStart(), 4);
                  }
                  else if (signal_addr < p->getVsyscallStart() ||
                           signal_addr >= p->getVsyscallEnd()) {
                     //FC-9 moved the vsyscall page, but didn't update its debug
                     // info or symbols from some hardcoded values.  Fix up the
                     // bad signal values.
                     p->addSignalHandler(syms[i].st_value - 0xffffe000 +
                                         p->getVsyscallStart(), 4);
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

static volatile int segfaulted = 0;
static void catchSigSegV(int) {
   segfaulted = 1;
}

static char *execVsyscallFetch(process *p, char *buffer) {
   //We can't read the Vsyscall page out of the process addr
   // space do to a kernel bug.  However, the versions of the
   // kernel with this bug don't move the vsyscall page between
   // processes, so let's read it out of the mutator's.
   //Latter versions of the kernel do move the page between
   // processes, which makes our read attempt seg fault.  Let's
   // install a handler to be safe.
   volatile char *start;
   unsigned size;
   sighandler_t old_segv;

   segfaulted = 0;
   start = (char *) p->getVsyscallStart();
   size = p->getVsyscallEnd() - p->getVsyscallStart();

   //Install SegSegV handler.
   old_segv = signal(SIGSEGV, catchSigSegV);

   //Copy buffer
   for (unsigned i=0; i<size; i++) {
      buffer[i] = start[i];
      if (segfaulted) 
         break;
   }

   //Restore handler.
   signal(SIGSEGV, old_segv);
   if (segfaulted)
      return NULL;
   return buffer;
}

static bool isVsyscallData(char *buffer, int dso_size) {
   //Start with an elf header?
   if (dso_size < 4)
      return false;
   return (buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' &&
           buffer[3] == 'F');
   
}


void calcVSyscallFrame(process *p)
{
  unsigned dso_size;
  char *buffer;

  /**
   * If we've already calculated and cached the DSO information then 
   * just return.
   **/
  if (p->getVsyscallObject())
     return;
  

  if (p->getAddressWidth() == 8) {
     // FIXME: HACK to disable vsyscall page for AMD64, for now.
     //  Reading the VSyscall data on ginger seems to trigger a
     //  kernel panic.
     p->setVsyscallRange(0x1000, 0x0);
     return;
  }

  /**
   * Read the location of the vsyscall page from /proc/.
   **/
  p->readAuxvInfo();
  if (p->getVsyscallStatus() != vsys_found) {
     p->setVsyscallRange(0x0, 0x0);
     return;
  }
  
  /**
   * Read the vsyscall page out of process memory.
   **/
  dso_size = p->getVsyscallEnd() - p->getVsyscallStart();
  buffer = (char *) calloc(1, dso_size);
  assert(buffer);
  if (!p->readDataSpace((caddr_t)p->getVsyscallStart(), dso_size, buffer,false))     
  {
     int major, minor, sub;
     get_linux_version(major, minor, sub);
     if (major == 2 && minor == 6 && sub <= 2 && sub >= 0) {
        //Linux 2.6.0 - Linux 2.6.2 has a  bug where ptrace 
        // can't read from the DSO.  The process can read the memory, 
        // it's just ptrace that's acting stubborn.
        if (!execVsyscallFetch(p, buffer))
        {
           p->setVsyscallStatus(vsys_notfound);
           return;
        }
     }
  }

  if (!isVsyscallData(buffer, dso_size)) {
     p->setVsyscallRange(0x0, 0x0);
     p->setVsyscallStatus(vsys_notfound);
     return;     
  }
  getVSyscallSignalSyms(buffer, dso_size, p);

  Symtab *obj;
  bool result = Symtab::openFile(obj, buffer, dso_size);
  if (result)
     p->setVsyscallObject(obj);
  return;
}

bool Frame::setPC(Address newpc) {
   if (!pcAddr_)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   if (!getProc()->writeDataSpace((void*)pcAddr_, sizeof(Address), &newpc))
      return false;
   pc_ = newpc;
   range_ = NULL;

   return true;
}

// Laziness here: this func is used by the iRPC code
// to get result registers. Don't use it other than that. 

Address dyn_lwp::readRegister(Register /*reg*/) {
   // On x86, the result is always stashed in %EAX
   if(status() == running) {
      cerr << "    performance problem in call to dyn_lwp::readRegister\n"
           << "       successive pauses and continues with ptrace calls\n";
   }

   int ptrace_errno = 0;
   Address ret = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(), P_offsetof(struct user_regs_struct, PTRACE_REG_AX), 0,
                        &ptrace_errno, proc_->getAddressWidth(),  __FILE__, __LINE__);
   return ret;
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

    if (target_pdf) {
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
    }
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
        sprintf(errorLine, "read error in process::hasBeenBound addr 0x%x, pid=%d\n (readDataSpace returns 0)",(unsigned)got_entry,getPid());
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

bool AddressSpace::getDyninstRTLibName() {
   startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
           std::string msg;
           process *proc;
           if ((proc = dynamic_cast<process *>(this)) != NULL) {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined for process ") +
                 utos(proc->getPid());
           }
           else {
              msg = std::string("Environment variable ") +
                 std::string("DYNINSTAPI_RT_LIB") +
                 std::string(" has not been defined");
           }           
           showErrorCallback(101, msg);
           return false;
        }
    }

    // Automatically choose 32-bit library if necessary.
    const char *modifier = "_m32";
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

    if ( getAddressWidth() == sizeof(void *) || P_strstr(name, modifier) ) {
        modifier = "";
    }

    const char *suffix = split;
    if( getAOut()->isStaticExec() ) {
        suffix = ".a";
    }else{
        if( P_strncmp(suffix, ".a", 2) == 0 ) {
	  // Add symlinks in makefiles as follows:
	  // (lib).so => lib.so.(major)
	  // lib.so.major => lib.so.major.minor
	  // lib.so.major.minor => lib.so.major.minor.maintenance
	  suffix = ".so";
        }
    }

    dyninstRT_name = std::string(name, split - name) +
                     std::string(modifier) +
                     std::string(suffix);

    startup_printf("Dyninst RT Library name set to '%s'\n",
            dyninstRT_name.c_str());

    // Check to see if the library given exists.
    if (access(dyninstRT_name.c_str(), R_OK)) {
        std::string msg = std::string("Runtime library ") + dyninstRT_name
        + std::string(" does not exist or cannot be accessed!");
        showErrorCallback(101, msg);
        return false;
    }

    return true;
}

bool process::loadDYNINSTlib()
{
    pdvector<int_function *> dlopen_funcs;

	//  allow user to override default dlopen func names 
	//  with env. var

	DL_OPEN_FUNC_USER = getenv("DYNINST_DLOPEN_FUNC");

	if (DL_OPEN_FUNC_USER)
	{
		if (findFuncsByAll(DL_OPEN_FUNC_USER, dlopen_funcs)) 
		{
			bool ok =  loadDYNINSTlib_exported(DL_OPEN_FUNC_USER, DLOPEN_MODE);

			if (ok) 
				return true;

			//  else fall through and try the default dlopen names
		} 
	}

    if (findFuncsByAll(DL_OPEN_FUNC_EXPORTED, dlopen_funcs)) 
	{
		return loadDYNINSTlib_exported(DL_OPEN_FUNC_EXPORTED, DLOPEN_MODE);
    } 
    else if( findFuncsByAll(DL_OPEN_LIBC_FUNC_EXPORTED, dlopen_funcs) ) {
            // If libc and the loader don't have symbols, we need to take a
            // different approach. We still need to the stack protection turned
            // off, but since we don't have symbols we use an undocumented flag
            // to turn off the stack protection
            return loadDYNINSTlib_exported(DL_OPEN_LIBC_FUNC_EXPORTED, DLOPEN_MODE | __RTLD_DLOPEN);
    }else{
		return loadDYNINSTlib_hidden();
    }
}

// Defined in inst-x86.C...
void emitPushImm(unsigned int imm, unsigned char *&insn); 

bool process::loadDYNINSTlib_hidden() {
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

  codeGen scratchCodeBuffer(MAX_IRPC_SIZE);
  scratchCodeBuffer.setAddrSpace(this);
  scratchCodeBuffer.setRegisterSpace(registerSpace::irpcRegSpace(this));

  // we need to make a call to dlopen to open our runtime library

  // Variables what we're filling in
  Address dyninstlib_str_addr = 0;
  Address dlopen_call_addr = 0;

  pdvector<int_function *> dlopen_funcs;
  if (!findFuncsByAll(DL_OPEN_FUNC_NAME, dlopen_funcs))
  {
    pdvector<int_function *> dlopen_int_funcs;                                    
    // If we can't find the do_dlopen function (because this library
    // is stripped, for example), try searching for the internal
    // _dl_open function and find the do_dlopen function by examining
    // the functions that call it. This depends on the do_dlopen
    // function having been parsed (though its name is not known)
    // through speculative parsing.
    if(!findFuncsByAll(DL_OPEN_FUNC_INTERNAL, dlopen_int_funcs))
    {    
      startup_printf("Failed to find _dl_open\n");
    } 
    else
    { 
      if(dlopen_int_funcs.size() > 1)
        {
            startup_printf("%s[%d] warning: found %d matches for %s\n",
                           __FILE__,__LINE__,dlopen_int_funcs.size(),
                           DL_OPEN_FUNC_INTERNAL);
        }
        dlopen_int_funcs[0]->getStaticCallers(dlopen_funcs);
        if(dlopen_funcs.size() > 1)
        {
            startup_printf("%s[%d] warning: found %d do_dlopen candidates\n",
                           __FILE__,__LINE__,dlopen_funcs.size());
        }
  
        if(dlopen_funcs.size() > 0)
        {
            // give it a name
            dlopen_funcs[0]->addSymTabName("do_dlopen",true);
        }
    }
  }

    if(dlopen_funcs.size() == 0)
    {
      startup_cerr << "Couldn't find method to load dynamic library" << endl;
      return false;
    } 

  Address dlopen_addr = dlopen_funcs[0]->getAddress();

  assert(dyninstRT_name.length() < BYTES_TO_SAVE);
  // We now fill in the scratch code buffer with appropriate data
  startup_cerr << "Dyninst RT lib name: " << dyninstRT_name << endl;

  dyninstlib_str_addr = codeBase + scratchCodeBuffer.used();
  scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

  startup_printf("(%d) dyninst str addr at 0x%x\n", getPid(), dyninstlib_str_addr);

  startup_printf("(%d) after copy, %d used\n", getPid(), scratchCodeBuffer.used());


#if defined(bug_syscall_changepc_rewind)
  // Reported by SGI, during attach to a process in a system call:

  // Insert eight NOP instructions before the actual call to dlopen(). Loading
  // the runtime library when the mutatee was in a system call will sometimes
  // cause the process to (on IA32 anyway) execute the instruction four bytes
  // PREVIOUS to the PC we actually set here. No idea why. Prepending the
  // actual dlopen() call with eight NOP instructions insures this doesn't
  // really matter. Eight was selected rather than four because I don't know
  // if x86-64 does the same thing (and jumps eight bytes instead of four).

  // We will put in <addr width> rather than always 8; this will be 4 on x86 and
  // 32-bit AMD64, and 8 on 64-bit AMD64.

  scratchCodeBuffer.fill(getAddressWidth(), codeGen::cgNOP);

  // And since we apparently execute at (addr - <width>), shift dlopen_call_addr
  // up past the NOPs.
#endif

  // Sync with whatever we've put in so far.
  dlopen_call_addr = codeBase + scratchCodeBuffer.used();

  if( !theRpcMgr->emitInferiorRPCheader(scratchCodeBuffer) ) {
    startup_cerr << "Couldn't emit inferior RPC header" << endl;
    return false;
  }

  // Since we are punching our way down to an internal function, we
  // may run into problems due to stack execute protection. Basically,
  // glibc knows that it needs to be able to execute on the stack in
  // in order to load libraries with dl_open(). It has code in
  // _dl_map_object_from_fd (the workhorse of dynamic library loading)
  // that unprotects a global, exported variable (__stack_prot), sets
  // the execute flag, and reprotects it. This only happens, however,
  // when the higher-level dl_open() functions (which we skip) are called,
  // as they append an undocumented flag to the library open mode. Otherwise,
  // assignment to the variable happens without protection, which will
  // cause a fault.
  //
  // Instead of chasing the value of the undocumented flag, we will
  // unprotect the __stack_prot variable ourselves (if we can find it).

  if(!tryUnprotectStack(scratchCodeBuffer,codeBase)) {
    startup_printf("Failed to disable stack protection.\n");
  }

#if defined(cap_32_64)
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
      insnCodeGen::generateCall(scratchCodeBuffer, scratchCodeBuffer.used() + codeBase, dlopen_addr);
      
      const unsigned stackUsage = 5*4; // 5 pushes
      RealRegister esp(REGNUM_ESP);
      RealRegister enull(Null_Register);
      emitLEA(esp, enull, 0, stackUsage, esp, scratchCodeBuffer);
      scratchCodeBuffer.rs()->incStack(-(stackUsage-4)); // 4 registered pushes

#if defined(cap_32_64)
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

      const unsigned stackUsage = 4*8; // 4 pushes
      emitLEA64(REGNUM_RSP, Null_Register, 0, stackUsage, REGNUM_RSP, true, scratchCodeBuffer);
  }
#endif

  // The break point address is computed by the following function
  unsigned breakOffset = 0;
  unsigned unused;
  if( !theRpcMgr->emitInferiorRPCtrailer(scratchCodeBuffer, breakOffset, 
                false, unused, unused) ) 
  {
    startup_cerr << "Couldn't emit inferior RPC trailer" << endl;
    return false;
  }
  dyninstlib_brk_addr = codeBase + breakOffset;

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

  lwp_to_use = NULL;

  if(process::IndependentLwpControl() && getRepresentativeLWP() ==NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();

  Address destPC = dlopen_call_addr;

  startup_printf("Changing PC to 0x%x\n", destPC);
  startup_printf("String at 0x%x\n", dyninstlib_str_addr);

  if (! lwp_to_use->changePC(destPC,NULL))
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

bool process::loadDYNINSTlib_exported(const char *dlopen_name, int mode)
{
	// dlopen takes two arguments:
	// const char *libname;
	// int mode;
	// We put the library name on the stack, push the args, and
	// emit the call

	Address codeBase = findFunctionToHijack(this);
	if (!codeBase) 
	{
		startup_cerr << "Couldn't find a point to insert dlopen call" << endl;
		return false;
	}

	Address dyninstlib_str_addr = 0;
	Address dlopen_call_addr = 0;

	pdvector<int_function *> dlopen_funcs;

	if (!findFuncsByAll(dlopen_name ? dlopen_name : DL_OPEN_FUNC_EXPORTED, dlopen_funcs)) 
	{
		startup_cerr << "Couldn't find method to load dynamic library" << endl;
		return false;
	} 

	assert(dlopen_funcs.size() != 0);
	
	if (dlopen_funcs.size() > 1) 
	{
		logLine("WARNING: More than one dlopen found, using the first\n");
	}

	Address dlopen_addr = dlopen_funcs[0]->getAddress();

	// We now fill in the scratch code buffer with appropriate data
	codeGen scratchCodeBuffer(MAX_IRPC_SIZE);
        scratchCodeBuffer.setAddrSpace(this);
        scratchCodeBuffer.setRegisterSpace(registerSpace::irpcRegSpace(this));

	assert(dyninstRT_name.length() < MAX_IRPC_SIZE);

	// The library name goes first
	dyninstlib_str_addr = codeBase;
	scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

#if defined(bug_syscall_changepc_rewind)
	//Fill in with NOPs, see loadDYNINSTlib_hidden
	scratchCodeBuffer.fill(getAddressWidth(), codeGen::cgNOP);
#endif
        // Now the real code
	dlopen_call_addr = codeBase + scratchCodeBuffer.used();

        if( !theRpcMgr->emitInferiorRPCheader(scratchCodeBuffer) ) {
            startup_cerr << "Couldn't emit inferior RPC header" << endl;
            return false;
        }
	
	bool mode64bit = (getAddressWidth() == sizeof(uint64_t));

	if (!mode64bit) 
	{
		// Push mode
		emitPushImm(mode, scratchCodeBuffer);

		// Push string addr
		emitPushImm(dyninstlib_str_addr, scratchCodeBuffer);

		insnCodeGen::generateCall(scratchCodeBuffer,
				scratchCodeBuffer.used() + codeBase,
				dlopen_addr);
                const unsigned stackUsage = 2*4; // 2 pushes
                RealRegister esp(REGNUM_ESP);
                RealRegister enull(Null_Register);
                emitLEA(esp, enull, 0, stackUsage, esp, scratchCodeBuffer);
                scratchCodeBuffer.rs()->incStack(-stackUsage);
	}
	else 
	{
		// Set mode
		emitMovImmToReg64(REGNUM_RSI, mode, false, scratchCodeBuffer);
		// Set string addr
		emitMovImmToReg64(REGNUM_RDI, dyninstlib_str_addr, true,
				scratchCodeBuffer);
		// The call (must be done through a register in order to reach)
		emitMovImmToReg64(REGNUM_RAX, dlopen_addr, true, scratchCodeBuffer);
		emitSimpleInsn(0xff, scratchCodeBuffer);
		emitSimpleInsn(0xd0, scratchCodeBuffer);
	}

        // The break point address is computed by the following function
        unsigned breakOffset = 0;
        unsigned unused;
        if( !theRpcMgr->emitInferiorRPCtrailer(scratchCodeBuffer, breakOffset, 
                    false, unused, unused) ) 
        {
            startup_cerr << "Couldn't emit inferior RPC trailer" << endl;
            return false;
        }
        dyninstlib_brk_addr = codeBase + breakOffset;

	if (!readDataSpace((void *)codeBase,
				sizeof(savedCodeBuffer), savedCodeBuffer, true)) 
	{
		fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
		return false;
	}

	if (!writeDataSpace((void *)(codeBase), scratchCodeBuffer.used(),
				scratchCodeBuffer.start_ptr())) 
	{
		fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
		return false;
	}

	// save registers
	dyn_lwp *lwp_to_use = NULL;

	if (process::IndependentLwpControl() && getRepresentativeLWP() == NULL)
	{
		lwp_to_use = getInitialThread()->get_lwp();
	}
	else
	{
		lwp_to_use = getRepresentativeLWP();
	}

	savedRegs = new dyn_saved_regs;
	bool status = lwp_to_use->getRegisters(savedRegs);

	assert((status != false) && (savedRegs != (void *)-1));

	if (!lwp_to_use->changePC(dlopen_call_addr,NULL))  
	{
		logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
		return false;
	}

	setBootstrapState(loadingRT_bs);
	return true;
}

Address process::tryUnprotectStack(codeGen &buf, Address codeBase) 
{
	// find variable __stack_prot

	// mprotect READ/WRITE __stack_prot
	pdvector<int_variable *> vars; 
	pdvector<int_function *> funcs;

	Address var_addr;
	Address func_addr;
	Address ret_addr;
	int size;
	int pagesize;
    Address page_start;
    bool ret;
    
    ret = findVarsByAll("__stack_prot", vars);

    if(!ret || vars.size() == 0) {
        return 0;
    } else if(vars.size() > 1) {
        startup_printf("Warning: found more than one __stack_prot variable\n");
    }

    pagesize = getpagesize();

    var_addr = vars[0]->getAddress();
    page_start = var_addr & ~(pagesize -1);
    size = var_addr - page_start +sizeof(int);

    ret = findFuncsByAll("mprotect",funcs);

    if(!ret || funcs.size() == 0) {
        startup_printf("Couldn't find mprotect\n");
        return 0;
    }

    int_function * mprot = funcs[0];
    func_addr = mprot->getAddress();
    ret_addr = codeBase + buf.used();

#if defined(arch_x86_64)
  if (getAddressWidth() == 4) {
#endif
      // Push caller
      emitPushImm(func_addr, buf);
      
      // Push mode (READ|WRITE|EXECUTE)
      emitPushImm(7, buf);
      
      // Push variable size
      emitPushImm(size, buf);
      
      // Push variable location
      emitPushImm(page_start, buf);
     
      startup_printf("(%d): emitting call for mprotect from 0x%x to 0x%x\n",
		     getPid(), codeBase + buf.used(), func_addr);
      insnCodeGen::generateCall(buf, buf.used() + codeBase, func_addr);

      const unsigned stackUsage = 4*4; // 4 pushes
      RealRegister esp(REGNUM_ESP);
      RealRegister enull(Null_Register);
      emitLEA(esp, enull, 0, stackUsage, esp, buf);
      buf.rs()->incStack(-stackUsage);

#if defined(arch_x86_64)
  } else {
      // Push caller
      //emitMovImmToReg64(REGNUM_RAX, func_addr, true, buf);
      //emitSimpleInsn(0x50, buf); // push %rax       

      // Push mode (READ|WRITE|EXECUTE)
      emitMovImmToReg64(REGNUM_RDX, 7, true, buf); //32-bit mov
   
      // Push variable size
      emitMovImmToReg64(REGNUM_RSI, size, true, buf); //32-bit mov

      // Push variable location 
      emitMovImmToReg64(REGNUM_RDI, page_start, true, buf);

      // The call (must be done through a register in order to reach)
      emitMovImmToReg64(REGNUM_RAX, func_addr, true, buf);
      emitSimpleInsn(0xff, buf); // group 5
      emitSimpleInsn(0xd0, buf); // mod=11, ext_op=2 (call Ev), r/m=0 (RAX)
  }
#endif
    
    return ret_addr;
}



Frame process::preStackWalkInit(Frame startFrame) 
{
  /* Do a special check for the vsyscall page.  Silently drop
     the page if it exists. */
  calcVSyscallFrame( this );
  
  Address next_pc = startFrame.getPC();
  if ((next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) ||
      /* RH9 Hack */ (next_pc >= 0xffffe000 && next_pc < 0xfffff000)) {
     return startFrame.getCallerFrame();
  }
  return startFrame;
}

#if defined(arch_x86_64)
void print_regs(dyn_lwp *lwp)
{
   struct dyn_saved_regs regs;
   bool result = lwp->getRegisters(&regs, false);

   if (result)
      fprintf(stderr, "rax = %lx\n", regs.gprs.rax);
}
#endif
