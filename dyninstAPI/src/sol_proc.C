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

// Solaris-style /proc support

// $Id: sol_proc.C,v 1.47 2004/03/11 22:20:40 bernat Exp $

#ifdef AIX_PROC
#include <sys/procfs.h>
#else
#include <procfs.h>
#endif
#include <limits.h>
#include <poll.h>
#include <sys/types.h>  // for reading lwps out of proc
#include <dirent.h>     // for reading lwps out of proc
#include "common/h/headers.h"
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/pathName.h" // for path name manipulation routines
#include "dyninstAPI/src/sol_proc.h"

/*
   osTraceMe is called after we fork a child process to set
   a breakpoint on the exit of the exec system call.
   When /proc is used, this breakpoint **will not** cause a SIGTRAP to 
   be sent to the process. The parent should use PIOCWSTOP to wait for 
   the child.
*/
// Interesting problem: I've seen a race condition occur when
// we set the proc trace flags from the mutator before the
// mutatee. In this case, DON'T reset the flags if there
// is a stop on exec in effect.

void OS::osTraceMe(void) {
    sysset_t *exitSet = SYSSET_ALLOC(getpid());
    int bufsize = SYSSET_SIZE(exitSet) + sizeof(long);

    char buf[bufsize];
    long *bufptr = (long *)buf;
    char procName[128];
    
    // Get the current set of syscalls
    pstatus_t status;
    sprintf(procName,"/proc/%d/status", (int) getpid());
    int stat_fd = P_open(procName, O_RDONLY, 0);
    if (pread(stat_fd, (void *)&status, sizeof(pstatus_t), 0) !=
        sizeof(pstatus_t)) {
        perror("osTraceMe::pread");
        return;
    }
#if defined(AIX_PROC) 
    if (status.pr_sysexit_offset)
        pread(stat_fd, exitSet, SYSSET_SIZE(exitSet), status.pr_sysexit_offset);
    else // No syscalls are being traced 
        premptysysset(exitSet);
#else
    memcpy(exitSet, &(status.pr_sysexit), SYSSET_SIZE(exitSet));
#endif
    close(stat_fd);

    sprintf(procName,"/proc/%d/ctl", (int) getpid());
    int fd = P_open(procName, O_WRONLY, 0);
    if (fd < 0) {
        perror("open");
        return;
    }
    
    /* set a breakpoint at the exit of exec/execve */
    if (SYSSET_MAP(SYS_exec, getpid()) != -1) {
        praddsysset(exitSet, SYSSET_MAP(SYS_exec, getpid()));
    }
    if (SYSSET_MAP(SYS_execve, getpid()) != -1) {
        praddsysset(exitSet, SYSSET_MAP(SYS_execve, getpid()));
    }
    // Write out the command
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, exitSet, SYSSET_SIZE(exitSet));
    if (write(fd, buf, bufsize) != bufsize) {
        perror("osTraceMe: PCSEXIT");
        P__exit(-1); // must use _exit here.
    }
    

    // AIX: do not close file descriptor or all changes are undone (?)
    // My guess is we need to fiddle with the run on close/kill on 
    // close bits. For now, leaving the FD open works.
    //close(fd);    
}
/*
 * DYN_LWP class
 * Operations on LWP file descriptors, or the representative LWP
 */

// determine if a process is running by doing low-level system checks, as
// opposed to checking the 'status_' member vrble.  May assume that attach()
// has run, but can't assume anything else.

bool dyn_lwp::isRunning() const {
   lwpstatus_t theStatus;
   
   if (!get_status(&theStatus)) return false;

   // Don't consider PR_ASLEEP to be stopped
   long stopped_flags = PR_STOPPED | PR_ISTOP;

   if (theStatus.pr_flags & stopped_flags)
      return false;
   else
       return true;

   
}

bool dyn_lwp::clearSignal() {
    long command[2];
    command[0] = PCRUN; command[1] = PRSTOP | PRCSIG;
    if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("clearSignal: PCRUN");
        return false;
    }
    command[0] = PCWSTOP;
    
    if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
        perror("clearSignal: PCWSTOP");
        return false;
    }
    return true;
}

// Get the process running again. May do one or more of the following:
// 1) Continue twice to clear a signal
// 2) Restart an aborted system call
bool dyn_lwp::continueLWP_(int signalToContinueWith) {
  lwpstatus_t status;
  long command[2];
  Address pc;  // PC at which we are trying to continue

  // Check the current status
  if (!get_status(&status)) {
      return false;
  }
  
  if ((0==status.pr_flags & PR_STOPPED) && (0==status.pr_flags & PR_ISTOP)) {
      return false;
  }


  // If the lwp is stopped on a signal we blip it (technical term).
  // The process will re-stop (since we re-run with PRSTOP). At
  // this point we continue it.
  if ((status.pr_flags & PR_STOPPED)
      && (status.pr_why == PR_SIGNALLED)) {
      if (status.pr_what == SIGSTOP || 
          status.pr_what == SIGINT ||
          status.pr_what == SIGTRAP) {
          clearSignal();
      }
  }
  command[0] = PCRUN;
  if (signalToContinueWith == dyn_lwp::NoSignal)
      command[1] = PRCSIG;  // clear the signal
  else {
      command[1] = 0;
  }
  pc = (Address)(GETREG_PC(status.pr_reg));
  // we don't want to operate on the process in this state
  ptraceOps++; 
  ptraceOtherOps++;

  if (! (stoppedInSyscall_ && pc == postsyscallpc_)) {
     // Continue the process the easy way
      if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
          perror("continueLWP: PCRUN2");
          return false;
      }
  } else {
      
      // We interrupted a sleeping system call at some previous pause
      // (i.e. stoppedInSyscall is true), we have not restarted that
      // system call yet, and the current PC is the insn following
      // the interrupted call.  It is time to restart the system
      // call.
      
      // Note that when we make the process runnable, we ignore
      // `flags', set if `hasNewPC' was true in the previous block,
      // because we don't want its PC; we want the PC of the system
      // call trap, which was saved in `syscallreg'.

      if (!restoreRegisters(*syscallreg_)) return false;
      delete syscallreg_;
      syscallreg_ = NULL;

      // We are done -- the process is in the kernel for the system
      // call, with the right registers values.  Make the process
      // runnable, restoring its previously blocked signals.
      // The signals were blocked as part of the abort mechanism.
      stoppedInSyscall_ = false;
      int bufsize = sizeof(long) + sizeof(proc_sigset_t);
      char buf[bufsize]; long *bufptr = (long *)buf;
      *bufptr = PCSHOLD; bufptr++;
      memcpy(bufptr, &sighold_, sizeof(proc_sigset_t));

      if (write(ctl_fd(), buf, bufsize) != bufsize) {
          perror("continueLWP: PCSHOLD"); return false;
      }

      long command[2];
      
      command[0] = PCRUN;
      command[1] = 0; // No arguments to PCRUN
      if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
          perror("continueLWP: PCRUN3"); return false;
      }
      
  }
  
  return true;
  
}

