/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/metricFocusNode.C,v 1.34 1994/08/02 18:22:55 hollings Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricFocusNode.C,v $
 * Revision 1.34  1994/08/02 18:22:55  hollings
 * Changed comparison for samples going backwards to use a ratio rather than
 * an absolute fudge factor.  This is required due to floating point rounding
 * errors on large numbers.
 *
 * Revision 1.33  1994/07/28  22:40:42  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.32  1994/07/26  19:58:35  hollings
 * added CMMDhostless variable.
 *
 * Revision 1.31  1994/07/22  19:20:12  hollings
 * moved computePauseTimeMetric to machine specific area.
 *
 * Revision 1.30  1994/07/21  01:34:19  hollings
 * Fixed to skip over null point and ast nodes for addInst calls.
 *
 * Revision 1.29  1994/07/20  18:21:55  rbi
 * Removed annoying printf
 *
 * Revision 1.28  1994/07/16  03:38:48  hollings
 * fixed stats to not devidi by 1meg, fixed negative time problem.
 *
 * Revision 1.27  1994/07/15  20:22:05  hollings
 * fixed 64 bit record to be 32 bits.
 *
 * Revision 1.26  1994/07/14  23:30:29  hollings
 * Hybrid cost model added.
 *
 * Revision 1.25  1994/07/14  14:39:17  jcargill
 * Removed call to flushPtrace
 *
 * Revision 1.24  1994/07/12  20:13:58  jcargill
 * Fixed logLine for printing out samples w/64 bit time
 *
 * Revision 1.23  1994/07/05  03:26:09  hollings
 * observed cost model
 *
 * Revision 1.22  1994/07/02  01:46:41  markc
 * Use aggregation operator defines from util/h/aggregation.h
 * Changed average aggregations to summations.
 *
 * Revision 1.21  1994/06/29  02:52:36  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.20  1994/06/27  18:56:56  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.19  1994/06/22  01:43:16  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed 
 * process.C reference to proc->status to use proc->heap->status.
 *
 * Revision 1.18  1994/06/02  23:27:57  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.17  1994/05/31  19:53:50  markc
 * Fixed pause time bug which was causing negative values to be reported.  The
 * fix involved adding an extra test in computePauseTimeMetric that did not
 * begin reporting pause times until firstSampleReceived is TRUE.
 *
 * Revision 1.16  1994/05/31  19:16:17  markc
 * Commented out assert test for elapsed.
 *
 * Revision 1.15  1994/05/31  18:14:18  markc
 * Modified check for covered less than rather than not equal.  This is a short
 * term fix.
 *
 * Revision 1.14  1994/05/03  05:07:24  markc
 * Removed comment on pauseMetric for paradyndPVM.
 *
 * Revision 1.13  1994/04/15  15:37:34  jcargill
 * Removed duplicate definition of pauseTimeNode; used initialized version
 *
 * Revision 1.12  1994/04/13  16:48:10  hollings
 * fixed pause_time to work with multiple processes/node.
 *
 * Revision 1.11  1994/04/13  03:09:00  markc
 * Turned off pause_metric reporting for paradyndPVM because the metricDefNode is
 * not setup properly.  Updated inst-pvm.C and metricDefs-pvm.C to reflect changes
 * in cm5 versions.
 *
 * Revision 1.10  1994/04/12  15:29:20  hollings
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
#include "comm.h"
#include "internalMetrics.h"


extern "C" {
// <sys/time.h> should define this
int gettimeofday(struct timeval *tp, struct timezone *tzp);
}

extern pdRPC *tp;
extern int metricCount;
extern int metResPairsEnabled;
extern HTable<metric> metricsUsed;
extern HTable<resourceList> fociUsed;

HTable<metricInstance> midToMiMap;

double currentHybridValue;
metricList globalMetricList;
extern process *nodePseudoProcess;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION	1000000.0

