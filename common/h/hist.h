#ifndef HIST
#define HIST

#include "util/h/list.h"
#include "util/h/sys.h"

typedef enum { HistInterval, HistBucket } histType;
typedef enum { HistNewValue, HistNewTimeBase } callType;
typedef sampleValue Bin;

typedef void (*foldCallBack)(timeStamp, void* userData);

typedef void (*dataCallBack)(sampleValue *buckets,
			     int numberOfBuckets,
			     int first,
			     void *userData);

class Histogram;

typedef enum { histActive, histInactive } histStatus;
typedef enum { histSum, histAverage } histCompact;
typedef enum { histSplit, histSet } histAddMode;
typedef enum { EventCounter, SampledFunction } metricStyle;

class Histogram {
	void newDataFunc(callType type, timeStamp time, void* userData);
    public:
	~Histogram();
	Histogram(metricStyle, dataCallBack, foldCallBack, void* );
	Histogram(Bin *buckets, metricStyle, dataCallBack, foldCallBack, void*);
	sampleValue getValue();
	sampleValue getValue(timeStamp start, timeStamp end);
	int getBuckets(sampleValue *buckets, int numberOfBuckets, int first);
	void addInterval(timeStamp start, timeStamp end, 
	    sampleValue value, bool smooth);
	void addPoint(timeStamp start, sampleValue value) {
	    addInterval(start, start, value, false);
	}
	timeStamp currentTime() { 
		return((timeStamp)(lastGlobalBin*bucketSize)); 
	}
	static int numBins;		/* max bins to use */
	static timeStamp bucketSize;	/* width of a bucket */
	static int lastGlobalBin;	/* global point we have data from */
    private:
	void foldAllHist();
	void convertToBins();
	void bucketValue(timeStamp start, timeStamp end, 
		sampleValue value, bool smooth);

	static timeStamp total_time;	/* numBins * bucketSize */
	static Histogram *allHist;	/* linked list of all histograms */

	Histogram *next;		/* linked list of all histograms */
	int lastBin;			/* current (for this hist) last bin */

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
};

#endif