// Abort a system call. Place a trap at the exit of the syscall in
// question, and run with the ABORT flag set. Wait for the trap
// to be hit and then return.

bool dyn_lwp::abortSyscall()
{
    sysset_t* scexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedexit = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scentry = SYSSET_ALLOC(proc_->getPid());
    sysset_t* scsavedentry = SYSSET_ALLOC(proc_->getPid());
    lwpstatus_t status;

    // MT: aborting syscalls does not work. Maybe someone with better knowledge
    // of Solaris can get it working. 
    if(proc_->multithread_capable())
       return false;

    // We do not expect to recursively interrupt system calls.  We could
    // probably handle it by keeping a stack of system call state.  But
    // we haven't yet seen any reason to have this functionality.
    assert(!stoppedInSyscall_);
    stoppedInSyscall_ = true;
    
    if (!get_status(&status)) return false;
    
    // 1. Save the syscall number, registers, and blocked signals
    stoppedSyscall_ = status.pr_syscall;
    // Copy registers as is getRegisters
    syscallreg_ = new dyn_saved_regs;

    getRegisters(syscallreg_);
/*
    memcpy(&(syscallreg_->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(syscallreg_->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));
*/   
 
    memcpy(&sighold_, &status.pr_lwphold, sizeof(proc_sigset_t));
    
#ifdef i386_unknown_solaris2_5
    // From Roger A. Faulkner at Sun (email unknown), 6/29/1997:
    //
    // On Intel and PowerPC machines, the system call trap instruction
    // leaves the PC (program counter, instruction pointer) referring to
    // the instruction that follows the system call trap instruction.
    // On Sparc machines, the system call trap instruction leaves %pc
    // referring to the system call trap instruction itself (the
    // operating system increments %pc on exit from the system call).
    //
    // We have to reset the PC back to the system call trap instruction
    // on Intel and PowerPC machines.
    //
    // This is 7 on Intel, 4 on PowerPC.
    
    // Note: On x86/Linux this should probably be 2 bytes, because Linux
    // uses "int" to trap, not lcall.
    
    syscallreg_.theIntRegs[PC_REG] -= 7;
#endif
    
    // 2. Abort the system call
    
    // Save current syscall exit traps
    proc_->get_entry_syscalls(scsavedentry);
    proc_->get_exit_syscalls(scsavedexit);
    
    // Set process to trap on exit from this system call
    premptyset(scentry);
    premptyset(scexit);
    praddset(scexit, stoppedSyscall_);
    
    proc_->set_entry_syscalls(scentry);
    proc_->set_exit_syscalls(scexit);
    
    // Continue, aborting this system call and blocking all sigs except
    // those needed by DynInst.
    proc_sigset_t sigs;
    prfillset(&sigs);
    prdelset(&sigs, SIGTRAP);
    prdelset(&sigs, SIGILL);
    
    // Set the signals to "ignore" and run, aborting the syscall
    int bufsize = sizeof(long) + sizeof(proc_sigset_t);
    char buf[bufsize]; long *bufptr = (long *)buf;
    *bufptr = PCSHOLD; bufptr++;
    memcpy(bufptr, &sigs, sizeof(proc_sigset_t));
    
    long command[2];
    
    command[0] = PCRUN;
    command[1] = PRSABORT;
    
    if (write(ctl_fd(), buf, bufsize) != bufsize) {
        perror("abortSyscall: PCSHOLD"); return false;
    }
    if (write(ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("abortSyscall: PCRUN"); return false;
    }
    
    command[0] = PCWSTOP;
    if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
        perror("abortSyscall: PCWSTOP"); return false;
    }
    
    // Note: We assume that is always safe to restart the call after
    // aborting it.  We are wrong if it turns out that some
    // interruptible system call can make partial progress before we
    // abort it.
    // We think this is impossible because the proc manpage says EINTR
    // would be returned to the process if we didn't trap the syscall
    // exit, and the manpages for interruptible system calls say an
    // EINTR return value means no progress was made.
    // If we are wrong, this is probably the place to decide whether
    // and/or how the syscall should be restarted later.
    
    if (!get_status(&status)) return false;
    
    // Verify that we're stopped in the right place
    if (((status.pr_flags & (PR_STOPPED|PR_ISTOP))
         != (PR_STOPPED|PR_ISTOP))
        || status.pr_why != PR_SYSEXIT
        || status.pr_syscall != stoppedSyscall_) {
        sprintf(errorLine,
                "warn: Can't step paused process out of syscall (Verify)\n");
        logLine(errorLine);
        return 0;
    }
    proc_->set_entry_syscalls(scsavedentry);
    proc_->set_exit_syscalls(scsavedexit);
    
    // Remember the current PC.  When we continue the process at this PC
    // we will restart the system call.
    postsyscallpc_ = (Address) GETREG_PC(status.pr_reg);
    
    return true;
}

dyn_lwp *process::createRepresentativeLWP() {
   // don't register the representativeLWP in the lwps since it's not a true
   // lwp
   representativeLWP = createFictionalLWP(0);
   return representativeLWP;
}

// Stop the LWP in question
bool dyn_lwp::stop_() {
  long command[2];
  command[0] = PCSTOP;
  if (write(ctl_fd(), command, sizeof(long)) != sizeof(long)) {
      perror("pauseLWP: PCSTOP");
      return false;
  }

  return true;
}

// Get the active frame (PC, FP/SP)
Frame dyn_lwp::getActiveFrame()
{
   struct dyn_saved_regs regs;
   bool status = getRegisters(&regs);
   if(status == false)
      return Frame();
  
   Frame newFrame = Frame(GETREG_PC(regs.theIntRegs),
                          GETREG_FP(regs.theIntRegs), 
                          proc_->getPid(), NULL, this, true);

   return newFrame;
}

// Get the registers of the stopped thread and return them.
bool dyn_lwp::getRegisters_(struct dyn_saved_regs *regs)
{
    lwpstatus_t status;
    if (!get_status(&status)) return false;

    // Process must be stopped for register data to be correct.

    assert((status.pr_flags & PR_STOPPED) || (status.pr_flags & PR_ISTOP)
           || (status.pr_flags & PR_PCINVAL)  // eg. a trap at syscall exit
           );

    memcpy(&(regs->theIntRegs), &(status.pr_reg), sizeof(prgregset_t));
    memcpy(&(regs->theFpRegs), &(status.pr_fpreg), sizeof(prfpregset_t));

    return true;
}

#if !defined(BPATCH_LIBRARY)
struct lwprequest_info {
public:
   lwprequest_info() : lwp_id(0), rpc_id(0), 
                       completed(false), cancelled(false) 
   { }
   unsigned lwp_id;
   unsigned rpc_id;
   bool completed;
   bool cancelled;
};

template class pdvector<lwprequest_info*>;

pdvector<lwprequest_info*> rpcReqBuf;

