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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <machine/vmparam.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/proc.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/user.h>
#include <unistd.h>
#include <kvm.h>
#include <sys/time.h>
#include <sys/resource.h>

extern void perror (const char *MESSAGE);

/* The following functions were copied from the rtinst directory */
/************************************************************************
 * caddr_t DYNINSTprobeUarea(void)
 *
 * test to see if uarea of the process can be mapped.  if so, return
 * the address of the uarea, else return null.
************************************************************************/

static caddr_t probeUarea(void) {
  kvm_t*       kd;
  struct user* u;
  struct proc* p;

  if (!(kd = kvm_open(0, 0, 0, O_RDONLY, 0))) {
    perror("kvm_open");
    return 0;
  }

  if (!(p = kvm_getproc(kd, getpid()))) {
    perror("kvm_getproc");
    return 0;
  }

  if (!(u = kvm_getu(kd, p))) {
    perror("kvm_getu");
    return 0;
  }

  kvm_close(kd);
  return (caddr_t) (p->p_uarea);
}

/************************************************************************
 * struct rusage *mapUarea(void)
 *
 * map urea of the process into its accessible address space.  if
 * such a mapping is possible, set clock pointers for fast access,
 * else use slower system calls.
************************************************************************/

struct rusage *mapUarea(void) {
  int     ret;
  int     kmem;
  caddr_t uAddr;
  static struct user* u;
  struct rusage *r;

  uAddr = probeUarea();
  if (!uAddr) return NULL;

  if ((kmem = open("/dev/kmem", O_RDONLY, 0)) == -1) {
    perror("/dev/kmem");
    return NULL;
  }

  if ((ret = (int) mmap(0, sizeof(struct user), PROT_READ, MAP_SHARED,
			kmem, (off_t) uAddr)) == -1) {
    perror("mmap");
    return NULL;
  }

  u = (struct user *) ret;
  r = &(u->u_ru);
  return r;
}
