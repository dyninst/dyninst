
/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * 
 * $Log: PCrules.C,v $
 * Revision 1.25  1995/06/02 20:50:14  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.24  1995/02/27  19:17:35  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.23  1995/02/16  08:19:18  markc
 * Changed Boolean to bool
 *
 * Revision 1.22  1994/12/21  00:46:33  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.21  1994/11/09  18:39:46  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.20  1994/10/25  22:08:10  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.19  1994/09/22  01:04:36  markc
 * Made lockOverhead 10 rather than 0 since it is used to divide
 *
 * Revision 1.18  1994/09/05  20:01:04  jcargill
 * Better control of PC output through tunable constants.
 *
 * Revision 1.17  1994/08/03  19:09:53  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.16  1994/07/25  04:47:09  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.15  1994/06/29  02:56:23  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.14  1994/06/27  18:55:10  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.13  1994/06/22  22:58:22  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.12  1994/06/14  17:18:59  markc
 * Modified highVariation test.  Changed threshold.
 *
 * Revision 1.11  1994/06/14  15:32:42  markc
 * Changed spelling of excessive.
 * Changed and renamed highVariationLock test to highVariation and made it more
 * generic, it now works across all sync objects.
 *
 * Revision 1.10  1994/06/12  22:40:50  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.9  1994/05/31  21:43:01  markc
 * Allow compensationFactor to be computed, but keep it within 0 and 1, which
 * is a short term fix.  Enable the hotSyncObject test in PCrules.C.
 *
 * Revision 1.8  1994/05/31  18:40:43  markc
 * Cleaned up debugging messages.  Commented out the code that divided by
 * metric values by the number of active processes in the test for
 * highSynctoCPU ratio until this is clarified by Jeff.
 *
 * Revision 1.7  1994/05/02  20:38:14  hollings
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


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <strstream.h>

#include "../src/pdMain/paradyn.h"
#include "PCwhy.h"
#include "PCmetric.h"
#include "PCshg.h"
#include "PCglobals.h"
#include "PCevalTest.h"
#include "../src/UIthread/UIstatDisp.h"

// enable printing for eval tests
#define PC_PRINT 

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
//const float maxIPSprocedureCallRate = 20000;
// paradyn and sparcs are faster
const float maxIPSprocedureCallRate = 2000000;

const float highInstOverheadThreshold	= 0.20;

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
const lockOverhead = 10;


