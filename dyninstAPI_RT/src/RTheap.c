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

/* $Id: RTheap.c,v 1.3 1999/11/11 00:58:16 wylie Exp $ */
/* RTheap.c: platform-generic heap management */

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


static void heap_printMappings(int nmaps, prmap_t *maps)
{
  int i;
  fprintf(stderr, "memory mappings:\n");
  for(i = 0; i < nmaps; i++) {
    prmap_t *map = &maps[i];
    Address addr = (Address)map->pr_vaddr;
    fprintf(stderr, "  heap %2i: 0x%016lx-0x%016lx (%3i pages, %.2fkB)\n", 
	    i, addr, addr + map->pr_size - 1, 
	    map->pr_size / psize, map->pr_size / 1024.0);
  }
}

static void heap_getMappings(int *nmaps, prmap_t **maps)
{
  int proc_fd;
  char proc_name[256];
  /*fprintf(stderr, "*** heap_getMappings()\n");*/

  sprintf(proc_name, "/proc/%05d", (int)getpid());
  *maps = NULL;
  if ((proc_fd = open(proc_name, O_RDONLY)) == -1) {
    perror("heap_getMappings(open)");
    return;
  } 
  if (ioctl(proc_fd, PIOCNMAP, nmaps) == -1) {
    perror("heap_getMappings(PIOCNMAP)");
    close(proc_fd);
    return;
  } 
  if ((*maps = (prmap_t *)malloc(((*nmaps) + 1) * sizeof(prmap_t))) == NULL) {
    fprintf(stderr, "heap_getMappings(): out of memory\n");
    close(proc_fd);
    return;
  } 
  if (ioctl(proc_fd, PIOCMAP, *maps) == -1) {
    perror("heap_getMappings(PIOCMAP)");
    close(proc_fd);
    free(*maps);
    *maps = NULL;
    return;
  }    
  close(proc_fd);
}

static void heap_checkMappings(int nmaps, prmap_t *maps)
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

static Address heap_findHole(size_t len, 
			     Address lo, 
			     Address hi,
			     prmap_t *maps, 
			     int nmaps)
{
  Address try2;
  int n, n_lo, n_hi;
  
  /* sanity check */
  if (hi < len-1) return 0;

  /* platform-specific range constraints */
  if (lo < DYNINSTheap_loAddr) lo = DYNINSTheap_loAddr;
  if (hi > DYNINSTheap_hiAddr) hi = DYNINSTheap_hiAddr;
  /*fprintf(stderr, "*** heap_findHole(0x%08x, 0x%08x, %i)\n", lo, hi, len);*/

  /* find relevant mappings for target range */
  for (n_lo = 0; n_lo < nmaps; n_lo++) {
    if (((Address)maps[n_lo].pr_vaddr) + maps[n_lo].pr_size > lo) break;
  }
  for (n_hi = nmaps-1; n_hi > n_lo; n_hi--) {
    if (((Address)maps[n_hi].pr_vaddr) <= hi) break;
  }

  /* find hole in memory */
  n = n_hi;
  try2 = heap_alignDown(hi-len+1, psize);
  while (try2 >= lo) {
    Address pr_vaddr = (Address)maps[n].pr_vaddr;
    Address pr_size = (Address)maps[n].pr_size;
    /* skip over any conflicting mappings */
    /* DEF: m1 and m2 conflict iff 
       ((m1.start < m2.end) && (m1.end > m2.start)) */
    if (n >= n_lo &&
	try2 < pr_vaddr + pr_size &&
	try2 + len > pr_vaddr) 
    {      
      /* find next eligible address */
      Address skip = pr_vaddr - len;
      if (len > pr_vaddr) return 0; /* underflow */
      try2 = heap_alignDown(pr_vaddr - len, psize);
      n--;
      continue;
    }

    return try2;
  }

  return 0;
}

static int heap_prmapCompare(const void *A, const void *B)
{
  prmap_t *a = (prmap_t *)A;
  prmap_t *b = (prmap_t *)B;
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
  fprintf(stderr, "*** DYNINSTos_malloc(%iB, 0x%016p, 0x%016p)\n", 
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

    int mmap_fd;
    int nmaps;
    prmap_t *maps;

    /* get sorted list of memory mappings */
    heap_getMappings(&nmaps, &maps);
    if (maps == NULL) {
      free(node);
      return NULL;
    }
    qsort(maps, (size_t)nmaps, (size_t)sizeof(prmap_t), &heap_prmapCompare);
    heap_checkMappings(nmaps, maps); /* sanity check */

    /*DYNINSTheap_printMappings(nmaps, maps);*/

    { /* find hole in target region and mmap */
      int mmap_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
      Address hi = (Address)hi_addr;
      Address try2 = (Address)lo_addr - psize;

      mmap_fd = DYNINSTheap_mmapFdOpen();
      for (heap = (void *)MAP_FAILED; heap == (void *)MAP_FAILED; ) {
	try2 = heap_findHole(size, try2 + psize, hi, maps, nmaps);
	if (try2 == 0) {
	  free(node);
	  free(maps);
	  DYNINSTheap_mmapFdClose(mmap_fd);
	  return NULL;
	}

	heap = mmap((void *)try2, size, mmap_prot, DYNINSTheap_mmapFlags, mmap_fd, 0);
	if (heap == (void *)MAP_FAILED) {
	  perror("mmap");
	}
      }
    }

    /* define new heap */
    node->heap.ret_addr = heap;
    node->heap.addr = heap;
    node->heap.len = size;
    node->heap.type = HEAP_TYPE_MMAP;

    /* cleanup */
    free(maps);
    DYNINSTheap_mmapFdClose(mmap_fd);
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
  fprintf(stderr, "*** DYNINSTos_free(0x%08x)\n", buf);

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

