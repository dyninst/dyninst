/*
 * Copyright (c) 1998-2000 Barton P. Miller
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

/* $Id: RTetc-irix.c,v 1.8 2000/10/17 17:42:51 schendel Exp $ */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <sys/mman.h>                 /* mmap() */
#include <unistd.h>                   /* procfs, sbrk() */
#include <sys/procfs.h>               /* procfs */
#include <dlfcn.h>                    /* dlopen() */
#include <sys/syssgi.h>               /* free running cycle counter */
#include <time.h>                     /* clock(), clock_getres() */
#include <sys/timers.h>               /* PTIMER macros */
#include <invent.h>                   /* getinvent() */
#include <limits.h>                   /* for INT_MAX */


/*
 * RTinst functions
 */
/* timer state */
static int ctr_procFd = -1;
uint64_t *walltime_ctr_addr  = NULL;

int ctr_mapCycleCounter(uint64_t **ctr_addr)
{
  ptrdiff_t paddr_timer_;
  unsigned res_psec = 0;
  uint64_t paddr_timer, paddr_page, vaddr_timer;
  uint64_t pageoffmask;
  int mmap_fd;
  void *vaddr_page;

  /* 32-bit counter wraps too quickly */
  if (syssgi(SGI_CYCLECNTR_SIZE) != 64) return -1;
  
  /* mmap() cycle counter */
  paddr_timer_ = syssgi(SGI_QUERY_CYCLECNTR, &res_psec);
  if (res_psec == 0) return -1;
  if (paddr_timer_ == -1) return -1;
  paddr_timer = (uint64_t)paddr_timer_;
  mmap_fd = open("/dev/mmem", O_RDONLY);
  if (mmap_fd == -1) return -1;
  pageoffmask = getpagesize() - 1;
  paddr_page = paddr_timer & ~pageoffmask;
  vaddr_page = mmap64(0, pageoffmask+1, PROT_READ, MAP_PRIVATE,
		      mmap_fd, (off64_t)paddr_page);
  close(mmap_fd);
  if (vaddr_page == MAP_FAILED) return -1;
  vaddr_timer = ((uint64_t)vaddr_page) + (paddr_timer & pageoffmask);
  (*ctr_addr) = (uint64_t *)(ulong_t)vaddr_timer;
      
  return 0;
}

void PARADYNos_init(int calledByFork, int calledByAttach)
{
  char fname[128];
  RTprintf("*** PARADYNos_init()\n");
  sprintf(fname, "/proc/%i", getpid());
  /* TODO: avoid conflict with alternate versions of open() - necessary? */
  if ((ctr_procFd = open(fname, O_RDONLY)) == -1) {
    perror("PARADYNos_init - open()");
    abort();
  }
  hintBestCpuTimerLevel  = SOFTWARE_TIMER_LEVEL;
  if (ctr_mapCycleCounter(&walltime_ctr_addr) == 0) {
    // high resolution wall time initialization successful
    hintBestWallTimerLevel = HARDWARE_TIMER_LEVEL;
  } else {
    hintBestWallTimerLevel = SOFTWARE_TIMER_LEVEL;
  }
}


/* see comments in RTsolaris.c */
static unsigned long long mulMillion(unsigned long long in) {
   unsigned long long result = in;
   result = (result << 7) - result - result - result;
   result = (result << 7) - result - result - result;
   result <<= 6;
   return result;
}


/*static int MaxRollbackReport = 0; */  /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; */  /*  only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


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
  static rawTime64 cpuPrevious;
  static int cpuRollbackOccurred = 0;
  rawTime64 ret, tmp_cpuPrevious=cpuPrevious;

  /*
  pracinfo_t t;
  ioctl(ctr_procFd, PIOCACINFO, &t);
  ret = div1000(t.pr_timers.ac_utime + t.pr_timers.ac_stime);
  */

  timespec_t t[MAX_PROCTIMER];
  if (ioctl(ctr_procFd, PIOCGETPTIMER, t) == -1) {
    perror("getInferiorProcessCPUtime - PIOCGETPTIMER");
    abort();
  }
  ret = (t[AS_USR_RUN].tv_sec + t[AS_SYS_RUN].tv_sec) * I64_C(1000000000);
  ret += (t[AS_USR_RUN].tv_nsec + t[AS_SYS_RUN].tv_nsec);

  if (ret < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {  
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld nsecs, using previous value %lld nsecs.",
                tmp_cpuPrevious-ret,ret,tmp_cpuPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData),&traceData,
				 1, 1, 1);
    }
    cpuRollbackOccurred++;
    ret = cpuPrevious;
  }
  else  cpuPrevious = ret;

  return ret;
}


/* --- Wall time retrieval functions --- */
/* Hardware Level --- 
   method:      free running hardware counter, address of value in mmapped
   return unit: resolution discovered at runtime
*/
rawTime64
DYNINSTgetWalltime_hw(void) {
  static rawTime64 wallPrevious = 0;
  static int wallRollbackOccurred = 0;
  rawTime64 ret, tmp_wallPrevious=wallPrevious;

  ret = (*walltime_ctr_addr);
  if (ret < tmp_wallPrevious) {  
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld raw units, using previous value %lld raw units.",
                tmp_wallPrevious-ret,ret,tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData),&traceData,
				 1, 1, 1);
    }
    wallRollbackOccurred++;
    ret = wallPrevious;
  }
  else  wallPrevious = ret;

  return ret;
}

/* Software Level ---
   method:        gettimeofday()
   return unit:   microseconds
*/
rawTime64
DYNINSTgetWalltime_sw(void) {
  static rawTime64 wallPrevious = 0;
  static int wallRollbackOccurred = 0;
  rawTime64 ret, tmp_wallPrevious=wallPrevious;
  
  /* sample wall time */
  struct timeval tv;
  gettimeofday(&tv);
  ret = mulMillion(tv.tv_sec) + tv.tv_usec;

  /*fprintf(stderr, "*** 0x%016llx us: DYNINSTgetWalltime()\n", ret);*/

  if (ret < tmp_wallPrevious) {  
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString,"Wall time rollback %lld with current time: "
	      "%lld usecs, using previous value %lld usecs.",
                tmp_wallPrevious-ret,ret,tmp_wallPrevious);
      traceData.errorNum = 112;
      traceData.msgType = rtWarning;
      DYNINSTgenerateTraceRecord(0, TR_ERROR, sizeof(traceData),&traceData,
				 1, 1, 1);
    }
    wallRollbackOccurred++;
    ret = wallPrevious;
  }
  else  wallPrevious = ret;

  return ret;
}


/* 
 * multithreading 
 */

#if defined(SHM_SAMPLING) && defined(MT_THREAD)
/* TODO: implement */
int DYNINSTthreadPos(void)
{
  return 0;
}

/* TODO: implement */
int DYNINSTthreadSelf(void)
{
  return 0;
}
#endif

