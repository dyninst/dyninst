/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricFocusNode.C,v $
 * Revision 1.103  1996/08/16 21:19:21  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.102  1996/08/12 16:27:07  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.101  1996/07/25 23:24:03  mjrg
 * Added sharing of metric components
 *
 * Revision 1.100  1996/06/20 21:34:01  naim
 * Minor change - naim
 *
 * Revision 1.99  1996/06/09  18:55:33  newhall
 * removed debug output
 *
 * Revision 1.98  1996/05/15  18:32:49  naim
 * Fixing bug in inferiorMalloc and adding some debugging information - naim
 *
 * Revision 1.97  1996/05/11  00:30:08  mjrg
 * Fixed memory leak in guessCost
 *
 * Revision 1.96  1996/05/10 22:36:35  naim
 * Bug fix and some improvements passing a reference instead of copying a
 * structure - naim
 *
 */

#include "util/h/headers.h"
#include <assert.h>

#include "metric.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
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
#include "paradynd/src/mdld.h"
#include "util/h/Timer.h"
#include "paradynd/src/mdld.h"
#include "showerror.h"
#include "costmetrics.h"

extern vector<unsigned> getAllTrampsAtPoint(instInstance *instance);

void flush_batch_buffer(int);
void batchSampleData(int program, int mid, double startTimeStamp,
                     double endTimeStamp, double value, unsigned val_weight,
		     bool internal_metric);

#ifdef sparc_tmc_cmost7_3
extern int getNumberOfCPUs();
#endif

double currentPredictedCost = 0.0;

dictionary_hash <unsigned, metricDefinitionNode*> midToMiMap(uiHash);

unsigned mdnHash(const metricDefinitionNode *&mdn) {
  return ((unsigned) mdn);
}


dictionary_hash<unsigned, metricDefinitionNode*> allMIs(uiHash);
dictionary_hash<string, metricDefinitionNode*> allMIComponents(string::hash);
vector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

bool mdl_internal_metric_data(string& metric_name, mdl_inst_data& result) {
  unsigned size = internalMetric::allInternalMetrics.size();
  for (unsigned u=0; u<size; u++) {
    internalMetric *theIMetric = internalMetric::allInternalMetrics[u];
    if (theIMetric->name() == metric_name) {
      result.aggregate = theIMetric->aggregate();
      result.style = theIMetric->style();
      return true;
    }
  }

  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++) {
    if (costMetric::allCostMetrics[u2]->name() == metric_name) {
      result.aggregate = costMetric::allCostMetrics[u2]->aggregate();
      result.style = costMetric::allCostMetrics[u2]->style();
      return true;
    }
  }

  return (mdl_metric_data(metric_name, result));
}

// for non-aggregate metrics
metricDefinitionNode::metricDefinitionNode(process *p, string& met_name, 
                                           vector< vector<string> >& foc,
                                           string& cat_name, int agg_style)
: aggregate_(false), 
  aggOp(agg_style), // CM5 metrics need aggOp to be set
  inserted_(false), met_(met_name), focus_(foc), flat_name_(cat_name),
  aggSample(0),
  cumulativeValue(0.0), samples(0),
  id_(-1), originalCost_(0.0), inform_(false), proc_(p)
{
  mdl_inst_data md;
  assert(mdl_internal_metric_data(met_name, md));
  style_ = md.style;
}

float metricDefinitionNode::getMetricValue()
{
    float total;

    if (aggregate_) {
        total = 0.0;
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          total += components[u]->getMetricValue();
        return(0.0);
    }
    return (data[0]->getMetricValue());
}

// for aggregate metrics
metricDefinitionNode::metricDefinitionNode(string& metric_name,
                                           vector< vector<string> >& foc,
                                           string& cat_name, 
                                           vector<metricDefinitionNode*>& parts,
					   int agg_op)
: aggregate_(true), aggOp(agg_op), inserted_(false), met_(metric_name), focus_(foc),
  flat_name_(cat_name), components(parts),
  aggSample(agg_op),
  id_(-1), originalCost_(0.0), inform_(false), proc_(NULL)
{
  unsigned p_size = parts.size();
  for (unsigned u=0; u<p_size; u++) {
    metricDefinitionNode *mi = parts[u];
    metricDefinitionNode *t = (metricDefinitionNode *) this;
    mi->aggregators += t;
    mi->samples += aggSample.newComponent();
  }
}

