
//
// Process Critical Path data from the various paradyn daemons.
//
//

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

vector<cpContext*> allCPContexts;

void paradynDaemon::cpDataCallbackFunc(int program,
                                       double timeStamp,
                                       int context,
                                       double total,
                                       double share)
{
    cpContext *conn;
    metricInstance *mi;

    assert(getEarliestFirstTime());
    timeStamp -= getEarliestFirstTime();

    conn = allCPContexts[context];
    if (!conn) {
	conn = new cpContext;

	conn->mi = metricInstance::allMetricInstances[context];
	assert(conn->mi);
	conn->total = total;
	conn->lastTime = timeStamp;
	conn->lastShare = share;
	conn->context = context;
	allCPContexts[context] = conn;
	return;
    }

    if (total > conn->total) {
	mi = conn->mi;
	mi->enabledTime += timeStamp - conn->lastTime;
	share -= conn->lastShare;
	// can go negative.
	// if (share > 0.0) {
	conn->lastShare += share;
	mi->data->addInterval(conn->lastTime, timeStamp, share, FALSE);
	conn->lastTime = timeStamp;
	conn->total = total;
	printf("paradyn got CP message for %d <%f,%f>\n", context, share, total);
        // }
    }
}
