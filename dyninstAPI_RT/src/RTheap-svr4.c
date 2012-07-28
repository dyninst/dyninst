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

/* $Id: RTheap-svr4.c,v 1.3 2006/05/03 00:31:25 jodom Exp $ */
/* RTheap-svr4.c: heap management for SVR4 platforms */

#include <stdio.h>
#include <sys/stat.h>   /* open */
#include <fcntl.h>      /* open */
#include <sys/types.h>  /* getpid, open */
#include <unistd.h>     /* getpid, ioctl */
#include <stdlib.h>     /* malloc, free */
#include "RTheap.h"     /* dyninstmm_t */

int
DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
{
     int fd;
     char buf[1024];
     dyninstmm_t *ms;
     unsigned num;

     sprintf(buf, "/proc/%d", getpid());
     fd = open(buf, O_RDONLY);
     if (0 > fd) {
	  perror("open /proc");
	  return -1;
     }
     if (0 > ioctl(fd, PIOCNMAP, &num)) {
	  perror("getMemoryMap (PIOCNMAP)");
	  close(fd);
	  return -1;
     }
     ms = (dyninstmm_t *) malloc((num+1) * sizeof(dyninstmm_t));
     if (!ms) {
	  fprintf(stderr, "DYNINSTgetMemoryMap: Out of memory\n");
	  close(fd);
	  return -1;
     }
     if (0 > ioctl(fd, PIOCMAP, ms)) {
	  perror("getMemoryMap (PIOCMAP)");
	  free(ms);
	  close(fd);
	  return -1;
     }

     /* success */
     close(fd);
     *nump = num;
     *mapp = ms;
     return 0;
}