float getProcessCount() {  return ((float) processVec.size()); }

// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
metricDefinitionNode *doInternalMetric(vector< vector<string> >& canon_focus,
                                       string& metric_name, string& flat_name,
                                       bool enable, bool& matched)
{
  // called by createMetricInstance, below.
  matched = false;
  metricDefinitionNode *mn = 0; 

  // check to see if this is an internal metric
  unsigned im_size = internalMetric::allInternalMetrics.size();
  for (unsigned im_index=0; im_index<im_size; im_index++){
    internalMetric *theIMetric = internalMetric::allInternalMetrics[im_index];
    if (theIMetric->name() == metric_name) {
      matched = true;
      if (!enable)
	 return NULL;

      if (!theIMetric->legalToInst(canon_focus)) {

// This message is handled by paradyn (it should not be handled by the daemons)
/* 
	 cout << "Sorry, illegal to instrument internal metric " << metric_name << " with focus of:" << endl;
	 for (unsigned hier=0; hier < canon_focus.size(); hier++) {
	    const vector<string> &thisHierStr = canon_focus[hier];

	    for (unsigned part=0; part < thisHierStr.size(); part++) {
	       cout << "/" << thisHierStr[part];
	    }

	    if (hier < canon_focus.size()-1)
               cout << ",";
         }
	 cout << endl;
*/

         return NULL;
      }

      mn = new metricDefinitionNode(NULL, metric_name, canon_focus, 
                                    flat_name, theIMetric->aggregate());
      assert(mn);

      theIMetric->enableNewInstance(mn);
      return(mn);
    }
  }

  // check to see if this is a cost metric
  for (unsigned i=0; i < costMetric::allCostMetrics.size(); i++){
     if(costMetric::allCostMetrics[i]->name() == metric_name){
	  matched = true;
	  if (!enable) return 0;
	  costMetric *nc = costMetric::allCostMetrics[i];
	  if (!nc->legalToInst(canon_focus)) return 0;
	  mn = new metricDefinitionNode(NULL, metric_name, canon_focus,
					flat_name, nc->aggregate());

          nc->enable(mn); 
	  return(mn);

     }
  }

  return NULL;
}

/*
 * See if we can find the requested metric instance.
 *   Currently this is only used to cache structs built for cost requests 
 *   which are then instantiated.  This could be used as a general system
 *   to request find sub-metrics that are already.defines and use them to
 *   reduce cost.  This would require adding the componenets of an aggregate
 *   into the allMIs list since searching tends to be top down, not bottom
 *   up.  This would also require adding a ref count to many of the structures
 *   so they only get deleted when we are really done with them.
 *
 */

metricDefinitionNode *createMetricInstance(string& metric_name, vector<u_int>& focus,
                                           bool enable, bool& internal)
{
    metricDefinitionNode *mi= NULL;

    // first see if it is already defined.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi(allMIs);

    vector< vector<string> > string_foc;
    if (!resource::foc_to_strings(string_foc, focus)) return NULL;
    vector< vector<string> > canon_focus;
    resource::make_canonical(string_foc, canon_focus);
    string flat_name(metric_name);

    unsigned cf_size = canon_focus.size();
    unsigned u;
    for (u=0; u<cf_size; u++) {
      unsigned v_size = canon_focus[u].size();
      for (unsigned v=0; v<v_size; v++) 
        flat_name += canon_focus[u][v];
    }

    // TODO -- a dictionary search here will be much faster
    while (mdi.next(u, mi))
      if (mi->getFullName() == flat_name)
        return mi;

    if (mdl_can_do(metric_name)) {
      mi = mdl_do(canon_focus, metric_name, flat_name, processVec);
    } else {
      bool matched;
      mi=doInternalMetric(canon_focus,metric_name,flat_name,enable,matched);
      internal = true;
    }
    return(mi);
}


