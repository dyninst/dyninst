
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
 * $Log: PCmetric.C,v $
 * Revision 1.29  1995/08/01 02:18:22  newhall
 * changes to support phase interface
 *
 * Revision 1.28  1995/06/02  20:50:10  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.27  1995/02/27  19:17:33  tamches
 * Changes to code having to do with tunable constants.
 * First, header files have moved from util lib to TCthread.
 * Second, tunable constants may no longer be declared globally.
 * Third, accessing tunable constants is different.
 *
 * Revision 1.26  1995/02/16  08:19:13  markc
 * Changed Boolean to bool
 *
 * Revision 1.25  1994/12/21  00:46:31  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.24  1994/11/09  18:39:42  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.23  1994/11/04  12:59:53  markc
 * Set start of interval to time of first sample if that interval start is earlier.
 *
 * Revision 1.22  1994/10/25  22:08:05  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.21  1994/09/22  01:03:16  markc
 * Added const char* to PCmetric constructors
 *
 * Revision 1.20  1994/08/05  16:04:13  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.19  1994/08/03  19:09:51  hollings
 * split tunable constant into float and boolean types
 *
 * added tunable constant for printing tests as they avaluate.
 *
 * added code to compute the min interval data has been enabled for a single
 * test rather than using a global min.  This prevents short changes from
 * altering long term trends among high level hypotheses.
 *
 * Revision 1.18  1994/07/28  22:34:01  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.17  1994/07/25  04:47:07  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.16  1994/07/14  23:48:28  hollings
 * added checks to make sure met is non null.
 *
 * Revision 1.15  1994/07/02  01:43:37  markc
 * Remove aggregation operator from enableDataCollection call.
 *
 * Revision 1.14  1994/06/29  02:56:20  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.13  1994/06/22  22:58:20  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.12  1994/06/14  15:29:45  markc
 * Added aggregationOperator argument to enableDataCollection call.
 *
 * Revision 1.11  1994/05/31  21:43:00  markc
 * Allow compensationFactor to be computed, but keep it within 0 and 1, which
 * is a short term fix.  Enable the hotSyncObject test in PCrules.C.
 *
 * Revision 1.10  1994/05/31  18:30:36  markc
 * Added msg_bytes_sent and msg_bytes_recv as default metrics.
 *
 * Revision 1.9  1994/05/19  00:00:28  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 * Revision 1.8  1994/05/18  02:49:28  hollings
 * Changed the time since last change to use the time of the first sample
 * arrivial after the change (rather than the time of the change).
 *
 * Revision 1.7  1994/05/18  00:48:54  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.6  1994/05/02  20:38:11  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.5  1994/04/12  15:32:47  hollings
 * generalized hysteresis into a normalization constant to cover pause,
 * contention, and ignored bottlenekcks too.
 *
 * Revision 1.4  1994/03/11  21:04:07  hollings
 * Wait min. observation time between calls to eval.  This prevents over
 * evaulation of the hypotheses when new data is arriving at a high frequency.
 *
 * Revision 1.3  1994/03/08  17:39:38  hollings
 * Added foldCallback and getResourceListName.
 *
 * Revision 1.2  1994/03/01  21:25:11  hollings
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "../TCthread/tunableConst.h"
#include "PCmetric.h"
#include "PCglobals.h"
#include "PCwhen.h"
#include "PCauto.h"

bool fetchPrint = false;

timeStamp PCcurrentTime;
int PCautoRefinementLimit;
timeStamp PCendTransTime;
timeStamp PCstartTransTime;
stringPool PCmetricStrings;
int samplesSinceLastChange;
timeStamp PCshortestEnableTime;
timeStamp PClastTestChangeTime;
bool explainationFlag = false;
extern List<datum *> *enabledGroup;

// for debugging
// #define PC_PRINT

//
// ugly global to pass cost around for a single hypothesis.
//
extern float globalPredicatedCost;

warningLevel noDataWarning = wNever;
extern perfStreamHandle pc_ps_handle;
extern searchHistoryNode *currentSHGNode;
extern bool doAutoRefinement(searchHistoryNode *, int);

PCmetric::PCmetric(const char *id)
{
    name = PCmetricStrings.findAndAdd(id);
    aggregationOperator = aggSum;
    calc = CalcNormalize;
    allMetrics.add(this, name);
}

PCmetric::PCmetric(const char *id, int ag, calcType c)
{
    name = PCmetricStrings.findAndAdd(id);
    aggregationOperator = ag;
    calc = c;
    allMetrics.add(this, name);
}

