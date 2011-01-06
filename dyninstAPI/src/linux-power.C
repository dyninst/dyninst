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

// $Id: linux-power.C,v 1.19 2008/06/19 19:53:26 legendre Exp $

#include <string>
#include <dlfcn.h>

#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/linux-power.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/signalgenerator.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/function.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)

#if defined(arch_64bit) && 0 // XXX FIXME - Need a configure variable to see
                             //             if struct pt_regs32 exists.
#define PT_REGS_OFFSET(m, mutatee_address_width)           \
        (                                                  \
            ((mutatee_address_width) == sizeof(uint64_t))  \
            ? (   /* 64-bit mutatee */                     \
                  P_offsetof(struct pt_regs, m)            \
              )                                            \
            : (   /* 32-bit mutatee */                     \
                  P_offsetof(struct pt_regs32, m)          \
              )                                            \
        )
#else
#define PT_REGS_OFFSET(m, mutatee_address_width)           \
        (                                                  \
            P_offsetof(struct pt_regs, m)                  \
        )
#endif


#define PT_FPSCR_OFFSET(mutatee_address_width)                  \
        (                                                       \
            ((mutatee_address_width) == sizeof(PTRACE_RETURN))  \
            ? (   /* N-bit mutatee, N-bit mutator   */          \
                  PT_FPSCR * sizeof(PTRACE_RETURN)              \
              )                                                 \
            : (   /* 32-bit mutatee, 64-bit mutator */          \
                  (PT_FPR0 + 2*32 + 1) * sizeof(uint32_t)       \
              )                                                 \
        )


#define SIZEOF_PTRACE_DATA(mutatee_address_width)  (mutatee_address_width)


void calcVSyscallFrame(process *p)
{
  assert(0);  //sunlung
  unsigned dso_size;
  char *buffer;

  /**
   * If we've already calculated and cached the DSO information then
   * just return.
   **/

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
/*if (!p->readDataSpace((caddr_t)p->getVsyscallStart(), dso_size, buffer,false))
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
  result = parseVsyscallPage(buffer, dso_size, p);
*/
  return;
}


bool dyn_lwp::changePC(Address loc,
                       struct dyn_saved_regs */*ignored registers*/)
{
   Address regaddr = PT_REGS_OFFSET(PTRACE_REG_IP, proc_->getAddressWidth());
   assert(get_lwp_id() != 0);
   int ptrace_errno = 0;
   if (0 != DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(), regaddr, loc,
                       &ptrace_errno, proc_->getAddressWidth(),
                       __FILE__, __LINE__ )) {
      fprintf(stderr, "dyn_lwp::changePC - PTRACE_POKEUSER failure for %u",
              get_lwp_id());
      return false;
   }

   return true;
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

   // frame pointer
   fp = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                   PT_REGS_OFFSET(PTRACE_REG_FP, proc_->getAddressWidth()), 0,
                   &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   // next instruction pointer
   pc = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                   PT_REGS_OFFSET(PTRACE_REG_IP, proc_->getAddressWidth()), 0,
                   &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   // no top-of-stack pointer for POWER/PowerPC
   sp = 0;

   dbi_printf("%s[%d]:  GET ACTIVE FRAME (pc = %p, sp = %p, fp = %p\n",
              FILE__, __LINE__, pc, sp, fp);

   return Frame(pc, fp, sp, proc_->getPid(), proc_, NULL, this, true);
}




bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs, bool includeFP) {
   int           error = 0;
   int           ptrace_errno;
   PTRACE_RETURN r;

   assert(get_lwp_id() != 0);

   // no PTRACE_GETREGS on PowerPC Linux 2.6.5
   for (int i = 0; i < 32; i++) {
      ptrace_errno = 0;
      r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                     PT_REGS_OFFSET(gpr[i], proc_->getAddressWidth()), 0,
                     &ptrace_errno, proc_->getAddressWidth(), __FILE__,
                     __LINE__);
      if ((r == -1) && ptrace_errno)
         error++;
      else
         regs->gprs.gpr[i] = r;
   }

   ptrace_errno = 0; 
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(nip, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.nip = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(msr, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.msr = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(ctr, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.ctr = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(link, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.link = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(xer, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.xer = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  PT_REGS_OFFSET(ccr, proc_->getAddressWidth()), 0,
                  &ptrace_errno, proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.ccr = r;

   if (error) {
      perror("dyn_lwp::getRegisters PTRACE_GETREGS" );
      return false;
   }

   if (includeFP) {
      // no PTRACE_GETFPREGS on PowerPC Linux 2.6.5

      size_t PEEKsPerFPR = sizeof(regs->fprs.fpr[0])
                           / SIZEOF_PTRACE_DATA(proc_->getAddressWidth());
      assert(PEEKsPerFPR * SIZEOF_PTRACE_DATA(proc_->getAddressWidth()) ==
             sizeof(regs->fprs.fpr[0]));

      for (int i = 0; i < 32; i++) {
         PTRACE_RETURN rs[PEEKsPerFPR];
         uint32_t      rs32[PEEKsPerFPR];
         for (size_t n = 0; n < PEEKsPerFPR; n++) {
            ptrace_errno = 0;
            r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                           (PT_FPR0 + i*PEEKsPerFPR + n)
                           * SIZEOF_PTRACE_DATA(proc_->getAddressWidth()),
                           0, &ptrace_errno, proc_->getAddressWidth(),
                           __FILE__, __LINE__);
            if ((r == -1) && ptrace_errno) {
               error++;
               break;
            }
            rs32[n] = rs[n] = r;
            if (n == PEEKsPerFPR - 1) {
              if (SIZEOF_PTRACE_DATA(proc_->getAddressWidth()) ==
                  sizeof(uint32_t))
                  memcpy(&regs->fprs.fpr[i], rs32, sizeof(regs->fprs.fpr[i]));
                else
                  memcpy(&regs->fprs.fpr[i], rs, sizeof(regs->fprs.fpr[i]));
            }
         }
      }

      ptrace_errno = 0;
      r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                     PT_FPSCR_OFFSET(proc_->getAddressWidth()), 0,
                     &ptrace_errno, proc_->getAddressWidth(), __FILE__,
                     __LINE__);
      if ((r == -1) && ptrace_errno)
         error++;
      else
         regs->fprs.fpscr = r;

      if (error) {
         perror("dyn_lwp::getRegisters PTRACE_GETFPREGS" );
         return false;
      }
   }

   return true;
}



Address dyn_lwp::readRegister(Register reg) {
   if(status() == running) {
      cerr << "    performance problem in call to dyn_lwp::readRegister\n"
           << "       successive pauses and continues with ptrace calls\n";
   }

   int ptrace_errno = 0;
   Address ret = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                            PT_REGS_OFFSET(gpr[reg], proc_->getAddressWidth()),
                            0, &ptrace_errno, proc_->getAddressWidth(),
                            __FILE__, __LINE__);
   return ret;
}


bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs, bool includeFP) {
   // Cycle through all registers, writing each from the
   // buffer with ptrace(PTRACE_POKEUSER ...

   bool retVal = true;
   int ptrace_errno = 0;
   int error = 0;

   assert(get_lwp_id() != 0);

   // no PTRACE_SETREGS on PowerPC Linux 2.6.5
   for (int i = 0; i < 32; i++) {
      error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                          PT_REGS_OFFSET(gpr[i], proc_->getAddressWidth()),
                          regs.gprs.gpr[i], &ptrace_errno,
                          proc_->getAddressWidth(), __FILE__, __LINE__);
   }

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(nip, proc_->getAddressWidth()),
                       regs.gprs.nip, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(msr, proc_->getAddressWidth()),
                       regs.gprs.msr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(ctr, proc_->getAddressWidth()),
                       regs.gprs.ctr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(link, proc_->getAddressWidth()),
                       regs.gprs.link, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(xer, proc_->getAddressWidth()),
                       regs.gprs.xer, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       PT_REGS_OFFSET(ccr, proc_->getAddressWidth()),
                       regs.gprs.ccr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   if (error) {
      perror("dyn_lwp::restoreRegisters PTRACE_SETREGS" );
      retVal = false;
   }
   error = 0;

   if (includeFP) {
      // no PTRACE_SETFPREGS on PowerPC Linux 2.6.5

      size_t POKEsPerFPR = sizeof(regs.fprs.fpr[0])
                           / SIZEOF_PTRACE_DATA(proc_->getAddressWidth());
      assert(POKEsPerFPR * SIZEOF_PTRACE_DATA(proc_->getAddressWidth()) ==
             sizeof(regs.fprs.fpr[0]));

      for (int i = 0; i < 32; i++) {
         PTRACE_RETURN ps[POKEsPerFPR];
         memcpy(ps, &regs.fprs.fpr[i], sizeof(regs.fprs.fpr[i]));
         if (SIZEOF_PTRACE_DATA(proc_->getAddressWidth()) == sizeof(uint32_t)) {
            uint32_t ps32[POKEsPerFPR];
            memcpy(ps32, &regs.fprs.fpr[i], sizeof(regs.fprs.fpr[i]));
            for (size_t n = 0; n < POKEsPerFPR; n++)
               ps[n] = ps32[n];
         }
         for (size_t n = 0; n < POKEsPerFPR; n++) {
            error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                                (PT_FPR0 + i*POKEsPerFPR + n)
                                * SIZEOF_PTRACE_DATA(proc_->getAddressWidth()),
                                ps[n], &ptrace_errno, proc_->getAddressWidth(),
                                __FILE__, __LINE__);
         }
      }

      error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                          PT_FPSCR_OFFSET(proc_->getAddressWidth()),
                          regs.fprs.fpscr, &ptrace_errno,
                          proc_->getAddressWidth(), __FILE__, __LINE__);

      if (error) {
         perror("dyn_lwp::restoreRegisters PTRACE_SETFPREGS" );
         retVal = false;
      }
   }

   return retVal;
}


