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

// $Id: unix.C,v 1.147 2005/11/21 17:16:14 jaw Exp $

#include "common/h/headers.h"
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debuggerinterface.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/callbacks.h"


#ifndef BPATCH_LIBRARY
#include "paradynd/src/main.h"  // for "tp" ?
#include "paradynd/src/pd_process.h" // for class pd_process
#endif

// the following are needed for handleSigChild
#include "dyninstAPI/src/signalhandler.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/stats.h"

// BREAK_POINT_INSN
#if defined(AIX_PROC)
#include "dyninstAPI/src/arch-power.h"
#endif
#include "dyninstAPI/src/sol_proc.h"

#include <sys/poll.h>

SignalGeneratorUnix *global_sh = NULL;

SignalGeneratorUnix *getSH() {
   if(global_sh == NULL) {
      signal_printf("%s[%d]:  about to create new SignalHandler\n", FILE__, __LINE__);
      global_sh = new SignalGeneratorUnix();
      global_sh->createThread();
   }

   return global_sh;
}

SignalHandler *SignalGeneratorUnix::newSignalHandler(char *name, int id)
{
  SignalHandlerUnix *sh;
  sh  = new SignalHandlerUnix(name, id);
  return (SignalHandler *)sh;
}

/////////////////////////////////////////////////////////////////////////////
/// Massive amounts of signal handling code
/////////////////////////////////////////////////////////////////////////////


// Turn the return result of waitpid into something we can use
bool decodeWaitPidStatus(procWaitpidStatus_t status,
                        EventRecord &ev) {
    // Big if-then-else tree
    if (WIFEXITED(status)) {
        ev.type = evtProcessExit;
        ev.status = statusNormal;
        ev.what = (eventWhat_t) (unsigned int) WEXITSTATUS(status);
        //ev.proc->set_status(exited);
        return true;
    } 
    else if (WIFSIGNALED(status)) {
        ev.type = evtProcessExit;
        ev.status = statusSignalled;
        ev.what = (eventWhat_t) (unsigned int) WTERMSIG(status);
        //ev.proc->set_status(exited);
        return true;
    }
    else if (WIFSTOPPED(status)) {
        ev.type = evtSignalled;
        ev.status = statusSignalled;
        ev.what = (eventWhat_t) (unsigned int) WSTOPSIG(status);
        return true;
    }
    else {
        ev.type = evtUndefined;
        ev.what = 0;
        return false;
    }
    return false;
}

int SignalHandlerUnix::forwardSigToProcess(EventRecord &ev) {
    process *proc = ev.proc;

    // Pass the signal along to the child
    bool res;
    if(process::IndependentLwpControl()) {
       fprintf(stderr, "%s[%d]:  before continueLWP(%d)\n", __FILE__, __LINE__, ev.info);
       res = ev.lwp->continueLWP(ev.info);
    } else {
       res = proc->continueProc(ev.info);
    } 
    if (res == false) {
        cerr << "Couldn't forward signal " << ev.info << endl;
        logLine("error  in forwarding  signal\n");
        showErrorCallback(38, "Error  in forwarding  signal");
        return 0;
    } 

    return 1;
}

bool SignalHandlerUnix::handleSigTrap(EventRecord &ev) 
{
    process *proc = ev.proc;
    
    // SIGTRAP is our workhorse. It's used to stop the process at a specific
    // address, notify the mutator/daemon of an event, and a few other things
    // as well.
    signal_printf("%s[%d]: SIGTRAP for pid %d, status = %s\n", FILE__, __LINE__,
                  proc->getPid(), proc->getStatusAsString().c_str());

    /////////////////////////////////////////
    // dlopen/close section
    /////////////////////////////////////////
    
    // check to see if trap is due to dlopen or dlcose event
    if(proc->isDynamicallyLinked()){
        if(proc->handleIfDueToSharedObjectMapping()){
            signal_cerr << "handle TRAP due to dlopen or dlclose event\n";
            proc->continueProc();
	    return true;
        }
    }

    // MT AIX is getting spurious trap instructions. I can't figure out where
    // they are coming from (and they're not at all deterministic) so 
    // we're ignoring them for now. 

    // On Linux we see a trap when the process execs. However,
    // there is no way to distinguish this trap from any other,
    // and so it is special-cased here.
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)
    if (proc->nextTrapIsExec) {
        signal_printf("%s[%d]: handling trap as exec exit\n", FILE__, __LINE__);
        return handleExecExit(ev);
    }
#endif

    // Check to see if this is a syscall exit
    if (proc->handleSyscallExit(0, ev.lwp)) {
      signal_printf("%s[%d]: handling trap as syscall exit\n", FILE__, __LINE__);
      proc->continueProc();
      return true;
    }

#if defined(os_linux)
   return forwardSigToProcess(ev);
#endif

    signal_printf("%s[%d]:  SigTrap failing\n", FILE__, __LINE__);
    return false;
}

// Needs to be fleshed out
bool SignalHandlerUnix::handleSigStopNInt(EventRecord &ev) 
{
   process *proc = ev.proc;
   bool retval = false;
   assert(ev.lwp);

#if defined(os_linux)
      // Linux uses SIGSTOPs for process control.  If the SIGSTOP
      // came during a process::pause (which we would know because
      // suppressEventConts() is set) then we'll handle the signal.
      // If it comes at another time we'll assume it came from something
      // like a Dyninst Breakpoint and not handle it.      
      proc->set_lwp_status(ev.lwp, stopped);
#else
      signal_cerr << "unhandled SIGSTOP for pid " << proc->getPid() 
		  << " so just leaving process in paused state.\n" 
		  << std::flush;
#endif
      getSH()->signalEvent(evtProcessStop);
      retval = true;

   // Unlike other signals, don't forward this to the process. It's stopped
   // already, and forwarding a "stop" does odd things on platforms
   // which use ptrace. PT_CONTINUE and SIGSTOP don't mix
   return retval;
}

bool SignalHandlerUnix::handleSigCritical(EventRecord &ev) 
{
   process *proc = ev.proc;
   fprintf(stderr, "%s[%d]:  SIG CRITICAL (sig = %d)  received, dying...\n", 
           FILE__, __LINE__, (int) ev.info);

   signal_printf("Process %d dying on signal %d\n", proc->getPid(), ev.info);
   
   exit(-1);
   for (unsigned thr_iter = 0; thr_iter <  proc->threads.size(); thr_iter++) {
       dyn_lwp *lwp = proc->threads[thr_iter]->get_lwp();
       if (lwp) {
#if defined(arch_alpha)
	 dyn_saved_regs regs;
	 lwp->getRegisters(&regs);
	 
	 fprintf(stderr, "GP: %lx\n", regs.theIntRegs.regs[REG_GP]);
	 fprintf(stderr, "SP: %lx\n", regs.theIntRegs.regs[REG_SP]);
	 fprintf(stderr, "FP: %lx\n", regs.theIntRegs.regs[15]);
	 fprintf(stderr, "RA: %lx\n", regs.theIntRegs.regs[REG_RA]);
#endif

           pdvector<Frame> stackWalk;
           lwp->walkStack(stackWalk);
           
           signal_printf( "TID: %d, LWP: %d\n",
                   proc->threads[thr_iter]->get_tid(),
                   lwp->get_lwp_id());
	   for (unsigned foo = 0; foo < stackWalk.size(); foo++)
	     signal_cerr << "   " << foo << ": " << stackWalk[foo] << endl;
       }
   }
   if (dyn_debug_signal) {
       signal_printf("Critical signal received, spinning to allow debugger to attach\n");
       while(1) sleep(10);
   }
   fprintf(stderr, "Process dying with critical signal...\n");
   proc->dumpImage("imagefile");
   forwardSigToProcess(ev);
   return true;
}

