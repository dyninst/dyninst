/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a SUNOS SPARC processor.
 *
 * $Log: RTfuncs.c,v $
 * Revision 1.28  1996/03/08 18:48:17  newhall
 * added wall and process time args to DYNINSTgenerateTraceRecord.  This fixes
 * a bug that occured when the appl. is paused between reading a timer to compute
 * a metric value and reading a timer again to compute a header value.
 *
 * Revision 1.27  1996/03/01  22:29:07  mjrg
 * Added type to resources.
 * Added function DYNINSTexit for better support for exit from the application.
 * Added reporting of sample in DYNINSTinit to avoid loosing sample values.
 *
 * Revision 1.26  1996/02/15 14:55:46  naim
 * Minor changes to timers and cost model - naim
 *
 * Revision 1.25  1996/02/13  21:36:24  naim
 * Minor change related to the cost model for the CM-5 - naim
 *
 * Revision 1.24  1996/02/01  17:47:57  naim
 * Fixing some problems related to timers and race conditions. I also tried to
 * make a more standard definition of certain procedures (e.g. reportTimer)
 * across all platforms - naim
 *
 * Revision 1.23  1995/12/17  20:58:10  zhichen
 * Hopefully, the samples will arrive
 *
 * Revision 1.22  1995/12/10  16:35:52  zhichen
 * Minor clean up
 *
 * Revision 1.21  1995/10/27  01:04:40  zhichen
 * Added some comments to DYNINSTsampleValues.
 * Added some prototypes
 *
 * Revision 1.20  1995/08/29  20:26:55  mjrg
 * changed sample.observedCost to sample.obsCostIdeal
 *
 * Revision 1.19  1995/05/18  11:08:25  markc
 * added guard prevent timer start-stop during alarm handler
 * added version number
 *
 * Revision 1.18  1995/02/16  09:07:15  markc
 * Made Boolean type RT_Boolean to prevent picking up a different boolean
 * definition.
 *
 * Revision 1.17  1994/11/11  10:16:00  jcargill
 * "Fixed" pause_time definition for CM5
 *
 * Revision 1.16  1994/11/06  09:45:29  jcargill
 * Added prototype for clock functions to fix pause_time metric for cm5
 *
 * Revision 1.15  1994/11/02  19:03:54  hollings
 * Made the observed cost model use a normal variable rather than a reserved
 * register.
 *
 * Revision 1.14  1994/10/27  16:15:53  hollings
 * Temporary hack to normalize cost data until the CM-5 inst supports a max
 * operation.
 *
 * Revision 1.13  1994/09/20  18:27:17  hollings
 * removed hard coded clock value.
 *
 * Revision 1.12  1994/08/02  18:18:56  hollings
 * added code to save/restore FP state on entry/exit to signal handle
 * (really jcargill, but commited by hollings).
 *
 * changed comparisons on time regression to use 64 bit int compares rather
 * than floats to prevent fp rounding error from causing false alarms.
 *
 * Revision 1.11  1994/07/26  20:04:49  hollings
 * removed slots used variables.
 *
 * Revision 1.10  1994/07/22  19:24:53  hollings
 * added actual paused time for CM-5.
 *
 * Revision 1.9  1994/07/14  23:35:34  hollings
 * added return of cost model record.
 *
 * Revision 1.8  1994/07/11  22:47:49  jcargill
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
#include <sys/types.h>
#include <assert.h>
#include <sys/signal.h>
#include <stdio.h>

/*
 * Now our include files.
 *
 */
#include "rtinst/h/trace.h"
#include "rtinst/h/rtinst.h"

#ifdef COSTTEST
extern time64 *DYNINSTtest;
extern int *DYNINSTtestN;
#endif

extern time64 DYNINSTgetCPUtime(void);
extern time64 DYNINSTgetWallTime(void);
/* zxu added the following */
extern void DYNINSTgenerateTraceRecord(traceStream sid,short type,short length,
				      void *eventData, int flush,
				      time64 wall_time,time64 process_time) ;
extern void saveFPUstate(float *base) ;
void restoreFPUstate(float *base) ;
void DYNINSTflushTrace() ;

/* see note below - jkh 10/19/94 */
int DYNINSTnumSampled;
int DYNINSTnumReported;
float DYNINSTsamplingRate;
int DYNINSTtotalAlaramExpires;
char DYNINSTdata[SYN_INST_BUF_SIZE];
char DYNINSTglobalData[SYN_INST_BUF_SIZE];
time64 DYNINSTtotalSampleTime;

