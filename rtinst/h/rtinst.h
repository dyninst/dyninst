
/*
 * This file contains the standard insrumentation functions that are provied
 *   by the instrumentation layer.
 *
 */

#ifndef _RTINST_H
#define _RTINST_H

typedef void (*instFunc)(void *cdata, int type, char *eventData);

typedef struct intCounterRec intCounter;

struct _parameteters {
    int	arg1;
    int	arg2;
    int	arg3;
    int	arg4;
    int	arg5;
    int	arg6;
    int	arg7;
    int	arg8;
    int	arg9;
    int	arg10;
    int	arg11;
    int	arg12;
    int	arg13;
    int	arg14;
    int	arg15;
};

/* parameters to a instremented function */
typedef struct _parameteters parameters;
typedef enum { processTime, wallTime } timerType;

/* 64 bit time values */
typedef long long int time64;
struct sampleIdRec {
    unsigned int aggregate:1;
    unsigned int id:31;
};

typedef struct sampleIdRec sampleId;


struct intCounterRec {
    intCounter *trigger;
    int value;
    sampleId id;
};


typedef struct floatCounterRec floatCounter;
struct floatCounterRec {
    intCounter *trigger;
    float value;
    sampleId id;
};


typedef struct tTimerRec tTimer;
struct tTimerRec {
    intCounter *trigger;
    int counter;	/* must be 0 to start/stop */
    time64	total;
    time64	start;
    time64	ni_start;
    time64	snapShot;	/* used to get consistant value during st/stp */
    timerType type;
    int		normalize;	/* value to divide total by to get seconds */
    sampleId id;
    volatile int mutex;
};


/*
 * standard inst. functions.
 *
 */

/*
 * start a timer.
 */
void DYNINSTstartTimer(tTimer *timer);

void DYNINSTstopTimer(tTimer *timer);

/*
 * increment the passed counter by one.
 *
 */
void DYNINSTincremmentCounter(intCounter*);

/*
 * decrement the passed counter by one.
 *
 */
void DYNINSTdecrementCounter(intCounter*);

/*
 * Special purpose inst functiions that use trace type specific data.
 *
 */

/* add bytes field to passed counter */
void addBytesCounter(intCounter *data);

typedef struct {
    void *filterData;           /* data to filter on */
    instFunc toCall;            /* function to call if it passes filter */
    void *callData;             /* data to pass to toCall if it is called */
} filterArgs;

/* check src/dest matched passed arg. */
void filterSrc(filterArgs, int type, void *eventData);
void filterDest(filterArgs, int type, void *eventData);

/* check message type field */
void filterMessageType(filterArgs, int type, void *eventData);

/* parse file name (using fd to fileName mapping as needed) */
void filterFileName(filterArgs, int type, void *eventData);

typedef traceStream;
/*
 * timestamp and write the passed trace type, and event data to the passed
 *   trace stream.
 *
 */
void DYNINSTgenerateTraceRecord(traceStream destination, 
				short type, 
				short length, 
    			        void *eventData);

/*
 * Define the size of the per process data area.
 *
 *  This should be a power of two to reduce paging and chacing shifts.
 */
#define SYN_INST_BUF_SIZE	256*1024

#endif
