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

#ifdef SHM_SAMPLING
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

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


/* sunos's header files don't have these: */
#ifdef sparc_sun_sunos4_1_3
extern int socket(int, int, int);
extern int connect(int, struct sockaddr *, int);
extern int fwrite(void *, int, int, FILE *);
extern int setvbuf(FILE *, char *, int, int);
#ifdef SHM_SAMPLING
extern int shmget(key_t, unsigned, unsigned);
extern void *shmat(int, void *, int);
extern int shmdt(void*);
extern int shmctl(int, int, struct shmid_ds *);
#endif
#endif

#include "util/h/spinMutex_cintf.h"

/************************************************************************
 * Per module flags
************************************************************************/

#ifndef SHM_SAMPLING
static int DYNINSTin_sample = 0;
#endif

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

extern void   DYNINSTos_init(int calledByFork, int calledByAttach);
extern time64 DYNINSTgetCPUtime(void);
extern time64 DYNINSTgetWalltime(void);





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
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifdef COSTTEST
    time64 startT,endT;
#endif

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample)
       return;
#endif

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

/* For shared-mem sampling only: bump protector1, do work, then bump protector2 */
#ifdef SHM_SAMPLING
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;
    /* How about some kind of inline asm that flushes writes when the architecture
       is using some kind of relaxed multiprocessor consistency? */
#endif

    /* Note that among the data vrbles, counter is incremented last; in particular,
       after start has been written.  This avoids a nasty little race condition in
       sampling where count is 1 yet start is undefined (or using an old value) when
       read, which usually leads to a rollback.  --ari */
    if (timer->counter == 0) {
        timer->start     = DYNINSTgetCPUtime();
    }
    timer->counter++;

#ifdef SHM_SAMPLING
    timer->protector2++; /* alternatively, timer->protector2 = timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    timer->normalize = MILLION; /* I think this vrble is obsolete & can be removed */
#endif


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
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifdef COSTTEST
    time64 startT,endT;
#endif

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample)
       return;       
#endif

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

#ifdef SHM_SAMPLING
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;

    if (timer->counter == 0)
       ; /* a strange condition; shouldn't happen.  Should we make it an assert fail? */
    else {
       if (timer->counter == 1) {
          const time64 now = DYNINSTgetCPUtime();
          timer->total += (now - timer->start);

          if (now < timer->start) {
	     fprintf(stderr, "rtinst: cpu timer rollback.\n");
	     abort();
	  }
       }
       timer->counter--;
    }

    timer->protector2++; /* alternatively, timer->protector2=timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    if (timer->counter == 0)
       ; /* should we make this an assert fail? */
    else if (timer->counter == 1) {
        time64 now = DYNINSTgetCPUtime();

        timer->snapShot = timer->total + (now - timer->start);

        timer->mutex    = 1;
        /*                 
         * The reason why we do the following line in that way is because
         * a small race condition: If the sampling alarm goes off
         * at this point (before timer->mutex=1), then time will go backwards 
         * the next time a sample is take (if the {wall,process} timer has not
         * been restarted).
	 *
         */

        timer->total = DYNINSTgetCPUtime() - timer->start + timer->total; 

        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("process timer rollback\n"); fflush(stdout);

            abort();
        }
        timer->counter = 0;
        timer->mutex = 0;
    }
    else {
      timer->counter--;
    }
#endif

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
    /* WARNING: write() could be instrumented to call this routine, so to avoid
       some serious infinite recursion, avoid calling anything that might directly
       or indirectly call write() in this routine; e.g. printf()!!!!! */

#ifdef COSTTEST
    time64 startT, endT;
#endif

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start/stop timers, then a timer could be
       (incorrectly) started/stopped every time a sample is written back */

    if (DYNINSTin_sample) {
       return;
    }
#endif

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

#ifdef SHM_SAMPLING
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;
#endif

    /* Note that among the data vrbles, counter is incremented last; in particular,
       after start has been written.  This avoids a nasty little race condition in
       sampling where count is 1 yet start is undefined (or using an old value) when
       read, which usually leads to a rollback.  --ari */
    if (timer->counter == 0) {
        timer->start     = DYNINSTgetWalltime();
    }
    timer->counter++;

#ifdef SHM_SAMPLING
    timer->protector2++; /* or, timer->protector2 = timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    timer->normalize = MILLION; /* I think this vrble is obsolete & can be removed */
#endif

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
#ifdef COSTTEST
    time64 startT, endT;
#endif

#ifndef SHM_SAMPLING
    /* if "write" is instrumented to start timers, a timer could be started */
    /* when samples are being written back */

    if (DYNINSTin_sample)
       return;
#endif

#ifdef COSTTEST
    startT=DYNINSTgetCPUtime();
#endif

#ifdef SHM_SAMPLING
    assert(timer->protector1 == timer->protector2);
    timer->protector1++;

    if (timer->counter == 0)
       ; /* a strange condition; should we make it an assert fail? */
    else if (--timer->counter == 0) {
       const time64 now = DYNINSTgetWalltime();

       timer->total += (now - timer->start);

       if (now < timer->start) {
	  fprintf(stderr, "rtinst wall timer rollback.\n");
	  abort();
       }
    }
    
    timer->protector2++; /* or, timer->protector2 = timer->protector1 */
    assert(timer->protector1 == timer->protector2);
#else
    if (timer->counter == 0)
       ; /* a strange condition; should we make it an assert fail? */
    else if (timer->counter == 1) {
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
        if (now < timer->start) {
            printf("id=%d, snapShot=%f total=%f, \n start=%f  now=%f\n",
                   timer->id.id, (double)timer->snapShot,
                   (double)timer->total, 
                   (double)timer->start, (double)now);
            printf("wall timer rollback\n"); 
            fflush(stdout);

            abort();
        }
        timer->counter  = 0;
        timer->mutex    = 0;
    }
    else {
        timer->counter--;
    }
#endif

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[3]+=endT-startT;
    DYNINSTtestN[3]++;
#endif 
}