// propagate this metric instance to process p.
// p is a process that started after the metric instance was created.
void metricDefinitionNode::propagateMetricInstance(process *p) {

  metricDefinitionNode *mi = NULL;
  vector<process *> vp(1,p);
  bool internal = false;

  unsigned comp_size = components.size();

  if (comp_size == 0)
    return;

  for (unsigned u = 0; u < comp_size; u++) {
    if (components[u]->proc() == p) {
      // The metric is already enabled for this process. This case can 
      // happen when we are starting several processes at the same time.
      return;
    }
  }

  if (mdl_can_do(met_)) {
      // Make the unique ID for this metric/focus visible in MDL.
      string vname = "$globalId";
      mdl_env::add(vname, false, MDL_T_INT);
      mdl_env::set(this->getMId(), vname);

      mi = mdl_do(focus_, met_, flat_name_, vp);
  } else {
    // internal metrics don't need to be propagated
    mi = NULL;
  }

  if (mi) {
    assert(mi->components.size() == 1);

    components += mi->components[0];
    mi->components[0]->aggregators[0] = this;
    mi->components[0]->samples[0] = aggSample.newComponent();
    if (!internal)
      mi->components[0]->insertInstrumentation();

    // update cost
    float cost = mi->cost();
    if (cost > originalCost_) {
      currentPredictedCost += cost - originalCost_;
      originalCost_ = cost;
    }

    mi->components.resize(0);
    delete mi;
  }
}


// called when all components have been removed (because the processes have exited)
void metricDefinitionNode::endOfDataCollection() {
  assert(components.size() == 0);

  // flush aggregateSamples
  sampleInterval ret = aggSample.aggregateValues();

  while (ret.valid) {
    assert(ret.end > ret.start);
    assert(ret.start >= (firstRecordTime/MILLION));
    assert(ret.end >= (firstRecordTime/MILLION));
    batchSampleData(0, id_, ret.start, ret.end, ret.value,
		    aggSample.numComponents(),false);
    ret = aggSample.aggregateValues();
  }
  flush_batch_buffer(0);
  tp->endOfDataCollection(id_);
}


// remove a component from an aggregate.
void metricDefinitionNode::removeFromAggregate(metricDefinitionNode *comp) {
  unsigned size = components.size();
  for (unsigned u = 0; u < size; u++) {
    if (components[u] == comp) {
      components[u] = components[size-1];
      components.resize(size-1);
      if (size == 1) {
	endOfDataCollection();
      }
      return;
    }
  }
  // should always find the right component 
  assert(0);
}

// remove this instance from all aggregators it is a component of.
void metricDefinitionNode::removeThisInstance() {
  assert(!aggregate_);
  unsigned aggr_size = aggregators.size();
  assert(aggr_size > 0);
  for (unsigned u = 0; u < aggr_size; u++) {
    aggregators[u]->aggSample.removeComponent(samples[u]);
    aggregators[u]->removeFromAggregate(this); 
  }
}


// Called when a process exits to remove the component associated to proc 
// from all metric instances
// Remove the metric instances that don't have any components left
void removeFromMetricInstances(process *proc) {
    vector<metricDefinitionNode *> MIs = allMIComponents.values();
    for (unsigned j = 0; j < MIs.size(); j++) {
      if (MIs[j]->proc() == proc)
	MIs[j]->removeThisInstance();
    }
    costMetric::removeProcessFromAll(proc);
}


// called when a process forks. this is a metricDefinitionNode of the parent.
// Duplicate it for the child
metricDefinitionNode *metricDefinitionNode::forkProcess(process *child) {
    metricDefinitionNode *mi = new metricDefinitionNode(child, met_, focus_, flat_name_, aggOp);

    for (unsigned u = 0; u < data.size(); u++) {
      mi->data += dataReqNode::forkProcess(mi, data[u]);
    }
    for (unsigned u = 0; u < requests.size(); u++) {
      mi->requests += instReqNode::forkProcess(requests[u], child);
    }
    mi->inserted_ = true;

    return mi;
}

void metricDefinitionNode::handleFork(process *parent, process *child) {
    vector<metricDefinitionNode *> MIs = allMIs.values();
    for (unsigned u = 0; u < MIs.size(); u++) {
      metricDefinitionNode *mi = MIs[u];
      for (unsigned v = 0; v < mi->components.size(); v++) {
	if (mi->components[v]->proc() == parent) {
	  metricDefinitionNode *childMI = mi->components[v]->forkProcess(child);
	  if (mi->focus_[resource::process].size()== 1) {
	    mi->components += childMI;
	    childMI->aggregators += mi;
	    childMI->samples += mi->aggSample.newComponent();
	  }
	  else {
	    // this metric is only being computed for selected processes.
	    for (unsigned w = 0; w < childMI->data.size(); w++)
		 midToMiMap.undef(childMI->data[w]->getSampleId());
	  }
	}
      }
    }
}



