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
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.44  1996/08/16 21:27:30  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.43  1996/06/27 14:34:57  naim
 * Preventing race condition in DYNINSTgenerateTraceRecord (change made by Xu
 * and commited by Oscar) - naim
 *
 * Revision 1.42  1996/05/16  22:20:36  mjrg
 * commented out a debugging message
 *
 * Revision 1.41  1996/04/09 22:20:53  newhall
 * changed DYNINSTgetWallTime to DYNINSTgetWalltime to fix undefined symbol
 * errors when applications are linked with libdyninstRT_cp.a
 *
 * Revision 1.40  1996/04/09  15:52:38  naim
 * Fixing prototype for procedure DYNINSTgenerateTraceRecord and adding
 * additional parameters to a call to this function in RTtags.c that has these
 * parameters missing - naim
 *
 * Revision 1.39  1996/03/12  20:50:01  mjrg
 * Improved handling of process termination
 *
 * Revision 1.38  1996/03/08 18:48:14  newhall
 * added wall and process time args to DYNINSTgenerateTraceRecord.  This fixes
 * a bug that occured when the appl. is paused between reading a timer to compute
 * a metric value and reading a timer again to compute a header value.
 *
 */

#include <stdlib.h>
/* #include <signal.h> */
#include <assert.h>

/* our include files */
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/sys.h"
#define extern
#include "traceio.h"
#undef extern

#include <cm/timers.h>
#include <cm/cmmd/amx.h>
#include <cm/cmmd/mp.h>
#include <cm/cmmd/cn.h>
#include <cm/cmmd/io.h>
#include <cm/cmmd/util.h>
#include <cm/cmmd/cmmd_constants.h>
#include <cm/cmmd.h>
#include <cmsys/cm_signal.h>
#define pe_obj
#include <cm/cmna.h>
#include <cmsys/ni_interface.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stdtypes.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <cm/cm_file.h>
#include <cm/cm_errno.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/filio.h>
#include <math.h>

#define NI_CLK_USEC 33
#define MILLION 1000000

#ifdef COSTTEST
time64 DYNINSTtest[10]={0,0,0,0,0,0,0,0,0,0};
int DYNINSTtestN[10]={0,0,0,0,0,0,0,0,0,0};
#endif

char *TRACELIBcurrPtr;		/* current pointer in buffer  */
char *TRACELIBfreePtr;		/* pointer to next free byte in buffer */
char *TRACELIBendPtr;		/* last byte in trace buffer */
char *TRACELIBtraceBuffer;	/* beginning of trace buffer */
int TRACELIBmustRetry;		/* signal variable from consumer -> producer */
extern float DYNINSTcyclesToUsec;

time64 getProcessTime();
time64 DYNINSTgetWalltime();
extern int DYNINSTin_sample;

extern time64 DYNINSTtotalSampleTime;
float DYNINSTcyclesToUsec;
int DYNINSTnprocs;
int DYNINSTtotalSamples;
time64 DYNINSTlastCPUTime;
time64 DYNINSTlastWallTime;


struct timer_buf {
    unsigned int high;
    unsigned int sync;
    time64   ni_time;
/*   instead of: */
/*     unsigned int ni_time_high; */
/*     unsigned int ni_time_low; */
    unsigned int trap_start;
    unsigned int trap_cnt;
    unsigned int trap_time;
};

typedef union {
    struct {
	unsigned int high;
	unsigned int low;
    } parts;
    time64 value;
} timeParts;

volatile unsigned int *ni; /* zxu deleted "static" (unneeded; clashes w/blz) */
volatile struct timer_buf timerBuffer;

int  must_end_timeslice()      /* changed to return int */
{
  /*printf ("Need to end timeslice!!!\n");*/
  return(0);
  /*
   * We still don't do anything good for this case.
   * Simple solutions are:
   * 1.  Increase buffer size
   * 2.  Implement a way for a node to trigger a context switch and
   * surrender the rest of a timeslice.
   */
}

