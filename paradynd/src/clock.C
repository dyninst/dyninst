
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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/clock.C,v 1.2 1994/09/22 01:45:55 markc Exp $";
#endif

/*
 *
 * Return the number of cycles the machine can issue per second.
 *
 * This may need to be made specific to different platforms.
 *
 * $Log: clock.C,v $
 * Revision 1.2  1994/09/22 01:45:55  markc
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
    return(0.0);
}

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