// startCollecting is a friend of metricDefinitionNode; can it be
// made a member function of metricDefinitionNode instead?
// Especially since it clearly is an integral part of the class;
// in particular, it sets the crucial vrble "id_"
int startCollecting(string& metric_name, vector<u_int>& focus, int id) 
{
    bool internal = false;

    // Make the unique ID for this metric/focus visible in MDL.
    string vname = "$globalId";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(id, vname);

    metricDefinitionNode *mi = createMetricInstance(metric_name, focus,
                                                    true, internal);

    if (!mi) return(-1);

    mi->id_ = id;

    allMIs[mi->id_] = mi;

    float cost = mi->cost();
    mi->originalCost_ = cost;

    currentPredictedCost += cost;
    if (!internal) mi->insertInstrumentation();
    metResPairsEnabled++;
    return(mi->id_);
}

float guessCost(string& metric_name, vector<u_int>& focus)
{
    float cost;
    metricDefinitionNode *mi;
    bool internal;
    mi = createMetricInstance(metric_name, focus, false, internal);
    if (!mi) return(0.0);
    cost = mi->cost();
    // delete the metric instance, if it is not being used 
    if (!allMIs.defines(mi->getMId()))
      delete mi;
    return(cost);
}

bool metricDefinitionNode::insertInstrumentation()
{
    bool needToCont = false;

    if (inserted_) return(true);

    /* check all proceses are in an ok state */
#ifdef notdef
    if (!isApplicationPaused()) {
        pauseAllProcesses();
        needToCont = true;
    }
#endif
    inserted_ = true;
    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          components[u]->insertInstrumentation();
    } else {
      needToCont = proc_->status() == running;
      bool res = proc_->pause();
      if (!res)
	return false;
      //if (proc_->status() == exited)
      //  return false;
      unsigned size = data.size();
      for (unsigned u=0; u<size; u++)
        if (!(data[u]->insertInstrumentation(this))) return(false);
      for (unsigned u1=0; u1<requests.size(); u1++)
        requests[u1].insertInstrumentation();
      if (needToCont) proc_->continueProc();
    }
    //if (needToCont) continueAllProcesses();
    return(true);
}

float metricDefinitionNode::cost()
{
    float ret;
    float nc;

    ret = 0.0;
    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++) {
          nc = components[u]->cost();
          if (nc > ret) ret = nc;
        }
    } else {
      for (unsigned u=0; u<requests.size(); u++)
        ret += requests[u].cost();
    }
    return(ret);
}

void metricDefinitionNode::disable()
{
    // check for internal metrics

    unsigned ai_size = internalMetric::allInternalMetrics.size();
    for (unsigned u=0; u<ai_size; u++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[u];
      if (theIMetric->disableByMetricDefinitionNode(this)) {
	//logLine("disabled internal metric\n");
        return;
      }
    }

    // check for cost metrics
    for (unsigned i=0; i<costMetric::allCostMetrics.size(); i++){
      if (costMetric::allCostMetrics[i]->node == this) {
        costMetric::allCostMetrics[i]->disable();
	//logLine("disabled cost metric\n");
        return;
    }}

    if (!inserted_) return;

    inserted_ = false;
    if (aggregate_) {
        /* disable components of aggregate metrics */
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++) {
	  //components[u]->disable();
	  metricDefinitionNode *m = components[u];
	  unsigned aggr_size = m->aggregators.size();
	  assert(aggr_size == m->samples.size());
	  for (unsigned u1=0; u1 < aggr_size; u1++) {
	    if (m->aggregators[u1] == this) {
	      m->aggregators[u1] = m->aggregators[aggr_size-1];
	      m->aggregators.resize(aggr_size-1);
	      m->samples[u1] = m->samples[aggr_size-1];
	      m->samples.resize(aggr_size-1);
	      break;
	    }
	  }
	  assert(m->aggregators.size() == aggr_size-1);
	  // disable component only if it is not being shared
	  if (aggr_size == 1) {
	    m->disable();
	  }
	}

    } else {
      vector<unsigVecType> pointsToCheck;
      for (unsigned u1=0; u1<requests.size(); u1++) {
        unsigVecType pointsForThisRequest = 
            getAllTrampsAtPoint(requests[u1].getInstance());
        pointsToCheck += pointsForThisRequest;
        requests[u1].disable(pointsForThisRequest);
      }
      for (unsigned u=0; u<data.size(); u++) {
        data[u]->disable(pointsToCheck);
      }
    }

}

