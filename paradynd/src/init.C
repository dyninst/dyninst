/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * update services, notics of latent defects, or correction of
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

// $Id: init.C,v 1.84 2005/01/28 18:12:05 legendre Exp $


#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/init.h"
#include "paradynd/src/resource.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/context.h"
#include "common/h/timing.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/context.h"    // elapsedPauseTime, startPause
#include "paradynd/src/dynrpc.h"
#include "pdutil/h/aggregationDefines.h"
#include "paradynd/src/processMgr.h"
#include "paradynd/src/pd_process.h"
#include "dyninstAPI/h/BPatch.h"

#ifdef PAPI
#include "papi.h"
#endif


extern pdRPC *tp;
extern int getNumberOfCPUs();

extern unsigned enable_pd_samplevalue_debug;
extern void addLibraryCallback(BPatch_thread *, BPatch_module *, bool);
#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

internalMetric *activeProcs = NULL;
internalMetric *sampling_rate = NULL;

internalMetric *pauseTime = NULL;
costMetric *totalPredictedCost= NULL;
costMetric *observed_cost = NULL;
internalMetric *number_of_cpus = NULL;
internalMetric *infHeapMemAvailable = NULL;

internalMetric *numOfActCounters = NULL;
internalMetric *numOfActProcTimers = NULL;
internalMetric *numOfActWallTimers = NULL;

internalMetric *stackwalkTime = NULL;
internalMetric *numOfCurrentThreads = NULL;
internalMetric *active_threads = NULL;

int numberOfCPUs;

pdvector<pdinstMapping*> initialRequestsPARADYN;//ccw 19 apr 2002 : SPLIT  ALSO CHANGED BELOW 
pdvector<sym_data> syms_to_findPARADYN; //ccw 19 apr 2002 : SPLIT

BPatch *__bpatch = NULL;

void initBPatch() {
   if(__bpatch == NULL)
      __bpatch = new BPatch;
}

pdSample computeSamplingRate(const machineMetFocusNode *) {
  // we'll transfer bucket width as milliseconds, instead of seconds because
  // we can't represent fractional second bucket widths (eg. .2 seconds). Now
  // that our sample value type (pdSample) is an int64_t when it used to be a
  // double
  return pdSample(getCurrSamplingRate().getI(timeUnit::ms()));
}

pdSample computeNumOfCPUs(const machineMetFocusNode *) {
  return pdSample(numberOfCPUs);
}

pdSample computeActiveProcessesProc(const machineMetFocusNode *node) {
   const Focus &theFocus = node->getFocus();

   // Now let's take a look at the /Machine hierarchy of the focus.
   // If there's a non-trivial refinement, then we obviously return
   // 1, since the focus refers to a single process.
   if (theFocus.process_defined())
      return pdSample(1);

   // Okay, if we've gotten this far, then the focus does _not_ refer
   // to a specific process.  So, at most, it can refer to a specific
   // machine.  Now, we only know how many processes there are for
   // us (this machine).  Indeed, if there's a specific machine in the
   // focus, and it's not us, then we're sunk --- we have no idea how
   // many processes there are on another paradynd.  Luckily, this
   // will never happen.
   // So, if we reach this point, we just return the total number of
   // processes on this machine.
   extern unsigned activeProcesses; // process.C (same as processVec.size())
   return pdSample(activeProcesses * 1);
}

pdSample computePauseTimeMetric(const machineMetFocusNode *) {
    // we don't need to use the machineMetFocusNode

   if (isInitFirstRecordTime()) {
      timeLength elapsed = elapsedPauseTime;
      if (isApplicationPaused()) {
         timeStamp now = getWallTime();
         elapsed += now - startPause;
      }
      
      assert(elapsed >= timeLength::Zero()); 
      return pdSample(elapsed);
   } else {
      return pdSample(timeLength::Zero());
   }
}

pdSample computeNumOfActCounters(const machineMetFocusNode *) {
   unsigned max = 0;
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *curProc = *itr++;
      if(!curProc) continue;
      if(curProc->hasExited()) continue;
      if(curProc->numOfActCounters_is > max)
	 max = curProc->numOfActCounters_is;
   }
   return pdSample(max);
}

