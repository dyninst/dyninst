/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.19  1994/10/04 18:52:54  jcargill
 * Got rid of "previous" array; now uses the per-timer lastValue instead
 *
 * Revision 1.18  1994/08/02  18:18:52  hollings
 * added code to save/restore FP state on entry/exit to signal handle
 * (really jcargill, but commited by hollings).
 *
 * changed comparisons on time regression to use 64 bit int compares rather
 * than floats to prevent fp rounding error from causing false alarms.
 *
 * Revision 1.17  1994/07/28  23:22:55  hollings
 * clear g7 on init, and fixed DYNINSTcyclesToUsec.
 *
 * Revision 1.16  1994/07/26  20:06:12  hollings
 * fixed clock code.
 *
 * Revision 1.15  1994/07/22  19:24:51  hollings
 * added actual paused time for CM-5.
 *
 * Revision 1.14  1994/07/16  03:39:30  hollings
 * stopped using assembly version of clocks (temporary).
 *
 * Revision 1.13  1994/07/15  04:19:47  hollings
 * added code to report stats at the end.
 *
 * Revision 1.12  1994/07/14  23:35:08  hollings
 * Added extra arg to generateTrace, ifdefs out DYNINST{start,top}* to use
 * assembly versions.
 *
 * Revision 1.11  1994/07/11  22:47:47  jcargill
 * Major CM5 commit: include syntax changes, some timer changes, removal
 * of old aggregation code, old pause code, added signal-driven sampling
 * within node processes
 *
 * Revision 1.10  1993/12/14  17:27:21  jcargill
 * Put sampleMultiple and length alignment fixes back, and one retry fix
 *
 * Revision 1.9  1993/12/14  16:35:27  hollings
 * moved getProcessTime() out of the ifdef notdef.
 *
 * Revision 1.8  1993/12/13  19:47:06  hollings
 * use assembly version of clock code.
 *
 * Revision 1.7  1993/10/19  15:29:58  hollings
 * new simpler primitives.
 *
 * Revision 1.6  1993/10/07  19:09:12  jcargill
 * Added true combines for global instrumentation
 *
 * Revision 1.5  1993/10/01  18:15:53  hollings
 * Added filtering and resource discovery.
 *
 * Revision 1.4  1993/09/02  22:09:38  hollings
 * fixed race condition caused be no re-trying sampling of process time.
 *
 * Revision 1.3  1993/08/26  23:07:34  hollings
 * made initTraceLibPN called from DYNINSTinitTraceLib.
 *
 * Revision 1.2  1993/07/02  21:53:33  hollings
 * removed unnecessary include files
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <stdlib.h>
/* #include <signal.h> */
#include <assert.h>

/* our include files */
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
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


char *TRACELIBcurrPtr;		/* current pointer in buffer  */
char *TRACELIBfreePtr;		/* pointer to next free byte in buffer */
char *TRACELIBendPtr;		/* last byte in trace buffer */
char *TRACELIBtraceBuffer;	/* beginning of trace buffer */
int TRACELIBmustRetry;		/* signal variable from consumer -> producer */
extern float DYNINSTcyclesToUsec;

time64 getProcessTime();
time64 DYNINSTgetWallTime();
extern int DYNINSTtotalSamples;
extern time64 DYNINSTlastCPUTime;
extern time64 DYNINSTlastWallTime;


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

static volatile unsigned int *ni;
volatile struct timer_buf timerBuffer;

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
/*    int high; */
    time64 ret;
    timeParts end;
    time64 ni_end;

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

    return(ret);
}


time64 DYNINSTgetCPUtime()
{
     time64 now;

     now = getProcessTime();
     now /= NI_CLK_USEC;

     return(now);
}


inline time64 getWallTime()
{
    timeParts end;

retry:
    timerBuffer.sync = 1;
    end.parts.high = timerBuffer.high;
    end.parts.low = *ni;
    if (timerBuffer.sync != 1) goto retry;
    return(end.value);
}

inline time64 DYNINSTgetWallTime()
{
    time64 now;

    now = getWallTime();
    now /= NI_CLK_USEC;

    return(now);
}

