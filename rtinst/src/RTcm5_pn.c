/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.4  1993/09/02 22:09:38  hollings
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
#include <cm/cmna.h>

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

void DYNINSTstartWallTimer(tTimer *timer)
{
    if (timer->trigger && (!timer->trigger->value)) return;
    if (timer->counter == 0) {
	 CMOS_get_time(&timer->start);
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;
}

void DYNINSTstopWallTimer(tTimer *timer)
{
    time64 now;

    if (timer->trigger && (timer->trigger->value <= 0)) return;
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 CMOS_get_time(&now);
	 timer->snapShot = timer->total + now - timer->start;
	 timer->mutex = 1;
	 timer->counter = 0;
	 timer->total = timer->snapShot;
	 timer->mutex = 0;
    } else {
	timer->counter--;
    }
}

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


void DYNINSTstartProcessTimer(tTimer *timer)
{
    if (timer->trigger && (!timer->trigger->value)) return;
    if (timer->counter == 0) {
	 timer->start = getProcessTime();
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;
}

double previous[1000];

void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 end;
    time64 elapsed;
    tTimer timerTemp;

    if (timer->trigger && (timer->trigger->value <= 0)) return;
    if (!timer->counter) return;

    if (timer->counter == 1) {
	end = getProcessTime();
	elapsed = end - timer->start;
	timer->snapShot = elapsed + timer->total;
	timer->mutex = 1;
	timer->counter = 0;
	/* read proces time again in case the value was sampled between
	 *  last sample and mutex getting set.
	 */
	timer->total += getProcessTime() - timer->start;
	timer->mutex = 0;

	/* for debugging */
	if (timer->total < 0) {
	    timerTemp = *timer;
	    abort();
	}

    } else {
	timer->counter--;
    }
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

    CMOS_get_time(&startNItime);
    gettimeofday(&tv, NULL);

    startWall = tv.tv_sec;
    startWall *= MILLION;
    startWall += tv.tv_usec;

    /* change time base to ni time */
    startWall *= NI_CLK_USEC;

    startWall -= startNItime;

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

    /* check and see if we should aggregate to other nodes */
    if ((type == TR_SAMPLE) && (((traceSample*) eventData)->id.aggregate)) {
	 /* not ready yet! */
	 abort();

	 /* use reduction net to compute aggregate */
	 sample = (traceSample*) eventData;
	 /* newVal = CMMD_reduce_float(sample->value, CMMD_combiner_fadd); */
	 newVal = CMCN_reduce_float(sample->value, CMMD_combiner_fadd);
	 sample->value = newVal;


	 /* only node zero reports value */
	 if (CMMD_self_address()) return;
    }

    CMOS_get_time(&header.wall);
    header.wall += startWall;
    header.wall /= NI_CLK_USEC;

    CMOS_get_time(&header.process);
    CMOS_get_NI_time(&pTime);
    header.process -= pTime;
    header.process /= NI_CLK_USEC;

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
