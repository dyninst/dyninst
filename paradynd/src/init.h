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

#ifndef INIT_HDR
#define INIT_HDR

/*
 * $Log: init.h,v $
 * Revision 1.21  1997/01/27 19:40:43  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.20  1996/10/31 08:44:58  tamches
 * removed globalMetricVec, unused.
 *
 * Revision 1.19  1996/08/16 21:18:47  tamches
 * updated copyright for release 1.1
 *
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
 */

#include "metric.h"
#include "internalMetrics.h"
#include "costmetrics.h"
#include "inst.h"
#include "process.h"

extern internalMetric *activeProcs;
extern internalMetric *bucket_width;
extern internalMetric *number_of_cpus;
extern internalMetric *total_CT;
extern internalMetric *active_CT;
extern internalMetric *infHeapMemAvailable;
extern internalMetric *mem_CT;

extern internalMetric *pauseTime;
extern costMetric *totalPredictedCost;
extern costMetric *observed_cost;
extern costMetric *smooth_obs_cost;

extern bool init();
extern bool initOS();
extern vector<instMapping*> initialRequests;

extern vector<sym_data> syms_to_find;
extern int numberOfCPUs;

#endif