void doneWithLwp(process *, unsigned rpc_id, void * /*data*/,
                 void *result_arg) {
   //int lwp_id = (int)data;
   int result = (int)result_arg;

   if(result == 1)
      return;  // successful

   // else, failure, mark as rpc cancelled ...
   // mark lwp initialization rpc as cancelled if when running, got tid of 0.
   // these lwps appear to not be the normal ones in the program but some
   // special lwps used by the thread library;
   pdvector<lwprequest_info*>::iterator iter = rpcReqBuf.begin();
   //bool any_active = false;

   while(iter != rpcReqBuf.end()) {
      lwprequest_info *rpcReq = (*iter);
      if(rpcReq->rpc_id == rpc_id) {
         rpcReq->cancelled = true;
         rpcReq->completed = false;
      }
      iter++;
   }      
}

// inferior RPC callback function type
typedef void(*inferiorRPCcallbackFunc)(process *p, unsigned rpcid, void *data,
                                       void *result);

// returns rpc_id
unsigned recognize_lwp(process *proc, int lwp_id) {
   dyn_lwp *lwp = proc->getLWP(lwp_id);

   pdvector<AstNode *> ast_args;
   AstNode *ast = new AstNode("DYNINSTregister_running_thread", ast_args);

   return proc->getRpcMgr()->postRPCtoDo(ast, true, doneWithLwp,
                                         (void *)lwp_id, false, NULL, lwp);
}

void determineLWPs(int pid, pdvector<unsigned> *all_lwps) {
   char procdir[128];
   sprintf(procdir, "/proc/%d/lwp", pid);
   DIR *dirhandle = opendir(procdir);
   struct dirent *direntry;
   while((direntry= readdir(dirhandle)) != NULL) {
      char str[100];
      strncpy(str, direntry->d_name, direntry->d_reclen);
      unsigned lwp_id = atoi(direntry->d_name);
      if(lwp_id != 0) // && lwp_id != 1)
         (*all_lwps).push_back(lwp_id);
   }
   closedir(dirhandle);
}

bool anyRpcsActive(process *proc, pdvector<lwprequest_info*> *rpcReqBuf) {
   pdvector<lwprequest_info*>::iterator iter = (*rpcReqBuf).begin();
   bool any_active = false;

   while(iter != (*rpcReqBuf).end()) {
      lwprequest_info *rpcReq = (*iter);
      if(rpcReq->completed || rpcReq->cancelled) {
         iter++;
         continue;
      }      

      int rpc_id = rpcReq->rpc_id;
      irpcState_t rpc_state = proc->getRpcMgr()->getRPCState(rpc_id);
      /*
      cerr << "  req: " << rpcReq->rpc_id << ", lwp: " << rpcReq->lwp_id
           << ", state: ";
      if(rpc_state == irpcNotValid) 
         cerr << "irpcNotValid\n";
      else if(rpc_state == irpcNotRunning)
         cerr << "irpcNotRunning\n";
      else if(rpc_state == irpcRunning)
         cerr << "irpcRunning\n";
      else if(rpc_state == irpcWaitingForSignal)
         cerr << "irpcWaitingForSignal\n";
      else if(rpc_state == irpcNotReadyForIRPC)
         cerr << "irpcNotReadyForIRPC\n";
      */

      if(rpc_state == irpcWaitingForSignal) {
          proc->getRpcMgr()->cancelRPC(rpc_id);
          rpcReq->cancelled = true;
      }
      else if(rpc_state == irpcNotValid) {   // ie. it's already completed
          rpcReq->completed = true;
      }
      else {
          any_active = true;
      }
      iter++;
   }
   return any_active;
}

void process::recognize_threads(pdvector<unsigned> *completed_lwps) {
   pdvector<unsigned> found_lwps;
   determineLWPs(getPid(), &found_lwps);

   rpcReqBuf.clear();
   for(unsigned i=0; i<found_lwps.size(); i++) {
      unsigned lwp_id = found_lwps[i];
      unsigned rpc_id = recognize_lwp(this, lwp_id);
      lwprequest_info *newReq = new lwprequest_info;
      newReq->lwp_id = lwp_id;
      newReq->rpc_id = rpc_id;
      rpcReqBuf.push_back(newReq);
   }

   do {
       getRpcMgr()->launchRPCs(false);
       if(hasExited())  return;
       getSH()->checkForAndHandleProcessEvents(false);
       // Loop until all rpcs are done
   } while(anyRpcsActive(this, &rpcReqBuf));
   
   pdvector<lwprequest_info*>::iterator iter = rpcReqBuf.begin();
   while(iter != rpcReqBuf.end()) {
      lwprequest_info *rpcReq = (*iter);
      if(rpcReq->completed) {
         (*completed_lwps).push_back(rpcReq->lwp_id);
      }
      iter++;
      delete rpcReq;
   }
}
#endif


// Restore registers saved as above.
bool dyn_lwp::restoreRegisters_(const struct dyn_saved_regs &regs)
{
    lwpstatus_t status;
    get_status(&status);
    
    // The fact that this routine can be shared between solaris/sparc and
    // solaris/x86 is just really, really cool.  /proc rules!
    int regbufsize = sizeof(long) + sizeof(prgregset_t);
    char regbuf[regbufsize]; long *regbufptr = (long *)regbuf;
    *regbufptr = PCSREG; regbufptr++;
    memcpy(regbufptr, &(regs.theIntRegs), sizeof(prgregset_t));
    int writesize;
    writesize = write(ctl_fd(), regbuf, regbufsize);
    
    if (writesize != regbufsize) {
        perror("restoreRegisters: GPR write");
        return false;
    }
    int fpbufsize = sizeof(long) + sizeof(prfpregset_t);
    char fpbuf[fpbufsize]; long *fpbufptr = (long *)fpbuf;
    *fpbufptr = PCSFPREG; fpbufptr++;
    memcpy(fpbufptr, &(regs.theFpRegs), sizeof(prfpregset_t));
    
    if (write(ctl_fd(), fpbuf, fpbufsize) != fpbufsize) {
        perror("restoreRegisters FPR write");
        return false;
    }
    return true;

}

// Determine if the LWP in question is in a system call.
bool dyn_lwp::executingSystemCall()
{
  lwpstatus_t status;
  if (!get_status(&status)) return false;
  // Old-style
  if (status.pr_syscall > 0 && // If we're in a system call
      status.pr_why != PR_SYSEXIT) {
      stoppedSyscall_ = status.pr_syscall;
      if (abortSyscall()) {
          // Not in a syscall any more :)
          return false;
      }
      else {
          return true;
      }
  }
  
#if defined(AIX_PROC)
  if (status.pr_why == PR_SIGNALLED &&
      status.pr_what == SIGSTOP) {
      // We can't operate on a process in SIGSTOP, so clear it
      proc()->getRepresentativeLWP()->clearSignal();
  }
  
  // I've seen cases where we're apparently not in a system
  // call, but can't write registers... we'll label this case
  // a syscall for now
  int regbufsize = sizeof(long) + sizeof(prgregset_t);
  char regbuf[regbufsize]; long *regbufptr = (long *)regbuf;
  *regbufptr = PCSREG; regbufptr++;
  memcpy(regbufptr, &(status.pr_reg), sizeof(prgregset_t));
  int writesize = write(ctl_fd(), regbuf, regbufsize);
  if (writesize == -1) {
      return true;
  }
#endif
  return false;
}


