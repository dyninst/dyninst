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
 * Revision 1.110  1996/11/14 14:27:59  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
 * Revision 1.109  1996/10/31 09:28:23  tamches
 * the shm-sampling commit; completely redesigned dataReqNode; designed
 * a handful of derived classes of dataReqNode, which replaces a lot of
 * existing code; removed some warnings; inferiorRPC.
 *
 * Revision 1.108  1996/10/20 20:18:16  mjrg
 * small change to assertions
 *
 * Revision 1.107  1996/10/03 22:12:01  mjrg
 * Removed multiple stop/continues when inserting instrumentation
 * Fixed bug on process termination
 * Removed machine dependent code from metric.C and process.C
 *
 * Revision 1.106  1996/09/26 18:58:51  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.105  1996/09/05 16:35:35  lzheng
 * Move the architecture dependent definations to the architecture dependent files
 *
 * Revision 1.104  1996/08/20 19:04:22  lzheng
 * Implementation of moving multiple instructions sequence and splitting
 * the instrumentation into two phases
 *
 * Revision 1.103  1996/08/16 21:19:21  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.102  1996/08/12 16:27:07  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.101  1996/07/25 23:24:03  mjrg
 * Added sharing of metric components
 *
 */

#include "util/h/headers.h"
#include <limits.h>
#include <assert.h>

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
#include "init.h"
#include "perfStream.h"
#include "main.h"
#include "stats.h"
#include "dynrpc.h"
#include "paradynd/src/mdld.h"
#include "util/h/Timer.h"
#include "showerror.h"
#include "costmetrics.h"
#include "metric.h"

extern vector<unsigned> getAllTrampsAtPoint(instInstance *instance);

void flush_batch_buffer();
void batchSampleData(int mid, double startTimeStamp, double endTimeStamp,
		     double value, unsigned val_weight, bool internal_metric);

#ifdef sparc_tmc_cmost7_3
extern int getNumberOfCPUs();
#endif

double currentPredictedCost = 0.0;

dictionary_hash <unsigned, metricDefinitionNode*> midToMiMap(uiHash);
   // maps low-level counter-ids to metricDefinitionNodes

