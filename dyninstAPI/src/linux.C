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

// $Id: linux.C,v 1.131 2004/03/15 01:52:20 tlmiller Exp $

#include <fstream>

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/dyn_lwp.h"

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

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/signalhandler.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/stats.h"
#include "common/h/Types.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/util.h" // getCurrWallTime
#include "common/h/pathName.h"
#ifndef BPATCH_LIBRARY
#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/init.h"
#endif

#include "dyninstAPI/src/addLibraryLinux.h"
#include "dyninstAPI/src/writeBackElf.h"
// #include "saveSharedLibrary.h" 

#ifdef PAPI
#include "papi.h"
#endif

// The following were defined in process.C
// Shouldn't they be in a header, then? -- TLM

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

extern void generateBreakPoint(instruction &insn);

#if defined(PTRACEDEBUG) && !defined(PTRACEDEBUG_ALWAYS)
static bool debug_ptrace = false;
#endif

bool dyn_lwp::deliverPtrace(int request, Address addr, Address data) {
   bool needToCont = false;
  
   if(request != PT_DETACH  &&  status() == running) {
      cerr << "  potential performance problem with use of dyn_lwp::deliverPtrace\n";
      if(pauseLWP() == true)
         needToCont = true;
   }

   bool ret = (P_ptrace(request, get_lwp_id(), addr, data) != -1);
   if (!ret) perror("Internal ptrace");
   
   if(request != PTRACE_DETACH  &&  needToCont == true)
      continueLWP();
         
   return ret;
}


// Some ptrace requests in Linux return the value rather than storing at the address in data
// (especially PEEK requests)
// - nash
int dyn_lwp::deliverPtraceReturn(int request, Address addr, Address data) {
   bool needToCont = false;
  
   if(request != PT_DETACH  &&  status() == running) {
      cerr << "  potential performance problem with use of "
           << "dyn_lwp::deliverPtraceReturn\n";
      if(pauseLWP() == true)
         needToCont = true;
   }

   int ret = P_ptrace(request, get_lwp_id(), addr, data);

   if(request != PTRACE_DETACH  &&  needToCont == true)
      continueLWP();
   
   return ret;
}

/* ********************************************************************** */

void printStackWalk( process *p ) {
  dyn_lwp *lwp_to_use = NULL;
  if(process::IndependentLwpControl() && p->getRepresentativeLWP() ==NULL)
     lwp_to_use = p->getInitialThread()->get_lwp();
  else
     lwp_to_use = p->getRepresentativeLWP();

  Frame theFrame = lwp_to_use->getActiveFrame();
  while (true) {
    // do we have a match?
    const Address framePC = theFrame.getPC();
    inferiorrpc_cerr << "stack frame pc @ " << (void*)framePC << endl;
    
    if (theFrame.isLastFrame(p))
      // well, we've gone as far as we can, with no match.
      break;
    
    // else, backtrace 1 more level
    theFrame = theFrame.getCallerFrame(p);
  }
}
 
// already setup on this FD.
// disconnect from controlling terminal 
void OS::osDisconnect(void) {
  int ttyfd = open ("/dev/tty", O_RDONLY);
  ioctl (ttyfd, TIOCNOTTY, NULL);
  P_close (ttyfd);
}

// may be needed in the future
#if defined(DECODEANDHANDLE_EVENT_ON_LWP)
// returns true if it handled an event
bool checkForAndHandleProcessEventOnLwp(bool block, dyn_lwp *lwp) {
   pdvector<procevent *> foundEvents;
   getSH()->checkForProcessEvents(&foundEvents, lwp->get_lwp_id(), block);
   getSH()->handleProcessEvents(foundEvents);
   //assert(selectedLWP == lwp);
   
   return true;
}
#endif

bool dyn_lwp::stop_() {
   return (P_kill(get_lwp_id(), SIGSTOP) != -1); 
}

void OS::osTraceMe(void) { P_ptrace(PTRACE_TRACEME, 0, 0, 0); }

// Wait for a process event to occur, then map it into
// the why/what space (a la /proc status variables)

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
        what = SYS_fork;
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

