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
 * RTgeneric.c: generic timer functions for POSIX systems
************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>

#include "kludges.h"
#include "rtinst/h/rtinst.h"





/************************************************************************
 * symbolic constants.
************************************************************************/

static const double MILLION = 1000000.0;





/************************************************************************
 * void DYNINSTos_init(void)
 *
 * null initialization for generic operating system.
************************************************************************/

void
DYNINSTos_init(void) {
}





/************************************************************************
 * time64 DYNINSTgetCPUtime(void)
 *
 * get the total CPU time used for "an" LWP of the monitored process.
 * this functions needs to be rewritten if a per-thread CPU time is
 * required.  time for a specific LWP can be obtained via the "/proc"
 * filesystem.
 * return value is in usec units.
************************************************************************/

time64
DYNINSTgetCPUtime(void) {
    struct tms t;
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
