
/*
 * This file contains the standard insrumentation functions that are provied
 *   by the instrumentation layer.
 *
 */

#ifndef _RTINST_H
#define _RTINST_H

typedef void (*instFunc)(void *cdata, int type, char *eventData);

typedef struct intCounterRec intCounter;

/* parameters to a instremented function */
typedef struct _parameteters parameters;
typedef enum { processTime, wallTime } timerType;

/* 64 bit time values */
typedef long long int time64;
typedef long long int int64;

struct sampleIdRec {
    unsigned int aggregate:1;
    unsigned int id:31;
};

struct endStatsRec {
    int alarms;
    int numReported;
    float instCycles;
    float instTime;
    float handlerCost;
    float totalCpuTime;
    int samplesReported;
    float samplingRate;
    float totalWallTime;
    int userTicks;
    int instTicks;
};

typedef struct sampleIdRec sampleId;


struct intCounterRec {
    int value;		/* this field must be first for setValue to work -jkh */
    sampleId id;
};


typedef struct floatCounterRec floatCounter;
struct floatCounterRec {
    float value;
    sampleId id;
};


typedef struct tTimerRec tTimer;
struct tTimerRec {
    int 	counter;		/* must be 0 to start/stop */
    time64	total;
    time64	start;
    time64	snapShot;	/* used to get consistant value during st/stp */
    int		normalize;	/* value to divide total by to get seconds */
    timerType 	type;
    sampleId 	id;
    volatile char mutex;
    volatile char sampled;
};

typedef (*filterFunc)(void *cdata, parameters *params);

/*
 * standard inst. functions.
 *
 */

/*
 * return the current CPU time in usec.
 */
time64 DYNINSTgetCPUtime();
time64 DYNINSTgetWallTime();

void DYNINSTflushTrace();

/*
 * start a timer.
 */
void DYNINSTstartTimer(tTimer *timer);

void DYNINSTstopTimer(tTimer *timer);

typedef traceStream;
/*
 * timestamp and write the passed trace type, and event data to the passed
 *   trace stream.
 *
 */
void DYNINSTgenerateTraceRecord(traceStream destination, 
				short type, 
				short length, 
    			        void *eventData,
				int flush);

/*
 * Define the size of the per process data area.
 *
 *  This should be a power of two to reduce paging and chacing shifts.
 */
#define SYN_INST_BUF_SIZE	1024*512

#endif
