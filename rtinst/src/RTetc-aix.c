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
 * $Id: RTetc-aix.c,v 1.28 2001/11/06 19:20:58 bernat Exp $
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
#include <dlfcn.h>

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

void DYNINSTstaticHeap_1048576_textHeap_libSpace(void);

#if USES_PMAPI
/* For the hardware counters */
#include <pmapi.h>

#define MAX_EVENTS	2
#define CMPL_INDEX	0
#define CYC_INDEX	1

/* Event 1: instructions completed (per kernel thread)
   Event 2: total machine cycles completed */

char *pdyn_search_evs[MAX_EVENTS] = {"PM_INST_CMPL", "PM_CYC"};
int pdyn_counter_mapping[MAX_EVENTS];
pm_prog_t pdyn_pm_prog;
pm_info_t pinfo;

/* 
   Given a list of (string) events, try and get a mapping for them. We
   use this because we know what events we want, but not (necessarily) the 
   mapping.

   So when this is done, you can get event foo by asking for 
   event.mappings[# of foo]
*/

int
pdyn_search_cpi(pm_info_t *myinfo, int evs[], int mappings[])
{
  int             s_index, pmc, ev, found = 0;
  pm_events_t     *wevp;
  
  for (pmc = 0; pmc < myinfo->maxpmcs; pmc++)
    evs[pmc] = -1;
  for (pmc = 0; pmc < MAX_EVENTS; pmc++)
    mappings[pmc] = -1;
  
  for (s_index = 0; s_index < MAX_EVENTS; s_index++) {
    found = 0;
    for (pmc = 0; pmc < myinfo->maxpmcs; pmc++) {
      wevp = myinfo->list_events[pmc];
      for (ev = 0; ev < myinfo->maxevents[pmc]; ev++, wevp++) {
	if ((evs[pmc] == -1) && 
	    (strcmp(pdyn_search_evs[s_index], wevp->short_name) == 0)) {
	  evs[pmc] = wevp->event_id;
	  mappings[s_index] = pmc;
	  break;
	}
      }
      if (mappings[s_index] != -1) break;
    }
  }
  for (pmc = 0; pmc < MAX_EVENTS; pmc++)
    if (mappings[pmc] == -1)
      return -1;
  return(0);
}


void
PARADYNos_init(int calledByFork, int calledByAttach) {
  int ret;
  void *lib;

  ret = pm_init(PM_VERIFIED | PM_CAVEAT | PM_UNVERIFIED, &pinfo);
  if (ret) pm_error("PARADYNos_init: pm_init", ret);
  
  if (pdyn_search_cpi(&pinfo, pdyn_pm_prog.events, pdyn_counter_mapping))
    fprintf(stderr, "Mapping failed for pm events\n");

  /* Count both user and kernel instructions */
  pdyn_pm_prog.mode.w = 0;
  pdyn_pm_prog.mode.b.user = 1;
  pdyn_pm_prog.mode.b.kernel = 1;
  /* Count for an entire process group. Catch all threads concurrently */
  pdyn_pm_prog.mode.b.process = 1;

  /* Prep the sucker for launch */
  ret = pm_set_program_mythread(&pdyn_pm_prog);
  if (ret) pm_error("PARADYNos_init: pm_set_program_mythread", ret); 

  /* Start it up, and let 'er rip */
  ret = pm_start_mythread();
  if (ret) pm_error("PARADYNos_init: pm_start_mythread", ret);

#ifdef USES_LIB_TEXT_HEAP
  /* Dummy call to get the library space actually included
     (not pruned by an optimizing linker) */
  DYNINSTstaticHeap_1048576_textHeap_libSpace();
#endif
}

#else
PARADYNos_init(int calledByFork, int calledByAttach) {
    hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
    hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;
#ifdef USES_LIB_TEXT_HEAP
  DYNINSTstaticHeap_1048576_textHeap_libSpace();
#endif
}

#endif USES_PMAPI
/* We'll see an occasional small rollback ( ~ 1 usec) that is ignored. */
static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1;*/ /* only report 1st rollback */
/*static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
/* This uses the PMAPI functions */
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  static int no_pmapi_error = 0;
#ifdef USES_PMAPI
  /* Get time for my thread */
  static int initialized = 0;
  static int cpuRollbackOccurred = 0;
  pm_data_t data;
  int ret;
  rawTime64 now;
  static rawTime64 cpuPrevious = 0;

  if (!initialized) {
    pm_set_program_mythread(&pdyn_pm_prog);
    initialized = 1;
  }
  ret = pm_get_data_mythread(&data);
  if (ret) pm_error("DYNINSTgetCPUtime: pm_get_data_mythread", ret);
  /* I'm pretty sure we want to use cycles completed, not instructions
     completed, using INSTR_CMPL could yield a total ratio > 1 for
     a superscalar processor */
  now = data.accu[pdyn_counter_mapping[CYC_INDEX]];

  if (now < cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld us, using previous value %lld us.",
                cpuPrevious-now, now, cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious;
  }
  else  cpuPrevious = now;
  
  //fprintf(stderr, "HARDWARE: %lld (%d)\n", now, thread_self());
  return now;
#else
  if (!no_pmapi_error)
    fprintf(stderr, "Error: hardware method in use without PMAPI defined\n");
  no_pmapi_error = 1;
  return 0;
#endif
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

#ifdef MT_THREAD
rawTime64
DYNINSTgetCPUtime_LWP(int lwp_id) {
#ifdef USES_PMAPI
  /* Get time for my thread */
  pm_data_t data;
  int ret;
  rawTime64 time;
  unsigned int lwp_self = thread_self();
  if (lwp_id == lwp_self)
    {
      ret = pm_get_data_mythread(&data);
    }
  else
    ret = pm_get_data_thread(getpid(), lwp_id, &data);
  if (ret) pm_error("DYNINSTgetCPUtime_LWP: pm_get_data_thread", ret);
  
  time = data.accu[pdyn_counter_mapping[0]]; 

  /*  fprintf(stderr, "CPU---perthread: %lld\n", time); */

  return time;
#else
  return 0;
#endif
}
#endif
