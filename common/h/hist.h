#ifndef HIST
#define HIST

#include "util/h/list.h"
#include "rtinst/h/trace.h"

#ifndef False
#define False	0
#define True	1
#endif

typedef double timeStamp;

typedef struct {
    timeStamp start;
    timeStamp end;
    sampleValue value;
} Interval;

typedef enum { HistInterval, HistBucket } histType;
typedef enum { HistNewValue, HistNewTimeBase } callType;
typedef sampleValue Bin;

typedef void (*foldCallBack)(timeStamp, void* userData);

typedef void (*dataCallBack)(timeStamp st,
			     timeStamp en,
			     timeStamp value,
			     void *userData);

class Histogram;

typedef enum { histActive, histInactive } histStatus;
typedef enum { histSum, histAverage } histCompact;
typedef enum { histSplit, histSet } histAddMode;
typedef enum { EventCounter, SampledFunction } metricStyle;

class Histogram {
	friend class histDisplay;
	void newDataFunc(callType type, timeStamp time, void* userData);
    public:
	Histogram(metricStyle, dataCallBack, foldCallBack, void* );
	Histogram(Bin *buckets, metricStyle, dataCallBack, foldCallBack, void*);
	sampleValue getValue();
	sampleValue getValue(timeStamp start, timeStamp end);
	void addInterval(timeStamp start, timeStamp end, 
	    sampleValue value, Boolean smooth);
	void addPoint(timeStamp start, sampleValue value) {
	    addInterval(start, start, value, False);
	}
	timeStamp currentTime() { 
		return((timeStamp)(lastGlobalBin*bucketSize)); 
	}
	static int numBins;		/* max bins to use */
	static timeStamp bucketSize;	/* width of a bucket */
    private:
	void foldAllHist();
	void convertToBins();
	void bucketValue(timeStamp start, timeStamp end, 
		sampleValue value, Boolean smooth);

	static timeStamp total_time;	/* numBins * bucketSize */
	static int lastGlobalBin;	/* global point we have data from */
	static Histogram *allHist;	/* linked list of all histograms */

	Histogram *next;		/* linked list of all histograms */
	int lastBin;			/* current (for this hist) last bin */

	histType storageType;	
	Boolean smooth;		/* prevent values greater than binWidth */
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
