/*
 * 
 * $Log: PCrules.C,v $
 * Revision 1.7  1994/05/02 20:38:14  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.6  1994/04/13  01:37:07  markc
 * Added ifdef print statements to see hypothesis checks.
 *
 * Revision 1.5  1994/04/12  15:32:49  hollings
 * generalized hysteresis into a normalization constant to cover pause,
 * contention, and ignored bottlenekcks too.
 *
 * Revision 1.4  1994/04/11  23:19:44  hollings
 * lowered cpu threshold to 60%.
 *
 * Revision 1.3  1994/03/01  21:25:12  hollings
 * added tunable constants.
 *
 * Revision 1.2  1994/02/08  17:21:27  hollings
 * made elapsed time optinal metric.
 *
 * Revision 1.1  1994/02/02  00:38:18  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.8  1993/08/16  16:26:50  hollings
 * changed the way explain prints cpu bottlenecks.
 *
 * Revision 1.7  1993/08/05  18:56:56  hollings
 * new include syntax.
 *
 * Revision 1.6  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.5  1993/02/03  00:06:49  hollings
 * removed execesive friends of focus and focusCell.
 *
 * Revision 1.4  1992/12/14  19:56:57  hollings
 * added true enable/disable.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:06:33  hollings
 * first cut at automated refinement.
 *
 * Revision 1.1  1992/08/03  20:42:59  hollings
 * Initial revision
 *
 *
 */


#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1992 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCrules.C,v 1.7 1994/05/02 20:38:14 hollings Exp $";
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "PCwhy.h"
#include "PCmetric.h"
#include "PCshg.h"
#include "PCevalTest.h"

// enable printing for eval tests
// #define PC_PRINT 

//
// list of all known tests.
//
testList allTests;

//
// currently running tests.
//
testList activeTests;

//
// corresponding lists for hypotheses
//
hypothesisList allHypotheses;
hypothesisList activeHypotheses;

// 20% sync is considered high.
const float highSyncThreshold	= 0.20;

// 75% of time in CPU imples cpu time should be checked.
const float highCPUtoSyncRatioThreshold	= 0.60;

// 10-50% seems right
const float highIOthreshold = 0.10;

// 10%
const float highLossThreshold = 0.10;

// rate in operations per second.
const float maxBarrierRate = 60000.0;	// actual # from ips2 67,608

const float maxSemaphoreRate = 6000.0;	// actual # from ips2 3092 pairs

const float maxLockRate = 600000.0;	// actual # from ips2 685,689

// actual from ips2 21,030
const float maxIPSprocedureCallRate = 20000;

const float highInstOverheadThreshold	= 0.15;

// large critical section is one held more than 30% of the time.
const float largeCriticalSectionThreshold = 0.30;

// critical section should be larger than 4 null locks 
const float minLockSize = 4.00;

// 50% of io delay is seek we call it seek bound.
const float seekBoundThreshold = 0.50;

// max contextSwitch rate (1/time for null context switch).
// exact value from ips2 is 7299.2701 (for reading 1 byte from /dev/null)
//
const float contextSwitchLimit = 7000;

//
// avg page fault (zero page create which is fastest) is 680usec on 
//  Symmetry. so use 1msec as time --> 1000 as the limit.
//
const float pageFaultLimit = 1000;

// physical disk blcoks
const int diskBlockSize = 4096;

// time to aquire an free lock (in usec).
const lockOverhead = 0;


Boolean highFuncInst_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = procedureCalls.changeCollection(newMode);
    status = activeProcesses.changeCollection(whereAxis, newMode) && status;
    return(status);
}

/* ARGSUSED */
void highFuncInst_TEST(testValue *result, float normalize)
{
    result->status = FALSE;
#ifdef PC_PRINT
    cout << "highFuncInst >? V=" << (procedureCalls.value() /
                                  activeProcesses.value(whereAxis) /
				  maxIPSprocedureCallRate) << " A="
				  << (highInstOverheadThreshold*hysteresis)
				    << "\n";
#endif
    if (procedureCalls.value()/activeProcesses.value(whereAxis)/
	maxIPSprocedureCallRate > highInstOverheadThreshold*normalize) {
	result->status = TRUE;
    }
    return;
}

Boolean highSyncToCPURatio_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = SyncTime.changeCollection(newMode);
    status = activeProcesses.changeCollection(whereAxis, newMode) && status;
    return(status);
}

