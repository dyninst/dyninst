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
 * RTsolaris.c: clock access functions for solaris-2.
************************************************************************/





/************************************************************************
 * header files.
************************************************************************/

#include <sys/time.h>
#include "rtinst/h/rtinst.h"

extern int    gettimeofday(struct timeval *, struct timezone *);
extern void perror(const char *);




/************************************************************************
 * symbolic constants.
************************************************************************/

static const double NANO_PER_USEC = 1.0e3;
static const double MILLION       = 1.0e6;





/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function---currently null.
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
 *
 * XXXX - This should really return time in native units and use normalize
 *	conversion to float abd division are way to expensive to
 *	do everytime we want to read a clock (slows this down 2x) -
 *	jkh 3/9/95
************************************************************************/

time64
DYNINSTgetCPUtime(void) {
    return ((time64)gethrvtime()/(time64)NANO_PER_USEC);
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
    if (gettimeofday(&tv,NULL) == -1) {
        perror("gettimeofday");
        abort();
    }
    return ((time64)tv.tv_sec*(time64)1000000 + (time64)tv.tv_usec);
}