pdSample computeNumOfActProcTimers(const machineMetFocusNode *) {
   unsigned max = 0;
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *curProc = *itr++;
      if(!curProc) continue;
      if(curProc->hasExited()) continue;
      if(curProc->numOfActProcTimers_is > max)
	 max = curProc->numOfActProcTimers_is;
   }
   return pdSample(max);
}

pdSample computeNumOfActWallTimers(const machineMetFocusNode *) {
  unsigned max = 0;
  processMgr::procIter itr = getProcMgr().begin();
  while(itr != getProcMgr().end()) {
     pd_process *curProc = *itr++;
     if(!curProc) continue;
     if(curProc->hasExited()) continue;
     if(curProc->numOfActWallTimers_is > max)
	max = curProc->numOfActWallTimers_is;
  }
  return pdSample(max);
}

timeStamp  startStackwalk;
timeLength elapsedStackwalkTime = timeLength::Zero();
bool       stackwalking = false;

pdSample computeStackwalkTimeMetric(const machineMetFocusNode *) {
    // we don't need to use the machineMetFocusNode
    if (isInitFirstRecordTime()) {
        timeLength elapsed = elapsedStackwalkTime;
        if (stackwalking) {
          timeStamp now = getWallTime();
          elapsed += now - startStackwalk;
        }
        assert(elapsed >= timeLength::Zero());
        return pdSample(elapsed);
    } else {
        return pdSample(timeLength::Zero());
    }
}

pdSample computeNumOfCurrentThreads(const machineMetFocusNode *) {
   unsigned max = 0;
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *curProc = *itr++;
      if(!curProc) continue;
      if(curProc->thrMgr().size() > max)
	 max = curProc->thrMgr().size();
   }
   return pdSample(max);
}

pdSample computeNumOfActiveThreads(const machineMetFocusNode *) {
   unsigned numOfActiveThreads = 0;
   processMgr::procIter itr = getProcMgr().begin();
   while(itr != getProcMgr().end()) {
      pd_process *curProc = *itr++;
      if(!curProc) continue;
      numOfActiveThreads += curProc->thrMgr().size();
   }
   return pdSample(numOfActiveThreads);
}


