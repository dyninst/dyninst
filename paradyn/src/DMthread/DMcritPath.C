/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

//
// Process Critical Path data from the various paradyn daemons.
//
/* $Log: DMcritPath.C,v $
/* Revision 1.7  1996/11/26 16:06:47  naim
/* Fixing asserts - naim
/*
 * Revision 1.6  1996/08/16 21:01:31  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.5  1996/05/08 15:55:41  hollings
 * Commented out a debugging printf
 *
 * Revision 1.4  1996/02/02  18:27:06  newhall
 * fixed compile error
 *
 * Revision 1.3  1996/02/01  19:51:58  hollings
 * Fixing Critical Path to work.
 *
 */

#include <assert.h>
extern "C" {
double   quiet_nan();
#include <malloc.h>
#include "thread/h/thread.h"
#include <stdio.h>
}

#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMmetric.h"
#include "DMdaemon.h"
#include "../pdMain/paradyn.h"
#include "../UIthread/Status.h"

class cpContext {
    public:
	cpContext() { lastTime = 0.0; total = 0.0; mi = NULL; lastShare = 0.0; }
	double	lastTime;
	double 	total;
	double  lastShare;
	metricInstance *mi;
	int context;
};

dictionary_hash<metricInstanceHandle,cpContext *> allCPContexts(uiHash);

void paradynDaemon::cpDataCallbackFunc(int,
                                       double timeStamp,
                                       int context,
                                       double total,
                                       double share)
{
    cpContext *conn;
    metricInstance *mi;

    bool aflag;
    aflag=getEarliestFirstTime();
    assert(aflag);
    timeStamp -= getEarliestFirstTime();

    if (!allCPContexts.defines(context)) {
	conn = new cpContext;

	conn->mi = metricInstance::allMetricInstances[context];
	assert(conn->mi);
	conn->total = total;
	conn->lastTime = timeStamp;
	conn->lastShare = share;
	conn->context = context;
	allCPContexts[context] = conn;
	return;
    } else {
	conn = allCPContexts[context];
    }

    if (total > conn->total) {
	mi = conn->mi;
	mi->enabledTime += timeStamp - conn->lastTime;
	share -= conn->lastShare;
	// can go negative.
	// if (share > 0.0) {
	conn->lastShare += share;
	mi->addInterval(conn->lastTime, timeStamp, share, FALSE);
	conn->lastTime = timeStamp;
	conn->total = total;
	// printf("paradyn got CP message for %d <%f,%f>\n", context, share, total);
        // }
    }
}
