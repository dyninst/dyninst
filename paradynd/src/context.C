/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/context.C,v 1.37 1996/02/13 06:17:20 newhall Exp";
#endif

/*
 * context.c - manage a performance context.
 *
 * $Log: context.C,v $
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
 * Revision 1.32  1995/09/26  20:17:42  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.31  1995/09/18  22:41:30  mjrg
 * added directory command.
 *
 * Revision 1.30  1995/09/11  19:19:28  mjrg
 * Removed redundant ptrace calls.
 *
 * Revision 1.29  1995/05/18  10:30:58  markc
 * Replace process dict with process map
 *
 * Revision 1.28  1995/02/26  22:44:29  markc
 * Changed vector of strings to reference to vector of strings for addProcess(...)
 *
 * Revision 1.27  1995/02/16  08:53:03  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.26  1995/02/16  08:33:00  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.25  1994/11/11  23:22:29  rbi
 * added status reporting for process stops
 *
 * Revision 1.24  1994/11/10  18:57:47  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.23  1994/11/09  18:39:55  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.22  1994/11/02  11:02:34  markc
 * Replaced old-style iterators and string-handles.
 *
 * Revision 1.21  1994/10/13  07:24:32  krisna
 * solaris porting and updates
 *
 * Revision 1.20  1994/09/30  19:46:58  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.19  1994/09/22  01:48:30  markc
 * Standardized ptrace, PCptrace signatures
 * Instantiate classes as classes, not structs
 * cast stringHandles for printing
 * cast args for PCptrace
 *
 * Revision 1.18  1994/08/17  18:05:50  markc
 * Moved call to install default instrumentation to SIGTRAP handler to
 * ensure no code is installed until the child process is stopped.
 *
 * Revision 1.17  1994/07/28  22:40:34  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.16  1994/07/22  19:15:19  hollings
 * moved computePauseTime to machine specific area.
 *
 * Revision 1.15  1994/07/14  23:30:21  hollings
 * Hybrid cost model added.
 *
 * Revision 1.14  1994/07/14  14:24:17  jcargill
 * Removed old call to flushPtrace -- not needed anymore
 *
 * Revision 1.13  1994/07/12  20:04:09  jcargill
 * Fixed tagArg; changed to instrumenting CMMDs instead of CMMPs, to
 * correctly handle optimized libraries.
 *
 * Revision 1.12  1994/07/05  03:26:00  hollings
 * observed cost model
 *
 * Revision 1.11  1994/06/29  02:52:23  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.10  1994/06/27  18:56:37  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.9  1994/06/22  01:43:13  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.8  1994/05/31  18:11:50  markc
 * Initialized v global values used to compute pause time.  Added pvm
 * specific default instrumentation.
 *
 * Revision 1.7  1994/05/18  00:52:23  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.6  1994/04/11  23:25:21  hollings
 * Added pause_time metric.
 *
 * Revision 1.5  1994/04/09  18:34:51  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 * Revision 1.4  1994/03/31  01:47:54  markc
 * Extended parameters for addProcess, which default to NULL and 0.
 *
 * Revision 1.3  1994/03/22  21:03:12  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.2  1994/03/20  01:53:03  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/01/27  20:31:15  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.11  1993/12/13  19:52:40  hollings
 * added applicationDefined
 *
 * Revision 1.10  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.9  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.8  1993/08/16  16:24:08  hollings
 * added prototype for ptrace.
 *
 * Revision 1.7  1993/08/11  01:45:37  hollings
 * added support for UNIX fork.
 *
 * Revision 1.6  1993/07/13  18:26:13  hollings
 * new include file syntax.
 *
 * Revision 1.5  1993/06/28  23:13:18  hollings
 * fixed process stopping.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:18  hollings
 * Initial revision
 *
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


// TODO mdc
int addProcess(vector<string> &argv, vector<string> &envp, string dir, bool stopAtFirstBrk)
{
    process *proc = createProcess(argv[0], argv, envp, dir);

    if (proc) {
      proc->stopAtFirstBreak = stopAtFirstBrk;
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


//
// This function is used for CM5 processes only. The process must stop after the 
// CM5 node daemon is started. When the node daemon is ready, it sends a
// nodeDaemonReady message to paradyn, which calls the continueProcWaitingForDaemon 
// to resume the application.
//
void continueProcWaitingForDaemon(void) {
  for (unsigned u = 0; u < processVec.size(); u++) {
    process *p = processVec[u];
    if (p->waitingForNodeDaemon) {
      p->waitingForNodeDaemon = false;
      if (!appPause) {
        // application is running. Continue the process that is waiting.
	statusLine("application running");
	p->continueProc();
      }
      else
	statusLine("application paused");
    }
  }
}
