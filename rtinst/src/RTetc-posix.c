/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */





/************************************************************************
 * RTposix.c: runtime instrumentation functions for generic posix.
 *
 * RTposix.c,v
 * Revision 1.10  1995/05/18  11:08:27  markc
 * added guard prevent timer start-stop during alarm handler
 * added version number
 *
 * Revision 1.9  1995/03/10  19:39:42  hollings
 * Fixed an ugly race condition in observed cost.
 *
 * Added the use of unix profil to compute the time spent in inst code.
 *
 *
 ************************************************************************/






/************************************************************************
 * header files.
************************************************************************/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <math.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/sys.h"



/************************************************************************
 * Per module flags
************************************************************************/
static int          DYNINSTin_sample = 0;

/************************************************************************
 * symbolic constants.
************************************************************************/

static const double MILLION = 1000000.0;

#ifdef USE_PROF
int DYNINSTbufsiz;
int DYNINSTprofile;
int DYNINSTprofScale;
int DYNINSTtoAddr;
short *DYNINSTprofBuffer;
#endif




/************************************************************************
 * external functions.
************************************************************************/

extern void   DYNINSTos_init(void);
extern time64 DYNINSTgetCPUtime(void);
extern time64 DYNINSTgetWalltime(void);





/************************************************************************
 * time64 DYNINSTgetUserTime(void)
 *
 * get the user time for "an" LWP of the monitored process.
************************************************************************/

time64
DYNINSTgetUserTime(void) {
    return DYNINSTgetCPUtime();
}





/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void
DYNINSTbreakPoint(void) {
    kill(getpid(), SIGSTOP);
}



/************************************************************************
 * void DYNINSTstartProcessTimer(tTimer* timer)
************************************************************************/

#ifdef COSTTEST
time64 DYNINSTtest[10]={0,0,0,0,0,0,0,0,0,0};
int DYNINSTtestN[10]={0,0,0,0,0,0,0,0,0,0};
#endif

void
DYNINSTstartProcessTimer(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT,endT;
#endif
   
    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
        timer->start     = DYNINSTgetCPUtime();
        timer->normalize = MILLION;
    }
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[0]+=endT-startT;
    DYNINSTtestN[0]++;
#endif
}





/************************************************************************
 * void DYNINSTstopProcessTimer(tTimer* timer)
************************************************************************/

void
DYNINSTstopProcessTimer(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT,endT;
#endif
   
    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (!timer->counter) {
        return;
    }

    if (timer->counter == 1) {
        time64 now = DYNINSTgetCPUtime();

        timer->snapShot = now - timer->start + timer->total;
        timer->mutex    = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
        timer->total = DYNINSTgetCPUtime() - timer->start + timer->total; 
        timer->counter = 0;
        timer->mutex = 0;

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("process timer rollback\n"); fflush(stdout);
            abort();
        }
    }
    else {
      timer->counter--;
    }

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[1]+=endT-startT;
    DYNINSTtestN[1]++;
#endif 
}





/************************************************************************
 * void DYNINSTstartWallTimer(tTimer* timer)
************************************************************************/

void
DYNINSTstartWallTimer(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (timer->counter == 0) {
        timer->start     = DYNINSTgetWalltime();
        timer->normalize = MILLION;
    }
    timer->counter++;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[2]+=endT-startT;
    DYNINSTtestN[2]++;
#endif 
}





/************************************************************************
 * void DYNINSTstopWallTimer(tTimer* timer)
************************************************************************/

void
DYNINSTstopWallTimer(tTimer* timer) {
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) return;       

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    if (!timer->counter) {
        return;
    }

    if (timer->counter == 1) {
        time64 now = DYNINSTgetWalltime();

        timer->snapShot = now - timer->start + timer->total;
        timer->mutex    = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
         */
        timer->total    = DYNINSTgetWalltime() - timer->start + timer->total;
        timer->counter  = 0;
        timer->mutex    = 0;

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("wall timer rollback\n"); 
            fflush(stdout);
            abort();
        }
    }
    else {
        timer->counter--;
    }
#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[3]+=endT-startT;
    DYNINSTtestN[3]++;