unsigned mdnHash(const metricDefinitionNode *&mdn) {
  return ((unsigned)mdn) >> 2; // assume all addrs are 4-byte aligned
//  return ((unsigned) mdn);
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
  inserted_(false), installed_(false), met_(met_name), focus_(foc), 
  flat_name_(cat_name),
  aggSample(0),
  cumulativeValue(0.0), samples(0),
//  id_(-1), originalCost_(0.0), inform_(false), proc_(p)
  id_(-1), originalCost_(0.0), proc_(p)
{
  mdl_inst_data md;
  assert(mdl_internal_metric_data(met_name, md));
  style_ = md.style;
}

//float metricDefinitionNode::getMetricValue()
//{
//    float total;
//
//    if (aggregate_) {
//        total = 0.0;
//        unsigned c_size = components.size();
//        for (unsigned u=0; u<c_size; u++)
//          total += components[u]->getMetricValue();
//        return(0.0);
//    }
//    return (data[0]->getMetricValue());
//}

// for aggregate metrics
metricDefinitionNode::metricDefinitionNode(string& metric_name,
                                           vector< vector<string> >& foc,
                                           string& cat_name, 
                                           vector<metricDefinitionNode*>& parts,
					   int agg_op)
: aggregate_(true), aggOp(agg_op), inserted_(false),  installed_(false),
  met_(metric_name), focus_(foc),
  flat_name_(cat_name), components(parts),
  aggSample(agg_op),
//  id_(-1), originalCost_(0.0), inform_(false), proc_(NULL)
  id_(-1), originalCost_(0.0), proc_(NULL)
{
  unsigned p_size = parts.size();
  for (unsigned u=0; u<p_size; u++) {
    metricDefinitionNode *mi = parts[u];
    mi->aggregators += this;
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

      if (!theIMetric->legalToInst(canon_focus))
	 // Paradyn will handle this case and report appropriate error msg
         return NULL;

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
          assert(mn);

          nc->enable(mn); 

	  return(mn);
     }
  }

  // No matches found among internal or cost metrics
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

metricDefinitionNode *createMetricInstance(string& metric_name, 
                                           vector<u_int>& focus,
                                           bool enable, bool& internal)
{
    metricDefinitionNode *mi= NULL;

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

    // first see if it is already defined.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi(allMIs);

    // TODO -- a dictionary search here will be much faster
    while (mdi.next(u, mi))
      if (mi->getFullName() == flat_name)
        return mi; // this metricDefinitionNode has already been defined

    if (mdl_can_do(metric_name)) {
      mi = mdl_do(canon_focus, metric_name, flat_name, processVec);
      internal = false;
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
    batchSampleData(id_, ret.start, ret.end, ret.value,
		    aggSample.numComponents(),false);
    ret = aggSample.aggregateValues();
  }
  flush_batch_buffer();
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

// obligatory definition of static member vrble:
int metricDefinitionNode::counterId=0;

dataReqNode *metricDefinitionNode::addSampledIntCounter(int initialValue) {
   dataReqNode *result=NULL;

#ifdef SHM_SAMPLING
   // shared memory sampling of a reported intCounter
   result = new sampledShmIntCounterReqNode(initialValue,
					    metricDefinitionNode::counterId);
      // implicit conversion to base class
#else
   // non-shared-memory sampling of a reported intCounter
   result = new sampledIntCounterReqNode(initialValue,
					 metricDefinitionNode::counterId);
      // implicit conversion to base class
#endif

   assert(result);
   
   metricDefinitionNode::counterId++;

   dataRequests += result;
   return result;
}

dataReqNode *metricDefinitionNode::addUnSampledIntCounter(int initialValue) {
   // sampling of a non-reported intCounter (probably just a predicate)
   // NOTE: In the future, we should probably put un-sampled intcounters
   // into shared-memory when SHM_SAMPLING is defined.  After all, the shared
   // memory heap is faster.
   dataReqNode *result = new nonSampledIntCounterReqNode
                         (initialValue, metricDefinitionNode::counterId);
      // implicit conversion to base class
   assert(result);

   metricDefinitionNode::counterId++;

   dataRequests += result;
   return result;
};

dataReqNode *metricDefinitionNode::addWallTimer() {
   dataReqNode *result = NULL;

#ifdef SHM_SAMPLING
   result = new sampledShmWallTimerReqNode(metricDefinitionNode::counterId);
      // implicit conversion to base class
#else
   result = new sampledTimerReqNode(wallTime, metricDefinitionNode::counterId);
      // implicit conversion to base class
#endif

   assert(result);

   metricDefinitionNode::counterId++;

   dataRequests += result;
   return result;
}

dataReqNode *metricDefinitionNode::addProcessTimer() {
   dataReqNode *result = NULL;

#ifdef SHM_SAMPLING
   result = new sampledShmProcTimerReqNode(metricDefinitionNode::counterId);
      // implicit conversion to base class
#else
   result = new sampledTimerReqNode(processTime, metricDefinitionNode::counterId);
      // implicit conversion to base class
#endif

   assert(result);

   metricDefinitionNode::counterId++;

   dataRequests += result;
   return result;
};

// called when a process forks. this is a metricDefinitionNode of the parent.
// Duplicate it for the child
metricDefinitionNode *metricDefinitionNode::forkProcess(process *child) {
    metricDefinitionNode *mi = new metricDefinitionNode(child, met_, focus_, flat_name_, aggOp);
    assert(mi);

    for (unsigned u = 0; u < dataRequests.size(); u++) {
       dataReqNode *newNode = dataRequests[u]->dup(child,
						   metricDefinitionNode::counterId);
         // calls sampledIntCounterReqNode::dup() or sampledTimerReqNode::dup()
       assert(newNode);

       metricDefinitionNode::counterId++;

       // add to midToMiMap:
       unsigned mid = newNode->getSampleId();
       midToMiMap[mid] = mi;

       mi->dataRequests += newNode;
    }

    for (unsigned u = 0; u < instRequests.size(); u++) {
      mi->instRequests += instReqNode::forkProcess(instRequests[u], child);
    }

    mi->inserted_ = true;

    return mi;
}

void metricDefinitionNode::handleFork(const process *parent, process *child) {
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
	    for (unsigned w = 0; w < childMI->dataRequests.size(); w++)
		 midToMiMap.undef(childMI->dataRequests[w]->getSampleId());
	  }
	}
      }
    }
}

bool metricDefinitionNode::anythingToManuallyTrigger() const {
   if (aggregate_) {
      for (unsigned i=0; i < components.size(); i++)
	 if (components[i]->anythingToManuallyTrigger())
	    return true;
      return false;
   }
   else {
      for (unsigned i=0; i < instRequests.size(); i++)
	 if (instRequests[i].anythingToManuallyTrigger())
	    return true;
      return false;
   }

   assert(false);
}

void metricDefinitionNode::manuallyTrigger() {
   assert(anythingToManuallyTrigger());

   if (aggregate_) {
      for (unsigned i=0; i < components.size(); i++)
	 components[i]->manuallyTrigger();
   }
   else {
      for (unsigned i=0; i < instRequests.size(); i++)
	 if (instRequests[i].anythingToManuallyTrigger())
	    if (!instRequests[i].triggerNow(proc())) {
	       cerr << "manual trigger failed for an inst request" << endl;
	    }
   }
}


