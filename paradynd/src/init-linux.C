/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: init-linux.C,v 1.29 2005/10/13 21:12:34 tlmiller Exp $

#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/init.h"

bool initOS() {
   BPatch_snippet *cmdArg;
   
   if (pd_process::pdFlavor == "mpi") {
      instMPI();
   } 
   cmdArg = new BPatch_paramExpr(4);
   static bool no_warn = false;
   static bool warn = true;
   initialRequestsPARADYN += new pdinstMapping("rexec", "DYNINSTrexec",
                                             FUNC_ENTRY|FUNC_ARG, BPatch_callBefore,
                                             BPatch_lastSnippet, cmdArg,no_warn);
   
   // ===  MULTI-THREADED FUNCTIONS  ======================================
   // Official gotten-from-tracing name. While pthread_create() is the
   // call made from user space, _pthread_body is the parent of any created
   // thread, and so is a good place to instrument.
   pdinstMapping *mapping;

   // Thread SyncObjects
   // mutex
   BPatch_snippet* arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("pthread_mutex_init", "PARADYNreportNewMutex", 
                             FUNC_ENTRY|FUNC_ARG, arg0, no_warn);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
    
#ifdef none    // rwlocks don't appear to exist on linux
   // rwlock
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("pthread_rwlock_init", "PARADYNreportNewRwLock", 
                             FUNC_ENTRY|FUNC_ARG, arg0, no_warn);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
#endif
    
   //Semaphore
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("i_need_a_name", "PARADYNreportNewSema", 
                             FUNC_ENTRY|FUNC_ARG, arg0, no_warn);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);
   
   
   // Conditional variable
   //
   arg0 = new BPatch_paramExpr(0);
   mapping = new pdinstMapping("pthread_cond_init", "PARADYNreportNewCondVar", 
                               FUNC_ENTRY|FUNC_ARG, arg0, no_warn);
   mapping->markAs_MTonly();
   initialRequestsPARADYN.push_back(mapping);

   return true;
};

#include "common/h/Time.h"
#include "common/h/timing.h"
#include "paradynd/src/timeMgr.h"

/* These four functions are wrappers for type-conversion. */
#include "rtinst/h/RThwtimer-linux.h"
rawTime64 getRawTimeTSC() {
	rawTime64 v;
	getTSC( v );
	return v;
	} /* end getRawTimeTSC() */

bool boolIsTSCAvail() {
	return isTSCAvail != 0;
	} /* end boolIsTSCAvail() */

#if defined( cap_mmtimer )
#include "common/src/mmtimer.c"
rawTime64 getRawTimeMMT() {
	// /* DEBUG */ fprintf( stderr, "%s[%d]: daemon calling getRawTimeMMT()\n", __FILE__, __LINE__ );
	rawTime64 v( mmdev_clicks_per_tick * (*mmdev_timer_addr) );
	return v;
	} /* end getRawTimeMMT() */

bool boolIsMMTimerAvail() {
	return isMMTimerAvail() != 0;
	} /* end boolIsMMTimerAvail() */
#endif /* defined( cap_mmtimer ) */

/* Set up the daemon's wall timers. */
void initWallTimeMgrPlt() {
	/* Install the default software wall timer. */
	getWallTimeMgr().installLevel(	wallTimeMgr_t::LEVEL_TWO, yesFunc,
									timeUnit::us(), timeBase::b1970(),
									& getRawTime1970, "swWallTimeFPtrInfo" );

	/* Try to install the optional hardware timers. */
#if defined( cap_mmtimer )
	if( isMMTimerAvail() ) {
		// /* DEBUG */ fprintf( stderr, "%s[%d]: using MMTIMER in daemon.\n", __FILE__, __LINE__ );
		timeStamp currentTime = getCurrentTime();
		timeLength mmtElapsedRealTime( getRawTimeMMT(), getCyclesPerSecond() );
		timeStamp mmtZeroTime = currentTime - mmtElapsedRealTime;
		timeBase mmtTimeBase( mmtZeroTime );
		
		/* I'd prefer to install the TSC first, if it's available, and
		   then override, but I don't know if getWallTimeMgr() allows that. */
		getWallTimeMgr().installLevel(	wallTimeMgr_t::LEVEL_ONE, & boolIsMMTimerAvail,
										getCyclesPerSecond(), mmtTimeBase,
										& getRawTimeMMT, "hwWallTimeFPtrInfo" );
		return;
		} /* end if mmtimer is available */
#endif /* defined( cap_mmtimer ) */

	if( isTSCAvail() ) {
		timeStamp currentTime = getCurrentTime();
		timeLength tscElapsedRealTime( getRawTimeTSC(), getCyclesPerSecond() );
		timeStamp tscZeroTime = currentTime - tscElapsedRealTime;
		timeBase tscTimeBase( tscZeroTime );
		
		getWallTimeMgr().installLevel(	wallTimeMgr_t::LEVEL_ONE, & boolIsTSCAvail,
										getCyclesPerSecond(), tscTimeBase,
										& getRawTimeTSC, "hwWallTimeFPtrInfo" );
		} /* end if the timestamp counter [-equivalent] is available */
	} /* end initWallTimeMgrPlt() */

void pd_process::initOSPreLib()
{
}

pdstring formatLibParadynName(pdstring orig)
{
   return orig;
}



