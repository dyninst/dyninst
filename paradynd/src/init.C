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

/*
 * $Log: init.C,v $
 * Revision 1.34  1996/08/16 21:18:45  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.33  1996/05/31 23:54:37  tamches
 * minor change to string usage
 *
 * Revision 1.32  1996/05/03 17:05:57  tamches
 * a don't blame me commit: active_processes can now be enabled for
 * any focus, not just processes, machines, and whole program.
 *
 * Revision 1.31  1996/04/29 03:34:10  tamches
 * added activeProcessesProc
 * changed declaration of active_processes internal metric
 *
 * Revision 1.30  1996/03/01 22:35:54  mjrg
 * Added a type to resources.
 * Changes to the MDL to handle the resource hierarchy better.
 *
 * Revision 1.29  1996/02/13 06:17:28  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.28  1996/02/12  20:07:13  naim
 * Making number_of_cpus a regular metric - naim
 *
 * Revision 1.27  1996/02/12  16:46:11  naim
 * Updating the way we compute number_of_cpus. On solaris we will return the
 * number of cpus; on sunos, hp, aix 1 and on the CM-5 the number of processes,
 * which should be equal to the number of cpus - naim
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "costmetrics.h"
#include "inst.h"
#include "init.h"
#include "resource.h"

extern pdRPC *tp;
extern int getNumberOfCPUs();

internalMetric *activeProcs = NULL;
internalMetric *bucket_width = NULL;

internalMetric *pauseTime = NULL;
costMetric *totalPredictedCost= NULL;
costMetric *smooth_obs_cost = NULL;
costMetric *observed_cost = NULL;
internalMetric *number_of_cpus = NULL;

int numberOfCPUs;

vector<instMapping*> initialRequests;
vector<sym_data> syms_to_find;

float activeProcessesProc(const metricDefinitionNode *node) {
   const vector< vector<string> > &theFocus = node->getFocus();

   // Now let's take a look at the /Process hierarchy of the focus.
   // If there's a non-trivial refinement, then we obviously return
   // 1, since the focus refers to a single process.
   if (theFocus[resource::process].size() > 1)
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


// In Elmer Fudd voice: "Be vewwwey vewwey careful!"

bool init() {
  struct utsname un;
  P_uname(&un);
  string hostName(un.nodename);
  rootResource = new resource;
  machineRoot = resource::newResource(rootResource, NULL, nullString,
				      string("Machine"), 0.0, "", MDL_T_STRING);
  machineResource = resource::newResource(machineRoot, NULL, nullString, hostName, 
					  0.0, "", MDL_T_STRING);
  processResource = resource::newResource(rootResource, NULL, nullString,
					  string("Process"), 0.0, "", MDL_T_STRING);
  moduleRoot = resource::newResource(rootResource, NULL, nullString,
				     string("Code"), 0.0, "", MDL_T_STRING);
  syncRoot = resource::newResource(rootResource, NULL, nullString, 
				   string("SyncObject"), 0.0, "", MDL_T_STRING);
  // TODO -- should these be detected and built ?
  resource::newResource(syncRoot, NULL, nullString, "MsgTag", 0.0, "", MDL_T_STRING);
  resource::newResource(syncRoot, NULL, nullString, "SpinLock", 0.0, "", MDL_T_STRING);
  resource::newResource(syncRoot, NULL, nullString, "Barrier", 0.0, "", MDL_T_STRING);
  resource::newResource(syncRoot, NULL, nullString, "Semaphore", 0.0, "", MDL_T_STRING);

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
						   "#CPUs",
						   NULL,
						   default_im_preds,
						   false,
						   Sampled);

  totalPredictedCost = costMetric::newCostMetric("predicted_cost",
						 EventCounter,
					         aggSum,	
						 "CPUs",
						 default_im_preds,
						 false, 
						 Normalized,
						 aggSum);

  smooth_obs_cost = costMetric::newCostMetric("smooth_obs_cost", 
					      EventCounter,
					      aggSum,
					      "CPUs",
					      obs_cost_preds,
					      true,
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
						  "operations",
						  activeProcessesProc,
						  active_procs_preds,
						  false,
						  Sampled);

  sym_data sd;
  sd.name = "DYNINSTobsCostLow"; sd.must_find = true; syms_to_find += sd;
  sd.name = EXIT_NAME; sd.must_find = true; syms_to_find += sd;
  sd.name = "main"; sd.must_find = true; syms_to_find += sd;

  numberOfCPUs = getNumberOfCPUs();

  initDefaultPointFrequencyTable();

  return (initOS());
}