// startCollecting is called by dynRPC::enableDataCollection (or enableDataCollection2)
// in dynrpc.C
// startCollecting is a friend of metricDefinitionNode; can it be
// made a member function of metricDefinitionNode instead?
// Especially since it clearly is an integral part of the class;
// in particular, it sets the crucial vrble "id_"
int startCollecting(string& metric_name, vector<u_int>& focus, int id,
		    vector<process *> &procsToCont)
{
    bool internal = false;

    // Make the unique ID for this metric/focus visible in MDL.
    string vname = "$globalId";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(id, vname);

    metricDefinitionNode *mi = createMetricInstance(metric_name, focus,
                                                    true, internal);
       // calls mdl_do()
    if (!mi) return(-1);

    mi->id_ = id;

    allMIs[mi->id_] = mi;

    const float cost = mi->cost();
    mi->originalCost_ = cost;

    currentPredictedCost += cost;

#ifdef ndef
    // enable timing stuff: also code in insertInstrumentation()
    u_int start_size = test_heapsize;
    printf("ENABLE: %d %s %s\n",start_size,
        (mi->getMetName()).string_of(),
        (mi->getFullName()).string_of());
    static timer inTimer;
    inTimer.start();
#endif


    if (!internal) {

        // pause processes that are running and add them to procsToCont.
        // We don't rerun the processes after we insert instrumentation,
        // this will be done by our caller, after all instrumentation
        // has been inserted.
        for (unsigned u = 0; u < mi->components.size(); u++) {
          process *p = mi->components[u]->proc();
          if (p->status() == running && p->pause()) {
            procsToCont += p;
          }
        }


	mi->insertInstrumentation(); // calls pause and unpause (this could be a bug, since the next line should be allowed to execute before the unpause!!!)
	mi->checkAndInstallInstrumentation();

	// Now that the timers and counters have been allocated on the heap, and
	// the instrumentation added, we can manually execute instrumentation
	// we may have missed at $start.entry.  But has the process been paused
	// all this time?  Hopefully so; otherwise things can get screwy.

	if (mi->anythingToManuallyTrigger()) {
	   process *theProc = mi->components[0]->proc();
	   assert(theProc);

	   bool alreadyRunning = (theProc->status_ == running);

	   if (alreadyRunning)
	      theProc->pause();

	   mi->manuallyTrigger();

	   if (alreadyRunning)
	      theProc->continueProc(); // the continue will trigger our code
	   else
	      ; // the next time the process continues, we'll trigger our code
	}
    }

#ifdef ndef
    inTimer.stop();
    if(!start_size) start_size = test_heapsize;
    printf("It took %f:user %f:system %f:wall seconds heap_left: %d used %d\n"
                , inTimer.usecs(), inTimer.ssecs(), inTimer.wsecs(),
                test_heapsize,start_size-test_heapsize);
#endif

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
    // returns true iff successful
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
          if (!components[u]->insertInstrumentation())
	     return false; // shouldn't we try to undo what's already put in?
    } else {
      needToCont = proc_->status() == running;
      bool res = proc_->pause();
      if (!res)
	return false;

      // Loop thru "dataRequests", an array of (ptrs to) dataReqNode:
      // (Here we allocate ctrs/timers in the inferior heap but don't
      //  stick in any code)
      unsigned size = dataRequests.size();
      for (unsigned u=0; u<size; u++) {
	// the following allocs an object in inferior heap and arranges for
        // it to be sampled, as appropriate.
        if (!dataRequests[u]->insertInstrumentation(proc_, this))
           return false; // shouldn't we try to undo what's already put in?

	unsigned mid = dataRequests[u]->getSampleId();
	if (midToMiMap.defines(mid)) {
	   cerr << "insertInstrumentation warning: data node id " << mid << " already in use!!!" << endl;
	}

	midToMiMap[mid] = this;
      }

      // Loop thru "instRequests", an array of instReqNode:
      // (Here we insert code instrumentation, tramps, etc. via addInstFunc())
      for (unsigned u1=0; u1<instRequests.size(); u1++) {
	  // NEW: the following may also manually trigger the instrumentation
	  // via inferiorRPC.
	  returnInstance *retInst=NULL;
	  if (!instRequests[u1].insertInstrumentation(proc_, retInst))
	     return false; // shouldn't we try to undo what's already put in?

	  //cerr << "metricDefinitionNode::insertInstrumentation: retInst was set to " << (void *)retInst << endl;

	  if (retInst) {
	    //cerr << "adding a returnInst now!" << endl;
	    returnInsts += retInst;
	  }
	  else {
	    //cerr << "NOT adding a returnInst now!" << endl;
	  }
      }

      if (needToCont)
	 proc_->continueProc();
    }

    return(true);
}

bool metricDefinitionNode::checkAndInstallInstrumentation() {
    bool needToCont = false;

    if (installed_)
       return(true);

    installed_ = true;

    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
            components[u]->checkAndInstallInstrumentation();
    } else {
        needToCont = proc_->status() == running;
        if (!proc_->pause()) {
	    cerr << "checkAnd... pause failed" << endl; cerr.flush();
            return false;
	}

	Frame frame(proc_); // formerly getActiveStackFrameInfo()
	int pc = frame.getPC();

        unsigned rsize = returnInsts.size();

	//cerr << "checkAndInstallInstrumentation: looping thru all " << rsize << " return instances now" << endl;

        for (unsigned u=0; u<rsize; u++) {

            bool installSafe = returnInsts[u] -> checkReturnInstance(pc); 
	    
            if (installSafe) {
	        //cerr << "installSafe!" << endl;
                returnInsts[u] -> installReturnInstance(proc_);
            } else {
	        //cerr << "install NOT Safe...putting in a TRAP!" << endl;
                frame = frame.getPreviousStackFrameInfo(proc_);// more work here
                pc = frame.getPC();                            // for funcEntry.

		returnInsts[u] -> addToReturnWaitingList(pc, proc_);
            }
        }

        if (needToCont) proc_->continueProc();
    }
    return(true);
}