bool paradyn_init() {
  pdstring hostName = getNetworkName();
  rootResource = new resource;

  initCyclesPerSecond();
  initWallTimeMgr();

  initPapi();
  initProcMgr();
  initBPatch();
  //__bpatch->registerDynLibraryCallback((BPatchDynLibraryCallback) (addLibraryCallback));

  machineRoot = resource::newResource(rootResource, NULL, nullString,
				      pdstring("Machine"), timeStamp::ts1970(), 
				      "", 
                      CategoryResourceType,
                      MDL_T_STRING, 
                      false);
  machineResource = resource::newResource(machineRoot, NULL, nullString,
					  hostName, timeStamp::ts1970(), "", 
                      MachineResourceType,
					  MDL_T_STRING, 
                      false);

  moduleRoot = resource::newResource(rootResource, NULL, nullString,
				     pdstring("Code"), timeStamp::ts1970(), "", 
                     CategoryResourceType,
				     MDL_T_STRING, 
                     false);
  syncRoot = resource::newResource(rootResource, NULL, nullString, 
				   pdstring("SyncObject"), timeStamp::ts1970(), "",
                   CategoryResourceType,
				   MDL_T_STRING, 
                   false);
  // TODO -- should these be detected and built ?
  resource::newResource(syncRoot, NULL, nullString, "Message",
			timeStamp::ts1970(), "",
            CategoryResourceType,
            MDL_T_STRING, 
            false);
  resource::newResource(syncRoot, NULL, nullString, "SpinLock",
			timeStamp::ts1970(), "",
            CategoryResourceType,
            MDL_T_STRING,
            false);
  resource::newResource(syncRoot, NULL, nullString, "Barrier",
			timeStamp::ts1970(), "",
            CategoryResourceType,
            MDL_T_STRING,
            false);
  resource::newResource(syncRoot, NULL, nullString, "Semaphore", 
			timeStamp::ts1970(), "",
            CategoryResourceType,
            MDL_T_STRING,
            false);
  resource::newResource(syncRoot, NULL, nullString, "Window",
                      timeStamp::ts1970(), "",
                      CategoryResourceType,
                      MDL_T_STRING, false);

  /*
  memoryRoot = resource::newResource(rootResource, NULL, nullString, 
				     "Memory", 0.0, "",
                     CategoryResourceType,
                     MDL_T_STRING,
				     true);
  */

  im_pred_struct default_im_preds, obs_cost_preds;
  default_im_preds.machine = pred_null;
  default_im_preds.procedure = pred_invalid;
  default_im_preds.process = pred_invalid;
  default_im_preds.sync = pred_invalid;

  obs_cost_preds.machine = pred_null;
  obs_cost_preds.procedure = pred_invalid;
  obs_cost_preds.process = pred_null;
  obs_cost_preds.sync = pred_invalid;

  // Allow active_processes to be enabled for (1) whole program (of course)
  // but also (2) a given machine and (3) a given process.
  im_pred_struct active_procs_preds;
  active_procs_preds.machine = pred_null;
  active_procs_preds.procedure = pred_null;
  active_procs_preds.process = pred_null;
  active_procs_preds.sync = pred_null;

  // The internal metrics used to be of style EventCounter but have been
  // switched to SampledFunction.  In the front end, eventCounter metric
  // values are normalized by the time delta whereas the SampledFunction
  // style metric values are left unnormalized.  When the internal metrics
  // were EventCounter style, the value was multiplied in this function by
  // the time delta to offset the normalization being done in the front end.
  // This is not only redundant but also led to difficulties representing
  // resultant fractional sample values with our now integral sample value
  // type.  eg. previously: (end-start) * sampleValue -> (.2sec) * 1 = .2

  sampling_rate = internalMetric::newInternalMetric(
       "sampling_rate", aggMax, "milliseconds", computeSamplingRate, 
       default_im_preds, true, Sampled, 
       internalMetric::firstSample_ForInitActualValue);
  sampling_rate->setStyle(SampledFunction);

  /* ie. the number of CPUs present on the machine */
  number_of_cpus = internalMetric::newInternalMetric(
       "number_of_cpus", aggSum, "CPUs", computeNumOfCPUs, default_im_preds, 
       false, Sampled, internalMetric::firstSample_ForInitActualValue);
  number_of_cpus->setStyle(SampledFunction);

  // Allow stackwalk_time for whole program (only)
  im_pred_struct default_proc_preds;
  default_proc_preds.machine = pred_null;
  default_proc_preds.procedure = pred_invalid;
  default_proc_preds.process = pred_invalid;
  default_proc_preds.sync = pred_invalid;

  stackwalkTime = internalMetric::newInternalMetric(
       "stackwalk_time", aggMax, "CPUs", computeStackwalkTimeMetric, 
       default_proc_preds, true, Normalized,
       internalMetric::zero_ForInitActualValue);
  stackwalkTime->setStyle(EventCounter);

  numOfCurrentThreads = internalMetric::newInternalMetric(
       "numOfCurrentThreads", aggMax, "ops", // operations
       computeNumOfCurrentThreads, default_im_preds, true, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  numOfCurrentThreads->setStyle(SampledFunction);

  active_threads = internalMetric::newInternalMetric(
       "active_threads", aggMax, "THREADs", computeNumOfActiveThreads,
       default_im_preds, false, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  active_threads->setStyle(SampledFunction);

  numOfActCounters = internalMetric::newInternalMetric(
       "numOfActCounters", aggMax, "ops", // operations 
       computeNumOfActCounters, default_im_preds, true, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  numOfActCounters->setStyle(SampledFunction);

  numOfActProcTimers = internalMetric::newInternalMetric(
       "numOfActProcTimers", aggMax, "ops", // operations
       computeNumOfActProcTimers, default_im_preds, true, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  numOfActProcTimers->setStyle(SampledFunction);

  numOfActWallTimers = internalMetric::newInternalMetric(
       "numOfActWallTimers", aggMax, "ops",//operations
       computeNumOfActWallTimers, default_im_preds, true, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  numOfActWallTimers->setStyle(SampledFunction);

#ifdef ndef
  infHeapMemAvailable = internalMetric::newInternalMetric(
       "infHeapMemAvailable", aggMax, "bytes", NULL, default_im_preds, true, 
       Sampled, internalMetric::firstSample_ForInitActualValue);
#endif

  totalPredictedCost = costMetric::newCostMetric(
       "predicted_cost", aggSum, "slowdown", default_im_preds, false, 
       Normalized, aggSum);

  observed_cost = costMetric::newCostMetric(
       "observed_cost", aggSum, "slowdown", obs_cost_preds, false, 
       Normalized, aggSum);

  pauseTime = internalMetric::newInternalMetric(
       "pause_time", aggMax, "CPUs", computePauseTimeMetric, default_im_preds,
       false, Normalized, internalMetric::zero_ForInitActualValue);
  pauseTime->setStyle(EventCounter);
  
  activeProcs = internalMetric::newInternalMetric(
       "active_processes", aggSum, "ops", // operations
       computeActiveProcessesProc, active_procs_preds, false, Sampled,
       internalMetric::firstSample_ForInitActualValue);
  activeProcs->setStyle(SampledFunction);

  sym_data sd;

  sd.name = "DYNINST_bootstrap_info"; sd.must_find = true; syms_to_findPARADYN += sd;

  sd.name = "PARADYN_bootstrap_info"; sd.must_find = true; syms_to_findPARADYN += sd; //ccw 19 apr 2002 : SPLIT

  // if libc is dynamically linked in then the exit symbol will not
  // be found when we call heapIsOk, so we don't want to set must_find 
  // to true here
  sd.name = EXIT_NAME; sd.must_find = false; syms_to_findPARADYN += sd;

  // The main function is platform dependent, and it is not always 'main'
  //sd.name = "main"; sd.must_find = true; syms_to_find += sd;

  numberOfCPUs = getNumberOfCPUs();

  initDefaultPointFrequencyTable();

  return (initOS());
}

void instMPI() {
  static BPatch_paramExpr mpiNormTagArg(4);
  static BPatch_paramExpr mpiNormCommArg(5);
  static BPatch_paramExpr mpiSRSendTagArg(4);
  static BPatch_paramExpr mpiSRCommArg(10);
  static BPatch_paramExpr mpiSRRSendTagArg(4);
  static BPatch_paramExpr mpiSRRCommArg(7);
  static BPatch_paramExpr mpiBcastCommArg(4);
  static BPatch_paramExpr mpiAlltoallCommArg(6);
  static BPatch_paramExpr mpiAlltoallvCommArg(8);
  static BPatch_paramExpr mpiGatherCommArg(7);
  static BPatch_paramExpr mpiGathervCommArg(8);
  static BPatch_paramExpr mpiAllgatherCommArg(6);
  static BPatch_paramExpr mpiAllgathervCommArg(7);
  static BPatch_paramExpr mpiReduceCommArg(6);
  static BPatch_paramExpr mpiAllreduceCommArg(5);
  static BPatch_paramExpr mpiReduceScatterCommArg(5);
  static BPatch_paramExpr mpiScatterCommArg(7);
  static BPatch_paramExpr mpiScattervCommArg(8);
  static BPatch_paramExpr mpiScanCommArg(5);
  static BPatch_paramExpr mpiWinCreateArg(5);
  static BPatch_paramExpr mpiWinNameWinArg(0);
  static BPatch_paramExpr mpiWinNameNameArg(1);
  static BPatch_paramExpr mpiWinFreeArg(0);
  static BPatch_paramExpr mpiCommNameCommArg(0);
  static BPatch_paramExpr mpiCommNameNameArg(1);
  static BPatch_paramExpr mpiCommFreeArg(0);

  pdvector<BPatch_snippet*> argList(2);
  initialRequestsPARADYN += new pdinstMapping("MPI_Win_create", "DYNINSTrecordWindow",
              FUNC_EXIT|FUNC_ARG, BPatch_callBefore, BPatch_firstSnippet,
              &mpiWinCreateArg, false);
  initialRequestsPARADYN += new pdinstMapping("MPI_Win_free", "DYNINSTretireWindow",
              FUNC_ENTRY|FUNC_ARG, BPatch_callBefore, BPatch_firstSnippet,
              &mpiWinFreeArg, false);
  argList[0] = &mpiWinNameWinArg;
  argList[1] = &mpiWinNameNameArg;
  initialRequestsPARADYN += new pdinstMapping("MPI_Win_set_name", "DYNINSTnameWindow",
              FUNC_ENTRY|FUNC_ARG, BPatch_callBefore, BPatch_firstSnippet,
              argList, false);

  argList[0] = &mpiCommNameCommArg;
  argList[1] = &mpiCommNameNameArg;
  initialRequestsPARADYN += new pdinstMapping("MPI_Comm_set_name", "DYNINSTnameGroup",
              FUNC_ENTRY|FUNC_ARG, BPatch_callBefore, BPatch_firstSnippet,
              argList, false);
  initialRequestsPARADYN += new pdinstMapping("MPI_Comm_free", "DYNINSTretireGroupTag",
              FUNC_ENTRY|FUNC_ARG, BPatch_callBefore, BPatch_firstSnippet,
              &mpiCommFreeArg, false);

  argList[0] = &mpiNormTagArg;
  argList[1] = &mpiNormCommArg;
  initialRequestsPARADYN += new pdinstMapping("MPI_Send", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequestsPARADYN += new pdinstMapping("MPI_Bsend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequestsPARADYN += new pdinstMapping("MPI_Ssend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequestsPARADYN += new pdinstMapping("MPI_Isend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequestsPARADYN += new pdinstMapping("MPI_Issend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRSendTagArg;
  argList[1] = &mpiSRCommArg;
  initialRequestsPARADYN += new pdinstMapping("MPI_Sendrecv", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRRSendTagArg;
  argList[1] = &mpiSRRCommArg;
  initialRequestsPARADYN += new pdinstMapping("MPI_Sendrecv_replace", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequestsPARADYN += new pdinstMapping("MPI_Bcast", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiBcastCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Alltoall", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAlltoallCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Alltoallv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAlltoallvCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Gather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGatherCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Gatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGathervCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Allgather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG,
				     &mpiAllgatherCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Allgatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAllgathervCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Reduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiReduceCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Allreduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAllreduceCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Reduce_scatter", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiReduceScatterCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Scatter", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScatterCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Scatterv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScattervCommArg);
  initialRequestsPARADYN += new pdinstMapping("MPI_Scan", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScanCommArg);
}

wallTimeMgr_t *wallTimeMgr = NULL; //time querying function, member of no class

bool papiInitialized = false;

// a time available function for timeMgr levels that always returns true
// (ie. the level is always available), eg. gettimeofday
bool yesFunc() { return true; }

bool bForceSoftwareLevelWallTimer() {
  char *evar;
  evar = getenv("PD_SOFTWARE_LEVEL_WALL_TIMER");
  if( evar  )
    return true;
  else
    return false;
}

bool bShowTimerInfo() {
  char *evar;
  evar = getenv("PD_SHOW_TIMER_INFO");
  if( evar  )
    return true;
  else
    return false;
}

void initWallTimeMgr() {
  if(wallTimeMgr != NULL) delete wallTimeMgr;
  wallTimeMgr = new wallTimeMgr_t();
  initWallTimeMgrPlt();

  if(bForceSoftwareLevelWallTimer()) {
    wallTimeMgr_t::mech_t *tm = 
      wallTimeMgr->getMechLevel(wallTimeMgr_t::LEVEL_TWO);
    wallTimeMgr->installMechLevel(wallTimeMgr_t::LEVEL_BEST, tm);
    sampleVal_cerr << "Forcing to software level wall timer\n";
  } else {
    wallTimeMgr->determineBestLevels();
  }
  wallTimeMgr_t::timeMechLevel ml = wallTimeMgr->getBestLevel();
  sampleVal_cerr << "Chosen wall timer level: " << int(ml)+1 << "  "
		 << *wallTimeMgr->getMechLevel(ml) << "\n\n";
  if(bShowTimerInfo()) {
    cerr << "Chosen wall timer level: " << int(ml)+1 << "  "
	 << *wallTimeMgr->getMechLevel(ml) << "\n\n";
  }
}

timeStamp getWallTime(wallTimeMgr_t::timeMechLevel l) {
  if(wallTimeMgr == NULL) assert(0);
  return wallTimeMgr->getTime(l);
}

rawTime64 getRawWallTime(wallTimeMgr_t::timeMechLevel l) {
  if(wallTimeMgr == NULL) assert(0);
  return wallTimeMgr->getRawTime(l);
}

void initWallTimeMgrPlt();  // platform specific initialization
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// initialize and access wall time through these functions
// ---------------------------------------------------------------------


void initPapi() {
 
#ifdef PAPI
   /* try to initialize PAPI library for daemon */


  int retval;
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if ( retval != PAPI_VER_CURRENT) {
    papiInitialized = false;
    fprintf(stderr, "PAPI init failed\n");
  }
  else {
    papiInitialized = true;
    fprintf(stderr, "PAPI init successful\n");
  }
#endif

}