bool SignalGeneratorUnix::decodeRTSignal(EventRecord &ev)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   process *proc = ev.proc;
   if (!proc) return false;

   pdstring status_str = pdstring("DYNINST_synch_event_id");
   pdstring arg_str = pdstring("DYNINST_synch_event_arg1");

   int status;
   Address arg;

   pdvector<int_variable *> vars;
   if (!proc->findVarsByAll(status_str, vars)) {
     return false;
   }

   if (vars.size() != 1) {
     fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
             FILE__, __LINE__, status_str.c_str());
     return false;
   }

   Address status_addr = vars[0]->getAddress();

   if (!proc->readDataSpace((void *)status_addr, sizeof(int),
                            &status, true)) {
      fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
      return false;
   }

   if (status == DSE_undefined) {
      return false; // Nothing to see here
   }

   vars.clear();
   if (!proc->findVarsByAll(arg_str, vars)) {
     return false;
   }

   if (vars.size() != 1) {
     fprintf(stderr, "%s[%d]:  ERROR:  %d vars matching %s, not 1\n", 
             FILE__, __LINE__, arg_str.c_str());
     return false;
   }

   Address arg_addr = vars[0]->getAddress();

   if (!proc->readDataSpace((void *)arg_addr, sizeof(Address),
                            &arg, true)) {
      fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
      return false;
   }

   ev.info = (eventInfo_t)arg;
   switch(status) {
     case DSE_forkEntry:
        /* Entry to fork */
        ev.type = evtSyscallEntry;
        ev.what = SYS_fork;
        break;
     case DSE_forkExit:
        ev.type = evtSyscallExit;
#if defined (os_aix)
        ev.what = SYSSET_MAP(SYS_fork, proc->getPid());
#else
        ev.what = SYS_fork;
#endif
        break;
     case DSE_execEntry:
        /* Entry to exec */
        ev.type = evtSyscallEntry;
        ev.what = SYS_exec;
        break;
     case DSE_execExit:
        /* Exit of exec, unused */
        break;
     case DSE_exitEntry:
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        ev.type = evtSyscallEntry;
        ev.what = SYS_exit;
        break;
   case DSE_loadLibrary:
     /* We need to hook this into the shared library handling code... */
        ev.type = evtSyscallExit;
        ev.what = SYS_load;
     break;

     default:
        assert(0);
        break;
   }
   return true;
}

bool SignalGeneratorUnix::decodeSigIll(EventRecord &ev) 
{
#if defined (arch_ia64) 
  return ev.proc->getRpcMgr()->decodeEventIfDueToIRPC(ev);
#endif
  return false;
}

bool SignalHandlerUnix::handleSIGCHLD(EventRecord &ev) 
{
#if defined (os_linux)
  // Linux fork() sends a SIGCHLD once the fork has been created
  ev.type = evtPreFork;
#endif
  assert (ev.proc);
  ev.proc->continueProc();

  return true;
}

bool SignalHandlerUnix::handleSIGSTOP(EventRecord &ev)
{
  assert(ev.proc);
   bool ret = handleSigStopNInt(ev);
   
#if defined (os_linux)
  if (! getSH()->resendSuppressedSignals(ev)) {
    //fprintf(stderr, "%s[%d]:  failed to resend suppressed signals\n", __FILE__, __LINE__);
  }
  //return true;
#endif
  return ret;
}

bool SignalHandlerUnix::handleSignal(EventRecord &ev) 
{
    process *proc = ev.proc;
    bool ret = false;

#if defined (os_linux)
    if (getSH()->suppressSignalWhenStopping(ev)) {
      ev.lwp->continueLWP_(0);
      ev.proc->set_lwp_status(ev.lwp, running);
      return true;
    }
#endif
    switch(ev.what) {
      case SIGTRAP: 
         signal_printf("%s[%d]:  SIGTRAP\n", FILE__, __LINE__);
         ret = handleSigTrap(ev); break;
      case SIGSTOP:
      case SIGINT: 
         signal_printf("%s[%d]:  SIGSTOP\n", FILE__, __LINE__);
         ret = handleSIGSTOP(ev); break;
      case SIGILL: 
         signal_printf("%s[%d]:  SIGILL\n", FILE__, __LINE__);
         ret = handleSigCritical(ev); break;
      case SIGCHLD: 
         signal_printf("%s[%d]:  SIGCHLD\n", FILE__, __LINE__);
         ret = handleSIGCHLD(ev); break;
      case SIGIOT:
        signal_printf("%s[%d]: SIGABRT\n", FILE__, __LINE__);
        ev.status = statusSignalled;
        ev.info = ev.what;
        sprintf(errorLine, "process %d has terminated on signal %d\n",
                proc->getPid(), (int) ev.info);
        logLine(errorLine);
        statusLine(errorLine);
        ev.proc->triggerSignalExitCallback(ev.info);
        ev.proc->handleProcessExit();
        flagBPatchStatusChange();
        getSH()->signalEvent(evtProcessExit);
        ret = true;
        break;
      case SIGBUS:
        signal_printf("%s[%d]: SIGBUS\n", FILE__, __LINE__);
      case SIGSEGV: 
         signal_printf("%s[%d]: SIGSEGV\n", FILE__, __LINE__);
         ret = handleSigCritical(ev); break;
      case SIGCONT:
#if defined(os_linux) || defined (os_aix)
         ret = true;
         break;
#endif
      case SIGALRM:
      case SIGUSR1:
      case SIGUSR2:
      case SIGVTALRM:
         signal_printf("%s[%d]:  sigalrm or sigusr\n", FILE__, __LINE__);
         proc->set_lwp_status(ev.lwp, stopped);
      default:
         fprintf(stderr, "%s[%d]:  bad signal id!: %d\n", FILE__, __LINE__, ev.what);
         ret = false;
         break;
    }
    
     bool exists = false;   
     BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid(), &exists);
     if (bproc) 
       setBPatchProcessSignal(bproc, ev.what);
    return ret;
 }

 //////////////////////////////////////////////////////////////////
 // Syscall handling
 //////////////////////////////////////////////////////////////////

 // Most of our syscall handling code is shared on all platforms.
 // Unfortunately, there's that 5% difference...

bool SignalHandlerUnix::handleForkEntry(EventRecord &ev) 
{
     signal_printf("Welcome to FORK ENTRY for process %d\n",
                   ev.proc->getPid());
     return ev.proc->handleForkEntry();
}

 // On AIX I've seen a long string of calls to exec, basically
 // doing a (for i in $path; do exec $i/<progname>
 // This means that the entry will be called multiple times
 // until the exec call gets the path right.
bool SignalHandlerUnix::handleExecEntry(EventRecord &ev) 
{
     return ev.proc->handleExecEntry((char *)ev.info);
}

bool SignalHandlerUnix::handleLwpExit(EventRecord &ev) 
{
   signal_printf("%s[%d]:  welcome to handleLwpExit\n", FILE__, __LINE__);
   process *proc = ev.proc;
   dyn_lwp *lwp = ev.lwp;
   dyn_thread *thr = NULL;
   //Find the exiting thread
   for (unsigned i=0; i<proc->threads.size(); i++)
      if (proc->threads[i]->get_lwp()->get_lwp_id() == lwp->get_lwp_id())
      {
         thr = proc->threads[i];
         break;
      }
   if (!thr)
   {
      return false;
   }

   ev.type = evtThreadExit;

   if (proc->IndependentLwpControl())
      proc->set_lwp_status(ev.lwp, exited);

   BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid());
   BPatch_thread *bthrd = bproc->getThread(thr->get_tid());

   pdvector<CallbackBase *> cbs;
   getCBManager()->dispenseCallbacksMatching(evtThreadExit, cbs);
   for (unsigned int i = 0; i < cbs.size(); ++i) {
     AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
     mailbox_printf("%s[%d]:  executing thread exit callback\n", FILE__, __LINE__);
     BPatch_thread *bpthread = bproc->getThreadByIndex(bthrd->getBPatchID());
     assert(bpthread);
     cb(bproc, bpthread);
   }

   flagBPatchStatusChange();
   return true;
}

