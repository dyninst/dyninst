/*
 * 
 * $Log: PCmetric.C,v $
 * Revision 1.2  1994/03/01 21:25:11  hollings
 * added tunable constants.
 *
 * Revision 1.1  1994/02/02  00:38:16  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.11  1993/09/03  19:02:22  hollings
 * added findMetric
 *
 * Revision 1.10  1993/08/30  22:14:05  hollings
 * Changed starting time to be the starting time of the first sample with
 * this metric enabled.
 *
 * Revision 1.9  1993/08/30  18:35:11  hollings
 * added check to make sure we get a min. number of samples from each source
 * before trying new refinements.
 *
 * Revision 1.8  1993/08/11  18:56:36  hollings
 * added cost model stuff.
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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCmetric.C,v 1.2 1994/03/01 21:25:11 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util/h/tunableConst.h"
#include "PCmetric.h"
#include "PCglobals.h"
#include "PCwhen.h"
#include "PCauto.h"

tunableConstant sufficientTime(6.0, 0.0, 1000.0, NULL, "sufficientTime",
  "How long to wait (in samples) before we can concule a hypothesis is false.");

int totalHistograms;
timeStamp currentTime;
int autoRefinementLimit;
stringPool metricStrings;
int samplesSinceLastChange;
timeStamp lastTestChageTime;
Boolean explainationFlag = FALSE;

//
// ugly global to pass cost around for a single hypothesis.
//
extern float globalPredicatedCost;

//
// Fix this soon... This should be based on some real information.
//
tunableConstant minObservationTime(0.5, 0.0, 60.0, NULL, "minObservationTime",
 "min. time (in seconds) to wait after chaning inst to start try hypotheses.");

warningLevel noDataWarning = wNever;
extern performanceStream *pcStream;
extern searchHistoryNode *currentSHGNode;
extern Boolean doAutoRefinement(searchHistoryNode *, int);

PCmetric::PCmetric(char *id)
{
    name = metricStrings.findAndAdd(id);
    aggregationOperator = Sum;
    calc = CalcNormalize;
    allMetrics.add(this, name);
}

PCmetric::PCmetric(char *id, aggregation ag, calcType c)
{
    name = metricStrings.findAndAdd(id);
    aggregationOperator = ag;
    calc = c;
    allMetrics.add(this, name);
}

void PCmetric::print()
{
    printf(name);
}

sampleValue PCmetric::value()
{
    timeStamp end;

    end = currentTime;
    return(value(currentFocus, (timeStamp) currentTime, end)); 
}

/* ARGSUSED */
sampleValue PCmetric::value(focus *f)
{
    timeStamp end;

    if (!f) return(0.0);

    end = currentTime;
    return(value(f, (timeStamp) currentTime, end));
}

/* ARGSUSED */
Boolean PCmetric::enabled(focus *f)
{
    datum *val;

    if (!f) return(False);

    val = findDatum(f);
    if (!val) return(False);

    return(val->enabled);
}

sampleValue PCmetric::value(focusList fList)
{
    timeStamp end;

    end = currentTime;
    return(value(fList, (timeStamp) currentTime, end));
}

/* ARGSUSED */
sampleValue PCmetric::value(focus *f, timeStamp start, timeStamp end)
{
    sampleValue ret;
    datum *val;
    extern void printCurrentFocus();

    val = findDatum(f);
    if (!val) {
	// printf("*** could not find metric %s for focus: ", name);
	// f->print();
	// printf("\n");
	// abort();
	// This situation happens when on item that is requested to be enabled
	// is not avaiable but other tests that are or toogether are.
	return(0.0);
    }

    if (!val->enabled) {
	printf("Attempt to use disabled metric\n");
	printf("*** metric %s for focus: ", name);
	f->print();
	printf("\n");
	abort();
    }

    // ask for time only from when it was enabled.
    start = val->enableTime;
    val->lastUsed = end;

    if (fetchPrint == TRUE) {
	printf("getting value for %s from %f to %f\n", name, 
	    start, end);
    }

    ret = val->sample;

    // see if we should normalize it.
    if (calc == CalcNormalize) {
	ret /= ((end-start) + val->totalUsed);
    }
    return(ret);
}