bool dyn_lwp::changePC(Address addr, struct dyn_saved_regs *regs)
{
    // Don't change the contents of regs if given
    dyn_saved_regs local;
    if (!regs) {
        getRegisters(&local);
    } else {
        memcpy(&local, regs, sizeof(struct dyn_saved_regs));
    }

    // Compatibility: we don't use PCSVADDR on Solaris because AIX doesn't 
    // support it
    // nPC MUST be set before PC! On platforms that don't have it,
    // nPC will be overwritten by the PC write
    GETREG_nPC(local.theIntRegs) = addr + sizeof(instruction);
    GETREG_PC(local.theIntRegs) = addr;

    if (!restoreRegisters(local))
        return false;
    
    dyn_saved_regs check;
    getRegisters(&check);
    assert(GETREG_PC(local.theIntRegs) == GETREG_PC(check.theIntRegs));

    return true;
}

#if defined(i386_unknown_solaris2_5)
bool process::changeIntReg(int reg, Address val) {
   assert(status_ == stopped); // /proc will require this

   prgregset_t theIntRegs;
   dyn_lwp *replwp = getRepresentativeLWP();
   if (ioctl(replwp->get_fd(), PIOCGREG, &theIntRegs) == -1) {
      perror("dyn_lwp::changeIntReg");
      if (errno == EBUSY) {
         cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
         assert(false);
      }
      return false;
   }

   theIntRegs[reg] = val;

   if (ioctl(replwp->get_fd(), PIOCSREG, &theIntRegs) == -1) {
      perror("process::changeIntReg PIOCSREG");
      if (errno == EBUSY) {
         cerr << "It appears that the process wasn't stopped in the eyes of /proc" << endl;
         assert(false);
      }
      return false;
   }

   return true;
}
#endif

// Utility function: get the appropriate lwpstatus_t struct
bool dyn_lwp::get_status(lwpstatus_t *status) const
{
    if(!is_attached()) {
        return false;
    }
    
   if (lwp_id_) {
      // We're using an lwp file descriptor, so get directly
      if (pread(status_fd_, 
                (void *)status, 
                sizeof(lwpstatus_t), 0) != sizeof(lwpstatus_t)) {
          // When we fork a LWP file might disappear.
          if (errno != ENOENT) {
              perror("dyn_lwp::get_status");            
          }
         return false;
      }
   }
   else {
      // No lwp, so we get the whole thing and pick it out
      pstatus_t procstatus;
      if (!proc_->get_status(&procstatus)) {
         return false;
      }
      memcpy(status, &(procstatus.pr_lwp), sizeof(lwpstatus_t));
   }
   return true;
}

// Read the value of a particular register
Address dyn_lwp::readRegister(Register reg)
{
    Address result;
    dyn_saved_regs regs;
    getRegisters(&regs);
    result = GETREG_GPR(regs.theIntRegs, reg);
    return result;
}

bool dyn_lwp::realLWP_attach_() {
   char temp[128];
   sprintf(temp, "/proc/%d/lwp/%d/lwpctl", (int)proc_->getPid(), lwp_id_);
   ctl_fd_ = P_open(temp, O_WRONLY, 0);
   if (ctl_fd_ < 0) perror("Opening lwpctl");
   sprintf(temp, "/proc/%d/lwp/%d/lwpstatus", (int)proc_->getPid(),lwp_id_);
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) perror("Opening lwpstatus");
#if !defined(AIX_PROC)
   sprintf(temp, "/proc/%d/lwp/%d/lwpusage", (int)proc_->getPid(), lwp_id_);
   usage_fd_ = P_open(temp, O_RDONLY, 0);
#else
   usage_fd_ = 0;
#endif
   return true;
}

bool dyn_lwp::representativeLWP_attach_() {
   /*
     Open the /proc file corresponding to process pid
   */

   char temp[128];
   // Open the process-wise handles
   sprintf(temp, "/proc/%d/ctl", getPid());
   ctl_fd_ = P_open(temp, O_WRONLY | O_EXCL, 0);
   if (ctl_fd_ < 0) perror("Opening (LWP) ctl");

   sprintf(temp, "/proc/%d/status", getPid());
   status_fd_ = P_open(temp, O_RDONLY, 0);    
   if (status_fd_ < 0) perror("Opening (LWP) status");

#if !defined(AIX_PROC)
   sprintf(temp, "/proc/%d/usage", getPid());
   usage_fd_ = P_open(temp, O_RDONLY, 0);    
   if (usage_fd_ < 0) perror("Opening (LWP) usage");
#else
   usage_fd_ = 0;
#endif

   as_fd_ = -1;
   sprintf(temp, "/proc/%d/as", getPid());
   as_fd_ = P_open(temp, O_RDWR, 0);
   if (as_fd_ < 0) perror("Opening as fd");

#if !defined(AIX_PROC)
   sprintf(temp, "/proc/%d/auxv", getPid());
   auxv_fd_ = P_open(temp, O_RDONLY, 0);
   if (auxv_fd_ < 0) perror("Opening auxv fd");
#else
   // AIX doesn't have the auxv file
   auxv_fd_ = 0;
#endif

   sprintf(temp, "/proc/%d/map", getPid());
   map_fd_ = P_open(temp, O_RDONLY, 0);
   if (map_fd_ < 0) perror("map fd");

   sprintf(temp, "/proc/%d/psinfo", getPid());
   ps_fd_ = P_open(temp, O_RDONLY, 0);
   if (ps_fd_ < 0) perror("Opening ps fd");

   sprintf(temp, "/proc/%d/status", getPid());
   status_fd_ = P_open(temp, O_RDONLY, 0);
   if (status_fd_ < 0) perror("Opening status fd");

   lwpstatus_t status;

   // special for aix: we grab the status and clear the STOP
   // signal (if any)
   is_attached_ = true;
   get_status(&status);

   if (status.pr_why == PR_SIGNALLED &&
       status.pr_what == SIGSTOP) {
       clearSignal();
   }

   return true;
}

// Close FDs opened above
void dyn_lwp::realLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
      
   // First we want to clear any signal which might be pending
   // for the process. Otherwise, when we detach the process has
   // a signal sent that it may have no clue about
   // But we can't do that here -- all FDs associated with the process
   // are closed.
   close(ctl_fd_);
   close(status_fd_);
   close(usage_fd_);
   is_attached_ = false;
}

