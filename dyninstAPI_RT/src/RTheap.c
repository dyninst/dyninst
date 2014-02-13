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

/* $Id: RTheap.c,v 1.25 2006/05/03 00:31:25 jodom Exp $ */
/* RTheap.c: platform-generic heap management */

#include <stdlib.h>
#include <stdio.h>
#if !defined(os_windows) /* ccw 15 may 2000 : 29 mar 2001 */
	/* win does not have these header files.  it appears the only
	one that is used assert.h anyway.
	*/
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#else
extern int getpagesize();
#endif
#include <assert.h>

#include "dyninstAPI_RT/src/RTheap.h"
#include "dyninstAPI_RT/src/RTcommon.h"


typedef enum {
  HEAP_TYPE_UNKNOWN = 0x0,
  HEAP_TYPE_MMAP =    0x1,
  HEAP_TYPE_MALLOC =  0x2
} heapType_t;
    
typedef struct heap_t {
  void *ret_addr;  /* address returned to mutator */
  void *addr;      /* actual heap address */
  size_t len;      /* actual heap length */
  heapType_t type; /* heap allocation type */
} heap_t;

typedef struct heapList_t {
  heap_t heap;
  struct heapList_t *prev;
  struct heapList_t *next;
} heapList_t;


/* local variables */
static heapList_t *Heaps = NULL;
static int psize = -1;


/*
static void heap_printMappings(int nmaps, dyninstmm_t *maps)
{
  int i;
  fprintf(stderr, "memory mappings:\n");
  for(i = 0; i < nmaps; i++) {
    dyninstmm_t *map = &maps[i];
    Address addr = (Address)map->pr_vaddr;
    fprintf(stderr, "  heap %2i: 0x%016lx-0x%016lx (%3lu pages, %.2fkB)\n", 
	    i, addr, addr + map->pr_size - 1, 
	    map->pr_size / psize, map->pr_size / 1024.0);
  }
}
*/

static void heap_checkMappings(int nmaps, dyninstmm_t *maps)
{
  int i;
  for (i = 0; i < nmaps-1; i++) {
    if (maps[i].pr_vaddr + maps[i].pr_size > maps[i+1].pr_vaddr) {
      fprintf(stderr, "*** memory mappings overlap\n");
      abort();
    }
  }
}

static Address heap_alignUp(Address addr, int align)
{
  if (addr % align == 0) return addr;
  return ((addr / align) + 1) * align;
}

/*
static Address heap_alignDown(Address addr, int align)
{
  if (addr % align == 0) return addr;
  return ((addr / align) + 0) * align;
}
*/

#define BEG(x) ((Address)(x)->pr_vaddr)
#define END(x) ((Address)(x)->pr_vaddr + (x)->pr_size)

static Address trymmap(size_t len, Address beg, Address end, size_t inc, int fd)
{
  Address addr;
  void *result;
  /*We have a possibly large region (beg to end) and a hopefully smaller */
  /* allocation size (len).  We try to map at every page in the region*/
  /* until we get one that succeeds.*/
  for (addr = beg; addr + len <= end; addr += inc) {
    result = map_region((void *) addr, len, fd);
    if (result)
        return (Address) result;
  }
  return (Address) NULL;
}

/* Attempt to mmap a region of memory of size LEN bytes somewhere
   between LO and HI.  Returns the address of the region on success, 0
   otherwise.  MAPS is the current address space map, with NMAPS
   elements.  FD is the mmap file descriptor argument. */
static Address constrained_mmap(size_t len, Address lo, Address hi,
                                const dyninstmm_t *maps, int nmaps, int fd)
{
   const dyninstmm_t *mlo, *mhi, *p;
   Address beg, end, try;
#if defined (os_linux)  && defined(arch_power)
// DYNINSTheap_loAddr should already be defined in DYNINSTos_malloc. 
// Redefining here, just in case constrained_mmap is called from a different call path.
   DYNINSTheap_loAddr = getpagesize();
#endif

   if (lo > DYNINSTheap_hiAddr) return 0;

   if (lo < DYNINSTheap_loAddr) lo = DYNINSTheap_loAddr;
   if (hi > DYNINSTheap_hiAddr) hi = DYNINSTheap_hiAddr;

   /* Round down to nearest page boundary */
   lo = lo & ~(psize-1);
   hi = hi & ~(psize-1);

   /* Round up to nearest page boundary */
   if (len % psize) {
      len += psize;
      len = len & ~(psize-1);
   }

   assert(lo < hi);
   /* Find lowest (mlo) and highest (mhi) segments between lo and
      hi.  If either lo or hi occurs within a segment, they are
      shifted out of it toward the other bound. */
   mlo = maps;
   mhi = &maps[nmaps-1];
   while (mlo <= mhi) {
      beg = BEG(mlo);
      end = END(mlo);

      if (lo < beg)
         break;

      if (lo >= beg && lo < end)
         /* lo occurs in this segment.  Shift lo to end of segment. */
         lo = end; /* still a page boundary */

      ++mlo;
   }
	     
   while (mhi >= mlo) {
      beg = BEG(mhi);
      end = END(mhi);

      if (hi > end)
         break;
      if (hi >= beg && hi <= end)
         /* hi occurs in this segment (or just after it).  Shift
            hi to beginning of segment. */
         hi = beg; /* still a page boundary */

      --mhi;
   }
   if (lo >= hi)
      return 0;

   /* We've set the bounds of the search, now go find some free space. */

   /* Pathological cases in which the range (lo,hi) is entirely
      above or below the rest of the address space, or there are no
      segments between lo and hi.  Return no matter what from
      here. */
   if (BEG(mlo) >= hi || END(mhi) <= lo) {
      return trymmap(len, lo, hi, psize, fd);
   }
   assert(lo < BEG(mlo) && hi > END(mhi));
   /* Try to mmap in space before mlo */
   try = trymmap(len, lo, BEG(mlo), psize, fd);
   if (try) {
      return try;
   }

   /* Try to mmap in space between mlo and mhi.  Try nothing here if
      mlo and mhi are the same. */
   for (p = mlo; p < mhi; p++) {
      try = trymmap(len, END(p), BEG(p+1), psize, fd);
      if (try)
         return try;
   }

   /* Try to mmap in space between mhi and hi */
   try = trymmap(len, END(mhi), hi, psize, fd);
   if (try)
      return try;

   /* We've tried everything */
   return 0;
}
#undef BEG
#undef END