#endif 
}





/************************************************************************
 * void DYNINSTpauseProcess(void)
 * void DYNINSTcontinueProcess(void)
 *
 * pause and continue process.
************************************************************************/

static volatile int DYNINSTpauseDone = 0;

void
DYNINSTpauseProcess(void) {
    DYNINSTpauseDone = 0;
    while (!DYNINSTpauseDone) {
    }
}

void
DYNINSTcontinueProcess(void) {
    DYNINSTpauseDone = 1;
}





/************************************************************************
 * void DYNINST_install_ualarm(unsigned value, unsigned interval)
 *
 * an implementation of "ualarm" using the "setitimer" syscall.
************************************************************************/

static void
DYNINST_install_ualarm(unsigned value, unsigned interval) {
    struct itimerval it;

    it.it_value.tv_sec     = value    / 1000000;
    it.it_value.tv_usec    = value    % 1000000;
    it.it_interval.tv_sec  = interval / 1000000;
    it.it_interval.tv_usec = interval % 1000000;

    if (setitimer(ITIMER_REAL, &it, 0) == -1) {
        perror("setitimer");
        abort();
    }
}





/************************************************************************
 * global data for DYNINST functions.
************************************************************************/

double DYNINSTdata[SYN_INST_BUF_SIZE/sizeof(double)];
double DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(double)];





/************************************************************************
 * float DYNINSTcyclesPerSecond(void)
 *
 * need a well-defined method for finding the CPU cycle speed
 * on each CPU.
************************************************************************/

#define NOPS_4  asm("nop"); asm("nop"); asm("nop"); asm("nop")
#define NOPS_16 NOPS_4; NOPS_4; NOPS_4; NOPS_4

