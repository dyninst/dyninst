
/*
 * $Log: init.C,v $
 * Revision 1.3  1994/11/09 18:40:01  rbi
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

  pauseTime = new internalMetric("pause_time", 
				 SampledFunction, 
				 aggMax, 
				 "% Time",
				 computePauseTimeMetric,
				 defaultIMpreds);

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

  return (initOS());
}

