/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/context.C,v 1.19 1994/09/22 01:48:30 markc Exp $";
#endif

/*
 * context.c - manage a performance context.
 *
 * $Log: context.C,v $
 * Revision 1.19  1994/09/22 01:48:30  markc
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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/ptrace.h>
#include <sys/signal.h>
#include <sys/wait.h>
}

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

#define MILLION 1000000

// <sts/ptrace.h> should really define this. 
extern "C" {

int ptrace(enum ptracereq request, 
		      int pid, 
		      char *addr, 
		      int data, 
		      char *addr2);
}

/*
 * find out if we have an application defined.
 */
Boolean applicationDefined()
{
    if (processList.count()) {
	return(True);
    } else {
	return(False);
    }
}

// NOTE - the tagArg integer (1 for pvm and 2 for cm5) is the parameter
//        number starting with 0.  
#ifdef PARADYND_PVM
// PVM stuff
// pvm puts the tag in the second position pvm_recv(tid, TAG)
static AstNode tagArg(Param, (void *) 1);

instMaping initialRequests[] = {
  { "pvm_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
  { "pvm_recv", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
  { "main", "DYNINSTalarmExpire", FUNC_EXIT },
  { "main", "DYNINSTinit", FUNC_ENTRY },
  { "exit", "DYNINSTalarmExpire", FUNC_ENTRY },
  { "exit", "DYNINSTbreakPoint", FUNC_ENTRY },
  { "DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY },
  { NULL, NULL, 0, NULL },
};

#else
// cm5 stuff
static AstNode tagArg(Param, (void *) 1);

instMaping initialRequests[] = {
    { "cmmd_debug", "DYNINSTnodeCreate", FUNC_ENTRY },
    { "cmmd_debug", "DYNINSTparallelInit", FUNC_EXIT },
    { "cmmd_debug", "DYNINSTbreakPoint", FUNC_EXIT },
    { "main", "DYNINSTalarmExpire", FUNC_EXIT },
#ifdef notdef
    { "fork", "DYNINSTfork", FUNC_EXIT|FUNC_FULL_ARGS },
#endif
    { "exit", "DYNINSTalarmExpire", FUNC_ENTRY },
    { "exit", "DYNINSTprintCost", FUNC_ENTRY },
    { "exit", "DYNINSTbreakPoint", FUNC_ENTRY },
    { "main", "DYNINSTinit", FUNC_ENTRY },
    { "DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY },
    { "CMMD_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_receive", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_receive_block", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_send_block", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_send_async", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_receive_async", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { NULL, NULL, 0, NULL },
};
#endif

void forkProcess(traceHeader *hr, traceFork *fr)
{
    int val;
    process *ret;
    char name[80];
    process *parent;
    executableRec *newExec;

    parent = findProcess(fr->ppid);
    assert(parent);

    /* attach to the process */
    val = ptrace(PTRACE_ATTACH, fr->pid, 0, 0, 0);
    if (val != 0) {
        perror("nptrace");
        return;
    }

    sprintf(name, "%s[%d]", (char*) parent->symbols->name, fr->pid);
    ret = allocateProcess(fr->pid, name);
    ret->symbols = parseImage((char*)parent->symbols->file, 0);
    ret->traceLink = parent->traceLink;
    ret->ioLink = parent->ioLink;
    ret->parent = parent;

    copyInferriorHeap(parent, ret);
    // installDefaultInst(ret, initialRequests);

    newExec = new executableRec;
    newExec->name = name;
    newExec->type = selfTermination;
    newExec->state = neonatal;
    newExec->proc = ret;
}

int addProcess(int argc, char *argv[], int nenv, char *envp[])
{
    int i;
    executableRec *newExec;

    newExec = new executableRec;

    newExec->argc = argc;
    newExec->argv = (char **) calloc(argc+1, sizeof(char *));
    for (i=0; i < argc; i++) {
	newExec->argv[i] = strdup(argv[i]);
    }

    newExec->name = strdup(argv[0]);
    newExec->type = selfTermination;
    newExec->state = neonatal;
    
    newExec->proc = createProcess(newExec->argv[0], newExec->argv, nenv, envp);
    if (newExec->proc) {
	return(newExec->proc->pid);
    } else {
	free(newExec);
	return(-1);
    }
}

Boolean detachProcess(int pid, Boolean paused)
{
    List<process *> curr;

    for (curr = processList; *curr; curr++) {
	if ((*curr)->pid == pid) {
	    PCptrace(PTRACE_DETACH, *curr, (void*) 1, SIGCONT, NULL);
	    if (paused) {
		(void) kill((*curr)->pid, SIGSTOP);
		sprintf(errorLine, "detaching process %d leaving it paused\n", 
		    (*curr)->pid);
		logLine(errorLine);
	    }
	}
    }
    return(False);
}

Boolean addDataSource(char *name, char *machine,
    char *login, char *command, int argc, char *argv[])
{
    abort();
    return(False);
}

Boolean startApplication()
{
    continueAllProcesses();
    return(False);
}

timeStamp endPause = 0.0;
timeStamp startPause = 0.0;
Boolean applicationPaused = FALSE;
extern Boolean firstSampleReceived;

// total processor time the application has been paused.
// so for a multi-processor system this should be processor * time.
timeStamp elapsedPauseTime = 0.0;

Boolean markApplicationPaused()
{

    if (applicationPaused) return(False);
    applicationPaused = True;

    // get the time when we paused it.

    startPause = getCurrentTime(FALSE);
    // sprintf(errorLine, "paused at %f\n", startPause);
    // logLine(errorLine);
    
    return(True);
}

Boolean isApplicationPaused()
{
    return(applicationPaused);
}

Boolean continueAllProcesses()
{
    List<process *> curr;

    for (curr = processList; *curr; curr++) {
	continueProcess(*curr);
    }

    if (!applicationPaused) return(False);
    applicationPaused = False;

    endPause = getCurrentTime(FALSE);
    if (!firstSampleReceived) {
	return(False);
    }

    elapsedPauseTime += (endPause - startPause);
    // sprintf(errorLine, "continued at %f\n", endPause);
    // logLine(errorLine);

    return(False);
}

Boolean pauseAllProcesses()
{
    Boolean changed;
    List<process *> curr;

    changed = markApplicationPaused();
    for (curr = processList; *curr; curr++) {
	pauseProcess(*curr);
    }
    return(changed);
}


