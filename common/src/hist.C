/* 
 * Copyright (c) 1992 Barton P. Miller, Morgan Clark, Timothy Torzewski,
 *     Jeff Hollingsworth, and Bruce Irvin. All rights reserved.
 *
 * This software is furnished under the condition that it may not
 * be provided or otherwise made available to, or used by, any
 * other person.  No title to or ownership of the software is
 * hereby transferred.  The name of the principals
 * may not be used in any advertising or publicity related to this
 * software without specific, written prior authorization.
 * Any use of this software must include the above copyright notice.
 *
 */

/*
 * hist.C - routines to manage hisograms.
 *
 * $Log: hist.C,v $
 * Revision 1.6  1994/04/12 22:11:22  hollings
 * removed special case of bucket a zero value since it caused upcalls not to
 * happen.
 *
 * Revision 1.5  1994/03/08  17:12:29  hollings
 * Added fold callback and changed from multiple data callbacks to one per
 * histogram instance.  Also made the data callbacks happen once per bucket.
 *
 * Revision 1.4  1994/02/10  23:08:26  hollings
 * Fixed list.h ++ function to work when a hash table has an element at
 * slot zero in the table.
 *
 * Removed unused fields in hist class.
 *
 * Revision 1.3  1994/02/08  00:30:39  hollings
 * Make libutil more compatable with ATT CC.
 *
 * Revision 1.2  1994/01/26  04:53:42  hollings
 * Change to using <module>/h/.h
 *
 * Revision 1.1  1994/01/25  20:50:25  hollings
 * First real version of utility library.
 *
 * Revision 1.4  1993/12/15  21:06:23  hollings
 * changed initial bucket width to 0.1.
 *
 * Revision 1.3  1993/08/11  18:03:54  hollings
 * removed printf for folding hist.
 *
 * Revision 1.2  1993/08/05  18:55:08  hollings
 * general fixups and include format.
 *
 * Revision 1.1  1993/05/07  20:19:17  hollings
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "util/h/hist.h"

/* number of intervals at which we switch to regular histograms */
#define MAX_INTERVALS	15

static void smoothBins(Bin *bins, int i, timeStamp bucketSize);

int Histogram::numBins = 1000;
timeStamp Histogram::bucketSize = 0.1;
timeStamp Histogram::total_time = (Histogram::numBins * Histogram::bucketSize);
Histogram *Histogram::allHist = NULL;
int Histogram::lastGlobalBin = 0;

Histogram::Histogram(metricStyle type, dataCallBack d, foldCallBack f, void *c)
{
    smooth = False;
    lastBin = 0;
    metricType = type;
    intervalCount = 0;
    intervalLimit = MAX_INTERVALS;
    storageType = HistInterval;
    dataPtr.intervals = (Interval *) calloc(sizeof(Interval)*intervalLimit, 1);
    next = allHist;
    allHist = this;
    dataFunc = d;
    foldFunc = f;
    cData = c;
}

/*
 * Given an array of buckets - turn them into a histogram.
 *
 */
Histogram::Histogram(Bin *buckets, 
		     metricStyle type, 
		     dataCallBack d, 
		     foldCallBack f,
		     void *c)
{
    // First call default constructor.
    (void) Histogram(type, d, f, c);

    storageType = HistBucket;
    dataPtr.buckets = (Bin *) calloc(sizeof(Bin), numBins);
    memcpy(dataPtr.buckets, buckets, sizeof(Bin)*numBins);
}

#define MAX(a, b)	((a) > (b) ? (a):(b))

/*
 * addInterval - add a value to a histogram from start to end.
 *
 * start, and end are relative to the global start time (i.e. 0.0 is the
 *   left side of the first hist bin).
 */
void Histogram::addInterval(timeStamp start, 
			    timeStamp end, 
			    sampleValue value, 
			    Boolean smooth)
{
    Histogram *h;

    while ((end >= total_time) || (start >= total_time)) {
	// colapse histogram.
	foldAllHist();
    }

    lastBin = (int) (end / bucketSize);
    lastGlobalBin = lastBin;
    for (h=allHist; h; h=h->next) {
	if ((h->lastBin < lastGlobalBin)) {
	    lastGlobalBin = h->lastBin;
	}
    }

    if (storageType == HistInterval) {
	/* convert to buckets */
	convertToBins();
    }
    bucketValue(start, end, value, smooth);
}