/* ARGSUSED */
Boolean PCmetric::changeCollection(collectMode newMode)
{
    return(changeCollection(currentFocus, newMode));
}

/* ARGSUSED */
Boolean PCmetric::changeCollection(focusList fl, collectMode newMode)
{
    focus *f;
    Boolean status = FALSE;

    for (; f = *fl; fl++) {
	status = changeCollection(f, newMode) || status;
    }
    return(status);
}

/* ARGSUSED */
Boolean PCmetric::changeCollection(focus *f, collectMode newMode)
{
    datum *val;
    extern void printCurrentFocus();

    lastTestChageTime = currentTime;

    // find the current sample for the given focus.
    val = findDatum(f);
    if (newMode == getCollectionCost) {
	float predictedCost;

	predictedCost = dataMgr->getPredictedDataCost(context, f->data, met);

	globalPredicatedCost += predictedCost;

	return(TRUE);
    } else if (newMode == enableCollection) {
	if (!val) {
	    val = new(datum);
	    val->f = f;
	    val->resList = f->data;
	    val->hist = NULL;
	    val->enabled = FALSE;
	    val->refCount = 0;
	    addDatum(val);
	}
	val->refCount++;
	if (!val->enabled) {
	    val->samplesSinceEnable = 0;
	    val->enableTime = currentTime;
	    val->used = TRUE;
	    val->mi = dataMgr->enableDataCollection(pcStream,val->resList, met);
	    if (val->mi) {
		// only the data that really exhists gets enabled.
		// val->hist->enable();
		miToDatumMap.add(val, val->mi);
		val->enabled = TRUE;
	    }
	}
	if (val->mi) 
	    return(TRUE);
	else 
	    return(FALSE);
    } else if (newMode == disableCollection) {
	assert(val);
	val->refCount--;
	if (val->refCount == 0) {
	    val->enabled = FALSE;

	    if (val->mi) {
		dataMgr->disableDataCollection(pcStream, val->mi);
		// val->hist->disable();
		val->totalUsed += val->lastUsed - val->enableTime;
	    }
	}
	return(TRUE);
    } else {
	// unknown mode.
	abort();
	return(FALSE);
    }
}

sampleValue PCmetric::value(focusList fList, timeStamp start, timeStamp end)
{
    switch (aggregationOperator) {
	case Sum:
	    return(sum(fList, start, end));
	case Avg:
	    return(avg(fList, start, end));
	case Min:
	    return(min(fList, start, end));
	case Max:
	    return(max(fList, start, end));
	default:
	    abort();
	    return(0);
    }
}

sampleValue PCmetric::min(focusList fList, timeStamp start, timeStamp end)
{
    sampleValue min;
    sampleValue val;
    focus *curr;
    Boolean first = TRUE;

    min = 0.0;
    for (; curr=*fList; fList++) {
	val = value(curr, start, end);
	if ((val < min) || (first == TRUE)) {
	    min = value(curr, start, end);
	    first = FALSE;
	}
    }
    return(min);
}



sampleValue PCmetric::max(focusList fList, timeStamp start, timeStamp end)
{
    sampleValue max;
    sampleValue cValue;
    focus *curr;

    max = 0.0;
    for (; curr=*fList; fList++) {
	cValue = value(curr, start, end);
	if (cValue > max) max = cValue;
    }
    return(max);
}



sampleValue PCmetric::sum(focusList fList, timeStamp start, timeStamp end)
{
    sampleValue total;
    focus *curr;

    total = 0;
    for (; curr=*fList; fList++) {
	total += value(curr, start, end);
    }
    return(total);
}



sampleValue PCmetric::avg(focusList fList, timeStamp start, timeStamp end)
{
    sampleValue total;
    focus *curr;

    total = 0.0;
    for (; curr=*fList; fList++) {
	total += value(curr, start, end);
    }
    return(total/fList.count());
}

datum::datum()
{
    f = NULL;
    resList = NULL;

    sample = 0.0;
    totalUsed = 0.0;

    enableTime = 0.0;
    lastUsed = 0.0;

    time = -1.0;
    enabled = FALSE;
    used = FALSE;
    hist = NULL;

    refCount = 0;
}

