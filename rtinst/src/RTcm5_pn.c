/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.10  1993/12/14 17:27:21  jcargill
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
#include <signal.h>
#include <assert.h>

/* our include files */
#include <h/rtinst.h>
#include <h/trace.h>
#include <rtinst/traceio.h>

#include <cm/cmmd/amx.h>
#include <cm/cmmd/mp.h>
#include <cm/cmmd/cn.h>
#include <cm/cmmd/io.h>
#include <cm/cmmd/util.h>
#include <cm/cmmd/cmmd_constants.h>
#include <cm/cmmd.h>
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


#ifdef notdef
time64 inline getProcessTime()
{
    time64 end;
    time64 ni_end;
    time64 ni2;

retry:
    CMOS_get_NI_time(&ni_end);
    CMOS_get_time(&end);
    CMOS_get_NI_time(&ni2);
    if (ni_end != ni2) goto retry;
    return(end-ni_end);
}
#endif

struct timer_buf {
    unsigned int high;
    unsigned int sync;
    time64   ni_time;
};

typedef union {
    struct {
	unsigned int high;
	unsigned int low;
    } parts;
    time64 value;
} timeParts;

static volatile unsigned int *ni;
static volatile struct timer_buf timerBuffer;

time64 inline getProcessTime()
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

#ifdef notdef
time64 inline getWallTime()
{
    timeParts end;
    time64 ni_end;

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
#endif

double previous[1000];

void set_timer_buf(struct timer_buf *param)
{
    asm("set 50,%g1");
    /* asm("set 23,%g1"); */
    asm("retl");
    asm("ta 0x8");
}

void DYNINSTreportAggregateCounter(intCounter *counter)
{
    traceSample sample;
    int aggregate;

    /* Everyone aggregates to a single value */
    CMNA_com(ADD_SCAN, SCAN_REDUCE, &counter->value, 1, &aggregate);

    sample.value = aggregate;
    sample.id = counter->id;

    if (CMNA_self_address == 0)
	DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
}


void DYNINSTreportTimer(tTimer *timer)
{
    double temp;
    double temp2;
    time64 now;
    double value;
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


Add_combine_64bit (x, aggregate)
    time64 *x, *aggregate;
{
    unsigned int *x_lsw_p;
    unsigned int *x_msw_p;
    unsigned int *agg_lsw_p;
    unsigned int *agg_msw_p;

   x_lsw_p = (((unsigned int *) x) + 1);
    x_msw_p = ((unsigned int *) x);
    agg_lsw_p = (((unsigned int *) aggregate) + 1);
    agg_msw_p = ((unsigned int *) aggregate);

    do {
	CMNA_com_send_first (UADD_SCAN, SCAN_REDUCE, 2, *x_lsw_p);
	CMNA_com_send_word (*x_msw_p);
    }
    while (!SEND_OK(CMNA_com_status()));

    while (!(RECEIVE_OK(CMNA_com_status())))
	continue;
/*     recv_length = RECEIVE_LENGTH_LEFT(CMNA_com_status()); */
    
    *agg_lsw_p = CMNA_com_receive_word();
    *agg_msw_p = CMNA_com_receive_word();
}


void DYNINSTreportAggregateTimer(tTimer *timer)
{
    double temp;
    double temp2;
    time64 now;
    double value;
    time64 total;
    time64 aggregate;
    tTimer timerTemp;
    traceSample sample;


    if (timer->mutex) {
	total = timer->snapShot;
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

    /* Combine all timers to get a single aggregate 64-bit timer */
    Add_combine_64bit (&total, &aggregate);

    sample.value = aggregate / (double) timer->normalize;
    sample.id = timer->id;

    /* only node 0 does sanity check and reports the timer */
    if (CMNA_self_address == 0) {
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

    ni = (unsigned int *) NI_TIME_A;
    set_timer_buf(&timerBuffer);

    if (getenv("DYNINSTsampleMultiple")) {
	DYNINSTsampleMultiple = atoi(getenv("DYNINSTsampleMultiple"));
    }

    DYNINSTinitDone = 1;
/*     initTraceLibPN(); */
}

/*
 * DYNINSTinitTraceLib - call initTraceLibPN & trap back.
 *
 */
asm(".global _DYNINSTinitTraceLib");
asm("_DYNINSTinitTraceLib:");
asm("	call	_initTraceLibPN");
asm("	nop	");
asm("	ta 0x1");
asm("	nop	");

void DYNINSTexit()
{
    cleanupTraceLibPN();
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
    traceSample *sample;
    traceHeader header;

    if (!DYNINSTinitDone) return;

    CMOS_get_time(&header.wall);
    header.wall += startWall;
    header.wall /= NI_CLK_USEC;

    CMOS_get_time(&header.process);
    CMOS_get_NI_time(&pTime);
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
    /* printf("Break point %d reached\n", arg); */
    asm("ta 0x81");
}
