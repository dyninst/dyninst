/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/metricFocusNode.C,v 1.10 1994/04/12 15:29:20 hollings Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricFocusNode.C,v $
 * Revision 1.10  1994/04/12 15:29:20  hollings
 * Added samplingRate as a global set by an RPC call to control sampling
 * rates.
 *
 * Revision 1.9  1994/04/11  23:25:22  hollings
 * Added pause_time metric.
 *
 * Revision 1.8  1994/04/07  00:37:53  markc
 * Checked for NULL metric instance returned from createMetricInstance.
 *
 * Revision 1.7  1994/04/01  20:06:42  hollings
 * Added ability to start remote paradynd's
 *
 * Revision 1.6  1994/03/26  19:31:36  hollings
 * Changed sample time to be consistant.
 *
 * Revision 1.5  1994/03/24  16:41:59  hollings
 * Moved sample aggregation to lib/util (so paradyn could use it).
 *
 * Revision 1.4  1994/03/01  21:23:58  hollings
 * removed unused now variable.
 *
 * Revision 1.3  1994/02/24  04:32:34  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.2  1994/02/01  18:46:52  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:28  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.17  1994/01/20  17:47:16  hollings
 * moved getMetricValue to this file.
 *
 * Revision 1.16  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.15  1993/12/13  19:55:16  hollings
 * counter operations.
 *
 * Revision 1.14  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.13  1993/10/07  19:42:43  jcargill
 * Added true combines for global instrumentation
 *
 * Revision 1.12  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.11  1993/09/03  18:36:16  hollings
 * removed extra printfs.
 *
 * Revision 1.10  1993/09/03  16:01:56  hollings
 * removed extra printf.
 *
 * Revision 1.9  1993/09/03  15:45:51  hollings
 * eat the first sample from an aggregate to get a good start interval time.
 *
 * Revision 1.8  1993/08/20  22:00:51  hollings
 * added getMetricValue for controller.
 * moved insertInstrumentation for predicates to before pred users.
 *
 * Revision 1.7  1993/08/16  16:24:50  hollings
 * commented out elapsedPauseTime because of CM-5 problems with node processes.
 * removed many debugging printfs.
 *
 * Revision 1.6  1993/08/11  01:52:12  hollings
 * chages for new build before use mode.
 *
 * Revision 1.5  1993/07/13  18:28:30  hollings
 * new include file syntax.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "dyninstRPC.SRVR.h"

extern dynRPC *tp;
extern int metricCount;
extern int metResPairsEnabled;
extern HTable<metric> metricsUsed;
extern HTable<resourceList> fociUsed;


HTable<metricInstance> midToMiMap;

metricList globalMetricList;
extern struct _metricRec DYNINSTallMetrics[];
extern process *nodePseudoProcess;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION	1000000.0

metricDefinitionNode::metricDefinitionNode(process *p)
{
    memset(this, '\0', sizeof(metricDefinitionNode));

    proc = p;
    aggregate = FALSE;
    sample.lastSampleEnd = 0.0;
    sample.lastSampleStart = 0.0;
}

float metricDefinitionNode::getMetricValue()
{
    float total;
    List<metricDefinitionNode*> curr;

    if (aggregate) {
	total = 0.0;
	for (curr = components; *curr; curr++) {
	    total += (*curr)->getMetricValue();
	}
	return(0.0);
    }
    return((*data)->getMetricValue());
}

metricDefinitionNode::metricDefinitionNode(metric m, 
					    List<metricDefinitionNode*> parts)
{

    memset(this, '\0', sizeof(metricDefinitionNode));

    met = m;
    aggregate = TRUE;
    components = parts;
    inform = FALSE;             // change this latter.

    for (; *parts; parts++) {
	(*parts)->aggregators.add(this);
	valueList.add(&(*parts)->sample);
    }
}

