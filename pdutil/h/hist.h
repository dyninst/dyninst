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

#ifndef HIST
#define HIST

#include "common/h/Vector.h"
#include "pdutil/h/sys.h"
#include "pdutil/h/makenan.h"

typedef enum { EventCounter, SampledFunction } metricStyle;

// We are currently using the Histogram code out of pdutilOld.  The histogram
// code here relies on the old timeStamp and sampleValue types.  It will be
// reactivated after it has been converted (along with the front end) to use
// the new time and sample value types.
#ifdef notdef

typedef enum { HistInterval, HistBucket } histType;
typedef enum { HistNewValue, HistNewTimeBase } callType;
typedef sampleValue Bin;

typedef void (*foldCallBack)(timeStamp, void* userData, bool globalFlag);

typedef void (*dataCallBack)(sampleValue *buckets,
			     timeStamp start_time,
			     int numberOfBuckets,
			     int first,
			     void *userData,
			     bool globalFlag);

class Histogram;

typedef enum { histActive, histInactive } histStatus;
typedef enum { histSum, histAverage } histCompact;
typedef enum { histSplit, histSet } histAddMode;

class Histogram {
	void newDataFunc(callType type, timeStamp time, void* userData);
    public:
	~Histogram();
	// constructors for base start time and bucket width
	Histogram(metricStyle, dataCallBack, foldCallBack, void* , bool);
	Histogram(Bin *buckets, metricStyle, dataCallBack, foldCallBack, void*, bool);
	// constructors for specified start time
	Histogram(timeStamp, metricStyle, dataCallBack, foldCallBack, void*, bool );
	Histogram(Bin *buckets,  
		  timeStamp start,  // binWidth is computed by this value 
		  metricStyle, 
		  dataCallBack, 
		  foldCallBack, 
		  void*,
		  bool);
	sampleValue getValue();
	sampleValue getValue(timeStamp start, timeStamp end);
	sampleValue getBinValue(int i){
	    if(i <= lastBin){
	       return(dataPtr.buckets[i]);
	    }
	    else
	       return(PARADYN_NaN);
	}
	int getBuckets(sampleValue *buckets, int numberOfBuckets, int first);
	int getCurrBin(){return(lastBin);}
	timeStamp getStartTime(){return(startTime);}
	timeStamp getBucketWidth(){ return(bucketWidth);}
	void setActive(){ active = true;}
	void clearActive(){ active = false;}
	void setFoldOnInactive(){fold_on_inactive = true;}
	void clearFoldOnInactive(){fold_on_inactive = false;}
	bool isActive(){ return active;}
	bool foldOnInactive(){ return fold_on_inactive;}
	void addInterval(timeStamp start, timeStamp end, 
	    sampleValue value, bool smooth);
	void addPoint(timeStamp start, sampleValue value) {
	    addInterval(start, start, value, false);
	}
	static timeStamp currentTime() { 
		return((timeStamp)(lastGlobalBin*globalBucketSize)); 
	}
	static timeStamp getGlobalBucketWidth(){ return(globalBucketSize); }
	static timeStamp getMinBucketWidth(){ return(baseBucketSize);}
	static int getNumBins(){ return(numBins);}
	void flushUnsentBuckets();
    private:
	void foldAllHist();
	void convertToBins();
	void bucketValue(timeStamp start, timeStamp end, 
		sampleValue value, bool smooth);

	static int numBins;		/* max bins to use */

	// used to compute the current Time
	static timeStamp baseBucketSize;  // min. bucket size for all hists.
	static timeStamp globalBucketSize; // largest curr. bucket width 
	static int lastGlobalBin;  // for all hists. with global bins 

	timeStamp total_time;	/* numBins * baseBucketSize */

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
	timeStamp bucketWidth;		// bucket width of this histogram 
	timeStamp startTime;		// not all histograms start at time 0
	bool active;			// if clear, don't add values 
	bool fold_on_inactive;		// if set, fold inactive histograms 

	histType storageType;	
	bool smooth;		/* prevent values greater than binWidth */
	metricStyle metricType; /* sampled function or event counter */
	int intervalCount;	/* # of intervals in use */
	int intervalLimit;	/* # of intervals in use */
	union {
	    Bin *buckets;
	    Interval *intervals;
	} dataPtr; 
	dataCallBack dataFunc;
	foldCallBack foldFunc;
	void *cData;
	bool globalData;    /* true if this histogram stores global phase data */
};
#endif // notdef

#endif