float metricDefinitionNode::cost() const
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
      for (unsigned u=0; u<instRequests.size(); u++)
        ret += instRequests[u].cost(proc_);
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
      for (unsigned u1=0; u1<instRequests.size(); u1++) {
        unsigVecType pointsForThisRequest = 
            getAllTrampsAtPoint(instRequests[u1].getInstance());
        pointsToCheck += pointsForThisRequest;

        instRequests[u1].disable(pointsForThisRequest); // calls deleteInst()
      }

      for (unsigned u=0; u<dataRequests.size(); u++) {
	unsigned mid = dataRequests[u]->getSampleId();
        dataRequests[u]->disable(proc_, pointsToCheck); // deinstrument
	assert(midToMiMap.defines(mid));
	midToMiMap.undef(mid);
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
      unsigned size = dataRequests.size();
      for (unsigned u=0; u<size; u++)
        delete dataRequests[u];
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

// The following routines (flush_batch_buffer() and batchSampleData() are
// in an inappropriate src file...move somewhere more appropriate)
void flush_batch_buffer() {
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
   assert(copyBatchBuffer.size() <= theBatchBuffer.size());
   for (unsigned i=0; i< batch_buffer_next; i++) {
      copyBatchBuffer[i] = theBatchBuffer[i];
   }

   //char myLogBuffer[120] ;
   //sprintf(myLogBuffer, "in metric.C batch size about to send = %d\n", batch_buffer_next) ;
   //logLine(myLogBuffer) ;

   // Now let's do the actual igen call!

#ifdef FREEDEBUG
timeStamp t1,t2;
t1=getCurrentTime(false);
#endif

   tp->batchSampleDataCallbackFunc(0, copyBatchBuffer);

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

void batchSampleData(int mid, double startTimeStamp,
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
   if (batch_buffer_next >= SAMPLE_BUFFER_SIZE || BURST_HAS_COMPLETED)
      flush_batch_buffer();

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
    assert(start + 0.000001 >= (firstRecordTime/MILLION));
    assert(end >= (firstRecordTime/MILLION));
    assert(end > start);

    batchSampleData(id_, start, end, value, weight, internal_met);
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
        assert(ret.start + 0.000001 >= (firstRecordTime/MILLION));
        assert(ret.end >= (firstRecordTime/MILLION));
	batchSampleData(id_, ret.start, ret.end, ret.value,
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
#ifndef SHM_SAMPLING
void processCost(process *proc, traceHeader *h, costUpdate *s)
{
    timeStamp newSampleTime = (h->wall / 1000000.0);
    timeStamp newProcessTime = (h->process / 1000000.0);

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
}
#endif

#ifndef SHM_SAMPLING
void processSample(traceHeader *h, traceSample *s)
{
    // called from processTraceStream (perfStream.C) when a TR_SAMPLE record
    // has arrived from the appl.
    unsigned mid = s->id.id; // low-level counterId (see primitives.C)

    static long long firstWall = 0;

    static bool firstTime = true;

    if (firstTime) {
       firstWall = h->wall;
    }

    metricDefinitionNode *mi; // filled in by find() if found
    if (!midToMiMap.find(mid, mi)) { // low-level counterId to metricDefinitionNode

#ifdef ndef
       sprintf(errorLine, "Sample %d not for a valid metric instance\n", 
	       s->id.id);
       logLine(errorLine);
#endif

       return;
    }

    //    sprintf(errorLine, "sample id %d at time %8.6f = %f\n", s->id.id, 
    //  ((double) *(int*) &h->wall) + (*(((int*) &h->wall)+1))/1000000.0, s->value);
    //    logLine(errorLine);
    mi->updateValue(h->wall, s->value);
    samplesDelivered++;
}
#endif

/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(instPoint *iPoint,
                         AstNode *iAst,
                         callWhen  iWhen,
                         callOrder o, bool iManuallyTrigger) {
    point = iPoint;
    when = iWhen;
    order = o;
    instance = NULL;
    ast = assignAst(iAst);
    manuallyTrigger = iManuallyTrigger;
    assert(point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parent, process *child) {
    instReqNode ret = instReqNode(parent.point, parent.ast, parent.when,
				  parent.order,
				  false // don't manually trigger
				  );
    assert(child->instInstanceMapping.defines(parent.instance));
    ret.instance = child->instInstanceMapping[parent.instance];
    return ret;
}

bool instReqNode::insertInstrumentation(process *theProc,
					returnInstance *&retInstance) {
    // NEW: We may manually trigger the instrumentation, via a delayed inferiorRPC call
    //      (delayed meaning that the inferiorRPC doesn't take place until the next
    //      call to continueProc()).

    // addInstFunc() is one of the key routines in all paradynd.
    // It installs a base tramp at the point (if needed), generates code
    // for the tramp, calls inferiorMalloc() in the text heap to get space for it,
    // and actually inserts the instrumentation.
    instance = addInstFunc(theProc, point, ast, when, order,
			   false, // false --> don't exclude cost
			   retInstance);
    //cerr << "instReqNode::insertInstrumentation: call to addInstFunc set retInstance to " << (void *)retInstance << endl;

    return (instance != NULL);
}

void instReqNode::disable(const vector<unsigned> &pointsToCheck)
{
    deleteInst(instance, pointsToCheck);
    instance = NULL;
}

instReqNode::~instReqNode()
{
    instance = NULL;
    removeAst(ast);
}

float instReqNode::cost(process *theProc) const
{
    float value;
    float unitCost;
    float frequency;
    int unitCostInCycles;

    unitCostInCycles = ast->cost() + getPointCost(theProc, point) +
                       getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
    // printf("unit cost = %d cycles\n", unitCostInCycles);
    unitCost = unitCostInCycles/ cyclesPerSecond;
    frequency = getPointFrequency(point);
    value = unitCost * frequency;
    return(value);
}

bool instReqNode::triggerNow(process *theProc) {
   assert(manuallyTrigger);

   theProc->postRPCtoDo(ast, false, // don't skip cost
			NULL, // no callback fn needed
			NULL
			);
      // the rpc will be launched with a call to launchRPCifAppropriate()
      // in the main loop (perfStream.C)

   return true;
}

/* ************************************************************************* */

#ifndef SHM_SAMPLING
sampledIntCounterReqNode::sampledIntCounterReqNode(int iValue, int iCounterId) :
                                                  dataReqNode() {
   sampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   counterPtr = NULL;
   sampler = NULL;
}

sampledIntCounterReqNode::sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
						   process *childProc, int iCounterId) {
   // a dup() routine (call after a fork())
   counterPtr = src.counterPtr; // assumes addr spaces have been dup()d.

   // is this right? What if src.sampler is NULL?
   assert(childProc->instInstanceMapping.defines(src.sampler));
   sampler = childProc->instInstanceMapping[src.sampler];

   sampleId = iCounterId;
 
   intCounter temp;
   temp.id.id = this->sampleId;
   temp.value = initialValue;
   writeToInferiorHeap(childProc, temp);
}

dataReqNode *sampledIntCounterReqNode::dup(process *childProc,
					   int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledIntCounterReqNode *tmp;
   tmp = new sampledIntCounterReqNode(*this, childProc, iCounterId);
      // fork ctor
  
   return tmp;
}

bool sampledIntCounterReqNode::insertInstrumentation(process *theProc,
						     metricDefinitionNode *) {
   // Remember counterPtr and sampler are NULL until this routine
   // gets called.
   counterPtr = (intCounter*)inferiorMalloc(theProc, sizeof(intCounter), dataHeap);
   if (counterPtr == NULL)
      return false; // failure!

   // initialize the intCounter in the inferior heap
   intCounter temp;
   temp.id.id = this->sampleId;
   temp.value = this->initialValue;

   writeToInferiorHeap(theProc, temp);

   pdFunction *sampleFunction = theProc->findOneFunction("DYNINSTsampleValues");
   assert(sampleFunction);

   AstNode *ast, *tmp;
   tmp = new AstNode(AstNode::Constant, counterPtr);
   ast = new AstNode("DYNINSTreportCounter", tmp);
   removeAst(tmp);

   sampler = addInstFunc(theProc, sampleFunction->funcEntry(),
			 ast, callPreInsn, orderLastAtPoint, false);
   removeAst(ast);

   return true; // success
}

void sampledIntCounterReqNode::disable(process *theProc,
				       const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   // Remove instrumentation added to DYNINSTsampleValues(), if necessary:
   if (sampler != NULL)
      ::deleteInst(sampler, getAllTrampsAtPoint(sampler));

   // Deallocate space for intCounter in the inferior heap:
   assert(counterPtr != NULL);
   inferiorFree(theProc, (unsigned)counterPtr, dataHeap, pointsToCheck);
}

void sampledIntCounterReqNode::writeToInferiorHeap(process *theProc,
						   const intCounter &dataSrc) const {
   // using the contents of "dataSrc", write to the inferior heap at loc
   // "counterPtr" via proc->writeDataSpace()
   assert(counterPtr);
   theProc->writeDataSpace(counterPtr, sizeof(intCounter), &dataSrc);
}
#endif

/* ************************************************************************* */

#ifdef SHM_SAMPLING

sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(int iValue, int iCounterId) :
                                                  dataReqNode() {
   sampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   allocatedIndex = UINT_MAX;
   inferiorCounterPtr = NULL;
}

sampledShmIntCounterReqNode::
sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
			    process *childProc, int iCounterId) {
   // a dup() routine (call after a fork())
   // WARNING: assumes that "childProc" (and in particular, its
   //          fastInferiorHeap<> member vrbles) have been copied correctly
   //          already.  In particular, childProc should have shared segs
   //          _distinct_ from that of the parent!!!

   // setting the new value of inferiorCounterPtr is a little tricky.
   // We use the following trick: the index is the same, so we just
   // peek at the new base addr and do some pointer arith.  Note that
   // the fastInferiorHeap class's fork ctor will have already copied the
   // actual data...

   this->allocatedIndex = src.allocatedIndex;

   const fastInferiorHeap<intCounterHK, intCounter> &theHeap =
                    childProc->getInferiorIntCounters();

   inferiorCounterPtr = theHeap.index2InferiorAddr(allocatedIndex);

   sampleId = iCounterId;
   initialValue = src.initialValue;

   // WARNING: DON'T WE NEED TO WRITE TO THE INFERIOR HEAP IN ORDER
   // TO GIVE THE NEW RAW ITEM ITS NEW AND DIFFERENT COUNTERID?
}

dataReqNode *sampledShmIntCounterReqNode::dup(process *childProc,
					      int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmIntCounterReqNode *tmp;
   tmp = new sampledShmIntCounterReqNode(*this, childProc, iCounterId);
      // fork ctor

   return tmp;
}

bool sampledShmIntCounterReqNode::insertInstrumentation(process *theProc,
							metricDefinitionNode *iMi) {
   // Remember counterPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the intCounter in the inferior heap
   intCounter iValue;
   iValue.id.id = this->sampleId;
   iValue.value = this->initialValue; // what about initializing 'theSpinner'???

   intCounterHK iHKValue(this->sampleId, iMi);

   fastInferiorHeap<intCounterHK, intCounter> &theShmHeap =
          theProc->getInferiorIntCounters();

   if (!theShmHeap.alloc(iValue, iHKValue, this->allocatedIndex))
      return false; // failure

   inferiorCounterPtr = theShmHeap.getBaseAddrInApplic() + allocatedIndex;
      // ptr arith.  Now we know where in the inferior heap this counter is
      // attached to, so getInferiorPtr() can work ok.

   assert(inferiorCounterPtr == theShmHeap.index2InferiorAddr(allocatedIndex));
      // just a check for fun

   return true; // success
}

void sampledShmIntCounterReqNode::disable(process *theProc,
					  const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   fastInferiorHeap<intCounterHK, intCounter> &theShmHeap =
          theProc->getInferiorIntCounters();

   // Remove from inferior heap; make sure we won't be sampled any more:
   vector<unsigned> trampsMaybeUsing;
   for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
      for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); tramplcv++)
	 trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];

   theShmHeap.makePendingFree(allocatedIndex, trampsMaybeUsing);
}

