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

// $Id: linux-power.C,v 1.3 2007/07/02 16:45:49 ssuen Exp $

#include <dlfcn.h>

#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/linux-power.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/process.h"

#define DLOPEN_MODE (RTLD_NOW | RTLD_GLOBAL)

const char DL_OPEN_FUNC_EXPORTED[] = "dlopen";
const char DL_OPEN_FUNC_INTERNAL[] = "_dl_open";
const char DL_OPEN_FUNC_NAME[] = "do_dlopen";

#define P_offsetof(s, m) (Address) &(((s *) NULL)->m)


void calcVSyscallFrame(process *p)
{
  assert(0);  //sunlung
  void *result;
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
     p->setVsyscallData(NULL);
     return;
  }

  /**
   * Read the location of the vsyscall page from /proc/.
   **/
  p->readAuxvInfo();
  if (p->getVsyscallStatus() != vsys_found) {
     p->setVsyscallRange(0x0, 0x0);
     p->setVsyscallData(NULL);
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
           p->setVsyscallData(NULL);
           p->setVsyscallStatus(vsys_notfound);
           return;
        }
     }
  }

  if (!isVsyscallData(buffer, dso_size)) {
     p->setVsyscallRange(0x0, 0x0);
     p->setVsyscallData(NULL);
     p->setVsyscallStatus(vsys_notfound);
     return;
  }
  getVSyscallSignalSyms(buffer, dso_size, p);
  result = parseVsyscallPage(buffer, dso_size, p);
  p->setVsyscallData(result);
*/
  return;
}