bool SignalHandlerUnix::handleSyscallEntry(EventRecord &ev) 
{
    signal_printf("%s[%d]:  welcome to handleSyscallEntry\n", FILE__, __LINE__);
    process *proc = ev.proc;
    procSyscall_t syscall = decodeSyscall(proc, ev.what);
    bool ret = false;
    switch (syscall) {
      case procSysFork:
          ret = handleForkEntry(ev);
          break;
      case procSysExec:
         ret = handleExecEntry(ev);
         break;
      case procSysExit:
          signal_printf("%s[%d]:  handleSyscallEntry exit(%d)\n", FILE__, __LINE__, ev.info);
          proc->triggerNormalExitCallback(ev.info);
          ret = true;
          break;
      case procLwpExit:
         signal_printf("%s[%d]:  handleSyscallEntry: lwp_exit\n", FILE__, __LINE__);
         ret = handleLwpExit(ev);
         break;
      default:
      // Check process for any other syscall
      // we may have trapped on entry to?
      ret = false;
      break;
    }
    // Continue the process post-handling
    proc->continueProc();
    return ret;
}

 /* Only dyninst for now... paradyn should use this soon */
bool SignalHandlerUnix::handleForkExit(EventRecord &ev) 
{
     signal_printf("Welcome to FORK EXIT for process %d\n",
                   ev.proc->getPid());

     process *proc = ev.proc;
     // Fork handler time
     extern pdvector<process*> processVec;
     int childPid = ev.info;

     if (childPid == getpid()) {
         // this is a special case where the normal createProcess code
         // has created this process, but the attach routine runs soon
         // enough that the child (of the mutator) gets a fork exit
         // event.  We don't care about this event, so we just continue
         // the process - jkh 1/31/00
         return true;
     } else if (childPid > 0) {

         unsigned int i;
         for (i=0; i < processVec.size(); i++) {
             if (processVec[i] && 
                 (processVec[i]->getPid() == childPid)) break;
         }
         if (i== processVec.size()) {
             // this is a new child, register it with dyninst
             sleep(1);

             // We leave the parent paused until the child is finished,
             // so that we can be sure to copy everything correctly.

             process *theChild = new process(proc, (int)childPid, -1);

             if (theChild->setupFork()) {
                 proc->handleForkExit(theChild);

                 // Okay, let 'er rip
                 proc->continueProc();
                 theChild->continueProc();
             }
             else {
                 // Can happen if we're forking something we can't trace
                 delete theChild;
                 proc->continueProc();
             }
        }
    }
    return true;
}

// the alwaysdosomething argument is to maintain some strange old code
bool SignalHandlerUnix::handleExecExit(EventRecord &ev)
{
    process *proc = ev.proc;
    proc->nextTrapIsExec = false;
    if((int)ev.info == -1) {
        // Failed exec, do nothing
        return false;
    }

    proc->execFilePath = proc->tryToFindExecutable(proc->execPathArg, proc->getPid());
    // As of Solaris 2.8, we get multiple exec signals per exec.
    // My best guess is that the daemon reads the trap into the
    // kernel as an exec call, since the process is paused
    // and PR_SYSEXIT is set. We want to ignore all traps 
    // but the last one.
    
    // We also see an exec as the first signal in a process we create. 
    // That's because it... wait for it... execed!
    if (!proc->reachedBootstrapState(begun_bs)) {
        getSH()->decodeSigTrap(ev);
        return handleProcessCreate(ev);
    }

    // Unlike fork, handleExecExit doesn't do all processing required.
    // We finish up when the trap at main() is reached.
    proc->handleExecExit();

    return true;
}

bool SignalHandlerUnix::handleLoadExit(EventRecord &ev) {
    // AIX: for 4.3.2 and later, load no longer causes the 
    // reinitialization of the process text space, and as
    // such we don't need to fix base tramps.
    return ev.proc->handleIfDueToSharedObjectMapping();
}


bool SignalHandlerUnix::handleSyscallExit(EventRecord &ev) {
    process *proc = ev.proc;
    procSyscall_t syscall = decodeSyscall(proc, ev.what);
    bool ret = false;
    
    // Check to see if a thread we were waiting for exited a
    // syscall
    bool wasHandled = proc->handleSyscallExit(ev.status, ev.lwp);

    // Fall through no matter what since some syscalls have their
    // own handlers.
    switch(syscall) {
      case procSysFork:
         ret = handleForkExit(ev);
         break;
      case procSysExec:
         ret = handleExecExit(ev);
         break;
      case procSysLoad:
         ret = handleLoadExit(ev);
         break;
      default:
         fprintf(stderr, "%s[%d]:  unknown syscall\n", __FILE__, __LINE__);
         break;
    }

#if defined(rs6000_ibm_aix4_1)
    // When we handle a fork exit on AIX, we need to keep both parent and
    // child stopped until we've seen the fork exit on both.  This is so
    // we can copy the instrumentation from the parent to the child (if we
    // don't keep the parent stopped, it may, for instance, exit before we
    // can do this).  So, don't continue the process here - it will be
    // continued at the appropriate time by handleForkExit.
    if (syscall != procSysFork)
#endif
      //if (proc->status() == stopped) proc->continueProc();
      proc->continueProc();
    
    return ret || wasHandled;
}
#if !defined (os_linux)
bool SignalGeneratorUnix::decodeProcStatus(process *,
                     procProcStatus_t status,
                     EventRecord &ev) 
{

   ev.info = GETREG_INFO(status.pr_reg);

   switch (status.pr_why) {
     case PR_SIGNALLED:
        ev.type = evtSignalled;
        ev.what = status.pr_what;
        break;
     case PR_SYSENTRY:
        ev.type = evtSyscallEntry;
        ev.what = status.pr_what;

#if defined(AIX_PROC)
        // We actually pull from the syscall argument vector
        if (status.pr_nsysarg > 0)
           ev.info = status.pr_sysarg[0];
        else
           ev.info = 0;
#endif
#if defined (os_osf)
       ev.info = status.pr_reg.regs[A0_REGNUM];
#endif
        break;
     case PR_SYSEXIT:
        ev.type = evtSyscallExit;
        ev.what = status.pr_what;

#if defined(AIX_PROC)
        // This from the proc header file: system returns are
        // left in pr_sysarg[0]. NOT IN MAN PAGE.
        ev.info = status.pr_sysarg[0];
#endif
#if defined (os_osf)
       ev.info = status.pr_reg.regs[V0_REGNUM];
#endif
        break;
     case PR_REQUESTED:
        // We don't expect PR_REQUESTED in the signal handler
        assert(0 && "PR_REQUESTED not handled");
#if defined(PR_SUSPENDED)
     case PR_SUSPENDED:
        // I'm seeing this state at times with a forking multi-threaded
        // child process, currently handling by just continuing the process
        ev.type = evtSuspended;
        break;
#endif
     case PR_JOBCONTROL:
     case PR_FAULTED:
     default:
        assert(0);
        break;
   }

   return true;
}
#endif
#if defined (os_aix)
procSyscall_t SignalHandlerUnix::decodeSyscall(process *p, eventWhat_t what)
#else
procSyscall_t SignalHandlerUnix::decodeSyscall(process *, eventWhat_t what)
#endif
{
#if defined (os_aix)
   int pid = p->getPid();
#endif
   int syscall = (int) what;

    if (syscall == SYSSET_MAP(SYS_fork, pid) ||
        syscall == SYSSET_MAP(SYS_fork1, pid) ||
        syscall == SYSSET_MAP(SYS_vfork, pid))
        return procSysFork;
    if (syscall == SYSSET_MAP(SYS_exec, pid) ||
        syscall == SYSSET_MAP(SYS_execv, pid) ||
        syscall == SYSSET_MAP(SYS_execve, pid))
        return procSysExec;
    if (syscall == SYSSET_MAP(SYS_exit, pid))
        return procSysExit;
    if (syscall == SYSSET_MAP(SYS_lwp_exit, pid))
        return procLwpExit;
    // Don't map -- we make this up
    if (syscall == SYS_load)
      return procSysLoad;

    return procSysOther;
}