#ifdef notdef
inline time64 getProcessTime()
{
    time64 end;
    time64 ni_end;
    time64 ni2;
    time64 dummy;

retry:
    CMOS_get_NI_time(&ni_end, &dummy);
    CMOS_get_time(&end);
    CMOS_get_NI_time(&ni2, &dummy);
    if (ni_end != ni2) goto retry;
    return(end-ni_end);
}
#endif

inline time64 getProcessTime()
{
    time64 ret;
    timeParts end;
    time64 ni_end;
    static time64 previous=0;

/* TEST code
    int nretries=0;
*/ 

retry:
    timerBuffer.sync = 1;
    ni_end = timerBuffer.ni_time;
    end.parts.high = timerBuffer.high;
    end.parts.low = *ni;
    if (timerBuffer.sync != 1) goto retry;

    /* check for three way race of start/stop & sample & wrap. */
    if (end.parts.high != timerBuffer.high) goto retry;

    /* three way race with start/stop & sample & scheduler */
    if (ni_end != timerBuffer.ni_time) goto retry;

    ret = end.value-ni_end;
    if (ret < 0) goto retry;

    if (ret<previous) {
/* TEST code
      printf("RTcm5_pn: PROCESS time going backwards\n");
      printf("current = %f, previous = %f\n",(double)ret,(double)previous);
      nretries++;
      printf("retrying (%d)...\n",nretries);
*/
      goto retry;
    }
    previous=ret;

    return(ret);
}


time64 DYNINSTgetCPUtime()
{
     time64 now;

     now = getProcessTime();
     now /= (time64) NI_CLK_USEC;
     return(now);
}


inline time64 getWallTime()
{
    timeParts end;
    static time64 previous=0;

/* TEST code
    int nretries=0;
*/

retry:
    timerBuffer.sync = 1;
    end.parts.high = timerBuffer.high;
    end.parts.low = *ni;
    if (timerBuffer.sync != 1) goto retry;

    /* i copied the following two lines from getProcessTime -zxu 11/04/95 */
    /* check for three way race of start/stop & sample & wrap. */
    /* if (end.parts.high != timerBuffer.high) goto retry;     */

    if (end.value < previous) {
/* TEST code
      printf("RTcm5_pn: WALL time going backwards\n");
      printf("current=%f, previous=%f\n",(double)end.value,(double)previous);
      nretries++;
      printf("retrying (%d)...\n",nretries);
*/
      goto retry;
    }
    previous = end.value;
    return(end.value);
}

inline time64 DYNINSTgetWalltime()
{
    time64 now;

    now = getWallTime();
    now /= (time64) NI_CLK_USEC;
    return(now);
}

void DYNINSTstartProcessTimer(tTimer *timer)
{
    /* events that trigger timer starts during trace flushes are to be 
       ignored */

#ifdef COSTTEST
    time64 startT,endT;
#endif

    if (DYNINSTin_sample) return;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
	 timer->start = getProcessTime();
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[0]+=endT-startT;
    DYNINSTtestN[0]++;
#endif
}


void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 now;

    /* events that trigger timer stops during trace flushes are to be 
       ignored */

#ifdef COSTTEST
    time64 startT,endT;
#endif

    if (DYNINSTin_sample) return;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    /* don't stop a counter that is not running */
    if (!timer->counter) return;

    /* Warning - there is a window between setting now, and mutex that
       can cause time to go backwards by the time to execute the
       instructions between these two points.  This is not a cummlative error
       and should not affect samples.  This was done (rather than re-sampling
       now because the cost of computing now is so high).
    */
    if (timer->counter == 1) {
	now = getProcessTime();
	timer->snapShot = now - timer->start + timer->total;
	timer->mutex = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
	timer->total = getProcessTime() - timer->start + timer->total;
	timer->counter = 0;
	timer->mutex = 0;
	if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("RTcm5_pn: process timer rollback\n"); fflush(stdout);
            abort();
	}
    } else {
	timer->counter--;
    }

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[1]+=endT-startT;
    DYNINSTtestN[1]++;
#endif
}