metricDefinitionNode::metricDefinitionNode(process *p, int agg_style)
{
    memset(this, '\0', sizeof(metricDefinitionNode));

    proc = p;
    aggregate = FALSE;
    sample.lastSampleEnd = 0.0;
    sample.lastSampleStart = 0.0;
    sample.aggOp = agg_style;            // set aggregation style
                                         // aggSum, ...

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
    sample.aggOp = m->info.aggregate;     // set aggregation style
                                          // aggSum, ...

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
    internalMetric *im;
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
    for (i=0; i < globalMetricList->count; i++) {
	curr = &globalMetricList->elements[i];
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
	    if (!strncmp(pred->namePrefix, r->parent->info.fullName, 
			 strlen(pred->namePrefix)) ||
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

    /* check for "special" metrics that are computed directly by paradynd */
    im = internalMetric::allInternalMetrics.find(m->info.name);
    if (im) {
	mn = new metricDefinitionNode(*pl, m->info.aggregate);
	im->enable(mn);
	sprintf(errorLine, "enabled internal metric %s\n", (m->info.name));
	logLine(errorLine);
	return(mn);
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
	instProcessList = (process **) xcalloc(2, sizeof(process *));
	if (!tid || (proc->thread == tid)) {
	    instProcessList[0] = proc;
	}
    } else if (nodePseudoProcess) {
	instProcessList = (process **) xcalloc(3, sizeof(process *));
	instProcessList[0] = nodePseudoProcess;
	instProcessList[1] = nodePseudoProcess->parent;
    } else {
	count = processList.count();
	instProcessList = (process **) xcalloc(count+1, sizeof(process *));
	for (pl = processList,count = 0; proc = *pl; pl++) {
	    /* HACK for machine for now */
	    if (!tid || (proc->thread == tid)) {
		instProcessList[count++] = proc;
	    }
	}
    }

    for (count=0; proc = instProcessList[count]; count++) {
	mn = new metricDefinitionNode(proc, m->info.aggregate);

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
		if (!strncmp(pred->namePrefix, r->parent->info.fullName, 
			     strlen(pred->namePrefix)) ||
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
		int i;
		char *constraint, *temp;

		// make constraint the part after the match in namePrefix
		//   also skip next /.
		temp = constraint = strdup(r->info.fullName);
		for (i=0; i <= strlen(pred->namePrefix); i++) constraint++;
		predInstance = pred->creator(mn, constraint, predInstance);
		free(temp);
	    } else {
		if (complexPred) {
		    logLine("Error two complex predicates in a single metric\n");
		    abort();
		} else {
		    complexPredResource = r;
		    if (predInstance) {
			logLine("Error predicate before complex in list\n");
			abort();
		    }
		    complexPred = pred;
		    predInstance = NULL;
		}
	    }
	}
	if (covered < l->count) {
	    logLine("unable to find a predicate\n");
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
	    int i;
	    char *constraint, *temp;

	    // make constraint the part after the match in namePrefix
	    //   also skip next /.
	    temp = constraint = strdup(complexPredResource->info.fullName);
	    for (i=0; i <= strlen(complexPred->namePrefix); i++) constraint++;

	    (void) complexPred->creator(mn, constraint, predInstance);
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
    float cost;
    metricInstance mi;
    extern Boolean CMMDhostless;
    extern double currentPredictedCost;
    extern void printResourceList(resourceList);

    if (CMMDhostless == TRUE) return(-1);

    mi = createMetricInstance(l, m);
    if (!mi) return(-1);

    cost = mi->cost();
    mi->originalCost = cost;

    currentPredictedCost += cost;
    // sprintf(errorLine, "*** metric cost = %f\n", cost);
    // logLine(errorLine);

    mi->insertInstrumentation();

    // sprintf(errorLine, "enable of %s for RL =", getMetricName(m));
    // logLine(errorLine);
    // printResourceList(l);
    // logLine("\n");

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
    Boolean needToCont = FALSE;
    List<metricDefinitionNode*> curr;

    if (inserted) return(True);

    /* check all proceses are in an ok state */
    if (!isApplicationPaused()) {
	pauseAllProcesses();
	needToCont = TRUE;
	return(NULL);
    }

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

    if (needToCont) continueAllProcesses();
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
    internalMetric *im;
    List<dataReqNode*> dp;
    List<instReqNode*> req;
    metricDefinitionNode *mi;
    List<metricDefinitionNode*> curr;

    // check for internal metrics
    im = internalMetric::activeInternalMetrics.find(this);
    if (im) {
	im->disable();
	logLine("disabled internal metric\n");
	return;
    }

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

    // sampleTime = wallTime/ 1000000.0 - elapsedPauseTime;
    // commented out elapsedPauseTime because we don't currently stop CM-5
    // node processes. (brought it back jkh 11/9/93).
    sampleTime = wallTime / 1000000.0; 
    assert(value >= -0.01);

    if (met->info.style == EventCounter) {
	// only use delta from last sample.
	if (value < sample.value) {
	    if ((value/sample.value) < 0.99999) {
		assert(value + 0.0001 >= sample.value);
	    } else {
		// floating point rounding error ignore
		sample.value = value;
	    }
	}
//	if (value + 0.0001 < sample.value)
//           printf ("WARNING:  sample went backwards!!!!!\n");
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
	// assert(ret.start >= 0.0);
	// I have no idea where negative time comes from but leave it to
	// the CM-5 to create it on the first sample -- jkh 7/15/94
	if (ret.start < 0.0) ret.start = 0.0;
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

void processCost(process *proc, traceHeader *h, costUpdate *s)
{
    int i;
    process *p;
    List<process *> curr;
    extern double currentPredictedCost;
    extern internalMetric totalPredictedCost;
    extern internalMetric hybridPredictedCost;

    if (proc->cost.wallTimeLastTrampSample) {
	proc->pauseTime = s->pauseTime/ 
	    ((h->wall - proc->cost.wallTimeLastTrampSample)/1000000.0);
    }
    proc->cost.wallTimeLastTrampSample = h->wall;

    // should really compute this on a per process basis.
    proc->cost.currentPredictedCost = currentPredictedCost;

    // update totalPredicted cost.
    proc->cost.totalPredictedCost += proc->cost.currentPredictedCost *
	    (h->process - proc->cost.timeLastTrampSample)/1000000.0;
    proc->cost.timeLastTrampSample = h->process;

    //
    // build circular buffer of recent values.
    //
    proc->cost.past[proc->cost.currentHist] = 
	(s->observedCost - proc->cost.lastObservedCost);
    if (++proc->cost.currentHist == HIST_LIMIT) proc->cost.currentHist = 0;

    // now compute current value of hybrid;
    for (i=0, proc->cost.hybrid = 0.0; i < HIST_LIMIT; i++) {
	proc->cost.hybrid += proc->cost.past[i];
    }
    proc->cost.hybrid /= HIST_LIMIT;

    proc->cost.lastObservedCost = s->observedCost;

    currentHybridValue = 0.0;
    for (curr = processList; p = *curr; curr++) {
	if (p->cost.hybrid > currentHybridValue) {
	    currentHybridValue = p->cost.hybrid;
	}
    }
    hybridPredictedCost.value = currentHybridValue;

    //
    // update total predicted cost.
    //
    totalPredictedCost.value = 0.0;
    for (curr = processList; p = *curr; curr++) {
	if (p->cost.totalPredictedCost > totalPredictedCost.value) {
	    totalPredictedCost.value = p->cost.totalPredictedCost;
	}
    }

    return;
}

void processSample(traceHeader *h, traceSample *s)
{
    metricDefinitionNode *mi;
    extern int samplesDelivered;

    mi = midToMiMap.find((void *) s->id.id);
    if (!mi) {
	// logLine("sample %d not for a valid metric instance\n", s->id.id);
	return;
    }
     
//    sprintf(errorLine, "sample id %d at time 0x%x%x = %f\n", s->id.id, 
//	*(int*) &h->wall, *(((int*) &h->wall)+1), s->value);
//    logLine(errorLine);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}

metricList getMetricList()
{
    int i;
    List<internalMetric*> curr;
    extern struct _metricRec DYNINSTallMetrics[];

    if (!globalMetricList) {
	 // merge internal and external metrics into one list.
	 globalMetricList = (metricList) 
	     xcalloc(1, sizeof(struct _metricListRec));
	 globalMetricList->count = 
	     metricCount + internalMetric::allInternalMetrics.count();
	 globalMetricList->elements = (struct _metricRec *)
	     xcalloc(globalMetricList->count, sizeof(struct _metricRec));
	 for (i=0; i < metricCount; i++) {
	     globalMetricList->elements[i] = DYNINSTallMetrics[i];
	 }
	 for (curr = internalMetric::allInternalMetrics; *curr; curr++) {
	     globalMetricList->elements[i++] = (*curr)->metRec;
	 }
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

    assert(proc && point);
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

// 66Mhz clock for ss10/40.
#define CYCLES_TO_SEC	66.0/1000000.0

float instReqNode::cost()
{
    float value;
    float unitCost;
    float frequency;

    unitCost = ast->cost() / CYCLES_TO_SEC;
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

caddr_t dataReqNode::getInferriorPtr() 
{
    caddr_t param;
    timerHandle *timerInst;
    intCounterHandle *counterInst;

    if (type == intCounter) {
	counterInst = (intCounterHandle *) instance;
	if (counterInst) {
	    param = (caddr_t) counterInst->counterPtr;
	} else {
	    param = (caddr_t) NULL;
	}
    } else if (type == timer) {
	timerInst = (timerHandle *) instance;
	if (timerInst) {
	    param = (caddr_t) timerInst->timerPtr;
	} else {
	    param = (caddr_t) NULL;
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


extern float computePauseTimeMetric();

resourcePredicate defaultInternalPreds[] = {
  { "/SyncObject", invalidPredicate, (createPredicateFunc) NULL },
  { "/Machine", nullPredicate, (createPredicateFunc) NULL },
  { "/Process", invalidPredicate, (createPredicateFunc) NULL },
  { "/Procedure", invalidPredicate, (createPredicateFunc) NULL },
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

internalMetric::internalMetric(char *n, 
			       int style, 
			       int a, 
		               char *units, 
  			       sampleValueFunc f) 
{
    allInternalMetrics.add(this, n);
    value = 0.0;
    cumlativeValue = 0.0;
    metRec.info.name = n;
    metRec.info.style = style;
    metRec.info.aggregate = a;
    metRec.info.units = units;
    metRec.definition.baseFunc = NULL;
    metRec.definition.predicates = defaultInternalPreds;
    func = f;
}

List<internalMetric*>internalMetric::allInternalMetrics;
List<internalMetric*>internalMetric::activeInternalMetrics;

double currentPredictedCost;

internalMetric pauseTime("pause_time", 
			 SampledFunction, 
			 aggMax, 
			 "% Time",
			 computePauseTimeMetric);

internalMetric totalPredictedCost("predicted_cost", 
				  EventCounter,
				  aggMax,
				  "Wasted CPUs",
				  NULL);


internalMetric hybridPredictedCost("hybrid_cost", 
				  SampledFunction,
				  aggMax,
				  "Wasted CPUs",
				  NULL);

internalMetric activeSlots("active_slots", 
			    SampledFunction,
			    aggSum,
			    "NUmber",
			    NULL);

timeStamp getCurrentTime(Boolean firstRecordRelative)
{
    time64 now;
    timeStamp ret;
    struct timeval tv;
    extern time64 firstRecordTime;

    gettimeofday(&tv, NULL);
    now = (time64) (tv.tv_sec * MILLION) + tv.tv_usec;
    if (firstRecordRelative) now -= firstRecordTime;
    
    ret = now/MILLION;

    return(ret);
}

void reportInternalMetrics()
{
    timeStamp now;
    timeStamp start;
    sampleValue value;
    internalMetric *im;
    static timeStamp end;
    extern float samplingRate;
    List<internalMetric*> curr;
    extern time64 firstRecordTime;


    // see if we have a sample to establish time base.
    if (!firstRecordTime) return;

    now = getCurrentTime(TRUE);

    //  check if it is time for a sample
    if (now < end + samplingRate) 
	return;

    start = end;
    end = now;
    for (curr = internalMetric::allInternalMetrics; im = *curr; curr++) {
	if (im->enabled()) {
	    value = im->getValue();
	    if (im->metRec.info.style == EventCounter) {
		assert(value + 0.0001 >= im->cumlativeValue);
		value -= im->cumlativeValue;
		im->cumlativeValue += value;
	    }
	    im->node->forwardSimpleValue(start, end, value);
	}
    }
}
