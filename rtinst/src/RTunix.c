/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTunix.c,v $
 * Revision 1.1  1993/07/02 21:49:35  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include "rtinst.h"
#include "trace.h"

#define MILLION	1000000
static int DYNINSTinSample;

void DYNINSTbreakPoint()
{
    kill(getpid(), SIGSTOP);
}

void DYNINSTstartProcessTimer(tTimer *timer)
{
    time64 temp;
    struct rusage ru;

    if (DYNINSTinSample) abort();

    if (timer->trigger && (!timer->trigger->value)) return;
    if (timer->counter == 0) {
	 getrusage(RUSAGE_SELF, &ru);
	 temp = ru.ru_utime.tv_sec;
	 temp *= MILLION;
	 temp += ru.ru_utime.tv_usec;
	 timer->start = temp;
	 timer->normalize = MILLION;
    }
    /* this must be last to prevent race conditions */
    timer->counter++;
}

void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 end;
    time64 temp;
    struct rusage ru;

    if (timer->trigger && (timer->trigger->value <= 0)) return;

    /* don't stop a counter that is not running */
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 getrusage(RUSAGE_SELF, &ru);
	 temp = ru.ru_utime.tv_sec;
	 temp *= MILLION;
	 temp += ru.ru_utime.tv_usec;
	 end = temp;
	 timer->total += end - timer->start;
    }
    /* this must be last to prevent race conditions */
    timer->counter--;
}


void DYNINSTstartWallTimer(tTimer *timer)
{
    struct timeval tv;

    if (timer->trigger && (!timer->trigger->value)) return;
    if (timer->counter == 0) {
	 gettimeofday(&tv, NULL);
	 timer->start = tv.tv_sec;
	 timer->start *= (time64) MILLION;
	 timer->start += tv.tv_usec;
	 timer->normalize = MILLION;
    }
    /* this must be last to prevent race conditions */
    timer->counter++;
}

void DYNINSTstopWallTimer(tTimer *timer)
{
    time64 end;
    struct timeval tv;

    if (timer->trigger && (timer->trigger->value <= 0)) return;

    /* don't stop a counter that is not running */
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 gettimeofday(&tv, NULL);
	 end = tv.tv_sec;
	 end *= MILLION;
	 end += tv.tv_usec;
	 timer->total += end - timer->start;
    }
    /* this must be last to prevent race conditions */
    timer->counter--;
}

time64 startWall;

int DYNINSTpauseDone = 0;

/*
 * change the variable to let the process proceed.
 *
 */
void DYNINSTcontinueProcess()
{
    printf("in signal handler to continue process\n");
    DYNINSTpauseDone = 1;
}

/*
 * pause the process and let only USR2 signal handlers run until a SIGUSR1.
 *    arrives.
 *
 */
void DYNINSTpauseProcess()
{
    int mask;
    int sigs;

#ifdef notdef
    sigs = ((1 << (SIGUSR2-1)) | (1 << (SIGUSR1-1)) | (1 << (SIGTSTP-1)));
    mask = ~sigs;
#endif
    DYNINSTpauseDone = 0;
    printf("into DYNINSTpauseProcess\n");
    while (!DYNINSTpauseDone) {
#ifdef notdef
       // temporary bust wait until we figure out what the TSD is up to. 
       sigpause(mask);
       printf("out of sigpuase\n");
#endif
    }
}

void DYNINSTinit()
{
    int val;
    int sigs;
    char *interval;
    struct timeval tv;
    struct sigvec alarmVector;
    struct sigvec pauseVector;
    extern void DYNINSTsampleValues();

    startWall = 0;

    /*
     * Define the signal handlers for stopping a process.
     *
     */
    pauseVector.sv_handler = DYNINSTpauseProcess;
    sigs = ((1 << (SIGUSR2-1)) | (1 << (SIGUSR1-1)) | (1 << (SIGTSTP-1)));
    pauseVector.sv_mask = ~sigs;
    pauseVector.sv_flags = 0;
    sigvec(SIGPROF, &pauseVector, NULL);

    signal(SIGUSR1, DYNINSTcontinueProcess);

    /* define the alarm signal vector. We block all signals while sampling.  
     *  This prevents race conditions where signal handlers cause timers to 
     *  be started and stopped.
     */
    alarmVector.sv_handler = DYNINSTsampleValues;
    alarmVector.sv_mask = ~0;
    alarmVector.sv_flags = 0;


    if (interval = (char *) getenv("DYNINSTsampleInterval")) {
	sigvec(SIGALRM, &alarmVector, NULL);
	val = atoi(interval);
	ualarm(val, val);
    } 

    /*
     * pause the process and wait for additional info.
     *
     */
    DYNINSTbreakPoint();
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
    struct rusage ru;
    struct timeval tv;
    char buffer[1024];
    traceHeader header;
    static Boolean pipeGone = False;

    if (pipeGone) return;

    gettimeofday(&tv, NULL);
    header.wall = tv.tv_sec;
    header.wall *= (time64) MILLION;
    header.wall += tv.tv_usec;
    header.wall -= startWall;

    getrusage(RUSAGE_SELF, &ru);
    header.process = ru.ru_utime.tv_sec;
    header.process *= (time64) MILLION;
    header.process += ru.ru_utime.tv_usec;

    header.type = type;
    header.length = length;
    count = 0;
    memcpy(&buffer[count], &sid, sizeof(traceStream));
    count += sizeof(traceStream);

    memcpy(&buffer[count], &header, sizeof(header));
    count += sizeof(header);

    memcpy(&buffer[count], eventData, length);
    count += length;

    /* on this platorm, we have a pipe to the controller process */
    ret = write(CONTROLLER_FD, buffer, count);
    if (ret != count) {
	extern char *sys_errlist[];
	(void) close(CONTROLLER_FD);

	printf("unable to write trace record %s\n", sys_errlist[errno]);
	printf("disabling further data logging\n");
	pipeGone = True;
    }
}

time64 lastValue[200];
double lastTime[200];

void DYNINSTreportTimer(tTimer *timer)
{
    time64 now;
    double value;
    time64 total;
    struct rusage ru;
    struct timeval tv;
    traceSample sample;

    DYNINSTinSample = 1;
    total = timer->total;
    if (timer->counter) {
	/* timer is running */

	if (timer->type == processTime) {
	    getrusage(RUSAGE_SELF, &ru);
	    now = ru.ru_utime.tv_sec;
	    now *= (time64) MILLION;
	    now += ru.ru_utime.tv_usec;

	    /* for debugging */
	    value = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec/1000000.0;
	    if (value < lastTime[timer->id.id]) abort();
	    lastTime[timer->id.id] = value;
	} else {
	    gettimeofday(&tv, NULL);
	    now = tv.tv_sec;
	    now *= MILLION;
	    now += tv.tv_usec;
	}
	total += now - timer->start;
    }
    if (total < lastValue[timer->id.id]) abort();
    lastValue[timer->id.id] = total;

    sample.id = timer->id;
    sample.value = ((double) total) / (double) timer->normalize;

    DYNINSTinSample = 0;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
#ifdef notdef
    printf("raw sample %d = %f\n", sample.id.id, sample.value);
#endif
}