/* ARGSUSED */
void highSyncToCPURatio_TEST(testValue *result, float normalize)
{
    float active;

    result->status = FALSE;
    active = activeProcesses.value(whereAxis);
    if (active > 0.0) {
	if (SyncTime.value()/active > highSyncThreshold * normalize) {
	    result->status = TRUE;
	}
    }
#ifdef PC_PRINT
    if (active <= 0.0)
      cout << "highSyncToCPURatio Not active\n";
    else
      cout << "highSyncToCPURatio >? V=" << (SyncTime.value() / active) <<
	" A=" << (highSyncThreshold * hysteresis) << "\n";
#endif
    return;
}

Boolean highCPUtoSyncRatio_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = CPUtime.changeCollection(newMode);
    status = activeProcesses.changeCollection(whereAxis, newMode) && status;

    // this is used in the explaination.
    // Doesn't changes status if not available.
    elapsedTime.changeCollection(whereAxis, newMode);

    return(status);
}

/* ARGSUSED */
// 
// If we are getting highCPUtoSyncRatioThreshold of the attempted parallelism
//   consider the job CPU bound.
//
void highCPUtoSyncRatio_TEST(testValue *result, float normalize)
{
    float processes;

    result->status = FALSE;
    processes = activeProcesses.value(whereAxis);
    if (CPUtime.value()/processes > highCPUtoSyncRatioThreshold * normalize) {
	result->status = TRUE;
	result->addHint(Procedures, "Lots of cpu time");
    }
#ifdef PC_PRINT
    cout << "highCPUtoSyncRatio >? V=" << (CPUtime.value() / processes) <<
      " A=" << (highCPUtoSyncRatioThreshold * hysteresis) << "\n";
#endif
    return;
}

Boolean criticalSectionTooLarge_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = CPUtime.changeCollection(newMode);
    status = lockHoldingTime.changeCollection(newMode) && status;

    return(status);
}

/* ARGSUSED */
void criticalSectionTooLarge_TEST(testValue *result, float normalize)
{
    float pctHeld;

    // This really should be blocking lock?  Do we only care if another process
    //   id holding it?
    pctHeld = lockHoldingTime.value() / CPUtime.value();
    result->status = FALSE;
    if (pctHeld > largeCriticalSectionThreshold * normalize) {
	result->status = TRUE;
    }
#ifdef PC_PRINT
    cout << "critSectooLarge >? V=" << pctHeld << " A=" <<
      (largeCriticalSectionThreshold * hysteresis) << "\n";
#endif   
    return;
}

Boolean highVariationLock_ENABLE(collectMode newMode)
{
    focus *i;
    Boolean status;
    focusList allLocks;
    Boolean lockStatus = FALSE;
     
    status = SyncTime.changeCollection(whereAxis, newMode);
    allLocks = currentFocus->magnify(Locks);
    for (; i= *allLocks; allLocks++) {
	lockStatus = SyncTime.changeCollection(i, newMode) || lockStatus;
    }
    return(status && lockStatus);
}

//
// Test if one lock is more than highSyncThreshold % of the sync time.
//
/* ARGSUSED */
void highVariationLock_TEST(testValue *result, float normalize)
{
    focus *i;
    float share;
    focusList allLocks;
     
    allLocks = currentFocus->magnify(Locks);
    result->status = FALSE;
    for (; i=*allLocks; allLocks++) {
	if (!SyncTime.enabled(i)) continue;
	share = SyncTime.value(i) / SyncTime.value(whereAxis);
	if (share > highSyncThreshold * normalize) {
	    result->status = TRUE;
	    // define a hint to refine on the selected lock.
	    if (i != currentFocus) {
		// there is something useful here.
		result->addHint(i, "HOT Sync");
	    }
	}
    }
    return;
}

Boolean highSyncRate_ENABLE(collectMode newMode)
{
    focusList newFoci;
    Boolean syncObjStatus;

    newFoci = currentFocus->magnify(SyncObject);
    if (*newFoci) {
	syncObjStatus = SyncOps.changeCollection(newFoci, newMode);
	return(syncObjStatus);
    } else {
	// no such focus.
	return(FALSE);
    }
}

