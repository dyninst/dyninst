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

/* $Id: dynrpc.C,v 1.88 2002/09/07 16:15:19 schendel Exp $ */

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/internalMetrics.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/stats.h"
#include "paradynd/src/resource.h"
#include "paradynd/src/mdld.h"
#include "paradynd/src/init.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/context.h"
#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/hist.h"

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream signal_cerr;

timeLength *imetricSamplingRate = NULL;
timeLength *currSamplingRate = NULL;

const timeLength &getIMetricSamplingRate() {
  if(imetricSamplingRate == NULL) {
    // default to once a second.
    imetricSamplingRate = new timeLength(timeLength::sec());
  }
  return *imetricSamplingRate;
}

void setCurrSamplingRate(timeLength tl) {
  if(currSamplingRate == NULL) {
    currSamplingRate = new timeLength(BASEBUCKETWIDTH_SECS, timeUnit::sec());
  }
  *currSamplingRate = tl;
}

const timeLength &getCurrSamplingRate() {
  if(currSamplingRate == NULL) {
    currSamplingRate = new timeLength(BASEBUCKETWIDTH_SECS, timeUnit::sec());
  }
  return *currSamplingRate;
}

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

extern vector<process*> processVec;
extern process* findProcess(int); // should become a static method of class process

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
    return (P_strdup(ret.c_str()));
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
				  string metName,
				  u_int clientID)
{
    if (!metName.length()) 
      getPredictedDataCostCallback(id, req_id, 0.0,clientID);
    else{
      timeLength cost = guessCost(metName, focus);
         // note: returns 0.0 in a variety of situations (if metric cannot be
         //       enabled, etc.)  Would we rather have a more explicit error
         //       return value?
      getPredictedDataCostCallback(id, req_id, 
       	             static_cast<float>(cost.getD(timeUnit::sec())), clientID);
    }
}

void dynRPC::disableDataCollection(int mid)
{
#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    begin_timing(1);
#endif

    machineMetFocusNode *mi = machineMetFocusNode::lookupMachNode(mid);
    if(mi == NULL)
       return;
    
    // cout << "disable of " << mi->getFullName() << endl; 
    // timeLength cost = mi->originalCost();
    timeLength cost = mi->cost();
    
    if(cost > currentPredictedCost)
        setCurrentPredictedCost(timeLength::Zero());
    else 
        subCurrentPredictedCost(cost);

    vector<process *> procsToCont;
    process *proc;
    for (unsigned i=0; i<processVec.size(); i++) {
      proc = processVec[i];
      if (proc->status()==running) {
#ifdef DETACH_ON_THE_FLY
         proc->reattachAndPause();
#else
         proc->pause();
#endif
	procsToCont += proc;
      }
      if (proc->existsRPCreadyToLaunch()) {
	proc->cleanRPCreadyToLaunch(mid);
      }
    }

    for (unsigned p=0;p<procsToCont.size();p++) {
#ifdef DETACH_ON_THE_FLY
      procsToCont[p]->detachAndContinue();
#else
      procsToCont[p]->continueProc();
#endif
    }
    delete mi;

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    end_timing(1,"disable");
#endif
}