// Close FDs opened above
void dyn_lwp::representativeLWP_detach_()
{
   assert(is_attached());  // dyn_lwp::detach() shouldn't call us otherwise
      
   // First we want to clear any signal which might be pending
   // for the process. Otherwise, when we detach the process has
   // a signal sent that it may have no clue about
   // But we can't do that here -- all FDs associated with the process
   // are closed.
   close(ctl_fd_);
   close(status_fd_);
   close(usage_fd_);
   close(as_fd_);
   close(ps_fd_);
   close(status_fd_);
   is_attached_ = false;
}

/*
 * Process-wide /proc operations
 */

bool process::setProcessFlags()
{
    long command[2];

    // Unset all flags
    command[0] = PCUNSET;
    command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

    dyn_lwp *replwp = getRepresentativeLWP();
    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("installProcessFlags: PRUNSET");
        return false;
    }
    command[0] = PCSET;

    long flags = PR_BPTADJ | PR_MSACCT | PR_FORK;

    command[1] = flags;

    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("installProcessFlags: PCSET");
        return false;
    }

   // step 2) /proc PIOCSTRACE: define which signals should be
   // forwarded to daemon These are (1) SIGSTOP and (2) either
   // SIGTRAP (sparc) or SIGILL (x86), to implement inferiorRPC
   // completion detection.
   // Also detect SIGCONT so that we know if a user has restarted
   // an application.
  
   proc_sigset_t sigs;
    
   premptyset(&sigs);
   praddset(&sigs, SIGSTOP);
    
   praddset(&sigs, SIGTRAP);
    
   praddset(&sigs, SIGCONT);
   praddset(&sigs, SIGBUS);
   praddset(&sigs, SIGSEGV);
   praddset(&sigs, SIGILL);
    
   int bufsize = sizeof(long) + sizeof(proc_sigset_t);
   char buf[bufsize]; 
   long *bufptr = (long *) buf;
   *bufptr = PCSTRACE;
   bufptr++;
   memcpy(bufptr, &sigs, sizeof(proc_sigset_t));

   if(write(replwp->ctl_fd(), buf, bufsize) != bufsize) {
      perror("attach: PCSTRACE");
      return false;
   }


    return true;
}

bool process::unsetProcessFlags()
{
    long command[2];

    if (!isAttached()) return false;
    // Unset all flags
    command[0] = PCUNSET;
    command[1] = PR_BPTADJ | PR_MSACCT | PR_RLC | PR_KLC | PR_FORK;

    dyn_lwp *replwp = getRepresentativeLWP();
    if (write(replwp->ctl_fd(), command, 2*sizeof(long)) != 2*sizeof(long)) {
        perror("unsetProcessFlags: PRUNSET");
        return false;
    }

    proc_sigset_t sigs;
    premptyset(&sigs);
    int sigbufsize = sizeof(long) + sizeof(proc_sigset_t);
    char sigbuf[sigbufsize]; long *sigbufptr = (long *)sigbuf;

    sigbufptr = (long *)sigbuf;
    *sigbufptr = PCSTRACE;
    if (write(replwp->ctl_fd(), sigbuf, sigbufsize) != sigbufsize) {
        perror("unsetProcessFlags: PCSTRACE");
        return false;
    }
    
    return true;
}

// AIX requires us to re-open the process-wide handles after
// an exec call

#if defined(AIX_PROC)
void dyn_lwp::reopen_fds() {
        // Reopen the process-wide handles
    char temp[128];

    close(as_fd_);
    sprintf(temp, "/proc/%d/as", getPid());
    as_fd_ = P_open(temp, O_RDWR, 0);
    if (as_fd_ <= 0) perror("Opening as fd");
#if !defined(AIX_PROC)
    close(auxv_fd_);
    sprintf(temp, "/proc/%d/auxv", getPid());
    auxv_fd_ = P_open(temp, O_RDONLY, 0);
    if (auxv_fd_ <= 0) perror("Opening auxv fd");
#else
    // AIX doesn't have the auxv file
    auxv_fd_ = 0;
#endif

    close(map_fd_);
    sprintf(temp, "/proc/%d/map", getPid());
    map_fd_ = P_open(temp, O_RDONLY, 0);
    if (map_fd_ <= 0) perror("map fd");

    close(ps_fd_);
    sprintf(temp, "/proc/%d/psinfo", getPid());
    ps_fd_ = P_open(temp, O_RDONLY, 0);
    if (ps_fd_ <= 0) perror("Opening ps fd");

    close(status_fd_);
    sprintf(temp, "/proc/%d/status", getPid());
    status_fd_ = P_open(temp, O_RDONLY, 0);
    if (status_fd_ <= 0) perror("Opening status fd");
}
#endif

bool process::isRunning_() const {
    // We can key off the representative LWP
    return getRepresentativeLWP()->isRunning();
}

/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    // these next two lines are a hack used to get the poll call initiated
    // by checkForAndHandleProcessEvents() in process::terminateProc to
    // still check process for events if it was previously stopped
    if(status() == stopped)
       status_ = running;

    long command[2];
    command[0] = PCKILL;
    command[1] = SIGKILL;
    dyn_lwp *cntl_lwp = getRepresentativeLWP();
    if (cntl_lwp) 
        if (write(cntl_lwp->ctl_fd(), 
                  command, 2*sizeof(long)) != 2*sizeof(long)) {
            perror("terminateProc: PCKILL");
            return false;
        }

    return true;
}

bool dyn_lwp::waitUntilStopped() {
   return true;
}

bool process::waitUntilStopped() {
   return true;
}

bool dyn_lwp::writeTextWord(caddr_t inTraced, int data) {
   //  cerr << "writeTextWord @ " << (void *)inTraced << endl; cerr.flush();
   return writeDataSpace(inTraced, sizeof(int), (caddr_t) &data);
}

bool dyn_lwp::writeTextSpace(void *inTraced, u_int amount, const void *inSelf)
{
   //  cerr << "writeTextSpace pid=" << getPid() << ", @ " << (void *)inTraced
   //       << " len=" << amount << endl; cerr.flush();
   return writeDataSpace(inTraced, amount, inSelf);
}

