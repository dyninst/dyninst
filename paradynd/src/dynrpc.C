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

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/dynrpc.C,v 1.18 1995/05/18 10:32:35 markc Exp";
#endif


/*
 * File containing lots of dynRPC function definitions for the paradynd..
 *
 * $Log: dynrpc.C,v $
 * Revision 1.19  1995/08/24 15:03:48  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.18  1995/05/18  10:32:35  markc
 * Replaced process dict with process map
 * Get metric definitions from two locations (internal, and mdl)
 *
 * Revision 1.17  1995/02/16  08:53:08  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.16  1995/02/16  08:33:12  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
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
#include "paradynd/src/mdld.h"
#include "paradynd/src/init.h"

// default to once a second.
float samplingRate = 1.0;

void dynRPC::printStats(void)
{
  printDyninstStats();
}

// TODO -- use a different creation time
void dynRPC::addResource(u_int parent_id, u_int id, string name)
{
  resource *parent = resource::findResource(parent_id);
  if (!parent) return;
  resource::newResource(parent, name, id);
}

void dynRPC::coreProcess(int id)
{
  process *proc = findProcess(id);
  if (proc)
    proc->dumpCore("core.out");
}

string dynRPC::getStatus(int id)
{
  char ret[50];
  process *proc = findProcess(id);
  if (!proc) {
    sprintf (ret, "PID:%d not found for getStatus\n", id);
    return (ret);
  } else 
    return (proc->getProcessStatus());
}

vector<T_dyninstRPC::metricInfo> dynRPC::getAvailableMetrics(void) {
  vector<T_dyninstRPC::metricInfo> metInfo;
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    metInfo += internalMetric::allInternalMetrics[u]->getInfo();

  mdl_get_info(metInfo);
  return(metInfo);
}

double dynRPC::getPredictedDataCost(vector<u_int> focus, string metName)
{
    if (!metName.length()) return(0.0);
    return (guessCost(metName, focus));
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

    if (!allMIs.defines(mid)) {
      sprintf(errorLine, "disableDataCollection mid %d not found\n", mid);
      logLine(errorLine);
      return;
    }

    mi = allMIs[mid];
    // cout << "disable of " << mi->getFullName() << endl; 

    cost = mi->originalCost();

    currentPredictedCost -= cost;

    mi->disable();
    allMIs.undef(mid);
    delete(mi);
}

bool dynRPC::setTracking(unsigned target, bool mode)
{
    resource *res = resource::findResource(target);
    if (res) {
	if (res->isResourceDescendent(moduleRoot)) {
	    image::changeLibFlag(res, (bool) mode);
	    res->suppress(true);
	    return(true);
	} else {
	    // un-supported resource hierarchy.
	    return(false);
	}
    } else {
      // cout << "Set tracking target " << target << " not found\n";
      return(false);
    }
}

void dynRPC::resourceInfoResponse(vector<string> resource_name, u_int resource_id) {
  resource *res = resource::findResource(resource_name);
  if (res)
    res->set_id(resource_id);
}

// TODO -- startCollecting  Returns -1 on failure ?
int dynRPC::enableDataCollection(vector<u_int> focus, string met, int gid)
{
    int id;
    totalInstTime.start();
    id = startCollecting(met, focus, gid);
    totalInstTime.stop();
    // cout << "Enabled " << met << " = " << id << endl;
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
  process *proc = findProcess(program);
  if (proc)
    return(proc->detach(pause));
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
    process *proc = findProcess(program);
    if (!proc) {
      sprintf(errorLine, "Can't continue PID %d\n", program);
      logLine(errorLine);
    }
    proc->continueProc();
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
    process *proc = findProcess(program);
    if (!proc) {
      sprintf(errorLine, "Can't pause PID %d\n", program);
      logLine(errorLine);
      return false;
    }
    return (proc->pause());
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