static float
DYNINSTcyclesPerSecond(void) {
    int            i;
    time64         start_cpu;
    time64         end_cpu;
    double         elapsed;
    double         speed;
    const unsigned LOOP_LIMIT = 500000;

    start_cpu = DYNINSTgetCPUtime();
    for (i = 0; i < LOOP_LIMIT; i++) {
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    end_cpu = DYNINSTgetCPUtime();
    elapsed = (double) (end_cpu - start_cpu);
    speed   = (double) (MILLION*256*LOOP_LIMIT)/elapsed;

    return speed;
}





/************************************************************************
 * void saveFPUstate(float* base)
 * void restoreFPUstate(float* base)
 *
 * save and restore state of FPU on signals.  these are null functions
 * for most well designed and implemented systems.
************************************************************************/

static void
saveFPUstate(float* base) {
}

static void
restoreFPUstate(float* base) {
}



/* The current observed cost since the last call to 
 *      DYNINSTgetObservedCycles(false) 
 */

/* This could be static, but gdb has can't find them if they are.  jkh 5/8/95 */
int64    DYNINSTvalue = 0;
unsigned DYNINSTlastLow;

unsigned DYNINSTobsCostLow;

/************************************************************************
 * int64 DYNINSTgetObservedCycles(RT_Boolean in_signal)
 *
 * report the observed cost of instrumentation in machine cycles.
 *
 * We keep cost as a 64 bit int, but the code generated by dyninst is
 *   a 32 bit counter (for speed).  So this function also converts the
 *   cost into a 64 bit value.
 *
 * We can't reset the low part of the counter since there might be an
 *   update of the counter going on while we are checking it.  So instead,
 *   we keep track of the last value of the low counter we saw, and update
 *   the high counter by the diference.  We also need to check that the
 *   delta is not negative (which happens when the low counter roles over).
 ************************************************************************/
int64 DYNINSTgetObservedCycles(RT_Boolean in_signal) 
{

    if (in_signal) {
        return DYNINSTvalue;
    }

    /* update the 64 bit version of the counter */
    if (DYNINSTobsCostLow < DYNINSTlastLow) {
        /* counter wrap around */
        /* note the calc here assume 32 bit int, but if ints are 64bit, then
           it won't wrap in the first place */
        DYNINSTvalue += (((unsigned) 0xffffffff) - DYNINSTlastLow) + 
 	    DYNINSTobsCostLow + 1;
    } else {
        DYNINSTvalue += (DYNINSTobsCostLow - DYNINSTlastLow);
    }

    DYNINSTlastLow = DYNINSTobsCostLow;
    return DYNINSTvalue;
}





/************************************************************************
 * void DYNINSTsampleValues(void)
 *
 * dummy function for sampling timers and counters.  the actual code
 * is added by dynamic instrumentation from the paradyn daemons.
************************************************************************/

static int DYNINSTnumReported = 0;

void
DYNINSTsampleValues(void) {
    DYNINSTnumReported++;
}





/************************************************************************
 * void DYNINSTflushTrace(void)
 *
 * flush any accumalated traces.
************************************************************************/

static FILE* DYNINSTtraceFp = 0;

static void
DYNINSTflushTrace(void) {
    if (DYNINSTtraceFp) fflush(DYNINSTtraceFp);
}





/************************************************************************
 * void DYNINSTgenerateTraceRecord(traceStream sid, short type,
 *                                 short length, void* data, int flush)
************************************************************************/

static time64 startWall = 0;

void
DYNINSTgenerateTraceRecord(traceStream sid, short type, short length,
    void *eventData, int flush) {
    int             ret;
    static unsigned pipe_gone = 0;
    traceHeader     header;
    int             count;
    char            buffer[1024];

    if (pipe_gone) {
        return;
    }

    header.wall    = DYNINSTgetWalltime() - startWall;
    header.process = DYNINSTgetCPUtime();

    length = ALIGN_TO_WORDSIZE(length);

    header.type   = type;
    header.length = length;

    count = 0;
    memcpy(&buffer[count], &sid, sizeof(traceStream));
    count += sizeof(traceStream);

    memcpy(&buffer[count], &header, sizeof(header));
    count += sizeof(header);

    count = ALIGN_TO_WORDSIZE(count);
    memcpy(&buffer[count], eventData, length);
    count += length;

    errno = 0;

    if (!DYNINSTtraceFp || (type == TR_EXIT)) {
        DYNINSTtraceFp = fdopen(dup(CONTROLLER_FD), "w");
    }

    ret = fwrite(buffer, count, 1, DYNINSTtraceFp);
    if (ret != 1) {
        printf("unable to write trace record, errno=%d\n", errno);
        printf("disabling further data logging, pid=%d\n", (int) getpid());
        fflush(stdout);
        pipe_gone = 1;
    }
    if (flush) DYNINSTflushTrace();
}





/************************************************************************
 * void DYNINSTreportBaseTramps(void)
 *
 * report the cost of base trampolines.
************************************************************************/

static float DYNINSTcyclesToUsec  = 0;
static time64 DYNINSTlastWallTime = 0;
static time64 DYNINSTlastCPUTime  = 0;
static time64 DYNINSTtotalSampleTime = 0;

void
DYNINSTreportBaseTramps() {
    costUpdate sample;
    time64     currentCPU;
    time64     currentWall;
    time64     elapsedWallTime;
    time64     currentPauseTime;

    sample.slotsExecuted = 0;

    //
    // Adding the cost corresponding to the alarm when it goes off.
    // This value includes the time spent inside the routine (DYNINSTtotal-
    // sampleTime) plus the time spent during the context switch (121 usecs
    // for SS-10, sunos)
    //

    sample.obsCostIdeal  = ((((double) DYNINSTgetObservedCycles(1) *
                              (double)DYNINSTcyclesToUsec) + 
                             DYNINSTtotalSampleTime + 121) / 1000000.0);

    currentCPU       = DYNINSTgetCPUtime();
    currentWall      = DYNINSTgetWalltime();
    elapsedWallTime  = currentWall - DYNINSTlastWallTime;
    currentPauseTime = elapsedWallTime - (currentCPU - DYNINSTlastCPUTime);

    sample.pauseTime  = ((double) currentPauseTime);
    sample.pauseTime /= 1000000.0;

    DYNINSTlastWallTime = currentWall;
    DYNINSTlastCPUTime  = currentCPU;

#ifdef notdef
    if (DYNINSTprofile) {
	int i;
	int limit;
	int pageSize;
	int startInst;
	extern void DYNINSTfirst();


	limit = DYNINSTbufsiz;
	/* first inst code - assumes data area above code space in virtual
	 * address */
	startInst = (int) &DYNINSTfirst;
	for (i=0; i < limit; i ++) {
	    if (i * DYNINSTtoAddr > startInst) {
		instTicks += DYNINSTprofBuffer[i];
	    }
	}

	sample.obsCostLow  = ((double) instTicks ) /100.0;
	sample.obsCostHigh = sample.obsCostLow + sample.obsCostIdeal;
    }
#endif

    DYNINSTgenerateTraceRecord(0, TR_COST_UPDATE, sizeof(sample), &sample, 0);
}





/************************************************************************
 * void DYNINSTalarmExpire(void)
 *
 * called periodically by signal handlers.  report sampled data back
 * to the paradyn daemons.  when the program exits, DYNINSTsampleValues
 * should be called directly.
************************************************************************/

#define N_FP_REGS 33

volatile int DYNINSTsampleMultiple    = 1;
static int          DYNINSTnumSampled        = 0;
static int          DYNINSTtotalAlarmExpires = 0;

void
DYNINSTalarmExpire(int signo) {
    time64     start_cpu;
    time64     end_cpu;
    float      fp_context[N_FP_REGS];

#ifdef COSTTEST
    time64 startT, endT;
#endif

    if (DYNINSTin_sample) {
        return;
    }
    DYNINSTin_sample = 1;

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

    DYNINSTtotalAlarmExpires++;
    if ((++DYNINSTnumSampled % DYNINSTsampleMultiple) == 0) {

        saveFPUstate(fp_context);
        start_cpu = DYNINSTgetCPUtime();

        /* to keep observed cost accurate due to 32-cycle rollover */
        (void) DYNINSTgetObservedCycles(0);

        DYNINSTsampleValues();
        DYNINSTreportBaseTramps();
        DYNINSTflushTrace();

        end_cpu = DYNINSTgetCPUtime();
        DYNINSTtotalSampleTime += (end_cpu - start_cpu);
        restoreFPUstate(fp_context);
    }

    DYNINSTin_sample = 0;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[4]+=endT-startT;
    DYNINSTtestN[4]++;
#endif 
}





/************************************************************************
 * void DYNINSTinit(int doskip)
 *
 * initialize the DYNINST library.  this function is called at the start
 * of the application program.
 *
 * the first this to do is to call the os specific initialization
 * function.
************************************************************************/

static float  DYNINSTsamplingRate   = 0;
static int    DYNINSTtotalSamples   = 0;
static tTimer DYNINSTelapsedCPUTime;
static tTimer DYNINSTelapsedTime;
static int DYNINSTtagCount = 0;
static int DYNINSTtagLimit = 100;
static int DYNINSTtags[1000];

void
DYNINSTinit(int doskip) {
    struct sigaction act;
    unsigned         val;
    char *temp;
#ifdef n_def
    const char*      interval;
#endif

    DYNINSTos_init();

    startWall = 0;

    DYNINSTcyclesToUsec = MILLION/DYNINSTcyclesPerSecond();
    DYNINSTlastCPUTime  = DYNINSTgetCPUtime();
    DYNINSTlastWallTime = DYNINSTgetWalltime();

    act.sa_handler = DYNINSTalarmExpire;
    act.sa_flags   = 0;

    /* for AIX - default (non BSD) library does not restart - jkh 7/26/95 */
#if defined(SA_RESTART)
    act.sa_flags  |= SA_RESTART;
#endif

    sigfillset(&act.sa_mask);

    if (sigaction(SIGALRM, &act, 0) == -1) {
        perror("sigaction(SIGALRM)");
        abort();
    }

#ifdef n_def
    val = 500000;
    interval = getenv("DYNINSTsampleInterval");
    if (interval) {
        val = atoi(interval);
    }
#endif

    /* assign sampling rate to be default value in util/h/sys.h */
    val = BASESAMPLEINTERVAL;

    DYNINSTsamplingRate = val/MILLION;

    DYNINST_install_ualarm(val, val);

    /* printf("Time at main %g us\n", (double) DYNINSTgetCPUtime()); */
    if (!doskip) {
        /* DYNINSTbreakPoint(); */
    }

    temp = getenv("DYNINSTtagLimit");
    if (temp) {
	int newVal;
	newVal = atoi(temp);
	if (newVal < DYNINSTtagLimit) {
	    DYNINSTtagLimit = newVal;
	}
    }

#ifdef USE_PROF
    {	extern int end;

	DYNINSTprofScale = sizeof(short);
	DYNINSTtoAddr = sizeof(short);
	DYNINSTbufsiz = (((unsigned int) &end)/DYNINSTprofScale) + 1;
	DYNINSTprofBuffer = (short *) calloc(sizeof(short), DYNINSTbufsiz);
	profil(DYNINSTprofBuffer, DYNINSTbufsiz*sizeof(short), 0, 0xffff);
	DYNINSTprofile = 1;
    }
#endif

    DYNINSTstartWallTimer(&DYNINSTelapsedTime);
    DYNINSTstartProcessTimer(&DYNINSTelapsedCPUTime);
}





/************************************************************************
 * void DYNINSTexit(void)
 *
 * handle `exit' in the application. current nothing is done.
************************************************************************/

void
DYNINSTexit(void) {
}





/************************************************************************
 * void DYNINSTreportTimer(tTimer* timer)
 *
 * report the timer `timer' to the paradyn daemon.
************************************************************************/

void
DYNINSTreportTimer(tTimer *timer) {
    time64 now = 0;
    time64 total;
    traceSample sample;

#ifdef COSTTEST
    time64 startT, endT;
    startT = DYNINSTgetCPUtime();
#endif

    if (timer->mutex) {
        total = timer->snapShot;
    }
    else if (timer->counter) {
        /* timer is running */
        if (timer->type == processTime) {
            now = DYNINSTgetCPUtime();
        } else {
            now = DYNINSTgetWalltime();
        }
        total = now - timer->start + timer->total;
    }
    else {
        total = timer->total;
    }

    if (total < timer->lastValue) {
        if (timer->type == processTime) {
            printf("process ");
        }
        else {
            printf("wall ");
        }
        printf("time regressed timer %d, total = %f, last = %f\n",
            timer->id.id, (float) total, (float) timer->lastValue);
        if (timer->counter) {
            printf("timer was active\n");
        } else {
            printf("timer was inactive\n");
        }
        printf("mutex=%d, counter=%d, sampled=%d, snapShot=%f\n",
            (int) timer->mutex, (int) timer->counter, (int) timer->sampled,
            (double) timer->snapShot);
        printf("now = %f, start = %f, total = %f\n",
            (double) now, (double) timer->start, (double) timer->total);
        fflush(stdout);
        abort();
    }

    timer->lastValue = total;

    sample.id = timer->id;
    sample.value = ((double) total) / (double) timer->normalize;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0);
    /* printf("raw sample %d = %f\n", sample.id.id, sample.value); */
#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[5]+=endT-startT;
    DYNINSTtestN[5]++;
#endif 
}