/************************************************************************
 * void DYNINST_install_ualarm(unsigned value, unsigned interval)
 *
 * an implementation of "ualarm" using the "setitimer" syscall.
************************************************************************/

#ifndef SHM_SAMPLING
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
#endif




/************************************************************************
 * global data for DYNINST functions.
************************************************************************/

double DYNINSTdata[SYN_INST_BUF_SIZE/sizeof(double)];
double DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(double)]; // NOT NEEDED ANY MORE!!!

/* As DYNINSTinit() completes, it has information to pass back
   to paradynd.  The data can differ; more stuff is needed
   when SHM_SAMPLING is defined, for example.  But in any event,
   the following gives us a general framework.  When DYNINSTinit()
   is about to complete, we used to send a TR_START trace record back and then
   DYNINSTbreakPoint().  But we've seen strange behavior; sometimes
   the breakPoint is received by paradynd first.  The following should
   work all the time:  DYNINSTinit() writes to the following vrble
   and does a DYNINSTbreakPoint() or TRAP, instead of sending a trace
   record.  Paradynd reads this vrble using ptrace(), and thus has
   the info that it needs.
   Note: we use this framework for DYNINSTfork(), too -- so the TR_FORK
   record is now obsolete, along with the TR_START record */

struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;




/************************************************************************
 * float DYNINSTcyclesPerSecond(void)
 *
 * need a well-defined method for finding the CPU cycle speed
 * on each CPU.
************************************************************************/

#if defined(rs6000_ibm_aix4_1)
#define NOPS_4  asm("oril 0,0,0"); asm("oril 0,0,0"); asm("oril 0,0,0"); asm("oril 0,0,0")
#else
#define NOPS_4  asm("nop"); asm("nop"); asm("nop"); asm("nop")
#endif
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

#ifdef i386_unknown_solaris2_5
    /* speed for the pentium is being overestimated by a factor of 2 */
    speed /= 2;
#endif
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
#ifdef i386_unknown_solaris2_5
    /* kludge for the pentium: we need to reset the FPU here, or we get 
       strange results on fp operations.
    */
    asm("finit");
#endif
}

static void
restoreFPUstate(float* base) {
}



/* The current observed cost since the last call to 
 *      DYNINSTgetObservedCycles(false) 
 */

#ifndef SHM_SAMPLING
/* This could be static, but gdb has can't find them if they are.  jkh 5/8/95 */
int64    DYNINSTvalue = 0;
unsigned DYNINSTlastLow;
unsigned DYNINSTobsCostLow;
#endif

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
#ifndef SHM_SAMPLING
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
#endif




/************************************************************************
 * void DYNINSTsampleValues(void)
 *
 * dummy function for sampling timers and counters.  the actual code
 * is added by dynamic instrumentation from the paradyn daemon.
************************************************************************/

static int DYNINSTnumReported = 0;

#ifndef SHM_SAMPLING
void
DYNINSTsampleValues(void) {
    DYNINSTnumReported++;
}
#endif



static int DYNINST_trace_fd = -1; /* low-level version of DYNINSTtraceFp */

/************************************************************************
 * void DYNINSTflushTrace(void)
 *
 * flush any accumulated traces.
************************************************************************/

static FILE* DYNINSTtraceFp = 0;

static void
DYNINSTflushTrace(void) {
    if (DYNINSTtraceFp) fflush(DYNINSTtraceFp);
}

/************************************************************************
 * void DYNINSTgenerateTraceRecord(traceStream sid, short type,
 *   short length, void* data, int flush,time64 wall_time,time64 process_time)
************************************************************************/

static time64 startWall = 0;

void
DYNINSTgenerateTraceRecord(traceStream sid, short type, short length,
			   void *eventData, int flush,
			   time64 wall_time, time64 process_time) {
    int             ret;
    static unsigned pipe_gone = 0;
    static unsigned inDYNINSTgenerateTraceRecord = 0;
    traceHeader     header;
    int             count;
    char            buffer[1024];

    if (inDYNINSTgenerateTraceRecord) return;
    inDYNINSTgenerateTraceRecord = 1;

    if (pipe_gone) {
        inDYNINSTgenerateTraceRecord = 0;
        return;
    }

    header.wall    = wall_time - startWall;
    header.process = process_time;
#ifdef ndef
    if(type == TR_SAMPLE){
	 traceSample *s = (traceSample*)eventData;
         printf("wall time = %f processTime = %f value = %f\n",
		(double)(header.wall/1000000.0), 
		(double)(header.process/1000000.0),
		s->value);
    }
#endif

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
        //DYNINSTtraceFp = fdopen(dup(CONTROLLER_FD), "w"); // why the dup()?
	assert(DYNINST_trace_fd != -1);
        DYNINSTtraceFp = fdopen(dup(DYNINST_trace_fd), "w"); // why the dup()?
	assert(DYNINSTtraceFp);
    }

    while (1) {
      errno=0;
      ret = fwrite(buffer, count, 1, DYNINSTtraceFp);
      if (errno || ret!=1) {
        if (errno==EINTR) {
          printf("(pid=%d) fwrite interrupted, trying again...\n",(int) getpid());
        } else {
	  perror("unable to write trace record");
          printf("disabling further data logging, pid=%d\n", (int) getpid());
          fflush(stdout);
          pipe_gone = 1;
          break;
        }
      }
      else break;
    }
    if (flush) DYNINSTflushTrace();

    inDYNINSTgenerateTraceRecord = 0;
}





/************************************************************************
 * void DYNINSTreportBaseTramps(void)
 *
 * report the cost of base trampolines.
************************************************************************/

static float DYNINSTcyclesToUsec  = 0;
static time64 DYNINSTtotalSampleTime = 0;

