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

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
#include <sys/thread.h>
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


/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * return value is in usec units.
************************************************************************/

time64 DYNINSTgetCPUtime(void) 
{
  time64        now;
  static time64 prevTime = 0;
  struct rusage ru;

  do {
    if (!getrusage(RUSAGE_SELF, &ru)) {
      now = (time64) ru.ru_utime.tv_sec + (time64) ru.ru_stime.tv_sec;
      now *= (time64) 1000000;
      now += (time64) ru.ru_utime.tv_usec + (time64) ru.ru_stime.tv_usec;
    } else {
      perror("getrusage");
      abort();
    }
  } while(prevTime > now);

  prevTime = now;
  return(now);
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64 DYNINSTgetWalltime(void)
{
  static time64 prevTime = 0;
  time64        now;

  register unsigned int timeSec asm("5");
  register unsigned int timeNano asm("6");
  register unsigned int timeSec2 asm("7");
  
  /* Need to read the first value twice to make sure it doesn't role
   *   over while we are reading it.
   */
  do {
    asm("mfspr   5,4");		/* read high into register 5 - timeSec */
    asm("mfspr   6,5");		/* read low into register 6 - timeNano */
    asm("mfspr   7,4");		/* read high into register 7 - timeSec2 */
  } while(timeSec != timeSec2);
  
  /* convert to correct form. */
  now = (time64) timeSec;
  now *= (time64) MILLION;
  now += (time64) timeNano/ (time64) 1000;

  if(prevTime > now) {
/*
    fprintf(stderr, "WARNING:  prevTime (%f) > now (%f)\n", 
	    (double) prevTime, (double) now);
*/
    return(prevTime);
  } else {
    prevTime = now;
    return(now);
  }
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
	case 5: /* system time - in mili-seconds */
	    value = 1000 * DYNINSTrusagePtr->ru_stime.tv_sec + 
	                   DYNINSTrusagePtr->ru_stime.tv_usec/1000;
	    break;
	default:
	    value = 0;
	    break;
    }
    return value;
}

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
extern unsigned DYNINST_hash_lookup(unsigned key);
extern unsigned DYNINST_initialize_done;
extern void DYNINST_initialize_hash(unsigned total);
extern void DYNINST_initialize_free(unsigned total);
extern unsigned DYNINST_hash_insert(unsigned k);

int DYNINSTthreadSelf(void) {
  return(thread_self());
}

int DYNINSTthreadPos(void) {
  if (DYNINST_initialize_done) {
    return(DYNINST_hash_lookup(DYNINSTthreadSelf()));
  } else {
    DYNINST_initialize_free(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_hash(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_done=1;
    return(DYNINST_hash_insert(DYNINSTthreadSelf()));
  }
}
#endif