void DYNINSTstartWallTimer(tTimer *timer)
{
    /* events that trigger timer starts during trace flushes are to be 
       ignored */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
	 timer->start = getWallTime();
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[2]+=endT-startT;
    DYNINSTtestN[2]++;
#endif
}

void DYNINSTstopWallTimer(tTimer *timer)
{
    time64 now;

    /* events that trigger timer stops during trace flushes are to be 
       ignored */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    /* don't stop a counter that is not running */
    if (!timer->counter) return;

    /* Warning - there is a window between setting now, and mutex that
       can cause time to go backwards by the time to execute the
       instructions between these two points.  This is not a cummlative error
       and should not affect samples.  This was done (rather than re-sampling
       now because the cost of computing now is so high).
    */
    if (timer->counter == 1) {
	now = getWallTime();
	timer->snapShot = now - timer->start + timer->total;
	timer->mutex = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
	timer->total = getWallTime() - timer->start + timer->total;
	timer->counter = 0;
	timer->mutex = 0;
	if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("RTcm5_pn: wall timer rollback\n"); fflush(stdout);
            abort();
	}
    } else {
	timer->counter--;
    }

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[3]+=endT-startT;
    DYNINSTtestN[3]++;
#endif
}

/* CMMD programs use set_timer_buf, as usual.
   For Blizzard programs, we cannot use set_timer_buf because
   the blizzard library already defines this function. */
#ifdef blizzard_cm5
#define set_timer_buf PD_set_timer_buf
#endif

void set_timer_buf(volatile struct timer_buf *param)
{
/*     asm("set 50,%g1"); */
    asm("set 29,%g1");
    /* asm("set 23,%g1"); */
    asm("retl");
    asm("ta 0x8");
}

time64 startWall;           /* zxu deleted storage class static for 
                               blz_RTcm5_pn.c */


void DYNINSTreportTimer(tTimer *timer)
/*
 * it reports the total time since beginning -zxu
 */
{
    time64 now=0;
    time64 total;
    traceSample sample;
    time64 pTime, wall_time, process_time;

   wall_time = getWallTime();
   process_time = getProcessTime();

#ifdef COSTTEST
    time64 startT, endT;
    startT = DYNINSTgetCPUtime();
#endif


    if (timer->mutex) {
	total = timer->snapShot;
    } else if (timer->counter) {
	/* timer is running */
	if (timer->type == processTime) {
	   now = process_time;
	   total = now - timer->start;
	} else {
           now = wall_time;   
	   total = now - timer->start;
	}
	total += timer->total;
    } else {
	total = timer->total;
    }

    if (total < timer->lastValue) {
	if (timer->type == processTime) {
	    printf("process ");
	} else {
	    printf("wall ");
	}
	printf("RTcm5_pn: time regressed timer %d, total = %f, last = %f\n",
	       timer->id.id, (float) total, (float) timer->lastValue);
	if (timer->counter) {
	    printf("timer was active\n");
	} else {
	    printf("timer was inactive\n");
	}
        printf("mutex=%d, counter=%d, sampled=%d, snapShot=%f\n\n",
	       (int)timer->mutex, (int)timer->counter, (int)timer->sampled,
	       (double) timer->snapShot);
	printf("now = %f\n start = %f\n total = %f\n",
	       (double) now, (double) timer->start, (double) timer->total);
	fflush(stdout);
	abort();
    }

    timer->lastValue = total;

    sample.value = total / (double) timer->normalize;
    sample.id = timer->id;
    DYNINSTtotalSamples++;

    wall_time += startWall;
    wall_time /= NI_CLK_USEC;
    process_time /= NI_CLK_USEC;
    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample),&sample,RT_FALSE,
			wall_time,process_time);

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[5]+=endT-startT;
    DYNINSTtestN[5]++;
#endif
}


int DYNINSTnoHandlers;
int DYNINSTinitDone;        /* zxu deleted storage class static */
time64 DYNINSTelapsedTime;
tTimer DYNINSTelapsedCPUTime;

extern float DYNINSTsamplingRate;
extern void DYNINSTreportSamples();
void DYNINSTprintCost();