Frame Frame::getCallerFrame()
{
// Matt's DynStackwalker will replace this

//Change this struct and add Vsyscall to see if mutatee is in a syscall
//--in a syscall, frame is in a different form
  typedef union {
      struct {
          uint32_t oldFp;
          uint32_t savedLR;
      } elf32;
      struct {
          uint64_t oldFp;
          uint64_t savedCR;
          uint64_t savedLR;
      } elf64;
  } linkArea_t;

  int savedLROffset;
  if (getProc()->getAddressWidth() == sizeof(uint64_t))
    savedLROffset = P_offsetof(linkArea_t, elf64.savedLR);
  else
    savedLROffset = P_offsetof(linkArea_t, elf32.savedLR);

  linkArea_t thisStackFrame;
  linkArea_t lastStackFrame;
  linkArea_t stackFrame;
  Address basePCAddr;

  Address newPC=0;
  Address newFP=0;
  Address newpcAddr=0;

  // Are we in a leaf function?
  bool isLeaf = false;
  bool noFrame = false;

  codeRange *range = getRange();
  int_function *func = range->is_function();

  if (uppermost_) {
    if (func) {
      //isLeaf = func->isLeafFunc();
      isLeaf = !func->savesReturnAddr();
      noFrame = func->hasNoStackFrame();
    }
  }

  // Get current stack frame link area
  if (getProc()->getAddressWidth() == sizeof(uint64_t)) {
    if (!getProc()->readDataSpace((caddr_t)fp_, sizeof(thisStackFrame.elf64),
                                  (caddr_t)&thisStackFrame.elf64, false))
      return Frame();
  }
  else {
    if (!getProc()->readDataSpace((caddr_t)fp_, sizeof(thisStackFrame.elf32),
                                  (caddr_t)&thisStackFrame.elf32, false))
      return Frame();
  }

  if (getProc()->getAddressWidth() == sizeof(uint64_t))
    getProc()->readDataSpace((caddr_t) (Address) thisStackFrame.elf64.oldFp,
                             sizeof(lastStackFrame.elf64),
                             (caddr_t) &lastStackFrame.elf64, false);
  else
    getProc()->readDataSpace((caddr_t) (Address) thisStackFrame.elf32.oldFp,
                             sizeof(lastStackFrame.elf32),
                             (caddr_t) &lastStackFrame.elf32, false);

  if (noFrame) {
    stackFrame = thisStackFrame;
    basePCAddr = fp_;
  }
  else {
    stackFrame = lastStackFrame;
    if (getProc()->getAddressWidth() == sizeof(uint64_t))
      basePCAddr = thisStackFrame.elf64.oldFp;
    else
      basePCAddr = thisStackFrame.elf32.oldFp;
  }

  // See if we're in instrumentation
  baseTrampInstance *bti = NULL;
  
  if (range->is_multitramp()) {
      bti = range->is_multitramp()->getBaseTrampInstanceByAddr(getPC());
      if (bti) {
          // If we're not in instru, then re-set this to NULL
          if (!bti->isInInstru(getPC()))
              bti = NULL;
      }
  }
  else if (range->is_minitramp()) {
      bti = range->is_minitramp()->baseTI;
  }
  if (bti) {
      // Oy. We saved the LR in the middle of the tramp; so pull it out
      // by hand.
      newpcAddr = fp_ + TRAMP_SPR_OFFSET + STK_LR;
      if (getProc()->getAddressWidth() == sizeof(uint64_t))
        newFP = thisStackFrame.elf64.oldFp;
      else
        newFP = thisStackFrame.elf32.oldFp;

      if (getProc()->getAddressWidth() == sizeof(uint64_t)) {
        if (!getProc()->readDataSpace((caddr_t) newpcAddr,
                                      sizeof(newPC), (caddr_t) &newPC, false))
          return Frame();
      }
      else {
        uint32_t u32;
        if (!getProc()->readDataSpace((caddr_t) newpcAddr,
                                      sizeof(u32), (caddr_t) &u32, false))
          return Frame();
        newPC = u32;
      }

      // Instrumentation makes its own frame; we want to skip the
      // function frame if there is one as well.
      instPoint *point = bti->baseT->instP();
      assert(point); // Will only be null if we're in an inferior RPC, which can't be.
      // If we're inside the function (callSite or arbitrary; bad assumption about
      // arbitrary but we don't know exactly where the frame was constructed) and the
      // function has a frame, tear it down as well.
      if ((point->getPointType() == callSite ||
          point->getPointType() == otherPoint) &&
          !point->func()->hasNoStackFrame()) {
        if (getProc()->getAddressWidth() == sizeof(uint64_t)) {
          if (!getProc()->readDataSpace((caddr_t) (Address)
                                        thisStackFrame.elf64.oldFp,
                                        sizeof(newFP),
                                        (caddr_t) &newFP, false))
            return Frame();
        }
        else {
          uint32_t u32;
          if (!getProc()->readDataSpace((caddr_t) (Address)
                                        thisStackFrame.elf32.oldFp,
                                        sizeof(u32), (caddr_t) &u32, false))
            return Frame();
          newFP = u32;
        }
      }
      // Otherwise must be at a reloc insn
  }
  else if (isLeaf) {
      // isLeaf: get the LR from the register instead of saved location on the stack
      if (lwp_ && lwp_->get_lwp_id()) {
          dyn_saved_regs regs;
          bool status = lwp_->getRegisters(&regs);
          if (! status) {
              return Frame();
          }
          newPC = regs.gprs.nip;
          newpcAddr = (Address) 1;
          // I'm using an address to signify a register
      }
      else if (thread_ && thread_->get_tid()) {
          cerr << "NOT IMPLEMENTED YET" << endl;
      }
      else { // normal
          dyn_saved_regs regs;
          bool status = getProc()->getRepresentativeLWP()->getRegisters(&regs);
          if (!status) {
              return Frame();
          }
          newPC = regs.gprs.nip;
          newpcAddr = (Address) 1;
      }


      if (noFrame)
          newFP = fp_;
      else
          if (getProc()->getAddressWidth() == sizeof(uint64_t))
              newFP = thisStackFrame.elf64.oldFp;
          else
              newFP = thisStackFrame.elf32.oldFp;
  }
  else {
      // Common case.
      if (getProc()->getAddressWidth() == sizeof(uint64_t))
        newPC = stackFrame.elf64.savedLR;
      else
        newPC = stackFrame.elf32.savedLR;
      newpcAddr = basePCAddr + savedLROffset;
      if (noFrame)
        newFP = fp_;
      else
        if (getProc()->getAddressWidth() == sizeof(uint64_t))
          newFP = thisStackFrame.elf64.oldFp;
        else
          newFP = thisStackFrame.elf32.oldFp;
  }

#ifdef DEBUG_STACKWALK
  fprintf(stderr, "PC %x, FP %x\n", newPC, newFP);
#endif
  return Frame(newPC, newFP, 0, newpcAddr, this);
}