bool highFuncInst_ENABLE(collectMode newMode)
{
    bool status = true;

    status = observedCost.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highFuncInst_TEST(testValue *result, float normalize)
{
    float oc;

    result->status = false;
    oc = observedCost.value();
    if (oc > highInstOverheadThreshold*normalize) {
	result->status = true;
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	cout << "highFuncInst >? V=" << oc << " A="
	     << (highInstOverheadThreshold*normalize) << "\n";
    }
    return;
}

bool highSyncToCPURatio_ENABLE(collectMode newMode)
{
    bool status = true;

    status = SyncTime.changeCollection(newMode);
    status = activeProcesses.changeCollection(whereAxis, newMode) && status;
    return(status);
}

/* ARGSUSED */
void highSyncToCPURatio_TEST(testValue *result, float normalize)
{
    float active;
    sampleValue st=0;

    result->status = false;
    active = activeProcesses.value(whereAxis);

    if (active > 0.0) {
	if ((st=SyncTime.value())/active > highSyncThreshold * normalize) {
	    result->status = true;
	}
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	if (active <= 0.0) {
	  cout << "highSyncToCPURatio Not active\n";
	} else {
	  cout << "highSyncToCPURatio" << (char *) currentFocus->getName();
	  cout << " >? V=";
	  cout << (st / active) <<
	    " A=" << (highSyncThreshold * normalize) << "\n"
	      << "SyncTime.value()=" << st << "\n" 
		<< "/active = " << active << "\n"
		  << "normalize =" << normalize << "\n";
	}
    }
    return;
}

bool highCPUtoSyncRatio_ENABLE(collectMode newMode)
{
    bool status = true;

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
    float cpu;
    float factor;
    float processes;
    bool conflict;
    focusList newFoci;

    result->status = false;
    if (!currentFocus->moreSpecific(Procedures, conflict)) {
	newFoci = whereAxis->magnify(Procedures);
	// 10% above average.
	factor = 1.1/newFoci.count();

	// guard for very simple programs.
	if (factor > highCPUtoSyncRatioThreshold) 
	    factor = highCPUtoSyncRatioThreshold;
    } else {
	factor = highCPUtoSyncRatioThreshold;
    }

    cpu = CPUtime.value();
    processes = activeProcesses.value(whereAxis);
    if (cpu/processes > factor * normalize) {
	result->status = true;
	result->addHint(Procedures, "Lots of cpu time");
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	cout << "highCPUtoSyncRatio " << (char *) currentFocus->getName();
	cout << " >? V=";
	cout << "(" << cpu << "/" << processes << ")" << (cpu / processes) << " A=" <<
	  "(" << factor << "*" << normalize << ")" << (factor * normalize) << "\n";
	if (factor < highCPUtoSyncRatioThreshold)
	    cout << "    factor was " << factor << "\n";
    }
    return;
}

bool criticalSectionTooLarge_ENABLE(collectMode newMode)
{
    bool status = true;

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
    result->status = false;
    if (pctHeld > largeCriticalSectionThreshold * normalize) {
	result->status = true;
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	cout << "critSectooLarge >? V=" << pctHeld << " A=" <<
	  (largeCriticalSectionThreshold * normalize) << "\n";
    }
    return;
}

//
// enable all sync objects to determine if the sync_wait time
// for one object is too high - this does not imply any where
// axis refinement
//
bool highVariation_ENABLE(collectMode newMode)
{
    bool status = true;
    focusList newFoci;
    focus *i;

    // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
    (void) (newFoci = currentFocus->magnify(SyncObject));
    for (; i = *newFoci; newFoci++)
      status = SyncTime.changeCollection(i, newMode) && status;

    return status;
}

//
// Test if one sync object is responsible for too much sync time
//
/* ARGSUSED */
void highVariation_TEST(testValue *result, float normalize)
{
  focus *i;
  float share, total;
  focusList allSO;

  result->status = false;
  
  // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
  (void) (allSO = currentFocus->magnify(SyncObject));
  total = SyncTime.value(whereAxis);

  for (; i = *allSO; allSO++)
    {
      if (!SyncTime.enabled(i)) continue;
      share = SyncTime.value(i) / total;
      if (share > .8 * normalize)
	{
	  result->status = true;
	  // define a hint to refine on the selected sync
	  if (i != currentFocus)
	    {
	      // there is something useful here.
	      result->addHint(i, "HOT Sync");
	    }
	}
    }
  return;
}

bool highSyncRate_ENABLE(collectMode newMode)
{
    focusList newFoci;
    bool syncObjStatus = false;
    focus *i;

    // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
    (void) (newFoci = currentFocus->magnify(SyncObject));
    for (; i = *newFoci; newFoci++)
      syncObjStatus = SyncOps.changeCollection(i, newMode) || syncObjStatus;

    return syncObjStatus;
}

/* ARGSUSED */
void highSyncRate_TEST(testValue *result, float normalize)
{
    focus *newFocus;

    result->status = false;
    newFocus = currentFocus->constrain(Locks);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > maxLockRate * normalize) {
	    result->status = true;
	    result->addHint(Locks, "Lots of lock operations");
	}
    }
    newFocus = currentFocus->constrain(Barriers);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > 
	    maxBarrierRate * highSyncThreshold * normalize) {
	    result->status = true;
	    result->addHint(Barriers, "Lots of barrier operations");
	}
    }
    newFocus = currentFocus->constrain(Semaphores);
    if (SyncOps.enabled(newFocus)) {
	if (SyncOps.value(newFocus) > maxSemaphoreRate * normalize) {
	    result->status = true;
	    result->addHint(Semaphores, "Lots of semaphore operations");
	}
    }
    return;
}

bool smallSyncRegion_ENABLE(collectMode newMode)
{
    bool status = true;

    status = SyncOps.changeCollection(newMode);
    status = lockHoldingTime.changeCollection(newMode) && status;

    return(status);
}

/* ARGSUSED */
void smallSyncRegion_TEST(testValue *result, float normalize)
{
    float avgHold;

    avgHold = lockHoldingTime.value() / SyncOps.value();
    result->status = false;
    if ((avgHold / lockOverhead) < (minLockSize * normalize)) {
	 // lock is too small.
	 result->status = true;
    }
    return;
}