#endif

/* ************************************************************************* */

nonSampledIntCounterReqNode::nonSampledIntCounterReqNode(int iValue, int iCounterId) :
                                                  dataReqNode() {
   sampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   counterPtr = NULL;
}

nonSampledIntCounterReqNode::
nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &src,
			    process *childProc, int iCounterId) {
   // a dup() routine (call after a fork())
   counterPtr = src.counterPtr; // assumes addr spaces have been dup()d.

   sampleId = iCounterId;
 
   intCounter temp;
   temp.id.id = this->sampleId;
   temp.value = initialValue;
   writeToInferiorHeap(childProc, temp);
}

dataReqNode *nonSampledIntCounterReqNode::dup(process *childProc,
					      int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   nonSampledIntCounterReqNode *tmp;
   tmp = new nonSampledIntCounterReqNode(*this, childProc, iCounterId);
      // fork ctor

   return tmp;
}

bool nonSampledIntCounterReqNode::insertInstrumentation(process *theProc,
							metricDefinitionNode *) {
   // Remember counterPtr is NULL until this routine gets called.
   counterPtr = (intCounter*)inferiorMalloc(theProc, sizeof(intCounter), dataHeap);
   if (counterPtr == NULL)
      return false; // failure!

   // initialize the intCounter in the inferior heap
   intCounter temp;
   temp.id.id = this->sampleId;
   temp.value = this->initialValue;

   writeToInferiorHeap(theProc, temp);

   return true; // success
}

