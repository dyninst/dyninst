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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/dynrpc.C,v 1.16 1995/02/16 08:33:12 markc Exp $";
#endif


/*
 * File containing lots of dynRPC function definitions for the paradynd..
 *
 * $Log: dynrpc.C,v $
 * Revision 1.16  1995/02/16 08:33:12  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.15  1995/01/26  18:11:54  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.14  1994/11/12  17:28:46  rbi
 * improved status reporting for applications pauses
 *
 * Revision 1.13  1994/11/09  18:39:58  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.12  1994/11/06  09:53:08  jcargill
 * Fixed early paradynd startup problem; resources sent by paradyn were
 * being added incorrectly at the root level.
 *
 * Revision 1.11  1994/11/03  16:12:19  rbi
 * Eliminated argc from addExecutable interface.
 *
 * Revision 1.10  1994/11/02  11:04:44  markc
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

#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "dyninstP.h"
#include "metric.h"
#include "internalMetrics.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "dyninst.h"
#include "stats.h"
#include "resource.h"

// default to once a second.
float samplingRate = 1.0;

void dynRPC::printStats(void)
{
  printDyninstStats();
}

void dynRPC::addResource(string parent, string name)
{
    resource *pr;

    pr = findResource(parent);
    if (!pr) {			// parent isn't defined
      char *tmp;
      resource *res;
      char *ptr = P_strdup(parent.string_of());
      char *resName = ptr + 1;
      assert(resName);

      pr = rootResource;
      while (resName) {
	tmp = P_strchr(resName, '/');
	if (tmp) {
	  *tmp = '\0';
	  tmp++;
	}
	res = newResource(pr, NULL, nullString, resName, 0.0, "");
	pr = res;
	resName = tmp;
      }
      delete ptr;
    }
    if (pr) newResource(pr, NULL, nullString, name, 0.0, "");
}

void dynRPC::coreProcess(int id)
{
    if (processMap.defines(id))
      (processMap[id])->dumpCore("core.out");
}

string dynRPC::getStatus(int id)
{
    char ret[50];
    if (!processMap.defines(id)) {
      sprintf (ret, "PID:%d not found for getStatus\n", id);
      return (ret);
    }
    else
      return ((processMap[id])->getProcessStatus());
}

// TODO - data structures
//
// NOTE: This version of getAvailableMetrics assumes that new metrics are
//   NOT added during execution.
//
vector<T_dyninstRPC::metricInfo> dynRPC::getAvailableMetrics(void)
{
    static bool inited=false;
    static vector<T_dyninstRPC::metricInfo> metInfo;
    int i;

    if (!inited) {
	metricListRec *stuff;

	stuff = getMetricList();
	for (i=0; i < stuff->count; i++) 
	    metInfo += stuff->elements[i].getMetInfo();
	inited = true;
    }
    return(metInfo);
}

double dynRPC::getPredictedDataCost(vector<string> focusString, string metName)
{
    metric *m;
    double val;
    resourceListRec *l;

    if (!metName.length()) return(0.0);
    m = findMetric(metName);
    l = findFocus(focusString);
    if (!l) return(0.0);
    val = guessCost(l, m);

    return(val);
}

double dynRPC::getCurrentHybridCost(void)
{
    statusLine("returning cost measurements");
    return(currentHybridValue);
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricDefinitionNode *mi;

//    commented out because it seems to perturb PC  -rbi 11/8/94
//    statusLine("altering instrumentation");  
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

bool dynRPC::setTracking(string target, bool mode)
{
    resource *res;

    res = findResource(target);
    if (res) {
	if (isResourceDescendent(moduleRoot, res)) {
	    image::changeLibFlag(res, (bool) mode);
	    res->suppressed = true;
	    return(true);
	} else {
	    // un-supported resource hierarchy.
	    return(false);
	}
    } else {
	return(false);
    }
}

// TODO get rid of these ifdefs
int dynRPC::enableDataCollection(vector<string> focusString, string met)
{
    int id;
    metric *m;
    resourceListRec *l;
    totalInstTime.start();

    m = findMetric(met);
    l = findFocus(focusString);
    if (!l) return(-1);

    id = startCollecting(l, m);
    totalInstTime.stop();
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

bool dynRPC::detachProgram(int program, bool pause)
{
  if (processMap.defines(program)) 
    return((processMap[program])->detach(pause));
  else
    return false;
}

//
// Continue all processes
//
void dynRPC::continueApplication(void)
{
    continueAllProcesses();
    statusLine("application running");
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
    (processMap[program])->continueProc();
}

//
//  Stop all processes 
//
bool dynRPC::pauseApplication(void)
{
    pauseAllProcesses();
    return true;
}

//
//  Stop a single process
//
bool dynRPC::pauseProgram(int program)
{
    if (!processMap.defines(program)) {
      sprintf(errorLine, "Can't pause PID %d\n", program);
      logLine(errorLine);
      return false;
    }
    return ((processMap[program])->pause());
}

bool dynRPC::startProgram(int program)
{
    statusLine("starting application");
    continueAllProcesses();
    return(false);
}

//
// This is not implemented yet.
//
bool dynRPC::attachProgram(int id)
{
    return(false);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(vector<string> argv)
{
  vector<string> envp;
  return(addProcess(argv, envp));
}
