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

// $Id: init.h,v 1.36 2003/06/20 22:08:02 schendel Exp $

#ifndef INIT_HDR
#define INIT_HDR

#include "paradynd/src/timeMgr.h"
#include "dyninstAPI/src/util.h"


// DON'T use these - private of sorts
// ---------------------------------------------------------------------
typedef timeMgr<> wallTimeMgr_t; // <> indicates that wall time querying
                         // function is a member of no class and takes no args
extern wallTimeMgr_t *wallTimeMgr;   

extern bool papiInitialized;

// This function is responsible for platform specific initialization of the
// timeMgr for the wall time.
void initWallTimeMgrPlt();
// ---------------------------------------------------------------------

// USE these functions
// ---------------------------------------------------------------------
void initWallTimeMgr();
timeStamp getWallTime(wallTimeMgr_t::timeMechLevel l =
		      wallTimeMgr_t::LEVEL_BEST);
rawTime64 getRawWallTime(wallTimeMgr_t::timeMechLevel l = 
			 wallTimeMgr_t::LEVEL_BEST);
inline wallTimeMgr_t &getWallTimeMgr() {
  return *wallTimeMgr;
}
// a function that can be used for determining if a timer level is available
// and in this case for timer level that is always available
// (eg. gettimeofday)
bool yesFunc();

void initPapi();

// ---------------------------------------------------------------------

inline bool isPapiInitialized() {
	return papiInitialized;
}

bool bShowTimerInfo();

class internalMetric;
class costMetric;
class instMapping;

extern internalMetric *activeProcs;
extern internalMetric *sampling_rate;
extern internalMetric *number_of_cpus;
extern internalMetric *infHeapMemAvailable;

extern internalMetric *numOfActCounters;
extern internalMetric *numOfActProcTimers;
extern internalMetric *numOfActWallTimers;
extern internalMetric *numOfCurrentThreads;

extern internalMetric *pauseTime;
extern costMetric *totalPredictedCost;
extern costMetric *observed_cost;

extern bool paradyn_init();
extern bool initOS();
extern void instMPI();
extern pdvector<instMapping*> initialRequestsPARADYN;//ccw 19 apr 2002 : SPLIT

extern pdvector<sym_data> syms_to_findPARADYN; //ccw 19 apr 2002 : SPLIT
extern int numberOfCPUs;

extern timeStamp  startStackwalk;
extern timeLength elapsedStackwalkTime;
extern bool       stackwalking;

inline void startTimingStackwalk() {
  startStackwalk = getWallTime();
  stackwalking = true;
}

inline void stopTimingStackwalk() {
  stackwalking = false;
  if (startStackwalk.isInitialized())
    elapsedStackwalkTime += (getWallTime() - startStackwalk);
}




#endif

