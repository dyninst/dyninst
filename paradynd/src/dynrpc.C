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
 * Revision 1.43  1996/04/21 21:42:02  newhall
 * changes to getPredictedDataCost and getPredictedDataCostCallback
 *
 * Revision 1.42  1996/04/18  22:02:39  naim
 * Changes to make getPredictedDataCost asynchronous - naim
 *
 * Revision 1.41  1996/04/03  14:27:37  naim
 * Implementation of deallocation of instrumentation for solaris and sunos - naim
 *
 * Revision 1.40  1996/03/14  14:23:25  naim
 * Batching enable data requests for better performance - naim
 *
 * Revision 1.39  1996/03/11  19:02:08  mjrg
 * Fixed a bug in getTime, the return was missing.
 *
 * Revision 1.38  1996/03/05 16:14:02  naim
 * Making enableDataCollection asynchronous in order to improve performance - naim
 *
 * Revision 1.37  1996/03/01  22:35:52  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.36  1996/02/27 20:10:08  naim
 * Changing getPredictedDataCost from double to float for consistency (it was
 * double in some places and float in some others) - naim
 *
 * Revision 1.35  1996/02/22  23:41:40  newhall
 * removed getCurrentSmoothObsCost, and fix to costMetric::updateSmoothValue
 *
 * Revision 1.34  1996/02/13  22:18:04  newhall
 * added test to make sure that currentPredictedCost is never negative
 *
 * Revision 1.33  1996/02/13  06:17:27  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.32  1996/01/29  22:09:22  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.31  1996/01/24 15:34:22  zhichen
 * A little bit cleanup
 *
 * Revision 1.30  1996/01/23 23:42:53  zhichen
 * Added stuff to adjust SAMPLEnodes, see also paradyndCM5/src/main.C
 *
 * Revision 1.29  1996/01/15 16:54:39  zhichen
 * Adjust the value of SAMPLEnodes with the formula "max(t, 1)"
 * A better formula SAMPLEnodes = max(f(t), 1) is underconstruction
 *
 * Revision 1.28  1995/12/15  14:40:49  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.27  1995/12/05  15:59:02  naim
 * Fixing bucket_width metric - naim
 *
 * Revision 1.26  1995/11/30  22:01:08  naim
 * Minor change to bucket_width metric - naim
 *
 * Revision 1.25  1995/11/30  16:53:48  naim
 * Adding bucket_width metric - naim
 *
 * Revision 1.24  1995/11/28  15:56:52  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.23  1995/11/03  00:06:05  newhall
 * changes to support changing the sampling rate: dynRPC::setSampleRate changes
 *     the value of DYNINSTsampleMultiple, implemented image::findInternalSymbol
 * fix so that SIGKILL is not being forwarded to CM5 applications.
 *
 * Revision 1.22  1995/10/19  22:36:39  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.21  1995/09/26  20:17:44  naim
 * Adding error messages using showErrorCallback function for paradynd
 *
 * Revision 1.20  1995/09/18  22:41:33  mjrg
 * added directory command.
 *
 * Revision 1.19  1995/08/24  15:03:48  hollings
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
#include "paradynd/src/costmetrics.h"
#include "showerror.h"
#include "util/h/sys.h" 

#define ONEMILLION 1000000
// default to once a second.
float samplingRate = 1.0;
float currSamplingRate = BASEBUCKETWIDTH;

void dynRPC::printStats(void)
{
  printDyninstStats();
}

// TODO -- use a different creation time
void dynRPC::addResource(u_int parent_id, u_int id, string name, u_int type)
{
  resource *parent = resource::findResource(parent_id);
  if (!parent) return;
  resource::newResource(parent, name, id, type);
}

void dynRPC::coreProcess(int id)
{
  process *proc = findProcess(id);
  if (proc)
    proc->dumpCore("core.out");
}

string dynRPC::getStatus(int id)
{
  process *proc = findProcess(id);
  if (!proc) {
    string ret = string("PID: ") + string(id);
    ret += string(" not found for getStatus\n");
    return (P_strdup(ret.string_of()));
  } else 
    return (proc->getProcessStatus());
}

vector<T_dyninstRPC::metricInfo> dynRPC::getAvailableMetrics(void) {
  vector<T_dyninstRPC::metricInfo> metInfo;
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++)
    metInfo += internalMetric::allInternalMetrics[u]->getInfo();
  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++)
    metInfo += costMetric::allCostMetrics[u2]->getInfo();
  mdl_get_info(metInfo);
  return(metInfo);
}