// returns true if got event, false otherwise
bool checkForEventLinux(procevent *new_event, int wait_arg, 
                        bool block, int wait_options)
{
   int result = 0, status = 0;

   if (!block) {
      wait_options |= WNOHANG;
   }

	/* If we're blocking, check to make sure we don't do so forever. */
	if( block ) {
		/* If we're waiting on just one process, only check it. */
		if( wait_arg > 0 ) { 
			process * proc = process::findProcess( wait_arg );
			assert( proc != NULL );
			
			if( proc->status() == exited ) { return false; }
			}
		else {
			/* Iterate over all the processes. */
			for( unsigned i = 0; i < processVec.size(); i++ ) {
				if( processVec[i] != NULL ) {
					process * proc = processVec[i];
					
					if( proc->status() == exited ) { return false; }
					}
				}
			} /* end multiple-process wait */
		} /* end if we're blocking */
		
   result = waitpid( wait_arg, &status, wait_options );

   if (result < 0 && errno == ECHILD) {
      return false; /* nothing to wait for */
   } else if (result < 0) {
      perror("checkForEventLinux: waitpid failure");
   } else if(result == 0)
      return false;

   int pertinentPid = result;

   // Translate the signal into a why/what combo.
   // We can fake results here as well: translate a stop in fork
   // to a (SYSEXIT,fork) pair. Don't do that yet.
   process *pertinentProc = process::findProcess(pertinentPid);
   dyn_lwp *pertinentLWP  = NULL;

   if(pertinentProc) {
      // Got a signal, process is stopped.
      if(process::IndependentLwpControl() &&
         pertinentProc->getRepresentativeLWP() == NULL) {
         pertinentLWP = pertinentProc->getInitialThread()->get_lwp();
      } else
         pertinentLWP = pertinentProc->getRepresentativeLWP();

      pertinentProc->set_lwp_status(pertinentLWP, stopped);
   } else {
      extern pdvector<process*> processVec;
      for (unsigned u = 0; u < processVec.size(); u++) {
         process *curproc = processVec[u];
         if(! curproc)
            continue;
         dyn_lwp *curlwp = NULL;
         if( (curlwp = curproc->lookupLWP(pertinentPid)) ) {
            pertinentProc = curproc;
            pertinentLWP  = curlwp;
            pertinentProc->set_lwp_status(curlwp, stopped);
            break;
         }
      }
   }
   
   procSignalWhy_t  why  = procUndefined;
   procSignalWhat_t what = 0;
   procSignalInfo_t info = 0;
   
   if (WIFEXITED(status)) {
      // Process exited via signal
      why = procExitedNormally;
      what = WEXITSTATUS(status);
   }
   else if (WIFSIGNALED(status)) {
      why = procExitedViaSignal;
      what = WTERMSIG(status);
   }
   else if (WIFSTOPPED(status)) {
      // More interesting (and common) case.  This is where return value
      // faking would occur as well. procSignalled is a generic return.  For
      // example, we translate SIGILL to SIGTRAP in a few cases
      why = procSignalled;
      what = WSTOPSIG(status);

      switch(what) {
        case SIGSTOP:
           decodeRTSignal(pertinentProc, why, what, info);
           break;
        case SIGTRAP:
           // We use int03s (traps) to do instrumentation when there
           // isn't enough room to insert a branch.
           why = procInstPointTrap;
           break;
        case SIGCHLD:
           // Linux fork() sends a SIGCHLD once the fork has been created
           why = procForkSigChild;
           break;
        case SIGILL:
           // The following is more correct, but breaks.  Problem is getting
           // the frame requires a ptrace...  which calls
           // loopUntilStopped. Which calls us.
           //Frame frame = proc->getDefaultLWP()->getActiveFrame();
           //Address pc = frame.getPC();
           Address pc = getPC(pertinentPid);
           
           if(pc == pertinentProc->dyninstlib_brk_addr ||
              pc == pertinentProc->main_brk_addr || 
              pertinentProc->getDyn()->reachedLibHook(pc)) {
               what = SIGTRAP;
           }
           break;
      }
   }

   if(! pertinentProc)
      return false;

   (*new_event).proc = pertinentProc;
   (*new_event).lwp  = pertinentLWP;
   (*new_event).why  = why;
   (*new_event).what = what;
   (*new_event).info = info;
   return true;
}

bool signalHandler::checkForProcessEvents(pdvector<procevent *> *events,
                                          int wait_arg, bool block)
{
   bool wait_on_initial_lwp = false;
   bool wait_on_spawned_lwp = false;

   if(wait_arg != -1) {
      if(process::findProcess(wait_arg))
         wait_on_initial_lwp = true;
      else wait_on_spawned_lwp = true;
   } else {
      wait_on_initial_lwp = true;
      wait_on_spawned_lwp = true;
   }

   procevent *new_event = new procevent;
   bool gotevent = false;
   while(1) {
      if(wait_on_initial_lwp) {
         gotevent = checkForEventLinux(new_event, wait_arg, false, 0);
         if(gotevent)
            break;
      }
      if(wait_on_spawned_lwp) {
         gotevent = checkForEventLinux(new_event, wait_arg, false, 
                                             __WCLONE);
         if(gotevent)
            break;
      }
      if(! block) {
         // no event found
         delete new_event;
         break;
      } else {
         // a slight delay to lesson impact of spinning.  this is
         // particularly noticable when traps are hit at instrumentation
         // points (seems to occur frequently in test1).
         // *** important for performance ***
         struct timeval timeout;
         timeout.tv_sec = 0;
         timeout.tv_usec = 1;
         select(0, NULL, NULL, NULL, &timeout);
      }
   }

   if(gotevent) {
      (*events).push_back(new_event);
      return true;
   } else
      return false;
}