void metricDefinitionNode::removeComponent(metricDefinitionNode *comp) {
    assert(!comp->aggregate_);
    unsigned aggr_size = comp->aggregators.size();
    unsigned found = aggr_size;

    if (aggr_size == 0) {
      delete comp;
      return;
    }

    // component has more than one aggregator. Remove this from list of aggregators
    for (unsigned u = 0; u < aggr_size; u++) {
      if (comp->aggregators[u] == this) {
	found = u;
	break;
      }
    }
    if (found == aggr_size)
     return;
    assert(found < aggr_size);
    assert(aggr_size == comp->samples.size());
    comp->aggregators[found] = comp->aggregators[aggr_size-1];
    comp->aggregators.resize(aggr_size-1);
    comp->samples[found] = comp->samples[aggr_size-1];
    comp->samples.resize(aggr_size-1);

    if (aggr_size == 1) {
      delete comp;
      return;
    }

}

metricDefinitionNode::~metricDefinitionNode()
{
    if (aggregate_) {
        /* delete components of aggregate metrics */
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
	  removeComponent(components[u]);
	  //delete components[u];
        components.resize(0);
    } else {
      allMIComponents.undef(flat_name_);
      unsigned size = data.size();
      for (unsigned u=0; u<size; u++)
        delete data[u];
    }
}


//////////////////////////////////////////////////////////////////////////////
// Buffer the samples before we actually send it                            //
//      Send it when the buffers are full                                   //
//      or, send it when the last sample in the interval has arrived.       //
//////////////////////////////////////////////////////////////////////////////

const unsigned SAMPLE_BUFFER_SIZE = (1*1024)/sizeof(T_dyninstRPC::batch_buffer_entry);
bool BURST_HAS_COMPLETED = false;
   // set to true after a burst (after a processTraceStream(), or sampleNodes for
   // the CM5), which will force the buffer to be flushed before it fills up
   // (if not, we'd have bad response time)

vector<T_dyninstRPC::batch_buffer_entry> theBatchBuffer (SAMPLE_BUFFER_SIZE);
unsigned int batch_buffer_next=0;

void flush_batch_buffer(int program) {
   // don't need to flush if the batch had no data (this does happen; see
   // perfStream.C)
   if (batch_buffer_next == 0)
      return;

   // alloc buffer of the exact size to make communication
   // more efficient.  Why don't we send theBatchBuffer with a count?
   // This would work but would always (in the igen call) copy the entire
   // vector.  This solution has the downside of calling new but is not too bad
   // and is clean.
   vector<T_dyninstRPC::batch_buffer_entry> copyBatchBuffer(batch_buffer_next);
   for (unsigned i=0; i< batch_buffer_next; i++)
      copyBatchBuffer[i] = theBatchBuffer[i];

   //char myLogBuffer[120] ;
   //sprintf(myLogBuffer, "in metric.C batch size about to send = %d\n", batch_buffer_next) ;
   //logLine(myLogBuffer) ;

   // Now let's do the actual igen call!

#ifdef FREEDEBUG
timeStamp t1,t2;
t1=getCurrentTime(false);
#endif

   tp->batchSampleDataCallbackFunc(program, copyBatchBuffer);

#ifdef FREEDEBUG
t2=getCurrentTime(false);
if ((float)(t2-t1) > 15.0) {
sprintf(errorLine,"++--++ TEST ++--++ batchSampleDataCallbackFunc took %5.2f secs, size=%d, Kbytes=%5.2f\n",(float)(t2-t1),sizeof(T_dyninstRPC::batch_buffer_entry),(float)(sizeof(T_dyninstRPC::batch_buffer_entry)*copyBatchBuffer.size()/1024.0));
logLine(errorLine);
}
#endif

   BURST_HAS_COMPLETED = false;
   batch_buffer_next = 0;
}