bool dynRPC::setTracking(unsigned target, bool /* mode */)
{
    resource *res = resource::findResource(target);
    if (res) {
	if (res->isResourceDescendent(moduleRoot)) {
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

void dynRPC::resourceInfoResponse(vector<u_int> temporaryIds, 
			 	  vector<u_int> resourceIds) {
    assert(temporaryIds.size() == resourceIds.size());

    for (unsigned u = 0; u < temporaryIds.size(); u++) {
      resource *res = resource::findResource(temporaryIds[u]);
      assert(res);
      res->set_id(resourceIds[u]);
    }
}

// TODO -- startCollecting  Returns -1 on failure ?
void dynRPC::enableDataCollection(vector<T_dyninstRPC::focusStruct> focus, 
				  vector<string> metric, vector<u_int> mi_ids, 
				  u_int daemon_id, u_int request_id) {
   assert(focus.size() == metric.size());
   totalInstTime.start();

   vector<u_int> send_mi_ids;
   vector<int>   send_return_ids;

   metricFocusRequestCallbackInfo *cbi = 
      new metricFocusRequestCallbackInfo(request_id, daemon_id);

   for (u_int i=0; i<metric.size(); i++) {
      instr_insert_result_t status = startCollecting(metric[i], 
			     focus[i].focus, mi_ids[i], cbi);
      if(status == insert_success) {
	 send_mi_ids.push_back(mi_ids[i]);
	 send_return_ids.push_back(mi_ids[i]);
      } else if (status == insert_failure) {
	 send_mi_ids.push_back(mi_ids[i]);
	 send_return_ids.push_back(-1);
      }
      // do nothing if status == instr_inserted_deferred
   }
   
   totalInstTime.stop();
   if(send_mi_ids.size() > 0) {
      cbi->makeCallback(send_return_ids, send_mi_ids);
   }
}

// synchronous, for propogating metrics
int dynRPC::enableDataCollection2(vector<u_int> focus, string met, int mid)
{
   totalInstTime.start();
   instr_insert_result_t ret_status = startCollecting(met, focus, mid);
   totalInstTime.stop();

   if(ret_status == insert_success)
      return mid;
   else  // deferred or failure
      return -1;
}

//
// computes new sample multiple value, and modifies the value of the
// symbol _DYNINSTsampleMultiple which will affect the frequency with
// which performance data is sent to the paradyn process 
//
void dynRPC::setSampleRate(double sampleInterval)
{
    // TODO: implement this:
    // want to change value of DYNINSTsampleMultiple to corr. to new
    // sampleInterval (sampleInterval % baseSampleInterval) 
    // if the sampleInterval is less than the BASESAMPLEINTERVAL ignore
    // use currSamplingRate to determine if the change to DYNINSTsampleMultiple
    // needs to be made
    timeLength newSampleRate(timeLength(sampleInterval, timeUnit::sec()));

    if(newSampleRate != getCurrSamplingRate()){
      // sample_multiple:  .2 sec  => 1 ;  .4 sec => 2 ;  .8 sec => 4
         int sample_multiple = static_cast<int>(
	   ((newSampleRate.getD(timeUnit::sec())+.01) / BASEBUCKETWIDTH_SECS));

//	 char buffer[200];
//	 sprintf(buffer, "ari fold; sampleInterval=%g so sample_multiple now %d\n",
//		 sampleInterval, *sample_multiple);
//	 logLine(buffer);
         
	// setSampleMultiple(sample_multiple);
	// set the sample multiple in all processes
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++){
	  if (processVec[u]->status() != exited) {
            internalSym ret_sym; 
            if(!(processVec[u]->findInternalSymbol("DYNINSTsampleMultiple",
							     true, ret_sym))){
                sprintf(errorLine, "error2 in dynRPC::setSampleRate\n");
                logLine(errorLine);
		P_abort();
	    }
	    Address addr = ret_sym.getAddr();
            processVec[u]->writeDataSpace((caddr_t)addr,sizeof(int),
					  (caddr_t)&sample_multiple);
	  }
        }
	setCurrSamplingRate(newSampleRate);
	machineMetFocusNode::updateAllAggInterval(newSampleRate);
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
      showErrorCallback(62,(const char *) errorLine,
		        machineResource->part_name());
      return;
    }
    if (proc->existsRPCinProgress())  {
      // An RPC is in progress, so we delay the continueProc until the RPC
      // finishes - naim
      proc->deferredContinueProc=true;
    } else {
	 if( proc->status() != running ) {
#ifdef DETACH_ON_THE_FLY
	        proc->detachAndContinue();
#else
		proc->continueProc();
#endif
	 }
	 // we are no longer paused, are we?
	 statusLine("application running");
	 if (!markApplicationRunning()) {
	      return;
	 }
    }
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
      showErrorCallback(63,(const char *) errorLine,
		        machineResource->part_name());
      return false;
    }
#ifdef DETACH_ON_THE_FLY
    return proc->reattachAndPause();
#else
    return proc->pause();
#endif
}

bool dynRPC::startProgram(int )
{
    statusLine("starting application");
    continueAllProcesses();
    return(false);
}

//
// Monitor the dynamic call sites contained in function <function_name>
//
void dynRPC::MonitorDynamicCallSites(string function_name){
  unsigned i;
  process *p;
  for(i = 0; i < processVec.size(); i++){
    p = processVec[i];
    p->MonitorDynamicCallSites(function_name);
  }
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(vector<string> argv, string dir)
{
  vector<string> envp;
  return(addProcess(argv, envp, dir)); // context.C
}


//
// Attach is the other way to start a process (application?)
// path gives the full path name to the executable, used _only_ to read
// the symbol table off disk.
// values for 'afterAttach': 1 --> pause, 2 --> run, 0 --> leave as is
//
bool dynRPC::attach(string progpath, int pid, int afterAttach)
{
    attach_cerr << "WELCOME to dynRPC::attach" << endl;
    attach_cerr << "progpath=" << progpath << endl;
    attach_cerr << "pid=" << pid << endl;
    attach_cerr << "afterAttach=" << afterAttach << endl;

#ifdef notdef
    // This code is for Unix platforms only, it will not compile on Windows NT.
    char *str = getenv("PARADYND_ATTACH_DEBUG");
    if (str != NULL) {
       cerr << "pausing paradynd pid " << getpid() << " before attachProcess()" << endl;
       kill(getpid(), SIGSTOP);
    }
#endif

    return attachProcess(progpath, pid, afterAttach); // process.C
}

//
// report the current time 
//
double dynRPC::getTime() {
  return getWallTime().getD(timeUnit::sec(), timeBase::bStd());
}
