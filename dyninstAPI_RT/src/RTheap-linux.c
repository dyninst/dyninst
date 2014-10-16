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

/* $Id: RTheap-linux.c,v 1.9 2008/01/31 18:01:54 legendre Exp $ */
/* RTheap-linux.c: Linux-specific heap components */

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

#define MAX_MAP_SIZE (1<<20)
#if defined(MUTATEE64)

int     DYNINSTheap_align = 4; /* heaps are word-aligned */

Address DYNINSTheap_loAddr = 0x10000; /* Bump to 64k to make SELinux happier */
Address DYNINSTheap_hiAddr = ~0x0;
#elif defined(arch_power)
int     DYNINSTheap_align  = 4; /* heaps are word-aligned */
Address DYNINSTheap_loAddr = ~(Address)0; // should be defined by getpagesize() when used.
Address DYNINSTheap_hiAddr = ~(Address)0;
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
  (void)lo; /* unused parameter */
  (void)hi; /* unused parameter */
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

/* Linux /proc/PID/maps is unreliable when it is read with more than one
read call (it can show pages that are not actually allocated).  We
read it all in one call into this buffer.

linux-2.4: reading /proc/PID/maps now returns after each line in the maps,
so we must loop to get everything.
*/

// Static so we dont have to do the exponential backoff each time.
static size_t mapSize = 1 << 15;

int
DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
{
   int fd, done;
   ssize_t ret;
   size_t length;
   char *p;
   dyninstmm_t *ms;
   unsigned i, num;
   char *procAsciiMap;


   /* 
      Here are two lines from 'cat /proc/self/maps' on Linux 2.2.  Each
      describes a segment of the address space.  We parse out the first
      two addresses for the start address and length of the segment.  We
      throw away the rest.

      |SADDR-| |EADDR-|
      0804a000-0804c000 rw-p 00001000 08:09 12089      /bin/cat
      0804c000-0804f000 rwxp 00000000 00:00 0
   */
   procAsciiMap = NULL;
   done = 0;
   while (1)
   {
       if(procAsciiMap == NULL) {
           procAsciiMap = malloc(mapSize);
           if (!procAsciiMap) {
               fprintf(stderr, "DYNINSTgetMemoryMap: Out of memory\n");
               goto end;
           }
       }
       else
       {
           void *newMap = realloc(procAsciiMap, mapSize);
           if (!newMap) {
               fprintf(stderr, "DYNINSTgetMemoryMap: Out of memory\n");
               goto freeBuffer;
           }
           procAsciiMap = newMap;
       }
       fd = open("/proc/self/maps", O_RDONLY);
       if (0 > fd) {
           perror("open /proc");
           goto freeBuffer;
       }
       length = 0;
       while (1)
       {
           ret = read(fd, procAsciiMap + length, mapSize - length);
           length += ret;
           if (0 == ret) {
               done = 1;
               break;
           }
           if (0 > ret) {
               close(fd);
               perror("read /proc");
               goto freeBuffer;
           }
           /* Check if the buffer was to small and exponentially
              increase its size */
           if (length >= mapSize) {
               close(fd);
               mapSize = mapSize << 1;

                  /* Return error if we reached the max size for
                     the buffer*/
                  if(mapSize > MAX_MAP_SIZE) {
                      fprintf(stderr, "DYNINSTgetMemoryMap: memory map buffer \
                                       is larger than the max size. max size=%d\n", MAX_MAP_SIZE);
                      goto freeBuffer;
                  }
               break;
           }
       }
       if(done) {
           break;
       }
   }
   procAsciiMap[length] = '\0'; /* Now string processing works */

   /* Count lines, which is the same as the number of segments.
      Newline characters separating lines are converted to nulls. */
   for (num = 0, p = strtok(procAsciiMap, "\n");
        p != NULL;
        num++, p = strtok(NULL, "\n"))
      ;

   ms = (dyninstmm_t *) malloc(num * sizeof(dyninstmm_t));
   if (! ms) {
      fprintf(stderr, "DYNINSTgetMemoryMap: Out of memory\n");
      goto freeBuffer;
   }

   p = procAsciiMap;
   for (i = 0; i < num; i++) {
      char *next = p + strlen(p) + 1; /* start of next line */
      Address saddr, eaddr;

      /* parse start address */
      p = strtok(p, "-");
      if (! p) goto parseerr;
      saddr = strtoul(p, &p, 16);
      ++p; /* skip '-' */

      /* parse end address */
      p = strtok(NULL, " ");
      if (! p) goto parseerr;
      eaddr = strtoul(p, NULL, 16);

      ms[i].pr_vaddr = saddr;
      ms[i].pr_size = eaddr - saddr;

      p = next;
   }

   *nump = num;
   *mapp = ms;
   free(procAsciiMap);
   return 0;
 parseerr:
   free(ms);
   fprintf(stderr, "DYNINSTgetMemoryMap: /proc/self/maps parse error\n");
 freeBuffer:
   free(procAsciiMap);
 end:
   return -1;
}