metricInstance buildMetricInstRequest(resourceList l, metric m)
{
    int i;
    int tid;
    int count;
    resource r;
    int covered;
    List<process*> pl;
    metricInstance ret;
    process *proc = NULL;
    resourcePredicate *pred;
    struct _metricRec *curr;
    process **instProcessList;
    resource complexPredResource;
    resourcePredicate *complexPred;
    List<metricDefinitionNode*> parts;
    metricDefinitionNode *mn;
    AstNode *predInstance = NULL;

    if (!processList.count()) return(NULL);

    /* first find the named metric */
    for (i=0; i < metricCount; i++) {
	curr = &DYNINSTallMetrics[i];
	if (curr == m) {
	    break;
	}
    }

    if (curr != m) return(NULL);

    // 
    // check that the predicates are valid.
    //
    for (pred = curr->definition.predicates; pred->namePrefix; pred++) {
	for (i=0; i < l->count; i++) {
	    r = l->elements[i];

	    /* test predicate for this resource */
	    if (!strcmp(pred->namePrefix, r->parent->info.fullName) ||
		!strcmp(pred->namePrefix, r->info.fullName)) {
		break;
	    }
	}
	if (i== l->count) continue;

	if (!strcmp(pred->namePrefix, r->info.fullName)) {
	    // null refinement skip it .
	    continue;
	}
	if (pred->type == invalidPredicate) {
	    return(NULL);
	}
    }

    /*
     * Identify if one or all processes should be inst.
     *
     */
    tid = 0;
    for (i=0; i < l->count; i++) {
	r = l->elements[i];
	/* HACK for process for now */
	if (!strcmp("/Process", r->parent->info.fullName)) {
	    proc = (process *) r->handle;
	}
	/* HACK for machine for now */
	if (!strcmp("/Machine", r->parent->info.fullName)) {
	    tid = (int) r->handle;
	}
    }

    if (proc) {
	instProcessList = (process **) xcalloc(sizeof(process *), 2);
	if (!tid || (proc->thread == tid)) {
	    instProcessList[0] = proc;
	}
    } else if (nodePseudoProcess) {
	instProcessList = (process **) xcalloc(sizeof(process *), 3);
	instProcessList[0] = nodePseudoProcess;
	instProcessList[1] = nodePseudoProcess->parent;
    } else {
	count = processList.count();
	instProcessList = (process **) xcalloc(sizeof(process *), count+1);
	for (pl = processList,count = 0; proc = *pl; pl++) {
	    /* HACK for machine for now */
	    if (!tid || (proc->thread == tid)) {
		instProcessList[count++] = proc;
	    }
	}
    }

    /* check all proceses are in an ok state */
    if (!isApplicationPaused()) {
	return(NULL);
    }

    for (count=0; proc = instProcessList[count]; count++) {
	mn = new metricDefinitionNode(proc);

	complexPred = NULL;
	predInstance = NULL;

	/*
	 * iterate through possible predicates.
	 *   Predicates must be the outer loop so predicates are
	 *     created in the correct order.
	 *
	 */
	covered = 0;
	for (pred = curr->definition.predicates; pred->namePrefix; pred++) {
	    for (i=0; i < l->count; i++) {
		r = l->elements[i];

		/* test predicate for this resource */
		if (!strcmp(pred->namePrefix, r->parent->info.fullName) ||
		    !strcmp(pred->namePrefix, r->info.fullName)) {
		    break;
		}
	    }
	    if (i== l->count) continue;

	    covered++;

	    if (!strcmp(pred->namePrefix, r->info.fullName)) {
		// null refinement skip it .
		continue;
	    }

	    assert(pred->type != invalidPredicate);
	    if (pred->type == nullPredicate) {
		continue;
	    } else if (pred->type == simplePredicate) {
		predInstance = pred->creator(mn, r->info.name, predInstance);
	    } else {
		if (complexPred) {
		    printf("Error two complex predicates in a single metric\n");
		    abort();
		} else {
		    complexPredResource = r;
		    if (predInstance) {
			printf("Error predicate before complex in list\n");
			abort();
		    }
		    complexPred = pred;
		    predInstance = NULL;
		}
	    }
	}
	if (covered != l->count) {
	    printf("unable to find a predicate\n");
	    abort();
	}

	/*
	 * now the base metric with the built-up trigger.
	 *
	 */
	if (complexPred) {
	    /* 
	     * complex preds replace base definitions.
	     *
	     */
	    (void) complexPred->creator(mn, 
		complexPredResource->info.name, predInstance);
	} else {
	    curr->definition.baseFunc(mn, predInstance);
	}
	if (mn) {
	    mn->met = m;
	    // currMi->proc = proc;
	    parts.add(mn, mn);
	}
    }

    if (parts.count() > 1) {
	ret = new metricDefinitionNode(m, parts);
    } else {
	ret = *parts;
    }

    if (ret) {
	ret->inform = TRUE;
	ret->resList = l;
    }

    return(ret);
}


