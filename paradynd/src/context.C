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

/*
 * context.c - manage a performance context.
 *
 * $Log: context.C,v $
 * Revision 1.51  1997/02/21 20:15:37  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.50  1997/01/30 18:17:58  tamches
 * continueAllProcesses() won't try to continue an already-running process;
 * pauseAllProcesses won't try to pause an already-paused process
 *
 * Revision 1.49  1997/01/27 19:40:38  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.48  1997/01/16 22:00:29  tamches
 * added processNewTSConnection().
 *
 * Revision 1.47  1997/01/15 00:19:36  tamches
 * improvided handling of fork and exec
 *
 * Revision 1.46  1996/12/06 09:30:10  tamches
 * check for null processVec entry
 *
 * Revision 1.45  1996/11/08 23:41:02  tamches
 * change from 3-->1 shm segment per process
 *
 * Revision 1.44  1996/10/31 08:37:54  tamches
 * the shm-sampling commit
 *
 * Revision 1.43  1996/09/26 18:58:25  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 */

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/metric.h"
#include "paradynd/src/perfStream.h"
#include "dyninstAPI/src/os.h"
#include "paradynd/src/showerror.h"
#include "paradynd/src/costmetrics.h"

extern vector<process*> processVec;

/*
 * find out if we have an application defined
 */
bool applicationDefined()
{
    if (processVec.size()) {
	return(true);
    } else {
	return(false);
    }
}

extern process *findProcess(int); // should become a static method of class process

//timeStamp getCurrentTime(bool);

#if defined(MT_THREAD)
void createThread(traceThread *fr)
{
    Thread *tp;
    process *parent=NULL;

    assert(fr);
    parent = findProcess(fr->ppid);

    assert(parent && parent->rid);

    string buffer;
    buffer = string("thread_") + string(fr->tid);
    resource *rid;
    rid = resource::newResource(parent->rid, (void *)tp, nullString, 
                                P_strdup(buffer.string_of()),
			        0.0, "", MDL_T_STRING);
    // creating new thread
    tp = new Thread(parent, fr->tid, fr->pos, rid);
    parent->threads += tp;
}

void updateThreadId(traceThrSelf *fr)
{
  process *proc = findProcess(fr->ppid);
  assert(proc);
  Thread *thr = proc->threads[0];
  assert(thr);
  thr->update_tid(fr->tid, fr->pos);
}
#endif

unsigned instInstancePtrHash(instInstance * const &ptr) {
   // would be a static fn but for now aix.C needs it.
   unsigned addr = (unsigned)ptr;
   return addrHash16(addr); // util.h
}

void forkProcess(int pid, int ppid, int trace_fd
#ifdef SHM_SAMPLING
		 ,key_t theKey,
		 void *applAttachedAtPtr
#endif
		 ) {
   process *parentProc = findProcess(ppid);
   if (!parentProc) {
      logLine("Error in forkProcess: could not find parent process\n");
      return;
   }

#ifdef FORK_EXEC_DEBUG
    timeStamp forkTime = getCurrentTime(false);
#endif

   dictionary_hash<instInstance*, instInstance*> map(instInstancePtrHash);
      // filled in by process::forkProcess() call.  The map is as follows: for each
      // instInstance in the parent process, it gives us the instInstance in the child
      // child process.

#ifdef SHM_SAMPLING
    process *childProc = process::forkProcess(parentProc, pid, map, trace_fd,
					      theKey, applAttachedAtPtr);
#else
    process *childProc = process::forkProcess(parentProc, pid, map, trace_fd);
#endif

   // For each mi with a component in parentProc, copy it to the child process --- if
   // the mi isn't refined to a specific process (i.e. is for 'any process')
   // NOTE: It's easy to not copy data items (timers, ctrs) that don't belong in the
   //       child.  But for trampolines, it's tricky, since the fork() syscall will
   //       copy all code whether we like it or not (except on AIX).  Since the
   //       meta-data for the conventional inferior heap has already been copied by the
   //       fork-ctor, when we detect code that shouldn't have been copied, we manually
   //       delete it with deleteInst().  "map" is helpful in this context.
   metricDefinitionNode::handleFork(parentProc, childProc, map);

#ifdef SHM_SAMPLING
   // The following routines perform some assertion checks.
   childProc->getInferiorIntCounters().forkHasCompleted();
   childProc->getInferiorWallTimers().forkHasCompleted();
   childProc->getInferiorProcessTimers().forkHasCompleted();
#endif

#ifdef FORK_EXEC_DEBUG
   cerr << "Fork process took " << (getCurrentTime(false)-forkTime) << " secs" << endl;
#endif

   // Here is where we (used to) continue the parent process...who has been waiting
   // patiently at a DYNINSTbreakPoint() since the beginning of DYNINSTfork() while all
   // this hubbub was going on.  But we can't issue the continueProc().  Why not?
   // Because it's quite possible that the signal delivered to paradynd by the
   // DYNINSTbreakPoint() hasn't yet been processed (yes, this happens in practice), so
   // paradynd still thinks that parentProc's status is running.  What's the
   // solution?  We create a stupid new field in the process structure that, when true,
   // tells paradynd that when the next SIGSTOP is delivered, to continue the process.
   // On the other hand, if parentProc's status is stopped, then we go ahead and issue
   // the continueProc now.  --ari

//   if (parentProc->status() == running)
//      parentProc->continueAfterNextStop();
//   else
//      if (!parentProc->continueProc())
//         assert(false);
}