bool highIOwait_ENABLE(collectMode newMode)
{
    bool status;

    status = IOwait.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highIOwait_TEST(testValue *result, float normalize)
{
    result->status = false;
    if (IOwait.value() > highIOthreshold * normalize) {
	result->status = true;
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	cout << "highIOwait >? V=" << IOwait.value() << " A=" <<
	  (highIOthreshold * normalize) << "\n";
    }
    return;
}


bool smallIO_ENABLE(collectMode newMode)
{
    bool status = true;

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
    result->status = false;
    if (avgRead < diskBlockSize * normalize) {
	result->status = true;
    }

    tunableBooleanConstant pcEvalPrint=tunableConstantRegistry::findBoolTunableConstant("pcEvalPrint");
    if (pcEvalPrint.getValue()) {
	cout << "smallIO >? V=" << avgRead << " A=";
	cout << (diskBlockSize * normalize) << "\n";
    }
    return;
}

bool manySeeks_ENABLE(collectMode newMode)
{
    bool status = true;

    status = IOwait.changeCollection(newMode);
    status = seekWait.changeCollection(newMode) && status;
    return(status);
}

/* ARGSUSED */
void manySeeks_TEST(testValue *result, float normalize)
{
    result->status = false;
    if ((seekWait.value()/IOwait.value()) > seekBoundThreshold * normalize) {
	result->status = true;
    }
    return;
}

bool highIOops_ENABLE(collectMode newMode)
{
    bool status;

    status = IOops.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highIOops_TEST(testValue *result, float normalize)
{
    double value;

    value = IOops.value();
    result->status = false;
    if (value/contextSwitchLimit > highIOthreshold * normalize) {
	result->status = true;
    }

    return;
}

bool highPageFaults_ENABLE(collectMode newMode)
{
    bool status;

    status = pageFaults.changeCollection(newMode);
    return(status);
}

/* ARGSUSED */
void highPageFaults_TEST(testValue *result, float normalize)
{
    double value;

    value = pageFaults.value();
    result->status = false;
    if (value/pageFaultLimit > highLossThreshold) {
	result->status = true;
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

test highVariation((changeCollectionFunc) &highVariation_ENABLE, 
	 (evalFunc) &highVariation_TEST, "highVariation");

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
    char *ptr;
    ostrstream name;

    if (explainee && explainee->why) {
	  PCstatusDisplay->updateStatusDisplay (PC_STATUSDISPLAY, 
	   "hypothesis: %s true for", explainee->why->name);
    } else {
	  PCstatusDisplay->updateStatusDisplay 
	    (PC_STATUSDISPLAY, "***** NO HYPOTHESIS *******\n");
    }
    name << *(explainee->where);
    ptr = name.str();
    PCstatusDisplay->updateStatusDisplay
      (PC_STATUSDISPLAY, "%s at %f\n", ptr, PCcurrentTime);
    delete(ptr); 
}


void cpuProfileExplanation(searchHistoryNode *explainee)
{
    focus *i;
    float share;
    float elapsed;
    float totalCpu;
    focusList allProcedures;
    char displayStr[80];

    defaultExplanation(explainee);
    PCstatusDisplay->updateStatusDisplay
      (PC_STATUSDISPLAY, 
       "your program is cpu bound, the top procedures by CPU time are:\n");

    // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
    (void) (allProcedures = currentFocus->magnify(Procedures));
    totalCpu = CPUtime.value(whereAxis);
    elapsed = elapsedTime.value(whereAxis);
    for (; i=*allProcedures; allProcedures++) {
	ostrstream name;
	if (!CPUtime.enabled(i)) continue;

	share = CPUtime.value(i) / totalCpu;
	name << *i;
	sprintf(displayStr, " %-25s %-7.2f %-7.2f\n", name.str(), 
		CPUtime.value(i), share*100.0);
	delete (name.str());
	PCstatusDisplay->updateStatusDisplay (PC_STATUSDISPLAY, displayStr);
      }
}

//
// Now for the Hypotheses
//

//
// root of search space.
//
hypothesis whyAxis(NULL, NULL, "topLevelHypothesis");

hypothesis syncBottleneck(&whyAxis, NULL, "syncBottleneck");

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

hypothesis excessiveBlockingTime(&syncBottleneck, 
	&highSyncToCPURatio, "excessiveBlockingTime");

//
// Stil being written.
//
// hypothesis lockTooLarge(&excessiveBlockingTime, &lockTooLarge);
//


hypothesis hotSyncObject(&excessiveBlockingTime, &highVariation, "hotSync");


hypothesis excessiveSyncRates(&syncBottleneck, &highSyncRate, 
	"excessiveSyncRates");

hypothesis syncRegionTooSmall(&excessiveSyncRates, &smallSyncRegion, 
    "SyncRegionTooSmall");
