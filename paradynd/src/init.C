
/*
 * $Log: init.C,v $
 * Revision 1.7  1995/03/10 19:33:46  hollings
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
#include "metricDef.h"

internalMetric *activeProcs = NULL;
internalMetric *pauseTime = NULL;
internalMetric *totalPredictedCost= NULL;
internalMetric *hybridPredictedCost = NULL;
internalMetric *activeSlots = NULL;

resourcePredicate *observedCostPredicates = NULL;
resourcePredicate *defaultIMpreds = NULL;

resourcePredicate *cpuTimePredicates = NULL;
resourcePredicate *wallTimePredicates = NULL;
resourcePredicate *procCallsPredicates = NULL;
resourcePredicate *msgPredicates = NULL;
resourcePredicate *globalOnlyPredicates = NULL;
resourcePredicate *defaultPredicates = NULL;
resourcePredicate *ioPredicates = NULL;

instMapping *initialRequests = NULL;
metric *DYNINSTallMetrics = NULL;
int metricCount = 0;

vector<sym_data> syms_to_find;

bool init() {

  observedCostPredicates = new resourcePredicate[5];

  observedCostPredicates[0].set("/SyncObject", invalidPredicate, NULL);
  observedCostPredicates[1].set("/Machine", nullPredicate, NULL);
  observedCostPredicates[2].set("/Process", nullPredicate, NULL);
  observedCostPredicates[3].set("/Procedure", invalidPredicate, NULL);
  observedCostPredicates[4].set((char*)NULL, nullPredicate, NULL, false);

  defaultIMpreds = new resourcePredicate[5];
  defaultIMpreds[0].set("/SyncObject", invalidPredicate, NULL);
  defaultIMpreds[1].set("/Machine", nullPredicate, NULL);
  defaultIMpreds[2].set("/Process", invalidPredicate, NULL);
  defaultIMpreds[3].set("/Procedure", invalidPredicate, NULL);
  defaultIMpreds[4].set((char*)NULL, nullPredicate, NULL, false);

  activeProcs = new internalMetric("active_processes",
				   EventCounter,
				   aggSum,
				   "Processes",
				   NULL,
				   observedCostPredicates); 

  pauseTime = new internalMetric("pause_time", 
				 EventCounter,
				 aggMax, 
				 "% Time",
				 computePauseTimeMetric,
				 defaultIMpreds,
				 true);

  totalPredictedCost = new internalMetric("predicted_cost", 
					  EventCounter,
					  aggMax,
					  "Wasted CPUs",
					  NULL,
					  defaultIMpreds);

  hybridPredictedCost = new internalMetric("hybrid_cost", 
					   SampledFunction,
					   aggMax,
					   "Wasted CPUs",
					   NULL,
					   defaultIMpreds);

  activeSlots = new internalMetric("active_slots", 
				   SampledFunction,
				   aggSum,
				   "Number",
				   NULL,
				   defaultIMpreds);
  
  sym_data sd;
  sd.name = "DYNINSTobsCostLow"; sd.must_find = true; syms_to_find += sd;

  initDefaultPointFrequencyTable();
  return (initOS());
}

