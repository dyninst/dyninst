
/*
 * This file contains the implementation of runtime dynamic instrumentation
 *   functions for a normal Sparc with SUNOS.
 *
 * $Log: RTsparc.c,v $
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
 * Revision 1.3  1994/02/02  00:46:12  hollings
 * Changes to make it compile with the new tree.
 *
 * Revision 1.2  1993/12/13  19:47:52  hollings
 * corrected rindex parameter error
 *
 * Revision 1.1  1993/08/26  19:43:28  hollings
 * Initial revision
 *
 * Revision 1.1  1993/07/02  21:49:35  hollings
 * Initial revision
 *
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

#include "kludges.h"
#include "../h/rtinst.h"

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