/* ARGSUSED */
void highSyncRate_TEST(testValue *result, float normalize)
{
    focus *newFocus;

    result->status = FALSE;
    newFocus = currentFocus->constrain(Locks);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > maxLockRate * normalize) {
	    result->status = TRUE;
	    result->addHint(Locks, "Lots of lock operations");
	}
    }
    newFocus = currentFocus->constrain(Barriers);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > 
	    maxBarrierRate * highSyncThreshold * normalize) {
	    result->status = TRUE;
	    result->addHint(Barriers, "Lots of barrier operations");
	}
    }
    newFocus = currentFocus->constrain(Semaphores);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > maxSemaphoreRate * normalize) {
	    result->status = TRUE;
	    result->addHint(Semaphores, "Lots of semaphore operations");
	}
    }
    return;
}

Boolean smallSyncRegion_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = SyncOps.changeCollection(newMode);
    status = lockHoldingTime.changeCollection(newMode) && status;

    return(status);
}

/* ARGSUSED */
void smallSyncRegion_TEST(testValue *result, float normalize)
{
    float avgHold;

    avgHold = lockHoldingTime.value() / SyncOps.value();
    result->status = FALSE;
    if (avgHold / lockOverhead < minLockSize * normalize) {
	 // lock is too small.
	 result->status = TRUE;
    }
    return;
}

Boolean highIOwait_ENABLE(collectMode newMode)
{
    Boolean status;

    status = IOwait.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highIOwait_TEST(testValue *result, float normalize)
{
    result->status = FALSE;
    if (IOwait.value() > highIOthreshold * normalize) {
	result->status = TRUE;
      }
#ifdef PC_PRINT
    cout << "highIOwait >? V=" << IOwait.value() << " A=" <<
      (highIOthreshold * hysteresis) << "\n";
#endif
    return;
}


Boolean smallIO_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = IOops.changeCollection(newMode);
    status = IOBytes.changeCollection(newMode) && status;

    return(status);
}

/* ARGSUSED */
void smallIO_TEST(testValue *result, float normalize)
{
    float avgRead;

    avgRead = IOBytes.value() / IOops.value();
    // should be at least a disk block. 
    result->status = FALSE;
    if (avgRead < diskBlockSize * normalize) {
	result->status = TRUE;
    }
#ifdef PC_PRINT    
    cout << "smallIO >? V=" << avgRead << " A=" << (diskBlockSize * hysteresis)
      << "\n";
#endif
    return;
}

Boolean manySeeks_ENABLE(collectMode newMode)
{
    Boolean status = TRUE;

    status = IOwait.changeCollection(newMode);
    status = seekWait.changeCollection(newMode) && status;
    return(status);
}

/* ARGSUSED */
void manySeeks_TEST(testValue *result, float normalize)
{
    result->status = FALSE;
    if ((seekWait.value()/IOwait.value()) > seekBoundThreshold * normalize) {
	result->status = TRUE;
    }
    return;
}

