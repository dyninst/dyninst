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
 * Revision 1.15  1995/08/01 01:56:32  newhall
 * fix to how global time is computed
 *
 * Revision 1.14  1995/07/20 22:30:13  rbi
 * Fixed a folding bug
 *
 * Revision 1.13  1995/07/06  01:52:13  newhall
 * support for Histograms with non-zero start time
 *
 * Revision 1.12  1995/06/02  21:00:08  newhall
 * added a NaN value generator
 * fixed memory leaks in Histogram class
 * added newValue member with a vector<sampleInfo *> to class sampleInfo
 *
 * Revision 1.11  1995/02/16  09:28:02  markc
 * Removed compiler warnings.
 * Changed Boolean to bool
 *
 * Revision 1.10  1994/06/29  02:48:31  hollings
 * Added destructor to Histogram class.
 *
 * Revision 1.9  1994/05/10  03:56:47  hollings
 * Changed hist data upcall to return array of buckets not single value.
 *
 * Revision 1.8  1994/04/30  21:00:03  hollings
 * Fixed bug in fold callback that caused all fold message to go to the
 * Histogram that caused the fold not the correct ones.
 *
 * Revision 1.7  1994/04/20  15:19:30  hollings
 * Added method to get histogram buckets.
 *
 * Revision 1.6  1994/04/12  22:11:22  hollings
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

#ifdef n_def
int Histogram::numBins = 1000;
timeStamp Histogram::bucketSize = 0.1;
timeStamp Histogram::total_time = (Histogram::numBins * Histogram::bucketSize);
Histogram *Histogram::allHist = NULL;
int Histogram::lastGlobalBin = 0;
#endif

int Histogram::numBins = 1000;
int Histogram::lastGlobalBin = 0;
timeStamp Histogram::baseBucketSize = 0.1;
timeStamp Histogram::globalBucketSize = 0.1;
vector<Histogram *> Histogram::allHist;