static int heap_memmapCompare(const void *A, const void *B)
{
  const dyninstmm_t *a = (const dyninstmm_t *)A;
  const dyninstmm_t *b = (const dyninstmm_t *)B;
  if (a->pr_vaddr < b->pr_vaddr) return -1;
  if (a->pr_vaddr > b->pr_vaddr) return 1;
  return 0;
}

void *DYNINSTos_malloc(size_t nbytes, void *lo_addr, void *hi_addr)
{
  void *heap;
  size_t size = nbytes;
  heapList_t *node = (heapList_t *)malloc(sizeof(heapList_t));

  /* initialize page size */
  if (psize == -1) psize = getpagesize();

  /* buffer size must be aligned */
  if (size % DYNINSTheap_align != 0) {
    free(node);
    return ((void *)-1);
  }

  /* use malloc() if appropriate */
  if (DYNINSTheap_useMalloc(lo_addr, hi_addr)) {

    Address ret_heap;
    int size_heap = size + DYNINSTheap_align;
    heap = malloc(size_heap);
    if (heap == NULL) {
      free(node);
#ifdef DEBUG
      fprintf(stderr, "Failed to MALLOC\n");      
#endif 
      return NULL;
    }
    ret_heap = heap_alignUp((Address)heap, DYNINSTheap_align);
    
    /* malloc buffer must meet range constraints */
    if (ret_heap < (Address)lo_addr ||
        ret_heap + size - 1 > (Address)hi_addr) {
      free(heap);
      free(node);
#ifdef DEBUG
      fprintf(stderr, "MALLOC'd area fails range constraints\n");
#endif 
      return NULL;
    }

    /* define new heap */
    node->heap.ret_addr = (void *)ret_heap;
    node->heap.addr = heap;
    node->heap.len = size_heap;
    node->heap.type = HEAP_TYPE_MALLOC;


  } else { /* use mmap() for allocation */
    Address lo = (Address) lo_addr;
    Address hi = (Address) hi_addr;
    int fd;
    unsigned nmaps;
    dyninstmm_t *maps;

    /* What if we need to allocate memory not in the area we can mmap? */
#if defined (os_linux)  && defined(arch_power)
   DYNINSTheap_loAddr = getpagesize();
#endif
    if ((hi < DYNINSTheap_loAddr) || (lo > DYNINSTheap_hiAddr)) {
      free(node);
#ifdef DEBUG
      fprintf(stderr, "CAN'T MMAP IN RANGE GIVEN\n");
#endif 
      return NULL;
    }
    

    /* Get memory map and sort it.  maps will point to malloc'd memory
       that we must free. */
    if (0 > DYNINSTgetMemoryMap(&nmaps, &maps)) {
      free(node);
#ifdef DEBUG
      fprintf(stderr, "failed MMAP\n");
#endif 
      return NULL;
    }
    qsort(maps, (size_t)nmaps, (size_t)sizeof(dyninstmm_t), &heap_memmapCompare);
    heap_checkMappings(nmaps, maps); /* sanity check */

    /*DYNINSTheap_printMappings(nmaps, maps);*/

    fd = DYNINSTheap_mmapFdOpen();
    if (0 > fd) {
      free(node);
      free(maps);
      return NULL;
    }
    heap = (void*) constrained_mmap(size, lo, hi, maps, nmaps, fd);
    free(maps);
    DYNINSTheap_mmapFdClose(fd);
    if (!heap) {
       free(node);
#ifdef DEBUG
       fprintf(stderr, "failed MMAP(2)\n");
#endif 
       return NULL;
    }

    /* define new heap */
    node->heap.ret_addr = heap;
    node->heap.addr = heap;
    node->heap.len = size;
    node->heap.type = HEAP_TYPE_MMAP;
  }

  /* insert new heap into heap list */
  node->prev = NULL;
  node->next = Heaps;
  if (Heaps) Heaps->prev = node;
  Heaps = node;
  
  return node->heap.ret_addr;
}

int DYNINSTos_free(void *buf)
{
  int ret = 0;
  heapList_t *t;
  /*
  fprintf(stderr, "*** DYNINSTos_free(0x%08x)\n", buf);
  */
  for (t = Heaps; t != NULL; t = t->next) {
    /* lookup heap by (returned) address */
    heap_t *heap = &t->heap;
    if (heap->ret_addr != buf) continue;

    /* remove heap from list */
    if (t->next) t->next->prev = t->prev;
    if (t->prev) t->prev->next = t->next;
    if (Heaps == t) Heaps = t->next;

    /* deallocate heap */
    switch (heap->type) {
    case HEAP_TYPE_MMAP:
      if (!unmap_region(heap->addr, heap->len)) {
        perror("DYNINSTos_free(munmap)");
        ret = -1;
      }
      break;
    case HEAP_TYPE_MALLOC:
      free(heap->addr);
      break;
    default:
      fprintf(stderr, "DYNINSTos_free(): unknown inferior heap type\n");
      ret = -1;
      break;
    }

    /* free list element */
    free(t);
    break;
  }

  return ret;
}