#define NI_BASE       	      (0x20000000)
#define NI_TIME_A     	      (NI_BASE + 0x0070)

/*
 * should be called before main in each process.
 *
 */
void DYNINSTinit()
{
    extern int DYNINSTnprocs;
    /* char *interval; */
    struct itimerval timeInterval;
    int sampleInterval;
    struct timeval tv;
    time64 startNItime;
    extern void DYNINSTalarmExpire();
    extern int DYNINSTsampleMultiple;

    /* temporary correction until we can make the CM-5 aggregation code
       perform a max operation in addition to sum - jk 10/19/94 */
    DYNINSTnprocs = 32;

    /* This is the base time, in generateTraceRecord
     * startWall is added to the hearder.wall 
     * - zxu 10/19/95
     */
    CMOS_get_time(&startNItime);
    gettimeofday(&tv, NULL);

    startWall = tv.tv_sec;  
    startWall *= MILLION;
    startWall += tv.tv_usec;

    /* change time base to ni time */
    startWall *= NI_CLK_USEC;

    startWall -= startNItime;

/*     printf ("startWall is %x%x\n", *(int *) &startWall,  */
/* 	    *((int *) &startWall) + 1); */

    ni = (unsigned int *) NI_TIME_A;
    set_timer_buf(&timerBuffer);

    if (getenv("DYNINSTsampleMultiple")) {
	DYNINSTsampleMultiple = atoi(getenv("DYNINSTsampleMultiple"));
    }
	
    /*
     * Allocate a trace buffer for this node, and set up the buffer
     * pointers.
     */
    TRACELIBcurrPtr = TRACELIBfreePtr = TRACELIBtraceBuffer = (char *) malloc (TRACE_BUF_SIZE);
    TRACELIBendPtr = TRACELIBtraceBuffer + TRACE_BUF_SIZE - 1;


    /* init these before the first alarm can expire */
    DYNINSTlastCPUTime = DYNINSTgetCPUtime();
    DYNINSTlastWallTime = DYNINSTgetWalltime();

    /*
     * Set up the SIGALRM handler stuff so counters/timers get sampled and
     * traces get puit into the traceBuffer every once in a while.
     */

#ifdef n_def
    sampleInterval = 500000;     /* default is 500msec  */
    interval = (char *) getenv("DYNINSTsampleInterval");
    if (interval) {
	sampleInterval = atoi(interval);
    }
#endif

    /* set sampling rate to default value in util/sys.h */
    sampleInterval = BASESAMPLEINTERVAL/2.0;  /* bart's information theory */

    DYNINSTsamplingRate = ((float) sampleInterval)/ 1000000.0;

    CMOS_signal (CM_SIGALRM, DYNINSTalarmExpire, ~0);
    timeInterval.it_interval.tv_sec = ((int) (sampleInterval) / 1000000);
    timeInterval.it_interval.tv_usec = sampleInterval % 1000000;
    CMOS_setitimer (1, &timeInterval, NULL);
    CMOS_ualarm (sampleInterval);

    DYNINSTcyclesToUsec = 1.0/((float) NI_CLK_USEC);

    CMOS_get_time(&DYNINSTelapsedTime);
    DYNINSTstartProcessTimer(&DYNINSTelapsedCPUTime);

    /* report samples here to stablish the initial time of samples. */
    DYNINSTreportSamples();

    DYNINSTinitDone = 1;
/* printf("DYNINSTinit is called ...\n") ; */

}

void DYNINSTexit()
{
  static int done;
  if (done) return;
  done = 1;

  DYNINSTreportSamples();
  DYNINSTprintCost();

  { 
    /* we need to wait here until paradynd gets all the data from the buffer */
    volatile unsigned *freePtr;
    do {
      freePtr = (unsigned *)&TRACELIBfreePtr;
    } while (*freePtr > (unsigned)TRACELIBtraceBuffer);
  }

}


/*
 * generate a trace record onto the named stream.
 *
 */
