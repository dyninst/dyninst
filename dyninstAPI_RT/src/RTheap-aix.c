/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: RTheap-aix.c,v 1.3 2004/03/23 01:12:16 eli Exp $ */
/* RTheap-aix.c: AIX-specific heap components */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/mman.h>
#include "dyninstAPI_RT/src/RTheap.h"


int     DYNINSTheap_align = 4; /* heaps are word-aligned */

/* Not really the heap addresses, but these are used by
   constrained_mmap to determine the bounds on the mmap search.
   The heap is in segment 2, and is used in malloc below */
Address DYNINSTheap_loAddr = 0x30000000;
Address DYNINSTheap_hiAddr = 0xcfffffff;

int     DYNINSTheap_mmapFlags = MAP_VARIABLE | MAP_PRIVATE;

/* Fixed mapping of AIX segments. Update as the OS updates */
/*
   0x00000000; segment 0; kernel
   0x10000000; segment 1; program text (ptrace'able)
   0x20000000; segment 2; data (low) and stack (high)
   0x30000000; segment 3; empty (mmappable)
   ...
   0xc0000000; segment 12; empty (mmappable)
   0xd0000000; segment 13; shared lib text
   0xe0000000; segment 14; kernel (mmappable in experiment,
                           I'm not sure I'd want to.)
   0xf0000000; segment 15; shared lib data
*/

/*
  Slight modification to the memory map: if the maxdata flag is
  set, then the heap starts at 0x30000000 and goes from there.
  So catch this behavior
*/

/* AIX mmap behavior: MAP_FIXED either works or fails. In
   experiments, MAP_VARIABLE returns either the desired
   area, or the first one which fits, with a _higher_ addr.
*/

Address DYNINSTheap_loMmap = 0x30000000;
Address DYNINSTheap_hiMmap = 0xcfffffff;

/* 24 bits of jump range, shifted 2 for a total of 26 */
Address DYNINSTheap_jumpRange = 0x03fffffc;

RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
{
  Address lo_addr = (Address)lo;
  Address hi_addr = (Address)hi;
  Address sbrk_addr = (Address)sbrk(0);

  if ((lo_addr <= sbrk_addr - DYNINSTheap_jumpRange) &&
      (hi_addr >= 0x2fffffff + DYNINSTheap_jumpRange))
    return RT_TRUE;
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

/*
 * Here's the fun one. We need to get a mapping of memory
 * areas that are available to the process. AFAIK, this _requires_
 * a /proc filesystem. Since AIX does not have this, and there
 * is no way to duplicate the functionality (other than a scan),
 * we fake the results. Bad us.
 */

int
DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
{
  /* Open /proc/self...*/
  /* Read from it...*/
  dyninstmm_t *ms;
  unsigned num = 2;

  ms = (dyninstmm_t *) malloc((num+1) * sizeof(dyninstmm_t));

  /* One _really_ big hole. Definitely a hack, but the best there
     is */

  /* Turns out this doesn't work, since constrained_mmap searches
     for a memory segment which is contained within the
     boundaries given to inferiorMalloc.
     This is because the memory mapping returned is a list of the _used_
     areas of memory, not the free ones. 
     So we basically want a lower-end empty "used" one, and a high-end
     empty "used" one. Empty==size of 0 here
  */

  ms[0].pr_vaddr = DYNINSTheap_loAddr;
  ms[0].pr_size = 0; /* Really, we swear, no used memory here */

  ms[1].pr_vaddr = DYNINSTheap_hiAddr;
  ms[1].pr_size = 0;

  *nump = num;
  *mapp = ms;
  return 0;
}
