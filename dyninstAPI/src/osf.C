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

// $Id: osf.C,v 1.40 2003/02/04 15:18:57 bernat Exp $

#include "common/h/headers.h"
#include "os.h"
#include "process.h"
#include "dyn_lwp.h"
#include "stats.h"
#include "common/h/Types.h"
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <fcntl.h>
#include <ldfcn.h>
#include "showerror.h"
#ifndef BPATCH_LIBRARY
#include "main.h"
#endif 
#include <sys/procfs.h>
#include <sys/poll.h>
#include <sys/fault.h>

#include "common/h/osfKludges.h"

#define V0_REGNUM 0	/* retval from integer funcs */
#define PC_REGNUM 31
#define SP_REGNUM 30
#define FP_REGNUM 15
#define A0_REGNUM 16	/* first param to funcs and syscalls */
#define RA_REGNUM 26
extern bool exists_executable(const string &fullpathname);
extern debug_ostream attach_cerr;


int getNumberOfCPUs()
{
  return(1);
}


bool process::emitInferiorRPCheader(void *insnPtr, Address &baseBytes, bool) {

  extern void emitSaveConservative(process *, char *, Address &baseBytes);

  emitSaveConservative(this, (char *) insnPtr, baseBytes);

  return true;
}

bool process::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
				     unsigned &breakOffset,
				     bool stopForResult,
				     unsigned &stopForResultOffset,
				     unsigned &justAfter_stopForResultOffset,
				     /*int thrId,*/
				     bool isMT) {
  instruction *insn = (instruction *)insnPtr;
  Address baseInstruc = baseBytes / sizeof(instruction);


  extern void generate_nop(instruction*);
  extern void generateBreakPoint(instruction &);
  extern void emitRestoreConservative(process *, char *, Address &baseBytes);

  emitRestoreConservative(this, (char *) insnPtr, baseBytes);

  if (stopForResult) {
    generateBreakPoint(insn[baseInstruc]);
    stopForResultOffset = baseInstruc * sizeof(instruction);
    baseInstruc++;

    justAfter_stopForResultOffset = baseInstruc * sizeof(instruction);
  }
  
  // Trap instruction (breakpoint):
  generateBreakPoint(insn[baseInstruc]);
  breakOffset = baseInstruc * sizeof(instruction);
  baseInstruc++;

  // And just to make sure that we don't continue, we put an illegal
  // insn here:
  extern void generateIllegalInsn(instruction &);
  generateIllegalInsn(insn[baseInstruc++]);

  baseBytes = baseInstruc * sizeof(instruction); // convert back

  return true;
}

Address dyn_lwp::readRegister(Register /*reg*/)
{  
  gregset_t theIntRegs;
  if (-1 == ioctl(fd_, PIOCGREG, &theIntRegs)) {
    perror("process::readRegister PIOCGREG");
    if (errno == EBUSY) {
      cerr << "It appears that the process was not stopped in the eyes of /proc" << endl;
      assert(false);
    }
    return 0; // assert(false)?
  }
  return theIntRegs.regs[0];
}

//void OS::osTraceMe(void) { P_ptrace(PT_TRACE_ME, 0, 0, 0); }


// getActiveFrame(): populate Frame object using toplevel frame
Frame dyn_lwp::getActiveFrame()
{
  Address pc, fp;
  Frame theFrame;
  gregset_t theIntRegs;
//  int proc_fd = p->getProcFileDescriptor();
  if (ioctl(fd_, PIOCGREG, &theIntRegs) != -1) {
    fp = theIntRegs.regs[SP_REGNUM];  
    pc = theIntRegs.regs[PC_REGNUM]-4; /* -4 because the PC is updated */
    theFrame = Frame(pc, fp, proc_->getPid(), NULL, this, true);
  }
  return theFrame;
}

#ifdef BPATCH_LIBRARY
/*
 * Use by dyninst to set events we care about from procfs
 *
 */