bool SignalHandlerUnix::handleEvent(EventRecord &ev)
{
  global_mutex->_Lock(FILE__, __LINE__);
  bool ret = handleEventLocked(ev);
  if (!events_to_handle.size())
    idle_flag = true;
  active_proc = NULL;
  global_mutex->_Unlock(FILE__, __LINE__);

  return ret;
}

bool SignalHandlerUnix::handleProcessCreate(EventRecord &ev)
{
  process * proc = ev.proc;
  proc->setBootstrapState(begun_bs);
  if (proc->insertTrapAtEntryPointOfMain()) {
      pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
      buffer += pdstring(", attached to process, stepping to main");
      statusLine(buffer.c_str());
      proc->continueProc();
      return true;
   } else {
      // We couldn't insert the trap... so detach from the process
      // and let it run. 
      fprintf(stderr, "%s[%d][%s]:  ERROR:  couldn't insert at entry of main,\n",
              FILE__, __LINE__, getThreadStr(getExecThreadID()));
      fprintf(stderr, "\tinstrumenting process impossible\n");
      // We should actually delete any mention of this
      // process... including (for Paradyn) removing it from the
      // frontend.
      proc->triggerNormalExitCallback(0);
      proc->handleProcessExit();
      proc->continueProc();
    }
   return false;
}

bool SignalHandlerUnix::handleEventLocked(EventRecord &ev)
{
  signal_printf("%s[%d]:  got event: %s\n", FILE__, __LINE__, eventType2str(ev.type));

  process *proc = ev.proc;
  bool ret = false;
  Frame activeFrame;
  assert(proc);

  // One big switch statement
  switch(ev.type) {
     // First the platform-independent stuff
     // (/proc and waitpid)
     case evtProcessExit:
        if (ev.status == statusNormal) {
          sprintf(errorLine, "Process %d has terminated with code 0x%x\n",
                  proc->getPid(), (int) ev.info);
          statusLine(errorLine);
          //proc->triggerNormalExitCallback(ev.info);
          proc->handleProcessExit();
          ret = true;
        } else if (ev.status == statusSignalled) {
          sprintf(errorLine, "process %d has terminated on signal %d\n",
                  proc->getPid(), (int) ev.info);
          logLine(errorLine);
          statusLine(errorLine);
          printDyninstStats();
          proc->triggerSignalExitCallback(ev.info);
          proc->handleProcessExit();
          ret = true;
        } else {
          sprintf(errorLine, "process %d has terminated for unknown reason\n",
                  proc->getPid());
          logLine(errorLine);
          proc->handleProcessExit();
          ret = true; //  maybe this should be false?  (this case is an error)
        }
        flagBPatchStatusChange();
        //if (BPatch::bpatch->waitingForStatusChange)
        //  __BROADCAST;
        getSH()->signalEvent(evtProcessExit);
        break;
     case evtProcessCreate:
        ret = handleProcessCreate(ev);
        break;
     case evtProcessInit:
        proc->handleTrapAtEntryPointOfMain(ev.lwp);
        proc->setBootstrapState(initialized_bs);
        // If we were execing, we now know we finished
        if (proc->execing()) {
           proc->finishExec();
        }
        ret = true;
        break;
     case evtProcessLoadedRT:
     {
        pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
        buffer += pdstring(", loaded dyninst library");
        statusLine(buffer.c_str());
        startup_cerr << "trapDueToDyninstLib returned true, trying to handle\n";
        proc->loadDYNINSTlibCleanup(ev.lwp);
        proc->setBootstrapState(loadedRT_bs);
        //getSH()->signalEvent(evtProcessLoadedRT);
        ret = true;
        break;
     }
     case evtInstPointTrap:
         // Linux inst via traps
         ev.lwp->changePC(ev.address, NULL);
         proc->continueProc();
         ret = true;
         break;

     case evtSignalled:
     case evtPreFork:
     {
        char buf[128];
        ret = handleSignal(ev);
        if (!ret) {
          fprintf(stderr, "%s[%d]:  handleSignal(%s) failed\n", __FILE__, __LINE__,
                  ev.sprint_event(buf));
        }
        signal_printf("%s[%d]:  after handleSignal, event is %s\n", FILE__, __LINE__,
                     ev.sprint_event(buf));
        break;
     }
        // Now the /proc only
        // AIX clones some of these (because of fork/exec/load notification)
     case evtRPCSignal:
       ret = proc->getRpcMgr()->handleRPCEvent(ev);
       break;
     case evtSyscallEntry:
        ret = handleSyscallEntry(ev);
        if (!ret)
            cerr << "handleSyscallEntry failed!" << endl;
        break;
     case evtSyscallExit:
        ret = handleSyscallExit(ev);
        if (!ret)
            fprintf(stderr, "%s[%d]: handlesyscallExit failed! ", __FILE__, __LINE__); ;
        break;
     case evtSuspended:
       proc->continueProc();   // ignoring this signal
       ret = true;
       flagBPatchStatusChange();
       break;
     case evtDebugStep:
         handleSingleStep(ev);
         ret = 1;
         break;
     case evtUndefined:
        // Do nothing
         cerr << "Undefined event!" << endl;
        break;
     case evtTimeout:
     case evtNullEvent:
     case evtThreadDetect:
        ret = true;
        break;
     default:
        fprintf(stderr, "%s[%d]:  cannot handle signal %s\n", FILE__, __LINE__, eventType2str(ev.type));
        assert(0 && "Undefined");
   }

   getSH()->signalEvent(ev);

   proc->setSuppressEventConts(false);

   return ret;
}

#if !defined (os_linux)
bool SignalGeneratorUnix::createPollEvent(pdvector<EventRecord> &events, struct pollfd fds, process *curProc)
{
  EventRecord ev;
  ev.proc = curProc;

  if (fds.revents & POLLHUP) {
     ev.proc = curProc;
     ev.lwp = curProc->getRepresentativeLWP();

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
     
     decodeWaitPidStatus(status, ev);
     decodeEvent(ev);      
     signal_printf("%s[%d]:  new event: %s\n", FILE__, __LINE__, eventType2str(ev.type)); 
     events.push_back(ev);
     return true;
  }
  
   procProcStatus_t procstatus;
   if(! curProc->getRepresentativeLWP()->get_status(&procstatus)) {
      if (ev.type == evtUndefined) {
        ev.type = evtProcessExit;
        ev.proc = curProc;
        events.push_back(ev);
        fprintf(stderr, "%s[%d]:  file desc for process exit not available\n", 
                FILE__, __LINE__);
        return true;
      }
      fprintf(stderr, "%s[%d]:  file desc for %s not available\n", 
              FILE__, __LINE__, eventType2str(ev.type));
      return false;
   }


   // copied from old code, must not care about events that don't stop proc
   if ( !(procstatus.pr_flags & PR_STOPPED || procstatus.pr_flags & PR_ISTOP) ) {
     ev.type = evtNullEvent;
     ev.proc = curProc;
     decodeEvent(ev);
     signal_printf("%s[%d]:  new event: %s\n", 
                   FILE__, __LINE__, eventType2str(ev.type));
     events.push_back(ev);
     return true;
   }
#if defined (os_osf)
   else {
      ev.lwp = curProc->getRepresentativeLWP();
      if (!decodeProcStatus(curProc, procstatus, ev)) {
         fprintf(stderr, "%s[%d]:  decodeProcStatus failed\n", FILE__, __LINE__);
         return false;
      }
      decodeEvent(ev);
     signal_printf("%s[%d]:  new event: %s\n", 
                   FILE__, __LINE__, eventType2str(ev.type));
      events.push_back(ev);
      return true;
   }
#endif

#if defined (os_solaris) || defined (os_aix)
   bool updated_events = false;
   unsigned lwp_to_use = (unsigned) procstatus.pr_lwpid;
   updated_events = updateEvents(events, curProc, lwp_to_use);
   return updated_events;
#endif
  return false;

}
#endif // !defined (os_linux)

