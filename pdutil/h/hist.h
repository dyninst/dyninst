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

// $Id: hist.h,v 1.25 2001/09/04 17:02:26 schendel Exp $

#ifndef HIST
#define HIST

#include "common/h/Vector.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"

#define BASEBUCKETWIDTH_SECS  0.2   /* .2 seconds  */

typedef enum { HistNewValue, HistNewTimeBase } callType;
typedef pdSample Bin;

typedef void (*foldCallBack)(const timeLength*, void* userData);

typedef void (*dataCallBack)(pdSample *buckets,
			     relTimeStamp start_time,
			     int numberOfBuckets,
			     int first,
			     void *userData);

class Histogram;

typedef enum { histActive, histInactive } histStatus;
typedef enum { histSum, histAverage } histCompact;
typedef enum { histSplit, histSet } histAddMode;

class Histogram {
	void newDataFunc(callType type, relTimeStamp time, void* userData);
    public:
	~Histogram();
	Histogram(relTimeStamp, dataCallBack, foldCallBack, void*);
	Histogram(Bin *buckets,  
		  relTimeStamp start,  // binWidth is computed by this value 
		  dataCallBack, 
		  foldCallBack, 
		  void*);
	pdSample getValue();
	pdSample getValue(relTimeStamp start, relTimeStamp end);
	pdSample getBinValue(int i){
	    if(i <= lastBin){
	       return buckets[i];
	    }
	    else
	       return pdSample::NaN();
	}
	int getBuckets(pdSample *saveBuckets, int numberOfBuckets, int first);
	int getCurrBin(){return(lastBin);}
	relTimeStamp getStartTime(){ return(startTime);}
	timeLength getBucketWidth(){ return(bucketWidth);}
	void setActive(){ active = true;}
	void clearActive(){ active = false;}
	void setFoldOnInactive(){fold_on_inactive = true;}
	void clearFoldOnInactive(){fold_on_inactive = false;}
	bool isActive(){ return active;}
	bool foldOnInactive(){ return fold_on_inactive;}
	void addInterval(relTimeStamp start, relTimeStamp end, pdSample value);
	void addPoint(relTimeStamp start, pdSample value) {
	    addInterval(start, start, value);
	}
	pdSample getCurrentActualValue();
	void setInitialActualValue(pdSample v) {
	  initActualVal = v;
	}

	static relTimeStamp currentTime() { 
		return relTimeStamp(lastGlobalBin*globalBucketSize); 
	}
	static timeLength getGlobalBucketWidth(){ return(globalBucketSize); }
	static timeLength getMinBucketWidth();
	static int getNumBins(){ return(numBins);}
	void flushUnsentBuckets();
    private:
	void foldAllHist();
	void bucketValue(relTimeStamp start, relTimeStamp end, pdSample value);

	static int numBins;		/* max bins to use */

	// used to compute the current Time
	static timeLength globalBucketSize; // largest curr. bucket width 
	static int lastGlobalBin;  // for all hists. with global bins 

	// static Histogram *allHist;	/* linked list of all histograms */
	// Histogram *next;		/* linked list of all histograms */
        static vector<Histogram *> allHist;  // list of all histograms

        // these are added to support histograms with different bucket widths
	// with different startTimes  (an individual histogram does not need
	// to start at time 0, and thus will contain only values added after
	// its start time
	int lastBin;			/* current (for this hist) last bin */
	int curBinFilling;
	int lastBinSent;
	pdSample initActualVal;         // used by getCurrentActualValue()
	timeLength bucketWidth;		// bucket width of this histogram 
	relTimeStamp startTime;		// not all histograms start at time 0
	relTimeStamp endTime;	        /* numBins * baseBucketSize */
	bool active;			// if clear, don't add values 
	bool fold_on_inactive;		// if set, fold inactive histograms 

	Bin *buckets;
	dataCallBack dataFunc;
	foldCallBack foldFunc;
	void *cData;
};

#endif