void batchSampleData(int program, int mid, double startTimeStamp,
                     double endTimeStamp, double value, unsigned val_weight,
		     bool internal_metric) 
{
   // This routine is called where we used to call tp->sampleDataCallbackFunc.
   // We buffer things up and eventually call tp->batchSampleDataCallbackFunc

#ifdef notdef
   char myLogBuffer[120] ;
   sprintf(myLogBuffer, "mid %d, value %g\n", mid, value) ;
   logLine(myLogBuffer) ;
#endif

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (batch_buffer_next >= SAMPLE_BUFFER_SIZE || BURST_HAS_COMPLETED) {
      flush_batch_buffer(program);
   }

   // Now let's batch this entry.
   T_dyninstRPC::batch_buffer_entry &theEntry = theBatchBuffer[batch_buffer_next];
   theEntry.mid = mid;
   theEntry.startTimeStamp = startTimeStamp;
   theEntry.endTimeStamp = endTimeStamp;
   theEntry.value = value;
   theEntry.weight = val_weight;
   theEntry.internal_met = internal_metric;
   batch_buffer_next++;
}

void metricDefinitionNode::forwardSimpleValue(timeStamp start, timeStamp end,
                                       sampleValue value, unsigned weight,
				       bool internal_met)
{
  // TODO mdc
    assert(start >= (firstRecordTime/MILLION));
    assert(end >= (firstRecordTime/MILLION));
    assert(end > start);

    batchSampleData(0, id_, start, end, value, weight, internal_met);
}

void metricDefinitionNode::updateValue(time64 wallTime, 
                                       sampleValue value)
{
    // sampleInterval ret;
    // extern timeStamp elapsedPauseTime;

    // sampleTime = wallTime/ 1000000.0 - elapsedPauseTime;
    // commented out elapsedPauseTime because we don't currently stop CM-5
    // node processes. (brought it back jkh 11/9/93).
    timeStamp sampleTime = wallTime / 1000000.0; 
    assert(value >= -0.01);

    // TODO -- is this ok?
    // TODO -- do sampledFuncs work ?
    if (style_ == EventCounter) { 

      // only use delta from last sample.
      if (value < cumulativeValue) {
        if ((value/cumulativeValue) < 0.99999) {
          assert((value + 0.0001)  >= cumulativeValue);
        } else {
          // floating point rounding error ignore
          cumulativeValue = value;
        }
      }

      //        if (value + 0.0001 < cumulativeValue)
      //           printf ("WARNING:  sample went backwards!!!!!\n");
      value -= cumulativeValue;
      cumulativeValue += value;
    } 

      // char buffer[200];
      // sprintf(buffer, "metricDefinitionNode::updateValue: value = %f aggOp = %d agg.size = %d valueList.size = %d\n", value,sample.aggOp,aggregators.size(),valueList.count());
      // logLine(buffer);
    //
    // If style==EventCounter then value is changed. Otherwise, it keeps the
    // the current "value" (e.g. SampledFunction case). That's why it is not
    // necessary to have an special case for SampledFunction.
    //


    assert(samples.size() == aggregators.size());
    for (unsigned u = 0; u < samples.size(); u++) {
      if (samples[u]->firstValueReceived())
	samples[u]->newValue(sampleTime, value);
      else {
	samples[u]->startTime(sampleTime);
      }
      aggregators[u]->updateAggregateComponent();
    }
}


void metricDefinitionNode::updateAggregateComponent()
{
    sampleInterval ret;
    ret = aggSample.aggregateValues();
    if (ret.valid) {
        assert(ret.end > ret.start);
        assert(ret.start >= (firstRecordTime/MILLION));
        assert(ret.end >= (firstRecordTime/MILLION));
	batchSampleData(0, id_, ret.start, ret.end, ret.value,
			aggSample.numComponents(),false);
    }
}

