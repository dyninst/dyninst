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

// $Id: init.C,v 1.55 2000/08/08 15:35:25 wylie Exp $

#include "dyninstAPI/src/dyninstP.h" // nullString

#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/costmetrics.h"
#include "dyninstAPI/src/inst.h"
#include "paradynd/src/init.h"
#include "paradynd/src/resource.h"
#include "paradynd/src/comm.h"

extern pdRPC *tp;
extern int getNumberOfCPUs();

internalMetric *activeProcs = NULL;
internalMetric *bucket_width = NULL;

internalMetric *pauseTime = NULL;
costMetric *totalPredictedCost= NULL;
costMetric *observed_cost = NULL;
internalMetric *number_of_cpus = NULL;
internalMetric *infHeapMemAvailable = NULL;

internalMetric *numOfActCounters = NULL;
internalMetric *numOfActProcTimers = NULL;
internalMetric *numOfActWallTimers = NULL;

internalMetric *stackwalkTime = NULL;
internalMetric *numOfCurrentLevels = NULL;
internalMetric *numOfCurrentThreads = NULL;
internalMetric *active_threads = NULL;

int numberOfCPUs;

vector<instMapping*> initialRequests;
vector<sym_data> syms_to_find;

float activeProcessesProc(const metricDefinitionNode *node) {
   const vector< vector<string> > &theFocus = node->getFocus();

   // Now let's take a look at the /Machine hierarchy of the focus.
   // If there's a non-trivial refinement, then we obviously return
   // 1, since the focus refers to a single process.
   if (theFocus[resource::machine].size() > 2)
      return 1.0;

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
   return activeProcesses * 1.0;
}