/************************************************************************
 * void DYNINSTfork(void* arg, int pid)
 *
 * track a fork() system call, and report to the paradyn daemon.
************************************************************************/

void
DYNINSTfork(void* arg, int pid) {
    int sid = 0;
    traceFork forkRec;

    printf("fork called with pid = %d\n", pid);
    fflush(stdout);
    if (pid > 0) {
        forkRec.ppid   = getpid();
        forkRec.pid    = pid;
        forkRec.npids  = 1;
        forkRec.stride = 0;
        DYNINSTgenerateTraceRecord(sid,TR_FORK,sizeof(forkRec), &forkRec, 1);
    } else {
        DYNINSTinit(1);
    }
}





/************************************************************************
 * void DYNINSTprintCost(void)
 *
 * print a detailed summary of the cost of the application's run.
************************************************************************/

void
DYNINSTprintCost(void) {
    FILE *fp;
    time64 now;
    int64 value;
    struct endStatsRec stats;

    DYNINSTstopProcessTimer(&DYNINSTelapsedCPUTime);
    DYNINSTstopWallTimer(&DYNINSTelapsedTime);

    value = DYNINSTgetObservedCycles(0);
    stats.instCycles = value;

    value *= DYNINSTcyclesToUsec;

    stats.alarms      = DYNINSTtotalAlarmExpires;
    stats.numReported = DYNINSTnumReported;
    stats.instTime    = (double)value/(double)MILLION;
    stats.handlerCost = (double)DYNINSTtotalSampleTime/(double)MILLION;

    now = DYNINSTgetCPUtime();
    stats.totalCpuTime  = (double)DYNINSTelapsedCPUTime.total/(double)MILLION;
    stats.totalWallTime = (double)DYNINSTelapsedTime.total/(double)MILLION;

    stats.samplesReported = DYNINSTtotalSamples;
    stats.samplingRate    = DYNINSTsamplingRate;

    stats.userTicks = 0;
    stats.instTicks = 0;

    fp = fopen("stats.out", "w");

#ifdef USE_PROF
    if (DYNINSTprofile) {
	int i;
	int limit;
	int pageSize;
	int startInst;
	extern void DYNINSTfirst();


	limit = DYNINSTbufsiz;
	/* first inst code - assumes data area above code space in virtual
	 * address */
	startInst = (int) &DYNINSTfirst;
	fprintf(fp, "startInst = %x\n", startInst);
	fprintf(fp, "limit = %x\n", limit * DYNINSTprofScale);
	for (i=0; i < limit; i++) {
#ifdef notdef
	    if (DYNINSTprofBuffer[i]) {
		fprintf(fp, "%x %d\n", i * DYNINSTtoAddr, 
			DYNINSTprofBuffer[i]);
	    }
#endif
	    if (i * DYNINSTtoAddr > startInst) {
		stats.instTicks += DYNINSTprofBuffer[i];
	    } else {
		stats.userTicks += DYNINSTprofBuffer[i];
	    }
	}

	/* fwrite(DYNINSTprofBuffer, DYNINSTbufsiz, 1, fp); */
	fprintf(fp, "stats.instTicks %d\n", stats.instTicks);
	fprintf(fp, "stats.userTicks %d\n", stats.userTicks);
    }
#endif

#ifdef notdef
    fprintf(fp, "DYNINSTtotalAlarmExpires %d\n", stats.alarms);
    fprintf(fp, "DYNINSTnumReported %d\n", stats.numReported);
    fprintf(fp,"Raw cycle count = %f\n", (double) stats.instCycles);
    fprintf(fp,"Total instrumentation cost = %f\n", stats.instTime);
    fprintf(fp,"Total handler cost = %f\n", stats.handlerCost);
    fprintf(fp,"Total cpu time of program %f\n", stats.totalCpuTime);
    fprintf(fp,"Elapsed wall time of program %f\n",
        stats.totalWallTime/1000000.0);
    fprintf(fp,"total data samples %d\n", stats.samplesReported);
    fprintf(fp,"sampling rate %f\n", stats.samplingRate);
    fprintf(fp,"Application program ticks %d\n", stats.userTicks);
    fprintf(fp,"Instrumentation ticks %d\n", stats.instTicks);

    fclose(fp);
#endif

    /* record that we are done -- should be somewhere better. */
    DYNINSTgenerateTraceRecord(0, TR_EXIT, sizeof(stats), &stats, 1);
}





