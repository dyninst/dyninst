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

/* $Id: RTetc-irix.c,v 1.7 2000/08/08 15:25:52 wylie Exp $ */

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

/* see comments in RTsolaris.c */
static unsigned long long div1000(unsigned long long in) 
{
   unsigned long long temp = in << 20; /* multiply by 1,048,576 */
   temp += in << 14; /* 16384 */
   temp += in << 13; /* 8192  */
   temp += in << 9;  /* 512   */
   temp += in << 6;  /* 64    */
   temp += in << 4;  /* 16    */
   temp -= in >> 2;  /* 2     */
   return (temp >> 30); /* divide by 2^30 */
}

/* see comments in RTsolaris.c */
static unsigned long long mulMillion(unsigned long long in) {
   unsigned long long result = in;
   result = (result << 7) - result - result - result;
   result = (result << 7) - result - result - result;
   result <<= 6;
   return result;
}

float DYNINSTos_cyclesPerSecond(void)
{
  float ret = 0.0;
  
  if (setinvent() != -1) {
    unsigned raw = 0;
    inventory_t *inv;
    for (inv = getinvent(); inv != NULL; inv = getinvent()) {
      /* only need PROCESSOR/CPUBOARD inventory entries */
      if (inv->inv_class != INV_PROCESSOR) continue;
      if (inv->inv_type != INV_CPUBOARD) continue;
      /* check for clock speed mismatch */
      if (raw == 0) raw = inv->inv_controller;
      if (inv->inv_controller != raw) {
	fprintf(stderr, "!!! non-uniform CPU speeds\n");
	break;
      }
    }
    endinvent();
    ret = raw * (float)1000000.0; /* convert MHz to Hz */
  }

  return ret;
}

/* timer state */
char       DYNINSTos_wallCtr_use = 0;
static int ctr_procFd            = -1;

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
}


/*
 * CPU timers 
 */

/*static int MaxRollbackReport = 0; */  /* don't report any rollbacks! */
/*static int MaxRollbackReport = 1; */  /*  only report 1st rollback */
static int MaxRollbackReport = INT_MAX; /* report all rollbacks */


/* return (user+sys) CPU time in microseconds (us) */
time64 DYNINSTgetCPUtime(void)
{
  static time64 cpuPrevious;
  static int cpuRollbackOccurred = 0;
  time64 ret, tmp_cpuPrevious=cpuPrevious;

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
  ret = mulMillion(t[AS_USR_RUN].tv_sec + t[AS_SYS_RUN].tv_sec);
  ret += div1000(t[AS_USR_RUN].tv_nsec + t[AS_SYS_RUN].tv_nsec);

  if (ret < tmp_cpuPrevious) {
    if (cpuRollbackOccurred < MaxRollbackReport) {  
      rtUIMsg traceData;
      sprintf(traceData.msgString, "CPU time rollback %lld with current time: "
	      "%lld usecs, using previous value %lld usecs.",
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


/* 
 * wall timers 
 */

static uint64_t ctr_gcd(uint64_t a, uint64_t b)
{
  if (b == 0) return a;
  return ctr_gcd(b, a % b);
}

static int ctr_mapCycleCounter(uint64_t **ctr_addr, 
			       uint64_t *ctr_numer, 
			       uint64_t *ctr_denom)
{
  ptrdiff_t paddr_timer_;
  unsigned res_psec = 0;
  uint64_t paddr_timer, paddr_page, vaddr_timer;
  uint64_t pageoffmask;
  int mmap_fd;
  void *vaddr_page;
  uint64_t ctr_div = 1;

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
      
  /* simplify conversion ratio */
  (*ctr_numer) = res_psec;
  (*ctr_denom) = 1000000; /* psec/usec */
  ctr_div = ctr_gcd((*ctr_denom), (*ctr_numer));
  (*ctr_numer) /= ctr_div;
  (*ctr_denom) /= ctr_div;

  return 0;
}


time64 DYNINSTgetWalltime(void)
{
  static time64 wallPrevious;
  static int wallRollbackOccurred = 0;
  time64 ret, tmp_wallPrevious=wallPrevious;
  static uint64_t *ctr_addr  = NULL;
  static uint64_t  ctr_numer = 1;
  static uint64_t  ctr_denom = 1;  
  static int inited = 0;
  
  /*fprintf(stderr, "*** DYNINSTgetWalltime()\n");*/
      
  /* initialize cycle counter */
  if (!inited) {
    if (ctr_mapCycleCounter(&ctr_addr, &ctr_numer, &ctr_denom) == 0) {
      /* wall time initialization successful */
      DYNINSTos_wallCtr_use = 1;
    }
    inited = 1; 
  }

  /* TODO: bypass high resolution wall time (TEMPORARY) */
  DYNINSTos_wallCtr_use = 0;

  /* sample wall time */
  if (DYNINSTos_wallCtr_use) {
    ret = (*ctr_addr);
    if (ctr_numer != 1) ret *= ctr_numer;
    if (ctr_denom != 1) ret /= ctr_denom;
    /*fprintf(stderr, "*** DYNINSTgetWalltime(800ns)\n");*/
  } else {
    struct timeval tv;
    gettimeofday(&tv);
    ret = mulMillion(tv.tv_sec) + tv.tv_usec;
    /*fprintf(stderr, "*** DYNINSTgetWalltime(10ns)\n");*/
  }

  /*fprintf(stderr, "*** 0x%016llx us: DYNINSTgetWalltime()\n", ret);*/

  if (ret < tmp_wallPrevious) {  
    if (wallRollbackOccurred < MaxRollbackReport) {
      rtUIMsg traceData;
      sprintf(traceData.msgString, "Wall time rollback %lld with current time: "
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