void process::independentLwpControlInit() {
   if(multithread_capable()) {
      // On linux, if process found to be MT, there will be no
      // representativeLWP since there is no lwp which controls the entire
      // process for MT linux.
      real_lwps[representativeLWP->get_lwp_id()] = representativeLWP;
      representativeLWP = NULL;
   }
}

dyn_lwp *process::createRepresentativeLWP() {
   // the initial lwp has a lwp_id with the value of the pid

   // if we identify this linux process as multi-threaded, then later we will
   // adjust this lwp to be identified as a real lwp.
   dyn_lwp *initialLWP = createFictionalLWP(getPid());
   representativeLWP = initialLWP;
   // Though on linux, if process found to be MT, there will be no
   // representativeLWP since there is no lwp which controls the entire
   // process for MT linux.

   return initialLWP;
}

bool process::trapAtEntryPointOfMain(Address)
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
  if( getPC(getPid()) == (Address)dyninstlib_brk_addr){
	  dyninstlib_brk_addr = 0; //ccw 30 apr 2002 : SPLIT3
	  //dyninstlib_brk_addr and paradynlib_brk_addr may be the same
	  //if they are we dont want to get them mixed up. once we
	  //see this trap is due to dyninst, reset the addr so
	  //we can now catch the paradyn trap
    return(true);
  } else{
    return(false);
  }
}

bool process::setProcessFlags() {
    // None that I'm aware of -- bernat, 24FEB04
    return true;
}

bool process::unsetProcessFlags(){
    // As above
    return true;
}


void emitCallRel32(unsigned disp32, unsigned char *&insn);

/* Input P points to a buffer containing the contents of
   /proc/PID/stat.  This buffer will be modified.  Output STATUS is
   the status field (field 3), and output SIGPEND is the pending
   signals field (field 31).  As of Linux 2.2, the format of this file
   is defined in /usr/src/linux/fs/proc/array.c (get_stat). */
static void parse_procstat_status(char *statfile, char *status) {
   char *t = strstr(statfile, "State:");
   assert(t);
   char *stat_str = t + 7;
   
   *status = stat_str[0];
}

static bool parse_procstat_pending(char *statfile, unsigned long *shgsigpend) {
     /* Advance to sigpending int */
   char *t = strstr(statfile, "ShdPnd:");
   if(! t)
      return false;

   char *res = t+8;
   res[16] = 0;
   *shgsigpend = strtoul(res, NULL, 8);
   return true;
}

bool dyn_lwp::isRunning() const {
   int fd;
   char name[32];
   char buf[400];
   char status;
   
   sprintf(name, "/proc/%d/status", get_lwp_id());

   /* Read current contents of /proc/pid/status */
   fd = open(name, O_RDONLY);
   assert(fd >= 0);
   assert(read(fd, buf, sizeof(buf)) > 0);
   close(fd);
   buf[399] = 0;

   /* Parse status and pending signals */
   parse_procstat_status(buf, &status);   
   //cerr << "  isRunning, got status: " << status << endl;

   if(status == 'T') {
      return false;
   } else {
      return true;
   }
}

bool dyn_lwp::isStopPending() const {
   int fd;
   char name[32];
   char buf[400];
   unsigned long shdsigpend;
   
   sprintf(name, "/proc/%d/status", get_lwp_id());

   /* Read current contents of /proc/pid/status */
   fd = open(name, O_RDONLY);
   assert(fd >= 0);
   assert(read(fd, buf, sizeof(buf)) > 0);
   close(fd);
   buf[399] = 0;
   
   /* Parse status and pending signals */
   bool couldparse = parse_procstat_pending(buf, &shdsigpend);

   if(couldparse) {
      if(shdsigpend & 040000) {
         return true;
      }
   }

   return false;
}

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
bool dyn_lwp::continueLWP_(int signalToContinueWith) {
   // we don't want to operate on the process in this state
   ptraceOps++; 
   ptraceOtherOps++;

   int arg3 = 0;
   int arg4 = 0;
   if(signalToContinueWith != dyn_lwp::NoSignal) {
      arg3 = 1;
      arg4 = signalToContinueWith;
   }

   int ret = P_ptrace(PTRACE_CONT, get_lwp_id(), arg3, arg4);
   if (ret == -1) {
      perror("continueProc_()");
      return false;
   }

   return true;
}