#if !defined (os_linux)
int fake_poll(struct pollfd *ufds, unsigned int nfds, int timeout)
{
   int pollret = 0;
   pollret  = poll(ufds, nfds, 0);
   if (pollret == 0) {
     sleep(1);
     pollret  = poll(ufds, nfds, 0);
   }
   return pollret;
}

bool SignalGeneratorUnix::waitNextEvent(EventRecord &ev)
{
  __LOCK;
  assert(getExecThreadID() == getThreadID());
  assert(getExecThreadID() != primary_thread_id);

  signal_printf("%s[%d][%s]:  waitNextEvent\n", FILE__, __LINE__, 
                getThreadStr(getExecThreadID()));
  bool ret = true;
  int timeout = -1;
  static pdvector<EventRecord> events;
  if (events.size()) {
    //  If we have events left over from the last call of this fn,
    //  just return one.
    ev = events[events.size() - 1];
    events.pop_back();
    char buf[128];
    signal_printf("%s[%d][%s]:  waitNextEvent: had existing event %s\n", FILE__, __LINE__, 
                getThreadStr(getExecThreadID()), ev.sprint_event(buf));
    __UNLOCK;
    return true;
  }

#if defined(os_osf)
  //  alpha-osf apparently does not detect process exits from poll events,
  //  so we have to check for exits before calling poll().
  extern bool checkForExit(EventRecord &);
  if (checkForExit(ev)) {
    char buf[128];
    signal_printf("%s[%d][%s]:  process exited %s\n", FILE__, __LINE__, 
                getThreadStr(getExecThreadID()), ev.sprint_event(buf));
    __UNLOCK;
    return true; 
  }
#endif

  pdvector<unsigned int> fds;
  while (!getFDsForPoll(fds)) {
    signal_printf("%s[%d]:  syncthread waiting for process to monitor\n", __FILE__, __LINE__);
    waiting_for_active_process = true;
    bool any_active_handlers = false;
    for (unsigned int i = 0; i < handlers.size(); ++i) {
      if (!handlers[i]->idle() && !handlers[i]->waiting())
        any_active_handlers = true;
    } 
    if (!any_active_handlers) {
      signalEvent(evtProcessStop);
    }
    __WAIT_FOR_SIGNAL;
  }
  waiting_for_active_process = false;
  //  all returns after this alloc should just use "goto cleanup"
  struct pollfd *pfds = new struct pollfd [fds.size()]; // argument for poll

  for (unsigned int i = 0; i < fds.size(); ++i) {
    pfds[i].fd = fds[i];
    pfds[i].events = POLLPRI;
    pfds[i].revents = 0;
  }

#if defined(os_osf)
   //  since process exit does not cause a poll event on alpha, we need a timeout
   timeout = 1000 /*ms*/;
#endif
  __UNLOCK;
  int num_selected_fds = poll(pfds, fds.size(), timeout);
  __LOCK;
  int handled_fds = 0;

  if (num_selected_fds < 0) {
    fprintf(stderr, "%s[%d]:  checkForProcessEvents: poll failed\n", FILE__, __LINE__);
    ret = false;
    goto cleanup;
  } else if (num_selected_fds == 0) {
    //  poll timed out with nothing to report
    fprintf(stderr, "%s[%d]:  poll timed out\n", FILE__, __LINE__);
    ev.type = evtTimeout;
    ret = true;
    goto cleanup;
  }

  //  build a pile of EventRecords for all of the poll events that we got.
  for(unsigned int i=0; i<fds.size(); i++) {
    if(pfds[i].revents == 0) continue;

     process *curProc = findProcessByFD(fds[i]);
     assert(curProc);
     if (curProc->status() == running) {
       curProc->set_status(stopped);
     }
     if (!createPollEvent(events, pfds[i], curProc)) {
       fprintf(stderr, "%s[%d]:  Internal Error: createPollEvent returned false\n", 
               FILE__, __LINE__);
     }
     handled_fds++;
  }

  assert(num_selected_fds == handled_fds);

  //fprintf(stderr, "%s[%d]:  events pile contains:\n", __FILE__, __LINE__);
  //for (unsigned int i = 0; i < events.size(); ++i) {
  //  fprintf(stderr, "\t%s\t%p\n", eventType2str(events[i].type), events[i].proc);
 // }

  // select one to return, and remove it from the pile
  //ev = events.back();
  //events.pop_back();
  ev = events[0];
  events.erase(0,0);
cleanup:
  __UNLOCK;
  delete [] pfds;
  return ret;
}

#endif

bool SignalGeneratorUnix::decodeSigTrap(EventRecord &ev)
{
  char buf[128];
  process *proc = ev.proc;
  bootstrapState_t bootstrapState = proc->getBootstrapState();

  signal_printf("%s[%d]:  welcome to decodeSigTrap for %d, state is %s\n",
                FILE__, __LINE__, ev.proc->getPid(), 
                proc->getBootstrapStateAsString().c_str());

  switch(bootstrapState) {
    case bootstrapped_bs:  
        break;
    case unstarted_bs:     
    case attached_bs:
        ev.type = evtProcessCreate; 
        signal_printf("%s[%d]:  decodeSigTrap for %s, produced: %s\n",
              FILE__, __LINE__, ev.sprint_event(buf), 
              proc->getBootstrapStateAsString().c_str());
        return true;
        break;
    case begun_bs:         
       if (proc->trapAtEntryPointOfMain(ev.lwp)) {
         ev.type = evtProcessInit; 
          signal_printf("%s[%d]:  decodeSigTrap for %s, produced: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf), 
                proc->getBootstrapStateAsString().c_str());
          return true;
       }
       break;
    case loadingRT_bs:
        if (proc->trapDueToDyninstLib(ev.lwp)) {
          ev.type = evtProcessLoadedRT;
          signal_printf("%s[%d]:  decodeSigTrap for %s, produced: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf), 
                proc->getBootstrapStateAsString().c_str());
          return true;
        }
        break;
    case initialized_bs:
    case loadedRT_bs:
    default:
      break;
  };

  Frame af = ev.lwp->getActiveFrame();

  if (proc->trampTrapMapping.defines(af.getPC())) {
     ev.type = evtInstPointTrap;
     ev.address = af.getPC();
     goto finish;
  }

  if (proc->getRpcMgr()->decodeEventIfDueToIRPC(ev)) {
      signal_printf("%s[%d]:  SIGTRAP due to RPC\n", FILE__, __LINE__);
      goto finish;
  }

  if (ev.lwp->isSingleStepping()) {
     ev.type = evtDebugStep;
     ev.address = af.getPC();
     signal_printf("Single step trap at %lx\n", ev.address);
   }

  finish:
  signal_printf("%s[%d]:  decodeSigTrap for %s, state: %s\n",
                FILE__, __LINE__, ev.sprint_event(buf), 
                proc->getBootstrapStateAsString().c_str());
  return true;
}


bool SignalGeneratorUnix::decodeSigStopNInt(EventRecord &ev)
{
  char buf[128];
  process *proc = ev.proc;

  signal_printf("%s[%d]:  welcome to decodeSigStopNInt for %d, state is %s\n",
                FILE__, __LINE__, ev.proc->getPid(), 
                proc->getBootstrapStateAsString().c_str());


  return proc->getRpcMgr()->decodeEventIfDueToIRPC(ev);
}

///////////////////////////////////////////
/////      DebuggerInterface
///////////////////////////////////////////

