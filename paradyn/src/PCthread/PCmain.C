
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

/* $Log: PCmain.C,v $
/* Revision 1.20  1994/08/03 19:09:49  hollings
/* split tunable constant into float and boolean types
/*
/* added tunable constant for printing tests as they avaluate.
/*
/* added code to compute the min interval data has been enabled for a single
/* test rather than using a global min.  This prevents short changes from
/* altering long term trends among high level hypotheses.
/*
 * Revision 1.19  1994/07/28  22:33:59  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.18  1994/07/25  04:47:05  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.17  1994/06/27  21:24:39  rbi
 * New abstraction parameter for performance streams
 *
 * Revision 1.16  1994/06/27  18:55:08  hollings
 * Added compiler flag to add SHG nodes to dag only on first evaluation.
 *
 * Revision 1.15  1994/06/22  22:58:19  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.14  1994/06/17  00:12:28  hollings
 * fixed the init of the control callback structure.
 *
 * Revision 1.13  1994/06/12  22:40:49  karavan
 * changed printf's to calls to status display service.
 *
 * Revision 1.12  1994/05/18  00:48:53  hollings
 * Major changes in the notion of time to wait for a hypothesis.  We now wait
 * until the earlyestLastSample for a metrics used by a hypothesis is at
 * least sufficient observation time after the change was made.
 *
 * Revision 1.11  1994/05/12  23:34:06  hollings
 * made path to paradyn.h relative.
 *
 * Revision 1.10  1994/05/10  03:57:43  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.9  1994/05/06  06:39:34  karavan
 * SHG now initialized only upon request
 *
 * Revision 1.8  1994/05/02  20:38:10  hollings
 * added pause search mode, and cleanedup global variable naming.
 *
 * Revision 1.7  1994/04/21  05:00:10  karavan
 * added global SHGid for visual display.
 *
 * Revision 1.6  1994/04/06  21:24:10  markc
 * First log message.
 * */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/PCmain.C,v 1.20 1994/08/03 19:09:49 hollings Exp $";
#endif

#include <assert.h>
#include <stdlib.h>
#include <memory.h>


#include "thread/h/thread.h"
#include "util/h/tunableConst.h"
#include "dataManager.CLNT.h"
#include "performanceConsultant.SRVR.h"
#include "UI.CLNT.h"
#include "PCglobals.h"
#include "PCmetric.h"
#include "../src/pdMain/paradyn.h"
#include "../src/UIthread/UIstatDisp.h"
#include "../src/DMthread/DMresource.h"

performanceStream *pcStream;
extern void initResources();
extern void PCevaluateWorld();
extern thread_t MAINtid;
extern timeStamp PCstartTransTime;
extern timeStamp PCendTransTime;

statusDisplayObj *PCstatusDisplay;   // token needed for PC status calls 

int SHGid;             // id needed for Search History Graph uim dag calls
static float PCbucketWidth;

void PCfold(performanceStream *ps,
	    timeStamp newWidth)
{
    PCbucketWidth = newWidth;
}

void PCnewData(performanceStream *ps,
	       metricInstance *mi,
	       sampleValue *buckets,
	       int count, 
	       int first)
{
    int i;
    datum *dp;
    sampleValue total;
    timeStamp start, end;
    extern tunableBooleanConstant pcEvalPrint;

    for (i=0, total = 0.0; i < count; i++) {
	// printf("mi %x = bin %d == %f\n", mi, i+first, buckets[i]);
	total += buckets[i];
    }
    total *= PCbucketWidth;

    dp = miToDatumMap.find(mi);
    assert(dp);
    start = PCbucketWidth * first;
    end = PCbucketWidth * (first + count);

    if (pcEvalPrint.getValue()) {
	cout << "AR: " << dp->metName << dp->resList->getCanonicalName();
	cout << " = " << total;
	cout << " from " << start << " to " << end << "\n";
    }

    dp->newSample(start, end, total);

    // see if we should check for new state change.
    //  we wait until time moves, otherwise we evaluate hypotheses too often.
    if (end > PCcurrentTime) {
	PCcurrentTime = end;
	if (PCstartTransTime < PCendTransTime) {
	    PCevaluateWorld();
	}
    }
}

void PCnewInfo()
{
}



void PCmetricFunc(performanceStream *ps, metric *met)
{
    char *name;
    PCmetric *pcMet;
    extern stringPool PCmetricStrings;

    name = PCmetricStrings.findAndAdd(dataMgr->getMetricName(met));
    pcMet = (PCmetric *) allMetrics.find(name);
    if (!pcMet) {
	// This warning was intended to make it easy to catch typos in metric
	//   names between paradynd and the PC.  However, there are now several
	//   metrics that paradynd defines that the PC doesn't need.
	//   - jkh 6/25/94
        // printf("WARNING performance consultant has no use for %s\n", name);
        pcMet = new PCmetric(name);
    }
    pcMet->met = met;
}

void PCmain(void* varg)
{
    int arg; memcpy((void *) &arg, varg, sizeof arg);

    int i;
    int from;
    metric *met;
    unsigned int tag;
    String_Array mets;
    performanceConsultant *pc;
    union dataCallback dataHandlers;
    struct controlCallback controlHandlers;
    char PCbuff[64];
    unsigned int msgSize = 64;

    thr_name("PerformanceConsultant");
    // ??? do inits and waits.

    pc = new performanceConsultant(MAINtid);

    msg_send (MAINtid, MSG_TAG_PC_READY, (char *) NULL, 0);
    tag = MSG_TAG_ALL_CHILDREN_READY;
    msg_recv (&tag, PCbuff, &msgSize);
    initResources();

    // make sure memory is clear.
    memset(&controlHandlers, '\0', sizeof(controlHandlers));
    controlHandlers.mFunc = PCmetricFunc;
    controlHandlers.fFunc = PCfold;

    dataHandlers.sample = PCnewData;
    pcStream = dataMgr->createPerformanceStream(context, Sample, BASE,
	dataHandlers, controlHandlers);
    PCbucketWidth = dataMgr->getCurrentBucketWidth();

    // now find about existing metrics.
    mets = dataMgr->getAvailableMetrics(context);
    for (i=0; i < mets.count; i++) {
	assert(mets.data[i]);
	met = dataMgr->findMetric(context, mets.data[i]);
	PCmetricFunc(pcStream, met);
    }

    // initialize PC status display object
    PCstatusDisplay = uiMgr->initStatusDisplay (PC_STATUSDISPLAY);

    while (1) {
	tag = MSG_TAG_ANY;
	from = msg_poll(&tag, TRUE);
	assert(from != THR_ERR);
	if (dataMgr->isValidUpCall(tag)) {
	    dataMgr->awaitResponce(-1);
	} else {
	    pc->mainLoop();
	}
    }
}
