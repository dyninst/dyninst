
#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.18  1996/08/12 16:27:15  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.17  1996/02/13 06:17:29  newhall
 * changes to how cost metrics are computed. added a new costMetric class.
 *
 * Revision 1.16  1996/02/12  16:46:12  naim
 * Updating the way we compute number_of_cpus. On solaris we will return the
 * number of cpus; on sunos, hp, aix 1 and on the CM-5 the number of processes,
 * which should be equal to the number of cpus - naim
 *
 * Revision 1.15  1996/02/10  21:01:44  naim
 * Changing name of metric number_of_nodes by number_of_cpus - naim
 *
 * Revision 1.14  1996/02/09  23:53:40  naim
 * Adding new internal metric number_of_nodes - naim
 *
 * Revision 1.13  1996/02/02  14:31:27  naim
 * Eliminating old definition for observed cost - naim
 *
 * Revision 1.12  1996/02/01  17:42:24  naim
 * Redefining smooth_obs_cost, fixing some bugs related to internal metrics
 * and adding a new definition for observed_cost - naim
 *
 * Revision 1.11  1995/12/18  15:03:09  naim
 * Eliminating all "daemon" metrics - naim
 *
 * Revision 1.10  1995/12/15  14:40:52  naim
 * Changing "hybrid_cost" by "smooth_obs_cost" - naim
 *
 * Revision 1.9  1995/11/30  22:01:11  naim
 * Minor change to bucket_width metric - naim
 *
 * Revision 1.8  1995/11/30  16:53:51  naim
 * Adding bucket_width metric - naim
 *
 * Revision 1.7  1995/11/13  14:55:36  naim
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
#include "costmetrics.h"
#include "inst.h"
#include "process.h"

extern internalMetric *activeProcs;
extern internalMetric *bucket_width;
extern internalMetric *number_of_cpus;

extern internalMetric *pauseTime;
extern costMetric *totalPredictedCost;
extern costMetric *observed_cost;
extern costMetric *smooth_obs_cost;

extern bool init();
extern bool initOS();
extern vector<instMapping*> initialRequests;
extern vector<metric*> globalMetricVec;

extern vector<sym_data> syms_to_find;
extern int numberOfCPUs;

#endif

