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

// hist.C - routines to manage histograms.
// $Id: hist.C,v 1.39 2000/10/17 17:42:48 schendel Exp $

#include "common/h/headers.h"
#include "pdutil/h/hist.h"
#include "common/h/debugOstream.h"
#include <math.h>

// We are currently using the Histogram code out of pdutilOld.  The histogram
// code here relies on the old timeStamp and sampleValue types.  It will be
// reactivated after it has been converted (along with the front end) to use
// the new time and sample value types.
#ifdef notdef

/* number of intervals at which we switch to regular histograms */
#define MAX_INTERVALS	15

static void smoothBins(Bin *bins, int i, timeStamp bucketSize);

int Histogram::numBins = 1000;
int Histogram::lastGlobalBin = 0;
timeStamp Histogram::baseBucketSize = BASEBUCKETWIDTH;
timeStamp Histogram::globalBucketSize = BASEBUCKETWIDTH;
vector<Histogram *> Histogram::allHist;

#ifdef HIST_DEBUG
debug_ostream hist_cerr(cerr, true);
#else
debug_ostream hist_cerr(cerr, false);
#endif

Histogram::Histogram(metricStyle type, dataCallBack d, foldCallBack f, void *c,
		     bool globalFlag)
:globalData(globalFlag)
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
    fold_on_inactive = false;
    bucketWidth = globalBucketSize; 
    total_time = bucketWidth*numBins;
    startTime = 0.0;
    curBinFilling = -1;
    lastBinSent = -1;
}

/*
 * Given an array of buckets - turn them into a histogram.
 *
 */
Histogram::Histogram(Bin *buckets, 
		     metricStyle type, 
		     dataCallBack d, 
		     foldCallBack f,
		     void *c,
		     bool globalFlag)
{
    // First call default constructor.
    (void) Histogram(type, d, f, c, globalFlag);

    storageType = HistBucket;
    dataPtr.buckets = (Bin *) calloc(sizeof(Bin), numBins);
    memcpy(dataPtr.buckets, buckets, sizeof(Bin)*numBins);
}


// constructor for histogram that doesn't start at time 0
Histogram::Histogram(timeStamp start, metricStyle type, dataCallBack d, 
		     foldCallBack f, void *c, bool globalFlag)
:globalData(globalFlag)
{
    smooth = false;
    lastBin = 0;
    metricType = type;
    intervalCount = 0;
    intervalLimit = MAX_INTERVALS;
    storageType = HistInterval;
    dataPtr.intervals = (Interval *) calloc(sizeof(Interval)*intervalLimit, 1);
    dataFunc = d;
    foldFunc = f;
    cData = c;
    active = true;
    fold_on_inactive = false;
    startTime = start;
    curBinFilling = -1;
    lastBinSent = -1;
    // try to find an active histogram with the same start time 
    // and use its bucket width  for "bucketWidth", otherwise, compute
    // a value for "bucketWidth" based on startTime and global time
    bool found = false;
    for(unsigned i = 0; i < allHist.size(); i++){
	if(((allHist[i])->startTime == startTime)&&(allHist[i])->active){
            found = true;
	    bucketWidth = (allHist[i])->bucketWidth;
	    break;
    } }
    if(!found){
        // compute bucketwidth based on start time
        timeStamp minBucketWidth = 
	        ((lastGlobalBin*globalBucketSize)  - startTime) / numBins;    
        timeStamp i2 = baseBucketSize;
        for(; i2 < minBucketWidth; i2 *= 2.0) ; 
        bucketWidth = i2; 
    }

    allHist += this;
    total_time = startTime + bucketWidth*numBins;
}


Histogram::Histogram(Bin *buckets, 
                     timeStamp start,
		     metricStyle type, 
		     dataCallBack d, 
		     foldCallBack f,
		     void *c,
		     bool globalFlag)
{
    // First call default constructor.
    (void) Histogram(start, type, d, f, c, globalFlag);
    storageType = HistBucket;
    dataPtr.buckets = (Bin *) calloc(sizeof(Bin), numBins);
    memcpy(dataPtr.buckets, buckets, sizeof(Bin)*numBins);
}

Histogram::~Histogram(){

    // remove from allHist
    unsigned i=0;
    for(; i < allHist.size(); i++){
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
	delete [] dataPtr.buckets;
    }
}

