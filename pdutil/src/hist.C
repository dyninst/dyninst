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
// $Id: hist.C,v 1.44 2003/03/04 19:16:19 willb Exp $

#include "common/h/headers.h"
#include "pdutil/h/hist.h"
#include "pdutil/h/pdDebugOstream.h"
#include <math.h>


/* number of intervals at which we switch to regular histograms */
#define MAX_INTERVALS	15

int Histogram::numBins = 1000;
int Histogram::lastGlobalBin = 0;
timeLength Histogram::globalBucketSize = timeLength(BASEBUCKETWIDTH_SECS,
						    timeUnit::sec());
pdvector<Histogram *> Histogram::allHist;

unsigned enable_pd_histogram_debug = 0;

#if ENABLE_DEBUG_CERR == 1
#define hist_cerr if (enable_pd_histogram_debug) cerr
#else
#define hist_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

// constructor for histogram that doesn't start at time 0
Histogram::Histogram(relTimeStamp start, dataCallBack d, 
		     foldCallBack f, void *c)
  : startTime(start)
{
    lastBin = -1;
    buckets = new Bin[numBins];
    for(int i = 0; i < numBins; i++){
        buckets[i] = pdSample::NaN(); 
    }
    dataFunc = d;
    foldFunc = f;
    cData = c;
    active = true;
    fold_on_inactive = false;
    curBinFilling = -1;
    lastBinSent = -1;
    // try to find an active histogram with the same start time 
    // and use its bucket width  for "bucketWidth", otherwise, compute
    // a value for "bucketWidth" based on startTime and global time
    bool found = false;
    for(unsigned j = 0; j < allHist.size(); j++){
	if(((allHist[j])->startTime == startTime)&&(allHist[j])->active){
            found = true;
	    bucketWidth = (allHist[j])->bucketWidth;
	    break;
    } }
    if(!found){
      // compute bucketwidth based on start time
      timeLength spanLen = globalBucketSize * lastGlobalBin - startTime;
      timeLength minBucketWidth = spanLen / numBins;
      timeLength i2 = getMinBucketWidth();
      for(; i2 < minBucketWidth; i2 *= 2.0) ; 
      bucketWidth = i2; 
    }

    allHist += this;
    endTime = startTime + bucketWidth*numBins;
}


Histogram::Histogram(Bin *buckets, 
                     relTimeStamp start,
		     dataCallBack d, 
		     foldCallBack f,
		     void *c)
{
    // First call default constructor.
    (void) Histogram(start, d, f, c);
    memcpy(buckets, buckets, sizeof(Bin)*numBins);
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

    delete [] buckets;
}

/*
 * addInterval - add a value to a histogram from start to end.
 *
 * start, and end are relative to the global start time (i.e. 0.0 is the
 *   left side of the first hist bin).
 */