void nonSampledIntCounterReqNode::disable(process *theProc,
					  const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   // Deallocate space for intCounter in the inferior heap:
   assert(counterPtr != NULL);
   inferiorFree(theProc, (unsigned)counterPtr, dataHeap, pointsToCheck);
}

void nonSampledIntCounterReqNode::writeToInferiorHeap(process *theProc,
						      const intCounter &dataSrc) const {
   // using the contents of "dataSrc", write to the inferior heap at loc
   // "counterPtr" via proc->writeDataSpace()
   assert(counterPtr);
   theProc->writeDataSpace(counterPtr, sizeof(intCounter), &dataSrc);
}

/* ****************************************************************** */

#ifndef SHM_SAMPLING
sampledTimerReqNode::sampledTimerReqNode(timerType iType, int iCounterId) :
                                                 dataReqNode() {
   sampleId = iCounterId;
   theTimerType = iType;

   // The following fields are NULL until insertInstrumentatoin():
   timerPtr = NULL;
   sampler  = NULL;
}

sampledTimerReqNode::sampledTimerReqNode(const sampledTimerReqNode &src,
					 process *childProc, int iCounterId) {
   // a dup()-like routine; call after a fork()
   timerPtr = src.timerPtr; // assumes addr spaces have been dup()'d

   // is this right? What if src.sampler is NULL?
   assert(childProc->instInstanceMapping.defines(src.sampler));
   sampler = childProc->instInstanceMapping[src.sampler];
   assert(sampler); // makes sense; timers are always sampled, whereas intCounters
                    // might be just non-sampled predicates.
   
   sampleId = iCounterId;
   theTimerType = src.theTimerType;

   tTimer temp;
   P_memset(&temp, '\0', sizeof(tTimer)); /* is this needed? */
   temp.id.id = this->sampleId;
   temp.type = this->theTimerType;
   temp.normalize = 1000000;
   writeToInferiorHeap(childProc, temp);
}

dataReqNode *sampledTimerReqNode::dup(process *childProc, int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledTimerReqNode *result = new sampledTimerReqNode(*this, childProc, iCounterId);
      // fork ctor
   if (result == NULL)
      return NULL; // on failure, return w/o incrementing counterId

   return result;
}

