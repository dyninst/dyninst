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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "dyninst.h"
#include "dyninstP.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "metric.h"
#include "perfStream.h"
#include "os.h"
#include "showerror.h"
#include "costmetrics.h"

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

unsigned instInstancePtrHash(instInstance * const &ptr) {
   // would be a static fn but for now aix.C needs it.
   unsigned addr = (unsigned)ptr;
   return addrHash16(addr); // util.h
}

void forkProcess(int pid, int ppid,
#ifdef SHM_SAMPLING
		 key_t theKey,
		 void *applAttachedAtPtr,
#endif
		 bool childHasInstr
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
    process *childProc = process::forkProcess(parentProc, pid, map,
					      theKey, applAttachedAtPtr,
					      childHasInstr);
#else
    process *childProc = process::forkProcess(parentProc, pid, map, childHasInstr);
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

int addProcess(vector<string> &argv, vector<string> &envp, string dir)
{
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
    for (unsigned u=0; u<p_size; u++)
       if (processVec[u] != NULL) {
	  //cerr << "continueAll continuing proc pid " << processVec[u]->getPid() << endl;
	  processVec[u]->continueProc();
       }

    if (!appPause) return(false);

    appPause = false;

    statusLine("application running");

    if (!firstRecordTime) return (false);

    if (startPause > 0.0) 
      elapsedPauseTime += (getCurrentTime(false) - startPause);

    // sprintf(errorLine, "continued at %f\n", getCurrentTime(false));
    // logLine(errorLine);

    return(false);
}

bool pauseAllProcesses()
{
    bool changed;
    changed = markApplicationPaused();

    unsigned p_size = processVec.size();
    for (unsigned u=0; u<p_size; u++)
       if (processVec[u] != NULL) {
	 //cerr << "pauseAll pausing proc pid " << processVec[u]->getPid() << endl;
         processVec[u]->pause();
       }

    if (changed)
      statusLine("application paused");
    return(changed);
}