void Histogram::addInterval(relTimeStamp start, 
			    relTimeStamp end, 
			    pdSample value)
{
    while ((end >= endTime) || (start >= endTime)) {
	// collapse histogram.
	foldAllHist();
    }

    lastBin = (int) ((end - startTime) / bucketWidth);

#ifdef n_def
    // update global info. if this histogram started at time 0
    if(startTime == relTimeStamp::Zero()){
        lastGlobalBin = lastBin;
        for (unsigned i=0; i < allHist.size(); i++) {
	    if((allHist[i])->startTime == relTimeStamp::Zero()){
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
    if (startTime == relTimeStamp::Zero()) {
	if(lastBin > lastGlobalBin)
            lastGlobalBin = lastBin;

	const unsigned all_hist_size = allHist.size();
        for (unsigned i=0; i < all_hist_size; i++) {
	    Histogram *theHist = allHist[i]; // a time saver; call operator[] just once

	  if (theHist->startTime==relTimeStamp::Zero() && theHist->isActive())
	        if (theHist->lastBin > lastGlobalBin)
	            lastGlobalBin = theHist->lastBin;
	}
    }

    bucketValue(start, end, value);
}

void Histogram::foldAllHist()
{
    hist_cerr << "Histogram::foldAllHist()\n";
    // update global info.
    if(startTime == relTimeStamp::Zero()){
	globalBucketSize *= 2.0;
        lastGlobalBin = numBins/2 - 1;
    }
    timeLength newBucketWidth = bucketWidth * 2.0;

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
	      int j;
	      Bin *bins = (allHist[i])->buckets;
	      int last_bin = 0;

	      for(j=0; j < numBins/2; j++) {
		pdSample firstBkt  = bins[j*2];
		pdSample secondBkt = bins[j*2+1];
		// both bins have data
		if(!firstBkt.isNaN() && !secondBkt.isNaN()) {
		  bins[j]  = (firstBkt + secondBkt) / 2.0;
		  last_bin = j;
		} // first bin has data, second doesn't
		else if(!firstBkt.isNaN() && secondBkt.isNaN()) {
		  bins[j]  = firstBkt / 2.0;	
		  last_bin = j;
		} // first bin doesn't have data, second has data
		else if(firstBkt.isNaN() && !secondBkt.isNaN()) {
		  bins[j]  = secondBkt / 2.0;	
		  last_bin = j;
		} // neither bin has data
		else if(firstBkt.isNaN() && secondBkt.isNaN()) {
		  bins[j] = pdSample::NaN();
		}
	      }
	      (allHist[i])->lastBin = last_bin; 
	      for(int k = numBins/2; k < numBins; k++){
		bins[k] = pdSample::NaN();
	      }
	      (allHist[i])->endTime = startTime + 
				       numBins*(allHist[i])->bucketWidth;
	      if((allHist[i])->foldFunc) 
		((allHist[i])->foldFunc)(&(allHist[i])->bucketWidth, 
					 (allHist[i])->cData);
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
      (dataFunc)(&buckets[firstBinToSend], startTime,
		 curBinFilling - lastBinSent, firstBinToSend, cData);
      lastBinSent = curBinFilling - 1;
    }
  }
}

void Histogram::bucketValue(relTimeStamp start_clock, relTimeStamp end_clock, 
			    pdSample value)
{
    register int i;

    // don't add values to an inactive histogram
    if(!active) return;

    timeLength elapsed_clock = end_clock - start_clock;

    hist_cerr << "bucketValue-  start: " << start_clock << "  end: " 
	      << end_clock << "  val: " << value << "  bktWidth: " 
	      << bucketWidth << "\n";

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
    relTimeStamp first_bin_start = relTimeStamp(bucketWidth * first_bin);
    relTimeStamp last_bin_start = relTimeStamp(bucketWidth  * last_bin);

    hist_cerr << "  firstBin: " << first_bin << "  lastBin: " << last_bin
	      << "  firstBinSt: " << first_bin_start << "  lastBinSt: "
	      << last_bin_start << "\n";

    relTimeStamp first_bin_interval_left  = start_clock - startTime;
    relTimeStamp userEndT = relTimeStamp(end_clock - startTime);
    relTimeStamp first_bin_interval_right = 
      earlier(first_bin_start + bucketWidth, userEndT);
    timeLength time_in_first_bin = first_bin_interval_right - 
      first_bin_interval_left;
    
    timeLength time_in_last_bin = timeLength::Zero();
    if(first_bin != last_bin)
      time_in_last_bin = userEndT - last_bin_start;
    else {
      // the interval is contained solely in the first bucket
      time_in_last_bin = timeLength::Zero();
    }

    timeLength time_in_other_bins = max(elapsed_clock - 
		   (time_in_first_bin + time_in_last_bin), timeLength::Zero());
    // ignore bad values
    if((time_in_first_bin < timeLength::Zero()) || 
       (time_in_last_bin < timeLength::Zero()) || 
       (time_in_other_bins < timeLength::Zero()))
      return;

    hist_cerr << "H2 elapsed_clock: " << elapsed_clock 
	      << "  time_in_first_bin: " << time_in_first_bin 
	      << "  time_in_last_bin: " << time_in_last_bin 
	      << "  time_in_other_bins: " << time_in_other_bins << "\n";
    
    /* determine how much of value should be in each bin in the interval */
    pdSample amt_first_bin = (time_in_first_bin / elapsed_clock) * value;
    pdSample amt_other_bins = (time_in_other_bins / elapsed_clock) * value;
    pdSample amt_last_bin  = value - amt_first_bin - amt_other_bins;

    int num_middle_bins = (last_bin - first_bin) - 1;
    double num_middle_binsD = static_cast<double>(num_middle_bins);
    if (last_bin > first_bin+1) 
      amt_other_bins = amt_other_bins / num_middle_binsD;
    
    // if bins contain NaN values set them to 0 before adding new value
    if(buckets[first_bin].isNaN()) 
      buckets[first_bin] = pdSample::Zero();
    if(buckets[last_bin].isNaN()) 
      buckets[last_bin] = pdSample::Zero();
    
    hist_cerr << " bucket[" << first_bin << "] = " 
	      << buckets[first_bin] << "   bucket[" << last_bin 
	      << "] = " << buckets[last_bin] << "\n";
    
    /* add the appropriate amount of time to each bin */
    buckets[first_bin] += amt_first_bin;
    hist_cerr << " bucket[" << first_bin << "] += " << amt_first_bin
	      << " = " << buckets[first_bin] << "\n";
    buckets[last_bin]  += amt_last_bin;
    
    for (i=first_bin+1; i < last_bin; i++) {
      if(buckets[i].isNaN()) 
	buckets[i] = pdSample::Zero();
      buckets[i]  += amt_other_bins;
      hist_cerr << " bucket[" << i << "] += " << amt_other_bins
		<< " = " << buckets[i] << "\n";
    }
    hist_cerr << " bucket[" << last_bin << "] += " << amt_last_bin
	      << " = " << buckets[last_bin] << "\n";

    // inform users about the data.
    // make sure they want to hear about it (dataFunc)
    //  && that we have a full bin (last_bin>first_bin)

    if ((dataFunc)  && (last_bin > first_bin)) {
      hist_cerr << "dataFunc()- firstBin: " << first_bin << ", val(1): " 
		<< buckets[first_bin] << "\n";

      // We want to send the actual value of the last bin that was sent along
      // with the change in sample values in order that the performance
      // streams can easily calculate the actual value for each bucket to
      // pass on (along with the given change in sample value)

      (dataFunc)(&buckets[first_bin], startTime, 
		 last_bin-first_bin, first_bin, cData);
      lastBinSent = last_bin - 1;
    }
}

// can this replace getValue()
pdSample Histogram::getCurrentActualValue() {
  pdSample curTot = pdSample::Zero();
  assert(! initActualVal.isNaN());
  for(int i=0; i<lastBin; i++) {
    if(! buckets[i].isNaN())  curTot += buckets[i];
  }
  return initActualVal + curTot;
}

/*
 * Get the component of the histogram from start to end.
 *
 */
pdSample Histogram::getValue(relTimeStamp begin, relTimeStamp end)
{		
    int i;
    pdSample retVal = pdSample::Zero();

    int first_bin, last_bin;
    first_bin=static_cast<int>((begin - relTimeStamp::Zero()) / bucketWidth);
    /* round up */
    last_bin= static_cast<int>(((end - relTimeStamp::Zero()) / bucketWidth) 
			       + 0.5);
    if (last_bin >= numBins) last_bin = numBins-1;
    for (i=first_bin; i <= last_bin; i++) {
      if(!buckets[i].isNaN())  retVal += buckets[i];
    }
    return retVal;
}

pdSample Histogram::getValue()
{		
    return(getValue(startTime, endTime));
}

timeLength Histogram::getMinBucketWidth() { 
  // once this is setup this way to avoid initialization dependency problems
  // between non-local static objects
  // this will be initialized only since it's static
  static timeLength baseBucketSize = timeLength(BASEBUCKETWIDTH_SECS,
						timeUnit::sec());
  return baseBucketSize;
}

int Histogram::getBuckets(pdSample *saveBuckets, int numberOfBuckets,int first)
{
    int i;
    int last;

    hist_cerr << "getBuckets - num: " << numberOfBuckets << "  first: " 
	      << first << "\n";
    last = first + numberOfBuckets - 1;

    if (lastBin < last) last = lastBin;  // lastBin is an index

    assert(first >= 0);
    assert(last <= lastBin); 
    for (i=first; i <= last; i++) {
	pdSample temp = buckets[i];
	hist_cerr << "   " << i << ": " << temp << "\n";
	saveBuckets[i-first] = temp;
    }
    int ret;
    if(last == -1)
      ret = 0;
    else
      ret = last - first + 1;
    return ret;
}


