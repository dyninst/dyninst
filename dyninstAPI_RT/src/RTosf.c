/*
 * Copyright (c) 1996 Barton P. Miller
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

/************************************************************************
 * RTosf.c: mutatee-side library function specific to OSF
************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>                 /* open() */
#include <fcntl.h>                    /* open() */
#include <unistd.h>                   /* procfs */
#include <sys/procfs.h>               /* procfs */
#include <sys/mman.h>                 /* mmap() */
#include <dlfcn.h>                    /* dlopen() */

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function
************************************************************************/


/* The alpha does not have a divide instruction */
/* Division is emulated in software */

int divide(int a,int b)
{
  return (a/b);
}


/************************************************************************
 * EXPORTED SYMBOLS:
 *   void   DYNINSTos_init(void): os initialization function
 *   seg_t *DYNINSTos_malloc(size_t, void *): directed heap allocation
 *   int    DYNINSTos_free(void *): heap deallocation
 ************************************************************************/

/* below (Address) must be consistent with dyninstAPI code */
typedef unsigned long Address;

typedef struct seg_t {
  void *addr;
  size_t len;
} seg_t;

typedef struct segList_t {
  seg_t seg;
  struct segList_t *prev;
  struct segList_t *next;
} segList_t;

/* local variables */
static segList_t *Heaps = NULL;
static int psize = -1;


static void print_mappings(int nmaps, prmap_t *maps)
{
  int i;
  fprintf(stderr, "memory mappings:\n");
  for(i = 0; i < nmaps; i++) {
    prmap_t *map = &maps[i];
    caddr_t addr = map->pr_vaddr;
    fprintf(stderr, "  segment %2i: %0#10x-%0#10x (%3i pages, %i bytes)\n", 
	    i, addr, addr + map->pr_size - 1, map->pr_size / psize, map->pr_size);
  }
}

caddr_t DYNINSTos_alignUp(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 1) * align) : (addr));
}

caddr_t DYNINSTos_alignDown(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 0) * align) : (addr));
}

void DYNINSTos_align(off_t *off_, size_t *size_, 
		     size_t *pad_pre_, size_t *pad_post_)
{
  off_t off_orig = *off_;
  size_t size_orig = *size_;

  int psize = getpagesize();
  size_t pad_pre = off_orig % psize;
  size_t over = (off_orig + size_orig) % psize;
  size_t pad_post = (over) ? (psize - over) : (0);
  size_t size = pad_pre + size_orig + pad_post;
  off_t off = off_orig - pad_pre;

  *off_ = off;
  *size_ = size;
  *pad_pre_ = pad_pre;
  *pad_post_ = pad_post;
}

void DYNINSTos_getMappings(prmap_t **maps, int *nmaps)
{
  int proc_fd;
  char proc_name[256];

  sprintf(proc_name, "/proc/%05d", (int)getpid());
  *maps = NULL;
  if ((proc_fd = open(proc_name, O_RDWR)) == -1) {
    perror("DYNINSTos_getMappings(open)");
    return;
  } 
  if (ioctl(proc_fd, PIOCNMAP, nmaps) == -1) {
    perror("DYNINSTos_getMappings(PIOCNMAP)");
    close(proc_fd);
    return;
  } 
  /* TODO: replace malloc() calls with alloca()? */
  if ((*maps = (prmap_t *) malloc(((*nmaps) + 1) * sizeof(prmap_t))) == NULL) {
    fprintf(stderr, "DYNINSTos_getMappings(): out of memory\n");
    close(proc_fd);
    return;
  } 
  if (ioctl(proc_fd, PIOCMAP, *maps) == -1) {
    perror("DYNINSTos_getMappings(PIOCMAP)");
    close(proc_fd);
    free(*maps);
    return;
  }    
  close(proc_fd);
}


#define LOWEST_ADDR  ((caddr_t)0x00400000)
#define HIGHEST_ADDR ((caddr_t)0x7ffffffffff)

caddr_t DYNINSTos_findHole(size_t len, caddr_t lo, caddr_t hi,
			   prmap_t *maps, int nmaps)
{
  caddr_t tryAddr;
  int n, n_lo, n_hi;
  
  /* special filter to avoid mapping the zero page */
  if (lo < LOWEST_ADDR) lo = LOWEST_ADDR;
  if (hi > HIGHEST_ADDR) hi = HIGHEST_ADDR;

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
  tryAddr = DYNINSTos_alignDown((hi+1)-len, psize);
  while(tryAddr >= lo) {
      /* skip over any conflicting mappings */
      /* mappings conflict if (m1.start < m2.end && m1.end > m2.start) */
      if (n >= n_lo &&
	  tryAddr < maps[n].pr_vaddr + maps[n].pr_size &&
	  tryAddr + len > maps[n].pr_vaddr) 
	{
	  caddr_t skip = maps[n].pr_vaddr - len;
	  tryAddr = DYNINSTos_alignDown(skip, psize);
	  n--;
	  continue;
	}
      return tryAddr;
    }

  return (caddr_t)0;
}

