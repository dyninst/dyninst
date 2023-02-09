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

#define _GNU_SOURCE

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
#include "unaligned_memory_access.h"


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


static Address heap_alignUp(Address addr, int align)
{
  if (addr % align == 0) return addr;
  return ((addr / align) + 1) * align;
}

static Address trymmap(size_t len, Address beg, Address end, size_t inc, int fd)
{
  Address addr;
  void *result;
  /*We have a possibly large region (beg to end) and a hopefully smaller */
  /* allocation size (len).  We try to map at every page in the region*/
  /* until we get one that succeeds.*/
  for (addr = beg; addr + len <= end; addr += inc) {
    result = map_region((void *) addr, len, fd);
    if (result) {
      /* Success doesn't necessarily mean it actually mapped at the hinted
       * address.  Return if it's in range, else unmap and try again. */
      if ((Address) result >= beg && (Address) result + len <= end)
        return (Address) result;
      unmap_region(result, len);
    }
  }
  return (Address) NULL;
}

void *DYNINSTos_malloc(size_t nbytes, void *lo_addr, void *hi_addr)
{
  char *heap;
  size_t size = nbytes;
  heapList_t *node = NULL;
    /* initialize page size */
  if (psize == -1) psize = getpagesize();

  /* buffer size must be aligned */
  if (size % DYNINSTheap_align != 0) {
    return ((void *)-1);
  }

  /* use malloc() if appropriate */
  if (DYNINSTheap_useMalloc(lo_addr, hi_addr)) {

    char* ret_heap;
    int size_heap = size + DYNINSTheap_align + sizeof(heapList_t);
    heap = malloc(size_heap);
    if (heap == NULL) {
#ifdef DEBUG
      fprintf(stderr, "Failed to MALLOC\n");      
#endif 
      return NULL;
    }
    ret_heap = (char*)heap_alignUp((Address)heap, DYNINSTheap_align);
    
    /* malloc buffer must meet range constraints */
    if (ret_heap < (char*)lo_addr ||
        ret_heap + size - 1 > (char*)hi_addr) {
      free(heap);
#ifdef DEBUG
      fprintf(stderr, "MALLOC'd area fails range constraints\n");
#endif 
      return NULL;
    }

    /* define new heap */
    node = CAST_WITHOUT_ALIGNMENT_WARNING(heapList_t*, (ret_heap + size));
    node->heap.ret_addr = (void *)ret_heap;
    node->heap.addr = heap;
    node->heap.len = size_heap;
    node->heap.type = HEAP_TYPE_MALLOC;


  } else { /* use mmap() for allocation */
    Address lo = heap_alignUp((Address)lo_addr, psize);
    Address hi = (Address) hi_addr;
    heap = (char*)trymmap(size + sizeof(struct heapList_t), lo, hi, psize, -1);
    if(!heap)
        return NULL;
    node = CAST_WITHOUT_ALIGNMENT_WARNING(heapList_t*, (heap + size));

    /* define new heap */
    node->heap.addr = heap;
    node->heap.ret_addr = heap;
    node->heap.len = size + sizeof(struct heapList_t);
    node->heap.type = HEAP_TYPE_MMAP;
  }

  /* insert new heap into heap list */
  node->prev = NULL;
  node->next = Heaps;
  if (Heaps) Heaps->prev = node;
  Heaps = node;
#ifdef DEBUG
  fprintf(stderr, "new heap at %lx, size %lx\n", node->heap.ret_addr, node->heap.len);
#endif
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

    break;
  }

  return ret;
}