bool process::terminateProc_()
{
   if( kill( getPid(), SIGKILL ) != 0 )
      return false;
   else
      return true;
}

bool process::waitUntilStopped() {
   bool result = true;
   pdvector<dyn_thread *>::iterator iter = threads.begin();

   while(iter != threads.end()) {
      dyn_thread *thr = *(iter);
      dyn_lwp *lwp = thr->get_lwp();
      assert(lwp);

      if(! lwp->waitUntilStopped()) {
         result = false;
      }
      iter++;
   }

   return result;
}

bool waitUntilStoppedGeneral(dyn_lwp *lwp, int options) {
   /* make sure the process is stopped in the eyes of ptrace */
   bool haveStopped = false;
    
   // Loop handling signals until we receive a sigstop. Handle
   // anything else.
   int loopCt = 0;
   while (!haveStopped) {
      if(lwp->proc()->hasExited()) {
         return false;
      }
      ++loopCt;

      if(loopCt>10000) {
         lwp->stop_();
         loopCt = 0;
      }

      procevent new_event;
      bool gotevent = checkForEventLinux(&new_event, lwp->get_lwp_id(), false,
                                         options);
      if(gotevent) {
         if(didProcReceiveSignal(new_event.why) && new_event.what == SIGSTOP) {
            dyn_lwp *lwp_for_event = NULL;
            if(new_event.lwp != NULL) {
               lwp_for_event = new_event.lwp;
            }

            if(lwp_for_event == lwp) {
               haveStopped = true;
            }
            // Don't call the general handler
         }
         else {
            getSH()->handleProcessEvent(new_event);
         }
      }
      // a slight delay to lesson impact of spinning
      // *** important for performance ***
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 1;
      select(0, NULL, NULL, NULL, &timeout);
   }
   
   return true;
}

bool dyn_lwp::waitUntilStopped() {
   bool res;
   if(proc_->getInitialThread()->get_lwp() == this) {
      res = waitUntilStoppedGeneral(this, 0);
   } else {
      res = waitUntilStoppedGeneral(this, __WCLONE);
   }

   return res;
}


void dyn_lwp::realLWP_detach_() {
    if(! proc_->isAttached()) {
        cerr << "Detaching, but not attached" << endl;
        return;
    }
    
    cerr <<"Detaching..." << endl;
    ptraceOps++;
    ptraceOtherOps++;
    fprintf(stderr, "%d\n", deliverPtrace(PTRACE_DETACH, 1, SIGCONT));
    
    return;
}

void dyn_lwp::representativeLWP_detach_() {
    // If the process is already exited, then don't call ptrace
    if(! proc_->isAttached())
        return;
    
    if (fd_) close(fd_);
    
    ptraceOps++;
    ptraceOtherOps++;
    deliverPtrace(PTRACE_DETACH, 1, SIGCONT);
    return;
}

bool process::dumpCore_(const pdstring/* coreFile*/) { return false; }

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
   return readDataSpace(inTraced, amount, const_cast<void*>( inSelf ));
}

bool dyn_lwp::writeDataSpace(void *inTraced, u_int nbytes, const void *inSelf)
{
   unsigned char *ap = (unsigned char*) inTraced;
   const unsigned char *dp = (const unsigned char*) inSelf;
   Address w;               /* ptrace I/O buffer */
   unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
   unsigned cnt;

#if defined(BPATCH_LIBRARY)
#if defined(i386_unknown_linux2_0)
   if (proc_->collectSaveWorldData) {
       codeRange *range = proc_->findCodeRangeByAddress((Address)inTraced);
       if (range &&
           range->sharedobject_ptr) {
           // If we're writing into a shared object, mark it as dirty.
           // _Unless_ we're writing "__libc_sigaction"
           if ((!range->function_ptr) ||
               (range->function_ptr->prettyName() != "__libc_sigaction"))
               range->sharedobject_ptr->setDirty();
       }
   }
#endif
#endif

   ptraceOps++; ptraceBytes += nbytes;

   if (0 == nbytes)
      return true;

   if ((cnt = ((Address)ap) % len)) {
      /* Start of request is not aligned. */
      unsigned char *p = (unsigned char*) &w;	  

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write the segment back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) (ap-cnt), 0);

      if (errno)
         return false;

      for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
         p[cnt+i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) (ap-cnt), w))
         return false;

      if (len-cnt >= nbytes) 
         return true; /* done */
	  
      dp += len-cnt;
      ap += len-cnt;
      nbytes -= len-cnt;
   }	  
	  
   /* Copy aligned portion */
   while (nbytes >= len) {
      assert(0 == ((Address)ap) % len);
      memcpy(&w, dp, len);
      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w))
         return false;
      dp += len;
      ap += len;
      nbytes -= len;
   }

   if (nbytes > 0) {
      /* Some unaligned data remains */
      unsigned char *p = (unsigned char *) &w;

      /* Read the segment containing the unaligned portion, edit
         in the data from DP, and write it back. */
      errno = 0;
      w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);

      if (errno)
         return false;

      for (unsigned i = 0; i < nbytes; i++)
         p[i] = dp[i];

      if (0 > P_ptrace(PTRACE_POKETEXT, get_lwp_id(), (Address) ap, w))
         return false;

   }

   return true;
}