bool init() {
  string hostName = getNetworkName();
  rootResource = new resource;
  machineRoot = resource::newResource(rootResource, NULL, nullString,
				      string("Machine"), 0.0, "", MDL_T_STRING,
				      false);
  machineResource = resource::newResource(machineRoot, NULL, nullString, 
					  hostName, 0.0, "", MDL_T_STRING,
					  false);
//
// processResource = resource::newResource(machineResource, NULL, nullString,
//				  string("Process"), 0.0, "", MDL_T_STRING,
//				  false);
//

  moduleRoot = resource::newResource(rootResource, NULL, nullString,
				     string("Code"), 0.0, "", MDL_T_STRING,
				     false);
  syncRoot = resource::newResource(rootResource, NULL, nullString, 
				   string("SyncObject"), 0.0, "", MDL_T_STRING,
				   false);
  // TODO -- should these be detected and built ?
  resource::newResource(syncRoot, NULL, nullString, "Message", 0.0, "", 
			MDL_T_STRING, false);
  resource::newResource(syncRoot, NULL, nullString, "SpinLock", 0.0, "", 
			MDL_T_STRING, false);
  resource::newResource(syncRoot, NULL, nullString, "Barrier", 0.0, "", 
			MDL_T_STRING, false);
  resource::newResource(syncRoot, NULL, nullString, "Semaphore", 0.0, "", 
			MDL_T_STRING, false);
#if defined(MT_THREAD)
  resource::newResource(syncRoot, NULL, nullString, "Mutex", 0.0, "", 
			MDL_T_STRING, false);
  resource::newResource(syncRoot, NULL, nullString, "RwLock", 0.0, "", 
			MDL_T_STRING, false);
  resource::newResource(syncRoot, NULL, nullString, "CondVar", 0.0, "", 
			MDL_T_STRING, false);

#endif
  /*
  memoryRoot = resource::newResource(rootResource, NULL, nullString, 
				     "Memory", 0.0, "", MDL_T_STRING,
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

  bucket_width = internalMetric::newInternalMetric("bucket_width", 
						   EventCounter,
						   aggMax,
						   "seconds",
						   NULL,
						   default_im_preds,
						   true,
						   Sampled);

  number_of_cpus = internalMetric::newInternalMetric("number_of_cpus", 
						   EventCounter,
						   aggSum,
						   "CPUs",
						   NULL,
						   default_im_preds,
						   false,
						   Sampled);

  // Allow stackwalk_time for whole program (only)
  im_pred_struct default_proc_preds;
  default_proc_preds.machine = pred_null;
  default_proc_preds.procedure = pred_invalid;
  default_proc_preds.process = pred_invalid;
  default_proc_preds.sync = pred_invalid;

  stackwalkTime = internalMetric::newInternalMetric("stackwalk_time",
					       EventCounter,
					       aggMax,
					       "CPUs",
					       computeStackwalkTimeMetric,
					       default_proc_preds,
					       true,
					       Normalized);

#if defined(MT_THREAD)
  numOfCurrentLevels = internalMetric::newInternalMetric(
                                                "numOfCurrentLevels", 
						EventCounter,
						aggMax,
						"ops",//operations
						NULL,
						default_im_preds,
						true,
						Sampled);

  numOfCurrentThreads = internalMetric::newInternalMetric(
                                                "numOfCurrentThreads", 
						EventCounter,
						aggMax,
						"ops",//operations
						NULL,
						default_im_preds,
						true,
						Sampled);

  active_threads = internalMetric::newInternalMetric(
                                                "active_threads", 
						EventCounter,
						aggMax,
						"THREADs",
						NULL,
						default_im_preds,
						false,
						Sampled);
#endif

  numOfActCounters = internalMetric::newInternalMetric(
                                                "numOfActCounters", 
						EventCounter,
						aggMax,
						"ops",//operations
						NULL,
						default_im_preds,
						true,
						Sampled);

  numOfActProcTimers = internalMetric::newInternalMetric(
                                                "numOfActProcTimers", 
						EventCounter,
						aggMax,
						"ops",//operations
						NULL,
						default_im_preds,
						true,
						Sampled);

  numOfActWallTimers = internalMetric::newInternalMetric(
                                                "numOfActWallTimers", 
						EventCounter,
						aggMax,
						"ops",//operations
						NULL,
						default_im_preds,
						true,
						Sampled);
#ifdef ndef
  infHeapMemAvailable = internalMetric::newInternalMetric(
                                                "infHeapMemAvailable", 
						EventCounter,
						aggMax,
						"bytes",
						NULL,
						default_im_preds,
						true,
						Sampled);
#endif

  totalPredictedCost = costMetric::newCostMetric("predicted_cost",
						 EventCounter,
					         aggSum,	
						 "CPUs",
						 default_im_preds,
						 false, 
						 Normalized,
						 aggSum);

  observed_cost = costMetric::newCostMetric("observed_cost",
					   EventCounter,
					   aggSum,
					   "CPUs",
					   obs_cost_preds,
					   false,
					   Normalized,
					   aggSum);

  pauseTime = internalMetric::newInternalMetric("pause_time",
					       EventCounter,
					       aggMax,
					       "CPUs",
					       computePauseTimeMetric,
					       default_im_preds,
					       false,
					       Normalized);


  activeProcs = internalMetric::newInternalMetric("active_processes",
						  EventCounter,
						  aggSum,
						  "ops",//operations
						  activeProcessesProc,
						  active_procs_preds,
						  false,
						  Sampled);

  sym_data sd;

#ifndef SHM_SAMPLING
  sd.name = "DYNINSTobsCostLow"; sd.must_find = true; syms_to_find += sd;
#endif

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  sd.name = "DYNINSTthreadTable"; sd.must_find = true; syms_to_find += sd;
#endif

  sd.name = "DYNINST_bootstrap_info"; sd.must_find = true; syms_to_find += sd;

  sd.name = "PARADYN_bootstrap_info"; sd.must_find = true; syms_to_find += sd;

  // if libc is dynamically linked in then the exit symbol will not
  // be found when we call heapIsOk, so we don't want to set must_find 
  // to true here
  sd.name = EXIT_NAME; sd.must_find = false; syms_to_find += sd;

  // The main function is platform dependent, and it is not always 'main'
  //sd.name = "main"; sd.must_find = true; syms_to_find += sd;

  numberOfCPUs = getNumberOfCPUs();

  initDefaultPointFrequencyTable();

  return (initOS());
}

void instMPI() {
  static AstNode mpiNormTagArg(AstNode::Param, (void *) 4);
  static AstNode mpiNormCommArg(AstNode::Param, (void *) 5);
  static AstNode mpiSRSendTagArg(AstNode::Param, (void *) 4);
  static AstNode mpiSRCommArg(AstNode::Param, (void *) 10);
  static AstNode mpiSRRSendTagArg(AstNode::Param, (void *) 4);
  static AstNode mpiSRRCommArg(AstNode::Param, (void *) 7);
  static AstNode mpiBcastCommArg(AstNode::Param, (void *) 4);
  static AstNode mpiAlltoallCommArg(AstNode::Param, (void *) 6);
  static AstNode mpiAlltoallvCommArg(AstNode::Param, (void *) 8);
  static AstNode mpiGatherCommArg(AstNode::Param, (void *) 7);
  static AstNode mpiGathervCommArg(AstNode::Param, (void *) 8);
  static AstNode mpiAllgatherCommArg(AstNode::Param, (void *) 6);
  static AstNode mpiAllgathervCommArg(AstNode::Param, (void *) 7);
  static AstNode mpiReduceCommArg(AstNode::Param, (void *) 6);
  static AstNode mpiAllreduceCommArg(AstNode::Param, (void *) 5);
  static AstNode mpiReduceScatterCommArg(AstNode::Param, (void *) 5);
  static AstNode mpiScatterCommArg(AstNode::Param, (void *) 7);
  static AstNode mpiScattervCommArg(AstNode::Param, (void *) 8);
  static AstNode mpiScanCommArg(AstNode::Param, (void *) 5);

  vector<AstNode*> argList(2);
  argList[0] = &mpiNormTagArg;
  argList[1] = &mpiNormCommArg;
  initialRequests += new instMapping("PMPI_Send", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Bsend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Ssend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Isend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  initialRequests += new instMapping("PMPI_Issend", "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRSendTagArg;
  argList[1] = &mpiSRCommArg;
  initialRequests += new instMapping("PMPI_Sendrecv", 
				     "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);
  argList[0] = &mpiSRRSendTagArg;
  argList[1] = &mpiSRRCommArg;
  initialRequests += new instMapping("PMPI_Sendrecv_replace",
				     "DYNINSTrecordTagAndGroup",
				     FUNC_ENTRY|FUNC_ARG, argList);

  initialRequests += new instMapping("PMPI_Bcast", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiBcastCommArg);
  initialRequests += new instMapping("PMPI_Alltoall", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiAlltoallCommArg);
  initialRequests += new instMapping("PMPI_Alltoallv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAlltoallvCommArg);
  initialRequests += new instMapping("PMPI_Gather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGatherCommArg);
  initialRequests += new instMapping("PMPI_Gatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiGathervCommArg);
  initialRequests += new instMapping("PMPI_Allgather", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG,
				     &mpiAllgatherCommArg);
  initialRequests += new instMapping("PMPI_Allgatherv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAllgathervCommArg);
  initialRequests += new instMapping("PMPI_Reduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiReduceCommArg);
  initialRequests += new instMapping("PMPI_Allreduce", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiAllreduceCommArg);
  initialRequests += new instMapping("PMPI_Reduce_scatter", 
				     "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, 
				     &mpiReduceScatterCommArg);
  initialRequests += new instMapping("PMPI_Scatter", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScatterCommArg);
  initialRequests += new instMapping("PMPI_Scatterv", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScattervCommArg);
  initialRequests += new instMapping("PMPI_Scan", "DYNINSTrecordGroup",
				     FUNC_ENTRY|FUNC_ARG, &mpiScanCommArg);
}
