/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a processor running UNIX.
 *
 * $Log: RTunix.c,v $
 * Revision 1.8  1994/04/07 01:21:30  markc
 * Cleaned up writes.  Writes interrupted by system calls get retried, others
 * do not.
 *
 * Revision 1.7  1994/03/25  16:03:11  markc
 * Added retry to write which could be interrupted by a signal.
 *
 * Revision 1.6  1994/02/16  00:07:24  hollings
 * Added a default sampling interval of 500msec.  Previous default was not
 * to collect any data.
 *
 * Revision 1.5  1994/02/02  00:46:14  hollings
 * Changes to make it compile with the new tree.
 *
 * Revision 1.4  1993/12/13  19:48:12  hollings
 * force records to be word aligned.
 *
 * Revision 1.3  1993/10/19  15:29:58  hollings
 * new simpler primitives.
 *
 * Revision 1.2  1993/08/26  19:43:17  hollings
 * added uarea mapping code.
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
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
#include <fcntl.h>
#include <assert.h>
#include <nlist.h>
#include <unistd.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#define MILLION	1000000
extern int DYNINSTmappedUarea;
extern int *_p_1, *_p_2;

/*
 * Missing stuff.
 *
 */
extern int getrusage(int who, struct rusage *rusage);

inline time64 DYNINSTgetUserTime()
{
    int first;
    time64 now;
    struct rusage ru;

    if (DYNINSTmappedUarea) {
retry:
	 first = *_p_1;
	 now = first;
	 now *= MILLION;
	 now += *_p_2;
	 if (*_p_1 != first) goto retry;
     } else {
	 getrusage(RUSAGE_SELF, &ru);
	 now = ru.ru_utime.tv_sec;
	 now *= MILLION;
	 now += ru.ru_utime.tv_usec;
     }
     return(now);
}

void DYNINSTbreakPoint()
{
    kill(getpid(), SIGSTOP);
}

void DYNINSTstartProcessTimer(tTimer *timer)
{
    time64 temp;

    if (timer->counter == 0) {
	 temp = DYNINSTgetUserTime();
	 timer->start = temp;
	 timer->normalize = MILLION;
    }
    /* this must be last to prevent race conditions */
    timer->counter++;
}

void DYNINSTstopProcessTimer(tTimer *timer)
{
    time64 now;
    struct rusage ru;

    /* don't stop a counter that is not running */
    if (!timer->counter) return;


    /* Warning - there is a window between setting now, and mutex that
	  can cause time to go backwards by the time to execute the
	  instructions between these two points.  This is not a cummlative error
	  and should not affect samples.  This was done (rather than re-sampling
	  now because the cost of computing now is so high).
    */
    if (timer->counter == 1) {
	now = DYNINSTgetUserTime();
	timer->snapShot = now - timer->start + timer->total;
	timer->mutex = 1;
	timer->counter = 0;
	timer->total = timer->snapShot;
	timer->mutex = 0;
	if (now < timer->start) {
	     getrusage(RUSAGE_SELF, &ru);
	     printf("end before start\n");
	     sigpause(0xffff);
	}
    } else {
	timer->counter--;
    }
}


void DYNINSTstartWallTimer(tTimer *timer)
{
    struct timeval tv;

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
    time64 now;
    struct timeval tv;

    /* don't stop a counter that is not running */
    if (!timer->counter) return;

    if (timer->counter == 1) {
	 gettimeofday(&tv, NULL);
	 now = tv.tv_sec;
	 now *= MILLION;
	 now += tv.tv_usec;
	 /* see note before StopProcess time for warning about this mutex */
	 timer->snapShot = timer->total + now - timer->start;
	 timer->mutex = 1;
	 timer->counter = 0;
	 timer->total = timer->snapShot;
	 timer->mutex = 0;
    } else {
	timer->counter--;
    }
}

time64 startWall;

volatile int DYNINSTpauseDone = 0;

/*
 * change the variable to let the process proceed.
 *
 */