void dynRPC::getPredictedDataCost(u_int id,
				  u_int req_id,
				  vector<u_int> focus, 
				  string metName)
{
    if (!metName.length()) 
      getPredictedDataCostCallback(id, req_id, 0.0);
    else{
      float cost = guessCost(metName, focus);
      getPredictedDataCostCallback(id, req_id, cost);
    }
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricDefinitionNode *mi;

    if (!allMIs.defines(mid)) {
      sprintf(errorLine, "Internal error: disableDataCollection mid %d not found\n", mid);
      logLine(errorLine);
      showErrorCallback(61,(const char *) errorLine);
      return;
    }

    mi = allMIs[mid];
    // cout << "disable of " << mi->getFullName() << endl; 

    cost = mi->originalCost();
    
    if(cost > currentPredictedCost)
	currentPredictedCost = 0.0;
    else 
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
void dynRPC::enableDataCollection(vector<u_int> focus, string met, 
				  int gid, int daemon_id)
{
    int id;
    totalInstTime.start();
    id = startCollecting(met, focus, gid);
    totalInstTime.stop();
    // cout << "Enabled " << met.string_of() << " = " << id << endl;
    enableDataCallback(daemon_id, id);
}

void 
dynRPC::enableDataCollectionBatch(vector<T_dyninstRPC::focusStruct> focus, 
                                  vector<string> met,
				  vector<int> gid, int daemon_id)
{
    vector<int> return_id;
    assert(focus.size() == met.size());
    return_id.resize(met.size());
    totalInstTime.start();
    for (int i=0;i<met.size();i++) {
      if (gid[i]>=0) {
        return_id[i] = startCollecting(met[i], focus[i].focus, gid[i]);
      }
      else {
        return_id[i] = gid[i];
      }        
    }
    totalInstTime.stop();
    enableDataCallbackBatch(daemon_id, return_id);
}

int dynRPC::enableDataCollection2(vector<u_int> focus, string met, int gid)
{
  int id;
  totalInstTime.start();
  id = startCollecting(met, focus, gid);
  totalInstTime.stop();
  // cout << "Enabled " << met << " = " << id << endl;
  return(id);
}

//
// computes new sample multiple value, and modifies the value of the
// symbol _DYNINSTsampleMultiple which will affect the frequency with
// which performance data is sent to the paradyn process 
//
#ifdef sparc_tmc_cmost7_3
extern float SAMPLEnodes ;
int SAMPLE_MULTIPLE ;
extern int getNumberOfJobs(void) ;
#endif


void dynRPC::setSampleRate(double sampleInterval)
{
    // TODO: implement this:
    // want to change value of DYNINSTsampleMultiple to corr. to new
    // sampleInterval (sampleInterval % baseSampleInterval) 
    // if the sampleInterval is less than the BASESAMPLEINTERVAL ignore
    // use currSamplingRate to determine if the change to DYNINSTsampleMultiple
    // needs to be made

    bucket_width->value = sampleInterval;
    if(sampleInterval != currSamplingRate){
         int *sample_multiple = new int; 
	*sample_multiple = 
	    (int)(((sampleInterval)*ONEMILLION)/BASESAMPLEINTERVAL);
         
#ifdef sparc_tmc_cmost7_3 
	int number_of_jobs = getNumberOfJobs() ;
	sprintf(errorLine, "FOLD, sample_multiple=%d\n", *sample_multiple) ;
	SAMPLE_MULTIPLE = *sample_multiple ;
	// adjust the snarf rate accordingly
	SAMPLEnodes = ((float) BASEBUCKETWIDTH)* 
		      ((float)(SAMPLE_MULTIPLE))*
		      number_of_jobs ;

	if(SAMPLEnodes < 1.0) 
		SAMPLEnodes =  1.0 ;
#endif
	// setSampleMultiple(sample_multiple);
	// set the sample multiple in all processes
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++){
	  if (processVec[u]->status() != exited) {
            internalSym *ret_sym = 0; 
            if(!(ret_sym = processVec[u]->symbols->findInternalSymbol(
				"DYNINSTsampleMultiple",true))){
                sprintf(errorLine, "error2 in dynRPC::setSampleRate\n");
                logLine(errorLine);
		P_abort();
	    }
	    Address addr = ret_sym->getAddr();
            processVec[u]->writeDataSpace((caddr_t)addr,sizeof(int),
					  (caddr_t)sample_multiple);
	  }
        }
        currSamplingRate = sampleInterval;
    }
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
      sprintf(errorLine, "Internal error: cannot continue PID %d\n", program);
      logLine(errorLine);
      showErrorCallback(62,(const char *) errorLine);
    }
    proc->continueProc();
    statusLine("application running");
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
      sprintf(errorLine, "Internal error: cannot pause PID %d\n", program);
      logLine(errorLine);
      showErrorCallback(63,(const char *) errorLine);
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
int dynRPC::addExecutable(vector<string> argv, string dir, bool stopAtFirstBreak)
{
  vector<string> envp;
  return(addProcess(argv, envp, dir, stopAtFirstBreak));
}


//
// report the current time 
//
double dynRPC::getTime() {
  return getCurrentTime(false);
}


//
// CM5 processes only: continue the process that is stopped waiting for the CM5 node
// daemon to start.
//
void dynRPC::nodeDaemonReady() {
  continueProcWaitingForDaemon();
}
