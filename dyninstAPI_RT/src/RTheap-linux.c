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

/* $Id: RTheap-linux.c,v 1.3 2003/06/10 17:45:31 tlmiller Exp $ */
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

#if defined( ia64_unknown_linux2_4 )
/* Align the heap at the IP granularity. */
int	DYNINSTheap_align = 16;

/* Try to stay away from the mutatee's stack and heap.
   For now, stash everything in what's supposed to
   be the shared memory region. */
Address	DYNINSTheap_loAddr = 0x2000000000000000;
Address DYNINSTheap_hiAddr = 0x3fffffffffffffff;
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

/* Linux /proc/PID/maps is unreliable when it is read with more than one
read call (it can show pages that are not actually allocated).  We
read it all in one call into this buffer.

linux-2.4: reading /proc/PID/maps now returns after each line in the maps,
so we must loop to get everything.
*/
static char procAsciiMap[1<<15];

int
DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
{
     int fd;
     ssize_t ret, length;
     char *p;
     dyninstmm_t *ms;
     unsigned i, num;

/* 
    Here are two lines from 'cat /proc/self/maps' on Linux 2.2.  Each
    describes a segment of the address space.  We parse out the first
    two addresses for the start address and length of the segment.  We
    throw away the rest.

|SADDR-| |EADDR-|
0804a000-0804c000 rw-p 00001000 08:09 12089      /bin/cat
0804c000-0804f000 rwxp 00000000 00:00 0
*/

     fd = open("/proc/self/maps", O_RDONLY);
     if (0 > fd) {
	  perror("open /proc");
	  return -1;
     }
     length = 0;
     while (1)
     {
         ret = read(fd, procAsciiMap + length, sizeof(procAsciiMap) - length);
         if (0 == ret) break;
         if (0 > ret) {
	      perror("read /proc");
	      return -1;
         }
         length += ret;
         if (length >= sizeof(procAsciiMap)) {
	      fprintf(stderr, "DYNINSTgetMemoryMap: memory map buffer overflow\n");
	      return -1;
         }
     }
     procAsciiMap[length] = '\0'; /* Now string processing works */

     close(fd);

     /* Count lines, which is the same as the number of segments.
	Newline characters separating lines are converted to nulls. */
     for (num = 0, p = strtok(procAsciiMap, "\n");
	  p != NULL;
	  num++, p = strtok(NULL, "\n"))
	  ;
     
     ms = (dyninstmm_t *) malloc(num * sizeof(dyninstmm_t));
     if (! ms) {
	  fprintf(stderr, "DYNINSTgetMemoryMap: Out of memory\n");
	  return -1;
     }

     p = procAsciiMap;
     for (i = 0; i < num; i++) {
	  char *next = p + strlen(p) + 1; /* start of next line */
	  Address saddr, eaddr;
	  unsigned long size;

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
     return 0;
parseerr:
     free(ms);
     fprintf(stderr, "DYNINSTgetMemoryMap: /proc/self/maps parse error\n");
     return -1;
}
