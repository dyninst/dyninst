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
 * RTaix.c: clock access functions for aix.
 *
 * $Log: RTaix.c,v $
 * Revision 1.15  1999/10/27 21:49:52  schendel
 * removed rollback checks in stop/startTimer functions and incorporated
 * standardized rollback checks and reporting via trace records
 *
 * Revision 1.14  1999/10/13 18:18:46  bernat
 * Removed any call to assert(), printf() from getWalltime.
 *
 * Added infrastructure for a better test of what is causing the rollbacks.
 *
 * Revision 1.13  1999/08/27 21:04:01  zhichen
 * tidy up
 *
 * Revision 1.12  1999/08/20 20:38:40  bernat
 * Enabled shared memory.
 *
 * Changed getWallTime to use system library call (more portable)
 *
 * Added getProcs() code (DISABLED) to getCPUTime
 *
 * Revision 1.11  1997/06/02 16:39:40  naim
 * Small change to comment out a warning message - naim
 *
 * Revision 1.10  1997/05/07 18:59:15  naim
 * Getting rid of old support for threads and turning it off until the new
 * version is finished - naim
 *
 * Revision 1.9  1997/02/18 21:34:37  sec
 * There were some bugs in how the time was accessed, fixed those; I also
 * removed DYNISTexecvp which is buggy, and is never called (it was called
 * for MPI stuff, but I replaced it with some modifications for poe/mpi in
 * paradyn/DMthread).
 *
 * Revision 1.8  1997/01/27 19:43:31  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.7  1997/01/16 22:19:34  tamches
 * added proper param names to DYNINSTos_init
 *
 * Revision 1.6  1997/01/16 20:55:45  tamches
 * params to DYNINSTos_init
 *
 * Revision 1.5  1996/08/16 21:27:27  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.4  1996/04/06 21:27:50  hollings
 * Add missing case for system time.
 *
 * Revision 1.3  1996/02/13  16:21:57  hollings
 * Fixed timer64 to be time64.
 *
 *
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

static const double NANO_PER_USEC   = 1.0e3;
static const long int MILLION       = 1000000;

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function---currently null.
************************************************************************/

void
DYNINSTos_init(int calledByFork, int calledByAttach) {
}

static int cpuRollbackOccurred = 0;

/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * return value is in usec units.
************************************************************************/
time64 DYNINSTgetCPUtime(void) 
{
  time64        now;

  static time64 cpuPrevious = 0;

  /* I really hate to use an ifdef, but I don't want to toss the code.

     Getprocs: uses the same method as the dyninst library, but causes
     a SIGILL (illegal instruction) in the bubba program. 

     Rusage: the old (and working) method. Uses much less time than
     the getprocs method.
  */

  /*#define USE_GETPROCS_METHOD */
#ifdef USE_GETPROCS_METHOD

  /* Constant for the number of processes wanted in info */
  const unsigned int numProcsWanted = 1;
  struct procsinfo procInfoBuf[numProcsWanted];
  struct fdsinfo fdsInfoBuf[numProcsWanted];
  int numProcsReturned;
  pid_t wantedPid = getpid();

  const int sizeProcInfo = sizeof(struct procsinfo);
  const int sizeFdsInfo = sizeof(struct fdsinfo);
  time64 nanoseconds;

#else /* RUSAGE method */

  struct rusage ru;

#endif /* USE_GETPROCS_METHOD */

#ifndef USE_GETPROCS_METHOD

  if (getrusage(RUSAGE_SELF, &ru)) {
    perror("getrusage");
    abort();
  }
  
  now = (time64) ru.ru_utime.tv_sec + (time64) ru.ru_stime.tv_sec;
  now *= (time64) 1000000;
  now += (time64) ru.ru_utime.tv_usec + (time64) ru.ru_stime.tv_usec;

#else /* Using GETPROCS */
  
  numProcsReturned = getprocs(procInfoBuf,
			      sizeProcInfo,
			      fdsInfoBuf,
			      sizeFdsInfo,
			      &wantedPid,
			      numProcsWanted);

  if (numProcsReturned == -1) /* Didn't work */
    perror("Failure in getInferiorCPUtime");

  /* Get the user+sys time from the rusage struct in procsinfo */
  now = (time64) procInfoBuf[0].pi_ru.ru_utime.tv_sec + // User time
        (time64) procInfoBuf[0].pi_ru.ru_stime.tv_sec;  // System time

  now *= (time64) 1000000; /* Secs -> millions of microsecs */

  /* Though the docs say microseconds, the usec fields are in nanos */
  /* Note: resolution is ONLY hundredths of seconds */
  nanoseconds= (time64) procInfoBuf[0].pi_ru.ru_utime.tv_usec + 
               (time64) procInfoBuf[0].pi_ru.ru_stime.tv_usec;  

  now += nanoseconds / (time64) 1000; /* Nanos->micros */

#endif

  if (now < cpuPrevious) {
    if(! cpuRollbackOccurred) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback with current time: %lld msecs, using previous value %lld msecs.",now,cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred = 1;
    now = cpuPrevious;
  }
  else  cpuPrevious = now;

  return now;
}

static int wallRollbackOccurred = 0;

/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64 DYNINSTgetWalltime(void)
{
  static time64 wallPrevious = 0;
  time64        now;
#ifdef BERNAT_LIBCALL_FOR_TIME
  timebasestruct_t timestruct;
#else
  register unsigned int timeSec asm("5");
  register unsigned int timeNano asm("6");
  register unsigned int timeSec2 asm("7");

  static unsigned int old_timeSec = 0;
  static unsigned int old_timeNano = 0;
#endif
  
  /* Need to read the first value twice to make sure it doesn't roll
   *   over while we are reading it.
   */
  /* This code was disabled, since there are library routines that 
     do the same thing and are safer (in general, I don't like
     register reads) -- bernat
  */
  /* I'm back to it since it seems the library routines are "fuzzy". I don't
     know why */

#ifndef BERNAT_LIBCALL_FOR_TIME
  do {
    asm("mfspr   5,4");		/* read high into register 5 - timeSec */
    asm("mfspr   6,5");		/* read low into register 6 - timeNano */
    asm("mfspr   7,4");		/* read high into register 7 - timeSec2 */
  } while(timeSec != timeSec2);

  /* Check to be sure there isn't rollback. Quick check: if nano < old_nano and
     sec !> old_sec, there was a rollback */

#ifdef BERNAT_TEST_TO_DESTRUCTION
  if ((timeNano < old_timeNano) && (timeSec <= old_timeSec))
    /* Unfortunately, printing anything here will cause the app to die, so
       exit (not even assert) */
    abort();
#endif

  now = (time64) timeSec;
  now *= (time64) MILLION;
  now += (time64) timeNano;

#else

  read_real_time(&timestruct, TIMEBASE_SZ);
  time_base_to_time(&timestruct, TIMEBASE_SZ);

  /* convert to correct form. */
  now = (time64) timestruct.tb_high;
  now *= (time64) MILLION;
  now += (time64) timestruct.tb_low;
#endif

  if (now < wallPrevious) {
    if(! wallRollbackOccurred) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "Wall time rollback with current time: %lld msecs, using previous value %lld msecs.",now,wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred = 1;
    now = wallPrevious;
  }
  else  wallPrevious = now;

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
