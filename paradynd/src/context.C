/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/context.C,v 1.24 1994/11/10 18:57:47 jcargill Exp $";
#endif

/*
 * context.c - manage a performance context.
 *
 * $Log: context.C,v $
 * Revision 1.24  1994/11/10 18:57:47  jcargill
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

#include "util/h/kludges.h"

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

#define MILLION 1000000

/*
 * find out if we have an application defined
 */
bool applicationDefined()
{
    if (processMap.size()) {
	return(true);
    } else {
	return(false);
    }
}

void forkProcess(traceHeader *hr, traceFork *fr)
{
    process *ret=NULL;
    char name[80];
    process *parent;

    if (!processMap.defines(fr->ppid))
      abort();
    parent = processMap[fr->ppid];

    /* attach to the process */
    if (!osAttach(fr->pid)) {
      logLine("Error in forkProcess ptrace\n");
      return;
    }

    ostrstream os(name, 80, ios::out);
    os << parent->symbols->name << "[" << fr->pid << "]" << ends;
    ret = allocateProcess(fr->pid, name);

    ret->symbols = parseImage(parent->symbols->file);
    ret->traceLink = parent->traceLink;
    ret->ioLink = parent->ioLink;
    ret->parent = parent;

    copyInferiorHeap(parent, ret);
    // installDefaultInst(ret, initialRequests);
}

// TODO mdc
int addProcess(int argc, char *argv[], int nenv, char *envp[])
{
    process *proc = createProcess(strdup(argv[0]), argc, argv, nenv, envp);

    if (proc)
      return(proc->pid);
    else
      return(-1);
}

bool detachProcess(int pid, bool paused)
{
  if (processMap.defines(pid)) {
    process *proc = processMap[pid];
    PCptrace(PTRACE_DETACH, proc, (char*) 1, SIGCONT, NULL);
    if (paused) {
      osStop(pid);
      sprintf(errorLine, "detaching process %d leaving it paused\n", 
	      proc->pid);
      logLine(errorLine);
    }
  }
  return(false);
}

bool addDataSource(char *name, char *machine,
    char *login, char *command, int argc, char *argv[])
{
    abort();
    return(false);
}

bool startApplication()
{
    continueAllProcesses();
    return(false);
}

timeStamp startPause = 0.0;

// total processor time the application has been paused.
// so for a multi-processor system this should be processor * time.
timeStamp elapsedPauseTime = 0.0;
static bool appPause = false;

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
    dictionary_hash_iter<int, process*> pi(processMap);
    int i; process *proc;
    while (pi.next(i, proc))
      continueProcess(proc);

    if (!appPause) return(false);

    appPause = false;

    if (!firstRecordTime) return (false);
    elapsedPauseTime += (getCurrentTime(false) - startPause);
    // sprintf(errorLine, "continued at %f\n", getCurrentTime(false));
    // logLine(errorLine);

    return(false);
}

bool pauseAllProcesses()
{
    bool changed;
    dictionary_hash_iter<int, process*> pi(processMap);
    int i; process *proc;

    changed = markApplicationPaused();

    while (pi.next(i, proc))
      pauseProcess(proc);
    
    return(changed);
}