int addProcess(vector<string> &argv, vector<string> &envp, string dir) {
    process *proc = createProcess(argv[0], argv, envp, dir);

    if (proc)
      return(proc->getPid());
    else
      return(-1);
}

#ifdef notdef
bool addDataSource(char *name, char *machine,
    char *login, char *command, int argc, char *argv[])
{
    P_abort();
    return(false);
}
#endif

bool startApplication()
{
    continueAllProcesses();
    return(false);
}

// TODO use timers here
timeStamp startPause = 0.0;

// total processor time the application has been paused.
// so for a multi-processor system this should be processor * time.
timeStamp elapsedPauseTime = 0.0;

static bool appPause = true;

bool markApplicationPaused()
{
  if (!appPause) {
    // get the time when we paused it.
    startPause = getCurrentTime(false);
    // sprintf(errorLine, "paused at %f\n", startPause);
    // logLine(errorLine);
    appPause = true;
    return true;
  } else 
    return false;
}

bool isApplicationPaused()
{
  return appPause;
}

bool continueAllProcesses()
{
    unsigned p_size = processVec.size();
    for (unsigned u=0; u<p_size; u++) {
       process *p = processVec[u];
       if (p != NULL && p->status() != running)
	  (void)processVec[u]->continueProc(); // ignore return value (is this right?)
    }

    statusLine("application running");

    if (!appPause) return(false);
    appPause = false;

    if (!firstRecordTime) return (false);

    if (startPause > 0.0) 
      elapsedPauseTime += (getCurrentTime(false) - startPause);

    // sprintf(errorLine, "continued at %f\n", getCurrentTime(false));
    // logLine(errorLine);

    return(false);
}

bool pauseAllProcesses()
{
    bool changed = markApplicationPaused();

    unsigned p_size = processVec.size();
    for (unsigned u=0; u<p_size; u++) {
       process *p = processVec[u];
       if (p != NULL && p->status() == running)
         processVec[u]->pause();
    }

    if (changed)
      statusLine("application paused");

    return(changed);
}

void processNewTSConnection(int tracesocket_fd) {
   // either a forked process or one created via attach is trying to get a new
   // tracestream connection.  accept() the new connection, then do some processing.

   int fd = RPC_getConnect(tracesocket_fd); // accept()
      // will become traceLink of new process
   assert(fd >= 0);

   unsigned cookie;
   if (sizeof(cookie) != read(fd, &cookie, sizeof(cookie)))
      assert(false);

   bool calledFromFork = false;
   bool calledFromAttach = false;

   const unsigned cookie_fork   = 0x11111111;
   const unsigned cookie_attach = 0x22222222;

   calledFromFork   = (cookie == cookie_fork);
   calledFromAttach = (cookie == cookie_attach);
   assert(calledFromFork || calledFromAttach);

   string str;
   if (calledFromFork)
      str = string("getting new connection from forked process");
   else
      str = string("getting new connection from attached process");
   statusLine(str.string_of());

   int pid;
   if (sizeof(pid) != read(fd, &pid, sizeof(pid)))
      assert(false);

   int ppid;
   if (sizeof(ppid) != read(fd, &ppid, sizeof(ppid)))
      assert(false);

#ifdef SHM_SAMPLING
   key_t theKey;
   if (sizeof(theKey) != read(fd, &theKey, sizeof(theKey)))
      assert(false);

   void *applAttachedAtPtr;
   if (sizeof(applAttachedAtPtr) != read(fd, &applAttachedAtPtr, sizeof(applAttachedAtPtr)))
      assert(false);
#endif

   process *curr = NULL;

   if (calledFromFork) {
      // the following will (1) call fork ctor (2) call metricDefinitionNode::handleFork
      // (3) continue the parent process, who has been waiting to avoid race conditions.
#ifdef SHM_SAMPLING
      forkProcess(pid, ppid, fd, theKey, applAttachedAtPtr);
#else
      forkProcess(pid, ppid, fd);
#endif

      curr = findProcess(pid);
      assert(curr);

      // continue process...the next thing the process will do is call
      // DYNINSTinit(-1, -1, -1)
      string str = string("running DYNINSTinit() for fork child pid ") + string(pid);
      statusLine(str.string_of());

      if (!curr->continueProc())
	 assert(false);

#ifdef rs6000_ibm_aix4_1
      // HACK to compensate for AIX goofiness: as soon as we call continueProc() above
      // (and not before!), a SIGTRAP appears to materialize out of thin air, stopping
      // the child process.  Thus, DYNINSTinit() won't run unless we issue an explicit
      // continue.  (Actually, there may be a semi-legit explanation.  It seems that on
      // non-solaris platforms, including sunos and aix, if a sigstop is sent and not
      // handled -- i.e. if we just leave the application in a paused state, without
      // continuing -- then the sigstop will be sent over and over again, nonstop, until
      // the application is continued.  So perhaps the sigtrap we see now was present all
      // along, but we never knew it because waitpid in the main loop kept returning
      // sigstops.  --ari)

      int wait_status;
      int wait_result = waitpid(curr->getPid(), &wait_status, WNOHANG);
      if (wait_result > 0) {
	 bool was_stopped = WIFSTOPPED(wait_status);
	 if (was_stopped) {
	    int sig = WSTOPSIG(wait_status);
	    if (sig == 5)  { // sigtrap
	       curr->status_ = stopped;
	       if (!curr->continueProc())
		  assert(false);
	    }
	 }
      }
#endif      
   }

   if (calledFromAttach) {
      // This routine gets called when the attached process is in
      // the middle of running DYNINSTinit.
      curr = findProcess(pid);
      assert(curr);

      curr->traceLink = fd;
   }

}