unsigned long dbi_thread_id = -1UL;
DebuggerInterface *global_dbi = NULL;
DebuggerInterface *getDBI()
{
  if (global_dbi) return global_dbi;
  global_dbi = new DebuggerInterface();
  return global_dbi; 
}

void dbi_signal_done_(CallbackBase *cb)
{
  dbi_printf("%s[%d][%s]:  welcome to dbi_signal_done\n", FILE__, __LINE__, 
         getThreadStr(getExecThreadID()));

  DBICallbackBase *dbi_cb = (DBICallbackBase *) cb;
  getDBI()->signalDone(dbi_cb);  

  dbi_printf("%s[%d][%s]:  leaving dbi_signal_done\n", FILE__, __LINE__, 
         getThreadStr(getExecThreadID()));
}

CallbackCompletionCallback dbi_signal_done = dbi_signal_done_;

bool DebuggerInterface::waitNextEvent(DBIEvent &ev)
{
  isReady = true;
  //fprintf(stderr, "%s[%d]: DBI:waitNextEvent is about to get lock\n", __FILE__, __LINE__);
  //fprintf(stderr, "%s[%d]:  welcome to waitNextEvent for DebugInterface\n", __FILE__, __LINE__);
  dbilock._Lock(__FILE__, __LINE__);
  if (evt == dbiUndefined) {
    //fprintf(stderr, "%s[%d]:  DebugInterface waiting for something to do\n", __FILE__, __LINE__);
    dbilock._WaitForSignal(__FILE__, __LINE__);
  }
  //  got something
  ev.type = evt;
  //fprintf(stderr, "%s[%d]:  DebuggerInterface got event %s\n", __FILE__, __LINE__,
   //       eventType2str(ev.type));
  dbilock._Unlock(__FILE__, __LINE__);
  return true;
}

bool DebuggerInterface::handleEventLocked(DBIEvent &ev)
{
  assert(dbilock.depth());

  evt = ev.type;

  getMailbox()->executeCallbacks(FILE__, __LINE__);

  //  event is handled, so set evt back to undefined. (our waiting criterion)
  evt = dbiUndefined;
  dbilock._Broadcast(FILE__, __LINE__);
  return true;
}

bool DBICallbackBase::DBILock()
{
  return getDBI()->dbilock._Lock(__FILE__, __LINE__);
}
bool DBICallbackBase::DBIUnlock()
{
  return getDBI()->dbilock._Unlock(__FILE__, __LINE__);
}

bool PtraceCallback::operator()(int req, pid_t pid, Address addr,
                                Address data, int word_len)
{
  //  No need to copy buffers since dbi callbacks will only be used in
  //  the immediate context of the call;
  req_ = req;
  pid_ = pid;
  addr_ = addr;
  data_ = data;
  word_len_  = word_len;
  ret = (PTRACE_RETURN)0;
  getMailbox()->executeOrRegisterCallback(this);
  getDBI()->waitForCompletion((DBICallbackBase *)this);
  assert(done_flag);
  return true;
}

bool PtraceCallback::execute()
{
  //  just do a simple call to ptrace:
  DBILock();
  errno = 0;

  ret = P_ptrace(req_, pid_, addr_, data_, word_len_);
  if (errno) {
    fprintf(stderr, "%s[%d][%s]:  ptrace(%d, pid = %d, %p, %p, %d) failed\n", 
            FILE__, __LINE__, 
           getThreadStr(getExecThreadID()), req_, pid_, addr_, data_, word_len_);
    perror("ptrace error");
  }
  ptrace_errno = errno;
  DBIUnlock();
  return true;
}

PTRACE_RETURN DebuggerInterface::ptrace(int req, pid_t pid, Address addr, 
                              Address data, int word_len, int *ptrace_errno)
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::ptrace()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();

  PTRACE_RETURN ret;
  PtraceCallback *ptp = new PtraceCallback();
  PtraceCallback &pt = *ptp;

  pt.enableDelete(false);
  pt(req, pid, addr, data, word_len);
  ret = pt.getReturnValue();
  if (ptrace_errno) 
    *ptrace_errno = pt.getErrno();
  pt.enableDelete();

  releaseBusy();
  return ret;
}



PTRACE_RETURN DBI_ptrace(int req, pid_t pid, Address addr, Address data, int *ptrace_errno, int word_len,  const char *file, unsigned int line) 
{
  PTRACE_RETURN ret;
  ret = getDBI()->ptrace(req, pid, addr, data, word_len, ptrace_errno);
  return ret;
}

bool WaitPidNoBlockCallback::operator()(int *status)
{
  status_ = status;
  getMailbox()->executeOrRegisterCallback(this);
  DBIEvent ev(dbiWaitPid);
  getDBI()->waitForCompletion((DBICallbackBase *)this);
  assert(done_flag);
  return true;
}

bool WaitPidNoBlockCallback::execute() 
{
  DBILock();
#if defined (os_linux)
  int wait_options = __WALL | WNOHANG;
  ret = waitpid(-1, status_, wait_options);
#else
  assert(0);
#endif
  DBIUnlock();
  return true;
}

int DebuggerInterface::waitpidNoBlock(int *status)
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::waitPidNoBlock()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();

  bool ret;
  WaitPidNoBlockCallback *cbp = new WaitPidNoBlockCallback();
  WaitPidNoBlockCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(status);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

/*****************************************************************************
 * forkNewProcess: starts a new process, setting up trace and io links between
 *                the new process and the daemon
 * Returns true if succesfull.
 * 
 * Arguments:
 *   file: file to execute
 *   dir: working directory for the new process
 *   argv: arguments to new process
 *   argp: environment variables for new process
 *   inputFile: where to redirect standard input
 *   outputFile: where to redirect standard output
 *   traceLink: handle or file descriptor of trace link (read only)
//removed all ioLink related code for output redirection
 *   ioLink: handle or file descriptor of io link (read only)
 *   pid: process id of new process
 *   tid: thread id for main thread (needed by WindowsNT)
 *   procHandle: handle for new process (needed by WindowsNT)
 *   thrHandle: handle for main thread (needed by WindowsNT)
 ****************************************************************************/
bool DebuggerInterface::forkNewProcess(pdstring file, pdstring dir,
                                        pdvector<pdstring> *argv,
                                        pdvector<pdstring> *envp,
                                        pdstring inputFile,
                                        pdstring outputFile,
                                        int &traceLink, int &pid, int &tid,
                                        int &procHandle, int &thrHandle,
                                        int stdin_fd, int stdout_fd, int stderr_fd)
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::forkNewProcess(%s)\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()), file.c_str());
  getBusy();

  bool ret;
  ForkNewProcessCallback *fnpp = new ForkNewProcessCallback();
  ForkNewProcessCallback &fnp = *fnpp;

  fnp.enableDelete(false);
  fnp(file, dir, argv, envp, inputFile, outputFile, traceLink, 
      pid, tid, procHandle, thrHandle, 
      stdin_fd, stdout_fd, stderr_fd);
  ret = fnp.getReturnValue();
#if !defined(BPATCH_LIBRARY)
  //traceLink = fnp.traceLink_;
  
#endif
  fnp.enableDelete();

  releaseBusy();
  return ret;
}

bool ForkNewProcessCallback::operator()(pdstring file, pdstring dir,
                                        pdvector<pdstring> *argv,
                                        pdvector<pdstring> *envp,
                                        pdstring inputFile,
                                        pdstring outputFile,
                                        int &traceLink, int &pid, int &tid,
                                        int &procHandle, int &thrHandle,
                                        int stdin_fd, int stdout_fd, int stderr_fd)
{
  file_ = &file;
  dir_ = &dir;
  argv_ = argv;
  envp_ = envp;
  inputFile_ = &inputFile;
  outputFile_ = &outputFile;
  traceLink_ = &traceLink;
  pid_ = &pid;
  tid_ = &tid;
  procHandle_ = &procHandle;
  thrHandle_ = &thrHandle;
  stdin_fd_ = stdin_fd;
  stdout_fd_ = stdout_fd;
  stderr_fd_ = stderr_fd;

  startup_printf("%s[%d]:  ForkNewProcessCallback, target thread is %lu(%s)\n", __FILE__, __LINE__, targetThread(), getThreadStr(targetThread()));
  DBIEvent ev(dbiForkNewProcess);
  getMailbox()->executeOrRegisterCallback(this);
  getDBI()->waitForCompletion((DBICallbackBase *)this);
  assert(done_flag);
  return true;
}