List<metricDefinitionNode*> allMIs;

/*
 * See if we can find the requested metric instance.
 *   Currently this is only used to cache structs built for cost requests 
 *   which are then instantuadted.  This could be used as a general system
 *   to request find sub-metrics that are already defined and use them to
 *   reduce cost.  This would require adding the componenets of an aggregate
 *   into the allMIs list since searching tends to be top down, not bottom
 *   up.  This would also require adding a ref count to many of the structures
 *   so they only get deleted when we are really done with them.
 *
 */

metricInstance createMetricInstance(resourceList l, metric m)
{
    static int MICount;
    metricDefinitionNode *mi;
    List<metricDefinitionNode*> curr;

    // first see if it already defined.
    for (curr = allMIs; *curr; curr++) {
    	if ((*curr)->match(l, m)) break;
    }
    if (*curr) return(*curr);

    // not found, build it.
    mi = buildMetricInstRequest(l, m);
    if (!mi) return mi;
    mi->id = ++MICount;
    if (mi) allMIs.add(mi, (void *) mi->id);
    return(mi);
}

int startCollecting(resourceList l, metric m)
{
    metricInstance mi;
    extern void printResourceList(resourceList);

    mi = createMetricInstance(l, m);
    if (!mi) return(-1);

    // cost = mi->cost();
    // printf("*** metric cost = %f\n", cost);

    mi->insertInstrumentation();
    flushPtrace();

    printf("enable of %s for RL =", getMetricName(m));
    printResourceList(l);
    printf("\n");

    // collect some stats.
    metResPairsEnabled++;
    fociUsed.addUnique(l, (void *) l);
    metricsUsed.addUnique(m, (void *) m);

    return(mi->id);
}

float guessCost(resourceList l , metric m)
{
    float cost;
    metricInstance mi;

    mi = createMetricInstance(l, m);
    if (!mi) return(0.0);

    cost = mi->cost();
    return(cost);
}

Boolean metricDefinitionNode::insertInstrumentation()
{
    List<instReqNode*> req;
    List<dataReqNode*> dp;
    List<metricDefinitionNode*> curr;

    if (inserted) return(True);

    inserted = True;
    if (aggregate) {
	for (curr = components; *curr; curr++) {
	     (*curr)->insertInstrumentation();
	}
    } else {
	for (dp = data; *dp; dp++) {
	    (*dp)->insertInstrumentation(this);
	}
	for (req = requests; *req; req++) {
	    (*req)->insertInstrumentation();
	}
    }
    return(True);
}

float metricDefinitionNode::cost()
{
    float ret;
    float nc;
    List<instReqNode*> req;
    List<metricDefinitionNode*> curr;

    ret = 0.0;
    if (aggregate) {
	for (curr = components; *curr; curr++) {
	     nc = (*curr)->cost();
	     if (nc > ret) ret = nc;
	}
    } else {
	for (req = requests; *req; req++) {
	    ret += (*req)->cost();
	}
    }
    return(ret);
}

void metricDefinitionNode::disable()
{
    List<dataReqNode*> dp;
    List<instReqNode*> req;
    metricDefinitionNode *mi;
    List<metricDefinitionNode*> curr;

    if (!inserted) return;

    inserted = False;
    if (aggregate) {
	/* disable components of aggregate metrics */
	for (curr = components; mi = *curr; curr++) {
	    mi->disable();
	}
    } else {
	for (dp = data; *dp; dp++) {
	    (*dp)->disable();
	}
	for (req = requests; *req; req++) {
	    (*req)->disable();
	}
    }
}

