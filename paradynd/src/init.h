
#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.7  1995/11/13 14:55:36  naim
 * Metric active_slots is not going to be used any longer - naim
 *
 * Revision 1.6  1995/05/18  10:35:02  markc
 * Removed resource preds
 *
 * Revision 1.5  1995/02/16  08:53:13  markc
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

extern internalMetric *activeProcs;
extern internalMetric *pauseTime;
extern internalMetric *totalPredictedCost;
extern internalMetric *observed_cost;
extern internalMetric *hybridPredictedCost;
//extern internalMetric *activeSlots;

extern internalMetric *cpu_daemon;
extern internalMetric *sys_daemon;

extern internalMetric *minflt_daemon;
extern internalMetric *majflt_daemon;
extern internalMetric *swap_daemon;
extern internalMetric *io_in_daemon;
extern internalMetric *io_out_daemon;
extern internalMetric *msg_send_daemon;
extern internalMetric *msg_recv_daemon;
extern internalMetric *sigs_daemon;
extern internalMetric *vol_csw_daemon;
extern internalMetric *inv_csw_daemon;

extern bool init();
extern bool initOS();
extern void osDependentInst(process *proc);
extern vector<instMapping*> initialRequests;
extern vector<metric*> globalMetricVec;

extern vector<sym_data> syms_to_find;

#endif