bool dyn_lwp::readDataSpace(const void *inTraced, u_int nbytes, void *inSelf) {
     const unsigned char *ap = (const unsigned char*) inTraced;
     unsigned char *dp = (unsigned char*) inSelf;
     Address w;               /* ptrace I/O buffer */
     unsigned len = sizeof(w); /* address alignment of ptrace I/O requests */
     unsigned cnt;

     ptraceOps++; ptraceBytes += nbytes;

     if (0 == nbytes)
	  return true;

     if ((cnt = ((Address)ap) % len)) {
	  /* Start of request is not aligned. */
	  unsigned char *p = (unsigned char*) &w;

	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) (ap-cnt), w);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < len-cnt && i < nbytes; i++)
	       dp[i] = p[cnt+i];

	  if (len-cnt >= nbytes)
	       return true; /* done */

	  dp += len-cnt;
	  ap += len-cnt;
	  nbytes -= len-cnt;
     }

     /* Copy aligned portion */
     while (nbytes >= len) {
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);
	  if (errno)
	       return false;
	  memcpy(dp, &w, len);
	  dp += len;
	  ap += len;
	  nbytes -= len;
     }

     if (nbytes > 0) {
	  /* Some unaligned data remains */
	  unsigned char *p = (unsigned char *) &w;
	  
	  /* Read the segment containing the unaligned portion, and
             copy what was requested to DP. */
	  errno = 0;
	  w = P_ptrace(PTRACE_PEEKTEXT, get_lwp_id(), (Address) ap, 0);
	  if (errno)
	       return false;
	  for (unsigned i = 0; i < nbytes; i++)
	       dp[i] = p[i];
     }

     return true;
}

// You know, /proc/*/exe is a perfectly good link (directly to the inode) to
// the executable file, who cares where the executable really is, we can open
// this link. - nash
pdstring process::tryToFindExecutable(const pdstring & /* iprogpath */, int pid) {
  return pdstring("/proc/") + pdstring(pid) + "/exe";
}

void process::recognize_threads(pdvector<unsigned> * /*completed_lwps*/) {
   // implement when handling forks for linux multi-threaded programs
}

#if !defined(BPATCH_LIBRARY)
#ifdef PAPI
papiMgr* dyn_lwp::papi() {

  return proc()->getPapiMgr();

}
#endif
#endif


#if !defined(BPATCH_LIBRARY)

rawTime64 dyn_lwp::getRawCpuTime_hw()
{
  rawTime64 result = 0;
  
#ifdef PAPI
  result = papi()->getCurrentVirtCycles();
#endif
  
  if (result < hw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = hw_previous_;
  }
  else 
    hw_previous_ = result;
  
  return result;
}

rawTime64 dyn_lwp::getRawCpuTime_sw()
{
  rawTime64 result = 0;
  int bufsize = 150, utime, stime;
  char procfn[bufsize], *buf;

  sprintf( procfn, "/proc/%d/stat", get_lwp_id());

  int fd;

  // The reason for this complicated method of reading and sseekf-ing is
  // to ensure that we read enough of the buffer 'atomically' to make sure
  // the data is consistent.  Is this necessary?  I *think* so. - nash
  do {
    fd = P_open(procfn, O_RDONLY, 0);
    if (fd < 0) {
      shmsample_cerr << "getInferiorProcessCPUtime: open failed: " << sys_errlist[errno] << endl;
      return false;
    }

    buf = new char[ bufsize ];

    if ((int)P_read( fd, buf, bufsize-1 ) < 0) {
      perror("getInferiorProcessCPUtime");
      return false;
    }

    if(2==sscanf(buf,"%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %d %d "
		 , &utime, &stime ) ) {
      // These numbers are in 'jiffies' or timeslices.
      // Oh, and I'm also assuming that process time includes system time
      result = static_cast<rawTime64>(utime) + static_cast<rawTime64>(stime);
      break;
    }

    delete [] buf;
    shmsample_cerr << "Inferior CPU time buffer expansion (" << bufsize << ")" << endl;
    bufsize = bufsize * 2;

    P_close( fd );
  } while ( true );

  delete [] buf;
  P_close(fd);

  if (result < sw_previous_) {
    logLine("********* time going backwards in paradynd **********\n");
    result = sw_previous_;
  }
  else 
    sw_previous_ = result;

  return result;
}

