/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: RTheap-irix.c,v 1.4 2004/03/23 01:12:16 eli Exp $ */
/* RTheap-irix.c: Irix-specific heap management */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <sys/procfs.h>               /* ioctl() */
#include <unistd.h>                   /* ioctl(), sbrk() */
#include <sys/mman.h>                 /* mmap() */
#include "dyninstAPI_RT/src/RTheap.h"


int     DYNINSTheap_align = 4; /* heaps are word-aligned */

/* avoid kernel, zero page, and stack */
#if (_MIPS_SZPTR == 64)
Address DYNINSTheap_loAddr = (Address)0x0000000000400000UL;
Address DYNINSTheap_hiAddr = (Address)0x000000ffffffffffUL;
#else
Address DYNINSTheap_loAddr = (Address)0x00400000UL;
Address DYNINSTheap_hiAddr = (Address)0x6fffffffUL;
#endif

int     DYNINSTheap_mmapFlags = MAP_FIXED | MAP_SHARED;


#define REGION_OFF        (28)
#define REGION_OFF_MASK   ((Address)((1 << REGION_OFF) - 1))
#define REGION_NUM_MASK   (~(Address)REGION_OFF_MASK)
#define region_num(addr)  (((Address)addr) >> REGION_OFF)
#define region_lo(addr)   (((Address)addr) & REGION_NUM_MASK)
#define region_hi(addr)   (region_lo(addr) | REGION_OFF_MASK)

RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
{
  Address lo_addr = (Address)lo;
  Address hi_addr = (Address)hi;
  Address sbrk_addr = (Address)sbrk(0);

  /* TODO: smarter malloc conditions */
  if (region_num(sbrk_addr) == region_num(lo_addr)) return RT_TRUE;
  if (region_num(sbrk_addr) == region_num(hi_addr)) return RT_TRUE;
  if (sbrk_addr >= lo_addr && sbrk_addr <= hi_addr) return RT_TRUE;
  return RT_FALSE;
}

int DYNINSTheap_mmapFdOpen(void)
{
  int fd = open("/dev/zero", O_RDWR);
  return fd;
}

void DYNINSTheap_mmapFdClose(int fd)
{
  close(fd);
}