bool Frame::setPC(Address newpc) {
   if (!pcAddr_)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   if (getProc()->getAddressWidth() == sizeof(uint64_t)) {
      uint64_t newpc64 = newpc;
      if (!getProc()->writeDataSpace((void*)pcAddr_, sizeof(newpc64), &newpc64))
         return false;
   }
   else {
      uint32_t newpc32 = newpc;
      if (!getProc()->writeDataSpace((void*)pcAddr_, sizeof(newpc32), &newpc32))
         return false;
   }
   pc_ = newpc;
   range_ = NULL;

   return true;
}

bool AddressSpace::getDyninstRTLibName() {
//full path to libdyninstAPI_RT (used an _m32 suffix for 32-bit version)
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        // Get env variable
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        }
        else {
            std::string msg = std::string("Environment variable ") +
                std::string("DYNINSTAPI_RT_LIB") +
               std::string(" has not been defined");
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
            // This will be incorrect if the RT library's version changes
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



bool process::handleTrapAtEntryPointOfMain(dyn_lwp *trappingLWP)
{
    assert(main_brk_addr);
    assert(trappingLWP);
    // restore original instruction

    if (!writeDataSpace((void *)main_brk_addr, instruction::size(),
                        (char *)savedCodeBuffer))
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
    // copied from aix.C
    int_function *f_main = NULL;
    pdvector<int_function *> funcs;
    bool res = findFuncsByPretty("main", funcs);
    if (!res) {
        // we can't instrument main - naim
        showErrorCallback(108,"main() uninstrumentable");
        return false;
    }

    if( funcs.size() > 1 ) {
      for (unsigned j = 0; j < funcs.size(); ++j) {
	funcs[j]->debugPrint();
      }

        cerr << __FILE__ << __LINE__
             << ": found more than one main! using the first" << endl;
    }
    f_main = funcs[0];
    assert(f_main);

    Address addr = f_main->getAddress();

    startup_printf("[%d]: inserting trap at 0x%x\n",
                   getPid(), addr);

    // save original instruction first
    readDataSpace((void *)addr, instruction::size(), savedCodeBuffer, true);
    // and now, insert trap
    codeGen gen(instruction::size());
    insnCodeGen::generateTrap(gen);

    assert(instruction::size() == gen.used());
    if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    main_brk_addr = addr;

    return true;
}

bool process::handleTrapAtLibcStartMain(dyn_lwp *)  { assert(0); }
bool process::instrumentLibcStartMain() { assert(0); }
bool process::decodeStartupSysCalls(EventRecord &) { assert(0); }
void process::setTraceSysCalls(bool) { assert(0); }
void process::setTraceState(traceState_t) { assert(0); }
bool process::getSysCallParameters(dyn_saved_regs *, long *, int) { assert(0); }
int process::getSysCallNumber(dyn_saved_regs *) { assert(0); }
long process::getSysCallReturnValue(dyn_saved_regs *) { assert(0); }
Address process::getSysCallProgramCounter(dyn_saved_regs *) { assert(0); }
bool process::isMmapSysCall(int) { assert(0); }
Offset process::getMmapLength(int, dyn_saved_regs *) { assert(0); }
Address process::getLibcStartMainParam(dyn_lwp *) { assert(0); }

bool process::loadDYNINSTlib()
{
    pdvector<int_function *> dlopen_funcs;

    if (findFuncsByAll(DL_OPEN_FUNC_EXPORTED, dlopen_funcs)) {
        return loadDYNINSTlib_exported();
    }
    else {
        return loadDYNINSTlib_hidden();
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

  delete savedRegs;
  savedRegs = NULL;
  return true;
}



bool process::loadDYNINSTlib_exported(const char *)
{
    // This functions was implemented by mixing parts of
    // process::loadDYNINSTlib() in aix.C and
    // process::loadDYNINSTlib_exported() in linux-x86.C.

    // dlopen takes two arguments:
    // const char *libname;
    // int mode;

    Address codeBase = findFunctionToHijack(this);
    if (!codeBase) {
        startup_cerr << "Couldn't find a point to insert dlopen call" << endl;
        return false;
    }
    Address dyninstlib_str_addr = 0;
    Address dlopen_call_addr = 0;

    pdvector<int_function *> dlopen_funcs;
    if (!findFuncsByAll(DL_OPEN_FUNC_EXPORTED, dlopen_funcs)) {
        startup_cerr << "Couldn't find method to load dynamic library" << endl;
        return false;
    }

    assert(dlopen_funcs.size() != 0);
    if (dlopen_funcs.size() > 1) {
        logLine("WARNING: More than one dlopen found, using the first\n");
    }
    //Address dlopen_addr = dlopen_funcs[0]->getAddress();
    int_function *dlopen_func = dlopen_funcs[0];  //aix.C

    // We now fill in the scratch code buffer with appropriate data
    codeGen scratchCodeBuffer(BYTES_TO_SAVE);
    assert(dyninstRT_name.length() < BYTES_TO_SAVE);
    scratchCodeBuffer.setAddrSpace(this);  //aix.C
    scratchCodeBuffer.setAddr(codeBase); //aix.C

    // The library name goes first
    dyninstlib_str_addr = codeBase;
    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

    // Need a register space                                           //aix.C
    // make sure this syncs with inst-power.C                          //aix.C
    registerSpace *dlopenRegSpace = registerSpace::savedRegSpace(this);//aix.C
    scratchCodeBuffer.setRegisterSpace(dlopenRegSpace);                //aix.C

#if defined(bug_syscall_changepc_rewind)
    //Fill in with NOPs, see loadDYNINSTlib_hidden
    scratchCodeBuffer.fill(getAddressWidth(), codeGen::cgNOP);
#endif

    // Now the real code
    dlopen_call_addr = codeBase + scratchCodeBuffer.used();

    pdvector<AstNodePtr> dlopenAstArgs(2);                         //aix.C
    AstNodePtr dlopenAst;                                          //aix.C
    dlopenAstArgs[0] = AstNode::operandNode(AstNode::Constant,     //aix.C
                                            (void*)dyninstlib_str_addr);
    dlopenAstArgs[1] = AstNode::operandNode(AstNode::Constant,     //aix.C
                                            (void*)DLOPEN_MODE);
    dlopenAst = AstNode::funcCallNode(dlopen_func, dlopenAstArgs); //aix.C

    // We need to push down the stack before we call this  //aix.C
    pushStack(scratchCodeBuffer);                          //aix.C
    dlopenAst->generateCode(scratchCodeBuffer, true);      //aix.C
    popStack(scratchCodeBuffer);                           //aix.C
 
    dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
    insnCodeGen::generateTrap(scratchCodeBuffer);

    if (!readDataSpace((void *)codeBase,
                       sizeof(savedCodeBuffer), savedCodeBuffer, true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
        return false;
    }

    if (!writeDataSpace((void *)(codeBase), scratchCodeBuffer.used(),
                        scratchCodeBuffer.start_ptr())) {
        fprintf(stderr, "%s[%d]:  writeDataSpace\n", __FILE__, __LINE__);
        return false;
    }

    // save registers
    dyn_lwp *lwp_to_use = NULL;
    if (process::IndependentLwpControl() && getRepresentativeLWP() == NULL)
        lwp_to_use = getInitialThread()->get_lwp();
    else
        lwp_to_use = getRepresentativeLWP();

    savedRegs = new dyn_saved_regs;
    bool status = lwp_to_use->getRegisters(savedRegs);

    assert((status != false) && (savedRegs != (void *)-1));

    if (!lwp_to_use->changePC(dlopen_call_addr,NULL))  {
        logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
        return false;
    }
    setBootstrapState(loadingRT_bs);
    return true;
}


bool process::loadDYNINSTlib_hidden() {
  // We need to make a call to do_dlopen to open our runtime library

  // This function was implemented by mixing parts of
  // process::loadDYNINSTlib() in aix.C and
  // process::loadDYNINSTlib_hidden() in linux-x86.C.

  startup_printf("**** LIBC21 dlopen for RT lib\n");

  // do_dlopen takes a pointer-to-struct argument. The struct is as follows:
  // const char *libname;
  // int mode;
  // void *result;
  // void *caller_addr;  (may be obsolete, but no harm leaving in)
  // Now, we have to put this somewhere writable. The idea is to
  // put it on the stack....

  union {
     struct {
        uint32_t libname;
        int32_t  mode;
        uint32_t result;
        uint32_t caller_addr;
     } elf32;
     struct {
        uint64_t libname;
        int32_t  mode;
        uint64_t result;
        uint64_t caller_addr;
     } elf64;
  } do_dlopen_struct;

  Address codeBase = findFunctionToHijack(this);

  if(!codeBase)
  {
      startup_cerr << "Couldn't find a point to insert dlopen call" << endl;
      return false;
  }

  startup_printf("(%d) writing in dlopen call at addr %p\n", getPid(),
                 (void *)codeBase);

  codeGen scratchCodeBuffer(BYTES_TO_SAVE);

  // Variables what we're filling in
  Address dyninstlib_str_addr = 0;
  Address dlopen_call_addr = 0;
  Address do_dlopen_struct_addr = 0;

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
        fprintf(stderr,"Failed to find _dl_open\n");
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
  int_function *dlopen_func = dlopen_funcs[0];  //aix.C

  assert(dyninstRT_name.length() < BYTES_TO_SAVE);
  startup_cerr << "Dyninst RT lib name: " << dyninstRT_name << endl;

  // We now fill in the scratch code buffer with appropriate data

  scratchCodeBuffer.setAddrSpace(this);  //aix.C
  scratchCodeBuffer.setAddr(codeBase); //aix.C

  // First copy the RT library name
  dyninstlib_str_addr = codeBase + scratchCodeBuffer.used();
  scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

  startup_printf("(%d) dyninst str addr at 0x%x\n", getPid(),
                                                    dyninstlib_str_addr);
  startup_printf("(%d) after copy, %d used\n", getPid(),
                                               scratchCodeBuffer.used());

  // Now we have enough to fill in the do_dlopen_struct
  if (getAddressWidth() == sizeof(uint64_t)) {
     do_dlopen_struct.elf64.libname     = dyninstlib_str_addr;
     do_dlopen_struct.elf64.mode        = DLOPEN_MODE;
     do_dlopen_struct.elf64.result      = 0;
     do_dlopen_struct.elf64.caller_addr = dlopen_addr;
  }
  else {
     do_dlopen_struct.elf32.libname     = dyninstlib_str_addr;
     do_dlopen_struct.elf32.mode        = DLOPEN_MODE;
     do_dlopen_struct.elf32.result      = 0;
     do_dlopen_struct.elf32.caller_addr = dlopen_addr;
  }

  // Need a register space                                           //aix.C
  // make sure this syncs with inst-power.C                          //aix.C
  registerSpace *dlopenRegSpace = registerSpace::savedRegSpace(this);//aix.C
  scratchCodeBuffer.setRegisterSpace(dlopenRegSpace);                //aix.C

  // Now we place the do_dlopen_struct on the mutatee's stack
  //   Step 1.  getRegisters() to get copy of mutatee's stack frame pointer
  //   Step 2.  decrement our local copy of mutatee's stack frame pointer
  //   Step 3.  restoreRegisters() to update mutatee's stack frame pointer
  //   Step 4.  increment our local copy of mutatee's stack frame pointer
  //              back to its original value so that we properly restore
  //              the mutatee's registers after the RT library is loaded
  //   Step 5.  writeDataSpace() to the stack our do_dlopen_struct

  //   Step 1.  getRegisters() to get copy of mutatee's stack frame pointer
  dyn_lwp *lwp_to_use = NULL;
  if(process::IndependentLwpControl() && getRepresentativeLWP() == NULL)
     lwp_to_use = getInitialThread()->get_lwp();
  else
     lwp_to_use = getRepresentativeLWP();

  savedRegs = new dyn_saved_regs;
  bool status = lwp_to_use->getRegisters(savedRegs);

  assert((status!=false) && (savedRegs!=(void *)-1));
   
  //   Step 2.  decrement our local copy of mutatee's stack frame pointer
  if (getAddressWidth() == sizeof(uint64_t))
     savedRegs->gprs.gpr[1] -= ALIGN_QUADWORD(STACKSKIP
                                              + sizeof(do_dlopen_struct.elf64));
  else
     savedRegs->gprs.gpr[1] -= ALIGN_QUADWORD(STACKSKIP
                                              + sizeof(do_dlopen_struct.elf32));
  do_dlopen_struct_addr = savedRegs->gprs.gpr[1];

  //   Step 3.  restoreRegisters() to update mutatee's stack frame pointer
  lwp_to_use->restoreRegisters(*savedRegs);

  //   Step 4.  increment our local copy of mutatee's stack frame pointer
  //              back to its original value so that we properly restore
  //              the mutatee's registers after the RT library is loaded
  if (getAddressWidth() == sizeof(uint64_t))
     savedRegs->gprs.gpr[1] += ALIGN_QUADWORD(STACKSKIP
                                              + sizeof(do_dlopen_struct.elf64));
  else
     savedRegs->gprs.gpr[1] += ALIGN_QUADWORD(STACKSKIP
                                              + sizeof(do_dlopen_struct.elf32));

  //   Step 5.  writeDataSpace() to the stack our do_dlopen_struct
  if (getAddressWidth() == sizeof(uint64_t))
     writeDataSpace((void *)(do_dlopen_struct_addr),
                    sizeof(do_dlopen_struct.elf64), &do_dlopen_struct.elf64);
  else
     writeDataSpace((void *)(do_dlopen_struct_addr),
                    sizeof(do_dlopen_struct.elf32), &do_dlopen_struct.elf32);


  // Now back to generating code for the call to do_dlopen ...

#if defined(bug_syscall_changepc_rewind)
  // Reported by SGI, during attach to a process in a system call:

  // Insert eight NOP instructions before the actual call to dlopen(). Loading
  // the runtime library when the mutatee was in a system call will sometimes
  // cause the process to (on IA32 anyway) execute the instruction four bytes
  // PREVIOUS to the PC we actually set here. No idea why. Prepending the
  // actual dlopen() call with eight NOP instructions insures this doesn't
  // really matter. Eight was selected rather than four because I don't know
  // if x86-64 does the same thing (and jumps eight bytes instead of four).

  // We will put in <addr width> rather than always 8; this will be 4 on x86 and  // 32-bit AMD64, and 8 on 64-bit AMD64.

  scratchCodeBuffer.fill(getAddressWidth(), codeGen::cgNOP);

  // And since we apparently execute at (addr - <width>), shift dlopen_call_addr  // up past the NOPs.
#endif

  dlopen_call_addr = codeBase + scratchCodeBuffer.used();

  pdvector<AstNodePtr> dlopenAstArgs(1);                         //aix.C
  AstNodePtr dlopenAst;                                          //aix.C
  dlopenAstArgs[0] = AstNode::operandNode(AstNode::Constant,     //aix.C
                                          (void*)do_dlopen_struct_addr);
  dlopenAst = AstNode::funcCallNode(dlopen_func, dlopenAstArgs); //aix.C

  startup_printf("(%d): emitting call from 0x%x to 0x%x\n",
                 getPid(), codeBase + scratchCodeBuffer.used(), dlopen_addr);

  // We need to push down the stack before we call this  //aix.C
  pushStack(scratchCodeBuffer);                          //aix.C
  dlopenAst->generateCode(scratchCodeBuffer, true);      //aix.C
  popStack(scratchCodeBuffer);                           //aix.C

  // And the break point
  dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
  insnCodeGen::generateTrap(scratchCodeBuffer);

  startup_printf("(%d) dyninst lib string addr at 0x%x\n", getPid(),
                                                           dyninstlib_str_addr);
  startup_printf("(%d) dyninst lib call addr at 0x%x\n", getPid(),
                                                         dlopen_call_addr);
  startup_printf("(%d) break address is at %p\n", getPid(),
                                                  (void *)dyninstlib_brk_addr);
  startup_printf("(%d) writing %d bytes\n", getPid(), scratchCodeBuffer.used());

  // savedCodeBuffer[BYTES_TO_SAVE] is declared in process.h
  // We can tighten this up if we record how much we saved

  if (!readDataSpace((void *)codeBase, sizeof(savedCodeBuffer),
                     savedCodeBuffer, true))
         fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);

  startup_printf("(%d) Writing from %p to %p\n", getPid(),
                 (char *)scratchCodeBuffer.start_ptr(), (char *)codeBase);
  writeDataSpace((void *)(codeBase), scratchCodeBuffer.used(),
                 scratchCodeBuffer.start_ptr());

  Address destPC = dlopen_call_addr;

  startup_printf("Changing PC to 0x%x\n", destPC);
  startup_printf("String at 0x%x\n", dyninstlib_str_addr);

  if (! lwp_to_use->changePC(destPC,NULL))
    {
      logLine("WARNING: changePC failed in dlopenDYNINSTlib\n");
      assert(0);
    }

  setBootstrapState(loadingRT_bs);
  return true;
}



Frame process::preStackWalkInit(Frame startFrame)
{
#if 0  // Matt's DynStackwalker will replace this; "quick and dirty" for now
  /* Do a special check for the vsyscall page.  Silently drop
     the page if it exists. */
  calcVSyscallFrame( this );

  Address next_pc = startFrame.getPC();
  if ((next_pc >= getVsyscallStart() && next_pc < getVsyscallEnd()) ||
      /* RH9 Hack */ (next_pc >= 0xffffe000 && next_pc < 0xfffff000)) {
     return startFrame.getCallerFrame();
  }
#endif
  return startFrame;
}


// floor of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_lo(const Address x) {
   const Address floor = getpagesize();

   assert(x >= floor);

   if ((x > floor) && (x - floor > getMaxBranch()))
      return x - getMaxBranch();

   return floor;
}


// floor of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_lo_64(const Address x) {
   const Address floor = getpagesize();

   assert(x >= floor);

   if ((x > floor) && (x - floor > getMaxBranch()))
      return x - getMaxBranch();

   return floor;
}


// ceiling of inferior malloc address range within a single branch of x
// for 32-bit ELF PowerPC mutatees
Address region_hi(const Address x) {
   const Address ceiling = ~(Address)0 & 0xffffffff;

   assert(x < ceiling);

   if ((x < ceiling) && (ceiling - x > getMaxBranch()))
      return x + getMaxBranch();

   return ceiling;
}


// ceiling of inferior malloc address range within a single branch of x
// for 64-bit ELF PowerPC mutatees
Address region_hi_64(const Address x) {
   const Address ceiling = ~(Address)0;

   assert(x < ceiling);

   if ((x < ceiling) && (ceiling - x > getMaxBranch()))
      return x + getMaxBranch();

   return ceiling;
}