metricDefinitionNode::~metricDefinitionNode()
{
    List<instReqNode*> req;
    List<dataReqNode*> dp;
    metricDefinitionNode *mi;
    List<metricDefinitionNode*> curr;

    if (aggregate) {
	/* delete components of aggregate metrics */
	for (curr = components; mi = *curr; curr++) {
	    delete(mi);
	}
    } else {
	for (dp = data; *dp; dp++) {
	    delete(*dp);
	}
	for (req = requests; *req; req++) {
	    delete(*req);
	}
    }
}

void metricDefinitionNode::forwardSimpleValue(timeStamp start, timeStamp end,
				       sampleValue value)
{
    tp->sampleDataCallbackFunc(0, id, start, end, value);
}

void metricDefinitionNode::updateValue(time64 wallTime, 
				       sampleValue value)
{
    timeStamp sampleTime;
    struct sampleInterval ret;
    List<metricDefinitionNode*> curr;
    // extern timeStamp elapsedPauseTime;

    // sampleTime = (wallTime - elapsedPauseTime) / 1000000.0; 
    // commented out elapsedPauseTime because we don't currently stop CM-5
    // node processes. (brought it back jkh 11/9/93).
    sampleTime = wallTime / 1000000.0; 
    assert(value >= -0.01);

    if (met->info.style == EventCounter) {
	// only use delta from last sample.
	assert(value + 0.0001 >= sample.value);
	value -= sample.value;
	sample.value += value;
    }

    ret = sample.newValue(valueList, sampleTime, value);

    for (curr = aggregators; *curr; curr++) {
	(*curr)->updateAggregateComponent(this, sampleTime, value);
    }

    /* 
     * must do this after all updates are done, because it may turn off this
     *  metric instance.
     */
    if (inform && ret.valid) {
	/* invoke call backs */
	assert(ret.start >= 0.0);
	assert(ret.end >= 0.0);
	assert(ret.end >= ret.start);
	tp->sampleDataCallbackFunc(0, id, ret.start, ret.end, ret.value);
    }
}

void metricDefinitionNode::updateAggregateComponent(metricDefinitionNode *curr,
						    timeStamp sampleTime, 
						    sampleValue value)
{
    struct sampleInterval ret;

    ret = sample.newValue(valueList, sampleTime, value);
    if (ret.valid) {
	tp->sampleDataCallbackFunc(0, id, ret.start, ret.end, ret.value);
    }
}

