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

/* $Id: RTheap-irix.c,v 1.1 1999/06/16 21:20:21 csserra Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <sys/procfs.h>               /* ioctl() */
#include <unistd.h>                   /* ioctl(), sbrk() */
#include <sys/mman.h>                 /* mmap() */
#include "rtinst/h/rtinst.h"          /* RT_Boolean */


/* platform-specific parameters*/
#define HEAP_ALIGN (4)                     /* heaps are word-aligned */
#define HEAP_LO_ADDR ((caddr_t)0x00400000) /* avoid zero page */
#define HEAP_HI_ADDR ((caddr_t)0x6fffffff) /* avoid stack and kernel */


/* (see "Address" in util/h/Types.h) */
typedef unsigned long Address;

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
    caddr_t addr = map->pr_vaddr;
    fprintf(stderr, "  heap %2i: %0#10x-%0#10x (%3i pages, %i bytes)\n", 
	    i, addr, addr + map->pr_size - 1, map->pr_size / psize, map->pr_size);
  }
}

static caddr_t heap_alignUp(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 1) * align) : (addr));
}

static caddr_t heap_alignDown(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 0) * align) : (addr));
}

static void heap_getMappings(prmap_t **maps, int *nmaps)
{
  int proc_fd;
  char proc_name[256];
  /*fprintf(stderr, "*** heap_getMappings()\n");*/

  sprintf(proc_name, "/proc/%05d", (int)getpid());
  *maps = NULL;
  if ((proc_fd = open(proc_name, O_RDWR)) == -1) {
    perror("heap_getMappings(open)");
    return;
  } 
  if (ioctl(proc_fd, PIOCNMAP, nmaps) == -1) {
    perror("heap_getMappings(PIOCNMAP)");
    close(proc_fd);
    return;
  } 
  if ((*maps = malloc(((*nmaps) + 1) * sizeof(prmap_t))) == NULL) {
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

static caddr_t heap_findHole(size_t len, caddr_t lo, caddr_t hi,
				  prmap_t *maps, int nmaps)
{
  caddr_t try;
  int n, n_lo, n_hi;
  
  /* platform-specific range constraints */
  if (lo < HEAP_LO_ADDR) lo = HEAP_LO_ADDR;
  if (hi > HEAP_HI_ADDR) hi = HEAP_HI_ADDR;
  /*fprintf(stderr, "*** heap_findHole(0x%08x, 0x%08x, %i)\n", lo, hi, len);*/

  /* TODO: system-aware allocation policy (heap, stack) */
  /* TODO: avoid mapping a hole adjacent to the heap (sbrk) */
  /* TODO: avoid mapping a hole adjacent to the stack */

  /* find relevant mappings for target range */
  for (n_lo = 0; n_lo < nmaps; n_lo++) {
    if (maps[n_lo].pr_vaddr + maps[n_lo].pr_size > lo) break;
  }
  for (n_hi = nmaps-1; n_hi > n_lo; n_hi--) {
    if (maps[n_hi].pr_vaddr <= hi) break;
  }

  /* find hole in memory */
  n = n_hi;
  try = heap_alignDown((hi+1)-len, psize);
  while(try >= lo) {
      /* skip over any conflicting mappings */
      /* mappings conflict if (m1.start < m2.end && m1.end > m2.start) */
      if (n >= n_lo &&
	  try < maps[n].pr_vaddr + maps[n].pr_size &&
	  try + len > maps[n].pr_vaddr) 
	{
	  caddr_t skip = maps[n].pr_vaddr - len;
	  try = heap_alignDown(skip, psize);
	  n--;
	  continue;
	}
      return try;
    }

  return (caddr_t)0;
}

static int heap_prmapCompare(const void *A, const void *B)
{
  prmap_t *a = (prmap_t *)A;
  prmap_t *b = (prmap_t *)B;
  if (a->pr_vaddr < b->pr_vaddr) return -1;
  if (a->pr_vaddr > b->pr_vaddr) return 1;
  return 0;
}

#define REGION_OFF        (28)
#define REGION_OFF_MASK   ((Address)((1 << REGION_OFF) - 1))
#define REGION_NUM_MASK   (~(Address)REGION_OFF_MASK)
#define region_num(addr)  (((Address)addr) >> REGION_OFF)
#define region_lo(addr)   (((Address)addr) & REGION_NUM_MASK)
#define region_hi(addr)   (region_lo(addr) | REGION_OFF_MASK)

/* platform-specific conditions for using malloc() */
static RT_Boolean heap_useMalloc(void *lo, void *hi)
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

void *DYNINSTos_malloc(size_t nbytes, void *loAddr, void *hiAddr)
{
  size_t size = nbytes;
  void *heap;
  heapList_t *node = malloc(sizeof(heapList_t));;

  /*
  fprintf(stderr, "*** DYNINSTos_malloc(%i, 0x%016p, 0x%016p)\n", 
	  nbytes, loAddr, hiAddr);
  */

  /* buffer size must be aligned */
  if (size % HEAP_ALIGN != 0) return ((void *)-1);

  /* use malloc() if appropriate */
  if (heap_useMalloc(loAddr, hiAddr)) {
    Address ret_heap;
    int size_heap = size + HEAP_ALIGN;
    heap = malloc(size_heap);
    if (heap == NULL) {
      free(node);
      return NULL;
    }
    ret_heap = heap_alignUp((caddr_t)heap, HEAP_ALIGN);
    /* malloc buffer must meet range constraints */
    if (ret_heap < (Address)loAddr ||
	ret_heap + size - 1 > (Address)hiAddr) {
      free(heap);
      free(node);
      return NULL;
    }
    /* define new heap */
    node->heap.ret_addr = ret_heap;
    node->heap.addr = heap;
    node->heap.len = size_heap;
    node->heap.type = HEAP_TYPE_MALLOC;
  } else {
    /* get sorted list of memory mappings */
    int nmaps;
    prmap_t *maps;
    int mmap_fd;
    heap_getMappings(&maps, &nmaps);
    if (maps == NULL) {
      free(node);
      return NULL;
    }
    qsort(maps, (size_t)nmaps, (size_t)sizeof(prmap_t), &heap_prmapCompare);
    /*print_mappings(nmaps, maps);*/
    if (psize == -1) psize = getpagesize(); /* initialize page size */
    { /* find hole in target region and mmap */
      int mmap_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
      int mmap_flags = MAP_FIXED | MAP_SHARED;
      caddr_t hi = (caddr_t)hiAddr;
      caddr_t try = (caddr_t)loAddr - psize;
      mmap_fd = open("/dev/zero", O_RDWR);
      for (heap = MAP_FAILED; heap == MAP_FAILED; ) {
	try = heap_findHole(size, try + psize, hi, maps, nmaps);
	if (try == (caddr_t)0) {
	  fprintf(stderr, "  DYNINSTos_malloc failed\n");
	  free(node);
	  free(maps);
	  close(mmap_fd);
	  return NULL;
	}
	/*fprintf(stderr, "  hole: %0#10x-%0#10x ", try, try + size - 1);*/
	heap = mmap((void *)try, size, mmap_prot, mmap_flags, mmap_fd, 0);
	if (heap == MAP_FAILED) fprintf(stderr, "!!! DYNINSTos_malloc(): mmap failed\n");
	/*fprintf(stderr, "%s\n", (heap == MAP_FAILED) ? ("failed\n") : ("mapped\n"));*/
      }
    }
    /* define new heap */
    node->heap.ret_addr = heap;
    node->heap.addr = heap;
    node->heap.len = size;
    node->heap.type = HEAP_TYPE_MMAP;
    /* cleanup */
    free(maps);
    close(mmap_fd);
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
  int ret = -1;
  heapList_t *t;
  fprintf(stderr, "*** DYNINSTos_free(0x%08x)\n", buf);

  for (t = Heaps; t != NULL; t = t->next) {
    /* lookup heap by (returned) address */
    heap_t *heap = &t->heap;
    if (heap->ret_addr != buf) continue;
    ret = 0;
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
    return ret;
  }
  return ret;
}