//
// Costs are now reported to paradyn like other metrics (ie. we are not
// calling reportInternalMetrics to deliver cost values, instead we wait
// until we have received a new interval of cost data from each process)
// note: this only works for the CM5 because all cost metrics are sumed
// at the daemons and at paradyn, otherwise the CM5 needs its own version
// of this routine that uses the same aggregate method as the one for paradyn 
//
void processCost(process *proc, traceHeader *h, costUpdate *s)
{
    timeStamp newSampleTime = (h->wall / 1000000.0);
    timeStamp newProcessTime = (h->process / 1000000.0);

    //  kludge for CM5 pauseTime computation
    proc->pauseTime = s->pauseTime;

    timeStamp lastProcessTime = 
			totalPredictedCost->getLastSampleProcessTime(proc); 

    // find the portion of uninstrumented time for this interval
    double unInstTime = ((newProcessTime - lastProcessTime) 
			 / (1+currentPredictedCost));
    // update predicted cost
    // note: currentPredictedCost is the same for all processes 
    //       this should be changed to be computed on a per process basis
    sampleValue newPredCost = totalPredictedCost->getCumulativeValue(proc);
    newPredCost += (float)(currentPredictedCost*unInstTime); 
    totalPredictedCost->updateValue(proc,newPredCost,
				    newSampleTime,newProcessTime);
    // update observed cost 
    observed_cost->updateValue(proc,s->obsCostIdeal,
			       newSampleTime,newProcessTime);

    // update smooth observed cost
    smooth_obs_cost->updateSmoothValue(proc,s->obsCostIdeal,
				 newSampleTime,newProcessTime);
    return;
}