#ifndef SHM_SAMPLING
void
DYNINSTreportBaseTramps() {
    // NOTE: this routine has a misleading name; how about DYNINSTsampleObsCost().

    costUpdate sample;

    //
    // Adding the cost corresponding to the alarm when it goes off.
    // This value includes the time spent inside the routine (DYNINSTtotal-
    // sampleTime) plus the time spent during the context switch (121 usecs
    // for SS-10, sunos)
    //

    sample.obsCostIdeal  = ((((double) DYNINSTgetObservedCycles(1) *
                              (double)DYNINSTcyclesToUsec) + 
                             DYNINSTtotalSampleTime + 121) / 1000000.0);

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

    DYNINSTgenerateTraceRecord(0, TR_COST_UPDATE, sizeof(sample), &sample, 0,
			       DYNINSTgetWalltime(), DYNINSTgetCPUtime());
}
#endif

/************************************************************************
 * static void DYNINSTreportSamples(void)
 *
 * report samples to paradyn daemons. Called by DYNINSTinit, DYNINSTexit,
 * and DYNINSTalarmExpires.
************************************************************************/

#define N_FP_REGS 33

#ifndef SHM_SAMPLING

static void 
DYNINSTreportSamples(void) {
    time64     start_cpu;
    float      fp_context[N_FP_REGS];

    ++DYNINSTin_sample;

    saveFPUstate(fp_context);
    start_cpu = DYNINSTgetCPUtime();

    /* to keep observed cost accurate due to 32-cycle rollover */
    (void) DYNINSTgetObservedCycles(0);

    DYNINSTsampleValues();
    DYNINSTreportBaseTramps();
    DYNINSTflushTrace();

    DYNINSTtotalSampleTime += (DYNINSTgetCPUtime() - start_cpu);
    restoreFPUstate(fp_context);
    --DYNINSTin_sample;
}
#endif




/************************************************************************
 * void DYNINSTalarmExpire(void)
 *
 * called periodically by signal handlers.  report sampled data back
 * to the paradyn daemons.  when the program exits, DYNINSTsampleValues
 * should be called directly.
************************************************************************/


/************************************************************************
 * DYNINSTalarmExpire is changed so that it will restart the sytem call
 * if that called is interrupted on HP
 * Duplicated for the same reason above.
 ************************************************************************/


volatile int DYNINSTsampleMultiple    = 1;
   /* written to by dynRPC::setSampleRate() (paradynd/dynrpc.C)
      (presumably, upon a fold) */

#ifndef SHM_SAMPLING
static int          DYNINSTnumSampled        = 0;
static int          DYNINSTtotalAlarmExpires = 0;
#endif

