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
 * Revision 1.43  1996/09/26 18:58:25  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.42  1996/08/16 21:18:21  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.41  1996/08/12 16:27:20  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.40  1996/05/16 19:29:53  mjrg
 * Fixed a bug in the computation of elapsedPauseTime
 *
 * Revision 1.39  1996/05/08 23:54:38  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.38  1996/04/06 21:25:26  hollings
 * Fixed inst free to work on AIX (really any platform with split I/D heaps).
 * Removed the Line class.
 * Removed a debugging printf for multiple function returns.
 *
 * Revision 1.37  1996/02/13  06:17:20  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.36  1996/01/29  22:09:19  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.35  1995/12/18 14:59:17  naim
 * Minor change to status line messages - naim
 *
 * Revision 1.34  1995/11/22  00:02:17  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.33  1995/10/19  22:36:35  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
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

//timeStamp getCurrentTime(bool);

void forkProcess(traceFork *fr)
{
    process *ret=NULL, *parent;
    parent = findProcess(fr->ppid);
    if (!parent) {
      logLine("Error in forkProcess: could not find parent process\n");
      return;
    }

    // timeStamp forkTime = getCurrentTime(false);
    ret = process::forkProcess(parent, fr->pid);

    //fprintf(stderr, "Fork process took %f secs\n", getCurrentTime(false)-forkTime);
}

void startProcess(traceStart *sr)
{
    process *proc = findProcess(sr->value);
    if (!proc) {
      logLine("Error in startProcess: could not find process\n");
      return;
    }
    if(!process::handleStartProcess(proc)){
      logLine("Error in startProcess: handleStartProcess returned false\n");
    }
}

int addProcess(vector<string> &argv, vector<string> &envp, string dir)
{
    process *proc = createProcess(argv[0], argv, envp, dir);

    if (proc) {
      return(proc->pid);
    }
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
	processVec[u]->continueProc();

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
         processVec[u]->pause();

    if (changed)
      statusLine("application paused");
    return(changed);
}

