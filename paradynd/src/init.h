
#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.2  1994/11/09 18:40:03  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/01  16:58:02  markc
 * Prototypes
 *
 */

#include "metric.h"
#include "internalMetrics.h"
#include "inst.h"
#include "process.h"

extern resourcePredicate *observedCostPredicates;
extern resourcePredicate *defaultIMpreds;
extern resourcePredicate *cpuTimePredicates;
extern resourcePredicate *wallTimePredicates;
extern resourcePredicate *procCallsPredicates;
extern resourcePredicate *msgPredicates;
extern resourcePredicate *globalOnlyPredicates;
extern resourcePredicate *defaultPredicates;
extern resourcePredicate *ioPredicates;

extern metric *DYNINSTallMetrics;
extern int metricCount;

extern internalMetric *pauseTime;
extern internalMetric *totalPredictedCost;
extern internalMetric *hybridPredictedCost;
extern internalMetric *activeSlots;

extern bool init();
extern bool initOS();
extern void osDependentInst(process *proc);
extern instMapping *initialRequests;

#endif