#ifndef SHM_SAMPLING
void
#if !defined(hppa1_1_hp_hpux)
DYNINSTalarmExpire(int signo) {
#else 
DYNINSTalarmExpire(int signo, int code, struct sigcontext *scp) {
#endif

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

    /* This piece of code is needed because DYNINSTalarmExpire's are always called
       at the initial pace (every .2 secs, I believe), whereas we only want to do
       stuff at the current pace (initially every .2 secs but less frequently after
       "folds").  We could just adjust the rate of SIGALRM, so this isn't
       needed. */

    if ((++DYNINSTnumSampled % DYNINSTsampleMultiple) == 0) {
      DYNINSTreportSamples();
    }

    DYNINSTin_sample = 0;

#if defined(hppa1_1_hp_hpux)
    scp->sc_syscall_action = SIG_RESTART;
#endif


#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[4]+=endT-startT;
    DYNINSTtestN[4]++;
#endif 
}
#endif





/************************************************************************
 * void DYNINSTinit()
 *
 * initialize the DYNINST library.  this function is called at the start
 * of the application program, as well as after a fork, and after an
 * attach.
 *
************************************************************************/

static float  DYNINSTsamplingRate   = 0;
static int    DYNINSTtotalSamples   = 0;
static tTimer DYNINSTelapsedCPUTime;
static tTimer DYNINSTelapsedTime;
static int DYNINSTtagCount = 0;
static int DYNINSTtagLimit = 100;
static int DYNINSTtags[1000];

#ifdef SHM_SAMPLING
int shm_create_existing(key_t theKey, unsigned theSize) {
   /* "create" segment (it already exists; paradynd did the real creating) but don't
      attach; returns seg id */

   int segid = shmget(theKey, theSize, 0666); // note no IPC_CREAT or IPC_EXCL flags
   if (segid == -1) {
       perror("shmget");
       fprintf(stderr, "DYNINSTinit cannot shmget for key %d, size %u\n",
               (int)theKey, theSize);
       return -1;
   }

   return segid;
}

void *shm_attach(int shmid) {
   void *result = shmat(shmid, NULL, 0);
   if (result == (void *)-1) {
       perror("DYNINSTinit: cannot shmat");
       return NULL;
   }

   return result;
}

void shm_detach(void *shmSegPtr) {
   (void)shmdt(shmSegPtr);
}

void *shm_detach_reattach_overlap(int newshmid, void *shmSegPtr) {
   /* returns NULL on success (attached at same location),
      (void*)-1 on total failure, and
      a new address on partial failure (attached, but not at same location)
    */

   int success = 1; /* success, so far */
   void *newAttachedAtPtr = NULL;

   int detach_result = shmdt(shmSegPtr);
   if (detach_result == -1) {
      perror("shm_detach_reattach_overlap shmdt");
      success = 0; /* failure; don't try to attach at same loc */
   }

   if (success) {
      newAttachedAtPtr = shmat(newshmid, shmSegPtr, 0);
      if (newAttachedAtPtr == (void*)-1) {
         perror("shm_detach_reattach_overlap: cannot shmat");
	 success = 0; /* couldn't attach at same loc */
      }
   }

   if (success)
      return NULL; /* attached, and at the same location */
   else {
      /* couldn't attach at same location, so attach at a different location */
      newAttachedAtPtr = shmat(newshmid, NULL, 0);
      if (newAttachedAtPtr == (void*)-1)
	 /* this is unexpected...would have though that at least this would work */
	 perror("shm_detach_reattach_overlap: cannot shmat2");

      return newAttachedAtPtr; /* -1 on total failure; an address on partial */
   }
}
#endif

#ifdef SHM_SAMPLING
/* these vrbles are global so that fork() knows the attributes of the
   shm segs we've been using */
static int the_shmSegKey;
static int the_shmSegNumBytes;
static int the_shmSegShmId; /* needed? */
static void *the_shmSegAttachedPtr;
#endif

void shmsampling_printf(const char *fmt, ...) {
#ifdef SHM_SAMPLING_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
extern char *sys_errlist[];

/************************************************************************
 * static int connectToDaemon(int port)
 *
 * get a connection to a paradyn daemon for the trace stream
 *
 * does: socket(), connect()
************************************************************************/

static int connectToDaemon(int daemonPort, unsigned long hostAddr) {
  struct sockaddr_in sadr;
  int sock_fd;

  memset((void*) &sadr, 0, sizeof(sadr));
  sadr.sin_family = PF_INET;
  sadr.sin_port = htons(daemonPort);
  sadr.sin_addr.s_addr = hostAddr;

  sock_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("DYNINST connectToDaemon socket()");
    abort();
  }

//  if (sock_fd != CONTROLLER_FD) {
//    fprintf(stderr, "DYNINST: socket error...sock_fd (%d) != CONTROLLER_FD (%d)\n",
//	    sock_fd, CONTROLLER_FD);
//    abort();
//  }

  if (connect(sock_fd, (struct sockaddr *) &sadr, sizeof(sadr)) < 0) {
    perror("DYNINST connectToDaemon: connect()");
    abort();
  }

  return sock_fd;
}

void
DYNINSTinit(int theKey, int shmSegNumBytes, int portnum, unsigned hostaddr) {
   /* If all params are -1 then we're being called by DYNINSTfork(). */
   /* portnum and hostaddr are -1 unless we're called by attach, in which case it
      gives the port number to connect() to get an fd for the trace stream. */

   int calledFromFork = (theKey == -1);
   int calledFromAttach = (portnum != -1);

#ifndef SHM_SAMPLING
    struct sigaction act;
    unsigned         val;
#endif

    char *temp;

#ifdef SHM_SAMPLING_DEBUG
   char thehostname[80];
   extern int gethostname(char*,int);

   (void)gethostname(thehostname, 80);
   thehostname[79] = '\0';

   shmsampling_printf("WELCOME to DYNINSTinit (%s, pid=%d), args are %d, %d, %d, %u\n",
		      thehostname, (int)getpid(), theKey, shmSegNumBytes,
		      portnum, hostaddr);
#endif

#ifdef SHM_SAMPLING
   if (!calledFromFork) {
      the_shmSegKey = theKey;
      the_shmSegNumBytes = shmSegNumBytes;
   
       /* note: error checking needs to be beefed up here: */
   
       the_shmSegShmId = shm_create_existing(theKey, shmSegNumBytes); /* -1 on error */
       if (the_shmSegShmId == -1) {
          /* note: in theory, when we get a shm error on startup, it would be nice
                   to automatically "downshift" into the SIGALRM non-shm-sampling
                   code.  Not yet implemented. */
          fprintf(stderr, "DYNINSTinit failed because shm_create_existing failed.\n");
          fprintf(stderr, "DYNINST program startup failed...exiting program now.\n");
          exit(5);
       }

       the_shmSegAttachedPtr = shm_attach(the_shmSegShmId); /* NULL on error */
       if (the_shmSegAttachedPtr == NULL) {
          /* see above note... */
          fprintf(stderr, "DYNINSTinit failed because shm_attach failed.\n");
          fprintf(stderr, "DYNINST program startup failed...exiting program now.\n");
          exit(5);
       }
   }
#endif

    // In accordance with usual stdio rules, stdout is line-buffered and stderr is
    // non-buffered.  Unfortunately, stdio is a little clever and when it detects
    // stdout/stderr redirected to a pipe/file/whatever, it changes to fully-buffered.
    // This indeed occurs with us (see paradynd/src/process.C to see how a program's
    // stdout/stderr are redirected to a pipe). So we reset back to the desired
    // "bufferedness" here.  See stdio.h for these calls.  When attaching, stdio
    // isn't under paradynd control, so we don't do this stuff.

    /* Note! Since we are messing with stdio stuff here, it should go without
       saying that DYNINSTinit() (or at least this part of it) shouldn't be
       invoked until stdio has been initialized! */

    if (!calledFromAttach) {
       setvbuf(stdout, NULL, _IOLBF, 0);
          /* make stdout line-buffered.  "setlinebuf(stdout)" is cleaner but HP
	     doesn't seem to have it */
       setvbuf(stderr, NULL, _IONBF, 0);
          /* make stderr non-buffered */
    }

    DYNINSTos_init(calledFromFork, calledFromAttach);
      /* is this needed when calledFromFork?  Depends on what it does, which is in turn
         os-dependent...so, calledFromFork should probably be passed to this routine. */

    startWall = 0;

    if (!calledFromFork)
       DYNINSTcyclesToUsec = MILLION/DYNINSTcyclesPerSecond();

   /* Do we need to re-create the alarm signal stuff when calledFromFork is true? */
#ifndef SHM_SAMPLING
    act.sa_handler = DYNINSTalarmExpire;
    act.sa_flags   = 0;

    /* for AIX - default (non BSD) library does not restart - jkh 7/26/95 */
#if defined(SA_RESTART)
    act.sa_flags  |= SA_RESTART;
#endif

    sigfillset(&act.sa_mask);

#if defined(i386_unknown_solaris2_5) || defined(rs6000_ibm_aix4_1) 
    /* we need to catch SIGTRAP inside the alarm handler */    
    sigdelset(&act.sa_mask, SIGTRAP);
#endif

    if (sigaction(SIGALRM, &act, 0) == -1) {
        perror("sigaction(SIGALRM)");
        abort();
    }

    /* assign sampling rate to be default value in util/h/sys.h */
    val = BASESAMPLEINTERVAL;

    DYNINSTsamplingRate = val/MILLION;

    DYNINST_install_ualarm(val, val);
#endif

    if (!calledFromFork) {				   
       temp = getenv("DYNINSTtagLimit");
       if (temp) {
	  int newVal = atoi(temp);
	  if (newVal < DYNINSTtagLimit) {
	     DYNINSTtagLimit = newVal;
	  }
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

    /* Fill in info for paradynd to receive: */

    DYNINST_bootstrap_info.ppid = -1; /* not needed really */
				   
    if (!calledFromFork) {
#ifdef SHM_SAMPLING
       shmsampling_printf("DYNINSTinit setting appl_attachedAtPtr in bs_record to %x\n",
			  (unsigned)the_shmSegAttachedPtr);
       DYNINST_bootstrap_info.appl_attachedAtPtr = the_shmSegAttachedPtr;
#endif
    }

    DYNINST_bootstrap_info.pid = getpid();
    if (calledFromFork)
       DYNINST_bootstrap_info.ppid = getppid();

    /* We do this field last as a way to synchronize; paradynd will ignore what it
       sees in this structure until the event field is nonzero */
   if (calledFromFork)
      DYNINST_bootstrap_info.event = 2; /* 2 --> end of DYNINSTinit (forked process) */
   else if (calledFromAttach)
      DYNINST_bootstrap_info.event = 3; /* 3 --> end of DYNINSTinit (attached process) */
   else				   
      DYNINST_bootstrap_info.event = 1; /* 1 --> end of DYNINSTinit (normal or when called by exec'd proc) */

#ifndef SHM_SAMPLING
    /* what good does this do here? */
//    DYNINSTreportSamples();
#endif

   /* If attaching, now's the time where we set up the trace stream connection fd */
   if (calledFromAttach) {
      int pid = getpid();
      int ppid = getppid();
      unsigned attach_cookie = 0x22222222;

      /* portnum and hostaddr are passed in as params */
      int tracestream_fd = connectToDaemon(portnum, hostaddr);
      assert(tracestream_fd != -1);
         // warning: many parts in rtinst assume that it's 3, but that won't
	 // necessarily be the case for attach (the program is already running and
         // may have opened dozens of fd's).					       

      DYNINSTtraceFp = fdopen(tracestream_fd, "w");
      assert(DYNINSTtraceFp);

      fwrite(&attach_cookie, sizeof(attach_cookie), 1, DYNINSTtraceFp);
      fwrite(&pid, sizeof(pid), 1, DYNINSTtraceFp);
      fwrite(&ppid, sizeof(ppid), 1, DYNINSTtraceFp);
#ifdef SHM_SAMPLING
      fwrite(&the_shmSegKey, sizeof(the_shmSegKey), 1, DYNINSTtraceFp);
      fwrite(&the_shmSegAttachedPtr, sizeof(the_shmSegAttachedPtr), 1, DYNINSTtraceFp);
#endif
      fflush(DYNINSTtraceFp);
   }

   /* Now, we stop ourselves.  When paradynd receives the forwarded signal,
      it will read from DYNINST_bootstrap_info */

   shmsampling_printf("DYNINSTinit (pid=%d) --> about to DYNINSTbreakPoint()\n",
		      (int)getpid());

   DYNINSTbreakPoint();

   /* After the break, we should clear DYNINST_bootstrap_info */
   DYNINST_bootstrap_info.event = 0; /* 0 --> nothing */
   DYNINST_bootstrap_info.pid = 0; /* prob not needed */

   shmsampling_printf("leaving DYNINSTinit (pid=%d) --> the process is running freely now\n", (int)getpid());
}




void DYNINSTprintCost(void);

/************************************************************************
 * void DYNINSTexit(void)
 *
 * handle `exit' in the application. 
 * report samples and print cost.
************************************************************************/

void
DYNINSTexit(void) {
    static int done = 0;
    if (done) return;
    done = 1;

#ifndef SHM_SAMPLING
    DYNINSTreportSamples();
#endif

    /* NOTE: For shm sampling, we should do more here.  For example, we should
       probably disconnect from the shm segments.
       Note that we don't have to inform paradynd of the exit; it will find out
       soon enough on its own, due to the waitpid() it does and/or finding an
       end-of-file on one of our piped file descriptors. */

    DYNINSTprintCost();
}




/************************************************************************
 * void DYNINSTfork(void* arg, int pid)
 *
 * track a fork() system call, and report to the paradyn daemon.
 *
 * Instrumented by paradynd to be called at the exit point of fork().  So, the address
 * space (including the instrumentation heap) has already been duplicated, though some
 * meta-data still needs to be manually duplicated by paradynd.  Note that for
 * shm_sampling, a fork() just increases the reference count for the shm segment.
 * This is not what we want, so we need to (1) detach from the old segment, (2) create
 * a new shm segment and attach to it in the *same* virtual addr spot as the old
 * one (otherwise, references to the shm segment will be out of date!), and (3) let
 * let paradynd know what segment key numbers we have chosen for the new segments, so
 * that it may attach to them (paradynd may also set some values w/in the segments such
 * as mid's).
 * 
 * Who sends the initial trace record to paradynd?  The child creates the new shm seg
 * and thus only the child can inform paradynd of the chosen keys.  One might argue that
 * the child does't have a traceRecord connection yet, but fork() should dup() all fd's
 * and take care of that limitation...
************************************************************************/

#ifdef SHM_SAMPLING
void pickNewShmSegBaseKey(key_t *resultKey,
			  int *resultSegId) {
   /* picks an avail shm seg and creates it.  Doesn't attach to it, though */

   *resultKey = the_shmSegKey; /* as good a place to start as any... */

   while (1) {
      /* permissions chosen s.t. the request will fail if already exists */
      int permissions = 0666 | IPC_CREAT | IPC_EXCL;
         /* using IPC_EXCL ensure failure if already-exists */
      int failure;

      *resultSegId = shmget(*resultKey, the_shmSegNumBytes, permissions);

      /* successful? */
      failure = (*resultSegId == -1);

      /* If successful, then keep the segments and update result vrbles */
      if (!failure)
         return;

      /* On failure, we assume the shmget() didn't create anything. */
      /* So, all we need to do is bump the key and retry... */
      (*resultKey)++;
   }
}

static void makeNewShmSegCopy(void) {
   /* detach from old shm segment, create new segment (IN THE SAME LOCATION AS THE OLD
      ONE(*)) */
   /* (*) the new shm segment MUST be in the same location as the old one, or else
          any and all mini-tramps will be pointing to invalid addresses in the child
          process! (the shm seg will appear to have moved) */

   /* question: we detach from the old seg; should we also destroy it?  No, certainly
      not, since paradynd still cares about measuring the old process. */
   /* note: we used to memcpy all data from the old seg to the new one.  Now, since
      we must detach from the old one before creating the new one, we can't do that.
      Instead. we let paradynd do that. */

   key_t new_shmSegKey;
   int new_shmSegShmId;
   void *new_attachedAtPtr;

   /* 1. Pick a new, unused, key: */
   pickNewShmSegBaseKey(&new_shmSegKey, &new_shmSegShmId);
      /* picks & creates new segment; doesn't attach to it though */

   shmsampling_printf("d-f makeNewShmSegsCopy: did pickNewShmSegBaseKey; chose key %d\n",
		      (int)new_shmSegKey);

   /* 2. Attach to the seg, IN THE SAME LOCATION AS THE OLD ONE.  In order to
         achieve this sleight of hand, we must detach from the old one first, and pass
         an address param to shm_attach.  Luckily, this doesn't require destroying the
	 old segment --- a good thing, since we mustn't do that (paradynd still cares
	 about measuring the parent!) */

   new_attachedAtPtr = shm_detach_reattach_overlap(new_shmSegShmId,
						   the_shmSegAttachedPtr);
   if (new_attachedAtPtr == (void*)-1) {
      /* total failure; could not attach */
      fprintf(stderr, "makeNewShmSegCopy: failed to attach to new segment at all.\n");
      abort();
   }
   else if (new_attachedAtPtr != NULL) {
      /* partial failure; could not attach in the same location; could only attach at a
	 different location.  For now at least, this is as bad as a total failure */
      fprintf(stderr, "makeNewShmSegCopy: could not attach to new segment in the same location as the old one.\n");
      abort();
   }

   shmsampling_printf("d-f makeNewShmSegsCopy pid=%d: attached to new seg key=%d IN SAME LOC!\n", (int)getpid(), new_shmSegKey);
   shmsampling_printf("d-f makeNewShmSegsCopy: also, detached from old seg.\n");

   /* 3. Copy data from the old shm seg the new one */
   /* (We don't [and can't] do this here any more, since we've already detached from the
      old segment.  It was always a toss-up whether to let paradynd do the memcpy.
      Now it must; and it does.) */

   /* 4. update important global variables; the key and shmid change; the sizes and the
         attached location doesn't change */
   the_shmSegKey = new_shmSegKey;
   the_shmSegShmId = new_shmSegShmId;
   /* the_shmSegAttachedPtr = new_shmSegAttachedPtr; MUSTN'T CHANGE */

   shmsampling_printf("d-f makeNewShmSegsCopy: done.\n");
}
#endif



/* debug code should call forkexec_printf as a way of avoiding
   putting #ifdef FORK_EXEC_DEBUG around their fprintf(stderr, ...) statements,
   which can lead to horribly ugly code --ari */
/* similar for SHM_SAMPLING_DEBUG */
void forkexec_printf(const char *fmt, ...) {
#ifdef FORK_EXEC_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}


void
DYNINSTfork(int pid) {
    //forkexec_printf("DYNINSTfork called with pid = %d\n", pid);
    printf("DYNINSTfork -- WELCOME -- called with pid = %d\n", pid);
    fflush(stdout);

    if (pid > 0) {
       /* We used to send TR_FORK trace record here, in the parent.  But shm sampling
          requires the child to do this, so we moved the code there... */
       /* See metric.C for an explanation why it's important for the parent to
          be paused (not just the child) while propagating metric instances.
          Here's the short explanation: to initialize some of the timers and counters,
	  the child will copy some fields from the parent, and for the child to get
          values from the parent after the fork would be a bad thing.  --ari */

       forkexec_printf("DYNINSTfork parent; about to DYNINSTbreakPoint\n");
       fflush(stderr);

       DYNINSTbreakPoint();
    } else if (pid == 0) {
       /* we are the child process */
	int pid = getpid();
	int ppid = getppid();

        char *traceEnv;
	unsigned long tracePort;
	unsigned long hostAddr;

       forkexec_printf("DYNINSTfork CHILD -- welcome\n");
       fflush(stderr);

#ifdef SHM_SAMPLING
	/* Here, we need to detach from the old shm segment, create a new one
	   (in the same virtual memory location as the old one), and attach to it */
	makeNewShmSegCopy();
#endif

	/* Here is where we used to send a TR_FORK trace record.  But we've found
	   that sending a trace record followed by a DYNINSTbreakPoint had unpredictable
	   results -- sometimes the breakPoint would get delivered to paradynd first.
	   So idea #2 was to fill in DYNINST_bootstrapStruct and then do a breakpoint.
	   But the breakPoint won't get forwarded to paradynd because paradynd hasn't
	   yet attached to the child process.
	   So idea #3 (the current one) is to just send all that information along the
	   new connection (whereas we used to just send the pid).

	   NOTE: soon attach will probably be implemented in a similar way -- by
	         writing to the connection.
	 */

	/* get the socket port number for traces from the environment */
	{ 
	  extern long strtol(const char *, char **, int);

	  char *s1 = 0, *s2 = 0;
	  traceEnv = getenv("PARADYND_TRACE_SOCKET");
	  assert(traceEnv);

	  tracePort = (unsigned long)strtol(traceEnv, &s1, 10);
	  assert(traceEnv != s1);

	  hostAddr = (unsigned long)strtol(s1, &s2, 10);
	  assert(s1 != s2);
	}

	/* set up a connection to the daemon for the trace stream */
	forkexec_printf("dyninst-fork child closing old connections...\n");
	assert(DYNINST_trace_fd != -1);
        fclose(DYNINSTtraceFp);
	(void)close(DYNINST_trace_fd);
	//close(CONTROLLER_FD);

	forkexec_printf("dyninst-fork child opening new connection.\n");
	DYNINSTtraceFp = fdopen(connectToDaemon(tracePort, hostAddr), "w");

	forkexec_printf("dyninst-fork child pid %d opened new connection...now sending pid etc. along it\n", (int)getpid());

	{
	  unsigned fork_cookie = 0x11111111;
	  fwrite(&fork_cookie, sizeof(fork_cookie), 1, DYNINSTtraceFp);
	}

	fwrite(&pid, sizeof(pid), 1, DYNINSTtraceFp);
	fwrite(&ppid, sizeof(ppid), 1, DYNINSTtraceFp);
#ifdef SHM_SAMPLING
	fwrite(&the_shmSegKey, sizeof(the_shmSegKey), 1, DYNINSTtraceFp);
	fwrite(&the_shmSegAttachedPtr, sizeof(the_shmSegAttachedPtr),
	       1, DYNINSTtraceFp);
#endif

	fflush(DYNINSTtraceFp);

	forkexec_printf("dyninst-fork child pid %d sent pid; now doing DYNINSTbreakPoint() to wait for paradynd to initialize me.\n", (int)getpid());

	DYNINSTbreakPoint();

	forkexec_printf("dyninst-fork child past DYNINSTbreakPoint()...calling DYNINSTinit(-1,-1)\n");

	DYNINSTinit(-1,-1, -1, (unsigned)-1);
	   /* -1 params indicate called from DYNINSTfork */

	forkexec_printf("dyninst-fork child done...running freely.\n");
    }
}

void
DYNINSTexec(char *path) {
    /* paradynd instruments programs to call this routine on ENTRY to exec
       (so the exec hasn't yet taken place).  All that we do here is inform paradynd
       of the (pending) exec, and pause ourselves.  Paradynd will continue us after
       digesting the info...then, presumably, a TRAP will be generated, as the exec
       syscall completes. */

    forkexec_printf("execve called, path = %s\n", path);

    if (strlen(path) + 1 > sizeof(DYNINST_bootstrap_info.path)) {
      fprintf(stderr, "DYNINSTexec failed...path name too long\n");
      abort();
    }

    /* We used to send a TR_EXEC record here and then DYNINSTbreakPoint().
       But we've seen a race condition -- sometimes the break-point is delivered
       to paradynd before the trace record.  So instead, we fill in
       DYNINST_bootstrap_info and then do a breakpoint.  This approach is the same
       as the end of DYNINSTinit. */
    DYNINST_bootstrap_info.event = 4;
    DYNINST_bootstrap_info.pid = getpid();
    strcpy(DYNINST_bootstrap_info.path, path);

    /* The following turns OFF the alarm signal (the 0,0 parameters); when
       DYNINSTinit() runs again for the exec'd process, it'll be turned back on.  This
       stuff is necessary to avoid the process dying on an uncaught sigalarm while
       processing the exec */
#ifndef SHM_SAMPLING
    DYNINST_install_ualarm(0,0);
#endif

    forkexec_printf("DYNINSTexec before breakpoint\n");

    DYNINSTbreakPoint();

    /* after the breakpoint, clear DYNINST_bootstrap_info */
    DYNINST_bootstrap_info.event = 0; /* 0 --> nothing */

    forkexec_printf("DYNINSTexec after breakpoint...allowing the exec to happen now\n");
}

void
DYNINSTexecFailed() {
    time64 process_time = DYNINSTgetCPUtime();
    time64 wall_time = DYNINSTgetWalltime();
    int pid = getpid();

    forkexec_printf("DYNINSTexecFAILED\n");

    DYNINSTgenerateTraceRecord(0, TR_EXEC_FAILED, sizeof(int), &pid, 1,
			       process_time, wall_time);

#ifndef SHM_SAMPLING
    DYNINST_install_ualarm(BASESAMPLEINTERVAL, BASESAMPLEINTERVAL);
#endif
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
    time64 wall_time; 

    DYNINSTstopProcessTimer(&DYNINSTelapsedCPUTime);
    DYNINSTstopWallTimer(&DYNINSTelapsedTime);

#ifndef SHM_SAMPLING
    value = DYNINSTgetObservedCycles(0);
#else
    value = *(unsigned*)((char*)DYNINST_bootstrap_info.appl_attachedAtPtr + 12);
#endif
    stats.instCycles = value;

    value *= DYNINSTcyclesToUsec;

#ifndef SHM_SAMPLING
    stats.alarms      = DYNINSTtotalAlarmExpires;
#else
    stats.alarms      = 0;
#endif
    stats.numReported = DYNINSTnumReported;
    stats.instTime    = (double)value/(double)MILLION;
    stats.handlerCost = (double)DYNINSTtotalSampleTime/(double)MILLION;

    now = DYNINSTgetCPUtime();
    wall_time = DYNINSTgetWalltime();
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
    DYNINSTgenerateTraceRecord(0, TR_EXIT, sizeof(stats), &stats, 1,
		wall_time,now);
}





/************************************************************************
 * void DYNINSTreportNewTags(void)
 *
 * inform the paradyn daemons of new message tags.  Invoked
 * periodically by DYNINSTsampleValues (or directly from DYNINSTrecordTag,
 * if SHM_SAMPLING).
************************************************************************/

void
DYNINSTreportNewTags(void) {
    int i;
    static int lastTagCount=0;

    time64 process_time = DYNINSTgetCPUtime();
       /* not used by consumer [createProcess() in perfStream.C], so can prob. be set to
          a dummy value to save a little time. */

    time64 wall_time = DYNINSTgetWalltime();
       /* this _is_ needed; paradynd keeps the 'creation' time of each resource
          (resource.h) */

#ifdef COSTTEST
    time64 startT,endT;
    startT=DYNINSTgetCPUtime();
#endif

    for (i=lastTagCount; i < DYNINSTtagCount; i++) {
        struct _newresource newRes;
        memset(&newRes, '\0', sizeof(newRes));
        sprintf(newRes.name, "SyncObject/MsgTag/%d", DYNINSTtags[i]);
        strcpy(newRes.abstraction, "BASE");
	newRes.type = RES_TYPE_INT;

        DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE, 
				   sizeof(struct _newresource), &newRes, 1,
				   wall_time,process_time);
    }
    lastTagCount = DYNINSTtagCount;

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[8]+=endT-startT;
    DYNINSTtestN[8]++;
#endif 
}


/************************************************************************
 * void DYNINSTrecordTag(int tag)
 *
 * mark a new tag in tag list.
 * Routines such as pvm_send() are instrumented to call us.
 * In the non-shm-sampling case, we just appent to DYNINSTtags, if this
 * tag hasn't been seen before.
 * In the shm-sampling case, we _also_ call DYNINSTreportNewTags().
 * One might worry about infinite recursion (DYNINSTreportNewTags() calls
 * DYNINSTgenerateTraceRecord which calls write() which can be instrumented
 * to call DYNINSTrecordTag().....) but I don't think there's a problem.
 * Let's trace it through, assuming write() is instrumented to call
 * DYNINSTrecordTag() on entry:
 * 
 * -- write() is called by the program, which calls DYNINSTrecordTag.
 * -- DYNINSTrecordTag calls DYNINSTreportNewTags, which 
 *    calls DYNINSTgenerateTraceRecord, which calls write.
 * -- write() is instrumented to call DYNINSTrecordTag, so it does.
 *    the tag equals that of the trace fd.  If it's been seen already,
 *    then DYNINSTrecordTag returns before calling DYNINSTgenerateTraceRecord.
 *    If not, then it gets reported and then (the next time write() gets
 *    implicitly called) ignored in all future calls.
 *
 * So in summary, infinite recursion is probably not a problem because
 * DYNINSTrecordTag returns before calling DYNINSTreportNewTags
 * (and hence DYNINSTgenerateTraceRecord) if the tag has been seen
 * before.
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

#ifdef SHM_SAMPLING
    /* Usually, DYNINSTreportNewTags() is invoked only periodically, by
     * DYNINSTalarmExpire.  But since that routine no longer exists, we need
     * another way for it to be called.  Let's just call it directly here; I
     * don't think it'll be too expensive (unless a program declares a new
     * tag every fraction of a second!)  I also don't think there will be an
     * infinite recursion problem; see comment above.
     */
   DYNINSTreportNewTags();
#endif    
}





/************************************************************************
 * void DYNINSTreportCounter(intCounter* counter)
 *
 * report value of counter to paradynd.
************************************************************************/

#ifndef SHM_SAMPLING
void
DYNINSTreportCounter(intCounter* counter) {
    traceSample sample;

#ifdef COSTTEST
    time64 endT;
    time64 startT=DYNINSTgetCPUtime();
#endif
    time64 process_time = DYNINSTgetCPUtime();
    time64 wall_time = DYNINSTgetWalltime();
    sample.value = counter->value;
       /* WARNING: changes "int" to "float", with the associated problems of
	            normalized number gaps; hence, the value has likely changed
		    a bit (more for higher integers, say in the millions). */
    sample.id    = counter->id;
    DYNINSTtotalSamples++;

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
			       wall_time,process_time);

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[6]+=endT-startT;
    DYNINSTtestN[6]++;
#endif 

}
#endif

/************************************************************************
 * void DYNINSTreportTimer(tTimer* timer)
 *
 * report the timer `timer' to the paradyn daemon.
************************************************************************/

#ifndef SHM_SAMPLING
void
DYNINSTreportTimer(tTimer *timer) {
    time64 total;
    traceSample sample;

#ifdef COSTTEST
    time64 endT;
    time64 startT = DYNINSTgetCPUtime();
#endif

    time64 process_time = DYNINSTgetCPUtime();
    time64 wall_time = DYNINSTgetWalltime();
    if (timer->mutex) {
        total = timer->snapShot;
	//printf("id %d using snapshot value of %f\n", timer->id.id, (double)total);
    }
    else if (timer->counter) {
        /* timer is running */
        time64 now;
        if (timer->type == processTime) {
            now = process_time;
        } else {
            now = wall_time;
        }
        total = now - timer->start + timer->total;
	//printf("id %d using added-to-total value of %f\n", timer->id.id, (double)total);
    }
    else {
        total = timer->total;
	//printf("using %d total value of %f\n", timer->id.id, (double)total);
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
        printf("mutex=%d, counter=%d, snapShot=%f\n",
            (int) timer->mutex, (int) timer->counter,
            (double) timer->snapShot);
        printf("start = %f, total = %f\n",
	       (double) timer->start, (double) timer->total);
        fflush(stdout);
        abort();
    }

    timer->lastValue = total;

    sample.id = timer->id;
    if (timer->normalize == 0) {
       fprintf(stderr, "DYNINSTreportTimer WARNING: timer->normalize is 0!!\n");
    }

    sample.value = ((double) total) / (double) timer->normalize;
    // check for nan now?
       // NOTE: The type of sample.value is float, not double, so some precision
       //       is lost here.  Besides, wouldn't it be better to send the raw
       //       "total", with no loss in precision; especially since timer->normalize
       //       always seems to be 1 million? (Well, it differs for cm5)
    DYNINSTtotalSamples++;

#ifdef ndef
    printf("raw sample %d = %f time = %f\n", sample.id.id, sample.value,
				(double)(now/1000000.0));  
#endif

    DYNINSTgenerateTraceRecord(0, TR_SAMPLE, sizeof(sample), &sample, 0,
				wall_time,process_time);

#ifdef COSTTEST
    endT=DYNINSTgetCPUtime();
    DYNINSTtest[5]+=endT-startT;
    DYNINSTtestN[5]++;
#endif 
}
#endif




/************************************************************************
 * DYNINST test functions.
 *
 * Since these routines aren't used regularly, shouldn't they be surrounded
 * with an ifdef? --ari
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