bool sampledTimerReqNode::insertInstrumentation(process *theProc,
						metricDefinitionNode *) {
   timerPtr = (tTimer *)inferiorMalloc(theProc, sizeof(tTimer), dataHeap);
   if (timerPtr == NULL)
      return false; // failure!

   // Now let's initialize the newly allocated tTimer in the inferior heap:
   tTimer temp;
   P_memset(&temp, '\0', sizeof(tTimer));
   temp.id.id = this->sampleId;
   temp.type = this->theTimerType;
   temp.normalize = 1000000;
   writeToInferiorHeap(theProc, temp);

   // Now instrument DYNINSTreportTimer:
   pdFunction *sampleFunction = theProc->findOneFunction("DYNINSTsampleValues");
   assert(sampleFunction);

   AstNode *ast, *tmp;
   tmp = new AstNode(AstNode::Constant, timerPtr);
   ast = new AstNode("DYNINSTreportTimer", tmp);
   removeAst(tmp);

   sampler = addInstFunc(theProc, sampleFunction->funcEntry(), ast,
			 callPreInsn, orderLastAtPoint, false);
   removeAst(ast);

   return true; // successful
}

void sampledTimerReqNode::disable(process *theProc,
				  const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   // Remove instrumentation added to DYNINSTsampleValues(), if necessary:
   if (sampler != NULL)
      ::deleteInst(sampler, getAllTrampsAtPoint(sampler));

   // Deallocate space for tTimer in the inferior heap:
   assert(timerPtr);
   inferiorFree(theProc, (unsigned)timerPtr, dataHeap, pointsToCheck);
}

void sampledTimerReqNode::writeToInferiorHeap(process *theProc,
					      const tTimer &dataSrc) const {
   // using contents of "dataSrc", a local copy of the data,
   // write to inferior heap at loc "timerPtr" via proc->writeDataSpace()
   assert(timerPtr);
   theProc->writeDataSpace(timerPtr, sizeof(tTimer), &dataSrc);
}
#endif

/* ****************************************************************** */

#ifdef SHM_SAMPLING
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(int iCounterId) :
                                                 dataReqNode() {
   sampleId = iCounterId;

   // The following fields are NULL until insertInstrumentation():
   allocatedIndex = UINT_MAX;
   inferiorTimerPtr = NULL;
}

sampledShmWallTimerReqNode::
sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src,
			   process *childProc, int iCounterId) {
   // a dup()-like routine; call after a fork()
   // WARNING: assumes that "childProc" (and in particular, its
   //          fastInferiorHeap<> member vrbles) have been copied correctly
   //          already.  In particular, childProc should have shared segs
   //          _distinct_ from that of the parent!!!

   // setting the new value of inferiorTimerPtr is a little tricky.
   // We use the following trick: the index is the same, so we just
   // peek at the new base addr and do some pointer arith.  Note that
   // the fastInferiorHeap class's fork ctor will have already copied the
   // actual data...

   this->allocatedIndex = src.allocatedIndex;

   const fastInferiorHeap<wallTimerHK, tTimer> &theHeap =
                    childProc->getInferiorWallTimers();

   inferiorTimerPtr = theHeap.index2InferiorAddr(allocatedIndex);

   sampleId = iCounterId;

   // WARNING: DON'T WE NEED TO WRITE TO THE INFERIOR HEAP IN ORDER
   // TO GIVE THE NEW RAW ITEM ITS NEW AND DIFFERENT COUNTERID?
//   tTimer temp;
//   P_memset(&temp, '\0', sizeof(tTimer)); /* is this needed? */
//   temp.id.id = this->sampleId;
//   temp.type = this->theTimerType;
//   temp.normalize = 1000000;
//   writeToInferiorHeap(childProc, temp);
}

dataReqNode *sampledShmWallTimerReqNode::dup(process *childProc, int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmWallTimerReqNode *tmp;
   tmp = new sampledShmWallTimerReqNode(*this, childProc, iCounterId);
      // fork constructor

   return tmp;
}

bool sampledShmWallTimerReqNode::insertInstrumentation(process *theProc,
						       metricDefinitionNode *iMi) {
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->sampleId;
   iValue.type = wallTime;
   iValue.normalize = 1000000;

   wallTimerHK iHKValue(this->sampleId, iMi, 0);

   fastInferiorHeap<wallTimerHK, tTimer> &theShmHeap =
          theProc->getInferiorWallTimers();

   if (!theShmHeap.alloc(iValue, iHKValue, this->allocatedIndex))
      return false; // failure

   inferiorTimerPtr = theShmHeap.getBaseAddrInApplic() + allocatedIndex;
      // ptr arith.  Now we know where in the inferior heap this counter is
      // attached to, so getInferiorPtr() can work ok.

   assert(inferiorTimerPtr == theShmHeap.index2InferiorAddr(allocatedIndex));
      // just a check for fun

   return true;
}

void sampledShmWallTimerReqNode::disable(process *theProc,
					 const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   fastInferiorHeap<wallTimerHK, tTimer> &theShmHeap =
          theProc->getInferiorWallTimers();

   // Remove from inferior heap; make sure we won't be sampled any more:
   vector<unsigned> trampsMaybeUsing;
   for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
      for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); tramplcv++)
         trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];

   theShmHeap.makePendingFree(allocatedIndex, trampsMaybeUsing);
}

/* ****************************************************************** */

sampledShmProcTimerReqNode::sampledShmProcTimerReqNode(int iCounterId) :
                                                 dataReqNode() {
   sampleId = iCounterId;

   // The following fields are NULL until insertInstrumentatoin():
   allocatedIndex = UINT_MAX;
   inferiorTimerPtr = NULL;
}