bool dyn_lwp::readTextSpace(void *inTraced, u_int amount, const void *inSelf) {
   return readDataSpace(inTraced, amount, const_cast<void *>(inSelf));
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int amount, const void *inSelf)
{
   ptraceOps++; ptraceBytes += amount;

   //  cerr << "process::writeDataSpace_ pid " << getPid() << " writing "
   //       << amount << " bytes at loc " << inTraced << endl;
#if defined(BPATCH_LIBRARY)
#if defined (sparc_sun_solaris2_4)
	if(proc_->collectSaveWorldData &&  ((Address) inTraced) >
      proc_->getDyn()->getlowestSObaseaddr() )
   {
		shared_object *sh_obj = NULL;
		bool result = false;
		for(unsigned i=0;
          proc_->shared_objects && !result && i<proc_->shared_objects->size();
          i++)
      {
			sh_obj = (*proc_->shared_objects)[i];
			result = sh_obj->isinText((Address) inTraced);
		}
		if( result  ){
         /*	printf(" write at %lx in %s amount %x insn: %x \n", 
				(off_t)inTraced, sh_obj->getName().c_str(), amount,
            *(unsigned int*) inSelf);
            */	
			sh_obj->setDirty();	
		}
	}
#endif
#endif
   off64_t loc;
   // Problem: we may be getting a address with the high bit
   // set. So how to convince the system that it's not negative?
   loc = (off64_t) ((unsigned) inTraced);

   int written = pwrite64(as_fd(), inSelf, amount, loc);
   if(written != (int)amount) {
      perror("writeDataSpace");
      return false;
   }
    
   return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int amount, void *inSelf) {
    ptraceOps++; ptraceBytes += amount;

    off64_t loc;
    loc = (off64_t) ((unsigned) inTraced);

    if(pread64(as_fd(), inSelf, amount, loc) != (int)amount) {
        perror("readDataSpace");
        fprintf(stderr, "From 0x%x (mutator) to 0x%x (mutatee), %d bytes\n",
                (int)inSelf, (int)inTraced, amount);
        return false;
    }
    return true;
}


bool process::get_status(pstatus_t *status) const
{
    if (!isAttached()) return false;
    if (!getRepresentativeLWP()->is_attached()) return false;
   
    if(pread(getRepresentativeLWP()->status_fd(), (void *)status,
             sizeof(pstatus_t), 0) != sizeof(pstatus_t)) {
        perror("process::get_status");
        return false;
    }
    
    return true;
}

bool process::set_entry_syscalls(sysset_t *entry)
{
    if (!isAttached()) return false;
    
    int bufentrysize = sizeof(long) + SYSSET_SIZE(entry);
    char bufentry[bufentrysize]; long *bufptr = (long *)bufentry;
    
    // Write entry syscalls
    *bufptr = PCSENTRY; bufptr++;
    memcpy(bufptr, entry, SYSSET_SIZE(entry));

    dyn_lwp *replwp = getRepresentativeLWP();        
    if (write(replwp->ctl_fd(), bufentry, bufentrysize) != bufentrysize)
       return false;

    return true;    
}

bool process::set_exit_syscalls(sysset_t *exit)
{
    if (!isAttached()) return false;
    
    int bufexitsize = sizeof(long) + SYSSET_SIZE(exit);
    char bufexit[bufexitsize]; long *bufptr = (long *)bufexit;
    *bufptr = PCSEXIT; bufptr++;
    memcpy(bufptr, exit, SYSSET_SIZE(exit));

    dyn_lwp *replwp = getRepresentativeLWP();        
    if (write(replwp->ctl_fd(), bufexit, bufexitsize) != bufexitsize)
       return false;
    return true;    
}

#if defined(AIX_PROC)
extern void generateBreakPoint(instruction &insn);
#endif

syscallTrap *process::trapSyscallExitInternal(Address syscall)
{
    syscallTrap *trappedSyscall = NULL;

    // First, the cross-platform bit. If we're already trapping
    // on this syscall, then increment the reference counter
    // and return

    for (unsigned iter = 0; iter < syscallTraps_.size(); iter++) {
        if (syscallTraps_[iter]->syscall_id == syscall) {
            trappedSyscall = syscallTraps_[iter];
            break;
        }
    }

    if (trappedSyscall) {
        // That was easy...
        trappedSyscall->refcount++;
        //fprintf(stderr, "Bumping refcount for syscall %d to %d\n",
        //        (int)trappedSyscall->syscall_id, trappedSyscall->refcount);
        return trappedSyscall;
    }
    else {
        // Okay, we haven't trapped this system call yet.
        // Things to do:
        // 1) Get the original value
        // 2) Place a trap
        // 3) Create a new syscallTrap object and return it
        trappedSyscall = new syscallTrap;
        trappedSyscall->refcount = 1;
        trappedSyscall->syscall_id = (int) syscall;

        //fprintf(stderr, "  trapping syscall %d for 1st time (%d)\n",
        //       (int)trappedSyscall->syscall_id, trappedSyscall->refcount);
        
#if defined(AIX_PROC) 
        // AIX does some weird things, as we can't modify a thread
        // at a system call exit -- this means that using /proc
        // doesn't get us anywhere.
        // The "address" of the system call is actually the LWP
        // in the call
        dyn_lwp *syslwp = getLWP((unsigned) syscall);
        Frame frame = syslwp->getActiveFrame();
        Frame callerFrame = frame.getCallerFrame(this);
        
        Address origLR;
        readDataSpace((void *)(callerFrame.getFP() + 8),
                      sizeof(Address),
                      (void *)&origLR, false);
        trappedSyscall->origLR = origLR;
        
        Address trapAddr = inferiorMalloc(sizeof(instruction));
        trappedSyscall->trapAddr = trapAddr;
        instruction insn;
        generateBreakPoint(insn);

        writeDataSpace((void *)trapAddr, sizeof(instruction),
                       (void *)&insn);
        writeDataSpace((void *)(callerFrame.getFP() + 8), 
                       sizeof(Address),
                       (void *)&trapAddr);
#else
        sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());

        if (!get_exit_syscalls(cur_syscalls)) return NULL;
        if (prissyssetmember(cur_syscalls, trappedSyscall->syscall_id))
            // Surprising case... but it's possible as we trap fork and exec
            trappedSyscall->orig_setting = 1;
        else
            trappedSyscall->orig_setting = 0;

        // 2) Place a trap
        praddsysset(cur_syscalls, trappedSyscall->syscall_id);
        if (!set_exit_syscalls(cur_syscalls)) return false;
        
        //fprintf(stderr, "PCSexit for %d, orig %d\n", 
        //trappedSyscall->syscall_id, trappedSyscall->orig_setting);
#endif
        // Insert into the list of trapped syscalls
        syscallTraps_ += (trappedSyscall);

        return trappedSyscall;
    }
    // Should never reach here.
    return NULL;
}