/************************************************************************
 * void DYNINSTrecordTag(int tag)
 *
 * mark a new tag in tag list.
************************************************************************/


void
DYNINSTrecordTag(int tag) {
    int i;

    if (DYNINSTtagCount >= DYNINSTtagLimit) return;

    for (i=0; i < DYNINSTtagCount; i++) {
        if (DYNINSTtags[i] == tag) return;
    }

    if (DYNINSTtagCount == DYNINSTtagLimit) return;
    DYNINSTtags[DYNINSTtagCount++] = tag;
}





/************************************************************************
 * void DYNINSTreportNewTags(void)
 *
 * inform the paradyn daemons of new message tags.
************************************************************************/

void
DYNINSTreportNewTags(void) {
    int i;
    static int lastTagCount=0;
    struct _newresource newRes;

#ifdef COSTTEST
    time64 startT,endT;
    startT=DYNINSTgetCPUtime();
#endif

    for (i=lastTagCount; i < DYNINSTtagCount; i++) {
        memset(&newRes, '\0', sizeof(newRes));
        sprintf(newRes.name, "SyncObject/MsgTag/%d", DYNINSTtags[i]);
        strcpy(newRes.abstraction, "BASE");
        DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE, 
            sizeof(struct _newresource), &newRes, 1);
    }
    lastTagCount = DYNINSTtagCount;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[8]+=endT-startT;
    DYNINSTtestN[8]++;