ostream& operator <<(ostream &os, PCmetric & pm)
{
    os << (char *) pm.name;
    return os; // added AT 12/9/94
}

sampleValue PCmetric::value()
{
    return(value(currentFocus, (timeStamp) PCstartTransTime, PCendTransTime)); 
}

/* ARGSUSED */
sampleValue PCmetric::value(focus *f)
{
    if (!f) return(0.0);

    return(value(f, (timeStamp) PCstartTransTime, PCendTransTime));
}

/* ARGSUSED */
bool PCmetric::enabled(focus *f)
{
    datum *val;

    if (!f) return(false);

    val = findDatum(f);
    if (!val) return(false);

    return(val->enabled);
}

sampleValue PCmetric::value(focusList fList)
{
    return(value(fList, (timeStamp) PCstartTransTime, PCendTransTime));
}

/* ARGSUSED */
sampleValue PCmetric::value(focus *f, timeStamp start, timeStamp end)
{
    sampleValue ret;
    datum *val;
    extern void printCurrentFocus();

    val = findDatum(f);
    if (!val) {
	// This situation happens when an item that is requested to be enabled
	// is not avaiable but other tests that are requested toogether are.
      return(0.0);
    }

    if (!val->enabled) {
	cout << "Attempt to use disabled metric\n";
	cout << "*** metric " <<  (char*)name;
	cout << " for focus: " <<  *f << endl;
	abort();
    }

    // TODO mdc - added to support searching - use proper interval for evalTests
    // ask for time only from when it was enabled.
    if (val->firstSampleTime > start)
      start = val->firstSampleTime;
    // assert(start >= val->firstSampleTime);

    val->lastUsed = end;


    // ret = val->sample;
    ret = val->hist->getValue(start, end);

    // see if we should normalize it.
    if (calc == CalcNormalize) {
	ret /= (end-start);
    }

    if (fetchPrint) {
	cout << "value for " << name << " from " << start << " to " << end;
	cout << " = " << ret << "\n";
    }

    return(ret);
}

/* ARGSUSED */
bool PCmetric::changeCollection(collectMode newMode)
{
    return(changeCollection(currentFocus, newMode));
}

/* ARGSUSED */
bool PCmetric::changeCollection(focusList fl, collectMode newMode)
{
    focus *f;
    bool status = false;

    for (; f = *fl; fl++) {
	status = changeCollection(f, newMode) || status;
    }
    return(status);
}

/* ARGSUSED */
bool PCmetric::changeCollection(focus *f, collectMode newMode)
{
    datum *val;
    extern void printCurrentFocus();

    PClastTestChangeTime = PCcurrentTime;

    // find the current sample for the given focus.
    val = findDatum(f);
    if (newMode == getCollectionCost) {
	float predictedCost;

	if (!met) return(false);

	assert(f->data);
	predictedCost = dataMgr->getPredictedDataCost(f->data->getHandle(),
						      met->getHandle());

	globalPredicatedCost += predictedCost;

	return(true);
    } else if (newMode == enableCollection) {
	if (!met) return(false);
	if (!val) {
	    val = new(datum);
	    val->f = f;
	    val->metName = name;
	    val->resList = f->data;
	    val->hist = new Histogram(EventCounter, NULL, NULL, NULL);
	    val->enabled = false;
	    val->refCount = 0;
	    addDatum(val);
	}
	val->refCount++;
	if (!val->enabled) {
	    val->samplesSinceEnable = 0;
	    val->lastSampleTime = 0.0;
	    val->firstSampleTime = 0.0;
	    val->used = true;

	    // TEMP until store handle rather than ptr to performanceStream
	    vector<resourceHandle> *r_handles = 
		 resourceList::getResourceHandles(val->resList->getHandle());
	    // TODO: change this to using CurrentPhase rather that GlobalPhase
            metricInstInfo *enabled_mi = dataMgr->enableDataCollection(
							pc_ps_handle,
						        r_handles,
						        met->getHandle(),
							GlobalPhase,
							0,0);
            if(!enabled_mi) return(false);
            val->mi = metricInstance::getMI(enabled_mi->mi_id); 
	    if (val->mi) {
		val->mi_id = enabled_mi->mi_id;
		// only the data that really exists gets enabled.
		miToDatumMap.add(val, val->mi);
		val->enabled = true;
	    }
	}
	if (val->mi) {
	    // global variable to record enables for this request
	    assert(enabledGroup);
	    enabledGroup->add(val);

	    return(true);
	} else {
	    return(false);
	}
    } else if (newMode == disableCollection) {
	if(!val){
            return(true);
	}
	val->refCount--;
	if (val->refCount == 0) {
	    val->enabled = false;

	    if (val->mi) {
	       // TODO: change to CurrentPhase rather that GlobalPhase
		dataMgr->disableDataCollection(pc_ps_handle, 
					       val->mi_id,
					       GlobalPhase);
		val->totalUsed += val->lastUsed - val->firstSampleTime;
	    }
	}
	return(true);
    } else {
	// unknown mode.
	abort();
	return(false);
    }
}