void Histogram::convertToBins()
{
    int i;
    Interval *intervals;
    Interval *currentInterval;

    if (storageType == HistBucket) return;

    intervals = dataPtr.intervals;
    dataPtr.buckets = (Bin *) calloc(sizeof(Bin), numBins);
    memset(dataPtr.buckets, '\0', sizeof(Bin)*numBins);
    for (i=0; i < intervalCount; i++) {
	currentInterval = &(intervals[i]);
	bucketValue(currentInterval->start, currentInterval->end, 
	    currentInterval->value, smooth);
    }
    free(intervals);
    intervalCount = -1;
    intervalLimit = 0;
    storageType = HistBucket;
}

void Histogram::foldAllHist()
{
    int i;
    Bin *bins;
    Histogram *curr;

    bucketSize *= 2.0;
    total_time = (numBins * bucketSize);

    lastGlobalBin = 0;
    for (curr=allHist; curr; curr= curr->next) {
	// fold individual hist.
	if (curr->storageType == HistBucket) {
	    bins = curr->dataPtr.buckets;
	    for (i=0; i < numBins/2; i++) {
		bins[i] = bins[i*2] + bins[i*2+1];
	    }
	    curr->lastBin = i-1;
	    memset(&bins[i], '\0', (numBins - i) * sizeof(Bin));
	}
	if (foldFunc) (foldFunc)(bucketSize, cData);
    }
}

void Histogram::bucketValue(timeStamp start_clock, 
			   timeStamp end_clock, 
			   sampleValue value, 
			   Boolean smooth)
{
    register int i;
    timeStamp start, end;
    int first_bin, last_bin;
    timeStamp elapsed_clock = (timeStamp) 0.0;
    timeStamp first_bin_start, last_bin_start;
    sampleValue amt_first_bin, amt_last_bin, amt_other_bins;
    timeStamp time_in_first_bin, time_in_last_bin, time_in_other_bins;


    elapsed_clock = end_clock - start_clock;

    /* set starting and ending bins */
    first_bin = (int) (start_clock / bucketSize);
    assert(first_bin >= 0);
    assert(first_bin <= numBins);
    if (first_bin == numBins)
	first_bin = numBins-1;
    last_bin = (int) (end_clock / bucketSize);
    assert(last_bin >= 0);
    assert(last_bin <= numBins);
    if (last_bin == numBins)
	last_bin = numBins-1;
    /* set starting times for first & last bins */
    first_bin_start = bucketSize * first_bin;
    last_bin_start = bucketSize  * last_bin;

    if (metricType == SampledFunction) {
	for (i=first_bin; i <= last_bin; i++) {
	    dataPtr.buckets[i] = value * bucketSize;
	}
    } else {
	if (last_bin == first_bin) {
	    dataPtr.buckets[first_bin] += value;
	    if (smooth && (dataPtr.buckets[first_bin] > bucketSize)) {
		/* value > 100% */
		smoothBins(dataPtr.buckets, first_bin, bucketSize);
	    }
	    return;
	}

	/* determine how much of the first & last bins were in this interval */
	time_in_first_bin = bucketSize - (start_clock - first_bin_start);
	time_in_last_bin = end_clock - last_bin_start;
	time_in_other_bins = 
	    MAX(elapsed_clock - (time_in_first_bin + time_in_last_bin), 0);

	/* determine how much of value should be in each bin in the interval */
	amt_first_bin = (time_in_first_bin / elapsed_clock) * value;
	amt_last_bin  = (time_in_last_bin  / elapsed_clock) * value;
	amt_other_bins = (time_in_other_bins / elapsed_clock) * value;
	if (last_bin > first_bin+1) 
	    amt_other_bins /= (last_bin - first_bin) - 1.0;

	/* add the appropriate amount of time to each bin */
	dataPtr.buckets[first_bin] += amt_first_bin;
	dataPtr.buckets[last_bin]  += amt_last_bin;
	for (i=first_bin+1; i < last_bin; i++)
	    dataPtr.buckets[i]  += amt_other_bins;
    }

    /* limit absurd time values - anything over 100% is pushed back into 
       previous buckets */
    if (smooth) {
	for (i = first_bin; i <= last_bin; i++) {
	    if (dataPtr.buckets[i] > bucketSize) 
		smoothBins(dataPtr.buckets, i, bucketSize);
	}
    }

    // inform users about the data.
    if (dataFunc) {
	start = first_bin * bucketSize;
	for (i=first_bin; i < last_bin; i++) {
	    end = start + bucketSize;
	    (dataFunc)(start, end, dataPtr.buckets[i], cData);
	    start = end;
	}
    }
}


