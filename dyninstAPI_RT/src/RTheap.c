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

/* $Id: RTheap.c,v 1.12 2001/02/27 15:18:40 pcroth Exp $ */
/* RTheap.c: platform-generic heap management */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <sys/mman.h>                 /* mmap() */
#include "dyninstAPI_RT/src/RTheap.h"


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


static void heap_printMappings(int nmaps, dyninstmm_t *maps)
{
  int i;
  fprintf(stderr, "memory mappings:\n");
  for(i = 0; i < nmaps; i++) {
    dyninstmm_t *map = &maps[i];
    Address addr = (Address)map->pr_vaddr;
    fprintf(stderr, "  heap %2i: 0x%016lx-0x%016lx (%3i pages, %.2fkB)\n", 
	    i, addr, addr + map->pr_size - 1, 
	    map->pr_size / psize, map->pr_size / 1024.0);
  }
}

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

static Address heap_alignDown(Address addr, int align)
{
  if (addr % align == 0) return addr;
  return ((addr / align) + 0) * align;
}

#define BEG(x) ((Address)(x)->pr_vaddr)
#define END(x) ((Address)(x)->pr_vaddr + (x)->pr_size)

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

static Address
trymmap(size_t len, Address beg, Address end, size_t inc, int fd)
{
  Address try;
  void *result;
  for (try = beg; try + len <= end; try += len)
    {
    /*
    fprintf(stderr, "Calling mmap(addr = 0x%x, len = 0x%x, prot = 0x%x, flags = 0x%x)\n",
	    try, len, PROT_READ|PROT_WRITE|PROT_EXEC, DYNINSTheap_mmapFlags);
    */
    result = mmap((void *)try, len, 
		  PROT_READ|PROT_WRITE|PROT_EXEC,
		  DYNINSTheap_mmapFlags, 
		  fd, 
		  0);
    if (result != MAP_FAILED)
      {
	return (Address)result;
      }
    
    perror("mmap");
    /* Ugly. Can someone fix this? */
  }
  return 0;
}

/* Attempt to mmap a region of memory of size LEN bytes somewhere
   between LO and HI.  Returns the address of the region on success, 0
   otherwise.  MAPS is the current address space map, with NMAPS
   elements.  FD is the mmap file descriptor argument. */
static Address
constrained_mmap(size_t len, Address lo, Address hi,
		 const dyninstmm_t *maps, int nmaps, int fd)
{
     const dyninstmm_t *mlo, *mhi, *p;
     Address beg, end, try;
     /*
      * Interesting question -- what if the low address is
      * above the high address? Hrm... right now we die
      * in the assert below.
      */
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
     if (BEG(mlo) >= hi || END(mhi) <= lo)
	  return trymmap(len, lo, hi, psize, fd);

     assert(lo < BEG(mlo) && hi > END(mhi));

     /* Try to mmap in space before mlo */
     try = trymmap(len, lo, BEG(mlo), psize, fd);
     if (try)
	  return try;

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
  dyninstmm_t *a = (dyninstmm_t *)A;
  dyninstmm_t *b = (dyninstmm_t *)B;
  if (a->pr_vaddr < b->pr_vaddr) return -1;
  if (a->pr_vaddr > b->pr_vaddr) return 1;
  return 0;
}

void *DYNINSTos_malloc(size_t nbytes, void *lo_addr, void *hi_addr)
{
  void *heap;
  size_t size = nbytes;
  heapList_t *node = (heapList_t *)malloc(sizeof(heapList_t));
  /*
  fprintf(stderr, "*** DYNINSTos_malloc(%iB, 0x%016x, 0x%016x)\n", 
	  nbytes, lo_addr, hi_addr);
  */
  /* initialize page size */
  if (psize == -1) psize = getpagesize();

  /* buffer size must be aligned */
  if (size % DYNINSTheap_align != 0) return ((void *)-1);

  /* use malloc() if appropriate */
  if (DYNINSTheap_useMalloc(lo_addr, hi_addr)) {

    Address ret_heap;
    int size_heap = size + DYNINSTheap_align;
    heap = malloc(size_heap);
    if (heap == NULL) {
      free(node);
      return NULL;
    }
    ret_heap = heap_alignUp((Address)heap, DYNINSTheap_align);

    /* malloc buffer must meet range constraints */
    if (ret_heap < (Address)lo_addr ||
	ret_heap + size - 1 > (Address)hi_addr) {
      free(heap);
      free(node);
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
    int fd, nmaps;
    dyninstmm_t *maps;

    /* Get memory map and sort it.  maps will point to malloc'd memory
       that we must free. */
    if (0 > DYNINSTgetMemoryMap(&nmaps, &maps)) {
      free(node);
      return NULL;
    }
    qsort(maps, (size_t)nmaps, (size_t)sizeof(dyninstmm_t), &heap_memmapCompare);
    heap_checkMappings(nmaps, maps); /* sanity check */

    /*DYNINSTheap_printMappings(nmaps, maps);*/

    fd = DYNINSTheap_mmapFdOpen();
#ifndef alpha_dec_osf4_0
    if (0 > fd) {
	 free(node);
	 return NULL;
    }
#endif
    heap = (void*) constrained_mmap(size, lo, hi, maps, nmaps, fd);
    free(maps);
    DYNINSTheap_mmapFdClose(fd);
    if (!heap) {
	 free(node);
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
      if (munmap(heap->addr, heap->len) == -1) {
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

