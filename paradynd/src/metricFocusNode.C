/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/metricFocusNode.C,v 1.45 1994/11/11 05:11:06 markc Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricFocusNode.C,v $
 * Revision 1.45  1994/11/11 05:11:06  markc
 * Turned off print message when internal metrics are enbled.
 *
 * Revision 1.44  1994/11/10  21:03:42  markc
 * metricValue gets intialized to 0.
 *
 * Revision 1.43  1994/11/10  18:58:06  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.42  1994/11/09  18:40:14  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.41  1994/11/02  11:10:59  markc
 * Attempted to clean up metric instrumentation requests with classes.
 * Removed string handles.
 *
 * Revision 1.40  1994/09/30  19:47:09  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.39  1994/09/22  02:13:35  markc
 * cast args to memset
 * cast stringHandles for string functions
 * change *allocs to news
 *
 * Revision 1.38  1994/09/20  18:18:28  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.37  1994/09/05  20:33:34  jcargill
 * Bug fix:  enabling certain metrics could cause no instrumentation to be
 * inserted, but still return a mid; this hosed the PC
 *
 * Revision 1.36  1994/08/17  16:43:32  markc
 * Removed early return from metricDefinitionNode::insertInstrumentation which
 * prevented instrumentation from being inserted if the application was
 * running when the request was made.
 *
 * Revision 1.35  1994/08/08  20:13:43  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.34  1994/08/02  18:22:55  hollings
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
 * begin reporting pause times until firstSampleReceived is true.
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

#include "util/h/kludges.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
}

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
#include <strstream.h>
#include "util/h/list.h"
#include "init.h"
#include "perfStream.h"
#include "main.h"
#include "stats.h"
#include "dynrpc.h"

double currentPredictedCost = 0.0;
double currentHybridValue= 0.0;

dictionary_hash <unsigned, metricDefinitionNode*> midToMiMap(uiHash);
metricListRec *globalMetricList= NULL;

unsigned mdnHash(const metricDefinitionNode *&mdn) {
  return ((unsigned) mdn);
}

dictionary_hash<unsigned, metricDefinitionNode*> allMIs(uiHash);
dictionary_hash<string, internalMetric*> internalMetric::allInternalMetrics(string::hash);
dictionary_hash<metricDefinitionNode*, internalMetric*>
internalMetric::activeInternalMetrics(mdnHash);

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION	1000000.0