int DYNINSTos_prmapCompare(const void *A, const void *B)
{
  int by_addr;
  prmap_t *a = (prmap_t *)A;
  prmap_t *b = (prmap_t *)B;
  /* Addresses are unsigned, so need to use if, not subtraction here. */
  if (a->pr_vaddr > b->pr_vaddr) {
      return 1;
  } else if (a->pr_vaddr == b->pr_vaddr) {
      return 0;
  } else {
      return -1;
  }
  return a->pr_vaddr - b->pr_vaddr;
}

/* TODO: use malloc() instead of mmap() if might conflict with heap */
seg_t *DYNINSTos_malloc(size_t nbytes, void *loAddr, void *hiAddr)
{
  int i;
  size_t size = nbytes;
  int nmaps;
  prmap_t *maps;
  long int heap;
  segList_t *node;
  unsigned long int endPrevSegment = 0;

  /* initialize pagesize value */
  if (psize == -1) psize = getpagesize();

  { /* align buffer size to page size */
    off_t off = 0;
    size_t pad_pre, pad_post;  
    DYNINSTos_align(&off, &size, &pad_pre, &pad_post);
  }

  /* get sorted list of memory mappings */
  DYNINSTos_getMappings(&maps, &nmaps);
  qsort(maps, (size_t)nmaps, (size_t)sizeof(prmap_t), &DYNINSTos_prmapCompare);

  /* verify snaity of segments */
  for (i=0; i < nmaps; i++) {
      if (endPrevSegment > maps[i].pr_vaddr) {
	  printf("*** Map Segments Overlap\n");
	  fflush(stdout);
	  abort();
      }
      endPrevSegment = maps[i].pr_vaddr+maps[i].pr_size;
  }

  { /* find a hole in the memory of the target region and mmap() it */
    int mmap_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    int mmap_flags = MAP_ANONYMOUS | MAP_FIXED | MAP_SHARED;
    caddr_t hi = (caddr_t)hiAddr;
    caddr_t tryAddr = (caddr_t)loAddr - psize;
    for (heap = MAP_FAILED; heap == MAP_FAILED; ) {
      tryAddr = DYNINSTos_findHole(size, tryAddr + psize, hi, maps, nmaps);
      if (tryAddr == (caddr_t)0) {
	fprintf(stderr, "  DYNINSTos_malloc failed\n");
	fflush(stderr);
	free(maps);
        return NULL;
      }
      heap = (long int) mmap((void *)tryAddr, size, mmap_prot, mmap_flags, -1, 0);
      if (heap == MAP_FAILED) {
	  perror("mmap");
	  printf("map failed\n");
	  fflush(stderr);
      }
    }
  }

  /* allocate node for new segment */
  node = (segList_t *) malloc(sizeof(segList_t));
  node->seg.addr = (void *) heap;
  node->seg.len = size;
  /* insert into segment list */
  node->prev = NULL;
  node->next = Heaps;
  if (Heaps) Heaps->prev = node;
  Heaps = node;

  /* cleanup */
  free(maps);

  return &(node->seg);
}

int DYNINSTos_free(void *buf)
{
  segList_t *t;

  for (t = Heaps; t != NULL; t = t->next) {
    seg_t *seg = &t->seg;
    if (seg->addr == buf) {
      /* remove segment from list */
      if (t->next) t->next->prev = t->prev;
      if (t->prev) t->prev->next = t->next;
      if (Heaps == t) Heaps = t->next;
      /* unmap segment */
      if (munmap(seg->addr, seg->len) == -1) {
	perror("DYNINSTos_free(munmap)");
	return -1;
      }
      /* free list element */
      free(t);
      return 0;
    }
  }
  return -1;
}

int DYNINSTloadLibrary(char *libname)
{
  /* TODO: return dlopen() handle? */
  void *ret = dlopen(libname, RTLD_NOW);
  return (ret != NULL) ? (1) : (0);
}

void DYNINSTos_init(int calledByFork, int calledByAttach)
{


}
