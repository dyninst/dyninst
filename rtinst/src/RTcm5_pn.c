/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.11  1994/07/11 22:47:47  jcargill
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

#ifdef notdef
time64 inline getProcessTime()
{
    time64 end;
    time64 ni_end;
    time64 ni2;

retry:
    CMOS_get_NI_time(&ni_end);
    CMOS_get_time((CM_TIME) &end);
    CMOS_get_NI_time(&ni2);
    if (ni_end != ni2) goto retry;
    return(end-ni_end);
}
#endif

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

time64 DYNINSTgetCPUtime()
{
     time64 now;

     now = getProcessTime();
     now /= NI_CLK_USEC * MILLION;

     return(now);
}


inline time64 getProcessTime()
{
    timeParts end;
    time64 ni_end;

retry:
    timerBuffer.sync = 1;
    ni_end = timerBuffer.ni_time;
    end.parts.high = timerBuffer.high;
    end.parts.low = *ni;
    if (timerBuffer.sync != 1) goto retry;
    return(end.value-ni_end);
}

/* #ifdef notdef */
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

#ifdef notdef
	/* for debugging */
	if (timer->total < 0) {
	    timerTemp = *timer;
	    abort();
	}
#endif

    } else {
	timer->counter--;
    }
}
/* #endif */

double previous[1000];

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

    if (total < 0) {
	timerTemp = *timer;
	abort();
    }

    timer->normalize = NI_CLK_USEC * MILLION;
    sample.value = total / (double) timer->normalize;
    sample.id = timer->id;

    temp = sample.value;
    if (temp < previous[sample.id.id]) {
	timerTemp = *timer;
	temp2 = previous[sample.id.id];
	abort();
	while(1);
    }
    previous[sample.id.id] = temp;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
}



static time64 startWall;
int DYNINSTnoHandlers;
static int DYNINSTinitDone;

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


    /*
     * Set up the SIGALRM handler stuff so counters/timers get sampled and
     * traces get puit into the traceBuffer every once in a while.
     */

    sampleInterval = 500000;     /* default is 500msec  */
    interval = (char *) getenv("DYNINSTsampleInterval");
    if (interval) {
	sampleInterval = atoi(interval);
    }
    CMOS_signal (CM_SIGALRM, DYNINSTalarmExpire, ~0);
    timeInterval.it_interval.tv_sec = ((int) (sampleInterval) / 1000000);
    timeInterval.it_interval.tv_usec = sampleInterval % 1000000;
    CMOS_setitimer (1, &timeInterval, NULL);
    CMOS_ualarm (sampleInterval);

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
    void *eventData)
{
    int ret;
    int count;
    time64 pTime;
    double newVal;
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

    memcpy(&buffer[count], eventData, length);
    count += length;

    TRACE(buffer, count);
}

void DYNINSTbreakPoint(int arg)
{
    printf("Break point %d reached\n", arg);
/*     asm("ta 0x81"); */
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
