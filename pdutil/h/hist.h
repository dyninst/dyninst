#ifndef HIST
#define HIST

#include "util/list.h"
#include "rtinst/trace.h"

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

typedef void (*subscriberCallBack)(callType, timeStamp, void* userData);
class Histogram;

/*
 * upcall mechanism for routines that need to find out about changes to
 *   a histogram.
 *
 */
class HistogramSubscriber {
	friend Histogram;
    public:
	HistogramSubscriber(timeStamp maxRate, subscriberCallBack func, void *userData);
	deliver(int bin);
    private:
	void *userData;
	timeStamp interval;
	timeStamp lastCall;
	subscriberCallBack callBack; 
};

typedef enum { histActive, histInactive } histStatus;
typedef enum { histSum, histAverage } histCompact;
typedef enum { histSplit, histSet } histAddMode;
typedef enum { EventCounter, SampledFunction } metricStyle;

class Histogram {
	friend class histDisplay;
	void newDataFunc(callType type, timeStamp time, void* userData);
    public:
	Histogram(metricStyle);
	Histogram(Bin *buckets, metricStyle);
	void enable() { status = histActive; }
	void disable() { status = histInactive; }
	sampleValue getValue();
	sampleValue getValue(timeStamp start, timeStamp end);
	void addInterval(timeStamp start, timeStamp end, 
	    sampleValue value, Boolean smooth);
	void addPoint(timeStamp start, sampleValue value) {
	    addInterval(start, start, value, False);
	}
	int subscribe(timeStamp maxRate,subscriberCallBack func,void *);
	void unsubscribe(int id) { 
		subscribers.remove((HistogramSubscriber*)id); 
	}
	timeStamp currentTime() { 
		return((timeStamp)(lastGlobalBin*bucketSize)); 
	}
	static int numBins;		/* max bins to use */
    private:
	void foldAllHist();
	void convertToBins();
	void bucketValue(timeStamp start, timeStamp end, 
		sampleValue value, Boolean smooth);

	static timeStamp bucketSize;	/* width of a bucket */
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
	List<HistogramSubscriber*> subscribers;
	histStatus status;
};

#endif