bool dyn_lwp::changePC(Address loc,
                       struct dyn_saved_regs */*ignored registers*/)
{
   Address regaddr = P_offsetof(struct pt_regs, PTRACE_REG_IP);
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
                   P_offsetof(struct pt_regs, PTRACE_REG_FP), 0, &ptrace_errno,
                   proc_->getAddressWidth(), __FILE__, __LINE__);
   if (ptrace_errno) return Frame();

   // next instruction pointer
   pc = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                   P_offsetof(struct pt_regs, PTRACE_REG_IP), 0, &ptrace_errno,
                   proc_->getAddressWidth(), __FILE__, __LINE__);
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
                     P_offsetof(struct pt_regs, gpr[i]), 0, &ptrace_errno,
                     proc_->getAddressWidth(), __FILE__, __LINE__);
      if ((r == -1) && ptrace_errno)
         error++;
      else
         regs->gprs.gpr[i] = r;
   }

   ptrace_errno = 0; 
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, nip), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.nip = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, msr), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.msr = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, ctr), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.ctr = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, link), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.link = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, xer), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
   if ((r == -1) && ptrace_errno)
      error++;
   else
      regs->gprs.xer = r;

   ptrace_errno = 0;
   r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                  P_offsetof(struct pt_regs, ccr), 0, &ptrace_errno,
                  proc_->getAddressWidth(), __FILE__, __LINE__);
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

      size_t PEEKsPerFPR = sizeof(regs->fprs.fpr[0]) / sizeof(PTRACE_RETURN);
      assert(PEEKsPerFPR * sizeof(PTRACE_RETURN) == sizeof(regs->fprs.fpr[0]));

      for (int i = 0; i < 32; i++) {
         PTRACE_RETURN rs[PEEKsPerFPR];
         for (size_t n = 0; n < PEEKsPerFPR; n++) {
            ptrace_errno = 0;
            rs[n] = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                               (PT_FPR0 + i*PEEKsPerFPR + n)
                               * sizeof(PTRACE_RETURN), 0, &ptrace_errno,
                               proc_->getAddressWidth(), __FILE__, __LINE__);
            if ((rs[n] == -1) && ptrace_errno) {
               error++;
               break;
            }
            if (n == PEEKsPerFPR - 1)
              memcpy(&regs->fprs.fpr[i], rs, sizeof(regs->fprs.fpr[i]));
         }
      }

      ptrace_errno = 0;
      r = DBI_ptrace(PTRACE_PEEKUSER, get_lwp_id(),
                     PT_FPSCR * sizeof(PTRACE_RETURN), 0, &ptrace_errno,
                     proc_->getAddressWidth(), __FILE__, __LINE__);
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
                            P_offsetof(struct pt_regs, gpr[reg]), 0,
                            &ptrace_errno, proc_->getAddressWidth(),
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
                          P_offsetof(struct pt_regs, gpr[i]),
                          regs.gprs.gpr[i], &ptrace_errno,
                          proc_->getAddressWidth(), __FILE__, __LINE__);
   }

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, nip),
                       regs.gprs.nip, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, msr),
                       regs.gprs.msr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, ctr),
                       regs.gprs.ctr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, link),
                       regs.gprs.link, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, xer),
                       regs.gprs.xer, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                       P_offsetof(struct pt_regs, ccr),
                       regs.gprs.ccr, &ptrace_errno,
                       proc_->getAddressWidth(), __FILE__, __LINE__);

   if (error) {
      perror("dyn_lwp::restoreRegisters PTRACE_SETREGS" );
      retVal = false;
   }
   error = 0;

   if (includeFP) {
      // no PTRACE_SETFPREGS on PowerPC Linux 2.6.5

      size_t POKEsPerFPR = sizeof(regs.fprs.fpr[0]) / sizeof(PTRACE_RETURN);
      assert(POKEsPerFPR * sizeof(PTRACE_RETURN) == sizeof(regs.fprs.fpr[0]));

      for (int i = 0; i < 32; i++) {
         PTRACE_RETURN ps[POKEsPerFPR];
         memcpy(ps, &regs.fprs.fpr[i], sizeof(regs.fprs.fpr[i]));
         for (size_t n = 0; n < POKEsPerFPR; n++) {
            error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                                (PT_FPR0 + i*POKEsPerFPR + n)
                                * sizeof(PTRACE_RETURN), ps[n], &ptrace_errno,
                                proc_->getAddressWidth(), __FILE__, __LINE__);
         }
      }

      error += DBI_ptrace(PTRACE_POKEUSER, get_lwp_id(),
                          PT_FPSCR * sizeof(PTRACE_RETURN),
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
assert(0);  //sunlung
//from AIX version  Maybe move to stackwalk-power.C?

//Change this struct and add Vsyscall to see if mutatee is in a syscall
//--in a syscall, frame is in a different form
  typedef struct {
    unsigned oldFp;
    unsigned savedCR;
    unsigned savedLR;
    unsigned compilerInfo;
    unsigned binderInfo;
    unsigned savedTOC;
  } linkArea_t;

  const int savedLROffset=8; //Needed to be modified for ppc64_linux
  //const int compilerInfoOffset=12;

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
      isLeaf = func->makesNoCalls();
      noFrame = func->hasNoStackFrame();
    }
  }

  // Get current stack frame link area
  if (!getProc()->readDataSpace((caddr_t)fp_, sizeof(linkArea_t),
                        (caddr_t)&thisStackFrame, false))
    return Frame();
  getProc()->readDataSpace((caddr_t) thisStackFrame.oldFp, sizeof(linkArea_t),
                           (caddr_t) &lastStackFrame, false);

  if (noFrame) {
    stackFrame = thisStackFrame;
    basePCAddr = fp_;
  }
  else {
    stackFrame = lastStackFrame;
    basePCAddr = thisStackFrame.oldFp;
  }

  // See if we're in instrumentation
  baseTrampInstance *bti = NULL;
/*
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
      newFP = thisStackFrame.oldFp;

      if (!getProc()->readDataSpace((caddr_t) newpcAddr,
                                    sizeof(Address),
                                    (caddr_t) &newPC, false))
          return Frame();

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
          if (!getProc()->readDataSpace((caddr_t) thisStackFrame.oldFp,
                                        sizeof(unsigned),
                                        (caddr_t) &newFP, false))
              return Frame();
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
          newPC = regs.theIntRegs.__lr;
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
          newPC = regs.theIntRegs.__lr;
          newpcAddr = (Address) 1;
      }


      if (noFrame)
          newFP = fp_;
      else
          newFP = thisStackFrame.oldFp;
  }
  else {
      // Common case.
      newPC = stackFrame.savedLR;
      newpcAddr = basePCAddr + savedLROffset;
      if (noFrame)
        newFP = fp_;
      else
        newFP = thisStackFrame.oldFp;
  }

#ifdef DEBUG_STACKWALK
  fprintf(stderr, "PC %x, FP %x\n", newPC, newFP);
#endif
*/return Frame(newPC, newFP, 0, newpcAddr, this);
}



