
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

/*
 * $Log: PCwhen.C,v $
 * Revision 1.5  1994/07/28 22:34:05  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.4  1994/06/29  02:56:26  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.3  1994/06/22  22:58:25  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.2  1994/05/18  00:48:57  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.1  1994/02/02  00:38:21  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.5  1993/08/05  18:58:43  hollings
 * new includes.
 *
 * Revision 1.4  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:29:48  hollings
 * fixed rcs log entry.
 *
 * Revision 1.1  1992/08/24  15:06:33  hollings
 * Initial revision
 *
 *
 * Implementation of then when axis (temporal).
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/Attic/PCwhen.C,v 1.5 1994/07/28 22:34:05 krisna Exp $";
#endif


#include <assert.h> 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dataManager.h"
#include "PCwhen.h"

static int nextId;

timeInterval::timeInterval(timeInterval* parent,timeStamp s,timeStamp e,char *n)
{

    start = s;
    end = e;
    name = n;
    id = nextId++;
    subIntervals = new(timeIntervalList);
    allTimeIntervals.add(this);
    if (parent) {
	assert(parent->start <= start);
	assert(parent->end >= end);
	parent->subIntervals->add(this);
    }
}

void timeInterval::print(int level)
{
    char name[80];

    print(level, name);
    printf("%s\n", name);
}

void timeInterval::print(int level, char *str)
{
    int i;
    timeIntervalList curr;

    if (level >= 0) {
	for (i=0; i < level;i++) sprintf(str, "    ");
	sprintf(str, "i%d: ", id);
	if (name) sprintf(str, "name = %s ", name);
	sprintf(str, "start = %f, end = %f\n", start, end);
	if (subIntervals) {
	    for (curr = *subIntervals; *curr; curr++) {
		(*curr)->print(level+1);
	    }
	}
    } else if (end == HUGE_VAL) {
	sprintf(str, "entire exeuction");
    } else {
	sprintf(str, "%5.2f to %5.2f", start, end);
    }
}

// This must be before whenAxis so that whenAxis appears in allTimeIntervals.
timeIntervalList allTimeIntervals;

timeInterval *currentInterval;
timeInterval whenAxis(NULL, 0.0, 0.0, "ALL");
