/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

/* $Id: RTetc-linux.c,v 1.25 2002/12/14 16:37:59 schendel Exp $ */

/************************************************************************
 * RTetc-linux.c: clock access functions, etc.
************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <assert.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <stdio.h>
#include <fcntl.h> /* O_RDONLY */
/* #include <sigcontext.h> - included in signal.h */
#include <unistd.h> /* getpid() */
#include <limits.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "rtinst/h/RThwtimer-linux.h"

#ifdef PAPI
#include "papi.h"
#endif

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
#include <thread.h>
#endif

/*extern int    gettimeofday(struct timeval *, struct timezone *);*/
extern void perror(const char *);

/************************************************************************
 * symbolic constants.
************************************************************************/

static int procfd = -1;
#ifdef HRTIME
struct hrtime_struct *hr_cpu_map = NULL;
#endif

rawTime64 cpuPrevious_hw  = 0;
rawTime64 cpuPrevious_sw  = 0;
rawTime64 wallPrevious_hw = 0;
rawTime64 wallPrevious_sw = 0;

/* PARADYNos_init formerly "void DYNINSTgetCPUtimeInitialize(void)" */
void PARADYNos_init(int calledByFork, int calledByAttach) {
#ifdef HRTIME
  if(isLibhrtimeAvail(&hr_cpu_map, (int)getpid()))
    hintBestCpuTimerLevel  = HARDWARE_TIMER_LEVEL;
  else
#endif

#ifdef PAPI
  if(isPapiAvail()) {
    hintBestCpuTimerLevel  = HARDWARE_TIMER_LEVEL;
  }
  else
#endif

    hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
  if(isTSCAvail()) {
    hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;   
  }
  else {
    hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;
  }

  /* needs to be reinitialized when fork occurs */
  cpuPrevious_hw = 0;
  cpuPrevious_sw = 0;
  wallPrevious_hw = 0;
  wallPrevious_sw = 0;

#ifdef notdef   /* Has this ever been active on this platform? */
   /* This must be done once for each process (including forked) children */
    char str[20];
    sprintf(str, "/proc/%d", (int)getpid());
   /* have to use syscall here for applications that have their own
      versions of open, poll...In these cases there is no guarentee that
      things have been initialized so that the application's version of
      open can be used when this open call occurs (in DYNINSTinit)
   */
    procfd = _syscall2(SYS_open,str, O_RDONLY);
    if (procfd < 0) {
      fprintf(stderr, "open of /proc failed in PARADYNos_init\n");
      perror("open");
      abort();
    }
#endif
}

void PARADYN_forkEarlyInit() {
}

static unsigned long long mulMillion(unsigned long long in) {
   unsigned long long result = in;

   /* multiply by 125 by multiplying by 128 and subtracting 3x */
   result = (result << 7) - result - result - result;

   /* multiply by 125 again, for a total of 15625x */
   result = (result << 7) - result - result - result;

   /* multiply by 64, for a total of 1,000,000x */
   result <<= 6;

   /* cost was: 3 shifts and 6 subtracts
    * cost of calling mul1000(mul1000()) would be: 6 shifts and 4 subtracts
    *
    * Another algorithm is to multiply by 2^6 and then 5^6.
    * The former is super-cheap (one shift); the latter is more expensive.
    * 5^6 = 15625 = 16384 - 512 - 256 + 8 + 1
    * so multiplying by 5^6 means 4 shift operations and 4 add/sub ops
    * so multiplying by 1000000 means 5 shift operations and 4 add/sub ops.
    * That may or may not be cheaper than what we're doing (3 shifts; 6 subtracts);
    * I'm not sure.  --ari
    */

   return result;
}

/*static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* --- CPU time retrieval functions --- */
/* Hardware Level ---
   method:      libhrtime get_hrvtime()
   return unit: ticks
*/
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  static int cpuRollbackOccurred = 0;
  rawTime64 now=0, tmp_cpuPrevious = cpuPrevious_hw;

#ifdef HRTIME  
  now = hrtimeGetVtime(hr_cpu_map);
#endif

#ifdef PAPI
  now = (rawTime64) PAPI_get_virt_cyc();
#endif

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld ticks, using previous value %lld ticks.",
	      tmp_cpuPrevious - now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData),
				 &traceData, 1, 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious_hw;
  }
  else  cpuPrevious_hw = now;
  
  return now;
}

/* Software Level --- 
   method:      times()
   return unit: jiffies  
*/
rawTime64
DYNINSTgetCPUtime_sw(void) {
  static int cpuRollbackOccurred = 0;
  rawTime64 now=0, tmp_cpuPrevious = cpuPrevious_sw;
  struct tms tm;
  
  times( &tm );
  now = (rawTime64)tm.tms_utime + (rawTime64)tm.tms_stime;
  
  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld jiffies, using previous value %lld jiffies.",
	      tmp_cpuPrevious - now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData),
				 &traceData, 1, 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious_sw;
  }
  else  cpuPrevious_sw = now;
  
  return now;
}


/* --- Wall time retrieval functions --- */
/* Hardware Level ---
   method:      direct read of TSC (ie. time stamp counter) register
   return unit: ticks
*/
rawTime64
DYNINSTgetWalltime_hw(void) {
  static int wallRollbackOccurred = 0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_hw;
  struct timeval tv;

  getTSC(now);

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld ticks, using previous value %lld ticks.",
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 
			       1, 1, 1);
    }
    wallRollbackOccurred++;
    wallPrevious_hw = now;
  }
  else  wallPrevious_hw = now;

  return now;
}

/* Software Level --- 
   method:      gettimeofday()
   return unit: microseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static int wallRollbackOccurred = 0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_sw;
  struct timeval tv;

  if (gettimeofday(&tv,NULL) == -1) {
    perror("gettimeofday");
    assert(0);
    abort();
  }
  
  now = mulMillion( (rawTime64)tv.tv_sec );
  now += (rawTime64)tv.tv_usec;

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld usecs, using previous value %lld usecs.",
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 
			       1, 1, 1);
    }
    wallRollbackOccurred++;
    wallPrevious_sw = now;
  }
  else  wallPrevious_sw = now;

  return(now);
}


#if defined(SHM_SAMPLING) && defined(MT_THREAD)
extern unsigned DYNINST_hash_lookup(unsigned key);
extern unsigned DYNINST_initialize_done;
extern void DYNINST_initialize_hash(unsigned total);
extern void DYNINST_initialize_free(unsigned total);
extern unsigned DYNINST_hash_insert(unsigned k);

int DYNINSTthreadSelf(void) {
  return(thr_self());
}

int DYNINSTthreadPos(void) {
  if (initialize_done) {
    return(DYNINST_hash_lookup(DYNINSTthreadSelf()));
  } else {
    DYNINST_initialize_free(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_hash(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_done=1;
    return(DYNINST_hash_insert(DYNINSTthreadSelf()));
  }
}
#endif