static void smoothBins(Bin *bins, int i, timeStamp bucketSize)
{
    int j;
    sampleValue extra_time;

    extra_time = (bins[i] - bucketSize);
    bins[i] = bucketSize;
    j = i - 1;
    while ((j >= 0) && (extra_time > 0.0)) {
	bins[j] += extra_time;
	extra_time = 0.0;
	if (bins[j] > bucketSize) {
	    extra_time = (bins[j] - bucketSize);
	    bins[j] = bucketSize;
	}
	j--;
    }
    if (extra_time > 0.0) {
	fprintf(stderr, "**** Unable to bucket %f seconds\n", extra_time);
	abort();
    }
}


static double splitTime(timeStamp startInterval, 
			timeStamp endInterval, 
			timeStamp startLimit, 
			timeStamp endLimit, 
			sampleValue value)
{
    sampleValue time;
    timeStamp inv_length;

    inv_length = endInterval-startInterval;
    if ((startInterval >= startLimit) && (endInterval <= endLimit)) { 	
	time = value;
    } else if (startInterval == endInterval) {	
	/* can't split delta functions */		
	time = 0.0;
    } else if ((startInterval >= startLimit) && (startInterval <= endLimit)) {
	/* overlaps the end */				
	time = value * (endLimit - startInterval) / inv_length;
    } else if ((endInterval <= endLimit) && (endInterval >= startLimit)) { 
	/* overlaps the end */				
	time = value * (endInterval - startLimit) / inv_length;    
    } else if ((startInterval < startLimit) && (endInterval > endLimit)) {
	/* detail contains interval */
	time = value * (endLimit - startLimit) / inv_length;
    } else {
	/* no overlap of time */
	time = 0.0;
    }
    return(time);
}

/*
 * Get the component of the histogram from start to end.
 *
 */
sampleValue Histogram::getValue(timeStamp start, timeStamp end)
{		
    int i;
    Interval *cp;
    sampleValue retVal = 0.0;
    int first_bin, last_bin;
    double pct_first_bin, pct_last_bin;

    if (storageType == HistInterval) {
	if (metricType == SampledFunction) {
	    // compute weight average of sample.
	    for (i=0; i < intervalCount; i++) {
		cp = &(dataPtr.intervals[i]);
		retVal += cp->value * (cp->end - cp->start);
	    }
	} else {
	    // total over the interval.
	    for (i=0; i < intervalCount; i++) {
		cp = &(dataPtr.intervals[i]);
		retVal += splitTime(cp->start, cp->end, start, end, cp->value);
	    }
	}
    } else {
	first_bin = (int) (start/bucketSize);
	/* round up */
	last_bin = (int) (end/bucketSize+0.5);
	if (last_bin >= numBins) last_bin = numBins-1;
	if (first_bin == last_bin) {
	    retVal = dataPtr.buckets[first_bin];
	} else {
	    /* (first_bin+1)*bucketSize == time at end of first bucket */
	    pct_first_bin = (((first_bin+1)*bucketSize)-start)/bucketSize;
	    retVal += pct_first_bin * dataPtr.buckets[first_bin];
	    /* last_bin+*bucketSize == time at start of last bucket */
	    pct_last_bin = (end - last_bin*bucketSize)/bucketSize;
	    retVal += pct_last_bin * dataPtr.buckets[last_bin];
	    for (i=first_bin+1; i <= last_bin-1; i++) {
		retVal += dataPtr.buckets[i];
	    }
	}
    }
    return(retVal);
}

sampleValue Histogram::getValue()
{		
    return(getValue(0.0, numBins * bucketSize));
}
