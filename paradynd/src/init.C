
/*
 * $Log: init.C,v $
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
 * Revision 1.26  1996/02/10  21:01:42  naim
 * Changing name of metric number_of_nodes by number_of_cpus - naim
 *
 * Revision 1.25  1996/02/09  23:53:39  naim
 * Adding new internal metric number_of_nodes - naim
 *
 * Revision 1.24  1996/02/02  14:31:25  naim
 * Eliminating old definition for observed cost - naim
 *
 * Revision 1.23  1996/02/01  17:42:20  naim
 * Redefining smooth_obs_cost, fixing some bugs related to internal metrics
 * and adding a new definition for observed_cost - naim
 *
 * Revision 1.22  1996/01/29  20:16:29  newhall
 * added enum type "daemon_MetUnitsType" for internal metric definition
 * changed bucketWidth internal metric to EventCounter
 *
 * Revision 1.21  1995/12/27  20:16:41  naim
 * Minor change to internal metric definition - naim
 *
 * Revision 1.20  1995/12/19  20:18:43  newhall
 * changed param to newInternalMetric from bool to int
 *
 * Revision 1.19  1995/12/18 15:03:07  naim
 * Eliminating all "daemon" metrics - naim
 *
 * Revision 1.18  1995/12/15  14:40:51  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.17  1995/11/30  23:09:03  naim
 * Changed the units of bucket_width - naim
 *
 * Revision 1.16  1995/11/30  22:01:10  naim
 * Minor change to bucket_width metric - naim
 *
 * Revision 1.15  1995/11/30  16:53:50  naim
 * Adding bucket_width metric - naim
 *
 * Revision 1.14  1995/11/28  15:55:24  naim
 * Changing metrics observed_cost, predicted_cost and pause_time back to
 * normal mode - naim
 *
 * Revision 1.13  1995/11/17  17:24:21  newhall
 * support for MDL "unitsType" option, added normalized member to metric class
 *
 * Revision 1.12  1995/11/13  16:28:24  naim
 * Making observed_cost, predicted_cost, active_processes and pause_time "normal"
 * metrics and not "developer" mode metrics - naim
 *
 * Revision 1.11  1995/11/13  14:56:31  naim
 * Making all internal metrics "developer mode" metrics - naim
 *
 * Revision 1.10  1995/09/26  20:28:46  naim
 * Minor warning fixes and some other minor error messages fixes
 *
 * Revision 1.9  1995/07/24  03:52:34  tamches
 * The Procedure -- > Code commit
 *
 * Revision 1.8  1995/05/18  10:34:38  markc
 * Removed resource definitions
 *
 * Revision 1.7  1995/03/10  19:33:46  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 * Revision 1.6  1995/02/16  08:53:12  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.5  1995/02/16  08:33:19  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.4  1994/11/10  18:57:57  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.3  1994/11/09  18:40:01  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/01  16:56:42  markc
 * Environment code that is shared by all environs (pvm, cm5, sunos)
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
				      "Machine", 0.0, "", MDL_T_STRING);
  machineResource = resource::newResource(machineRoot, NULL, nullString, hostName, 
					  0.0, "", MDL_T_STRING);
  processResource = resource::newResource(rootResource, NULL, nullString,
					  "Process", 0.0, "", MDL_T_STRING);
  moduleRoot = resource::newResource(rootResource, NULL, nullString,
				     "Code", 0.0, "", MDL_T_STRING);
  syncRoot = resource::newResource(rootResource, NULL, nullString, 
				   "SyncObject", 0.0, "", MDL_T_STRING);
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

