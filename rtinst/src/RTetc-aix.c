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
 * $Id: RTetc-aix.c,v 1.40 2003/04/11 22:46:46 schendel Exp $
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
#include "rtinst/h/rthwctr-aix.h"
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

/* We switch methods on the fly -- keep a previous for both */
rawTime64 wallPrevious_hw = 0;
rawTime64 wallPrevious_sw = 0;
rawTime64 cpuPrevious  = 0;

#if USES_PMAPI
/* For the hardware counters */
#include <pmapi.h>

pm_prog_t pdyn_pm_prog;
pm_info_t pinfo;
int using_groups = 0;
#ifdef PMAPI_GROUPS
pm_groups_info_t pginfo;
#endif

/* returns 0 on success, 1 on error */
int pmapi_bind_event_to_hwctr(pm_info_t *myinfo, int destination_mapping[],
                               pmapi_event_t event_type, 
                               int hwctr_num) {
   int num_avail_ctrs = myinfo->maxpmcs;
   int num_events = myinfo->maxevents[hwctr_num];
   pm_events_t *list_of_avail_events = myinfo->list_events[hwctr_num];
   char *event_string = get_event_string(hwctr_num);
   unsigned i;
   assert(hwctr_num < num_avail_ctrs);

   /* fprintf(stderr, "   num_avail_ctrs: %d, event_string: %s, num_events: %d"
              "\n", num_avail_ctrs, event_string, num_events); */
   for(i=0; i<num_events; i++) {
      pm_events_t cur_event = list_of_avail_events[i];
      /* fprintf(stderr, "      event: %d  str: %s\n", i, 
                 cur_event.short_name); */
      if(! strcmp(cur_event.short_name, event_string)) {
         /* fprintf(stderr, "      GOT A MATCH, mapping[%d] = %d\n",
                    hwctr_num, cur_event.event_id); */
         destination_mapping[hwctr_num] = cur_event.event_id;
         return 0;
      }
   }
   return 1;
}

/* returns 0 on success, 1 on error */
int pmapi_setup_bindings(pm_info_t *myinfo, int destination_mapping[]) {
   unsigned hwctr_num;
   pmapi_event_t cur_ev;
   for(hwctr_num = 0; hwctr_num < myinfo->maxpmcs; hwctr_num++)
      destination_mapping[hwctr_num] = -1;

   for(cur_ev=PM_FIRST_EVENT; cur_ev != PM_END_EVENT; cur_ev++) {
      int hwctr_to_assign = get_hwctr_binding(cur_ev);
      /* fprintf(stderr, "going to assign event %d to hwctr %d\n",
                 cur_ev, hwctr_to_assign); */
      int result = pmapi_bind_event_to_hwctr(myinfo, destination_mapping, 
                                             cur_ev, hwctr_to_assign);
      if(result == 1) return 1;
   }
   return 0;
}

void PARADYN_initialize_pmapi(int calledByFork) {
  // Zero out the program setup info
  int ret;
  pdyn_pm_prog.mode.w = 0;
#ifdef PMAPI_GROUPS
  // Check to see if we have a verified group, and if so use it. If not,
  // default to the old behavior
  ret = pm_init(PM_VERIFIED | PM_CAVEAT | PM_UNVERIFIED | PM_GET_GROUPS, &pinfo, &pginfo);
  if (pginfo.maxgroups) {
      // We have groups, set it up that way
      // ...
      fprintf(stderr, "Using PMAPI groups is not supported yet. Please contact paradyn@cs.wisc.edu.\n");
      using_groups = 1;
      pdyn_pm_prog.mode.b.is_group = 1;
  }
#else
  ret = pm_init(PM_VERIFIED | PM_CAVEAT | PM_UNVERIFIED, &pinfo);
#endif
  if (ret) pm_error("PARADYNos_init: pm_init", ret);
  
      
  if (!using_groups)
      if(pmapi_setup_bindings(&pinfo, pdyn_pm_prog.events))
          fprintf(stderr, "Mapping failed for pm events\n");

  /* Count both user and kernel instructions */
  pdyn_pm_prog.mode.b.user = 1;
  pdyn_pm_prog.mode.b.kernel = 1;
  /* Count for an entire process group. Catch all threads concurrently */
  pdyn_pm_prog.mode.b.process = 1;
  pdyn_pm_prog.mode.b.count  = 1;  /* start counting immediately,
                                      precludes the need for pm_start_...() */

  /* Prep the sucker for launch */
  /* we need to set up a group because we need to sample the cycle hwctr
     for all of the threads (in addition to, for each thread) */
  if(!calledByFork) {
     ret = pm_set_program_mygroup(&pdyn_pm_prog);
     if (ret) pm_error("PARADYNos_init: pm_set_program_mythread", ret); 
  } else {
     pm_delete_program_mythread();
     ret = pm_set_program_mygroup(&pdyn_pm_prog);
     if (ret) pm_error("PARADYNos_init: pm_set_program_mythread", ret); 
  }
}

