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

/************************************************************************
 * RTaix.c: clock access functions for AIX.
 * $Id: RTetc-aix.c,v 1.23 2000/10/17 17:42:51 schendel Exp $
 ************************************************************************/

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <sys/ldr.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <limits.h>   /* for INT_MAX */

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#include <sys/types.h>

/* For read_real_time */
#include <sys/systemcfg.h>


#if defined(SHM_SAMPLING) && defined(MT_THREAD)
#include <sys/thread.h>
#endif

/* See comment below. Uses the same method as paradynd, but causes
   a SIGILL right now 
*/
#ifdef USE_GETPROCS_METHOD
#include <procinfo.h> /* For getprocs() call */
#endif


/************************************************************************
 * void PARADYNos_init
 *
 * OS initialization function---currently null.
 ************************************************************************/

void
PARADYNos_init(int calledByFork, int calledByAttach) {
 hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
 hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;
}

/*static int MaxRollbackReport = 0;*/ /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1;*/ /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  return 0;
}

/* Software Level --- 
   method:      getrusage()
   return unit: microseconds
*/
rawTime64
DYNINSTgetCPUtime_sw(void) {
  static rawTime64 cpuPrevious = 0;
  static int cpuRollbackOccurred = 0;
  rawTime64 now, tmp_cpuPrevious=cpuPrevious, us;
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru)) {
    perror("getrusage");
    abort();
  }
  
  now = (rawTime64) ru.ru_utime.tv_sec + (rawTime64) ru.ru_stime.tv_sec;
  now *= I64_C(1000000);
  /* Though the docs say microseconds, the usec fields are in nanos */
  /* Note: resolution is ONLY hundredths of seconds */  
  us = (rawTime64) ru.ru_utime.tv_usec + (rawTime64) ru.ru_stime.tv_usec;
  now += us;

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld us, using previous value %lld us.",
                tmp_cpuPrevious-now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious;
  }
  else  cpuPrevious = now;
  
  return now;
}

#ifdef USE_GETPROCS_METHOD
/* Software Level --- 
   method:      getprocs()
   return unit: microseconds
   note: not currently used since causes a SIGILL (illegal instruction) in
         the bubba program.
*/
rawTime64 DYNINSTgetCPUtime_sw_proc(void) 
{
  static rawTime64 cpuPrevious = 0;
  static int cpuRollbackOccurred = 0;
  rawTime64 now, tmp_cpuPrevious=cpuPrevious;

  /* I really hate to use an ifdef, but I don't want to toss the code.

     Getprocs: uses the same method as the dyninst library, but causes
     a SIGILL (illegal instruction) in the bubba program. 

     Rusage: the old (and working) method. Uses much less time than
     the getprocs method.
  */

  /* Constant for the number of processes wanted in info */
  const unsigned int numProcsWanted = 1;
  struct procsinfo procInfoBuf[numProcsWanted];
  struct fdsinfo fdsInfoBuf[numProcsWanted];
  int numProcsReturned;
  pid_t wantedPid = getpid();

  const int sizeProcInfo = sizeof(struct procsinfo);
  const int sizeFdsInfo = sizeof(struct fdsinfo);
  rawTime64 nanoseconds;

  numProcsReturned = getprocs(procInfoBuf,
			      sizeProcInfo,
			      fdsInfoBuf,
			      sizeFdsInfo,
			      &wantedPid,
			      numProcsWanted);

  if (numProcsReturned == -1) /* Didn't work */
    perror("Failure in getInferiorCPUtime");

  /* Get the user+sys time from the rusage struct in procsinfo */
  now = (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_sec + // User time
        (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_sec;  // System time

  now *= (rawTime64) 1000000; /* Secs -> millions of microsecs */

  /* Though the docs say microseconds, the usec fields are in nanos */
  /* Note: resolution is ONLY hundredths of seconds */
  nanoseconds= (rawTime64) procInfoBuf[0].pi_ru.ru_utime.tv_usec + 
               (rawTime64) procInfoBuf[0].pi_ru.ru_stime.tv_usec;  

  /* The daemon time retrieval function and conversion ratio is currently set
     up for microseconds (for aix) because the getrusage method requires
     this. */
  now += nanoseconds / (rawTime64) 1000; /* Nanos->micros */

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld ns, using previous value %lld ns.",
                tmp_cpuPrevious-now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious;
  }
  else  cpuPrevious = now;
  
  return now;
}
#endif

/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64
DYNINSTgetWalltime_hw(void) {
  return 0;
}

/* Software Level --- 
   method:      read_real_time()
   return unit: nanoseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static rawTime64 wallPrevious=0;
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious=wallPrevious;
  struct timebasestruct timestruct;
#if 0
  register unsigned int timeSec asm("5");
  register unsigned int timeNano asm("6");
  register unsigned int timeSec2 asm("7");
  
  /* Need to read the first value twice to make sure it doesn't roll
   *   over while we are reading it.
   */

  do {
    asm("mfspr   5,4");		/* read high into register 5 - timeSec */
    asm("mfspr   6,5");		/* read low into register 6 - timeNano */
    asm("mfspr   7,4");		/* read high into register 7 - timeSec2 */
  } while(timeSec != timeSec2);

  now = (rawTime64) timeSec;
  /* Bump seconds into billions of nanoseconds */
  now *= (rawTime64) 1000000000;
  now += (rawTime64) timeNano;
#endif

  read_real_time(&timestruct, TIMEBASE_SZ);
  time_base_to_time(&timestruct, TIMEBASE_SZ);
  /* ts.tb_high is seconds, ts.tb_low is nanos */
  now = (rawTime64) timestruct.tb_high;
  now *= (rawTime64) 1000000000;
  now += (rawTime64) timestruct.tb_low;

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld ns, using previous value %lld ns.",
                tmp_wallPrevious-now,now,tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious;
  }
  else wallPrevious = now;
  return now;
}


/*
 * DYNINSTgetRusage(id) - Return the value of various OS stats.
 *
 *    The id is an integer, and when they are changed, any metric that uses
 *        DYNINSTgetRusage will also need to be updated.
 *
 */
int DYNINSTgetRusage(int id)
{

    int ret;
    int value;
    struct rusage rusage;
    struct rusage *DYNINSTrusagePtr;

    ret = getrusage(RUSAGE_SELF, &rusage);
    if (ret) {
	perror("getrusage");
    }
    DYNINSTrusagePtr = &rusage;
    switch (id) {
	case 0:	/* page faults */
	    value = DYNINSTrusagePtr->ru_minflt+DYNINSTrusagePtr->ru_majflt;
	    break;
	case 1:	/* swaps */
	    value = DYNINSTrusagePtr->ru_nswap;
	    break;
	case 2: /* signals received */
	    value = DYNINSTrusagePtr->ru_nsignals;
	    break;
	case 3: /* max rss */
	    value = DYNINSTrusagePtr->ru_maxrss;
	    break;
	case 4: /* context switches */
	    value = DYNINSTrusagePtr->ru_nvcsw + DYNINSTrusagePtr->ru_nivcsw;
	    break;
	case 5: /* system time - in milli-seconds */
	  /* Umm... why milli-seconds? */
	    value = 1000 * DYNINSTrusagePtr->ru_stime.tv_sec + 
	                   DYNINSTrusagePtr->ru_stime.tv_usec/1000;
	    break;
	default:
	    value = 0;
	    break;
    }
    return value;
}