bool ForkNewProcessCallback::execute()
{
  DBILock();
  dbi_printf("%s[%d][%s]:  welcome to ForkNewProcessCallback::execute(%s)\n",
          __FILE__, __LINE__, getThreadStr(getExecThreadID()), file_->c_str());
   int &pid = *pid_;
   pdstring &file = *file_;
   pdstring &dir = *dir_;
   pdvector<pdstring> *envp = envp_;
   pdvector<pdstring> *argv = argv_;
  
#if 0
   fprintf(stderr, "%s[%d]:  ARGV:\n\t", __FILE__, __LINE__);
   char dstr[4096];
   dstr[0] = '\0';
   for (unsigned int i = 0; i < argv->size(); ++i) {
     sprintf(dstr, "%s %s", dstr, (*argv)[i].c_str());
   }
   fprintf(stderr, "%s\n", dstr);
   fprintf(stderr, "%s[%d]:  ENVP:\n\t", __FILE__, __LINE__);
   dstr[0] = '\0';
   if (envp)
     for (unsigned int i = 0; i < envp->size(); ++i) {
       sprintf(dstr, "%s %s", dstr, (*envp)[i].c_str());
     }
   else
      sprintf(dstr, "null");
   fprintf(stderr, "%s\n", dstr);
#endif

#ifndef BPATCH_LIBRARY
   // Strange, but using socketpair here doesn't seem to work OK on SunOS.
   // Pipe works fine.
   // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, tracePipe);
   int tracePipe[2];
   int r = P_pipe(tracePipe);
   if (r) {
      // P_perror("socketpair");
      pdstring msg = pdstring("Unable to create trace pipe for program '") + file +
         pdstring("': ") + pdstring(strerror(errno));
      showErrorCallback(68, msg);
      ret = false;
      DBIUnlock();
      return ret;
   }

   /* removed for output redirection
   // ioPipe is used to redirect the child's stdout & stderr to a pipe which is in
   // turn read by the parent via the process->ioLink socket.
   int ioPipe[2];

   // r = P_socketpair(AF_UNIX, SOCK_STREAM, (int) NULL, ioPipe);
   r = P_pipe(ioPipe);
   if (r) {
	// P_perror("socketpair");
   pdstring msg = pdstring("Unable to create IO pipe for program '") + file +
   pdstring("': ") + pdstring(sys_errlist[errno]);
	showErrorCallback(68, msg);
	return false;
   }
   */
#endif

   //
   // WARNING This code assumes that vfork is used, and a failed exec will
   //   corectly change failed in the parent process.
   //
    
   errno = 0;

   pid = fork();

   if (pid != 0) {

      startup_printf("%s[%d][%s]:  ForkNewProcessCallback::execute(%s): FORK PARENT\n",
               __FILE__, __LINE__, getThreadStr(getExecThreadID()), file_->c_str());
      // *** parent

#if (defined(BPATCH_LIBRARY) && !defined(alpha_dec_osf4_0))
      /*
       * On Irix, errno sometimes seems to have a non-zero value after
       * the fork even though it succeeded.  For now, if we're using fork
       * and not vfork, we will check the return code of fork to determine
       * if there was error, instead of relying on errno (so make sure the
       * condition for this section of code is the same as the condition for
       * using fork instead of vfork above). - brb
       */
      if (pid == -1)
#else
         if (errno)
#endif
         {
      fprintf(stderr, "%s[%d][%s]:  ForkNewProcessCallback::execute(%s): FORK ERROR\n",
               __FILE__, __LINE__, getThreadStr(getExecThreadID()), file_->c_str());
            sprintf(errorLine, "Unable to start %s: %s\n", file.c_str(), 
                    strerror(errno));
            logLine(errorLine);
            showErrorCallback(68, (const char *) errorLine);
            ret = false;
            DBIUnlock();
            return ret;
         }

#ifndef BPATCH_LIBRARY
      P_close(tracePipe[1]);
	   // parent never writes trace records; it only receives them.

      /* removed for output redirection
         close(ioPipe[1]);
         // parent closes write end of io pipe; child closes its read end.
         // pipe output goes to the parent's read fd (ret->ioLink); pipe input
         // comes from the child's write fd.  In short, when the child writes to
         // its stdout/stderr, it gets sent to the pipe which in turn sends it to
         // the parent's ret->ioLink fd for reading.

         //ioLink = ioPipe[0];
         */

      *traceLink_ = tracePipe[0];
#endif
      ret = true;
      DBIUnlock();
      return ret;

   } else if (pid == 0) {
      // *** child

#ifndef BPATCH_LIBRARY
      // handle stdio.

      /* removed for output redirection We only write to ioPipe.  Hence we
      // close ioPipe[0], the read end.  Then we call dup2() twice to assign
      // our stdout and stderr to the write end of the pipe.
      // close(ioPipe[0]);
   
      //dup2(ioPipe[1], 1);

	
      // assigns fd 1 (stdout) to be a copy of ioPipe[1].  (Since stdout is
      // already in use, dup2 will first close it then reopen it with the
      // characteristics of ioPipe[1].)  In short, stdout gets redirected
      // towards the write end of the pipe.  The read end of the pipe is read
      // by the parent (paradynd), not by us.

      dup2(ioPipe[1], 2); // redirect fd 2 (stderr) to the pipe, like above.

      // We're not using ioPipe[1] anymore; close it.
      if (ioPipe[1] > 2) close (ioPipe[1]);
      */

      //setup output redirection to termWin
      dup2(stdout_fd_,1);
      dup2(stdout_fd_,2);

      // Now that stdout is going to a pipe, it'll (unfortunately) be block
      // buffered instead of the usual line buffered (when it goes to a tty).
      // In effect the stdio library is being a little too clever for our
      // purposes.  We don't want the "bufferedness" to change.  So we set it
      // back to line-buffered.  The command to do this is setlinebuf(stdout)
      // [stdio.h call] But we don't do it here, since the upcoming execve()
      // would undo our work [execve keeps fd's but resets higher-level stdio
      // information, which is recreated before execution of main()] So when
      // do we do it?  In rtinst's DYNINSTinit (RTposix.c et al.)

      // setup stderr for rest of exec try.
      FILE *childError = P_fdopen(2, "w");

      P_close(tracePipe[0]);

      if (P_dup2(tracePipe[1], 3) != 3) {
         fprintf(childError, "dup2 failed\n");
         fflush(childError);
         P__exit(-1);
      }

      /* close if higher */
      if (tracePipe[1] > 3) P_close(tracePipe[1]);


      if ((dir.length() > 0) && (P_chdir(dir.c_str()) < 0)) {
         bpfatal("cannot chdir to '%s': %s\n", dir.c_str(), 
                 strerror(errno));
         P__exit(-1);
      }
#endif
#if !defined(BPATCH_LIBRARY)
      /* see if I/O needs to be redirected */
      if (inputFile_->length()) {
         int fd = P_open(inputFile_->c_str(), O_RDONLY, 0);
         if (fd < 0) {
            fprintf(childError, "stdin open of %s failed\n", inputFile_->c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 0);
            P_close(fd);
         }
      }

      if (outputFile_->length()) {
         int fd = P_open(outputFile_->c_str(), O_WRONLY|O_CREAT, 0444);
         if (fd < 0) {
            fprintf(childError, "stdout open of %s failed\n", outputFile_->c_str());
            fflush(childError);
            P__exit(-1);
         } else {
            dup2(fd, 1); // redirect fd 1 (stdout) to a copy of descriptor "fd"
            P_close(fd); // not using descriptor fd any more; close it.
         }
      }
#endif

#if defined (BPATCH_LIBRARY)
      // Should unify with (fancier) Paradyn handling
      /* see if we should use alternate file decriptors */
      if (stdin_fd_ != 0) dup2(stdin_fd_, 0);
      if (stdout_fd_ != 1) dup2(stdout_fd_, 1);
      if (stderr_fd_ != 2) dup2(stderr_fd_, 2);
#endif

#ifdef BPATCH_LIBRARY
      // define our own session id so we don't get the mutators signals

#ifndef rs6000_ibm_aix4_1
      setsid();
#endif
#endif
      /* indicate our desire to be traced */
      errno = 0;
      OS::osTraceMe();
      if (errno != 0) {
         fprintf(stderr, 
                 "Could perform set PTRACE_TRACEME on forked process\n");
         fprintf(stderr, 
                 " Perhaps your executable doesn't have the exec bit set?\n");
         P__exit(-1);   // double underscores are correct
      }

      char **envs = NULL;
      if (envp) {
	  envs = new char*[envp->size() + 2]; // Also room for PARADYN_MASTER_INFO
	  for(unsigned ei = 0; ei < envp->size(); ++ei)
	      envs[ei] = P_strdup((*envp)[ei].c_str());
	  envs[envp->size()] = NULL;
      }
      
#ifndef BPATCH_LIBRARY
      // hand off info about how to start a paradynd to the application.
      //   used to catch rexec calls, and poe events.
      //
      char* paradynInfo = new char[1024];
      sprintf(paradynInfo, "PARADYN_MASTER_INFO= ");
      for (unsigned i=0; i < pd_process::arg_list.size(); i++) {
         const char *str;

         str = P_strdup(pd_process::arg_list[i].c_str());
         if (!strcmp(str, "-l1")) {
            strcat(paradynInfo, "-l0");
         } else {
            strcat(paradynInfo, str);
         }
         strcat(paradynInfo, " ");
      }

      if (envp) {
	  envs[envp->size()] = P_strdup(paradynInfo);
	  envs[envp->size() + 1] = NULL;
      } else {
	  P_putenv(paradynInfo);
      }
#endif

      char **args;
      args = new char*[argv->size()+1];
      for (unsigned ai=0; ai<argv->size(); ai++)
         args[ai] = P_strdup((*argv)[ai].c_str());
      args[argv->size()] = NULL;

     startup_printf("%s[%d]:  before exec\n", __FILE__, __LINE__);
     char argstr[2048];
     argstr[0] = '\0';
     for (unsigned int ji=0; ji < argv->size(); ji++) {
       pdstring &s = (*argv)[ji];
       sprintf(argstr, "%s %s", argstr, s.c_str());
     }
     startup_printf("%s[%d]:  EXEC: %s %s\n", __FILE__, __LINE__, file.c_str(), argstr);
      if (envp) {
	  P_execve(file.c_str(), args, envs);
      }else
	  P_execvp(file.c_str(), args);

      sprintf(errorLine, "paradynd: execv failed, errno=%d\n", errno);
      logLine(errorLine);
    
      logLine(strerror(errno));
      {
         int i=0;
         while (args[i]) {
            sprintf(errorLine, "argv %d = %s\n", i, args[i]);
            logLine(errorLine);
            i++;
         }
      }
      {
	  for(unsigned i = 0; envs[i] != NULL; ++i) {
	      sprintf(errorLine, "envp %d = %s\n", i, envs[i]);
	      logLine(errorLine);
	  }
      }	      
      P__exit(-1);
      // not reached
    
      return false;
   } 
   return false;
}

