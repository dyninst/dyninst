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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/perfStream-cm5.C,v 1.6 1996/02/08 23:03:43 newhall Exp $";
#endif



/*
 * File containing the CM5 specific traceHandling stuff.  Will we
 * still need this after changing to synchronous sampling of the nodes?
 *
 * $Log: perfStream-cm5.C,v $
 * Revision 1.6  1996/02/08 23:03:43  newhall
 * fixed Ave. aggregation for CM5 daemons, Max and Min don't work, but are
 * approximated by ave rather than sum
 *
 * Revision 1.5  1994/11/09  18:40:29  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.4  1994/11/02  11:13:46  markc
 * Removed compiler warnings.
 *
 * Revision 1.3  1994/07/26  20:00:52  hollings
 * switch to select based polling of nodes.
 *
 * Revision 1.2  1994/07/21  01:34:49  hollings
 * removed extra polls of the nodes for printfs.
 *
 * Revision 1.1  1994/07/14  14:45:52  jcargill
 * Added new file for dynRPC functions, and a default (null) function for
 * processArchDependentTraceStream, and the cm5 version.
 *
 */

#include <cm/cmmd.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "rtinst/src/traceio.h"
#include "primitives.h"
#include "util.h"
#include "dyninst.h"
#include "metric.h"

void processArchDependentTraceStream()
{
    timeStamp now;
    static timeStamp last=0;
    extern float SAMPLEnodes;
    extern void sampleNodes();

    now = getCurrentTime(false);
    if (now > last + SAMPLEnodes) {
	sampleNodes();
	last = now;
    }
}