void DYNINSTgenerateTraceRecord(traceStream sid, short type, short length,
    void *eventData, int flush,time64 wall_time,time64 process_time)
{
    int count;
    char buffer[1024];
    traceHeader header;
    static unsigned inDYNINSTgenerateTraceRecord = 0;

    if (inDYNINSTgenerateTraceRecord) return;
    inDYNINSTgenerateTraceRecord = 1;

    if (!DYNINSTinitDone) {
      inDYNINSTgenerateTraceRecord = 0;
      return;
    }

    header.wall = wall_time;
    header.process = process_time;

    length = ALIGN_TO_WORDSIZE(length); 

    header.type = type;
    header.length = length;
    count = 0;
    memcpy(&buffer[count], &sid, sizeof(traceStream));
    count += sizeof(traceStream);

    memcpy(&buffer[count], &header, sizeof(header));
    count += sizeof(header);

    count = ALIGN_TO_WORDSIZE(count);
    memcpy(&buffer[count], eventData, length);
    count += length;

    TRACE(buffer, count);
    inDYNINSTgenerateTraceRecord = 0;
}

void DYNINSTbreakPoint(int arg)
{
    printf("Break point %d reached\n", arg);
/*     asm("ta 0x81"); */
}

/*
 * Traces are not buffered (this way at least) on the nodes of the CM-5.
 *   Make this a nop.
 */
void DYNINSTflushTrace()
{
}

extern int DYNINSTnumReported;
extern int DYNINSTtotalAlaramExpires;
extern time64 DYNINSTtotalSampleTime;

void DYNINSTprintCost()
{
    int64 value;
    time64 endWall;
    struct endStatsRec stats;
    extern int64 DYNINSTgetObservedCycles(RT_Boolean);
    time64 pTime, wall_time, process_time;

    wall_time = getWallTime();
    wall_time += startWall;
    wall_time /= NI_CLK_USEC;

    process_time = getProcessTime();
    process_time /= NI_CLK_USEC;

    DYNINSTstopProcessTimer(&DYNINSTelapsedCPUTime);
    CMOS_get_time(&endWall);
    endWall -= DYNINSTelapsedTime;

    value = DYNINSTgetObservedCycles(0);
    stats.instCycles = value;

    value *= DYNINSTcyclesToUsec;

    stats.alarms = DYNINSTtotalAlaramExpires;
    stats.numReported = DYNINSTnumReported;
    stats.instTime = ((double) value)/1000000.0;
    stats.handlerCost = ((double) DYNINSTtotalSampleTime)/1000000.0;

    stats.totalCpuTime = ((double) DYNINSTelapsedCPUTime.total)/
	(MILLION * NI_CLK_USEC);
    stats.totalWallTime = ((double) endWall)/ (NI_CLK_USEC * MILLION);

    stats.samplesReported = DYNINSTtotalSamples;
    stats.samplingRate = DYNINSTsamplingRate;

    stats.userTicks = 0;
    stats.instTicks = 0;

    /* record that we are done -- should be somewhere better. */
    DYNINSTtotalSamples++;
    DYNINSTgenerateTraceRecord(0, TR_EXIT, sizeof(stats), &stats, 1,
				wall_time,process_time);
}


/*
 * These are null routines for Unix: signal handler semantics
 * guarantee that the FPU state is saved/restored.  Not so on CM5 nodes! 
 */

void saveFPUstate(float *base)
{
    asm ("std     %f0,[%o0+0]");
    asm ("std     %f2,[%o0+8]");
    asm ("std     %f4,[%o0+16]");
    asm ("std     %f6,[%o0+24]");
    asm ("std     %f8,[%o0+32]");
    asm ("std     %f10,[%o0+40]");
    asm ("std     %f12,[%o0+48]");
    asm ("std     %f14,[%o0+56]");
    asm ("std     %f16,[%o0+64]");
    asm ("std     %f18,[%o0+72]");
    asm ("std     %f20,[%o0+80]");
    asm ("std     %f22,[%o0+88]");
    asm ("std     %f24,[%o0+96]");
    asm ("std     %f26,[%o0+104]");
    asm ("std     %f28,[%o0+112]");
    asm ("std     %f30,[%o0+120]");

    asm ("st      %fsr,[%o0+128]"); /* Store FSR. */
}


