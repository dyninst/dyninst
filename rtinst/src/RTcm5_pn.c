/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a TMC CM-5 machine.
 *
 *
 * $Log: RTcm5_pn.c,v $
 * Revision 1.2  1993/07/02 21:53:33  hollings
 * removed unnecessary include files
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <signal.h>
#include <assert.h>
#include "rtinst.h"
#include "trace.h"
#include "traceio.h"

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
    time64 end;

    if (timer->trigger && (timer->trigger->value <= 0)) return;
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 CMOS_get_time(&end);
	 timer->total += (end - timer->start);
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter--;
}


void DYNINSTstartProcessTimer(tTimer *timer)
{
    if (timer->trigger && (!timer->trigger->value)) return;
    if (timer->counter == 0) {
	 CMOS_get_time(&timer->start);
	 CMOS_get_NI_time(&timer->ni_start);
	 timer->normalize = NI_CLK_USEC * MILLION;
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter++;
}

void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 end;
    time64 ni_end;

    if (timer->trigger && (timer->trigger->value <= 0)) return;
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 CMOS_get_time(&end);
	 CMOS_get_NI_time(&ni_end);
	 timer->total += (end - timer->start);
	 timer->total -= (ni_end - timer->ni_start);
	 if (timer->total < 0) abort();
    }
    /* this must be last to prevent race conditions with the sampler */
    timer->counter--;
}

void DYNINSTreportTimer(tTimer *timer)
{
    time64 now;
    double value;
    time64 total;
    time64 ni_now;
    traceSample sample;


    total = timer->total;
    if (timer->counter) {
	/* timer is running */

	CMOS_get_time(&now);
	CMOS_get_NI_time(&ni_now);
	if (timer->type == processTime) {
	    total += (now - ni_now) - (timer->start - timer->ni_start);
	} else {
	    total += (now - timer->start);
	}
    }
    if (total < 0) {
	double w1, w2, ni1, ni2;

	w1 = timer->start;
	w2 = now;
	ni1 = timer->ni_start;
	ni2 = ni_now;
	abort();
    }

    sample.value = total / (double) timer->normalize;
    sample.id = timer->id;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
}

time64 startWall;
int DYNINSTnoHandlers;

/*
 * should be called before main in each process1.
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

    initTraceLibPN();
}

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