bool Frame::setPC(Address newpc) {
assert(0);  //sunlung
   if (!pcAddr_)
   {
       //fprintf(stderr, "[%s:%u] - Frame::setPC aborted", __FILE__, __LINE__);
      return false;
   }

   //fprintf(stderr, "[%s:%u] - Frame::setPC setting %x to %x",
   //__FILE__, __LINE__, pcAddr_, newpc);
   getProc()->writeDataSpace((void*)pcAddr_, sizeof(Address), &newpc);
   pc_ = newpc;
   range_ = NULL;

   return false;
}

bool process::getDyninstRTLibName() {
//full path to libdyninstAPI_RT (used an _m32 suffix for 32-bit version)
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
                            + pdstring(getPid());
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
    instruction::generateTrap(gen);

    if (!writeDataSpace((void *)addr, gen.used(), gen.start_ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    main_brk_addr = addr;

    return true;
}



bool process::loadDYNINSTlib()
{
assert(0);  //sunlung
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
assert(0); //sunlung
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



bool process::loadDYNINSTlib_exported()
{
assert(0);  //sunlung
//use AIX version for the Power instruction generation
//use Linux version for the actual dlopen interface
/*  // dlopen takes two arguments:
    // const char *libname;
    // int mode;
    // We put the library name on the stack, push the args, and
    // emit the call

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
    Address dlopen_addr = dlopen_funcs[0]->getAddress();

    // We now fill in the scratch code buffer with appropriate data
    codeGen scratchCodeBuffer(BYTES_TO_SAVE);
    assert(dyninstRT_name.length() < BYTES_TO_SAVE);
    // The library name goes first
    dyninstlib_str_addr = codeBase;
    scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

    //Fill in with NOPs, see loadDYNINSTlib_hidden
    scratchCodeBuffer.fill(getAddressWidth(), codeGen::cgNOP);

    // Now the real code
    dlopen_call_addr = codeBase + scratchCodeBuffer.used();

    bool mode64bit = (getAddressWidth() == sizeof(uint64_t));
    if (!mode64bit) {
        // Push mode
        // emitPushImm(DLOPEN_MODE, scratchCodeBuffer);

        // Push string addr
        // emitPushImm(dyninstlib_str_addr, scratchCodeBuffer);

        instruction::generateCall(scratchCodeBuffer,
                                  scratchCodeBuffer.used() + codeBase,
                                  dlopen_addr);

        // And the break point
        dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
        instruction::generateTrap(scratchCodeBuffer);
    }
    else {
        // Set mode
        emitMovImmToReg64(REGNUM_RSI, DLOPEN_MODE, false, scratchCodeBuffer);
        // Set string addr
        emitMovImmToReg64(REGNUM_RDI, dyninstlib_str_addr, true,
                          scratchCodeBuffer);
        // The call (must be done through a register in order to reach)
        emitMovImmToReg64(REGNUM_RAX, dlopen_addr, true, scratchCodeBuffer);
        emitSimpleInsn(0xff, scratchCodeBuffer);
        emitSimpleInsn(0xd0, scratchCodeBuffer);

        // And the break point
        dyninstlib_brk_addr = codeBase + scratchCodeBuffer.used();
        instruction::generateTrap(scratchCodeBuffer);
    }

    if (!readDataSpace((void *)codeBase,
                       sizeof(savedCodeBuffer), savedCodeBuffer, true)) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
        return false;
    }

    if (!writeDataSpace((void *)(codeBase), scratchCodeBuffer.used(),
                        scratchCodeBuffer.start_ptr())) {
        fprintf(stderr, "%s[%d]:  readDataSpace\n", __FILE__, __LINE__);
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
*/  return true;
}


bool process::loadDYNINSTlib_hidden() {
assert(0);  //sunlung
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
  Address mprotect_call_addr = 0;

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

  assert(dyninstRT_name.length() < BYTES_TO_SAVE);
  // We now fill in the scratch code buffer with appropriate data
  startup_cerr << "Dyninst RT lib name: " << dyninstRT_name << endl;

  dyninstlib_str_addr = codeBase + scratchCodeBuffer.used();
  scratchCodeBuffer.copy(dyninstRT_name.c_str(), dyninstRT_name.length()+1);

  startup_printf("(%d) dyninst str addr at 0x%x\n", getPid(), dyninstlib_str_addr);

  startup_printf("(%d) after copy, %d used\n", getPid(), scratchCodeBuffer.used());

  // Reported by SGI, during attach to a process in a system call:

  // Insert eight NOP instructions before the actual call to dlopen(). Loading
  // the runtime library when the mutatee was in a system call will sometimes
  // cause the process to (on IA32 anyway) execute the instruction four bytes
  // PREVIOUS to the PC we actually set here. No idea why. Prepending the
  // actual dlopen() call with eight NOP instructions insures this doesn't
  // really matter. Eight was selected rather than four because I don't know
  // if x86-64 does the same thing (and jumps eight bytes instead of four).

  // We will put in <addr width> rather than always 8; this will be 4 on x86 and  // 32-bit AMD64, and 8 on 64-bit AMD64.

  scratchCodeBuffer.fill(getAddressWidth(),
                         codeGen::cgNOP);
  // And since we apparently execute at (addr - <width>), shift dlopen_call_addr  // up past the NOPs.
  dlopen_call_addr = codeBase + scratchCodeBuffer.used();


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

  if(!( mprotect_call_addr = tryUnprotectStack(scratchCodeBuffer,codeBase) )) {
    startup_printf("Failed to disable stack protection.\n");
  }

#if defined(arch_x86_64)
  if (getAddressWidth() == 4) {
#endif

      // Push caller
      // emitPushImm(dlopen_addr, scratchCodeBuffer);

      // Push hole for result
      // emitPushImm(0, scratchCodeBuffer);

      // Push mode
      // emitPushImm(DLOPEN_MODE, scratchCodeBuffer);

      // Push string addr
      // emitPushImm(dyninstlib_str_addr, scratchCodeBuffer);

      // Push the addr of the struct: esp
      // emitSimpleInsn(PUSHESP, scratchCodeBuffer);

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

  if(mprotect_call_addr != 0) {
    startup_printf("(%d) mprotect call addr at 0x%lx\n", getPid(), mprotect_call_addr);
  }
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

    Address destPC;
    if(mprotect_call_addr != 0)
        destPC = mprotect_call_addr;
    else
        destPC = dlopen_call_addr;

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



Frame process::preStackWalkInit(Frame startFrame)
{
assert(0);  //sunlung
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


Address process::tryUnprotectStack(codeGen &buf, Address codeBase) {
assert(0);  //sunlung
    // find variable __stack_prot

    // mprotect READ/WRITE __stack_prot
    pdvector<int_variable *> vars;
    pdvector<int_function *> funcs;

    Address var_addr;
    Address func_addr;
    Address ret_addr;
    int size;
    int pagesize;
    int page_start;
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
      // emitPushImm(func_addr, buf);

      // Push mode (READ|WRITE|EXECUTE)
      // emitPushImm(7, buf);

      // Push variable size
      // emitPushImm(size, buf);

      // Push variable location
      // emitPushImm(page_start, buf);

      startup_printf("(%d): emitting call for mprotect from 0x%x to 0x%x\n",
                     getPid(), codeBase + buf.used(), func_addr);
      instruction::generateCall(buf, buf.used() + codeBase, func_addr);
#if defined(arch_x86_64)
  } else {
      // Push caller
      emitMovImmToReg64(REGNUM_RAX, func_addr, true, buf);
      emitSimpleInsn(0x50, buf); // push %rax

      // Push mode (READ|WRITE|EXECUTE)
      emitMovImmToReg64(REGNUM_EAX, 7, true, buf); //32-bit mov
      emitSimpleInsn(0x50, buf); // push %rax

      // Push variable size
      emitMovImmToReg64(REGNUM_EAX, size, true, buf); //32-bit mov
      emitSimpleInsn(0x50, buf); // push %rax

      // Push variable location
      emitMovImmToReg64(REGNUM_RAX, page_start, true, buf);
      emitSimpleInsn(0x50, buf); // push %rax

      // The call (must be done through a register in order to reach)
      emitMovImmToReg64(REGNUM_RAX, func_addr, true, buf);
      emitSimpleInsn(0xff, buf); // group 5
      emitSimpleInsn(0xd0, buf); // mod=11, ext_op=2 (call Ev), r/m=0 (RAX)
  }
#endif

    return ret_addr;
}

