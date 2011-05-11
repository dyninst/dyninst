/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: RTheap-solaris.c,v 1.10 2004/03/23 01:12:16 eli Exp $ */
/* RTheap-solaris.c: Solaris-specific heap components */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>                  /* read() */
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <procfs.h>
#include <unistd.h>                   /* ioctl(), sbrk(), read() */
#include <sys/mman.h>                 /* mmap() */
#include "dyninstAPI_RT/src/RTheap.h"


int     DYNINSTheap_align = 4; /* heaps are word-aligned */

Address DYNINSTheap_loAddr;
Address DYNINSTheap_hiAddr;

int     DYNINSTheap_mmapFlags = MAP_FIXED | MAP_PRIVATE;


RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
{
  Address lo_addr = (Address)lo;
  Address hi_addr = (Address)hi;
  Address sbrk_addr = (Address)sbrk(0);

#if defined(i386_unknown_solaris2_5)
  /* We do not save footprint space by allocating in
     the user's heap on this platform, so we stay out of it. */
  return RT_FALSE;
#else
  if (lo_addr <= sbrk_addr + 0x800000) return RT_TRUE;
  return RT_FALSE;
#endif
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

/* 
   Set the bounds for heap allocations.  Currently we ensure that heap
   space is not allocated ON the stack on x86, or ABOVE the stack on
   SPARC.  */
void DYNINSTheap_setbounds()
{
     int fd;
     pstatus_t ps;
     ssize_t ret;

     fd = open("/proc/self/status", O_RDONLY);
     if (0 > fd) {
	  perror("open /proc/self/status");
	  return;
     }
     ret = read(fd, &ps, sizeof(ps));
     if (0 > ret) {
	  perror("read /proc/self/status");
	  close(fd);
	  return;
     }
     close(fd);
     if (ret != sizeof(ps)) {
	  fprintf(stderr, "Unexpected structure size in /proc/self/status\n");
	  return;
     }

#if defined(sparc_sun_solaris2_4)
     DYNINSTheap_loAddr = 0;
     DYNINSTheap_hiAddr = (Address)ps.pr_stkbase;
     /* shrink this bound to allow stack room to grow */
     assert(DYNINSTheap_hiAddr > 128*1024);
     DYNINSTheap_hiAddr -= 128*1024;
#elif defined(i386_unknown_solaris2_5)
     /* x86 Solaris memory layout (ca. Solaris 2.6):

     	     -lowest address-
     	     Stack
     	     a.out (executable)
     	     Heap
     
     	     Shared libraries
     
     	     -highest address-

	 For programs that make heavy use of the heap, we need to keep
	 away from memory near the 
     */
     assert(ps.pr_brkbase > ps.pr_stkbase); /* Expect heap to be above stack */
     assert(ps.pr_brkbase < 0xd0000000);
     DYNINSTheap_loAddr = (Address)0xd0000000;
     DYNINSTheap_hiAddr = (Address)0xdfffffff; /* Userspace limit? */
     assert(DYNINSTheap_loAddr < DYNINSTheap_hiAddr);
#else
/* To date these bounds work on 32-bit x86 and SPARC Solaris through 2.7. */
#error unknown architecture
#endif
}