bool process::clearSyscallTrapInternal(syscallTrap *trappedSyscall) {
    // Decrement the reference count, and if it's 0 remove the trapped
    // system call
    assert(trappedSyscall->refcount > 0);

    trappedSyscall->refcount--;
    if (trappedSyscall->refcount > 0) {
        //fprintf(stderr, "Refcount on syscall %d reduced to %d\n",
        //        (int)trappedSyscall->syscall_id,
        //        trappedSyscall->refcount);
        return true;
    }
    // Erk... it hit 0. Remove the trap at the system call
#if defined(AIX_PROC) 
    dyn_lwp *lwp = getLWP(trappedSyscall->syscall_id);
    inferiorFree(trappedSyscall->trapAddr);
    lwp->changePC(trappedSyscall->origLR, NULL);
#else
    sysset_t *cur_syscalls = SYSSET_ALLOC(getPid());

    if (!get_exit_syscalls(cur_syscalls)) return false;
    if (!trappedSyscall->orig_setting) {
        prdelsysset(cur_syscalls, trappedSyscall->syscall_id);
    }
    
    if (!set_exit_syscalls(cur_syscalls)) return false;
    

#endif
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

Address dyn_lwp::getCurrentSyscall() {
    
    // Return the system call we're currently in
    lwpstatus_t status;
    if (!get_status(&status)) {
        fprintf(stderr, "Failed to get status\n");
        return 0;
    }
#if defined(AIX_PROC)
    return get_lwp_id();
#else
    return status.pr_syscall;
#endif
}



bool dyn_lwp::stepPastSyscallTrap()
{
#if defined(AIX_PROC)
    // What is something else doing hitting our trap?
    assert(0);
#endif
    // Nice... on /proc systems we don't need to do anything here
    return true;
}

// Check if a LWP has hit a trap we inserted to catch the exit of a
// system call trap.
// Return values:
// 0: did not trap at the exit of a syscall
// 1: Trapped, but did not have a syscall trap placed for this LWP
// 2: Trapped with a syscall trap desired.
int dyn_lwp::hasReachedSyscallTrap() {
#if defined(AIX_PROC) 
    if (!trappedSyscall_) return 0;
    
    Frame frame = getActiveFrame();
    
    if (frame.getPC() == trappedSyscall_->trapAddr)
        return 2;
#else
    Address syscall;
    // Get the status of the LWP
    lwpstatus_t status;
    if (!get_status(&status)) {
        fprintf(stderr, "Failed to get thread status\n");
        return 0;
    }
    
    if (status.pr_why != PR_SYSEXIT) {
        return 0;
    }
    syscall = status.pr_what;
    
    if (trappedSyscall_ && syscall == trappedSyscall_->syscall_id) {
        return 2;
    }
    // Unfortunately we can't check a recorded system call trap,
    // since we don't have one saved. So do a scan through all traps the
    // process placed
    if (proc()->checkTrappedSyscallsInternal(syscall))
        return 1;
#endif

    // This is probably an error case... but I'm not sure.
    return 0;
}

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall)
{
   int pid = p->getPid();

    if (syscall == SYSSET_MAP(SYS_fork, pid) ||
        syscall == SYSSET_MAP(SYS_fork1, pid) ||
        syscall == SYSSET_MAP(SYS_vfork, pid))
        return procSysFork;
    if (syscall == SYSSET_MAP(SYS_exec, pid) || 
        syscall == SYSSET_MAP(SYS_execve, pid))
        return procSysExec;
    if (syscall == SYSSET_MAP(SYS_exit, pid))
        return procSysExit;


    return procSysOther;
}



int decodeProcStatus(process *,
                     lwpstatus_t status,
                     procSignalWhy_t &why,
                     procSignalWhat_t &what,
                     procSignalInfo_t &info) {
   info = GETREG_INFO(status.pr_reg);

   switch (status.pr_why) {
     case PR_SIGNALLED:
        why = procSignalled;
        what = status.pr_what;
        break;
     case PR_SYSENTRY:
        why = procSyscallEntry;
        what = status.pr_what;
        
#if defined(AIX_PROC)
        // We actually pull from the syscall argument vector
        if (status.pr_nsysarg > 0)
           info = status.pr_sysarg[0];
        else
           info = 0;
#endif
        break;
     case PR_SYSEXIT:
        why = procSyscallExit;
        what = status.pr_what;
        
#if defined(AIX_PROC)
        // This from the proc header file: system returns are
        // left in pr_sysarg[0]. NOT IN MAN PAGE.
        info = status.pr_sysarg[0];
#endif
        break;
     case PR_REQUESTED:
        // We don't expect PR_REQUESTED in the signal handler
        assert(0 && "PR_REQUESTED not handled");
#if defined(PR_SUSPENDED)
     case PR_SUSPENDED:
        // I'm seeing this state at times with a forking multi-threaded
        // child process, currently handling by just continuing the process
        why = procSuspended;
        break;
#endif
     case PR_JOBCONTROL:
     case PR_FAULTED:
     default:
        assert(0);
        break;
   }
   
   return 1;
}

int showProcStatus(lwpstatus_t status)
{
   switch (status.pr_why) {
     case PR_SIGNALLED:
        cerr << "PR_SIGNALED, what: " << (int)status.pr_what << endl;
        break;
     case PR_SYSENTRY:
        cerr << "PR_SYSENTRY, what: " << (int)status.pr_what << endl;
        break;
     case PR_SYSEXIT:
        cerr << "PR_SYSEXIT, what: " << (int)status.pr_what << endl;
        break;
     case PR_REQUESTED:
        cerr << "PR_REQUESTED\n";
        break;
#if defined(PR_SUSPENDED)
     case PR_SUSPENDED:
        // I'm seeing this state at times with a forking multi-threaded
        // child process, currently handling by just continuing the process
        cerr << "PR_SUSPENDED\n";
        break;
#endif
     case PR_JOBCONTROL:
     case PR_FAULTED:
     default:
        cerr << "OTHER\n";
        assert(0);
        break;
   }
   return 1;
}

procevent *find_matching_event(const pdvector<procevent *> &events,
                               process *proc, procSignalWhy_t  why,
                               procSignalWhat_t what) {
   procevent *matching_ev = NULL;
   for(unsigned i=0; i<events.size(); i++) {
      procevent *cur_event = events[i];
      if(cur_event->proc == proc && cur_event->why == why &&
         cur_event->what == what) {
         // assume that there's at most one matching event
         assert(matching_ev == NULL);
         matching_ev = cur_event;
      }
   }

   return matching_ev;
}
                               
// returns true if updated events structure for this lwp 
bool updateEventsWithLwpStatus(process *curProc, dyn_lwp *lwp,
                               pdvector<procevent *> *events)
{

   lwpstatus_t lwpstatus;
   bool res = lwp->get_status(&lwpstatus);

   if(res == false) {
      return false;
   }

   // This is the "why" for the lwps that were stopped because some other
   // lwp stopped for an interesting reason.  We don't care about lwps
   // that stopped for this reason.
   if(lwpstatus.pr_why == PR_REQUESTED) {
      return false;
   }
   
   procSignalWhy_t  why  = procUndefined;
   procSignalWhat_t what = 0;
   procSignalInfo_t info = 0;

   if(!decodeProcStatus(curProc, lwpstatus, why, what, info))
      return false;
   
   procevent *matching_event =
      find_matching_event(*events, curProc, why, what);

   if(matching_event == NULL) {
      procevent *new_event = new procevent;
      new_event->proc = curProc;
      new_event->lwp  = lwp;
      new_event->why  = why;
      new_event->what = what;
      new_event->info = info;
      (*events).push_back(new_event);
   } else {
      matching_event->lwp = lwp;
   }
   
   return true;
}

