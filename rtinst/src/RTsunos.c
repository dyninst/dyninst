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
 * RTsunos.c: SunOs-4.1.3 specific clock functions.
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





/************************************************************************
 * caddr_t DYNINSTprobeUarea(void)
 *
 * test to see if uarea of the process can be mapped.  if so, return
 * the address of the uarea, else return null.
************************************************************************/

static caddr_t
DYNINSTprobeUarea(void) {
    char*        cmd;
    kvm_t*       kd;
    char**       args;
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

    kvm_getcmd(kd, p, u, &args, 0);
    if ((cmd = (char *) strrchr(*args, '/'))) {
        cmd++;
    }
    else {
        cmd = *args;
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

static          int       DYNINSTmappedUarea = 0;
static volatile unsigned* _p_1               = 0;
static volatile unsigned* _p_2               = 0;

static void
DYNINSTmapUarea(void) {
    int     ret;
    int     kmem;
    caddr_t uAddr;
    static struct user* u;

    if (!(uAddr = DYNINSTprobeUarea())) {
        printf("WARNING: program compiled for wrong version of SPARC chip.\n");
        printf(" using getrusage for times, this may slow your program down\n");
        printf(" by a factor of ten or more.\n");
        printf("\n");
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

    u = (struct user *) ret;
    DYNINSTmappedUarea = 1;

    _p_1 = (int *) &(u->u_ru.ru_utime.tv_sec);
    _p_2 = (int *) &(u->u_ru.ru_utime.tv_usec);
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
    struct tms    t;
    unsigned long p1;
    time64        tm;

    if (DYNINSTmappedUarea) {
retry:
        p1 = *_p_1;
        tm = (p1*MILLION + *_p_2);

        if (p1 != (*_p_1)) {
            goto retry;
        }
        return tm;
    }

    if (times(&t) == -1) {
        perror("times");
        abort();
    }
    return (time64) ((t.tms_utime+t.tms_stime)*MILLION/HZ);
}





/************************************************************************
 * time64 DYNINSTgetWalltime(void)
 *
 * get the total walltime used by the monitored process.
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetWalltime(void) {
    struct timeval tv;
    if (gettimeofday(&tv, 0) == -1) {
        perror("gettimeofday");
        abort();
    }
    return (time64) (tv.tv_sec*MILLION + tv.tv_usec);
}