sampleValue PCmetric::value(focusList fList, timeStamp start, timeStamp end)
{
    switch (aggregationOperator) {
	case aggSum:
	    return(sum(fList, start, end));
	case aggAvg:
	    return(avg(fList, start, end));
	case aggMin:
	    return(min(fList, start, end));
	case aggMax:
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
    bool first = true;

    min = 0.0;
    for (; curr=*fList; fList++) {
	val = value(curr, start, end);
	if ((val < min) || first) {
	    min = value(curr, start, end);
	    first = false;
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
    firstSampleTime = 0.0;
    lastSampleTime = 0.0;
    lastUsed = 0.0;
    samplesSinceEnable=0;
    time = -1.0;
    enabled = false;
    used = false;
    hist = NULL;
    refCount = 0;
    metName = NULL;
    mi = NULL;
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

//
// find the min samples seen for any active datum.
//
// 
timeStamp globalMinEnabledTime()
{
    datum *d;
    datumList curr;
    timeStamp min;
    timeStamp elapsed;

    min = HUGE_VAL;
    PCendTransTime = HUGE_VAL;
    PCstartTransTime = 0.0;
    for (curr = miToDatumMap; d = *curr; curr++) {
	elapsed = d->lastSampleTime - d->firstSampleTime;
	if ((d->enabled) && (elapsed < min)) {
	    min = elapsed;
	}
	if ((d->enabled) && (d->firstSampleTime > PCstartTransTime)) {
	    PCstartTransTime = d->firstSampleTime;
	}

	// earliest last sample 
	if ((d->enabled) && (d->lastSampleTime < PCendTransTime)) {
	  PCendTransTime = d->lastSampleTime;
	  // TODO mdc
	  // if (!PCendTransTime) {
	  // printf("0 at %s:%s\n", (char*) d->metName, (char*)d->f->getName());
	  // }
	}
    }
    return(min);
}

void datum::newSample(timeStamp start, timeStamp end, sampleValue value)
{
    hist->addInterval(start, end, value, false);

    sample += value;
    samplesSinceEnable++;
    lastSampleTime = end;
    if (samplesSinceEnable == 1) {
	// consider the sample enabled only from the time we get the first
	//   sample.  This removes inst latency, and the fact currentTime is
	//   trailing edge of the time wavefront.
	firstSampleTime = start;
    }

    // TODO mdc
    if (!end) {
      printf("newSample 0 at %s:%s\n", (char*) metName, (char*)f->getName());
    }
    samplesSinceLastChange = globalMinSampleCount();
    PCshortestEnableTime = globalMinEnabledTime();
}

PCmetricList allMetrics;
focus *tempFocus;

PCmetric *findMetric(char *name)
{
    stringHandle id;

    id = PCmetricStrings.findAndAdd(name);
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
PCmetric elapsedTime("exec_time", aggMax, CalcSum);
PCmetric activeProcesses("active_processes");
PCmetric msgBytes("msg_bytes");
PCmetric msgBytesSent("msg_bytes_sent");
PCmetric msgBytesRecv("msg_bytes_recv");
PCmetric msgs("msgs");
PCmetric fileWriteBytes("file_wr_bytes");
PCmetric fileWriteOps("file_writes");
PCmetric fileReadBytes("file_rd_bytes");
PCmetric fileReadOps("file_reads");
PCmetric procedureCalls("procedure_calls");
PCmetric blockingLocks("blocking_locks");
PCmetric LockWaits("spin_waits");
PCmetric unbalancedWorkWaits("system_busy_waits");

PCmetric observedCost("observed_cost", aggMax, CalcNormalize);
PCmetric compensationFactor("pause_time", aggAvg, CalcNormalize);

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
