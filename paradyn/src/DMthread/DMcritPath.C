
//
// Process Critical Path data from the various paradyn daemons.
//
/* $Log: DMcritPath.C,v $
/* Revision 1.5  1996/05/08 15:55:41  hollings
/* Commented out a debugging printf
/*
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

    assert(getEarliestFirstTime());
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
