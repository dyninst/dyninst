/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, \
  Karen Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/dynrpc.C,v 1.3 1994/07/20 23:22:48 hollings Exp $";
#endif


/*
 * File containing lots of dynRPC function definitions for the paradynd..
 *
 * $Log: dynrpc.C,v $
 * Revision 1.3  1994/07/20 23:22:48  hollings
 * added code to record time spend generating instrumentation.
 *
 * Revision 1.2  1994/07/14  23:30:22  hollings
 * Hybrid cost model added.
 *
 * Revision 1.1  1994/07/14  14:45:48  jcargill
 * Added new file for dynRPC functions, and a default (null) function for
 * processArchDependentTraceStream, and the cm5 version.
 *
 */

#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "dyninstP.h"
#include "metric.h"
#include "internalMetrics.h"
#include "dyninstRPC.SRVR.h"

// default to once a second.
float samplingRate = 1.0;



void dynRPC::printStats()
{
    extern void printDyninstStats();

    printDyninstStats();
}

void dynRPC::addResource(String parent, String name)
{
    resource pr;
    extern resource findResource(char*);

    pr = findResource(parent);
    if (pr) (void) newResource(pr, NULL, NULL, name, 0.0, FALSE);
}

void dynRPC::coreProcess(int pid)
{
    process *proc;

    proc = processList.find((void *) pid);
    dumpCore(proc);
}

String dynRPC::getStatus(int pid)
{
    process *proc;
    extern char *getProcessStatus(process *proc);
    char ret[50];

    proc = processList.find((void *) pid);

    if (!proc) {
	sprintf (ret, "PID:%d not found for getStatus\n", pid);
	return (ret);
    }
    else
	return(getProcessStatus(proc));
}

//
// NOTE: This version of getAvailableMetrics assumes that new metrics are
//   NOT added during execution.
//
metricInfo_Array dynRPC::getAvailableMetrics(void)
{
    int i;
    static int inited;
    static metricInfo_Array metInfo;

    if (!inited) {
	metricList stuff;

	stuff = getMetricList();
	metInfo.count = stuff->count;
	metInfo.data = (metricInfo*) calloc(sizeof(metricInfo), metInfo.count);
	for (i=0; i < metInfo.count; i++) {
	    metInfo.data[i] = stuff->elements[i].info;
	}
	inited = 1;
    }
    return(metInfo);
}

double dynRPC::getPredictedDataCost(String_Array focusString, String metric)
{
    metric m;
    double val;
    resourceList l;

    m = findMetric(metric);
    l = findFocus(focusString.count, focusString.data);
    if (!l) return(0.0);
    val = guessCost(l, m);

    return(val);
}

double dynRPC::getCurrentHybridCost()
{
    extern double currentHybridValue;

    return(currentHybridValue);
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricInstance mi;
    extern double currentPredictedCost;
    extern void printResourceList(resourceList);

    mi = allMIs.find((void *) mid);

    sprintf(errorLine, "disable of %s for RL =", getMetricName(mi->met));
    logLine(errorLine);
    printResourceList(mi->resList);
    logLine("\n");

    cost = mi->originalCost;

    currentPredictedCost -= cost;

    mi->disable();
    allMIs.remove(mi);
    delete(mi);
}

#include <sys/time.h>
#include <sys/resource.h>

int dynRPC::enableDataCollection(String_Array foucsString,String metric)
{
    int id;
    metric m;
    resourceList l;
    double start;
    double end;
    struct rusage ru;
    extern time64 totalInstTime;

    getrusage(RUSAGE_SELF, &ru);
    start = ((double) ru.ru_utime.tv_sec) * 1000000.0 + ru.ru_utime.tv_usec;

    m = findMetric(metric);
    l = findFocus(foucsString.count, foucsString.data);
    if (!l) return(-1);


    id = startCollecting(l, m);

    getrusage(RUSAGE_SELF, &ru);
    end = ((double) ru.ru_utime.tv_sec) * 1000000.0 + ru.ru_utime.tv_usec;
    totalInstTime += (end - start);

    return(id);
}

//
// not implemented yet.
//
void dynRPC::setSampleRate(double sampleInterval)
{
    samplingRate = sampleInterval;
    return;
}

Boolean dynRPC::detachProgram(int program,Boolean pause)
{
    return(detachProcess(program, pause));
}

//
// Continue all processes
//
void dynRPC::continueApplication()
{
    continueAllProcesses();
}

//
// Continue a process
//
void dynRPC::continueProgram(int program)
{
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
	if ((*curr)->pid == pid) break;
    }
    if (*curr) {
        continueProcess(*curr);
    } else {
	sprintf(errorLine, "Can't continue PID %d\n", program);
	logLine(errorLine);
    }
}

//
//  Stop all processes 
//
Boolean dynRPC::pauseApplication()
{
    pauseAllProcesses();
    return TRUE;
}

//
//  Stop a single process
//
Boolean dynRPC::pauseProgram(int program)
{
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
        if ((*curr)->pid == program) break;
    }
    if (!(*curr)) {
	sprintf(errorLine, "Can't pause PID %d\n", program);
	logLine(errorLine);
	return FALSE;
    }
    pauseProcess(*curr);
    return TRUE;
}

Boolean dynRPC::startProgram(int program)
{
    continueAllProcesses();
    return(False);
}

//
// This is not implemented yet.
//
Boolean dynRPC::attachProgram(int pid)
{
    return(FALSE);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(int argc,String_Array argv)
{
    return(addProcess(argc, argv.data));
}
