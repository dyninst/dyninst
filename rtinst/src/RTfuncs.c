/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a SUNOS SPARC processor.
 *
 * $Log: RTfuncs.c,v $
 * Revision 1.8  1994/07/11 22:47:49  jcargill
 * Major CM5 commit: include syntax changes, some timer changes, removal
 * of old aggregation code, old pause code, added signal-driven sampling
 * within node processes
 *
 * Revision 1.7  1994/07/05  03:25:09  hollings
 * obsereved cost model.
 *
 * Revision 1.6  1994/02/02  00:46:11  hollings
 * Changes to make it compile with the new tree.
 *
 * Revision 1.5  1993/12/13  19:47:29  hollings
 * support for DYNINSTsampleMultiple.
 *
 * Revision 1.4  1993/10/19  15:29:58  hollings
 * new simpler primitives.
 *
 * Revision 1.3  1993/10/01  18:15:53  hollings
 * Added filtering and resource discovery.
 *
 * Revision 1.2  1993/08/26  19:43:58  hollings
 * new include syntax.
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <assert.h>
#include <sys/signal.h>
#include <stdio.h>

/*
 * Now our include files.
 *
 */
#include "rtinst/h/trace.h"
#include "rtinst/h/rtinst.h"

/* This marks the end of user code in the text file. */
/* This is to prevent system libraries with symbols compiled into them
 *    from adding extranious material to our inst. environment.
 */
void DYNINSTendUserCode()
{
}

char DYNINSTdata[SYN_INST_BUF_SIZE];
char DYNINSTglobalData[SYN_INST_BUF_SIZE];
int DYNINSTnumSampled;
int DYNINSTnumReported;
int DYNINSTtotalAlaramExpires;

/*
 * for now costCount is in cycles. 
 */
float DYNINSTcyclesToUsec = 1/66.0;
time64 DYNINSTtotalSampleTime;

void DYNINSTreportCounter(intCounter *counter)
{
    traceSample sample;

    sample.value = counter->value;
    sample.id = counter->id;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
}

void DYNINSTsimplePrint()
{
    printf("inside dynamic inst function\n");
}

void DYNINSTentryPrint(int arg)
{
    printf("enter %d\n", arg);
}

void DYNINSTcallFrom(int arg)
{
    printf("call from %d\n", arg);
}

void DYNINSTcallReturn(int arg)
{
    printf("return to %d\n", arg);
}

void DYNINSTexitPrint(int arg)
{
    printf("exit %d\n", arg);
}

volatile int DYNINSTsampleMultiple = 1;

/*
 * This is a function that should be called when we want to sample the
 *   timers and counters.  The code to do the sampling is added as func
 *   entry dynamic instrumentation.
 *
 *   It also reports the current value of the observed cost.
 *
 */
void DYNINSTsampleValues()
{
/*     printf ("DYNINSTsampleValues called...\n"); */
    DYNINSTnumReported++;
}

#define FOUR_BILLION (((double) 1.0) * 1024 * 1024 * 1024 * 4)

/* 
 * Define a union to let us get at the bits of a 64 bit integer.
 *
 *  This is needed since gcc doesn't support 64 bit ints fully. 
 *
 *  Think at least twice before changing this. 	jkh 7/2/94
 */
union timeUnion {
    unsigned int array[2];
    int64 i64;
};

/*
 * Return the observed cost of instrumentation in machine cycles.
 *
 */
int64 DYNINSTgetObservedCycles()
{
    static int64 previous;
    static union timeUnion value;
    register unsigned int lowBits asm("%g7");

    value.array[1] = lowBits;
    if (value.i64 < previous) {
	/*  add to high word 
	 *
	 ************************** WARNING ***************************
	 *     this assumes we sample frequenly enough to catch these *
	 **************************************************************
	 */
        fprintf(stderr, "current %f, previous %f\n", ((double) value.i64),
		((double) previous));
        fprintf(stderr, "Warning observed cost register wrapped\n");
	value.array[0] += 1;

	fflush(stderr);
    }
    previous = value.i64;
    return(value.i64);
}

void DYNINSTreportCost(intCounter *counter)
{
    /*
     *     This should eventually be replaced by the normal code to report
     *     a mapped counter???
     */

    double cost;
    int64 value; 
    static double prevCost;
    traceSample sample;

    value = DYNINSTgetObservedCycles();
    cost = ((double) value) * (DYNINSTcyclesToUsec / 1000000.0);

    if (cost < prevCost) {
	fprintf(stderr, "Fatal Error Cost counter went backwards\n");
	fflush(stderr);
	sigpause(0xffff);
    }

    prevCost = cost;

    sample.value = cost;
    sample.id = counter->id;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
}

/*
 * Call this function to generate a sample when needed.
 *   Exception is the exit from the program which DYNINSTsampleValues should
 *   be called directly!!!
 *
 */
void DYNINSTalarmExpire()
{
    time64 start, end;
    static int inSample;

/*     printf ("DYNINSTalarmExpired\n"); */
    /* should use atomic test and set for this */
    if (inSample) return;

    inSample = 1;

    /* only sample every DYNINSTsampleMultiple calls */
    DYNINSTtotalAlaramExpires++;
    if ((++DYNINSTnumSampled % DYNINSTsampleMultiple) == 0)  {
	start = DYNINSTgetCPUtime();
	DYNINSTsampleValues();
	end = DYNINSTgetCPUtime();
	DYNINSTtotalSampleTime += end - start;
    }

    inSample = 0;
}