class instReqNode;

bool process::catchupSideEffect( Frame & /* frame */, instReqNode * /* inst */ )
{
  return true;
}
#endif

procSyscall_t decodeSyscall(process * /*p*/, procSignalWhat_t what)
{
    switch (what) {
  case SYS_fork:
      return procSysFork;
      break;
  case SYS_exec:
      return procSysExec;
      break;
  case SYS_exit:
      return procSysExit;
      break;
  default:
      return procSysOther;
      break;
    }
    assert(0);
    return procSysOther;
}

bool process::dumpImage( pdstring imageFileName ) {
	/* What we do is duplicate the original file,
	   and replace the copy's .text section with
	   the (presumably instrumented) in-memory
	   executable image.  Note that we don't seem
	   to be concerned with making sure that we
	   actually grab the instrumentation code itself... */
	
	/* Acquire the filename. */
	image * theImage = getImage();
	pdstring originalFileName = theImage->file();
	
	/* Use system() to execute the copy. */
	pdstring copyCommand = "cp " + originalFileName + " " + imageFileName;
    system( copyCommand.c_str() );

	/* Open the copy so we can use libelf to find the .text section. */
	int copyFD = open( imageFileName.c_str(), O_RDWR, 0 );
	if( copyFD < 0 ) { return false; }

	/* Start up the elven widgetry. */
	Elf * elfPointer = elf_begin( copyFD, ELF_C_READ, NULL );
	char * elfIdent = elf_getident( elfPointer, NULL );
	char elfClass = elfIdent[ EI_CLASS ];
	
	bool is64Bits;
	switch( elfClass ) {
		case ELFCLASSNONE:
			/* Shouldn't happen. */
			elf_end( elfPointer );
			close( copyFD );
			return false;

		case ELFCLASS32:
			is64Bits = false;
			break;
			
		case ELFCLASS64:
			is64Bits = true;
			break;
			
		default:
			/* Can't happen. */
			assert( 0 );
			break;
		} /* end elfClass switch */

    /* Acquire the shared names pointer. */
    const char * sharedNames = NULL;
    if( is64Bits ) {
    	Elf64_Ehdr * elfHeader = elf64_getehdr( elfPointer );
    	assert( elfHeader != NULL );
    	
    	Elf_Scn * elfSection = elf_getscn( elfPointer, elfHeader->e_shstrndx );
    	assert( elfSection != NULL );

    	Elf_Data * elfData = elf_getdata( elfSection, 0 );
    	assert( elfData != NULL );
    	    	    	
    	sharedNames = (const char *) elfData->d_buf;
    	}
    else {
    	Elf32_Ehdr * elfHeader = elf32_getehdr( elfPointer );
    	assert( elfHeader != NULL );
    	
    	Elf_Scn * elfSection = elf_getscn( elfPointer, elfHeader->e_shstrndx );
    	assert( elfSection != NULL );

    	Elf_Data * elfData = elf_getdata( elfSection, 0 );
    	assert( elfData != NULL );
    	    	    	
    	sharedNames = (const char *) elfData->d_buf;
		}   
    
	/* Iterate over the sections to find the text section's
	   offset, length, and base address. */
	Address offset = 0;
	Address length = 0;
	Address baseAddr = 0;
	   
	Elf_Scn * elfSection = NULL;
	while( (elfSection = elf_nextscn( elfPointer, elfSection )) != NULL ) {
		if( is64Bits ) {
			Elf64_Shdr * elfSectionHeader = elf64_getshdr( elfSection );
			const char * name = (const char *) & sharedNames[ elfSectionHeader->sh_name ];

			if( strcmp( name, ".text" ) == 0 ) {
				offset = elfSectionHeader->sh_offset;
				length = elfSectionHeader->sh_size;
				baseAddr = elfSectionHeader->sh_addr;
				break;
				} /* end if we've found the text section */
			} else {
			Elf32_Shdr * elfSectionHeader = elf32_getshdr( elfSection );
			const char * name = (const char *) & sharedNames[ elfSectionHeader->sh_name ];

			if( strcmp( name, ".text" ) == 0 ) {
				offset = elfSectionHeader->sh_offset;
				length = elfSectionHeader->sh_size;
				baseAddr = elfSectionHeader->sh_addr;
				break;
				} /* end if we've found the text section */
			}
		} /* end iteration over sections */

	/* Copy the code out of the mutatee. */
	char * codeBuffer = (char *)malloc( length );
	assert( codeBuffer != NULL );

	if( ! readTextSpace( (void *) baseAddr, length, codeBuffer ) ) {
		free( codeBuffer );
		elf_end( elfPointer );
		close( copyFD );
		return false;
		}

	/* Write that code to the image file. */
    lseek( copyFD, offset, SEEK_SET );
    write( copyFD, codeBuffer, length );
    
    /* Clean up. */
    free( codeBuffer );
    elf_end( elfPointer );
    close( copyFD );
    return true;
}