sampledShmProcTimerReqNode::
sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src,
			   process *childProc, int iCounterId) {
   // a dup()-like routine; call after a fork()
   // WARNING: assumes that "childProc" (and in particular, its
   //          fastInferiorHeap<> member vrbles) have been copied correctly
   //          already.  In particular, childProc should have shared segs
   //          _distinct_ from that of the parent!!!

   // setting the new value of inferiorTimerPtr is a little tricky.
   // We use the following trick: the index is the same, so we just
   // peek at the new base addr and do some pointer arith.  Note that
   // the fastInferiorHeap class's fork ctor will have already copied the
   // actual data...

   this->allocatedIndex = src.allocatedIndex;

   const fastInferiorHeap<processTimerHK, tTimer> &theHeap =
                    childProc->getInferiorProcessTimers();

   inferiorTimerPtr = theHeap.index2InferiorAddr(allocatedIndex);

   sampleId = iCounterId;

   // WARNING: DON'T WE NEED TO WRITE TO THE INFERIOR HEAP IN ORDER
   // TO GIVE THE NEW RAW ITEM ITS NEW AND DIFFERENT COUNTERID?
//   tTimer temp;
//   P_memset(&temp, '\0', sizeof(tTimer)); /* is this needed? */
//   temp.id.id = this->sampleId;
//   temp.type = this->theTimerType;
//   temp.normalize = 1000000;
//   writeToInferiorHeap(childProc, temp);
}

dataReqNode *sampledShmProcTimerReqNode::dup(process *childProc, int iCounterId) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmProcTimerReqNode *tmp;
   tmp = new sampledShmProcTimerReqNode(*this, childProc, iCounterId);
      // fork constructor

   return tmp;
}

bool sampledShmProcTimerReqNode::insertInstrumentation(process *theProc,
						       metricDefinitionNode *iMi) {
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->sampleId;
   iValue.type = processTime;
   iValue.normalize = 1000000;

   processTimerHK iHKValue(this->sampleId, iMi, 0);

   fastInferiorHeap<processTimerHK, tTimer> &theShmHeap =
          theProc->getInferiorProcessTimers();

   if (!theShmHeap.alloc(iValue, iHKValue, this->allocatedIndex))
      return false; // failure

   inferiorTimerPtr = theShmHeap.getBaseAddrInApplic() + allocatedIndex;
      // ptr arith.  Now we know where in the inferior heap this counter is
      // attached to, so getInferiorPtr() can work ok.

   assert(inferiorTimerPtr == theShmHeap.index2InferiorAddr(allocatedIndex));
      // just a check for fun

   return true;
}

void sampledShmProcTimerReqNode::disable(process *theProc,
					 const vector<unsigVecType> &pointsToCheck) {
   // We used to remove the sample id from midToMiMap here but now the caller is
   // responsible for that.

   fastInferiorHeap<processTimerHK, tTimer> &theShmHeap =
          theProc->getInferiorProcessTimers();

   // Remove from inferior heap; make sure we won't be sampled any more:
   vector<unsigned> trampsMaybeUsing;
   for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
      for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); tramplcv++)
         trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];

   theShmHeap.makePendingFree(allocatedIndex, trampsMaybeUsing);
}
#endif

/* **************************** */

//float dataReqNode::getMetricValue()
//{
//    float ret;
//
//    if (type == INTCOUNTER) {
//        ret = getIntCounterValue((intCounterHandle*) instance);
//    } else if (type == TIMER) {
//        ret = getTimerValue((timerHandle*) instance);
//    } else {
//        // unknown type.
//        abort();
//        return(0.0);
//    }
//    return(ret);
//}

//// allow a global "variable" to be inserted
//// this will not report any values
//// it is used internally by generated code -- see metricDefs-pvm.C
//void dataReqNode::insertGlobal() {
//  if (type == INTCOUNTER) {
//    intCounterHandle *ret = createCounterInstance();
//    if (!ret) return;
//    instance = (void *) ret;
//    id = ret->data.id;
//  } else 
//    abort();
//}

/* ************************************************************************ */

void reportInternalMetrics(bool force) 
{
    if (isApplicationPaused())
       return; // we don't sample when paused (is this right?)

    static timeStamp end=0.0;

    // see if we have a sample to establish time base.
    if (!firstRecordTime) {
       cerr << "reportInternalMetrics: no because firstRecordTime==0" << endl;
       return;
    }

    if (end==0.0)
        end = (timeStamp)firstRecordTime/MILLION;

    const timeStamp now = getCurrentTime(false);

    //  check if it is time for a sample
    if (!force && now < end + samplingRate)  {
//        cerr << "reportInternalMetrics: no because now < end + samplingRate (end=" << end << "; samplingRate=" << samplingRate << "; now=" << now << ")" << endl;
//	cerr << "difference is " << (end+samplingRate-now) << endl;
	return;
    }

    timeStamp start = end;
    end = now;

    // TODO -- clean me up, please

    unsigned ai_size = internalMetric::allInternalMetrics.size();
    for (unsigned u=0; u<ai_size; u++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[u];
      // Loop thru all enabled instances of this internal metric...

      for (unsigned v=0; v < theIMetric->num_enabled_instances(); v++) {
	internalMetric::eachInstance &theInstance = theIMetric->getEnabledInstance(v);
           // not "const" since bumpCumulativeValueBy() may be called

	sampleValue value = 0;
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