void fillInPollEvents(struct pollfd fds, process *curProc,
                      pdvector<procevent *> *events)
{
   if (fds.revents & POLLHUP) {
      procevent *new_event = new procevent;
      new_event->proc = curProc;
      new_event->lwp = curProc->getRepresentativeLWP();
      
      // True if the process exited out from under us
      int status;
      int ret;
      do {
          ret = waitpid(curProc->getPid(), &status, 0);
      } while ((ret < 0) && (errno == EINTR));
      if (ret < 0) {
          // This means that the application exited, but was not our child
          // so it didn't wait around for us to get it's return code.  In
          // this case, we can't know why it exited or what it's return
          // code was.
          ret = curProc->getPid();
          status = 0;
          // is this the bug??
          // processVec[curr]->continueProc_();
      }
      
      decodeWaitPidStatus(curProc, status, &new_event->why, &new_event->what);
      (*events).push_back(new_event);
      return;
   }

   lwpstatus_t procstatus;
   if(! curProc->getRepresentativeLWP()->get_status(&procstatus))
      return;
   int lwp_to_use = procstatus.pr_lwpid;

   // copied from old code, must not care about events that don't stop proc
   if(! (procstatus.pr_flags & PR_STOPPED || procstatus.pr_flags & PR_ISTOP) )
      return;

   dictionary_hash_iter<unsigned, dyn_lwp *> lwp_iter(curProc->real_lwps);
   dyn_lwp *cur_lwp;
   unsigned index;
   dyn_lwp *replwp = curProc->getRepresentativeLWP();
   int numreal_lwps = curProc->real_lwps.size();

   bool added_an_event = false;
   if(numreal_lwps == 0)
      added_an_event = updateEventsWithLwpStatus(curProc, replwp, events);
   else {
      while (lwp_iter.next(index, cur_lwp)) {
         if(cur_lwp->get_lwp_id() != lwp_to_use)
            continue;
         if(updateEventsWithLwpStatus(curProc, cur_lwp, events))
            added_an_event = true;
      }
   }

   /*
   if(added_an_event == false) {
      assert(false);
   }
   */
}

#if defined(bug_aix_proc_broken_fork)

int decodeRTSignal(process *proc,
                   procSignalWhy_t &why,
                   procSignalWhat_t &what,
                   procSignalInfo_t &info)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   if (!proc) return 0;

   pdstring status_str = pdstring("DYNINST_instSyscallState");
   pdstring arg_str = pdstring("DYNINST_instSyscallArg1");

   int status;
   Address arg;

   bool err = false;
   Address status_addr = proc->findInternalAddress(status_str, false, err);
   if (err) {
      // Couldn't find symbol
      return 0;
   }

   if (!proc->readDataSpace((void *)status_addr, sizeof(int), 
                            &status, true)) {
      return 0;
   }

   if (status == 0) {
      return 0; // Nothing to see here
   }

   Address arg_addr = proc->findInternalAddress(arg_str, false, err);
   if (err) {
      return 0;
   }
    
   if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                            &arg, true))
      assert(0);
   info = (procSignalInfo_t)arg;
   switch(status) {
     case 1:
        /* Entry to fork */
        why = procSyscallEntry;
        what = SYS_fork;
        break;
     case 2:
        why = procSyscallExit;
        what = SYSSET_MAP(SYS_fork, proc->getPid());
        break;
     case 3:
        /* Entry to exec */
        why = procSyscallEntry;
        what = SYS_exec;
        break;
     case 4:
        /* Exit of exec, unused */
        break;
     case 5:
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        why = procSyscallEntry;
        what = SYS_exit;
        break;
     default:
        assert(0);
        break;
   }
   return 1;

}
#endif

void specialHandlingOfEvents(const pdvector<procevent *> &events) {
   for(unsigned i=0; i<events.size(); i++) {
      procevent *cur_event = events[i];
      process *cur_proc = cur_event->proc;

#if defined(os_aix)
      if (cur_event->why == procSignalled && cur_event->what == SIGSTOP) {
         // On AIX we can't manipulate a process stopped on a
         // SIGSTOP... in any case, we clear it.
         // No other signal exhibits this behavior.
         cur_proc->getRepresentativeLWP()->clearSignal();
      }
#endif

#if defined(bug_aix_proc_broken_fork)
      if (cur_event->why == procSignalled && cur_event->what == SIGSTOP) {
          // Possibly a fork stop in the RT library
          decodeRTSignal(cur_proc, cur_event->why, cur_event->what, cur_event->info);
      }
#endif

      if(cur_event->lwp != NULL) {
         // not necessary now that have lwp info in the event info
         if(cur_event->why == procSyscallExit && cur_event->what == 2) {
            cur_proc->setLWPStoppedFromForkExit(
                                        cur_event->lwp->get_lwp_id());
         }
      }
   }
}      

// Get and decode a signal for a process
// We poll /proc for process events, and so it's possible that
// we'll get multiple hits. In this case we return one and queue
// the rest. Further calls of decodeProcessEvent will burn through
// the queue until there are none left, then re-poll.
// Return value: 0 if nothing happened, or process pointer

// the pertinantLWP and wait_options are ignored on Solaris, AIX

bool signalHandler::checkForProcessEvents(pdvector<procevent *> *events,
                                          int wait_arg, bool block)
{
   extern pdvector<process*> processVec;
   bool any_active_procs = false;
   struct pollfd fds[OPEN_MAX];  // argument for poll
   int num_fds_to_watch = processVec.size();

   for(unsigned u = 0; u < processVec.size(); u++) {
       process *lproc = processVec[u];
       
       if(lproc && (lproc->status() == running || lproc->status() == neonatal))
       {
           if (wait_arg == -1 || lproc->getPid() == wait_arg) {
               fds[u].fd = lproc->getRepresentativeLWP()->status_fd();
               any_active_procs = true;
           } else {
               fds[u].fd = -1;
           }                
       } else {
           fds[u].fd = -1;
       }	
       fds[u].events = POLLPRI;
       fds[u].revents = 0;
   }
   
   if(any_active_procs == false) {
       return false;
   }
   
   int timeout;
   if (block) timeout = -1;
   else timeout = 0;
   
   int num_selected_fds = poll(fds, num_fds_to_watch, timeout);
   if (num_selected_fds <= 0) {
       if (num_selected_fds < 0) {
           perror("checkForProcessEvents: poll failed");
       }
       return false;
   }
   //handled_fds = 0; handled_fds < num_selected_fds; ) {
   int handled_fds = 0;
   for(int i=0; i<num_fds_to_watch; i++) {
       if(fds[i].revents == 0)
           continue;
       
       handled_fds++;  // going to handle this now
       process *curProc = processVec[i];
       curProc->set_status(stopped);
       fillInPollEvents(fds[i], curProc, events);
   }
   assert(num_selected_fds == handled_fds);
   
   specialHandlingOfEvents(*events);
   if((*events).size()) {
       return true;
   } else
       return false;
}

pdstring process::tryToFindExecutable(const pdstring &/*iprogpath*/, int pid) {
    return pdstring("/proc/") + pdstring(pid) + pdstring("/object/a.out");
}




