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
 * RTsolaris.c: mutatee-side library function specific to Solaris
************************************************************************/

#include <signal.h>
#include <sys/ucontext.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* getpid() */

#include "dyninstAPI_RT/h/rtinst.h"

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function
************************************************************************/

void
DYNINSTos_init(int calledByFork, int calledByAttach)
{
    /*
       Install trap handler.
       This is currently being used only on the x86 platform.
    */
#ifdef i386_unknown_solaris2_5
    void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap);
    struct sigaction act;
    act.sa_handler = DYNINSTtrapHandler;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    if (sigaction(SIGTRAP, &act, 0) != 0) {
        perror("sigaction(SIGTRAP)");
	assert(0);
	abort();
    }
#endif


}





/****************************************************************************
   The trap handler. Currently being used only on x86 platform.

   Traps are used when we can't insert a jump at a point. The trap
   handler looks up the address of the base tramp for the point that
   uses the trap, and set the pc to this base tramp.
   The paradynd is responsible for updating the tramp table when it
   inserts instrumentation.
*****************************************************************************/

#ifdef i386_unknown_solaris2_5
trampTableEntry DYNINSTtrampTable[TRAMPTABLESZ];
unsigned DYNINSTtotalTraps = 0;

static unsigned lookup(unsigned key) {
    unsigned u;
    unsigned k;
    for (u = HASH1(key); 1; u = (u + HASH2(key)) % TRAMPTABLESZ) {
      k = DYNINSTtrampTable[u].key;
      if (k == 0)
        return 0;
      else if (k == key)
        return DYNINSTtrampTable[u].val;
    }
    /* not reached */
    assert(0);
    abort();
}

void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap) {
    unsigned pc = uap->uc_mcontext.gregs[PC];
    unsigned nextpc = lookup(pc);

    if (!nextpc) {
      /* kludge: maybe the PC was not automatically adjusted after the trap */
      /* this happens for a forked process */
      pc--;
      nextpc = lookup(pc);
    }

    if (nextpc) {
      uap->uc_mcontext.gregs[PC] = nextpc;
    } else {
      assert(0);
      abort();
    }
    DYNINSTtotalTraps++;
}
#endif

int DYNINSTloadLibrary(char *libname)
{
    if (dlopen(libname, RTLD_NOW | RTLD_GLOBAL) != NULL)
	return 1;
    else 
	return 0;
}

/*
 * dynamic inferior heap management
 */
#include <sys/mman.h>
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

/*static*/
caddr_t DYNINSTos_alignUp(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 1) * align) : (addr));
}

/*static*/
caddr_t DYNINSTos_alignDown(caddr_t addr_, size_t align)
{
  Address addr = (Address)addr_;
  return (caddr_t)((addr % align) ? (((addr / align) + 0) * align) : (addr));
}

/*static*/
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

  /*fprintf(stderr, "*** DYNINSTos_align(%i bytes at 0x%08x)\n", *size_, *off_);*/

  *off_ = off;
  *size_ = size;
  *pad_pre_ = pad_pre;
  *pad_post_ = pad_post;
}

/*static*/
void DYNINSTos_getMappings(prmap_t **maps, int *nmaps)
{
  int proc_fd;
  char proc_name[256];
  /*fprintf(stderr, "*** DYNINSTos_getMappings()\n");*/

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
  if ((*maps = (prmap_t*) malloc(((*nmaps) + 1) * sizeof(prmap_t))) == NULL) {
    fprintf(stderr, "DYNINSTos_getMappings(): out of memory\n");
    close(proc_fd);
    return;
  }
  if (ioctl(proc_fd, PIOCMAP, *maps) == -1) {
    perror("DYNINSTos_getMappings(PIOCMAP)");
    close(proc_fd);
    free(*maps);
    *maps = NULL;
    return;
  }
  close(proc_fd);
}