void DYNINSTstartWallTimer(tTimer *timer)
{
    if (timer->counter == 0) {
	 timer->start = getWallTime();
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;
}

void DYNINSTstopWallTimer(tTimer *timer)
{
    time64 now;

    if (!timer->counter) return;

    if (timer->counter == 1) {
	 now = getWallTime();
	 timer->snapShot = timer->total + now - timer->start;
	 timer->mutex = 1;
	 timer->counter = 0;
	 timer->total = timer->snapShot;
	 timer->mutex = 0;
    } else {
	timer->counter--;
    }
}

void DYNINSTstartProcessTimer(tTimer *timer)
{
    if (timer->counter == 0) {
	 timer->start = getProcessTime();
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;
}


void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 end;
    time64 elapsed;
    tTimer timerTemp;

    if (!timer->counter) return;

    if (timer->counter == 1) {
retry:
	end = getProcessTime();
	elapsed = end - timer->start;
	timer->snapShot = elapsed + timer->total;
	timer->mutex = 1;
	timer->counter = 0;
	/* read proces time again in case the value was sampled between
	 *  last sample and mutex getting set.
	 */
	if (timer->sampled) {
	    timer->sampled = 0;
	    goto retry;
	}
	timer->total = timer->snapShot;
	timer->sampled = 0;
	timer->mutex = 0;

	/* for debugging */
	if (timer->total < 0) {
	    double st, el, en, to;

	    el = elapsed;
	    en = end;
	    st = timer->start;
	    to = timer->total;
	    printf("FATAL ERROR NEGATIVE TIME****\n");
	    printf("el = %f, en = %f, st = %f, to = %f\n", el, en,
		st, to);
	    fflush(stdout);
	    abort();
	}
    } else {
	timer->counter--;
    }
}

time64 previous[1000];

void set_timer_buf(volatile struct timer_buf *param)
{
/*     asm("set 50,%g1"); */
    asm("set 29,%g1");
    /* asm("set 23,%g1"); */
    asm("retl");
    asm("ta 0x8");
}



void DYNINSTreportTimer(tTimer *timer)
{
    double temp;
    double temp2;
    time64 now;
    time64 total;
    tTimer timerTemp;
    traceSample sample;


    if (timer->mutex) {
	total = timer->snapShot;
	timer->sampled = 1;
    } else if (timer->counter) {
	/* timer is running */
	if (timer->type == processTime) {
	    now = getProcessTime();
	    total = now - timer->start;
	} else {
	    CMOS_get_time(&now);
	    total = (now - timer->start);
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
	 printf("time regressed timer %d, total = %f, last = %f\n",
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

/*    timer->normalize = NI_CLK_USEC * MILLION; */
    sample.value = total / (double) timer->normalize;
    sample.id = timer->id;

    DYNINSTtotalSamples++;
    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, FALSE);
}



static time64 startWall;
int DYNINSTnoHandlers;
static int DYNINSTinitDone;
time64 DYNINSTelapsedTime;
tTimer DYNINSTelapsedCPUTime;

extern double DYNINSTsamplingRate;

#define NI_BASE       (0x20000000)
#define NI_TIME_A     	      (NI_BASE + 0x0070)

/*
 * should be called before main in each process.
 *
 */
void DYNINSTinit()
{
    char *interval;
    struct itimerval timeInterval;
    int sampleInterval;
    struct timeval tv;
    time64 startNItime;
    extern void DYNINSTalarmExpire();
    extern int DYNINSTsampleMultiple;

    asm("mov 0,	%g7");

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
    DYNINSTlastWallTime = DYNINSTgetWallTime();

    /*
     * Set up the SIGALRM handler stuff so counters/timers get sampled and
     * traces get puit into the traceBuffer every once in a while.
     */

    sampleInterval = 500000;     /* default is 500msec  */
    interval = (char *) getenv("DYNINSTsampleInterval");
    if (interval) {
	sampleInterval = atoi(interval);
    }
    DYNINSTsamplingRate = ((float) sampleInterval)/ 1000000.0;

    CMOS_signal (CM_SIGALRM, DYNINSTalarmExpire, ~0);
    timeInterval.it_interval.tv_sec = ((int) (sampleInterval) / 1000000);
    timeInterval.it_interval.tv_usec = sampleInterval % 1000000;
    CMOS_setitimer (1, &timeInterval, NULL);
    CMOS_ualarm (sampleInterval);

    DYNINSTcyclesToUsec = 1.0/((float) NI_CLK_USEC);

    CMOS_get_time(&DYNINSTelapsedTime);
    DYNINSTstartProcessTimer(&DYNINSTelapsedCPUTime);

    DYNINSTinitDone = 1;

}


void DYNINSTexit()
{
}


/*
 * generate a trace record onto the named stream.
 *
 */
void DYNINSTgenerateTraceRecord(traceStream sid, short type, short length,
    void *eventData, int flush)
{
    int count;
    time64 pTime;
    char buffer[1024];
    traceHeader header;

    if (!DYNINSTinitDone) return;

    CMOS_get_time(&header.wall);
    header.wall += startWall;
    header.wall /= NI_CLK_USEC;

    CMOS_get_times(&header.process, &pTime);
    header.process -= pTime;
    header.process /= NI_CLK_USEC;

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

void must_end_timeslice()
{
  printf ("Need to end timeslice!!!\n");
  /*
   * We still don't do anything good for this case.
   * Simple solutions are:
   * 1.  Increase buffer size
   * 2.  Implement a way for a node to trigger a context switch and
   * surrender the rest of a timeslice.
   */
}

extern int DYNINSTnumReported;
extern int DYNINSTtotalAlaramExpires;
extern time64 DYNINSTtotalSampleTime;

void DYNINSTprintCost()
{
    int64 value;
    time64 endWall;
    struct endStatsRec stats;
    extern int64 DYNINSTgetObservedCycles(Boolean);

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
    DYNINSTgenerateTraceRecord(0, TR_EXIT, sizeof(stats), &stats, 1);
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



