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
 * clock access functions for solaris-2.
 * $Id: RTetc-solaris.c,v 1.42 2003/01/06 19:27:26 bernat Exp $
 ************************************************************************/

#include <signal.h>
#include <sys/ucontext.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/syscall.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <stdio.h>
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* getpid() */
#include <limits.h>  /* for INT_MAX */

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

extern void perror(const char *);



extern void DYNINSTheap_setbounds();  /* RTheap-solaris.c */


/************************************************************************
 * symbolic constants.
************************************************************************/




static int procfd = -1;
rawTime64 cpuPrevious_hw = 0;
rawTime64 cpuPrevious_sw = 0;
rawTime64 wallPrevious_hw = 0;
rawTime64 wallPrevious_sw = 0;

/* PARADYNos_init formerly "void DYNINSTgetCPUtimeInitialize(void)" */
void PARADYNos_init(int calledByFork, int calledByAttach) {
   /* This must be done once for each process (including forked) children */
   char str[20];
   sprintf(str, "/proc/%d", (int)getpid());
   /* have to use syscall here for applications that have their own
      versions of open, poll...In these cases there is no guarentee that
      things have been initialized so that the application's version of
      open can be used when this open call occurs (in DYNINSTinit)
   */
   procfd = syscall(SYS_open,str, O_RDONLY);
   if (procfd < 0) {
      fprintf(stderr, "open of /proc failed in PARADYNos_init\n");
      perror("open");
      abort();
   }
   hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
   hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;

  /* needs to be reinitialized when fork occurs */
   cpuPrevious_hw  = 0;
   cpuPrevious_sw  = 0;
   wallPrevious_hw = 0;
   wallPrevious_sw = 0;
}

void PARADYN_forkEarlyInit() {
}



unsigned PARADYNgetFD(unsigned lwp)
{
    char lwpPath[256];
    sprintf(lwpPath, "/proc/self/lwp/%d", lwp);
    return open(lwpPath, O_RDONLY, 0);
}


/************************************************************************
 * rawTime64 DYNINSTgetCPUtime(void)
 *
 * get the total CPU time used for "an" LWP of the monitored process.
 * this functions needs to be rewritten if a per-thread CPU time is
 * required.  time for a specific LWP can be obtained via the "/proc"
 * filesystem.
 * return value is in usec units.
 *
************************************************************************/

rawTime64
DYNINSTgetCPUtime_LWP(unsigned lwp_id, unsigned fd) {
  hrtime_t lwpTime;
  rawTime64 now = 0;
  prusage_t theUsage;
  int needs_close = 0;

  if (lwp_id > 0) {
    if (!fd) {
        char lwpPath[256];
        sprintf(lwpPath, "/proc/self/lwp/%d/lwpusage", lwp_id);
        fd = open(lwpPath, O_RDONLY, 0);
        needs_close = 1;
    }

    if (fd != -1) {
        if (pread(fd, &theUsage, sizeof(prusage_t), 0) !=
            sizeof(prusage_t)) {
            assert(0);
        }
        now = (theUsage.pr_utime.tv_sec) * I64_C(1000000000); /* sec to nsec */
        now += theUsage.pr_utime.tv_nsec;
    }
  } else {
      lwpTime = gethrvtime();
      now = lwpTime;
  }
  if (needs_close) close(fd);
  return(now);  
}

/*static int MaxRollbackReport = 0; /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; /* only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */

/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  return 0;
}

/* Software Level ---
   method:      gethrvtime
   return unit: nanoseconds
*/
rawTime64
DYNINSTgetCPUtime_sw(void) {
  static int cpuRollbackOccurred = 0;
  rawTime64 now, tmp_cpuPrevious = cpuPrevious_sw;
  static int fd = 0;
#if 0
  now = gethrvtime();
#endif
  prusage_t theUsage;
  if (!fd) {
      char usage_fd[256];
      sprintf(usage_fd, "/proc/self/usage");
      fd = open(usage_fd, O_RDONLY, 0);
      if (fd == -1) assert(0);
  }
  if (pread(fd, &theUsage, sizeof(prusage_t), 0) !=
      sizeof(prusage_t)) {
      assert(0);
  }

  now =  (theUsage.pr_utime.tv_sec + theUsage.pr_stime.tv_sec) * 1000000000LL;
  now += (theUsage.pr_utime.tv_nsec+ theUsage.pr_stime.tv_nsec);
      
#ifndef MT_THREAD
  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time:"
	      " %lld nsecs, using previous value %lld nsecs.",
	      tmp_cpuPrevious - now, now, tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    cpuRollbackOccurred++;
    now = cpuPrevious_sw;
  }
  else  cpuPrevious_sw = now;
#endif
  return now;  
}

/* --- Wall time retrieval functions --- */
/* Hardware Level --- */
rawTime64
DYNINSTgetWalltime_hw(void) {
  return 0;
}

/* Software Level ---
   method:      gethrtime
   return unit: nanoseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static int wallRollbackOccurred = 0;
  rawTime64 now, tmp_wallPrevious = wallPrevious_sw;

  now = gethrtime();

#ifndef MT_THREAD
  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "Wall time rollback %lld with current time:"
	      " %lld nsecs, using previous value %lld nsecs.",
	      tmp_wallPrevious - now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
				 1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious_sw;
  }
  else  wallPrevious_sw = now;
#endif

  return now;
}
