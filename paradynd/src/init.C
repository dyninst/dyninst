
/*
 * $Log: init.C,v $
 * Revision 1.11  1995/11/13 14:56:31  naim
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
#include "inst.h"
#include "init.h"
#include "resource.h"

internalMetric *activeProcs = NULL;
internalMetric *pauseTime = NULL;
internalMetric *totalPredictedCost= NULL;
internalMetric *hybridPredictedCost = NULL;
internalMetric *observed_cost = NULL;
//internalMetric *activeSlots = NULL;

internalMetric *cpu_daemon = NULL;
internalMetric *sys_daemon = NULL;

internalMetric *minflt_daemon = NULL;
internalMetric *majflt_daemon = NULL;
internalMetric *swap_daemon = NULL;
internalMetric *io_in_daemon = NULL;
internalMetric *io_out_daemon = NULL;
internalMetric *msg_send_daemon = NULL;
internalMetric *msg_recv_daemon = NULL;
internalMetric *sigs_daemon = NULL;
internalMetric *vol_csw_daemon = NULL;
internalMetric *inv_csw_daemon = NULL;

vector<instMapping*> initialRequests;
vector<sym_data> syms_to_find;

// In Elmer Fudd voice: "Be vewwwey vewwey careful!"

bool init() {
  struct utsname un;
  P_uname(&un);
  string hostName(un.nodename);
  rootResource = new resource;
  machineRoot = resource::newResource(rootResource, NULL, nullString,
				      "Machine", 0.0, "");
  machineResource = resource::newResource(machineRoot, NULL, nullString, hostName, 0.0, "");
  processResource = resource::newResource(rootResource, NULL, nullString,
					  "Process", 0.0, "");
//  moduleRoot = resource::newResource(rootResource, NULL, nullString,
//				     "Procedure", 0.0, "");
  moduleRoot = resource::newResource(rootResource, NULL, nullString,
				     "Code", 0.0, "");
  syncRoot = resource::newResource(rootResource, NULL, nullString, 
				   "SyncObject", 0.0, "");

  // TODO -- should these be detected and built ?
  resource::newResource(syncRoot, NULL, nullString, "MsgTag", 0.0, "");
  resource::newResource(syncRoot, NULL, nullString, "SpinLock", 0.0, "");
  resource::newResource(syncRoot, NULL, nullString, "Barrier", 0.0, "");
  resource::newResource(syncRoot, NULL, nullString, "Semaphore", 0.0, "");

  im_pred_struct default_im_preds, obs_cost_preds;
  default_im_preds.machine = pred_null;
  default_im_preds.procedure = pred_invalid;
  default_im_preds.process = pred_invalid;
  default_im_preds.sync = pred_invalid;

  obs_cost_preds.machine = pred_null;
  obs_cost_preds.procedure = pred_invalid;
  obs_cost_preds.process = pred_null;
  obs_cost_preds.sync = pred_invalid;

  totalPredictedCost = internalMetric::newInternalMetric("predicted_cost",
							 EventCounter,
					                 aggMax,	
							 "Wasted CPUs",
							 NULL,
							 default_im_preds,
							 true);

  hybridPredictedCost = internalMetric::newInternalMetric("hybrid_cost", 
							  SampledFunction,
							  aggMax,
							  "Wasted CPUs",
							  NULL,
							  default_im_preds,
							  true);

  observed_cost = internalMetric::newInternalMetric("observed_cost",
						   EventCounter,
						   aggMax,
						   "Wasted CPUs",
						   NULL,
						   default_im_preds,
						   true);

//  activeSlots = internalMetric::newInternalMetric("active_slots", 
//						  SampledFunction,
//						  aggSum,
//						  "Number",
//						  NULL,
//						  default_im_preds,
//						  true);

  cpu_daemon = internalMetric::newInternalMetric("cpu_daemon",
						 EventCounter,
						 aggSum,
						 "Seconds",
						 OS::compute_rusage_cpu,
						 default_im_preds,
						 true);
  
  sys_daemon = internalMetric::newInternalMetric("sys_daemon",
						 EventCounter,
						 aggSum,
						 "Seconds",
						 OS::compute_rusage_sys,
						 default_im_preds,
						 true);

  minflt_daemon = internalMetric::newInternalMetric("min_fault_daemon",
						    EventCounter,
						    aggSum,
						    "Seconds",
						    OS::compute_rusage_min,
						    default_im_preds,
						    true);

  majflt_daemon = internalMetric::newInternalMetric("maj_fault_daemon",
						    EventCounter,
						    aggSum,
						    "Seconds",
						    OS::compute_rusage_maj,
						    default_im_preds,
						    true);

  swap_daemon = internalMetric::newInternalMetric("swap_daemon",
						  EventCounter,
						  aggSum,
						  "Seconds",
						  OS::compute_rusage_swap,
						  default_im_preds,
						  true);

  io_in_daemon = internalMetric::newInternalMetric("io_in_daemon",
						   EventCounter,
						   aggSum,
						   "Seconds",
						   OS::compute_rusage_io_in,
						   default_im_preds,
						   true);

  io_out_daemon = internalMetric::newInternalMetric("io_out_daemon",
						    EventCounter,
						    aggSum,
						    "Seconds",
						    OS::compute_rusage_io_out,
						    default_im_preds,
						    true);

  msg_send_daemon = internalMetric::newInternalMetric("msg_send_daemon",
						    EventCounter,
						    aggSum,
						    "Seconds",
						    OS::compute_rusage_msg_send,
						    default_im_preds,
						    true);

  msg_recv_daemon = internalMetric::newInternalMetric("msg_recv_daemon",
						    EventCounter,
						    aggSum,
						    "Seconds",
						    OS::compute_rusage_msg_recv,
						    default_im_preds,
						    true);

  sigs_daemon = internalMetric::newInternalMetric("signals_daemon",
						  EventCounter,
						  aggSum,
						  "Seconds",
						  OS::compute_rusage_sigs,
						  default_im_preds,
						  true);

  vol_csw_daemon = internalMetric::newInternalMetric("vol_csw_daemon",
						     EventCounter,
						     aggSum,
						     "Seconds",
						     OS::compute_rusage_vol_cs,
						     default_im_preds,
						     true);

  inv_csw_daemon = internalMetric::newInternalMetric("inv_csw_daemon",
						     EventCounter,
						     aggSum,
						     "Seconds",
						     OS::compute_rusage_inv_cs,
						     default_im_preds,
						     true);
 
   pauseTime = internalMetric::newInternalMetric("pause_time",
						 EventCounter,
						 aggMax,
						 "% Time",
						 computePauseTimeMetric,
						 default_im_preds,
						 true);

  activeProcs = internalMetric::newInternalMetric("active_processes",
						  EventCounter,
						  aggSum,
						  "Processes",
						  NULL,
						  obs_cost_preds,
						  true);

  sym_data sd;
  sd.name = "DYNINSTobsCostLow"; sd.must_find = true; syms_to_find += sd;
  sd.name = EXIT_NAME; sd.must_find = true; syms_to_find += sd;
  sd.name = "main"; sd.must_find = true; syms_to_find += sd;

  initDefaultPointFrequencyTable();
  return (initOS());
}