#define LOWEST_ADDR  ((caddr_t)0x00000000)
#define HIGHEST_ADDR ((caddr_t)0xefffffff)
/*static*/
caddr_t DYNINSTos_findHole(size_t len, caddr_t lo, caddr_t hi,
                           prmap_t *maps, int nmaps)
{
  caddr_t try;
  int n, n_lo, n_hi;

  /* special filter to avoid mapping the zero page */
  if (lo < LOWEST_ADDR) lo = LOWEST_ADDR;
  if (hi > HIGHEST_ADDR) hi = HIGHEST_ADDR;
  /*fprintf(stderr, "*** DYNINSTos_findHole(0x%08x, 0x%08x, %i)\n", lo, hi, len);*/

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
  try = DYNINSTos_alignDown((hi+1)-len, psize);
  while(try >= lo) {
      /* skip over any conflicting mappings */
      /* mappings conflict if (m1.start < m2.end && m1.end > m2.start) */
      if (n >= n_lo &&
          try < maps[n].pr_vaddr + maps[n].pr_size &&
          try + len > maps[n].pr_vaddr)
        {
          caddr_t skip = maps[n].pr_vaddr - len;
          try = DYNINSTos_alignDown(skip, psize);
          n--;
          continue;
        }
      return try;
    }

  return (caddr_t)0;
}

/*static*/
int DYNINSTos_prmapCompare(const void *A, const void *B)
{
  int by_addr;
  prmap_t *a = (prmap_t *)A;
  prmap_t *b = (prmap_t *)B;
  if (a->pr_vaddr > b->pr_vaddr)
    return 1 ;
  else if(a->pr_vaddr < b->pr_vaddr)
    return -1 ;
  return 0;
}

/* TODO: use malloc() instead of mmap() if might conflict with heap */
seg_t *DYNINSTos_malloc(size_t nbytes, void *loAddr, void *hiAddr)
{
  size_t size = nbytes;
  int nmaps;
  prmap_t *maps = NULL;
  int mmap_fd;
  void *heap;
  segList_t *node;
  /*fprintf(stderr, "*** DYNINSTos_malloc(%i, %0#18lx, %0#18lx)\n", nbytes, loAddr, hiAddr);*/

  /*fprintf(stderr, "*** sbrk: 0x%08x\n", sbrk(0));*/
  if ((char*) loAddr <= ((char*) sbrk(0))+ 0x800000) {
    if (SYN_INST_BUF_SIZE/4 > size) size = SYN_INST_BUF_SIZE/4  ;
    heap = (void*) malloc(size) ;
  } else {
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
    /*print_mappings(nmaps, maps);*/

    { /* find a hole in the memory of the target region and mmap() it */
      int mmap_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
      int mmap_flags = MAP_FIXED | MAP_SHARED ;
      caddr_t hi = (caddr_t)hiAddr;
      caddr_t try = (caddr_t)loAddr - psize;
      mmap_fd = open("/dev/zero", O_RDWR);
      for (heap = MAP_FAILED; heap == MAP_FAILED; ) {
        try = DYNINSTos_findHole(size, try + psize, hi, maps, nmaps);
        if (try == (caddr_t)0) {
          fprintf(stderr, "  DYNINSTos_malloc failed\n");
          free(maps);
          close(mmap_fd);
          return NULL;
        }
        /*fprintf(stderr, "  hole: %0#10x-%0#10x ", try, try + size - 1);*/
        heap = mmap((void *)try, size, mmap_prot, mmap_flags, mmap_fd, 0);
        /*fprintf(stderr, "%s\n", (heap == MAP_FAILED) ? ("failed\n") : ("mapped\n"));*/
      }
    }
  }
  /* allocate node for new segment */
  node = (segList_t*) malloc(sizeof(segList_t));
  node->seg.addr = heap;
  node->seg.len = size;
  /* insert into segment list */
  node->prev = NULL;
  node->next = Heaps;
  if (Heaps) Heaps->prev = node;
  Heaps = node;

  /* cleanup */
  if (maps) free(maps);
  close(mmap_fd);

  /* return node->seg.addr; */
  return &(node->seg);
}

int DYNINSTos_free(void *buf)
{
  segList_t *t;
  fprintf(stderr, "*** DYNINSTos_free(0x%08x)\n", buf);

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

