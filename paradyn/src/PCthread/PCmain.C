
/* $Log: PCmain.C,v $
/* Revision 1.12  1994/05/18 00:48:53  hollings
/* Major changes in the notion of time to wait for a hypothesis.  We now wait
/* until the earlyestLastSample for a metrics used by a hypothesis is at
/* least sufficient observation time after the change was made.
/*
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

#include <assert.h>
#include <stdlib.h>


#include "thread/h/thread.h"
#include "dataManager.CLNT.h"
#include "performanceConsultant.SRVR.h"
#include "PCglobals.h"
#include "PCmetric.h"
#include "../pdMain/paradyn.h"

performanceStream *pcStream;
extern void initResources();
extern void PCevaluateWorld();
extern thread_t MAINtid;
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

    for (i=0, total = 0.0; i < count; i++) {
	// printf("mi %x = bin %d == %f\n", mi, i+first, buckets[i]);
	total += buckets[i];
    }
    total *= PCbucketWidth;

    dp = miToDatumMap.find(mi);
    assert(dp);
    start = PCbucketWidth * first;
    end = PCbucketWidth * (first + count);

    // see if we should check for new state change.
    //  we wait until time moves, otherwise we evaluate hypotheses too often.
    if (end > PCcurrentTime) {
	PCcurrentTime = end;
	PCevaluateWorld();
    }

    dp->newSample(start, end, total);
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
        printf("WARNING performance consultant has no use for %s\n", name);
        pcMet = new PCmetric(name);
    }
    pcMet->met = met;
}

void PCmain(int arg)
{
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
    controlHandlers.mFunc = PCmetricFunc;
    controlHandlers.rFunc = NULL;
    controlHandlers.fFunc = PCfold;
    dataHandlers.sample = PCnewData;
    pcStream = dataMgr->createPerformanceStream(context, Sample, 
	dataHandlers, controlHandlers);
    PCbucketWidth = dataMgr->getCurrentBucketWidth();

    // now find about existing metrics.
    mets = dataMgr->getAvailableMetrics(context);
    for (i=0; i < mets.count; i++) {
	assert(mets.data[i]);
	met = dataMgr->findMetric(context, mets.data[i]);
	PCmetricFunc(pcStream, met);
    }

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
