/*
 * DMmain.C: main loop of the Data Manager thread.
 *
 * $Log: DMmain.C,v $
 * Revision 1.2  1994/02/02 00:42:33  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 * Revision 1.1  1994/01/28  01:34:17  hollings
 * The initial version of the Data Management thread.
 *
 *
 */
#include <assert.h>
extern "C" {
#include <math.h>
double   quiet_nan(int unused);
#include <malloc.h>
#include "thread/h/thread.h"
}

#include "dataManager.h"
#include "DMinternals.h"

static dataManager *dm;
stringPool metric::names;
HTable<metric *> metric::allMetrics;
HTable<metricInstance*> component::allComponents;
List<paradynDaemon*> paradynDaemon::allDaemons;

metricInstance *performanceStream::enableDataCollection(resourceList *rl, 
							metric *m)
{
    metricInstance *mi;

    if (!m || !rl) return(NULL);

    mi = m->enabledCombos.find(rl);
    if (mi) {
        mi->count++;
	mi->users.add(this);
    } else {
	mi = appl->enableDataCollection(rl, m);
	if (mi) mi->users.add(this);
    }
    return(mi);
}

//
// Turn off data collection for this perf stream.  Other streams may still
//    get the data.
//
void performanceStream::disableDataCollection(metricInstance *mi)
{
    mi->count--;
    mi->users.remove(this);
    if (!mi->count) {
	appl->disableDataCollection(mi);
    }
}


void performanceStream::enableResourceCreationNotification(resource *r)
{
    r->notify.add(this);
}

void performanceStream::disableResourceCreationNotification(resource *r)
{
    r->notify.remove(this);
}

void performanceStream::callSampleFunc(metricInstance *mi,
				       double startTimeStamp,
				       double endTimeStamp,
				       double value) 
{
    if (dataFunc.sample) {
	dm->setTid(threadId);
	dm->newPerfData(dataFunc.sample, this, mi, startTimeStamp, 
		    endTimeStamp, value);
    }
}

//
// upcalls from remote process.
//
void dynRPCUser::resourceInfoCallback(int program,
				      String parentString,
				      String newResource,
				      String name)
{
    resource *parent;

    // create the resource.
    if (*parentString != '\0') {
	// non-null string.
	parentString = resource::names.findAndAdd(parentString);
	parent = resource::allResources.find(parentString);
	if (!parent) abort();
    } else {
	parent = resource::rootResource;
    }

    createResource(parent, name);
}

//
// used when a new program gets forked.
//
void dynRPCUser::newProgramCallbackFunc(int program)
{
    abort();
}

void dynRPCUser::newMetricCallback(metricInfo info)
{
    addMetric(info);
}

void dynRPCUser::sampleDataCallbackFunc(int program,
				        int mid,
					double startTimeStamp,
					double endTimeStamp,
					double value)
{
    metricInstance *mi;
    performanceStream *ps;
    List<performanceStream*> curr;

    mi = component::allComponents.find((void*) mid);
    if (!mi) {
	printf("ERROR: data for unknown mid: %d\n", mid);
	exit(-1);
    }

    if (mi->components.count() != 1) {
	printf("ERROR: multiple data sources for one mi, not supported yet\n");
	exit(-1);
    }

    mi->data->addInterval(startTimeStamp, endTimeStamp, value, FALSE);

    //
    // call callbacks for perfstreams.
    // 
    for (curr = mi->users; ps = *curr; curr++) {
	ps->callSampleFunc(mi, startTimeStamp, endTimeStamp,value);
    }
}

//
// Main loop for the dataManager thread.
//
void *DMmain(int arg)
{
    int ret;
    unsigned int tag;
    List<paradynDaemon*> curr;

    thr_name("Data Manager");
    printf("mm running\n");

    dm = new dataManager(arg);

    while (1) {
	tag = MSG_TAG_ANY;
	ret = msg_poll(&tag, TRUE);
	assert(ret != THR_ERR);
	if (tag == MSG_TAG_FILE) {
	    // must be an upcall on something speaking the dynRPC protocol.
	    for (curr = paradynDaemon::allDaemons; *curr; curr++) {
		if ((*curr)->fd == ret) {
		    (*curr)->awaitResponce(-1);
		}
	    }
	} else {
	    dm->mainLoop(); 
	}
    }
}

void addMetric(metricInfo info)
{
    char *iName;
    metric *met;
    performanceStream *stream;
    List<performanceStream *> curr;

    iName = metric::names.findAndAdd(info.name);
    met = metric::allMetrics.find(iName);
    if (met) {
	// check that it is compatible ????
	return;
    }

    //
    // It's really new 
    //
    met = new metric(info);
    metric::allMetrics.add(met, iName);

    //
    // now tell all perfStreams
    //
    for (curr = applicationContext::streams; *curr; curr++) {
	stream = *curr;
	if (stream->controlFunc.mFunc) {
	    // set the correct destination thread.
	    dm->setTid(stream->threadId);
	    dm->newMetricDefined(stream->controlFunc.mFunc, stream, met);
	}
    }
}