#endif 
}





/************************************************************************
 * void DYNINSTreportCounter(intCounter* counter)
 *
 * report value of counter to paradynd.
************************************************************************/

void
DYNINSTreportCounter(intCounter* counter) {
    traceSample sample;

#ifdef COSTTEST
    time64 startT, endT;
    startT=DYNINSTgetCPUtime();
#endif

    sample.value = counter->value;
    sample.id    = counter->id;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0);

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[6]+=endT-startT;
    DYNINSTtestN[6]++;
#endif 

}





/************************************************************************
 * DYNINST test functions.
************************************************************************/

void DYNINSTsimplePrint(void) {
    printf("inside dynamic inst function\n");
}

void DYNINSTentryPrint(int arg) {
    printf("enter %d\n", arg);
}

void DYNINSTcallFrom(int arg) {
    printf("call from %d\n", arg);
}

void DYNINSTcallReturn(int arg) {
    printf("return to %d\n", arg);
}

void DYNINSTexitPrint(int arg) {
    printf("exit %d\n", arg);
}





/************************************************************************
 * void DYNINSTreportCost(intCounter* counter)
 *
 * report the cost (from the cost model).
************************************************************************/

void
DYNINSTreportCost(intCounter *counter) {
    int64         value;
    double        cost;
    static double prev_cost = 0;
    traceSample   sample;

#ifdef COSTTEST
    time64 startT, endT;
    startT=DYNINSTgetCPUtime();
#endif

    value = DYNINSTgetObservedCycles(1);
    cost  = ((double) value) * (DYNINSTcyclesToUsec / MILLION);

#ifdef notdef
    printf("value = %f, reportedCost %f\n", ((double) value), cost);
    printf("DYNINSTcyclesToUsec = %f\n", (double) DYNINSTcyclesToUsec);
    fflush(stdout);
#endif
    if (cost < prev_cost) {
        fprintf(stderr, "Fatal Error: cost counter went backwards\n");
        fflush(stderr);
        abort();
    }

    prev_cost = cost;

    sample.value = cost;
    sample.id    = counter->id;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof sample, &sample, 0);

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[7]+=endT-startT;
    DYNINSTtestN[7]++;
#endif 
}


void
DYNINSTreportObsCostLow(intCounter *counter) {
    traceSample   sample;

#ifdef USE_PROF
    if (DYNINSTprofile) {
	int i;
	int limit;
        double cost;
	int pageSize;
        int64 value;
	int startInst;
	int instTicks;
	extern void DYNINSTfirst();


	limit = DYNINSTbufsiz;
	/* first inst code - assumes data area above code space in virtual
	 * address */
	startInst = (int) &DYNINSTfirst;
	instTicks = 0;
	for (i=0; i < limit; i++) {
	    if (i * DYNINSTtoAddr > startInst) {
		instTicks += DYNINSTprofBuffer[i];
	    }
	}

	cost  = ((double) instTicks ) /100.0;
#else
	{
#endif

	sample.value = 0;
	sample.id    = counter->id;
	DYNINSTtotalSamples++;

	DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof sample, &sample, 0);
    }
}

/*
 * Return the number of bytes in a message hdr iovect.
 *
 */
int
DYNINSTgetMsgVectBytes(struct msghdr *msg)
{
    int i;
    int count;

    for (i=0, count=0; i < msg->msg_iovlen; i++) {
	count += msg->msg_iov[i].iov_len;
    }
    return count;
}