#if !defined(MAX)
#define MAX(a, b)	((a) > (b) ? (a):(b))
#endif
#if !defined(MIN)
#define MIN(a, b)	((a) < (b) ? (a):(b))
#endif

/*
 * addInterval - add a value to a histogram from start to end.
 *
 * start, and end are relative to the global start time (i.e. 0.0 is the
 *   left side of the first hist bin).
 */
void Histogram::addInterval(timeStamp start, 
			    timeStamp end, 
			    sampleValue value, 
			    bool doSmooth)
{
    while ((end >= total_time) || (start >= total_time)) {
	// collapse histogram.
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
    if (startTime == 0.0) {
	if(lastBin > lastGlobalBin)
            lastGlobalBin = lastBin;

	const unsigned all_hist_size = allHist.size();
        for (unsigned i=0; i < all_hist_size; i++) {
	    Histogram *theHist = allHist[i]; // a time saver; call operator[] just once

	    if (theHist->startTime == 0.0 && theHist->isActive())
	        if (theHist->lastBin > lastGlobalBin)
	            lastGlobalBin = theHist->lastBin;
	}
    }

    if (storageType == HistInterval) {
	/* convert to buckets */
	convertToBins();
    }
    bucketValue(start, end, value, doSmooth);
}

void Histogram::convertToBins()
{
    int i;
    Interval *intervals;
    Interval *currentInterval;

    if (storageType == HistBucket) return;

    intervals = dataPtr.intervals;
    dataPtr.intervals = 0;
    dataPtr.buckets = new Bin[numBins];
    for(i = 0; i < numBins; i++){
        dataPtr.buckets[i] = PARADYN_NaN; 
    }

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
    hist_cerr << "Histogram::foldAllHist()\n";
    // update global info.
    if(startTime == 0.0){
	globalBucketSize *= 2.0;
        lastGlobalBin = numBins/2 - 1;
    }
    timeStamp newBucketWidth = bucketWidth*2.0;

    // fold all histograms with the same time base
    for(unsigned i = 0; i < allHist.size(); i++){
	if(((allHist[i])->startTime == startTime)  // has same base time and 
	   && ((allHist[i])->active                // either, histogram active
	   || (allHist[i])->fold_on_inactive)){    // or fold on inactive set 

          // don't fold this histogram if it has already folded
	  // This can happen to histograms that are created right
	  // after a fold, so that the initial bucket width may 
	  // not be correct and then the first data values will cause
	  // another fold...in this case we only want to fold the
	  // histograms that were not folded in the first round.  
	  if((allHist[i])->bucketWidth < newBucketWidth) {
	      (allHist[i])->bucketWidth *= 2.0;
	      if((allHist[i])->storageType == HistBucket){
		  int j;
                  Bin *bins = (allHist[i])->dataPtr.buckets;
		  int last_bin = -1;
		  for(j=0; j < numBins/2; j++){
		      if(!isnan(bins[j*2+1])){   // both are not NaN
                          bins[j] = (bins[j*2] + bins[j*2+1]) / 2.0F;
                      }
		      else if(!isnan(bins[j*2])){  // one is NaN
		          bins[j] = (bins[j*2])/2.0F;	
			  if(last_bin == -1) last_bin = j;
		      }
		      else {  // both are NaN
			  bins[j] = PARADYN_NaN;
			  if(last_bin == -1) last_bin = j;
		      }
		  }
		  if(last_bin == -1) last_bin = j-1;
	          (allHist[i])->lastBin = last_bin; 
		  for(int k=numBins/2; k<numBins; k++){
		      bins[k] = PARADYN_NaN;
		  }
	      }
	      (allHist[i])->total_time = startTime + 
				       numBins*(allHist[i])->bucketWidth;
	      if((allHist[i])->foldFunc) 
	  	  ((allHist[i])->foldFunc)((allHist[i])->bucketWidth, 
					(allHist[i])->cData,
					(allHist[i])->globalData);
	  }
	}
    }
}

void Histogram::flushUnsentBuckets() {
  hist_cerr << "Histogram::FlushUnsentBuckets\n";
  if (dataFunc) {
    if(curBinFilling > lastBinSent && curBinFilling!=-1) {
      int firstBinToSend = lastBinSent + 1;
      hist_cerr << "sending buckets: " << firstBinToSend << ", for count of: " 
		<< curBinFilling - lastBinSent << "\n";
      (dataFunc)(&dataPtr.buckets[firstBinToSend], startTime,
		 curBinFilling - lastBinSent, firstBinToSend,cData,globalData);
    }
  }
}

void Histogram::bucketValue(timeStamp start_clock, 
			   timeStamp end_clock, 
			   sampleValue value, 
			   bool doSmooth)
{
    register int i;

    // don't add values to an inactive histogram
    if(!active) return;

    timeStamp elapsed_clock = end_clock - start_clock;

    hist_cerr << "bucketValue-  start: " << start_clock << "  end: " 
	      << end_clock << "  val: " << value << "  doSmooth: " 
	      << doSmooth << "  bktWidth: " << bucketWidth << "\n";

    /* set starting and ending bins */
    int first_bin = (int) ((start_clock - startTime )/ bucketWidth);

    // ignore bad values
    if((first_bin < 0) || (first_bin > numBins)) return;
    if (first_bin == numBins)
	first_bin = numBins-1;
    int last_bin = (int) ((end_clock - startTime) / bucketWidth);
    curBinFilling = last_bin;

    // ignore bad values
    if((last_bin < 0) || (last_bin > numBins)) return;
    if (last_bin == numBins)
	last_bin = numBins-1;

    /* set starting times for first & last bins */
    timeStamp first_bin_start = bucketWidth * first_bin;
    timeStamp last_bin_start = bucketWidth  * last_bin;

    hist_cerr << "  firstBin: " << first_bin << "  lastBin: " << last_bin
	      << "  firstBinSt: " << first_bin_start << "  lastBinSt: "
	      << last_bin_start << "\n";

    if (metricType == SampledFunction) {
      hist_cerr << "SampledFunction-  \n";
	for (i=first_bin; i <= last_bin; i++) {
	  hist_cerr << " bucket[" << i << "] = " << value << "\n";
	    dataPtr.buckets[i] = value;
	}
    } else {
	// normalize by bucket size.
	value /= (sampleValue)bucketWidth;
	hist_cerr << "EventCounter-  normalized val: " << value << "\n";

	sampleValue first_bin_interval_left  = start_clock - startTime;
	sampleValue first_bin_interval_right = 
	  MIN(first_bin_start + bucketWidth, end_clock - startTime);
	timeStamp time_in_first_bin = first_bin_interval_right - 
	                              first_bin_interval_left;

	timeStamp time_in_last_bin = 0;
	if(first_bin != last_bin)
	  time_in_last_bin = (end_clock- startTime) - last_bin_start;
	else {
	  // the interval is contained solely in the first bucket
	  time_in_last_bin = 0;  
	}

	timeStamp time_in_other_bins = 
	    MAX(elapsed_clock - (time_in_first_bin + time_in_last_bin), 0);
        // ignore bad values
        if((time_in_first_bin < 0) || 
	   (time_in_last_bin < 0) || 
	   (time_in_other_bins < 0))
	    return;

	hist_cerr << "H2 elapsed_clock: " << elapsed_clock 
		  << "  time_in_first_bin: " << time_in_first_bin 
		  << "  time_in_last_bin: " << time_in_last_bin 
		  << "  time_in_other_bins: " << time_in_other_bins << "\n";
	  
	/* determine how much of value should be in each bin in the interval */
	sampleValue amt_first_bin = 
                (sampleValue)(time_in_first_bin / elapsed_clock) * value;
	sampleValue amt_last_bin  = 
                (sampleValue)(time_in_last_bin  / elapsed_clock) * value;
	sampleValue amt_other_bins = 
                (sampleValue)(time_in_other_bins / elapsed_clock) * value;
	if (last_bin > first_bin+1) 
	    amt_other_bins /= (last_bin - first_bin) - 1.0F;

        // if bins contain NaN values set them to 0 before adding new value
	if(isnan(dataPtr.buckets[first_bin])) 
	    dataPtr.buckets[first_bin] = 0.0;
	if(isnan(dataPtr.buckets[last_bin])) 
	    dataPtr.buckets[last_bin] = 0.0;

	hist_cerr << " bucket[" << first_bin << "] = " 
		  << dataPtr.buckets[first_bin] << "   bucket[" << last_bin 
		  << "] = " << dataPtr.buckets[last_bin] << "\n";

	/* add the appropriate amount of time to each bin */
	dataPtr.buckets[first_bin] += amt_first_bin;
	hist_cerr << " bucket[" << first_bin << "] += " << amt_first_bin
		  << " = " << dataPtr.buckets[first_bin] << "\n";
	dataPtr.buckets[last_bin]  += amt_last_bin;

	for (i=first_bin+1; i < last_bin; i++){
	    if(isnan(dataPtr.buckets[i])) 
	        dataPtr.buckets[i] = 0.0;
	    dataPtr.buckets[i]  += amt_other_bins;
	    hist_cerr << " bucket[" << i << "] += " << amt_other_bins
		      << " = " << dataPtr.buckets[i] << "\n";
        }
	hist_cerr << " bucket[" << last_bin << "] += " << amt_last_bin
		  << " = " << dataPtr.buckets[last_bin] << "\n";
    }

    /* limit absurd time values - anything over 100% is pushed back into 
       previous buckets */
    if (doSmooth) {
	for (i = first_bin; i <= last_bin; i++) {
	    if (dataPtr.buckets[i] > bucketWidth) 
		smoothBins(dataPtr.buckets, i, bucketWidth);
	}
    }

    // inform users about the data.
    // make sure they want to hear about it (dataFunc)
    //  && that we have a full bin (last_bin>first_bin)

    if ((dataFunc)  && (last_bin > first_bin)) {
      hist_cerr << "dataFunc()- firstBin: " << first_bin << ", val(1): " 
		<< dataPtr.buckets[first_bin] << "\n"; 
      lastBinSent = first_bin;
	(dataFunc)(&dataPtr.buckets[first_bin], 
		   startTime,
		   last_bin-first_bin, 
		   first_bin, 
		   cData,
		   globalData);
    }
}


static void smoothBins(Bin *bins, int i, timeStamp bucketSize)
{
    int j;
    sampleValue extra_time;
    hist_cerr << "smoothBins\n";
    extra_time = (sampleValue)(bins[i] - bucketSize);
    bins[i] = (sampleValue)bucketSize;
    j = i - 1;
    while ((j >= 0) && (extra_time > 0.0)) {
	bins[j] += extra_time;
	extra_time = 0.0;
	if (bins[j] > bucketSize) {
	    extra_time = (sampleValue)(bins[j] - bucketSize);
	    bins[j] = (sampleValue)bucketSize;
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
	time = (sampleValue)(value * (endLimit - startInterval) / inv_length);
    } else if ((endInterval <= endLimit) && (endInterval >= startLimit)) { 
	/* overlaps the end */				
	time = (sampleValue)(value * (endInterval - startLimit) / inv_length);
    } else if ((startInterval < startLimit) && (endInterval > endLimit)) {
	/* detail contains interval */
	time = (sampleValue)(value * (endLimit - startLimit) / inv_length);
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
		retVal += (sampleValue)(cp->value * (cp->end - cp->start));
	    }
	} else {
	    // total over the interval.
	    for (i=0; i < intervalCount; i++) {
		cp = &(dataPtr.intervals[i]);
		retVal += (sampleValue)splitTime(cp->start, cp->end, start, end, cp->value);
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
	    retVal += (sampleValue)(pct_first_bin * dataPtr.buckets[first_bin]);
	    /* last_bin+*bucketWidth == time at start of last bucket */
	    pct_last_bin = (end - last_bin*bucketWidth)/bucketWidth;
	    retVal += (sampleValue)(pct_last_bin * dataPtr.buckets[last_bin]);
	    for (i=first_bin+1; i <= last_bin-1; i++) {
		retVal += dataPtr.buckets[i];
	    }
	}
    }
    return((sampleValue)(retVal * bucketWidth));
}

sampleValue Histogram::getValue()
{		
    return(getValue(0.0, numBins * bucketWidth));
}

int Histogram::getBuckets(sampleValue *buckets, int numberOfBuckets, int first)
{
    int i;
    int last;
    hist_cerr << "getBuckets - num: " << numberOfBuckets << "  first: " 
	      << first << "\n";
    last = first + numberOfBuckets - 1;
    if (lastBin < last) last = lastBin;  // lastBin is an index

    // make sure its in bin form.
    convertToBins();

    assert(first >= 0);
    assert(last <= lastBin); 
    for (i=first; i <= last; i++) {
	float temp = dataPtr.buckets[i];
	hist_cerr << "   " << i << ": " << temp << "\n";
	buckets[i-first] = temp;
    }
    return(last-first+1);
}

#endif  // notdef