void PARADYN_forkEarlyInit() {
   PARADYN_initialize_pmapi(1);
}

void PARADYNos_init(int calledByFork, int calledByAttach) {
  void *lib;
  hintBestCpuTimerLevel  = HARDWARE_TIMER_LEVEL;
  hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;

  if(! calledByFork) {
     /* should already have been initialized by parent process */
     PARADYN_initialize_pmapi(0);
  }

  /* needs to be reinitialized when fork occurs */
  wallPrevious_hw = 0;
  wallPrevious_sw = 0;
  cpuPrevious = 0;

}

#else
PARADYNos_init(int calledByFork, int calledByAttach) {
    hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
    hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;
  /* needs to be reinitialized when fork occurs */
  wallPrevious_hw = 0;
  wallPrevious_sw = 0;
  cpuPrevious = 0;
}

#endif /* USES_PMAPI */


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
  static int cpuRollbackOccurred = 0;
  pm_data_t data;
  int ret;
  rawTime64 now;

  ret = pm_get_data_mythread(&data);
  if (ret) pm_error("DYNINSTgetCPUtime: pm_get_data_mythread", ret);
  /* I'm pretty sure we want to use cycles completed, not instructions
     completed, using INSTR_CMPL could yield a total ratio > 1 for
     a superscalar processor */
  now = data.accu[get_hwctr_binding(PM_CYC_EVENT)];

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
  static int cpuRollbackOccurred = 0;
  rawTime64 now, tmp_cpuPrevious = cpuPrevious, us;
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
                tmp_cpuPrevious - now, now, tmp_cpuPrevious);
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

union bigWord {
  uint64_t b64;
  unsigned b32[2];
} bitGrabber;

/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64
DYNINSTgetWalltime_hw(void) {
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_hw;
  struct timebasestruct timestruct;
  read_real_time(&timestruct, TIMEBASE_SZ);
  bitGrabber.b32[0] = timestruct.tb_high;
  bitGrabber.b32[1] = timestruct.tb_low;
  now = bitGrabber.b64;

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld fast units, using previous value %lld fast units.",
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious_hw;
  }
  else wallPrevious_hw = now;
  return now;
}
 

/* Software Level --- 
   method:      read_real_time()
   return unit: nanoseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_sw;
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
                tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious_sw;
  }
  else { 
    wallPrevious_sw = now;
  }
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

unsigned PARADYNgetFD(unsigned lwp)
{
  return 0;
}

#ifdef MT_THREAD

rawTime64
DYNINSTgetCPUtime_LWP(unsigned lwp_id, unsigned fd) {
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
  if (ret) {
    pm_error("DYNINSTgetCPUtime_LWP: pm_get_data_thread", ret);
    fprintf(stderr, "Time requested for thread %d, running on thread %d\n", lwp_id, thread_self());
  }
  time = data.accu[get_hwctr_binding(PM_CYC_EVENT)]; 
  return time;
#else
  assert(0);
  return 0;
#endif
}
#endif