void processSample(traceHeader *h, traceSample *s)
{
    metricDefinitionNode *mi;
    extern int samplesDelivered;

    mi = midToMiMap.find((void *) s->id.id);
    if (!mi) {
	// printf("sample %d not for a valid metric instance\n", s->id.id);
	return;
    }
     
    // printf("sample id %d at time %f = %f\n", s->id.id, now, s->value);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}

metricList getMetricList()
{
    if (!globalMetricList) {
	 globalMetricList = (metricList) 
	     xcalloc(sizeof(struct _metricListRec), 1);
	 globalMetricList->elements = DYNINSTallMetrics;
	 globalMetricList->count = metricCount;
    }
    return(globalMetricList);
}

metric findMetric(char *name)
{
    int i;
    metric met;
    metricList metrics;

    metrics = getMetricList();
    for (i=0, met = metrics->elements; i < metrics->count; i++, met++) {
	if (!strcmp(met->info.name, name)) {
 	    return(met);
	}
    }
    return(NULL);
}

char *getMetricName(metric m)
{
    return(m->info.name);
}

metricInfo *getMetricInfo(metric m)
{
    return(&m->info);
}

metric getMetric(metricInstance mi)
{
    return(mi->met);
}

/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(process *iProc,
                         instPoint *iPoint,
                         AstNode *iAst,
                         callWhen  iWhen,
			 callOrder o) 
{
    proc = iProc;
    point = iPoint;
    ast = iAst;
    when = iWhen;
    order = o;
}

Boolean instReqNode::insertInstrumentation()
{
    instance = addInstFunc(proc, point, ast, when, order);
    if (instance) return(True);

    return(False);
}

void instReqNode::disable()
{
    deleteInst(instance);
    instance = NULL;
}

instReqNode::~instReqNode()
{
    instance = NULL;
}

float instReqNode::cost()
{
    float value;
    float unitCost;
    float frequency;

    unitCost = ast->cost();
    frequency = getPointFrequency(point);
    value = unitCost * frequency;

    return(value);
}

dataReqNode::dataReqNode(dataObjectType iType,
                      process *iProc,
                      int iInitialValue,
                      Boolean iReport,
                      timerType iTType) 
{
    type = iType;
    proc = iProc;
    initialValue = iInitialValue;
    report = iReport;
    tType = iTType;
    instance = NULL;
};

float dataReqNode::getMetricValue()
{
    float ret;

    if (type == intCounter) {
	ret = getIntCounterValue((intCounterHandle*) instance);
    } else if (type == timer) {
	ret = getTimerValue((timerHandle*) instance);
    } else {
	// unknown type.
	abort();
	return(0.0);
    }
    return(ret);
}

void *dataReqNode::getInferriorPtr() 
{
    void *param;
    timerHandle *timerInst;
    intCounterHandle *counterInst;

    if (type == intCounter) {
	counterInst = (intCounterHandle *) instance;
	if (counterInst) {
	    param = (void *) counterInst->counterPtr;
	} else {
	    param = NULL;
	}
    } else if (type == timer) {
	timerInst = (timerHandle *) instance;
	if (timerInst) {
	    param = (void *) timerInst->timerPtr;
	} else {
	    param = NULL;
	}
    } else {
	abort();
    }
    return(param);
}

timerHandle *dataReqNode::createTimerInstance()
{
    timerHandle *ret;

    ret = createTimer(proc, tType, report);
    return(ret);
}

intCounterHandle *dataReqNode::createCounterInstance()
{
    intCounterHandle *ret;

    ret = createIntCounter(proc, initialValue, report);
    return(ret);
}

void dataReqNode::insertInstrumentation(metricDefinitionNode *mi) 
{
    if (type == intCounter) {
	intCounterHandle *ret;
	ret = createCounterInstance();
	instance = (void *) ret;
	id = ret->data.id;
	midToMiMap.add(mi, (void *) id.id);
    } else {
	timerHandle *ret;
	ret = createTimerInstance();
	instance = (void *) ret;
	id = ret->data.id;
	midToMiMap.add(mi, (void *) id.id);
    }
}

void dataReqNode::disable()
{
    Boolean found;
    metricInstance mi;

    if (!instance) return;

    found = midToMiMap.remove((void *) id.id);
    assert(found == True);

    /* make sure it is gone =-- debugging code */
    mi = midToMiMap.find((void *) id.id);
    assert(mi == NULL);

    if (type == timer) {
	freeTimer((timerHandle *) instance);
    } else if (type == intCounter) {
	freeIntCounter((intCounterHandle *) instance);
    } else {
	abort();
    }
    instance = NULL;
}

dataReqNode::~dataReqNode()
{
    instance = NULL;
}

// used in other modules.
metricDefinitionNode *pauseTimeNode;

void computePauseTimeMetric()
{
    timeStamp now;
    timeStamp start;
    timeStamp elapsed;
    struct timeval tv;
    static timeStamp end;
    extern float samplingRate;
    extern timeStamp startPause;
    extern time64 firstRecordTime;
    extern Boolean applicationPaused;
    extern timeStamp elapsedPauseTime;
    static timeStamp reportedPauseTime;

    if (pauseTimeNode && firstRecordTime) {
	gettimeofday(&tv, NULL);
	now = (tv.tv_sec * MILLION + tv.tv_usec - firstRecordTime)/MILLION;
	if (now < end + samplingRate) 
	    return;

	start = end;
	end = now;
	elapsed = elapsedPauseTime - reportedPauseTime;
	if (applicationPaused) {
	    elapsed += tv.tv_sec * MILLION + tv.tv_usec - startPause;
	}
	reportedPauseTime += elapsed;
	pauseTimeNode->forwardSimpleValue(start, end, elapsed/MILLION);
    }
}

