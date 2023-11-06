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

/* $Id: RTheap.h,v 1.10 2007/01/18 07:53:56 jaw Exp $ */

#ifndef _RT_HEAP_H
#define _RT_HEAP_H

#include "dyninstAPI_RT/h/dyninstAPI_RT.h" /* RT_Boolean, Address */
#include "dyntypes.h"

#if defined(os_linux) || defined(os_freebsd)

/* LINUX */
typedef struct {
     Address pr_vaddr;
     unsigned long pr_size;
} dyninstmm_t;

#elif defined(os_windows)
typedef struct {
  Address pr_vaddr;
  unsigned long pr_size;
} dyninstmm_t;

#else
#error Dynamic heaps are not implemented on this platform
#endif

/* 
 * platform-specific variables
 */

extern int     DYNINSTheap_align;
extern Address DYNINSTheap_loAddr;
extern Address DYNINSTheap_hiAddr;
extern int     DYNINSTheap_mmapFlags;


/* 
 * platform-specific functions
 */

RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi);
int        DYNINSTheap_mmapFdOpen(void);
void       DYNINSTheap_mmapFdClose(int fd);
int        DYNINSTheap_getMemoryMap(unsigned *, dyninstmm_t **mmap);

int DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp);

#endif /* _RT_HEAP_H */
