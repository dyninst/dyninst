/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a SUNOS SPARC processor.
 *
 * $Log: RTfuncs.c,v $
 * Revision 1.6  1994/02/02 00:46:11  hollings
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
 */
void DYNINSTsampleValues()
{
    DYNINSTnumReported++;
}

void DYNINSTalarmExpire()
{
    static int inSample;

    /* should use atomic test and set for this */
    if (inSample) return;

    inSample = 1;

    /* only sample every DYNINSTsampleMultiple calls */
    if ((++DYNINSTnumSampled % DYNINSTsampleMultiple) == 0) 
	DYNINSTsampleValues();

    inSample = 0;
}

