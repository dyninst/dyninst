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
 * $Id: RTetc-osf.c,v 1.3 2000/10/17 17:42:51 schendel Exp $
 * RTosf.c: mutatee-side library function specific to OSF
************************************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <sys/procfs.h>
#include <assert.h>
#include <fcntl.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"


static int ctr_procFd = -1;

void PARADYNos_init(int calledByFork, int calledByAttach)
{
  char fname[128];
  sprintf(fname, "/proc/%i", getpid());
  if ((ctr_procFd = open(fname, O_RDONLY)) == -1) {
    perror("DYNINSTinitCPUtime - open()");
    abort();
  }
}

/*static int MaxRollbackReport = 0; /* don't report any rollbacks!*/
static int MaxRollbackReport = 1; /* only report 1st rollback */
/*static int MaxRollbackReport = INT_MAX; /* report all rollbacks */

/* --- CPU time retrieval functions --- */
/* Hardware Level --- */
rawTime64 
DYNINSTgetCPUtime_hw(void) {
  return 0;
}

/* Software Level ---
   method:        gets out of proc fs
   return unit:   nanoseconds
*/
rawTime64
DYNINSTgetCPUtime_sw(void) {
/* return (user+sys) CPU time in microseconds (us) */
  static rawTime64 cpuPrevious=0;
  static int cpuRollbackOccurred=0;
  rawTime64 now, tmp_cpuPrevious=cpuPrevious;

  prpsinfo_t procinfo;
  
  if (ioctl(ctr_procFd, PIOCPSINFO, &procinfo) == -1) {
      perror("DYNINSTgetCPUtime - PIOCPSINFO");
      abort();
  }

  /* Put secs and nsecs into usecs */
  now = procinfo.pr_time.tv_sec;
  now *= I64_C(1000000000);
  now += procinfo.pr_time.tv_nsec;

  if (now < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time:"
                          " %lld ns, using previous value %lld ns.",
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

/* --- Wall time retrieval functions --- */
/* Hardware Level --- */
rawTime64
DYNINSTgetWalltime_hw(void) {
  return 0;
}

/* Software Level --- 
   method:      gettimeofday()
   return unit: microseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static rawTime64 wallPrevious=0;
  static int wallRollbackOccurred=0;
  rawTime64 now, tmp_wallPrevious=wallPrevious;

  struct timeval tv;
  if (gettimeofday(&tv,NULL) == -1) {
      perror("gettimeofday");
      assert(0);
      abort();
  }

  now = (rawTime64)tv.tv_sec*(rawTime64)1000000 + (rawTime64)tv.tv_usec;

  if (now < tmp_wallPrevious) {
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "Wall time rollback %lld with current time:"
                          " %lld usecs, using previous value %lld usecs.",
                          tmp_wallPrevious-now, now, tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData), &traceData, 1,
                                             1, 1);
    }
    wallRollbackOccurred++;
    now = wallPrevious;
  }
  else  wallPrevious = now;

  
  return(now);
}

