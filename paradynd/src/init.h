
#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.5  1995/02/16 08:53:13  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.4  1995/02/16  08:33:20  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.3  1994/11/10  18:57:59  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.2  1994/11/09  18:40:03  rbi
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

extern internalMetric *activeProcs;
extern internalMetric *pauseTime;
extern internalMetric *totalPredictedCost;
extern internalMetric *hybridPredictedCost;
extern internalMetric *activeSlots;

extern bool init();
extern bool initOS();
extern void osDependentInst(process *proc);
extern instMapping *initialRequests;

extern vector<sym_data> syms_to_find;

#endif