Boolean highIOops_ENABLE(collectMode newMode)
{
    Boolean status;

    status = IOops.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highIOops_TEST(testValue *result, float normalize)
{
    double value;

    value = IOops.value();
    result->status = FALSE;
    if (value/contextSwitchLimit > highIOthreshold * normalize) {
	result->status = TRUE;
    }

    return;
}

Boolean highPageFaults_ENABLE(collectMode newMode)
{
    Boolean status;

    status = pageFaults.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highPageFaults_TEST(testValue *result, float normalize)
{
    double value;

    value = pageFaults.value();
    result->status = FALSE;
    if (value/pageFaultLimit > highLossThreshold) {
	result->status = TRUE;
    }

    return;
}

//
// Declare the actual test objects.
//
test highInstOverhead((changeCollectionFunc) &highFuncInst_ENABLE, 
	(evalFunc) &highFuncInst_TEST, "highInstOverhead");

test highSyncToCPURatio((changeCollectionFunc) &highSyncToCPURatio_ENABLE, 
	(evalFunc) &highSyncToCPURatio_TEST, "highSyncToCPURatio");

test highCPUtoSyncRatio((changeCollectionFunc) &highCPUtoSyncRatio_ENABLE, 
	(evalFunc) &highCPUtoSyncRatio_TEST, "highCPUtoSyncRatio");

test criticalSectionTooLarge((changeCollectionFunc) 
	&criticalSectionTooLarge_ENABLE, 
	(evalFunc) &criticalSectionTooLarge_TEST, "criticalSectionTooLarge");

test highVariationLock((changeCollectionFunc) &highVariationLock_ENABLE, 
	 (evalFunc) &highVariationLock_TEST, "highVariationLock");

test highSyncRate((changeCollectionFunc) &highSyncRate_ENABLE,
	(evalFunc) &highSyncRate_TEST, "highSyncRate");

test smallSyncRegion((changeCollectionFunc) &smallSyncRegion_ENABLE, 
	(evalFunc) &smallSyncRegion_TEST, "smallSyncRegion");

test highIOwait((changeCollectionFunc) &highIOwait_ENABLE, 
	(evalFunc) &highIOwait_TEST, "highIOwait");

test smallIO((changeCollectionFunc) &smallIO_ENABLE, 
	(evalFunc) &smallIO_TEST, "smallIO");

test manySeeks((changeCollectionFunc) &manySeeks_ENABLE, 
	(evalFunc) &manySeeks_TEST, "manySeeks");

test highIOops((changeCollectionFunc) &highIOops_ENABLE, 
	(evalFunc) &highIOops_TEST, "highIOops");

test highPageFaults((changeCollectionFunc) &highPageFaults_ENABLE, 
	(evalFunc) &highPageFaults_TEST, "highPageFaults");

//
// explanation functions.
//
/* ARGSUSED */
void defaultExplanation(searchHistoryNode *explainee)
{
    if (explainee && explainee->why) {
	printf("hypothesis: %s true for", explainee->why->name);
    } else {
	printf("***** NO HYPOTHESIS *******\n");
    }
    explainee->where->print();
    printf("at %f\n", PCcurrentTime);

}


void cpuProfileExplanation(searchHistoryNode *explainee)
{
    focus *i;
    float share;
    float elapsed;
    float totalCpu;
    focusList allProcedures;

    defaultExplanation(explainee);

    printf("your program is cpu bound, the top procedures by CPU time are:\n");
    // allProcedures = whereAxis->magnify(Procedures);
    allProcedures = currentFocus->magnify(Procedures);
    totalCpu = CPUtime.value(whereAxis);
    elapsed = elapsedTime.value(whereAxis);
    for (; i=*allProcedures; allProcedures++) {
	char name[80];
	if (!CPUtime.enabled(i)) continue;

	share = CPUtime.value(i) / totalCpu;
	i->print(name);
	printf(" %-25s %-7.2f %-7.2f\n", name, CPUtime.value(i), share*100.0);
    }
}

//
// Now for the Hypotheses
//

//
// root of search space.
//
hypothesis whyAxis(NULL, NULL, "topLevelHypothesis");

hypothesis ioBottleneck(&whyAxis, NULL, "ioBottleneck");

hypothesis cpuBottleneck(&whyAxis, NULL, "cpuBottleneck");

hypothesis vmBottleneck(&whyAxis, NULL, "vmBottleneck");

hypothesis instBottleneck(&whyAxis, &highInstOverhead, "instBottleneck");

hypothesis excessivePageFaults(&vmBottleneck, &highPageFaults, 
	"tooManyPageFaults");

hypothesis excessiveIOBlockingTime(&ioBottleneck, 
	&highIOwait, "excessiveIOBlockingTime");

hypothesis excessiveSmallIOops(&excessiveIOBlockingTime, &smallIO, 
	"excessiveSmallIOops");

hypothesis tooManySeeks(&excessiveIOBlockingTime, &manySeeks, "tooManySeeks");

hypothesis excessiveIOoperations(&ioBottleneck, &highIOops, 
	"excessiveIOoperations");

//
// CPU time bottlenecks.
//
hypothesis cpuBound(&cpuBottleneck, &highCPUtoSyncRatio, "cpuBound",
    (explanationFunction) cpuProfileExplanation);

//
// Sync bottlenecks
//
hypothesis syncBottleneck(&whyAxis, NULL, "syncBottleneck");

hypothesis excesiveBlockingTime(&syncBottleneck, 
	&highSyncToCPURatio, "excesiveBlockingTime");

//
// Stil being written.
//
// hypothesis lockTooLarge(&excesiveBlockingTime, &lockTooLarge);
//

hypothesis hotSyncObject(&excesiveBlockingTime, &highVariationLock, "hotSync");

hypothesis excesiveSyncRates(&syncBottleneck, &highSyncRate, 
	"excesiveSyncRates");

hypothesis syncRegionTooSmall(&excesiveSyncRates, &smallSyncRegion, 
    "SyncRegionTooSmall");
