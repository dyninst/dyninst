/*
 * Copyright (c) 1998 Barton P. Miller
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

/* $Id: RTirix.c,v 1.1 1999/06/16 21:20:21 csserra Exp $ */

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "rtinst/h/rtinst.h"
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <sys/mman.h>                 /* mmap() */
#include <unistd.h>                   /* procfs, sbrk() */
#include <sys/procfs.h>               /* procfs */
#include <dlfcn.h>                    /* dlopen() */
#include <invent.h>                   /* getinvent() */
#include <sys/hwperftypes.h>          /* HW performance counters */
#include <sys/hwperfmacros.h>         /* HW performance counters */
#include <sys/syssgi.h>               /* free running cycle counter */
#include <time.h>                     /* clock(), clock_getres() */


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


/* hardward performance counter state */
#define HW_CTR_NUM (0)

char       DYNINSTos_CPUctr_use    = 0;
int        DYNINSTos_CPUctr_gen    = 0;
uint64_t   DYNINSTos_CPUctr_cycles = 0; // cycles per usec
char       DYNINSTos_wallCtr_use   = 0;
static int ctr_procFd        = -1;

void DYNINSTos_initCPUtime(void)
{
  char namebuf[64];
  inventory_t *inv;
  hwperf_profevctrarg_t ev;
  
  static int inited = 0;
  if (inited) return;
  inited = 1;
  /*fprintf(stderr, "*** DYNINSTinitCPUtime()\n");*/  
  
  sprintf(namebuf, "/proc/%i", getpid());
  /* TODO: avoid conflict with alternate versions of open() - necessary? */
  if ((ctr_procFd = open(namebuf, O_RDWR)) == -1) {
    perror("DYNINSTinitCPUtime - open()");
    abort();
  }
  
  /* check if all CPU (board) clock speeds are consistent;
   * if so, we can use the HW performance counters (PIOCGETEVCTRS);
   * if not, we have to use the slow clock (PIOCUSAGE) 
   */
  if (setinvent() == -1) {
    perror("DYNINSTinitCPUtime - setinvent()");
    abort();
  }
  DYNINSTos_CPUctr_use = 1;
  for (inv = getinvent(); inv != NULL; inv = getinvent()) {
    /* only need PROCESSOR/CPUBOARD inventory entries */
    if (inv->inv_class != INV_PROCESSOR) continue;
    if (inv->inv_type != INV_CPUBOARD) continue;
    /* check for clock speed mismatch */
    if (DYNINSTos_CPUctr_cycles == 0) DYNINSTos_CPUctr_cycles = inv->inv_controller;
    if (inv->inv_controller != DYNINSTos_CPUctr_cycles) {
      fprintf(stderr, "!!! inconsistent CPU speeds - cycle counter unusable\n");
      DYNINSTos_CPUctr_use = 0;
      break;
    }
  }
  endinvent();

  /* initialize HW performance counters (for CPU time) */
  if (DYNINSTos_CPUctr_use) {
    /* define cycle counter "event" */
    hwperf_ctrl_t *evctrl;
    memset(&ev, 0, sizeof(hwperf_profevctrarg_t)); /* important */
    evctrl = &ev.hwp_evctrargs.hwp_evctrl[HW_CTR_NUM];
    evctrl->hwperf_spec = 0;
    evctrl->hwperf_creg.hwp_ev = HWPERF_C0PRFCNT0_CYCLES;
    evctrl->hwperf_creg.hwp_ie = 0;
    evctrl->hwperf_creg.hwp_mode = HWPERF_CNTEN_U | HWPERF_CNTEN_K;
    /* enable event counters */
    /* (this will fail if not running as root) */
    if ((DYNINSTos_CPUctr_gen = ioctl(ctr_procFd, PIOCENEVCTRS, &ev)) == -1) {
      DYNINSTos_CPUctr_use = 0;
      return;
    }
  }
}

/* TODO: trap handler needed? */
void DYNINSTos_init(int calledByFork, int calledByAttach)
{
  /*fprintf(stderr, "*** DYNINSTos_init()\n");*/
  DYNINSTos_initCPUtime();
}

/* return (user+sys) CPU time in microseconds (us) */
time64 DYNINSTgetCPUtime(void)
{
  time64 ret;
  if (DYNINSTos_CPUctr_use) {
    hwperf_cntr_t count;
    int gen;
    if ((gen = ioctl(ctr_procFd, PIOCGETEVCTRS, &count)) == -1) {
      perror("DYNINSTgetCPUtime - PIOCGETEVCTRS");
      abort();
    }
    ret = count.hwp_evctr[HW_CTR_NUM] / DYNINSTos_CPUctr_cycles;
    /* counter generation numbers - see r10k_counters(5) */
    if (gen != DYNINSTos_CPUctr_gen) {
      fprintf(stderr, "!!! rtinst: hwperf counters generation number change\n");
      DYNINSTos_CPUctr_gen = gen;
    }
    /*fprintf(stderr, "*** DYNINSTgetCPUtime(cycles)\n");*/
  } else {
    prusage_t usage;
    if (ioctl(ctr_procFd, PIOCUSAGE, &usage) == -1) {
      perror("DYNINSTgetCPUtime - PIOCUSAGE");
      abort();
    }
    ret = mulMillion(usage.pu_utime.tv_sec + usage.pu_stime.tv_sec);
    ret += div1000(usage.pu_utime.tv_nsec + usage.pu_stime.tv_nsec);
    /*fprintf(stderr, "*** DYNINSTgetCPUtime(10ns)\n");*/
  }
  return ret;
}

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
  (*ctr_addr) = (uint64_t *)vaddr_timer;
      
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
  time64 ret;
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
  return ret;
}

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