void processSample(traceHeader *h, traceSample *s)
{
    unsigned mid = s->id.id;

    if (!midToMiMap.defines(mid)) {
#ifdef ndef
      sprintf(errorLine, "Sample %d not for a valid metric instance\n", 
              s->id.id);
      logLine(errorLine);
#endif
      return;
    }
    metricDefinitionNode *mi = midToMiMap[mid];
    //    sprintf(errorLine, "sample id %d at time %8.6f = %f\n", s->id.id, 
    //  ((double) *(int*) &h->wall) + (*(((int*) &h->wall)+1))/1000000.0, s->value);
    //    logLine(errorLine);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}


/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(process *iProc,
                         instPoint *iPoint,
                         const AstNode &iAst,
                         callWhen  iWhen,
                         callOrder o) : ast(iAst) {
    proc = iProc;
    point = iPoint;
    when = iWhen;
    order = o;
    instance = NULL;

    assert(proc && point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parent, process *child) {
    instReqNode ret = instReqNode(child, parent.point, parent.ast, parent.when,
				  parent.order);
    assert(child->instInstanceMapping.defines(parent.instance));
    ret.instance = child->instInstanceMapping[parent.instance];
    return ret;
}


bool instReqNode::insertInstrumentation()
{
    instance = addInstFunc(proc, point, ast, when, order);
    if (instance) return(true);
    return(false);
}

void instReqNode::disable(const vector<unsigned> &pointsToCheck)
{
    deleteInst(instance, pointsToCheck);
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
    int unitCostInCycles;

    unitCostInCycles = ast.cost() + getPointCost(proc, point) +
        getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
    // printf("unit cost = %d cycles\n", unitCostInCycles);
    unitCost = unitCostInCycles/ cyclesPerSecond;
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

    if (type == INTCOUNTER) {
        ret = getIntCounterValue((intCounterHandle*) instance);
    } else if (type == TIMER) {
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

    if (type == INTCOUNTER) {
        counterInst = (intCounterHandle *) instance;
        if (counterInst) {
            param = (unsigned) counterInst->counterPtr;
        } else {
            param = 0;
        }
    } else if (type == TIMER) {
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
  if (type == INTCOUNTER) {
    intCounterHandle *ret;
    ret = createCounterInstance();
    if (!ret) return;
    instance = (void *) ret;
    id = ret->data.id;
  } else 
    abort();
}

bool dataReqNode::insertInstrumentation(metricDefinitionNode *mi) 
{
    if (type == INTCOUNTER) {
        intCounterHandle *ret;
        ret = createCounterInstance();
        if (!ret) return false;
        instance = (void *) ret;
        id = ret->data.id;
        unsigned mid = id.id;
        midToMiMap[mid] = mi;
    } else {
        timerHandle *ret;
        ret = createTimerInstance();
        if (!ret) return false;
        instance = (void *) ret;
        id = ret->data.id;
        unsigned mid = id.id;
        midToMiMap[mid] = mi;
    }
    return true;
}

dataReqNode *dataReqNode::forkProcess(metricDefinitionNode *childMi, dataReqNode *parent) {
    dataReqNode *ret = new dataReqNode(parent->type, childMi->proc(), 
				       parent->initialValue, 
				       parent->report, parent->tType);
    if (ret->type == INTCOUNTER) {
        intCounterHandle *h = dupIntCounter((intCounterHandle *)parent->instance, 
					    childMi->proc(),
					    ret->initialValue);
	ret->instance = (void *) h;
        ret->id = h->data.id;
        unsigned mid = ret->id.id;
        midToMiMap[mid] = childMi;
    } else {
        timerHandle *h = dupTimer((timerHandle *)parent->instance, childMi->proc());
	ret->instance = (void *) h;
        ret->id = h->data.id;
        unsigned mid = ret->id.id;
        midToMiMap[mid] = childMi;
    }
    return ret;
}

void dataReqNode::disable(const vector<unsigVecType> &pointsToCheck)
{
    if (!instance) return;

    unsigned mid = id.id;
    if (!midToMiMap.defines(mid))
      abort();
    midToMiMap.undef(mid);

    if (type == TIMER) {
        freeTimer((timerHandle *) instance, pointsToCheck);
    } else if (type == INTCOUNTER) {
        freeIntCounter((intCounterHandle *) instance, pointsToCheck);
    } else {
        abort();
    }
    instance = NULL;
}

dataReqNode::~dataReqNode()
{
    instance = NULL;
}

timeStamp getCurrentTime(bool firstRecordRelative)
{
    static double previousTime=0.0;
    struct timeval tv;
  retry:
    assert(gettimeofday(&tv, NULL) == 0); // 0 --> success; -1 --> error

    double seconds_dbl = tv.tv_sec * 1.0;
    assert(tv.tv_usec < 1000000);
    double useconds_dbl = tv.tv_usec * 1.0;
  
    seconds_dbl += useconds_dbl / 1000000.0;

    if (seconds_dbl < previousTime) goto retry;
    previousTime = seconds_dbl;

    if (firstRecordRelative)
       seconds_dbl -= firstRecordTime;

    return seconds_dbl;    
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
    // We don't deliver a sample if the application is paused
    if (isApplicationPaused() || now < end + samplingRate) 
	return;

    start = end;
    end = now;

    // TODO -- clean me up, please

    unsigned ai_size = internalMetric::allInternalMetrics.size();
    for (unsigned u=0; u<ai_size; u++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[u];
      // Loop thru all enabled instances of this internal metric...

      for (unsigned v=0; v < theIMetric->num_enabled_instances(); v++) {
	internalMetric::eachInstance &theInstance = theIMetric->getEnabledInstance(v);
           // not "const" since bumpCumulativeValueBy() may be called

        if (theIMetric->name() == "active_processes") {
	  //value = (end - start) * activeProcesses;
	  value = (end - start) * theInstance.getValue();
        } else if (theIMetric->name() == "bucket_width") {
	  //value = (end - start)* theInstance.getValue();
	  // I would prefer to use (end-start) * theInstance.getValue(); however,
	  // we've had some problems getting setValue() called in time, thus
	  // leaving us with getValues() of 0 sometimes.  See longer comment in dynrpc.C --ari
	  extern float currSamplingRate;
	  value = (end - start) * currSamplingRate;
        } else if (theIMetric->name() == "number_of_cpus") {
          value = (end - start) * numberOfCPUs;
        } else if (theIMetric->style() == EventCounter) {
          value = theInstance.getValue();
          // assert((value + 0.0001)  >= imp->cumulativeValue);
          value -= theInstance.getCumulativeValue();
          theInstance.bumpCumulativeValueBy(value);
        } else if (theIMetric->style() == SampledFunction) {
          value = theInstance.getValue();
        }

	theInstance.report(start, end, value);
	   // calls metricDefinitionNode->forwardSimpleValue()
      }
    }
}

void disableAllInternalMetrics() {
    for (unsigned u=0; u < internalMetric::allInternalMetrics.size(); u++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[u];

      // Now loop thru all the enabled instances of this internal metric...
      while (theIMetric->num_enabled_instances() > 0) {
 	internalMetric::eachInstance &theInstance = theIMetric->getEnabledInstance(0);
	tp->endOfDataCollection(theInstance.getMId());
	theIMetric->disableInstance(0);
      }
    }  
}