void restoreFPUstate(float *base)
{
    asm ("ld      [%o0+128],%fsr"); /* Restore FSR. */

    asm ("ldd     [%o0+0],%f0");
    asm ("ldd     [%o0+8],%f2");
    asm ("ldd     [%o0+16],%f4");
    asm ("ldd     [%o0+24],%f6");
    asm ("ldd     [%o0+32],%f8");
    asm ("ldd     [%o0+40],%f10");
    asm ("ldd     [%o0+48],%f12");
    asm ("ldd     [%o0+56],%f14");
    asm ("ldd     [%o0+64],%f16");
    asm ("ldd     [%o0+72],%f18");
    asm ("ldd     [%o0+80],%f20");
    asm ("ldd     [%o0+88],%f22");
    asm ("ldd     [%o0+96],%f24");
    asm ("ldd     [%o0+104],%f26");
    asm ("ldd     [%o0+112],%f28");
    asm ("ldd     [%o0+120],%f30");
}



int callFunc(int x)
{
	return x ;
}

void DYNINSTreportCounter(intCounter *counter)
{
    traceSample sample;
    time64 process_time;
    time64 wall_time;

    wall_time = getWallTime();
    wall_time += startWall;
    wall_time /= NI_CLK_USEC;

    process_time = getProcessTime();
    process_time /= NI_CLK_USEC;


    sample.value = counter->value;
    sample.id = counter->id;
    DYNINSTtotalSamples++;
    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
				wall_time, process_time);
}


void DYNINSTreportBaseTramps()
{
    costUpdate sample;
    time64 currentCPU;
    time64 currentWall;
    time64 elapsedWallTime;
    static time64 elapsedPauseTime = 0;
    time64 wall_time,process_time;

    sample.slotsExecuted = 0;

    /*
    // Adding the cost corresponding to the alarm when it goes off.
    // This value includes the time spent inside the routine (DYNINSTtotal-
    // sampleTime) plus the time spent during the context switch (106 usecs
    // for monona, CM-5)
    */

    sample.obsCostIdeal  = ((((double) DYNINSTgetObservedCycles(1) *
                              (double)DYNINSTcyclesToUsec) + 
                             DYNINSTtotalSampleTime + 106) / 1000000.0);

    wall_time = getWallTime();
    currentWall =  wall_time/(time64)NI_CLK_USEC;
    wall_time += startWall;
    wall_time /= NI_CLK_USEC;

    process_time = getProcessTime();
    process_time /= (time64)NI_CLK_USEC;

    currentCPU = process_time;
    elapsedWallTime = currentWall - DYNINSTlastWallTime;
    elapsedPauseTime += elapsedWallTime - (currentCPU - DYNINSTlastCPUTime);
    sample.pauseTime = ((double) elapsedPauseTime);
    sample.pauseTime /= 1000000.0;
    DYNINSTlastWallTime = currentWall;
    DYNINSTlastCPUTime = currentCPU;

    DYNINSTgenerateTraceRecord(0, TR_COST_UPDATE, sizeof(sample), 
	&sample, 0,wall_time,process_time);
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
    time64 process_time;
    time64 wall_time;

    wall_time = getWallTime();
    wall_time += startWall;
    wall_time /= NI_CLK_USEC;
    process_time = getProcessTime();
    process_time /= NI_CLK_USEC;

    value = DYNINSTgetObservedCycles(1);

    cost = ((double) value) * (DYNINSTcyclesToUsec / 1000000.0);

    if (cost < prevCost) {
	fprintf(stderr, "Fatal Error Cost counter went backwards\n");
	fflush(stderr);
	abort();
    }

    prevCost = cost;

    /* temporary until CM-5 aggregation code can handle avg operator */
    if (DYNINSTnprocs) cost /= DYNINSTnprocs;

    sample.value = cost;
    sample.id = counter->id;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
				wall_time,process_time);

}