Histogram::Histogram(metricStyle type, dataCallBack d, foldCallBack f, void *c)
{
    smooth = false;
    lastBin = 0;
    metricType = type;
    intervalCount = 0;
    intervalLimit = MAX_INTERVALS;
    storageType = HistInterval;
    dataPtr.intervals = (Interval *) calloc(sizeof(Interval)*intervalLimit, 1);
    allHist += this;
    dataFunc = d;
    foldFunc = f;
    cData = c;
    active = true;
    bucketWidth = globalBucketSize; 
    total_time = bucketWidth*numBins;
    startTime = 0.0;
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


// constructor for histogram that doesn't start at time 0
Histogram::Histogram(timeStamp start, metricStyle type, dataCallBack d, 
		     foldCallBack f, void *c)
{
    smooth = false;
    lastBin = 0;
    metricType = type;
    intervalCount = 0;
    intervalLimit = MAX_INTERVALS;
    storageType = HistInterval;
    dataPtr.intervals = (Interval *) calloc(sizeof(Interval)*intervalLimit, 1);
    allHist += this;
    dataFunc = d;
    foldFunc = f;
    cData = c;
    active = true;
    startTime = start;

    // compute bucketwidth based on start time
    timeStamp minBucketWidth = 
	    ((lastGlobalBin*globalBucketSize)  - startTime) / numBins;    
    for(timeStamp i = baseBucketSize; i < minBucketWidth; i *= 2.0) ; 
    bucketWidth = i; 
    total_time = startTime + bucketWidth*numBins;
}


Histogram::Histogram(Bin *buckets, 
                     timeStamp start,
		     metricStyle type, 
		     dataCallBack d, 
		     foldCallBack f,
		     void *c)
{
    // First call default constructor.
    (void) Histogram(start, type, d, f, c);
    storageType = HistBucket;
    dataPtr.buckets = (Bin *) calloc(sizeof(Bin), numBins);
    memcpy(dataPtr.buckets, buckets, sizeof(Bin)*numBins);
}

Histogram::~Histogram(){

    // remove from allHist
    for(unsigned i=0; i < allHist.size(); i++){
        if(allHist[i] == this){
	    break;
        }
    }
    for(unsigned j = i; j < (allHist.size() - 1); j++){
        allHist[j] = allHist[j+1];
    }
    allHist.resize(allHist.size() - 1);

    // free bin space
    if (storageType == HistInterval) {
        free(dataPtr.intervals);
    }
    else {
	free(dataPtr.buckets);
    }
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
			    bool smooth)
{
    while ((end >= total_time) || (start >= total_time)) {
	// colapse histogram.
	foldAllHist();
    }

    lastBin = (int) ((end - startTime) / bucketWidth);

#ifdef n_def
    // update global info. if this histogram started at time 0
    if(startTime == 0.0){
        lastGlobalBin = lastBin;
        for (unsigned i=0; i < allHist.size(); i++) {
	    if((allHist[i])->startTime == 0.0){
	        if (((allHist[i])->lastBin < lastGlobalBin)) {
	            lastGlobalBin = (allHist[i])->lastBin;
	    }}
	}
    }
#endif

    // TODO: this should be replaced with the above code when
    // the performance consultant is changed to correctly
    // delete histograms that it is no longer collection 
    // data values for
    // change this so that lastGlobalbin is max of all lastBins
    if(startTime == 0.0){
	if(lastBin > lastGlobalBin)
            lastGlobalBin = lastBin;
        for (unsigned i=0; i < allHist.size(); i++) {
	    if(((allHist[i])->startTime == 0.0)
	       && ((allHist[i])->isActive())){
	        if (((allHist[i])->lastBin > lastGlobalBin)) {
	            lastGlobalBin = (allHist[i])->lastBin;
	    }}
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
    // update global info.
    if(startTime == 0.0){
	globalBucketSize *= 2.0;
        lastGlobalBin = numBins/2 - 1;
    }

    // fold all histograms with the same time base
    for(unsigned i = 0; i < allHist.size(); i++){
	if(((allHist[i])->startTime == startTime) 
	   && (allHist[i])->active){
	    (allHist[i])->bucketWidth *= 2.0;
	    if((allHist[i])->storageType == HistBucket){
                Bin *bins = (allHist[i])->dataPtr.buckets;
		for(unsigned j=0; j < numBins/2; j++){
                    bins[j] = (bins[j*2] + bins[j*2+1]) / 2.0;
		}
	        (allHist[i])->lastBin = j - 1; 
		memset(&bins[j], '\0', (numBins - j) * sizeof(Bin));
	    }
	    (allHist[i])->total_time = startTime + 
				       numBins*(allHist[i])->bucketWidth;
	    if((allHist[i])->foldFunc) 
		((allHist[i])->foldFunc)((allHist[i])->bucketWidth, 
					(allHist[i])->cData,
					(allHist[i])->startTime);
	}
    }
}

void Histogram::bucketValue(timeStamp start_clock, 
			   timeStamp end_clock, 
			   sampleValue value, 
			   bool smooth)
{
    register int i;

    // don't add values to an inactive histogram
    if(!active) return;

    timeStamp elapsed_clock = end_clock - start_clock;

    /* set starting and ending bins */
    int first_bin = (int) ((start_clock - startTime )/ bucketWidth);
    // ignore bad values
    if((first_bin < 0) || (first_bin > numBins)) return;
    //assert(first_bin >= 0);
    //assert(first_bin <= numBins);
    if (first_bin == numBins)
	first_bin = numBins-1;
    int last_bin = (int) ((end_clock - startTime) / bucketWidth);
    // ignore bad values
    if((last_bin < 0) || (last_bin > numBins)) return;
    // assert(last_bin >= 0);
    // assert(last_bin <= numBins);
    if (last_bin == numBins)
	last_bin = numBins-1;
    /* set starting times for first & last bins */
    timeStamp first_bin_start = bucketWidth * first_bin;
    timeStamp last_bin_start = bucketWidth  * last_bin;

    if (metricType == SampledFunction) {
	for (i=first_bin; i <= last_bin; i++) {
	    dataPtr.buckets[i] = value;
	}
    } else {
	// normalize by bucket size.
	value /= bucketWidth;
	if (last_bin == first_bin) {
	    dataPtr.buckets[first_bin] += value;
	    if (smooth && (dataPtr.buckets[first_bin] > 1.0)) {
		/* value > 100% */
		smoothBins(dataPtr.buckets, first_bin, bucketWidth);
	    }
	    return;
	}

	/* determine how much of the first & last bins were in this interval */
	timeStamp time_in_first_bin = 
			bucketWidth - ((start_clock - startTime)- first_bin_start);
	timeStamp time_in_last_bin = (end_clock- startTime) - last_bin_start;
	timeStamp time_in_other_bins = 
	    MAX(elapsed_clock - (time_in_first_bin + time_in_last_bin), 0);
        // ignore bad values
        if((time_in_first_bin < 0) || 
	   (time_in_last_bin < 0) || 
	   (time_in_other_bins < 0)) 
	    return;

	/* determine how much of value should be in each bin in the interval */
	sampleValue amt_first_bin = (time_in_first_bin / elapsed_clock) * value;
	sampleValue amt_last_bin  = (time_in_last_bin  / elapsed_clock) * value;
	sampleValue amt_other_bins = 
				(time_in_other_bins / elapsed_clock) * value;
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
	    if (dataPtr.buckets[i] > bucketWidth) 
		smoothBins(dataPtr.buckets, i, bucketWidth);
	}
    }

    // inform users about the data.
    // make sure they want to hear about it (dataFunc)
    //  && that we have a full bin (last_bin>first_bin)
    if (dataFunc && (last_bin-first_bin)) {
	(dataFunc)(&dataPtr.buckets[first_bin], 
		   startTime,
		   last_bin-first_bin, 
		   first_bin, 
		   cData);
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
	first_bin = (int) (start/bucketWidth);
	/* round up */
	last_bin = (int) (end/bucketWidth+0.5);
	if (last_bin >= numBins) last_bin = numBins-1;
	if (first_bin == last_bin) {
	    retVal = dataPtr.buckets[first_bin];
	} else {
	    /* (first_bin+1)*bucketWidth == time at end of first bucket */
	    pct_first_bin = (((first_bin+1)*bucketWidth)-start)/bucketWidth;
	    retVal += pct_first_bin * dataPtr.buckets[first_bin];
	    /* last_bin+*bucketWidth == time at start of last bucket */
	    pct_last_bin = (end - last_bin*bucketWidth)/bucketWidth;
	    retVal += pct_last_bin * dataPtr.buckets[last_bin];
	    for (i=first_bin+1; i <= last_bin-1; i++) {
		retVal += dataPtr.buckets[i];
	    }
	}
    }
    return(retVal * bucketWidth);
}

sampleValue Histogram::getValue()
{		
    return(getValue(0.0, numBins * bucketWidth));
}

int Histogram::getBuckets(sampleValue *buckets, int numberOfBuckets, int first)
{
    int i;
    int last;

    last = first + numberOfBuckets;
    if (lastBin < last) last = lastBin;

    // make sure its in bin form.
    convertToBins();

    for (i=first; i < last; i++) {
	buckets[i-first] = dataPtr.buckets[i];
    }
    return(last-first);
}
