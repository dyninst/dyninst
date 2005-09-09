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

/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTsparc.c,v $
 * Revision 1.13  2005/09/09 18:05:47  legendre
 * Reorganized both the Paradyn and Dyninst runtime libraries.  Some big chunks
 * of the Paradyn library that deal with threads were moved down into the
 * dyninst library.  We no longer have seperate runtime libraries for
 * threads and non-thread versions.
 *
 * Revision 1.12  2004/03/23 01:12:43  eli
 * Updated copyright string
 *
 * Revision 1.11  1999/08/27 21:04:02  zhichen
 * tidy up
 *
 * Revision 1.10  1996/08/16 21:27:44  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.9  1996/02/01 17:48:37  naim
 * Fixing some problems related to timers and race conditions. I also tried to
 * make a more standard definition of certain procedures (e.g. reportTimer)
 * across all platforms - naim
 *
 * Revision 1.8  1994/11/12  17:32:12  rbi
 * removed /dev/kmem warning messages
 *
 * Revision 1.7  1994/11/11  10:39:10  markc
 * Commented out non-emergency printfs
 *
 * Revision 1.6  1994/09/20  18:25:00  hollings
 * removed call to getcmd since it was causing a SS-5 slow down.
 *
 * Revision 1.5  1994/07/14  23:34:08  hollings
 * added include of kludges.h
 *
 * Revision 1.4  1994/07/05  03:25:10  hollings
 * obsereved cost model.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <machine/vmparam.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <kvm.h>
#include <errno.h>

#include "rtinst/h/rtinst.h"

extern time64 DYNINSTgetCPUtime(void);

int DYNINSTmappedUarea;
int *_p_1, *_p_2, *_p_3, *_p_4;
static int _kmem = -1;

/*
 * find out if the uarea is safe for mapping.
 *    If it is, return the address of the uarea.
 *
 */
caddr_t DYNINSTprobeUarea()
{
     int pid;
     kvm_t *kd;
#ifdef notdef
     char *cmd;
     char **args;
#endif
     struct user *u;
     struct proc *p;

     kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
     if (!kd) {
	 perror("kvm_open");
	 return(0);
     }
     pid = getpid();
     p = kvm_getproc(kd, pid);
     if (!p) {
	 return(0);
	 perror("kvm_getproc");
     }
     u = kvm_getu(kd, p);
     if (!u) {
	 perror("kvm_getu");
	 return(0);
     }

#ifdef notdef
     /* removed this call since is seems to cause a 10% slow down of the
     application on SS-5 programs (I have no idea why) -- jkh 9/18/94 */
     kvm_getcmd(kd, p, u, &args, NULL);
     cmd = (char *) rindex(args[0], '/');
     if (cmd) {
	 cmd++;
     } else {
	 cmd = args[0];
     }
#endif
#ifdef notdef
     if (strcmp(cmd, u->u_comm)) {
	 printf("cmd = %s, u_comm = %s\n", cmd, u->u_comm);
	 perror("no cmd");
	 return(0);
     }
#endif
     kvm_close(kd);

     return((caddr_t)p->p_uarea);
}

int DYNINSTmapUarea()
{
    int ret;
    caddr_t uAddr;
    static struct user *u;

    uAddr = DYNINSTprobeUarea();

    if (!uAddr) {
/*
 *  commented out to avoid dirty looks during SC94   -rbi 11/11/94
 *
 *   printf("WARNING: program compiled for wrong version of SPARC chip.\n");
 *   printf(" using getrusage for times, this may slow your program down\n");
 *   printf(" by a factor of ten or more.\n");
 *   printf("\n"); 
 *   fflush(stdout);
 */
	return(0);
    }

    _kmem = open("/dev/kmem", O_RDONLY, 0);
    if (_kmem < 0) {
	return(0);
    }

    ret = (int) mmap((caddr_t) 0, sizeof(struct user), PROT_READ, 
	MAP_SHARED, _kmem, (off_t) uAddr);
    if (ret == -1) {
	return(0);
    }
    u = (struct user *) ret;
    _p_1 = (int *) &(u->u_ru.ru_utime.tv_sec);
    _p_2 = (int *) &(u->u_ru.ru_utime.tv_usec);
    _p_3 = (int *) &(u->u_ru.ru_stime.tv_sec);
    _p_4 = (int *) &(u->u_ru.ru_stime.tv_usec);

    return(1);
}

/*
 * Run a nop loop to estimate clock frequency.
 *
 */

#define LOOP_LIMIT	50000
#define MILLION		1000000

float DYNINSTgetClock()
{

  int i;
  float elapsed;
  float clockSpeed;
  time64 startF, endF;

  startF = DYNINSTgetCPUtime();
  for (i=0; i < LOOP_LIMIT; i++) {
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
  }
  endF = DYNINSTgetCPUtime();

  elapsed = (endF-startF)/((double) MILLION);
  clockSpeed = (256*LOOP_LIMIT)/elapsed/MILLION;

  /* printf("elapsed = %f\n", elapsed); */
  /* printf("clockSpeed = %f\n", clockSpeed); */

  return(clockSpeed);
}
