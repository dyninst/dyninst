
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

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, \
  Karen Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/clock.C,v 1.4 1994/10/25 22:16:34 hollings Exp $";
#endif

/*
 *
 * Return the number of cycles the machine can issue per second.
 *
 * This may need to be made specific to different platforms.
 *
 * $Log: clock.C,v $
 * Revision 1.4  1994/10/25 22:16:34  hollings
 * added default clock speed to prevent divide by zero problems with
 * the cost model.
 *
 * Revision 1.3  1994/10/13  07:24:30  krisna
 * solaris porting and updates
 *
 * Revision 1.2  1994/09/22  01:45:55  markc
 * Made system includes extern "C"
 *
 * Revision 1.1  1994/09/20  18:18:23  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 *
 */

extern "C" {
#include <stdio.h>
#include <string.h>
}

#include "util.h"

//
// If we can't read it, try to guess it.
//
float guessClock()
{
    printf("pid %d: using default clock speed of %d\n", getpid(), 33000000);
    return(33000000.0);
}

#if defined(SPARC_SUN_SUNOS4_1_3)
#define PATTERN	"\tclock-frequency:"
//
// find the number of cycles per second on this machine.
//
float getCyclesPerSecond()
{
    FILE *fp;
    char *ptr;
    int speed;
    char line[80];

    fp = popen("/usr/etc/devinfo -vp", "r");
    if (!fp) {
	// can't run command so guess the clock rate.
    } else {
	while (fgets(line, sizeof(line), fp)) {
	    if (!strncmp(PATTERN, line, strlen(PATTERN))) {
		ptr = strchr(line, ' ');
		if (!ptr) return(guessClock());
		speed = strtol(ptr, NULL, 16);
		break;
	    }
	}
    }
    pclose(fp);
    sprintf(line, "Clock = %d\n", speed);
    logLine(line);
    return(speed);
}
#endif

#if defined(SPARC_SUN_SOLARIS2_3)

/************************************************************************
 * header files.
************************************************************************/

#include <sys/types.h>
#include <sys/time.h>





/************************************************************************
 * float getCyclesPerSecond()
 *
 * need a well-defined method for finding the CPU cycle speed
 * on each CPU.
************************************************************************/

static const double BILLION = 1.0e9;

#define NOPS_4  asm("nop"); asm("nop"); asm("nop"); asm("nop")
#define NOPS_16 NOPS_4; NOPS_4; NOPS_4; NOPS_4

float
getCyclesPerSecond() {
    int            i;
    hrtime_t       start_cpu;
    hrtime_t       end_cpu;
    hrtime_t       elapsed;
    double         speed;
    const unsigned LOOP_LIMIT = 50000;

    start_cpu = gethrvtime();
    for (i = 0; i < LOOP_LIMIT; i++) {
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
        NOPS_16; NOPS_16; NOPS_16; NOPS_16;
    }
    end_cpu = gethrvtime();
    elapsed = end_cpu - start_cpu;
    speed   = (BILLION*256*LOOP_LIMIT)/elapsed;

printf("clock speed = %g\n", speed);

    return speed;
}
#endif