bool process::setProcfsFlags()
{

  long flags = 0;
  if (BPatch::bpatch->postForkCallback) {
      // cause the child to inherit trap-on-exit from exec and other traps
      // so we can learn of the child (if the user cares)
      flags = PR_FORK | PR_ASYNC | PR_RLC;
  }

  if (ioctl (getDefaultLWP()->get_fd(), PIOCSET, &flags) < 0) {
    fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // cause a stop on the exit from fork
  sysset_t sysset;

  if (ioctl(getDefaultLWP()->get_fd(), PIOCGEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  if (BPatch::bpatch->postForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_execv);
      praddset (&sysset, SYS_execve);
  }
  
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSEXIT, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  // now worry about entry too
  if (ioctl(getDefaultLWP()->get_fd(), PIOCGENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  if (BPatch::bpatch->exitCallback) {
      praddset (&sysset, SYS_exit);
  }

  if (BPatch::bpatch->preForkCallback) {
      praddset (&sysset, SYS_fork);
      praddset (&sysset, SYS_vfork);
      // praddset (&sysset, SYS_waitsys);
  }

  // should these be for exec callback??
  prdelset (&sysset, SYS_execv);
  prdelset (&sysset, SYS_execve);
  
  if (ioctl(getDefaultLWP()->get_fd(), PIOCSENTRY, &sysset) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    return false;
  }

  return true;
}
#endif

static inline bool execResult(prstatus_t stat) 
{
  return (stat.pr_reg.regs[V0_REGNUM] == 0);
}

#ifndef OPEN_MAX
#define OPEN_MAX 1024
#endif


#ifdef BPATCH_LIBRARY
int process::waitforRPC(int *status, bool /* block */)
{
  static struct pollfd fds[OPEN_MAX];  // argument for poll
  static int selected_fds;             // number of selected
  static int curr;                     // the current element of fds

  if (selected_fds == 0) {
      for (unsigned u = 0; u < processVec.size(); u++) {
	if (processVec[u]->status() == running || 
	    processVec[u]->status() == neonatal) {
	    fds[u].fd = processVec[u]->getDefaultLWP()->get_fd();
	    selected_fds++;
	} else {
	  fds[u].fd = -1;
	}
	fds[u].events = 0xffff;
	fds[u].revents = 0;
      }
      curr = 0;
  }
  if (selected_fds > 0)
    {
      while(fds[curr].fd == -1) ++curr;
      prstatus_t stat;
      int ret = 0;
      int flag = 0;
      // Busy wait till the process actually stops on a fault
      while(!flag)
	{
	  if (ioctl(fds[curr].fd,PIOCSTATUS,&stat) != -1 && (stat.pr_flags & PR_STOPPED || stat.pr_flags & PR_ISTOP)) {
	    switch(stat.pr_why) {
	    case PR_FAULTED:
	    case PR_SIGNALLED:
	  *status = SIGTRAP << 8 | 0177;
	  ret = processVec[curr]->getPid();
	  flag = 1;
	  break;
	    }
	  }
	}
      --selected_fds;
      ++curr;
      if (ret > 0) return ret;
    }
  return 0;
}
#endif

// wait for a process to terminate or stop
//   return the pid of the process that has stopped or blocked.
#ifdef BPATCH_LIBRARY
int process::waitProcs(int *status, bool block = false) 
#else
int process::waitProcs(int *status)
#endif
{
    int ret = 0;
    static struct pollfd fds[OPEN_MAX];  // argument for poll
    static int selected_fds;             // number of selected
    static int curr;                     // the current element of fds

#ifndef BPATCH_LIBRARY
    bool block = false;
#endif

    do {
       /* 
	 Each call to poll may return many selected fds. Since we only report 
	 the status of one process per each call to waitProcs, we keep the result of 
	 the last poll buffered, and simply process an element from the buffer until 
	 all of the selected fds in the last poll have been processed.
        */
      
	if (selected_fds <= 0) {
	    selected_fds = 0;
	    for (unsigned u = 0; u < processVec.size(); u++) {
		if ((processVec[u]->status_ == exited)) {
		    fds[u].fd = -1;
		    continue;
		}

		fds[u].events = POLLPRI | POLLIN;
		fds[u].revents = 0;
		if (processVec[u]) {
		     fds[u].fd = processVec[u]->getDefaultLWP()->get_fd();

		     // need to check for this since is doesn't generate a poll event
		     int retWait;
		     if (retWait = waitpid(processVec[u]->getPid(), status, WNOHANG|WNOWAIT)) {
			 if (retWait == -1) {
			    // skip over stopped proceses
			    if ((processVec[u]->status_ == stopped)) continue;

			 }

			 fds[u].revents = POLLPRI;
			 selected_fds++;
			 break;
		     }
		} else {
		    fds[u].fd = -1;
		}
	    }

	    // only poll if no waitpid event was identified
	    if (selected_fds == 0) {
		int timeout;
		// since exit doesn't provide an event, we need to timeout and test via waitpid

		if (block) timeout = 5;
		else timeout = 0;
		selected_fds = poll(fds, processVec.size(), timeout);
	    }
	}

	if (selected_fds < 0) {
	   fprintf(stderr, "waitProcs: poll failed: %s\n", sys_errlist[errno]);
	   selected_fds = 0;
	   return 0;
	}

	curr = 0;
    
    if (selected_fds > 0) {
        while (fds[curr].revents == 0) {
            ++curr;
            if (curr == processVec.size()) abort();
        }
        
        // fds[curr] has an event of interest
        prstatus_t stat;
        int ret = 0;
        
        if (fds[curr].revents & POLLHUP) {
            do {
                ret = waitpid(processVec[curr]->getPid(), status, 0);
            } while ((ret < 0) && (errno == EINTR));
            if (ret < 0) {
                // This means that the application exited, but was not our child
                // so it didn't wait around for us to get it's return code.  In
                // this case, we can't know why it exited or what it's return
                // code was.
                printf("grand child exited\n");
                ret = processVec[curr]->getPid();
                *status = 0;
            }
            ret = processVec[curr]->getPid();
        } else if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) != -1) {
            if (stat.pr_why == PR_DEAD) {
                do {
                    ret = waitpid(processVec[curr]->getPid(), status, 0);
                } while ((ret < 0) && (errno == EINTR));
                if (ret < 0) {
                    // This means that the application exited, but was not our child
                    // so it didn't wait around for us to get it's return code.  In
                    // this case, we can't know why it exited or what it's return
                    // code was.
                    ret = processVec[curr]->getPid();
                    *status = 0;
                }
                assert(ret == processVec[curr]->getPid());
            } else if (stat.pr_flags & PR_STOPPED || stat.pr_flags & PR_ISTOP) {
                switch (stat.pr_why) {
		      case PR_SIGNALLED: {
                  process *p = processVec[curr];
                  // return the signal number
                  *status = stat.pr_what << 8 | 0177;
                  fprintf(stderr, "Process received signal %d\n");
                  
                  ret = processVec[curr]->getPid();
                  break;
		      }
		      case PR_SYSEXIT: {
                  // exit of a system call.
		         process *p = processVec[curr];

			 if (p->isAnyIRPCwaitingForSyscall()) {
			    // reset PIOCSEXIT mask
			    assert(p->save_exitset_ptr != NULL);
			    if (-1 == ioctl(p->getDefaultLWP()->get_fd(), PIOCSEXIT, p->save_exitset_ptr))
			       assert(false);
			    delete [] p->save_exitset_ptr;
			    p->save_exitset_ptr = NULL;

			    // fall through on purpose (so status, ret get set)
			 } else if (((stat.pr_what == SYS_execv) ||
				     (stat.pr_what == SYS_execve)) && !execResult(stat)) {
			   // a failed exec. continue the process
			   processVec[curr]->continueProc_();
			   break;
			 }
#ifdef BPATCH_LIBRARY
			 if (stat.pr_what == SYS_fork) {
			     int childPid = stat.pr_reg.regs[V0_REGNUM];
			     if (childPid > 0) {
				 unsigned int i;

				 for (i=0; i < processVec.size(); i++) {
				     if (processVec[i]->getPid() == childPid) break;
				 }
				 if (i== processVec.size()) {

				     // this is a new child, register it with dyninst
				     int parentPid = processVec[curr]->getPid();


				     process *theParent = processVec[curr];
				     process *theChild = new process(*theParent, (int)childPid, -1);
				     processVec.push_back(theChild);
				     activeProcesses++;


				     // it's really stopped, but we need to mark it running so
				     //   it can report to us that it is stopped - jkh 1/5/00
				     // or should this be exited???
				     theChild->status_ = neonatal;

				     // parent is stopped too (on exit fork event)
				     theParent->status_ = stopped;

				     theChild->execFilePath = 
					 theChild->tryToFindExecutable("", childPid);
				     theChild->inExec = false;
				     BPatch::bpatch->registerForkedThread(parentPid,
					 childPid, theChild);
				     selected_fds = 0;
				 }
			     } else {
				 printf("fork errno %lx\n", stat.pr_reg.regs[V0_REGNUM]);
			     }
			 } else if (((stat.pr_what == SYS_execv) || 
				     (stat.pr_what == SYS_execve)) && execResult(stat)) {
			     process *proc = processVec[curr];
			     proc->execFilePath = proc->tryToFindExecutable("", proc->getPid());

			     // only handle if this is in the child - is this right??? jkh
			     if (proc->reachedBootstrapState(initialized)) {
				 // mark this for the sig TRAP that will occur soon
				 proc->inExec = true;

				 // leave process stopped until signal handler runs
				 // mark it running so we get the stop signal
				 proc->status_ = stopped;

				 // reset buffer pool to empty (exec clears old mappings)
				 pdvector<heapItem *> emptyHeap;
				 proc->heap.bufferPool = emptyHeap;
			     }
			 } else {
			     process *p = processVec[curr];
			     p->status_ = stopped;
			     if (p->handleTrapIfDueToRPC()) {
				 continue;
			     } else {
				 sprintf(errorLine, "got unexpected PIOCSEXIT: ret syscall #%d\n", 
				     (int) stat.pr_what);
				 logLine(errorLine);
				 p->continueProc();
			     }
			 }
#endif
			 // ---------------------------------------------
			 // I think the following two lines are part of
			 // the earlier SIGTRAP hack that I removed.
			 //
			 // Ray Chen 03/22/02
			 // ---------------------------------------------
			 // *status = SIGTRAP << 8 | 0177;
			 // ret = processVec[curr]->getPid();
			 break;
		     }

#ifdef BPATCH_LIBRARY
		     case PR_SYSENTRY: {
			 bool alreadyCont = false;
			 process *p = processVec[curr];

			 if ((stat.pr_what == SYS_fork) || (stat.pr_what == SYS_vfork)) {
			     if (BPatch::bpatch->preForkCallback) {
				 assert(p->thread);
				 p->setProcfsFlags();
				 BPatch::bpatch->preForkCallback(p->thread, NULL);
			     }

			     if (stat.pr_what == SYS_vfork)  {
				 unsigned int i;
				 int childPid = 0;
				 alreadyCont = true;
				 struct DYNINST_bootstrapStruct bootRec;

				 (void) processVec[curr]->continueProc_();
				 processVec[curr]->status_ = stopped;
				 do {
				     processVec[curr]->extractBootstrapStruct(&bootRec);

				     childPid = bootRec.pid;
				 } while (bootRec.event != 3);

				 for (i=0; i < processVec.size(); i++) {
				     if (processVec[i]->getPid() == childPid) break;
				 }
				 if (i== processVec.size()) {
				     // this is a new child, register it with dyninst
				     int parentPid = processVec[curr]->getPid();
				     process *theParent = processVec[curr];
				     process *theChild = new process(*theParent, (int)childPid, -1);
				     processVec.push_back(theChild);
				     activeProcesses++;

				     // it's really stopped, but we need to mark it running so
				     //   it can report to us that it is stopped - jkh 1/5/00
				     // or should this be exited???
				     theChild->status_ = running;

				     theChild->execFilePath = 
					 theChild->tryToFindExecutable("", childPid);
				     BPatch::bpatch->registerForkedThread(parentPid,
					 childPid, theChild);
				     selected_fds = 0;
				 }
			     }
			 } else if (stat.pr_what == SYS_exit) {
			     int code;
			     bool readRet;
			     unsigned long sp;
			     
			     code = stat.pr_reg.regs[A0_REGNUM];
			     BPatch::bpatch->registerExit(processVec[curr]->thread, code);
			 } else {
			     printf("got PR_SYSENTRY\n");
			     printf("    unhandeled sys call #%d\n", (int) stat.pr_what);
			 }
			 if (!alreadyCont) (void) processVec[curr]->continueProc_();
			 break;
		       }
#endif
		      case PR_REQUESTED:
			    stat.pr_flags = PRCSIG;
			    if (ioctl(fds[curr].fd, PIOCRUN, &stat) == -1) {
				fprintf(stderr, "attach: PIOCRUN failed: %s\n",
				      sys_errlist[errno]);
				return false;
			    }
			    break;

		      case PR_FAULTED:
			    *status = SIGTRAP << 8 | 0177;
			    ret = processVec[curr]->getPid();
			    break;

		      case PR_JOBCONTROL:
			    *status = stat.pr_what << 8 | 0177;
			    ret = processVec[curr]->getPid();
			    break;
		}	
	    }
	} else if (ioctl(fds[curr].fd, PIOCSTATUS, &stat) == -1) {
	    // ------------------------------------------------------------
	    // Does this 'else if' look funny to anyone else?  It's the
	    // same condition that we checked earlier in the 'if' tree.
	    // I'm leaving it in because I don't know what it does.
	    //
	    // Ray Chen 03/22/02
	    // ------------------------------------------------------------
	    ret = processVec[curr]->getPid();
	    *status = 0;
	}
    
	// clear the processed event
	fds[curr].revents = 0;
	--selected_fds;

	if (selected_fds > 0) ++curr;

	if (ret > 0) {
	      return ret;
	}

	if (curr == processVec.size()) abort();
      }
    } while(block);
    return ret;
}



/*
   Open the /proc file correspoding to process pid, 
   set the signals to be caught to be only SIGSTOP,
   and set the kill-on-last-close and inherit-on-fork flags.
*/
bool process::attach() {

  dyn_lwp *lwp = new dyn_lwp(0, this);
  if (!lwp->openFD()) {
    delete lwp;
    return false;
  }
  lwps[0] = lwp;
  int fd = lwp->get_fd();

  /* we don't catch any child signals, except SIGSTOP */
  sigset_t sigs;
  fltset_t faults;
  premptyset(&sigs);
  praddset(&sigs, SIGSTOP);
  praddset(&sigs, SIGTRAP);
  praddset(&sigs, SIGSEGV);

  if (ioctl(fd, PIOCSTRACE, &sigs) < 0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  premptyset(&faults);
  praddset(&faults,FLTBPT);
  if (ioctl(fd,PIOCSFAULT,&faults) <0) {
    fprintf(stderr, "attach: ioctl failed: %s\n", sys_errlist[errno]);
    close(fd);
    return false;
  }

  /* turn on the kill-on-last-close and inherit-on-fork flags. This will cause
     the process to be killed when paradynd exits.
     Also, any child of this process will stop at the exit of an exec call.
  */
#if defined(PIOCSET)
  {
    long flags = PR_KLC | PR_FORK;
    if (ioctl (fd, PIOCSET, &flags) < 0) {
      fprintf(stderr, "attach: PIOCSET failed: %s\n", sys_errlist[errno]);
      close(fd);
      return false;
    }
  }
  #else
  #if defined(PRFS_KOLC)
  {
    long pr_flags;
    if (ioctl(fd, PIOCGSPCACT, &pr_flags) < 0) {
      sprintf(errorLine, "Cannot get status\n");
      logLine(errorLine);
      close(fd);
      return false;
    }
    pr_flags |= PRFS_KOLC;
    ioctl(fd, PIOCSSPCACT, &pr_flags);
  }
#endif
#endif

#ifdef BPATCH_LIBRARY
  setProcfsFlags();
#endif
  return true;
}

Frame Frame::getCallerFrame(process *p) const
{
  Frame ret;
  Address values[2];
  gregset_t theIntRegs;
  pd_Function *currFunc;
  instPoint     *ip = NULL;
  trampTemplate *bt = NULL;
  instInstance  *mt = NULL;

  if (fp_ == 0) return Frame();

  if (uppermost_) {
    int proc_fd = p->getDefaultLWP()->get_fd();
    if (ioctl(proc_fd, PIOCGREG, &theIntRegs) != -1) {
      ret.pc_ = theIntRegs.regs[PC_REGNUM];  

      currFunc = p->findAddressInFuncsAndTramps(ret.pc_, &ip, &bt, &mt);
      if (currFunc && currFunc->frame_size) {
	  ret.fp_ = theIntRegs.regs[SP_REGNUM] + currFunc->frame_size;  
	  ret.sp_ = theIntRegs.regs[SP_REGNUM];
	  //printf(" %s fp=%lx\n",currFunc->prettyName().c_str(), ret.fp_);
      } else {
	  ret.fp_ = 0;
	  sprintf(errorLine, "pc %lx, not in a known function\n", ret.pc_);
	  logLine(errorLine);
      }

    } else {
      return Frame(); // zero frame
    }
  } else {
    if (!p->readDataSpace((void *)sp_, sizeof(Address), values, false)){
      printf("error reading frame at %lx\n", fp_);
      return Frame(); // zero frame
    } else {
      // (*sp_) = RA
      // fp_ + frame_size = saved fp
      ret.pc_ = values[0];

      currFunc = p->findAddressInFuncsAndTramps(ret.pc_, &ip, &bt, &mt);
      if (currFunc && currFunc->frame_size) {
	  ret.sp_ = fp_;		/* current stack pointer is old fp */
	  ret.fp_ = fp_ + currFunc->frame_size;  
	  //printf(" %s fp=%lx\n",currFunc->prettyName().c_str(), ret.fp_);
      } else {
	  sprintf(errorLine, "pc %lx, not in a known function\n", ret.pc_);
	  logLine(errorLine);
	  ret.fp_ = 0;
      }
    }
  }
  return ret;
}

bool process::dumpCore_(const string coreFile) 
{
  //fprintf(stderr, ">>> process::dumpCore_()\n");
  bool ret;
#ifdef BPATCH_LIBRARY
  ret = dumpImage(coreFile);
#else
  ret = dumpImage();
#endif
  return ret;

}

string process::tryToFindExecutable(const string &progpath, int pid) 
{
   // returns empty string on failure

   if (exists_executable(progpath)) // util lib
      return progpath;

   char buffer[128];
   sprintf(buffer, "/proc/%05d", pid);
   int procfd = open(buffer, O_RDONLY, 0);
   if (procfd == -1) {
      attach_cerr << "tryToFindExecutable failed since open of /proc failed" << endl;
      return "";
   }
   attach_cerr << "tryToFindExecutable: opened /proc okay" << endl;

   struct prpsinfo the_psinfo;

   if (ioctl(procfd, PIOCPSINFO, &the_psinfo) == -1) {
       close(procfd);
       return "";
   }

   char commandName[256];
   strcpy(commandName, the_psinfo.pr_psargs);
   if (strchr(commandName, ' ')) *(strchr(commandName, ' ')) = '\0';

   if (!access(commandName, X_OK)) {
       // found the file, return the results
       (void)close(procfd);
       return commandName;
   }

   printf("access to  %s failed \n", commandName);
   attach_cerr << "tryToFindExecutable: giving up" << endl;

   (void)close(procfd);

   return ""; // failure
}


//
// Write out the current contents of the text segment to disk.  This is useful
//    for debugging dyninst.
//
#ifdef BPATCH_LIBRARY
bool process::dumpImage(string outFile)
#else
bool process::dumpImage()
#endif
{
#if !defined(BPATCH_LIBRARY)
  string outFile = getImage()->file() + ".real";
#endif
  int i;
  int rd;
  int ifd;
  int ofd;
  int total;
  int length;
  Address baseAddr;
    extern int errno;
    const int COPY_BUF_SIZE = 4*4096;
    char buffer[COPY_BUF_SIZE];
    struct filehdr hdr;
    struct stat statBuf;
    SCNHDR sectHdr;
    LDFILE      *ldptr = NULL;
    image       *im;
    long text_size , text_start,file_ofs;

    im = getImage();
    string origFile = im->file();

    ifd = open(origFile.c_str(), O_RDONLY, 0);
    if (ifd < 0) {
      sprintf(errorLine, "Unable to open %s\n", origFile.c_str());
      logLine(errorLine);
      showErrorCallback(41, (const char *) errorLine);
      perror("open");
      return true;
    }

    rd = fstat(ifd, &statBuf);
    if (rd != 0) {
      perror("fstat");
      sprintf(errorLine, "Unable to stat %s\n", origFile.c_str());
      logLine(errorLine);
      showErrorCallback(72, (const char *) errorLine);
      return true;
    }
    length = statBuf.st_size;

    sprintf(errorLine, "saving program to %s\n", outFile.c_str());
    logLine(errorLine);

    ofd = open(outFile.c_str(), O_WRONLY|O_CREAT, 0777);
    if (ofd < 0) {
      perror("open");
      exit(-1);
    }

    /* read header and section headers */
    /* Uses ldopen to parse the section headers */
    /* try */ 
    if (!(ldptr = ldopen((char *) origFile.c_str(), ldptr))) {
       perror("Error in Open");
       exit(-1);
     }
     
     if (TYPE(ldptr) != ALPHAMAGIC) {
       printf("%s is not an alpha executable\n", outFile.c_str());
       exit(-1);
     }
     // Read the text and data sections
     hdr = HEADER(ldptr);
     /* Find text segment and then */
     /* compute text segment length and start offset */
     for (int k=0;k<hdr.f_nscns;k++) {
	 if (ldshread(ldptr, k , &sectHdr) == SUCCESS) {
	   // sprintf(errorLine,"Section: %s  Start: %ld ",sectHdr.s_name,
	   //  sectHdr.s_vaddr); 
	   // logLine(errorLine);
	   // cout << "Section: " << sectHdr.s_name << "\tStart: " << sectHdr.s_vaddr 
	   // << "\tEnd: " << sectHdr.s_vaddr + sectHdr.s_size << endl;
	   // cout.flush();
	 } else {
	     perror("Error reading section header");
	     exit(-1);
	 }

	 if (!P_strcmp(sectHdr.s_name, ".text")) {
	   text_size = sectHdr.s_size;
	   text_start = sectHdr.s_vaddr;
	   file_ofs = sectHdr.s_scnptr;
	 }
       }
     ldclose(ldptr);
    /* ---------end section header read ------------*/

    /* now copy the entire file */
    lseek(ofd, 0, SEEK_SET);
    lseek(ifd, 0, SEEK_SET);
    for (i=0; i < length; i += COPY_BUF_SIZE) {
        rd = read(ifd, buffer, COPY_BUF_SIZE);
        write(ofd, buffer, rd);
        total += rd;
    }

    baseAddr = (Address) text_start;
    sprintf(errorLine, "seeking to %ld as the offset of the text segment \n",
	file_ofs);
    logLine(errorLine);
    sprintf(errorLine, " code offset= %ld\n", baseAddr);
    logLine(errorLine);

    /* seek to the text segment */
    lseek(ofd,(off_t)file_ofs, SEEK_SET);
    for (i=0; i < text_size; i+= 1024) {
        errno = 0;
        length = ((i + 1024) < text_size) ? 1024 : text_size -i;
        if (lseek(getDefaultLWP()->get_fd(),(off_t)(baseAddr + i), SEEK_SET) != (long)(baseAddr + i)) {
	    fprintf(stderr,"Error_:%s\n",sys_errlist[errno]);
	    fprintf(stderr,"[%d] Couldn't lseek to the designated point\n",i);
	}
	read(getDefaultLWP()->get_fd(),buffer,length);
	write(ofd, buffer, length);
    }

    close(ofd);
    close(ifd);

    return true;
}

#ifdef BPATCH_LIBRARY
/*
   terminate execution of a process
 */
bool process::terminateProc_()
{
    long flags = PRFS_KOLC;
    if (ioctl (getDefaultLWP()->get_fd(), PIOCSSPCACT, &flags) < 0)
        return false;

    // just to make sure it is dead
    kill(getPid(), 9);

    Exited();

    return true;
}
#endif



#if !defined(BPATCH_LIBRARY)
rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  return 0;
}

/* return unit: nsecs */
rawTime64 dyn_lwp::getRawCpuTime_sw() 
{
  // returns user+sys time from the u or proc area of the inferior process,
  // which in turn is presumably obtained by mmapping it (sunos)
  // or by using a /proc ioctl to obtain it (solaris).
  // It must not stop the inferior process in order to obtain the result,
  // nor can it assue that the inferior has been stopped.
  // The result MUST be "in sync" with rtinst's DYNINSTgetCPUtime().
  
  // We use the PIOCUSAGE /proc ioctl
  
  // Other /proc ioctls that should work too: PIOCPSINFO and the
  // lower-level PIOCGETPR and PIOCGETU which return copies of the proc
  // and u areas, respectively.
  // PIOCSTATUS does _not_ work because its results are not in sync
  // with DYNINSTgetCPUtime
  
  rawTime64 now;
  
  prpsinfo_t procinfo;
  
  if (ioctl(fd_, PIOCPSINFO, &procinfo) == -1) {
    perror("process::getInferiorProcessCPUtime - PIOCPSINFO");
    abort();
  }
  
  /* Put secs and nsecs into usecs */
  now = procinfo.pr_time.tv_sec;
  now *= I64_C(1000000000);
  now += procinfo.pr_time.tv_nsec;
  
  if (now<sw_previous_) {
    // time shouldn't go backwards, but we have seen this happening
    // before, so we better check it just in case - naim 5/30/97
    logLine("********* time going backwards in paradynd **********\n");
    now=sw_previous_;
  }
  else {
    sw_previous_=now;
  }
  
  return now;
}
#endif

fileDescriptor *getExecFileDescriptor(string filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

#ifndef BPATCH_LIBRARY
void process::initCpuTimeMgrPlt() {
  cpuTimeMgr->installLevel(cpuTimeMgr_t::LEVEL_TWO, &process::yesAvail, 
			   timeUnit::ns(), timeBase::bNone(), 
			   &process::getRawCpuTime_sw, "DYNINSTgetCPUtime_sw");
}
#endif

bool dyn_lwp::openFD()
{
  char procName[128];    
  sprintf(procName, "/proc/%d", (int)proc_->getPid());
  fd_ = P_open(procName, O_RDWR, 0);
  if (fd_ == (unsigned) -1) {
    perror("Error opening process file descriptor");
    return false;
  }
  return true;
}

void dyn_lwp::closeFD()
{
  if (fd_) close(fd_);
}