int getNumberOfCPUs()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
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

   if((target = static_cast<function_base *>(instr.getCallee()))) {
      return true; // callee already set
   }
   
   // find the corresponding image in this process  
   const image *owner = instr.getOwner();
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
   //    Address insn_addr = instr.pointAddr(); 
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
   if( (pdf = this->findFuncByAddr(target_addr))) {
       target = pdf;
       instr.setCallee(pdf);
       return true; // target found...target is in this image
   }
   
   // else, get the relocation information for this image
   const Object &obj = owner->getObject();
   const pdvector<relocationEntry> *fbt;
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
            instr.setCallee(target_pdf);
            return true;  // target has been bound
         } 
         else {
	    pdvector<function_base *> pdfv;
	    bool found = findAllFuncsByName((*fbt)[i].name(), pdfv);
	    if(found) {
	       assert(pdfv.size());
#ifdef BPATCH_LIBRARY
	       if(pdfv.size() > 1)
		   cerr << __FILE__ << ":" << __LINE__ 
			<< ": WARNING:  findAllFuncsByName found " 
			<< pdfv.size() << " references to function " 
			<< (*fbt)[i].name() << ".  Using the first.\n";
#endif
	       target = pdfv[0];
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

               pdstring s("_");
	       s += (*fbt)[i].name();
	       found = findAllFuncsByName(s, pdfv);
	       if(found) {
		  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
		  if(pdfv.size() > 1)
		     cerr << __FILE__ << ":" << __LINE__ 
			  << ": WARNING: findAllFuncsByName found " 
			  << pdfv.size() << " references to function " 
			  << s << ".  Using the first.\n";
#endif
		  target = pdfv[0];
		  return true;
	       }
		    
	       s = pdstring("__");
	       s += (*fbt)[i].name();
	       found = findAllFuncsByName(s, pdfv);
	       if(found) {
		  assert(pdfv.size());
#ifdef BPATCH_LIBRARY
		  if(pdfv.size() > 1)
		     cerr << __FILE__ << ":" << __LINE__ 
			  << ": WARNING: findAllFuncsByName found " 
			  << pdfv.size() << " references to function "
			  << s << ".  Using the first.\n";
#endif
		  target = pdfv[0];
		  return true;
	       }
#ifdef BPATCH_LIBRARY
	       else
		  cerr << __FILE__ << ":" << __LINE__
		       << ": WARNING: findAllFuncsByName found no "
		       << "matches for function " << (*fbt)[i].name() 
		       << " or its possible aliases\n";
#endif
            }
         }
         target = 0;
         return false;
      }
   }
   target = 0;
   return false;  
}

fileDescriptor *getExecFileDescriptor(pdstring filename,
				     int &,
				     bool)
{
  fileDescriptor *desc = new fileDescriptor(filename);
  return desc;
}

#if defined(USES_DYNAMIC_INF_HEAP)
static const Address lowest_addr = 0x0;
void process::inferiorMallocConstraints(Address near, Address &lo, Address &hi,
			       inferiorHeapType /* type */ )
{
  if (near)
    {
      lo = region_lo(near);
      hi = region_hi(near);  
    }
}

void process::inferiorMallocAlign(unsigned &size)
{
     /* 32 byte alignment.  Should it be 64? */
  size = (size + 0x1f) & ~0x1f;
}
#endif


bool dyn_lwp::realLWP_attach_() {
   char procName[128];
   sprintf(procName, "/proc/%d/mem", get_lwp_id());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) {
      cerr << "  failed to open file " << procName << ", ret false\n";
      return false;
   }

   attach_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
   if( 0 != P_ptrace(PTRACE_ATTACH, get_lwp_id(), 0, 0) )
   {
      perror( "process::attach - PTRACE_ATTACH" );
      return false;
   }
   
   if (0 > waitpid(get_lwp_id(), NULL, __WCLONE)) {
      perror("process::attach - waitpid");
      exit(1);
   }

   continueLWP();
   return true;
}