bool DebuggerInterface::writeDataSpace(pid_t pid, Address addr, int nbytes, Address data, int word_len, const char * /*file*/, unsigned int /*line*/) 
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::writeDataSpace()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();

  bool ret;
  WriteDataSpaceCallback *cbp = new WriteDataSpaceCallback();
  WriteDataSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid, addr, nbytes, data, word_len);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool WriteDataSpaceCallback::operator()(pid_t pid, Address addr, int nelem, Address data, int word_len)
{
  pid_ = pid;
  addr_ = addr;
  nelem_ = nelem;
  data_ = data;
  word_len_ = word_len;
  getMailbox()->executeOrRegisterCallback(this);
  DBIEvent ev(dbiWriteDataSpace);
  getDBI()->waitForCompletion((DBICallbackBase *)this);
  assert(done_flag);
  return true;
}

bool WriteDataSpaceCallback::execute()
{
  DBILock();
#if defined (os_linux)
  ret =  getDBI()->bulkPtraceWrite((void *)addr_, nelem_, (void *)data_, pid_, word_len_);
#else
  assert(0);
#endif
  DBIUnlock();
  return true;
}

bool DBI_writeDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
{
  dbi_printf("%s[%d]: DBI_writeDataSpace(%d, %p, %d, %p, %d) called from %s[%d]\n", 
            FILE__, __LINE__, pid, (void *) addr, nelem, (void *) data, word_len,file,line);
  return getDBI()->writeDataSpace(pid, addr, nelem, data, word_len, file, line);
}

bool ReadDataSpaceCallback::operator()(pid_t pid, Address addr, int nelem, Address data, int word_len)
{
  pid_ = pid;
  addr_ = addr;
  nelem_ = nelem;
  data_ = data;
  word_len_ = word_len;
  getMailbox()->executeOrRegisterCallback(this);
  DBIEvent ev(dbiReadDataSpace);
  getDBI()->waitForCompletion((DBICallbackBase *)this);
  assert(done_flag);
  return true;
}

bool ReadDataSpaceCallback::execute()
{
  DBILock();
#if defined (os_linux)
  ret =  getDBI()->bulkPtraceRead((void *)addr_, nelem_, (void *)data_, pid_, word_len_);
#else
  assert(0);
#endif
  DBIUnlock();
  return true;
}

bool SignalHandlerUnix::handleSingleStep(const EventRecord &ev) {
   ev.lwp->setSingleStepping(false);
   getSH()->signalEvent(evtDebugStep);
   return true;
}

bool DebuggerInterface::readDataSpace(pid_t pid, Address addr, int nbytes, Address data, int word_len, const char * /*file*/, unsigned int /*line*/) 
{
  dbi_printf("%s[%d][%s]:  welcome to DebuggerInterface::readDataSpace()\n",
          FILE__, __LINE__, getThreadStr(getExecThreadID()));
  getBusy();

  bool ret;
  ReadDataSpaceCallback *cbp = new ReadDataSpaceCallback();
  ReadDataSpaceCallback &cb = *cbp;

  cb.enableDelete(false);
  cb(pid, addr, nbytes, data, word_len);
  ret = cb.getReturnValue();
  cb.enableDelete();

  releaseBusy();
  return ret;
}

bool DBI_readDataSpace(pid_t pid, Address addr, int nelem, Address data, int word_len, const char *file, unsigned int line)
{
  bool ret =  getDBI()->readDataSpace(pid, addr, nelem, data, word_len, file, line);
  if (!ret)
    fprintf(stderr, "%s[%d]:  readDataSpace at %s[%d] failing\n", FILE__, __LINE__, file, line);
  return ret;
}

bool  OS::osKill(int pid) 
{
  return (P_kill(pid,9)==0);
}

void OS::unlink(char *file) 
{
   unlink(file);
}
void OS::make_tempfile(char *s)
{
   int result;
   result = mkstemp(s);
   if (result != -1)
      close(result);
}

bool OS::execute_file(char *path) {
   int result;
   result = system(path);
   if (result == -1) {
      fprintf(stderr, "%s ");
      perror("couldn't be executed");
   }
   return (result != -1);
}

