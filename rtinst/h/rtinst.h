
/*
 * This file contains the standard insrumentation functions that are provied
 *   by the instrumentation layer.
 *
 */

#ifndef _RTINST_H
#define _RTINST_H

/* We sometimes include this into assembly files, so guard the struct defs. */
#if !defined(__ASSEMBLER__)

/* If we must make up a boolean type, we should make it unique */
#define RT_Boolean unsigned char
#define RT_TRUE 1
#define RT_FALSE 0

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
    volatile int 	counter;	/* must be 0 to start/stop */
    volatile time64	total;
    volatile time64	start;
    volatile time64     lastValue;
    volatile time64	snapShot;	/* used to get consistant value 
					   during st/stp */
    volatile int	normalize;	/* value to divide total by to 
					   get seconds */
    volatile timerType 	type;
    volatile sampleId 	id;
    volatile char mutex;
    volatile char sampled;
};

typedef int (*filterFunc)(void *cdata, parameters *params);
typedef int traceStream;

#endif

/*
 * Define the size of the per process data area.
 *
 *  This should be a power of two to reduce paging and chacing shifts.
 */
#if defined(hppa1_1_hp_hpux)
#define SYN_INST_BUF_SIZE	1024*256 /* cannot support larger jumps */
#else
#define SYN_INST_BUF_SIZE	1024*1024
#endif

#endif