/* Prevents timers from being started-stopped during alarm handling */
int DYNINSTin_sample = 0;


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
/*    printf ("DYNINSTsampleValues called...\n"); */
/* 
 * There is alarm that will go off periodically. Each time the alaram expires. A routine
 * called DYNINSTalarmExpires (which is a handler for the alarm) will be executed.
 *
 * DYNINSTalarmExpires()
 * {
 * DYNINSTsampleValue() ; //DYNINSTsampleValue is initially  a dummy at the beginning 
 * }
 *
 * Each time a new focus-metric is instrumented (generated), paradynd
 * will also instrument DYNINSTsamplevalue() accordingly, basically it will add 
 * a pair of rountines DYNINSTreportCounter and DYNINSTreportTimer which will
 * report the corresponding timer and counter value when the alarm expires.
 * So my understanding now is that the alarm actually trigers the writing of 
 * TRACELIBbuffer which make sense. 
 */
    DYNINSTnumReported++;
}

/* The current observed cost since the last call to 
 *      DYNINSTgetObservedCycles(false) 
 */
unsigned DYNINSTobsCostLow;

/************************************************************************
 * int64 DYNINSTgetObservedCycles(RT_Boolean in_signal)
 *
 * report the observed cost of instrumentation in machine cycles.
 *
 * We keep cost as a 64 bit int, but the code generated by dyninst is
 *   a 32 bit counter (for speed).  So this function also converts the
 *   cost into a 64 bit value.
 ************************************************************************/
int64 DYNINSTgetObservedCycles(RT_Boolean in_signal) 
{
    static int64    value = 0;

    if (in_signal) {
        return value;
    }

    /* update the 64 bit version of the counter */
    value += DYNINSTobsCostLow;

    /* reset the low counter */
    DYNINSTobsCostLow = 0;
    return value;
}

/* This function is called from DYNINSTinit, DYNINSTalarmExpire, and DYNINSTexit,
   to report samples.
*/
void DYNINSTreportSamples() {
    time64 start, end;
    float fp_context[33];	/* space to store fp context */

    saveFPUstate(fp_context);
    start = DYNINSTgetCPUtime();

    /* make sure we call this enough to keep observed cost accurate due to
       32 cycle rollover */
    (void) DYNINSTgetObservedCycles(0);

    /* generate actual samples */
    DYNINSTsampleValues();

    DYNINSTreportBaseTramps();

    DYNINSTflushTrace();
    end = DYNINSTgetCPUtime();
    DYNINSTtotalSampleTime += end - start;

    restoreFPUstate(fp_context);
}


/*
 * Call this function to generate a sample when needed.
 *   Exception is the exit from the program which DYNINSTsampleValues should
 *   be called directly!!!
 *
 */
void DYNINSTalarmExpire()
{
#ifdef notdef
    time64 start, end;
    float fp_context[33];	/* space to store fp context */
#endif

#ifdef COSTTEST
    time64 startT, endT;
#endif

    /* printf ("DYNINSTalarmExpired\n");       */
    /* should use atomic test and set for this */
    if (DYNINSTin_sample) return;
    DYNINSTin_sample = 1;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    /* only sample every DYNINSTsampleMultiple calls */
    DYNINSTtotalAlaramExpires++;
    /*
    printf("DYNINSTnumSampled = %d, DYNINSTsampleMultiple=%d\n",
  	    DYNINSTnumSampled, DYNINSTsampleMultiple) ;
    */
    if ((++DYNINSTnumSampled % DYNINSTsampleMultiple) == 0)  
      {

	DYNINSTreportSamples();
#ifdef notdef
	saveFPUstate(fp_context);
	start = DYNINSTgetCPUtime();

	/* make sure we call this enough to keep observed cost accurate due to
	   32 cycle rollover */
	(void) DYNINSTgetObservedCycles(0);

	/* generate actual samples */
	DYNINSTsampleValues();

	DYNINSTreportBaseTramps();

	DYNINSTflushTrace();
	end = DYNINSTgetCPUtime();
	DYNINSTtotalSampleTime += end - start;

	restoreFPUstate(fp_context);
#endif
    }

    DYNINSTin_sample = 0;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[4]+=endT-startT;
    DYNINSTtestN[4]++;
#endif
}
