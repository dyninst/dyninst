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

/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a SUNOS SPARC processor.
 *
 * $Log: RTfuncs.c,v $
 * Revision 1.31  1996/08/16 21:27:35  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.30  1996/04/09 22:20:54  newhall
 * changed DYNINSTgetWallTime to DYNINSTgetWalltime to fix undefined symbol
 * errors when applications are linked with libdyninstRT_cp.a
 *
 * Revision 1.29  1996/04/09  15:52:40  naim
 * Fixing prototype for procedure DYNINSTgenerateTraceRecord and adding
 * additional parameters to a call to this function in RTtags.c that has these
 * parameters missing - naim
 *
 * Revision 1.28  1996/03/08  18:48:17  newhall
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
extern time64 DYNINSTgetWalltime(void);
/* zxu added the following */
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