//
// find the min samples seen for any active datum.
//
// 
int globalMinSampleCount()
{
    datum *d;
    datumList curr;
    unsigned int min;

    min = 0xffffffff;
    for (curr = miToDatumMap; d = *curr; curr++) {
	if ((d->enabled) && (d->samplesSinceEnable < min)) {
	    min = d->samplesSinceEnable;
	}
    }
    return(min);
}

void datum::newSample(timeStamp start, timeStamp end, sampleValue value)
{
    Boolean changed;
    extern Boolean doScan();
    extern Boolean verifyPreviousRefinements();

    sample += value;
    samplesSinceEnable++;
    if (samplesSinceEnable == 1) {
	// consider the sample enabled only from the time we get the first
	//   sample.  This removes inst latency, and the fact currenTime is
	//   trailing edge of the time wavefront.
	enableTime = start;
    }

    samplesSinceLastChange = globalMinSampleCount();

    if (end > currentTime) {
	currentTime = end;

	if (currentTime < lastTestChageTime + minObservationTime.getValue()) 
	    return;

	/* try to evaluate a test */
	changed = doScan();
	if (autoRefinementLimit != 0) {
	    if (changed) {
		autoTestRefinements();
	    } else if ((currentTime > timeLimit) && 
		       (samplesSinceLastChange >= sufficientTime.getValue())) {
		// we have waited sufficient observation time move on.
		printf("autorefinement timelimit reached at %f\n", currentTime);
		printf("samplesSinceLastChange = %d\n", samplesSinceLastChange);
		autoTimeLimitExpired();
	    }
	} else if (changed) {
	    if (verifyPreviousRefinements()) {
		dataMgr->pauseApplication(context);
		printf("application paused\n");
	    } else {
		// previous refinement now false.
		dataMgr->pauseApplication(context);
	    }
	}
    }
}

PCmetricList allMetrics;
focus *tempFocus;

PCmetric *findMetric(char *name)
{
    char *id;

    id = metricStrings.findAndAdd(name);
    return(allMetrics.find(id));
}

//
// The actual metrics.
//

PCmetric CPUtime("cpu");
PCmetric SyncTime("sync_wait");
PCmetric SyncOps("sync_ops");
PCmetric lockHoldingTime("locks_held");
PCmetric IOwait("io_wait");
PCmetric IOops("io_oops");
PCmetric IOBytes("io_bytes");
PCmetric seekWait("seek_wait");
PCmetric elapsedTime("exec_time", Max, CalcSum);
PCmetric activeProcesses("active_processes");
PCmetric msgBytes("msg_bytes");
PCmetric msgs("msgs");
PCmetric fileWriteBytes("file_wr_bytes");
PCmetric fileWriteOps("file_writes");
PCmetric fileReadBytes("file_rd_bytes");
PCmetric fileReadOps("file_reads");
PCmetric procedureCalls("procedure_calls");
PCmetric blockingLocks("blocking_locks");
PCmetric LockWaits("spin_waits");
PCmetric unbalancedWorkWaits("system_busy_waits");

// EDCU based metrics.
PCmetric pageFaults("vm_faults");

// PCmetric systemTime("system_time");
// PCmetric runningProcesses("num_running");
// PCmetric runableProcesses("num_runable");
// PCmetric systemCallRate("syscall_rate");
// PCmetric contextSwitchRate("cs_rate");
// PCmetric interuptRate("intr_rate");
// PCmetric trapRate("trap_rate");
// PCmetric packetsIn("pkts_in");
// PCmetric packetsOut("pkts_out");
// PCmetric pageReclaims("vm_rec");
// PCmetric dirtyPageReclaims("vm_dirty_rec");
// PCmetric zeroPageCreates("vm_zero_pages");
// PCmetric codePageIns("vm_code_in");
// PCmetric dataPageIns("vm_pg_ins");
// PCmetric pageOuts("vm_pg_outs");
// PCmetric swapIn("vm_swp_ins");
// PCmetric swapOut("vm_swp_outs");
// PCmetric vmDeficit("vm_deficit");
// PCmetric vmFree("vm_free");
// PCmetric vmDirty("vm_dirty");
// PCmetric activeVM("vm_avm");

datumList miToDatumMap;
