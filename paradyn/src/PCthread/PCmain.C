#include <assert.h>
#include <stdlib.h>


#include "thread/h/thread.h"
#include "dataManager.CLNT.h"
#include "performanceConsultant.SRVR.h"
#include "PCglobals.h"
#include "PCmetric.h"

extern int rootThread;
performanceStream *pcStream;
extern void initResources();
extern void shgInit();

void PCnewData(performanceStream *ps,
	       metricInstance *mi,
	       timeStamp startTimeStamp,
	       timeStamp endTimeStamp, 
	       sampleValue value)
{
    datum *dp;

    // printf("mi %x = %f %f to %f\n", mi, value, startTimeStamp, endTimeStamp);
    dp = miToDatumMap.find(mi);
    assert(dp);
    dp->newSample(startTimeStamp, endTimeStamp, value);
}

void PCnewInfo()
{
}



void PCmetricFunc(performanceStream *ps, metric *met)
{
    char *name;
    PCmetric *pcMet;
    extern stringPool metricStrings;

    name = metricStrings.findAndAdd(dataMgr->getMetricName(met));
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

    thr_name("PerformanceConsultant");
    // ??? do inits and waits.
    pc = new performanceConsultant(arg);

    initResources();

    controlHandlers.mFunc = PCmetricFunc;
    controlHandlers.rFunc = NULL;
    dataHandlers.sample = (sampleDataCallbackFunc) PCnewData;
    pcStream = dataMgr->createPerformanceStream(context, Sample, 
	dataHandlers, controlHandlers);

    // now find about of exhisting metrics.
    mets = dataMgr->getAvailableMetrics(context);
    for (i=0; i < mets.count; i++) {
	assert(mets.data[i]);
	met = dataMgr->findMetric(context, mets.data[i]);
	PCmetricFunc(pcStream, met);
    }

    shgInit();

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