bool dyn_lwp::representativeLWP_attach_() {
   // step 1) /proc open: attach to the inferior process memory file
   char procName[128];
   sprintf(procName, "/proc/%d/mem", (int) proc_->getPid());
   fd_ = P_open(procName, O_RDWR, 0);
   if (fd_ < 0) {
      cerr << "  failed to open file " << procName << ", ret false\n";
      return false;
   }
   
   bool running = false;
   if( proc_->wasCreatedViaAttach() )
      running = proc_->isRunning_();
   
   // QUESTION: does this attach operation lead to a SIGTRAP being forwarded
   // to paradynd in all cases?  How about when we are attaching to an
   // already-running process?  (Seems that in the latter case, no SIGTRAP
   // is automatically generated)
   

   // Only if we are really attaching rather than spawning the inferior
   // process ourselves do we need to call PTRACE_ATTACH
   if(proc_->wasCreatedViaAttach() || proc_->wasCreatedViaFork() || 
      proc_->wasCreatedViaAttachToCreated())
   {
      attach_cerr << "process::attach() doing PTRACE_ATTACH" << endl;
      if( 0 != P_ptrace(PTRACE_ATTACH, getPid(), 0, 0) )
      {
         perror( "process::attach - PTRACE_ATTACH" );
         return false;
      }

      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach - waitpid");
         exit(1);
      }
   }

   if(proc_->wasCreatedViaAttach() )
   {
      // If the process was running, it will need to be restarted, as
      // PTRACE_ATTACH kills it
      // Actually, the attach process contructor assumes that the process is
      // running.  While this is foolish, let's play along for now.
      if( proc_->status() != running || !proc_->isRunning_() ) {
         if( 0 != P_ptrace(PTRACE_CONT, getPid(), 0, 0) )
         {
            perror( "process::attach - continue 1" );
         }
      }
   }

   if(proc_->wasCreatedViaAttachToCreated())
   {
      // This case is a special situation. The process is stopped
      // in the exec() system call but we have not received the first 
      // TRAP because it has been caught by another process.
      
      /* lose race */
      sleep(1);
      
      /* continue, clearing pending stop */
      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
         perror("process::attach: PTRACE_CONT 1");
         return false;
      }
     
      if (0 > waitpid(getPid(), NULL, 0)) {
         perror("process::attach: WAITPID");
         return false;
      }

      /* continue, resending the TRAP to emulate the normal situation*/
      if ( 0 > kill(getPid(), SIGTRAP)){
         perror("process::attach: KILL");
         return false;
      }
      
      if (0 > P_ptrace(PTRACE_CONT, getPid(), 0, SIGCONT)) {
         perror("process::attach: PTRACE_CONT 2");
         return false;
      }

      proc_->set_status(neonatal);
   } // end - if createdViaAttachToCreated

   return true;
}

#if defined(arch_x86)
//These constants are not defined in all versions of elf.h
#ifndef AT_NULL
#define AT_NULL 0
#endif
#ifndef AT_SYSINFO
#define AT_SYSINFO 32
#endif
#ifndef AT_SYSINFO_EHDR
#define AT_SYSINFO_EHDR 33
#endif

bool process::readAuxvInfo()
{
  /**
   * The location of the vsyscall is stored in /proc/PID/auxv in Linux 2.6
   * auxv consists of a list of name/value pairs, ending with the AT_NULL
   * name.  There isn't a direct way to get the vsyscall info on Linux 2.4
   **/
  char buffer[32];
  int fd;
  Address dso_start = 0x0, text_start = 0x0;
  unsigned page_size = 0x0;
  struct {
    int type;
    int value;
  } auxv_entry;
  
  sprintf(buffer, "/proc/%d/auxv", pid);

  fd = open(buffer, O_RDONLY);
  if (fd == -1)
  {
    //This is expected on linux 2.4 systems
    return false;
  }

  do {
    read(fd, &auxv_entry, sizeof(auxv_entry));
    if (auxv_entry.type == AT_SYSINFO)
      text_start = auxv_entry.value;
    else if (auxv_entry.type == AT_SYSINFO_EHDR)
      dso_start = auxv_entry.value;
    else if (auxv_entry.type == AT_PAGESZ)
      page_size = auxv_entry.value;
  } while (auxv_entry.type != AT_NULL);

  close(fd);

  assert(text_start != 0x0 && dso_start != 0x0);
  if (page_size == 0x0) page_size = getpagesize();
  
  vsyscall_start_ = dso_start;
  vsyscall_end_ = dso_start + page_size;
  vsyscall_text_ = text_start;

  return true;
}
#endif

void loadNativeDemangler() {}













