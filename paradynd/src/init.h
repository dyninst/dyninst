
#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.1  1994/11/01 16:58:02  markc
 * Prototypes
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "inst.h"

extern resourcePredicate *observedCostPredicates;
extern resourcePredicate *defaultIMpreds;
extern resourcePredicate *cpuTimePredicates;
extern resourcePredicate *wallTimePredicates;
extern resourcePredicate *procCallsPredicates;
extern resourcePredicate *msgPredicates;
extern resourcePredicate *globalOnlyPredicates;
extern resourcePredicate *defaultPredicates;
extern metric *DYNINSTallMetrics;
extern int metricCount;

extern internalMetric *pauseTime;
extern internalMetric *totalPredictedCost;
extern internalMetric *hybridPredictedCost;
extern internalMetric *activeSlots;

extern bool init();
extern bool initOS();
extern instMapping *initialRequests;

#endif

