/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

/* RTheap-freebsd.c: FreeBSD-specific heap components */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>                   /* str* */
#include <assert.h>
#include <sys/types.h>
#include <sys/uio.h>                  /* read() */
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <unistd.h>                   /* sbrk(), read(), mmap */
#include <sys/mman.h>                 /* mmap() */
#include "RTheap.h"

#if defined(MUTATEE64)
int     DYNINSTheap_align = 4; /* heaps are word-aligned */

Address DYNINSTheap_loAddr = 0x4096;
Address DYNINSTheap_hiAddr = ~0x0;
#else
int     DYNINSTheap_align = 4; /* heaps are word-aligned */

Address DYNINSTheap_loAddr = 0x50000000;
Address DYNINSTheap_hiAddr = 0xb0000000;
#endif

int     DYNINSTheap_mmapFlags = MAP_FIXED | MAP_PRIVATE;


RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
{
  /* We do not save footprint space by allocating in
     the user's heap on this platform, so we stay out of it. */
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

int
DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
{
    assert(!"Unimplemented on FreeBSD for the time being");
    return -1;
}