void DYNINSTcontinueProcess()
{
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

    sigs = ((1 << (SIGUSR2-1)) | (1 << (SIGUSR1-1)) | (1 << (SIGTSTP-1)));
    mask = ~sigs;
    DYNINSTpauseDone = 0;
    while (!DYNINSTpauseDone) {
#ifdef notdef
       sigpause(mask);
       // temporary busy wait until we figure out what the TSD is up to. 
       printf("out of sigpuase\n");
#endif
    }
}

void DYNINSTinit(int skipBreakpoint)
{
    int val;
    int sigs;
    char *interval;
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


    /* default is 500msec */
    val = 500000;
    interval = (char *) getenv("DYNINSTsampleInterval");
    if (interval) {
	val = atoi(interval);
    }
    sigvec(SIGALRM, &alarmVector, NULL);
    ualarm(val, val);

    DYNINSTmappedUarea = DYNINSTmapUarea();

    /*
     * pause the process and wait for additional info.
     *
     */
    if (!skipBreakpoint) DYNINSTbreakPoint();
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
    char buffer[1024], *bufptr;
    traceHeader header;
    static Boolean pipeGone = False;

    if (pipeGone) return;

    gettimeofday(&tv, NULL);
    header.wall = tv.tv_sec;
    header.wall *= (time64) MILLION;
    header.wall += tv.tv_usec;
    header.wall -= startWall;

#ifdef notdef
    if (DYNINSTmappedUarea) {
	header.process = *_p_1;
	header.process *= MILLION;
	header.process += *_p_2;
    } else {
#endif
	getrusage(RUSAGE_SELF, &ru);
	header.process = ru.ru_utime.tv_sec;
	header.process *= (time64) MILLION;
	header.process += ru.ru_utime.tv_usec;
#ifdef notdef
    }
#endif

    /* round length off to a word aligned unit */
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

    /* on this platorm, we have a pipe to the controller process */
    errno = 0;
    /* write may be interrupted by a system call */
    bufptr = buffer;
    while (count) {
        ret = write(CONTROLLER_FD, bufptr, count);

	if (ret >= 0)
            ;     /* check most common case first */
	else if (errno != EINTR)
	    break; 
	else
	    ret = 0;

	count -= ret;
	bufptr += ret;
    }
    if (count) {
	extern char *sys_errlist[];
	(void) close(CONTROLLER_FD);
	printf("unable to write trace record %s\n", sys_errlist[errno]);
	printf("disabling further data logging, pid=%d\n", getpid());
	pipeGone = True;
    }
}

time64 lastValue[200];
double lastTime[200];

void DYNINSTreportTimer(tTimer *timer)
{
    time64 now;
    time64 total;
    struct rusage ru;
    struct timeval tv;
    traceSample sample;

    if (timer->mutex) {
	total = timer->snapShot;
    } else if (timer->counter) {
	/* timer is running */
	if (timer->type == processTime) {
	    getrusage(RUSAGE_SELF, &ru);
	    now = ru.ru_utime.tv_sec;
	    now *= (time64) MILLION;
	    now += ru.ru_utime.tv_usec;
	} else {
	    gettimeofday(&tv, NULL);
	    now = tv.tv_sec;
	    now *= MILLION;
	    now += tv.tv_usec;
	}
	total = now - timer->start + timer->total;
    } else {
	total = timer->total;
    }
    if (total < lastValue[timer->id.id]) {
	 printf("time regressed\n");
	 sigpause(0xffff);
    }
    lastValue[timer->id.id] = total;

    sample.id = timer->id;
    sample.value = ((double) total) / (double) timer->normalize;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample);
    /* printf("raw sample %d = %f\n", sample.id.id, sample.value); */
}

void DYNINSTfork(void *arg, int pid)
{
    int sid = 0;
    traceFork forkRec;

    printf("fork called with pid = %d\n", pid);
    if (pid > 0) {
	forkRec.ppid = getpid();
	forkRec.pid = pid;
	forkRec.npids = 1;
	forkRec.stride = 0;
	DYNINSTgenerateTraceRecord(sid,TR_FORK,sizeof(forkRec), &forkRec);
    } else {
	/* set up signals and stop at a break point */
	DYNINSTinit(1);
	sigpause();
    }
}
