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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/dynrpc.C,v 1.10 1994/11/02 11:04:44 markc Exp $";
#endif


/*
 * File containing lots of dynRPC function definitions for the paradynd..
 *
 * $Log: dynrpc.C,v $
 * Revision 1.10  1994/11/02 11:04:44  markc
 * Replaced iterators.
 *
 * Revision 1.9  1994/10/13  07:24:38  krisna
 * solaris porting and updates
 *
 * Revision 1.8  1994/09/22  16:02:25  markc
 * Removed #include "resource.h"
 *
 * Revision 1.7  1994/09/22  01:53:48  markc
 * Made system includes extern "C"
 * added const to char* args to stop compiler warnings
 * changed string to char*
 * declare classes as classes, not structs
 * use igen methods to access igen member vars
 *
 * Revision 1.6  1994/08/08  20:13:36  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/07/28  22:40:36  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.4  1994/07/26  19:56:42  hollings
 * commented out print statements.
 *
 * Revision 1.3  1994/07/20  23:22:48  hollings
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

#include "util/h/kludges.h"

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
#include "dyninst.h"
#include "kludges.h"
#include "stats.h"
#include "resource.h"

// default to once a second.
float samplingRate = 1.0;

void dynRPC::printStats(void)
{
  printDyninstStats();
}

void dynRPC::addResource(const char *parent, const char *name)
{
    resource *pr;

    pr = findResource(parent);
    if (pr) (void) newResource(pr, NULL, nullString, name, 0.0, false);
}

void dynRPC::coreProcess(int id)
{
    if (processMap.defines(id))
      dumpCore(processMap[id]);
}

char *dynRPC::getStatus(int id)
{
    char ret[50];

    if (!processMap.defines(id)) {
      sprintf (ret, "PID:%d not found for getStatus\n", id);
      return (ret);
    }
    else
      return (getProcessStatus(processMap[id]));
}

//
// NOTE: This version of getAvailableMetrics assumes that new metrics are
//   NOT added during execution.
//
metricInfo_Array dynRPC::getAvailableMetrics(void)
{
    static int inited=false;
    static metricInfo_Array metInfo;
    int i;

    if (!inited) {
	metricListRec *stuff;

	stuff = getMetricList();
	metInfo.count = stuff->count;
	metInfo.data = new metricInfo[metInfo.count];
	for (i=0; i < metInfo.count; i++) {
	    metInfo.data[i] = stuff->elements[i].getMetInfo();
	}
	inited = 1;
    }
    return(metInfo);
}

double dynRPC::getPredictedDataCost(String_Array focusString, char *metName)
{
    metric *m;
    double val;
    resourceListRec *l;

    if (!metName) return(0.0);
    m = findMetric(metName);
    l = findFocus(focusString.count, focusString.data);
    if (!l) return(0.0);
    val = guessCost(l, m);

    return(val);
}

double dynRPC::getCurrentHybridCost(void)
{
    return(currentHybridValue);
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricDefinitionNode *mi;

    if (!allMIs.defines(mid)) {
      sprintf(errorLine, "disableDataCollection mid %d not found\n", mid);
      logLine(errorLine);
      return;
    }

    mi = allMIs[mid];
    // sprintf(errorLine, "disable of %s for RL =", getMetricName(mi->met));
    // logLine(errorLine);
    // printResourceList(mi->resList);
    // logLine("\n");

    cost = mi->originalCost;

    currentPredictedCost -= cost;

    mi->disable();
    allMIs.undef(mi->id);
    delete(mi);
}

Boolean dynRPC::setTracking(char *target, Boolean mode)
{
    resource *res;

    res = findResource(target);
    if (res) {
	if (isResourceDescendent(moduleRoot, res)) {
	    changeLibFlag(res, (bool) mode);
	    res->suppressed = true;
	    return(True);
	} else {
	    // un-supported resource hierarchy.
	    return(FALSE);
	}
    } else {
	return(FALSE);
    }
}

int dynRPC::enableDataCollection(String_Array focusString, char *met)
{
    int id;
    metric *m;
    resourceListRec *l;
    long long start;
    long long end;
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_tmc_cmost7_3)
    struct rusage ru;
#elif defined(sparc_sun_solaris2_3)
    struct timeval tv;
#endif

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_tmc_cmost7_3)
    getrusage(RUSAGE_SELF, &ru);
    start = ru.ru_utime.tv_sec * 1000000 + ru.ru_utime.tv_usec;
#elif defined(sparc_sun_solaris2_3)
    // TODO - use a standard timer - this isn't the way to do it for solaris
    gettimeofday(&tv, NULL);
    start = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

    m = findMetric(met);
    l = findFocus(focusString.count, focusString.data);
    if (!l) return(-1);


    id = startCollecting(l, m);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_tmc_cmost7_3)
    getrusage(RUSAGE_SELF, &ru);
    end = ru.ru_utime.tv_sec * 1000000 + ru.ru_utime.tv_usec;
#elif defined(sparc_sun_solaris2_3)
    // TODO - this needs to be cleaned up
    gettimeofday(&tv, NULL);
    end = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

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
    return(detachProcess(program, (bool) pause));
}

//
// Continue all processes
//
void dynRPC::continueApplication(void)
{
    continueAllProcesses();
}

//
// Continue a process
//
void dynRPC::continueProgram(int program)
{
    if (!processMap.defines(program)) {
      sprintf(errorLine, "Can't continue PID %d\n", program);
      logLine(errorLine);
    }
    continueProcess(processMap[program]);
}

//
//  Stop all processes 
//
Boolean dynRPC::pauseApplication(void)
{
    pauseAllProcesses();
    return TRUE;
}

//
//  Stop a single process
//
Boolean dynRPC::pauseProgram(int program)
{
    if (!processMap.defines(program)) {
      sprintf(errorLine, "Can't pause PID %d\n", program);
      logLine(errorLine);
      return FALSE;
    }
    pauseProcess(processMap[program]);
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
Boolean dynRPC::attachProgram(int id)
{
    return(FALSE);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(int argc,String_Array argv)
{
    return(addProcess(argv.count, argv.data));
}