metricDefinitionNode::metricDefinitionNode(process *p, int agg_style) : sample(agg_style)
{
    met = NULL;
    originalCost = 0.0;
    resList = NULL;
    id = -1;
    proc = p;
    inform=false;
    aggregate = false;
    inserted=false;
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

metricDefinitionNode::metricDefinitionNode(metric *m, 
					   List<metricDefinitionNode*> parts) :
					   met(m), components(parts),
					   sample(m->info.aggregate)
{
    originalCost = 0.0;
    resList = NULL;
    id = -1;
    proc = NULL;
    inform=false;
    inserted=false;
    aggregate = true;

    for (; *parts; parts++) {
	(*parts)->aggregators.add(this);
	valueList.add(&(*parts)->sample);
    }
}

float getProcessCount() {
  return ((float) processMap.size());
}

metricDefinitionNode *buildMetricInstRequest(resourceListRec *l, metric *m, bool enable=true)
{
    int i;
    int tid;
    int count;
    resource *r=NULL;
    int covered;

    metricDefinitionNode *ret;
    internalMetric *im = NULL;
    process *proc = NULL;
    resourcePredicate *pred;
    metric *curr = NULL;
    resource *complexPredResource = NULL;
    resourcePredicate *complexPred = NULL;
    List<metricDefinitionNode*> parts;
    metricDefinitionNode *mn = NULL;
    AstNode *predInstance = NULL;

    if (!processMap.size()) return(NULL);

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
    // TODO - use pred->more to signal NULL for now
    for (pred = curr->definition.predicates; pred->more; pred++) {
	for (i=0; i < l->count; i++) {
	    r = l->elements[i];

	    if (r->suppressed) {
		// should not collect info about this.
		return(NULL);
	    }

	    /* test predicate for this resource */
	    if (r->parent->info.fullName.prefixed_by(pred->namePrefix) ||
		(pred->namePrefix == r->info.fullName))
	      break;
	  }
	if (i== l->count) continue;

	// null refinement skip it .
    	if (pred->namePrefix == r->info.fullName)
	    continue;

	if (pred->type == invalidPredicate) {
	    return(NULL);
	}
    }

    /* check for "special" metrics that are computed directly by paradynd */
    if (internalMetric::allInternalMetrics.defines(m->info.name)) {
      // if a cost of an internal metric is asked for, enable=false
      if (!enable) return NULL;
      im = internalMetric::allInternalMetrics[m->info.name];
      // TODO why was *pl being used here, when it was always NULL
      mn = new metricDefinitionNode(NULL, m->info.aggregate);
      im->enable(mn);
      ostrstream os(errorLine, 1024, ios::out);
      // os <<  "enabled internal metric " << m->info.name << "\n";
      // logLine(errorLine);
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
	if (r->parent->info.fullName == "/Process") {
	  proc = (process *) r->handle;
	}
	/* HACK for machine for now */
	if (r->parent->info.fullName == "/Machine") {
	  tid = (int) r->handle;
	}
    }

    vector<process*> instProcessList;

    if (proc) {
	if (!tid || (proc->thread == tid))
	    instProcessList += proc;
    } else if (nodePseudoProcess) {
	instProcessList += nodePseudoProcess;
	// TODO - jon nodePseudoProcess->parent is never set.
	if (nodePseudoProcess->parent)
	  instProcessList += nodePseudoProcess->parent;
    } else {
        int i; process *proc;
	dictionary_hash_iter<int, process*> pi(processMap);
	while (pi.next(i, proc)) {
	  /* HACK for machine for now */
	  if (!tid || (proc->thread == tid)) {
	    instProcessList += proc;
	  }
	}
    }

    for (count=0; count < instProcessList.size(); count++) {
        proc = instProcessList[count];
	assert(proc);
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
	// TODO this is a kludgey way to iterate
	for (pred = curr->definition.predicates; pred->more; pred++) {
	    for (i=0; i < l->count; i++) {
		r = l->elements[i];

		/* test predicate for this resource */
		if (r->parent->info.fullName.prefixed_by(pred->namePrefix) ||
		    pred->namePrefix == r->info.fullName) {
		  break;
		}
	    }
	    if (i== l->count) continue;
	    ostrstream os, os1;
	    covered++;
	    if (pred->namePrefix == r->info.fullName)
	      continue;           // null refinement skip it .

	    switch (pred->type) {
	    case invalidPredicate:
	      assert(0);  break;

	    case nullPredicate:
	      break; 
	      
	    case simplePredicate:
	      int i; char *constraint, *temp, *t2;

	      // make constraint the part after the match in namePrefix
	      //   also skip next /.

	      os << r->info.fullName << ends; os1 << pred->namePrefix << ends;
	      constraint = temp = os.str(); t2 = os1.str(); 
	      int slen = strlen(t2);
	      delete t2;
	      
	      // TODO why not just increment in one bunch ?
	      for (i=0; i <= slen; i++) constraint++;
	      predInstance = pred->creator(mn, constraint, predInstance);
	      assert(predInstance);
	      free(temp);
	      break;

	    case replaceBase:
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
	      break;
	    default:
	      assert(0);
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
	    int i; char *constraint, *temp; ostrstream os, os1;
	    // make constraint the part after the match in namePrefix
	    //   also skip next /.
	    os << complexPredResource->info.fullName << ends;
	    os1 << complexPred->namePrefix << ends;
	    temp = constraint = os.str();
	    char *t2 = os1.str();
	    int slen = strlen(t2);
	    // TODO why not increment at once ?
	    for (i=0; i <= slen; i++) constraint++;
	    assert(complexPred->type == replaceBase);
	    // TODO - this is replacing a metric, and better return void, mdc
	    // It is not returning a counter to evaluate a conditional
	    assert(!complexPred->creator(mn, constraint, predInstance));
	} else {
	  metricDefinition di;
	  di = curr->definition;
	    curr->definition.baseFunc(mn, predInstance);
	}
	if (mn && mn->nonNull()) {
	    mn->met = m;
	    // currMi->proc = proc;
	    parts.add(mn, mn);
	}

	else {
	  delete mn;
	  mn = NULL;
	}
    }

    if (parts.count() > 1) {
	ret = new metricDefinitionNode(m, parts);
    } else {
	ret = *parts;
    }

    if (ret) {
	ret->inform = true;
	ret->resList = l;
    }

    return(ret);
}



/*
 * See if we can find the requested metric instance.
 *   Currently this is only used to cache structs built for cost requests 
 *   which are then instantuadted.  This could be used as a general system
 *   to request find sub-metrics that are already.defines and use them to
 *   reduce cost.  This would require adding the componenets of an aggregate
 *   into the allMIs list since searching tends to be top down, not bottom
 *   up.  This would also require adding a ref count to many of the structures
 *   so they only get deleted when we are really done with them.
 *
 */

metricDefinitionNode *createMetricInstance(resourceListRec *l, metric *m, bool enable=true)
{
    static int MICount=0;
    metricDefinitionNode *mi= NULL;

    // first see if it already.defines.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi(allMIs);
    unsigned u;
    while (mdi.next(u, mi)) {
      if (mi->match(l, m))
	return mi;
    }

    // not found, build it.
    mi = buildMetricInstRequest(l, m, enable);
    if (!mi) return mi;
    mi->id = ++MICount;
    if (mi)
      allMIs[mi->id] = mi;
    return(mi);
}

int startCollecting(resourceListRec *l, metric *m)
{
    float cost;
    metricDefinitionNode *mi;

    if (CMMDhostless == true) return(-1);

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
    // commented out since these were never accessed
    // fociUsed.addUnique(l, (void *) l);
    // metricsUsed.addUnique(m, (void *) m);

    return(mi->id);
}

float guessCost(resourceListRec *l , metric *m)
{
    float cost;
    metricDefinitionNode *mi;

    mi = createMetricInstance(l, m, false);
    if (!mi) return(0.0);

    cost = mi->cost();
    return(cost);
}

bool metricDefinitionNode::insertInstrumentation()
{
    List<instReqNode*> req;
    List<dataReqNode*> dp;
    bool needToCont = false;
    List<metricDefinitionNode*> curr;

    if (inserted) return(true);

    /* check all proceses are in an ok state */

    if (!isApplicationPaused()) {
	pauseAllProcesses();
	needToCont = true;
    }

    inserted = true;
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
    // TODO mdc
    struct timeval tv;
    timeStamp now;
    gettimeofday(&tv, NULL);
    now = ((tv.tv_sec*MILLION) + tv.tv_usec - firstRecordTime)/MILLION;
//    sprintf(errorLine, "metric %d installed at %f, aggregate=%d\n", id, now, aggregate);
//    logLine(errorLine);

    if (needToCont) continueAllProcesses();

    return(true);
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

    // check for internal metrics

    const metricDefinitionNode *mdn = this;
    if (internalMetric::activeInternalMetrics.defines(mdn)) {
      internalMetric *imp = internalMetric::activeInternalMetrics[mdn];
      imp->disable();
      logLine("disabled internal metric\n");
      return;
    }

    if (!inserted) return;

    inserted = false;
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
  // TODO mdc
    assert(start >= (firstRecordTime/MILLION));
    assert(end >= (firstRecordTime/MILLION));
    assert(end > start);
    tp->sampleDataCallbackFunc(0, id, start, end, value);
}

void metricDefinitionNode::updateValue(time64 wallTime, 
				       sampleValue value)
{
    timeStamp sampleTime;
    sampleInterval ret;
    List<metricDefinitionNode*> curr;
    // extern timeStamp elapsedPauseTime;

    // sampleTime = wallTime/ 1000000.0 - elapsedPauseTime;
    // commented out elapsedPauseTime because we don't currently stop CM-5
    // node processes. (brought it back jkh 11/9/93).
    sampleTime = wallTime / 1000000.0; 
    assert(value >= -0.01);

// TODO mdc
//    if (!sample.firstSampleReceived) {
//      sprintf(errorLine, "First for %s:%d at %f\n", met->info.name.string_of(), id,
//	      sampleTime-(firstRecordTime/MILLION));
//      logLine(errorLine);
//    }

    if (met->info.style == EventCounter) { 
	if (met->reallyIsEventCounter) {
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
      } else {
	// spoof the event counter, since the sampled function data is getting
	// corrupted in paradyn
	// don't have to do anything here, except pass the data forward
	// we wan't the outside world to think that this is an event counter
      }
    }

    ret = sample.newValue(valueList, sampleTime, value);
//    if (!ret.valid && inform) {
//       sprintf(errorLine, "Invalid for %s:%d at %f, val=%f, inform=%d\n",
//	      met->info.name.string_of(), id, sampleTime-(firstRecordTime/MILLION),
//	      value, inform);
//      logLine(errorLine);
//    }

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

	// TODO mdc
//	assert(ret.start >= (firstRecordTime/ MILLION));
//	assert(ret.end >= (firstRecordTime/MILLION));
	tp->sampleDataCallbackFunc(0, id, ret.start, ret.end, ret.value);
    }
}

void metricDefinitionNode::updateAggregateComponent(metricDefinitionNode *curr,
						    timeStamp sampleTime, 
						    sampleValue value)
{
    sampleInterval ret;

    ret = sample.newValue(valueList, sampleTime, value);
    if (ret.valid) {
        assert(ret.end > ret.start);
	assert(ret.start >= (firstRecordTime/MILLION));
	assert(ret.end >= (firstRecordTime/MILLION));
	tp->sampleDataCallbackFunc(0, id, ret.start, ret.end, ret.value);
    }
}

void processCost(process *proc, traceHeader *h, costUpdate *s)
{
    int i;
    process *p;

    if (proc->theCost.wallTimeLastTrampSample) {
	proc->pauseTime = s->pauseTime/ 
	    ((h->wall - proc->theCost.wallTimeLastTrampSample)/1000000.0);
    }
    proc->theCost.wallTimeLastTrampSample = h->wall;

    // should really compute this on a per process basis.
    proc->theCost.currentPredictedCost = currentPredictedCost;

    // update totalPredicted cost.
    proc->theCost.totalPredictedCost += proc->theCost.currentPredictedCost *
	    (h->process - proc->theCost.timeLastTrampSample)/1000000.0;
    proc->theCost.timeLastTrampSample = h->process;

    //
    // build circular buffer of recent values.
    //
    proc->theCost.past[proc->theCost.currentHist] = 
	(s->observedCost - proc->theCost.lastObservedCost);
    if (++proc->theCost.currentHist == HIST_LIMIT) proc->theCost.currentHist = 0;

    // now compute current value of hybrid;
    for (i=0, proc->theCost.hybrid = 0.0; i < HIST_LIMIT; i++) {
	proc->theCost.hybrid += proc->theCost.past[i];
    }
    proc->theCost.hybrid /= HIST_LIMIT;

    proc->theCost.lastObservedCost = s->observedCost;

    currentHybridValue = 0.0;
    dictionary_hash_iter<int, process*> pi(processMap);

    while (pi.next(i, p)) {
      if (p->theCost.hybrid > currentHybridValue) {
	currentHybridValue = p->theCost.hybrid;
      }
    }
    hybridPredictedCost->value = currentHybridValue;

    //
    // update total predicted cost.
    //
    totalPredictedCost->value = 0.0;
    pi.reset();
    while (pi.next(i, p)) {
      if (p->theCost.totalPredictedCost > totalPredictedCost->value) {
	totalPredictedCost->value = p->theCost.totalPredictedCost;
      }
    }

    return;
}

void processSample(traceHeader *h, traceSample *s)
{
    metricDefinitionNode *mi;

    char errorLine[255];

    if (!midToMiMap.defines(s->id.id)) {
      sprintf(errorLine, "sample %d not for a valid metric instance\n", 
	      s->id.id);
      logLine(errorLine);
      return;
    }
    mi = midToMiMap[s->id.id];

    //    sprintf(errorLine, "sample id %d at time %8.6f = %f\n", s->id.id, 
    //	((double) *(int*) &h->wall) + (*(((int*) &h->wall)+1))/1000000.0, s->value);
    //    logLine(errorLine);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}

metricListRec *getMetricList()
{
    int i;

    if (!globalMetricList) {
	 // merge internal and external metrics into one list.
	 globalMetricList =  new metricListRec;
	 globalMetricList->count = 
	     metricCount + internalMetric::allInternalMetrics.size();
	 globalMetricList->elements = new metric[globalMetricList->count];
	 for (i=0; i < metricCount; i++) {
	     globalMetricList->elements[i] = DYNINSTallMetrics[i];
	 }
	 dictionary_hash_iter<string, internalMetric*> imi(internalMetric::allInternalMetrics);
	 string pds; internalMetric *im;
	 i = metricCount;
	 while (imi.next(pds, im)) {
	   globalMetricList->elements[i] = im->metRec;
	   i++;
	 }
    }
    return(globalMetricList);
}

metric *findMetric(const string name)
{
  int i;
  metric *met;
  metricListRec *metrics;

  metrics = getMetricList();
  for (i=0, met = metrics->elements; i < metrics->count; i++, met++) {
    if (name == met->info.name)
      return(met);
  }
  return(NULL);
}

string getMetricName(metric *m)
{
    return(m->info.name);
}

dynMetricInfo *getMetricInfo(metric *m)
{
    return(&m->info);
}

metric *getMetric(metricDefinitionNode *mi)
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
    instance = NULL;

    assert(proc && point);
}

bool instReqNode::insertInstrumentation()
{
    instance = addInstFunc(proc, point, ast, when, order);
    if (instance) return(true);

    return(false);
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

    unitCost = ast->cost() / cyclesPerSecond;
    frequency = getPointFrequency(point);
    value = unitCost * frequency;

    return(value);
}

dataReqNode::dataReqNode(dataObjectType iType,
                      process *iProc,
                      int iInitialValue,
                      bool iReport,
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

unsigned dataReqNode::getInferiorPtr() 
{
    unsigned param=0;
    timerHandle *timerInst;
    intCounterHandle *counterInst;

    if (type == intCounter) {
	counterInst = (intCounterHandle *) instance;
	if (counterInst) {
	    param = (unsigned) counterInst->counterPtr;
	} else {
	    param = 0;
	}
    } else if (type == timer) {
	timerInst = (timerHandle *) instance;
	if (timerInst) {
	    param = (unsigned) timerInst->timerPtr;
	} else {
	    param = 0;
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

// allow a global "variable" to be inserted
// this will not report any values
// it is used internally by generated code -- see metricDefs-pvm.C
void dataReqNode::insertGlobal() {
  if (type == intCounter) {
    intCounterHandle *ret;
    ret = createCounterInstance();
    instance = (void *) ret;
    id = ret->data.id;
  } else 
    abort();
}

void dataReqNode::insertInstrumentation(metricDefinitionNode *mi) 
{
    if (type == intCounter) {
	intCounterHandle *ret;
	ret = createCounterInstance();
	instance = (void *) ret;
	id = ret->data.id;
	midToMiMap[id.id] = mi;
    } else {
	timerHandle *ret;
	ret = createTimerInstance();
	instance = (void *) ret;
	id = ret->data.id;
	midToMiMap[id.id] = mi;
    }
}

void dataReqNode::disable()
{
    if (!instance) return;

    if (!midToMiMap.defines(id.id))
      abort();
    midToMiMap.undef(id.id);

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

internalMetric::internalMetric(const string n,
			       int style,
			       int a, 
		               const string units, 
  			       sampleValueFunc f,
			       resourcePredicate *r,
			       bool really)
: metRec(n, style, a, units, NULL, r), name(n), node(NULL), value(0.0),
  func(f), cumulativeValue(0.0), reallyIsEventCounter(really)
{
    allInternalMetrics[name] = this;
}

timeStamp getCurrentTime(bool firstRecordRelative)
{
    time64 now;
    timeStamp ret;
    struct timeval tv;

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
    sampleValue value=0;

    static timeStamp end=0.0;

    // see if we have a sample to establish time base.
    if (!firstRecordTime) return;
    if (end==0.0)
	end = firstRecordTime/MILLION;

    now = getCurrentTime(false);

    //  check if it is time for a sample
    if (now < end + samplingRate) 
	return;

    start = end;
    end = now;

    metricDefinitionNode *mdn;
    internalMetric *imp;
    dictionary_hash_iter<metricDefinitionNode*, internalMetric*>
	imi(internalMetric::activeInternalMetrics);
    while (imi.next(mdn, imp)) { 
	if (imp->enabled()) {
	    // KLUDGE - sampledFunc doesn't work - mdc
	    if (imp->getName() == "active_processes") {
		value = (end - start) * processMap.size();
	    } else if (imp->metRec.info.style == EventCounter) {
		value = imp->getValue();
		if (imp->reallyIsEventCounter) {
		    assert(value + 0.0001 >= imp->cumulativeValue);
		    value -= imp->cumulativeValue;
		    imp->cumulativeValue += value;
		}
	    }
	    imp->node->forwardSimpleValue(start, end, value);
	}
    }
}
