/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.2  1994/02/01 18:46:52  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:27  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>

#include "util/h/list.h"
#include "rtinst/h/rtinst.h"

#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "dyninstP.h"
#include "metric.h"


dynRPC *tp;
extern void controllerMainLoop();
extern void initLibraryFunctions();

main(int argc, char *argv[])
{
    int i;
    metricList stuff;

    initLibraryFunctions();

    if (argc != 1) {
        printf("remote start not supported\n");
        exit(-1);
    }
    tp = new dynRPC(0, NULL, NULL);

    //
    // tell client about our metrics.
    //
    stuff = getMetricList();
    for (i=0; i < stuff->count; i++) {
	tp->newMetricCallback(stuff->elements[i].info);
    }

    controllerMainLoop();
}

void dynRPC::addResource(String parent, String name)
{
    resource pr;
    extern resource findResource(char*);

    pr = findResource(parent);
    if (pr) (void) newResource(pr, NULL, name, 0.0);
}

void dynRPC::coreProcess(int pid)
{
    process *proc;

    proc = processList.find((void *) pid);
    dumpCore(proc);
}

String dynRPC::getStatus(int pid)
{
    process *proc;
    extern char *getProcessStatus(process *proc);

    proc = processList.find((void *) pid);
    return(getProcessStatus(proc));
}

//
// NOTE: This version of getAvailableMetrics assumes that new metrics are
//   NOT added during execution.
//
metricInfo_Array dynRPC::getAvailableMetrics(void)
{
    int i;
    static int inited;
    static metricInfo_Array metInfo;

    if (!inited) {
	metricList stuff;

	stuff = getMetricList();
	metInfo.count = stuff->count;
	metInfo.data = (metricInfo*) calloc(sizeof(metricInfo), metInfo.count);
	for (i=0; i < metInfo.count; i++) {
	    metInfo.data[i] = stuff->elements[i].info;
	}
	inited = 1;
    }
    return(metInfo);
}

double dynRPC::getPredictedDataCost(String_Array focusString, String metric)
{
    metric m;
    double val;
    resourceList l;

    m = findMetric(metric);
    l = findFocus(focusString.count, focusString.data);
    if (!l) return(0.0);
    val = guessCost(l, m);

    return(val);
}

void dynRPC::disableDataCollection(int mid)
{
    metricInstance mi;
    extern void printResourceList(resourceList);

    printf("disable of %s for RL =", getMetricName(mi->met));
    printResourceList(mi->resList);
    printf("\n");

    mi = allMIs.find((void *) mid);
    mi->disable();
    allMIs.remove(mi);
    delete(mi);
}

int dynRPC::enableDataCollection(String_Array foucsString,String metric)
{
    int id;
    metric m;
    resourceList l;

    m = findMetric(metric);
    l = findFocus(foucsString.count, foucsString.data);
    if (!l) return(-1);
    id = startCollecting(l, m);
    return(id);
}

//
// not implemented yet.
//
void dynRPC::setSampleRate(double sampleInterval)
{
    return;
}

Boolean dynRPC::detachProgram(int program,Boolean pause)
{
    return(detachProcess(program, pause));
}

//
// Continue program.  We really should make this work on a single pid.
//
void dynRPC::continueProgram(int program)
{
    continueApplication();
}
//
// We really should make this work on a single pid.
//
Boolean dynRPC::pauseProgram(int program)
{
    return(pauseApplication());
}

Boolean dynRPC::startProgram(int program)
{
    continueApplication();
    return(False);
}

//
// This is not implemented yet.
//
Boolean dynRPC::attachProgram(int pid)
{
    return(FALSE);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(int argc,String_Array argv)
{
    return(addProcess(argc, argv.data));
}
