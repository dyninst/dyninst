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

/* $Id: dynrpc.C,v 1.76 2000/07/28 17:22:10 pcroth Exp $ */

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metric.h"
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
#include "pdutil/h/sys.h" 
#include "common/h/debugOstream.h"

// The following were defined in process.C
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;
extern debug_ostream signal_cerr;

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
				  string metName,
				  u_int clientID)
{
    if (!metName.length()) 
      getPredictedDataCostCallback(id, req_id, 0.0,clientID);
    else{
      float cost = guessCost(metName, focus);
         // note: returns 0.0 in a variety of situations (if metric cannot be
         //       enabled, etc.)  Would we rather have a more explicit error
         //       return value?
      getPredictedDataCostCallback(id, req_id, cost,clientID);
    }
}

void dynRPC::disableDataCollection(int mid)
{
    float cost;
    metricDefinitionNode *mi;

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    begin_timing(1);
#endif

    if (!allMIs.defines(mid)) {
      // sprintf(errorLine, "Internal error: disableDataCollection mid %d not found\n", mid);
      // logLine(errorLine);
      // showErrorCallback(61,(const char *) errorLine);
      // because of async enables this can happen, so ignore it
      return;
    }

    mi = allMIs[mid];
    // cout << "disable of " << mi->getFullName() << endl; 

    cost = mi->originalCost();
    
    if(cost > currentPredictedCost)
	currentPredictedCost = 0.0;
    else 
        currentPredictedCost -= cost;

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

    mi->disable();
    for (unsigned p=0;p<procsToCont.size();p++) {
#ifdef DETACH_ON_THE_FLY
      procsToCont[p]->detachAndContinue();
#else
      procsToCont[p]->continueProc();
#endif
    }
    allMIs.undef(mid);
    delete(mi);

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    end_timing(1,"disable");
#endif
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
                              vector<string> metric,
			      vector<u_int> mi_ids, 
		 	      u_int daemon_id,
			      u_int request_id){
    vector<int> return_id;
    assert(focus.size() == metric.size());
    return_id.resize(metric.size());
    totalInstTime.start();

    vector<process *>procsToContinue;

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    begin_timing(0);
#endif

    for (u_int i=0;i<metric.size();i++) {
        return_id[i] = startCollecting(metric[i], focus[i].focus, mi_ids[i],                                          procsToContinue);
    }

#if defined(sparc_sun_solaris2_4) && defined(TIMINGDEBUG)
    end_timing(0,"enable");
#endif

    // continue the processes that were stopped in start collecting
    for (unsigned u = 0; u < procsToContinue.size(); u++) {
#ifdef DETACH_ON_THE_FLY
      procsToContinue[u]->detachAndContinue();
#else
      procsToContinue[u]->continueProc();
#endif
      // uncomment next line for debugging purposes on AIX
      // procsToContinue[u]->detach(false);
    }
    totalInstTime.stop();

    enableDataCallback(daemon_id,return_id,mi_ids,request_id);
}

int dynRPC::enableDataCollection2(vector<u_int> focus, string met, int gid)
{
  int id;

  totalInstTime.start();
  vector<process *>procsToContinue;

  id = startCollecting(met, focus, gid, procsToContinue);

  for (unsigned u = 0; u < procsToContinue.size(); u++) {
#ifdef DETACH_ON_THE_FLY
    procsToContinue[u]->detachAndContinue();
#else
    procsToContinue[u]->continueProc();
#endif
  }
  totalInstTime.stop();
  // cout << "Enabled " << met << " = " << id << endl;
  return(id);
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

    // TODO:
    // Update the value of bucket_width, an internal metric.
    // (Code used to be here to update the value, but it wasn't quite enough.
    //  In particular, if there were no enabled instances of bucket_width, then
    //  no updating would be done; thus the update could get lost.
    //  Example: put up table with active_processes; run program 5 seconds;
    //           then add bucket_width to the table.  The bucket width will be 0
    //           because the routine that updated bucket width (i.e. right here)
    //           was called only after active_processes was added; not after
    //           bucket_width was added.
    // In metric.C, we work around the problem by putting in a kludge for reporting
    // the internal metric bucket_width; we simply ignore the value stored in the
    // internalMetrics class and instead return the extern float "sampleInterval" --ari
    //
    // We keep the following code because it's harmless; but remember, it's also
    // not really being used at this time:
    if (bucket_width->num_enabled_instances() > 0)
       bucket_width->getEnabledInstance(0).setValue(sampleInterval);

    if(sampleInterval != currSamplingRate){
         int sample_multiple = (int)((sampleInterval*ONEMILLION)/BASESAMPLEINTERVAL);

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

        currSamplingRate = sampleInterval;
	cerr << "dynrpc: currSamplingRate set to " << currSamplingRate << endl;
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
  return getCurrentTime(false);
}
