/*
 * Copyright (c) 1996-1998 Barton P. Miller
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
/* $Id: DMcritPath.C,v 1.12 2001/08/23 14:43:35 schendel Exp $ */

#include <assert.h>
#include "../pdMain/paradyn.h"
extern "C" {
#include <malloc.h>
#include "thread/h/thread.h"
#include <stdio.h>
}

#include "dataManager.thread.SRVR.h"
#include "dyninstRPC.xdr.CLNT.h"
#include "DMmetric.h"
#include "DMdaemon.h"
#include "../UIthread/Status.h"
#include "common/h/Time.h"
#include "pdutil/h/pdSample.h"

class cpContext {
public:
  cpContext() : lastTime(timeStamp::ts1970()), total(pdSample::Zero()), 
    lastShare(pdSample::Zero()), mi(NULL)  { }
  timeStamp lastTime;
  pdSample  total;
  pdSample  lastShare;
  metricInstance *mi;
  int context;
};

dictionary_hash<metricInstanceHandle,cpContext *> allCPContexts(uiHash);

void paradynDaemon::cpDataCallbackFunc(int,
                                       double tStamp,
                                       int context,
                                       double total,
                                       double share)
{
    cpContext *conn;
    metricInstance *mi;

    timeStamp curTime = timeStamp(tStamp, timeUnit::sec(), timeBase::bStd());
    pdSample inTotal(static_cast<int64_t>(total));
    pdSample inShare(static_cast<int64_t>(share));
    timeStamp aval = getEarliestStartTime();
    assert(aval > timeStamp::ts1970());

    if (!allCPContexts.defines(context)) {
	conn = new cpContext;

	conn->mi = metricInstance::allMetricInstances[context];
	assert(conn->mi);
	conn->total = inTotal;
	conn->lastTime = curTime;
	conn->lastShare = inShare;
	conn->context = context;
	allCPContexts[context] = conn;
	return;
    } else {
	conn = allCPContexts[context];
    }

    if (inTotal > conn->total) {
	mi = conn->mi;
	timeStamp earliestFirstTime = mi->getEarliestFirstTime();
	relTimeStamp relStartTime = relTimeStamp(conn->lastTime - 
						 earliestFirstTime);
	relTimeStamp relEndTime = relTimeStamp(curTime - earliestFirstTime);

	mi->enabledTime += relEndTime - relStartTime;
	inShare -= conn->lastShare;
	// can go negative.
	// if (share > 0.0) {
	conn->lastShare += inShare;
	mi->addInterval(relStartTime, relEndTime, inShare);
	conn->lastTime = curTime;
	conn->total = inTotal;
    }
}
