/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */





/************************************************************************
 * RTsunos.c: SunOs-4.1.3 specific functions.
 *
 * $Log: RTsunos.c,v $
 * Revision 1.6  1996/02/19 22:22:16  naim
 * Fixing DYNINSTgetWalltime. gettimeofday was going backwards. If it does,
 * we retry again - naim
 *
 * Revision 1.5  1996/02/01  17:48:39  naim
 * Fixing some problems related to timers and race conditions. I also tried to
 * make a more standard definition of certain procedures (e.g. reportTimer)
 * across all platforms - naim
 *
 * Revision 1.4  1995/08/24  15:12:44  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.3  1995/03/10  19:38:03  hollings
 * Removed use of floating point to compute times. This speed up
 * the timer routines by a factor of 2-3.
 *
 * Added DYNINSTuseGetrusage environment variable to force use of getrusage over
 * the mmaped uarea.
 *
 *
 ************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <machine/vmparam.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/proc.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/user.h>
#include <unistd.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"

extern int getrusage (int, struct rusage *);
extern char *getenv(char *);




/************************************************************************
 * caddr_t DYNINSTprobeUarea(void)
 *
 * test to see if uarea of the process can be mapped.  if so, return
 * the address of the uarea, else return null.
************************************************************************/

static caddr_t
DYNINSTprobeUarea(void) {
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
 * void DYNINSTmapUarea(void)
 *
 * map urea of the process into its accessible address space.  if
 * such a mapping is possible, set clock pointers for fast access,
 * else use slower system calls.
************************************************************************/

/*
 * NOTE: This could be made static, but gdb has trouble finding them if
 *   they are static.  jkh 3/8/95
 */
int DYNINSTmappedUarea = 0;
volatile unsigned* DYNINSTuareaTimeSec = 0;
volatile unsigned* DYNINSTuareaTimeUsec = 0;
volatile unsigned* DYNINSTuareaSTimeSec = 0;
volatile unsigned* DYNINSTuareaSTimeUsec = 0;
struct rusage *DYNINSTrusagePtr;
struct user *DYNINSTuareaPtr;

static void
DYNINSTmapUarea(void) {
    int     ret;
    int     kmem;
    caddr_t uAddr;
    static struct user* u;

    if (getenv("DYNINSTuseGetrusage")) {
        printf("Using getrusage\n");
        fflush(stdout);
        return;
    }

    uAddr = DYNINSTprobeUarea();
    if (!uAddr) {
	printf("WARNING: unable to map uarea, using getrusage.\n");
	printf(" This may slow your program down quite a bit.\n");
	fflush(stdout);
	return;
    }

    if ((kmem = open("/dev/kmem", O_RDONLY, 0)) == -1) {
        perror("/dev/kmem");
        return;
    }

    if ((ret = (int) mmap(0, sizeof(struct user), PROT_READ, MAP_SHARED,
        kmem, (off_t) uAddr)) == -1) {
        perror("mmap");
        return;
    }

    DYNINSTuareaPtr = (struct user *) ret;
    u = (struct user *) ret;
    DYNINSTmappedUarea = 1;

    DYNINSTuareaTimeSec = (int *) &(u->u_ru.ru_utime.tv_sec);
    DYNINSTuareaTimeUsec = (int *) &(u->u_ru.ru_utime.tv_usec);
    DYNINSTuareaSTimeSec = (int *) &(u->u_ru.ru_stime.tv_sec);
    DYNINSTuareaSTimeUsec = (int *) &(u->u_ru.ru_stime.tv_usec);
}





/************************************************************************
 * void DYNINSTos_init(void)
 *
 * run any operating system dependent initialization.  this function
 * will be called from DYNINSTinit().
 *
 * for SunOs, simply try to map the process uarea.
************************************************************************/

void
DYNINSTos_init(void) {
    DYNINSTmapUarea();
}





/************************************************************************
 * symbolic constants.
************************************************************************/

static const double MILLION = 1000000.0;





/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * get the total CPU time used for by the monitored process.
 * return value is in usec units.
 *
 * while reading mapped clocks, there can be a race condition between
 * the updates of the sec/usec fields, hence the retry loop.
************************************************************************/

time64
DYNINSTgetCPUtime(void) {
    unsigned long p1, p2;
    time64 now;
    static time64 previous=0;
    struct rusage ru;

    if (DYNINSTmappedUarea) {
retry:
        p1 = *DYNINSTuareaTimeSec;
        p2 = *DYNINSTuareaSTimeSec;
	/* This MUST not use float constant MILLION - jkh 2/8/95 */
        now  = (time64)p1 + (time64)p2;
        now *= (time64)1000000;
        now += (time64)(*DYNINSTuareaTimeUsec) + 
               (time64)(*DYNINSTuareaSTimeUsec);
        if (p1 != (*DYNINSTuareaTimeSec) || p2 != (*DYNINSTuareaSTimeSec)) {
          goto retry;
        }        
        if (now<previous) {
          goto retry;
        }
        previous=now;
        return now;
    }

try_again:    
    if (!getrusage(RUSAGE_SELF, &ru)) {
      now = (time64)ru.ru_utime.tv_sec + (time64)ru.ru_stime.tv_sec;
      now *= (time64)1000000;
      now += (time64)ru.ru_utime.tv_usec + (time64)ru.ru_stime.tv_usec;
      if (now<previous) {
        goto try_again;
      }
      previous=now;
      return now;
    }
    else {
      perror("getrusage");
      abort();
    }
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetWalltime(void) {
    static time64 previous=0;
    time64 now;
    struct timeval tv;

retryWT:
    if (gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday");
        abort();
    }
    now = (time64)tv.tv_sec*(time64)1000000 + (time64)tv.tv_usec;
    if (now < previous) {
      goto retryWT;
    }
    previous = now;
    return (now);
}

/*
 * DYNINSTgetRusage(id) - Return the value of various OS stats.
 *
 *    The id is an integer, and when they are changed, any metric that uses
 *        DYNINSTgetRusage will also need to be updated.
 *
 */
int DYNINSTgetRusage(int id)
{
    int value;
    struct rusage rusage;

    if (!DYNINSTmappedUarea) {
	getrusage(0, &rusage);
	DYNINSTrusagePtr = &rusage;
    } else {
	DYNINSTrusagePtr = &DYNINSTuareaPtr->u_ru;
    }
    switch (id) {
	case 0:	/* page faults */
	    value = DYNINSTrusagePtr->ru_minflt+DYNINSTrusagePtr->ru_majflt;
	    break;
	case 1:	/* swaps */
	    value = DYNINSTrusagePtr->ru_nswap;
	    break;
	case 2: /* signals received */
	    value = DYNINSTrusagePtr->ru_nsignals;
	    break;
	case 3: /* max rss */
	    value = DYNINSTrusagePtr->ru_maxrss;
	    break;
	case 4: /* context switches */
	    value = DYNINSTrusagePtr->ru_nvcsw + DYNINSTrusagePtr->ru_nivcsw;
	    break;
	default:
	    value = 0;
	    break;
    }
    return value;
}
