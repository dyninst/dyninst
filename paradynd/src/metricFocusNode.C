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
#include "util/h/debugOstream.h"

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;

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

unsigned componentMdnPtrHash(metricDefinitionNode * const &ptr) {
   // maybe assert that "ptr" isn't for an aggregate mi
   return string::hash(ptr->getFullName());
}


dictionary_hash<unsigned, metricDefinitionNode*> allMIs(uiHash);
dictionary_hash<string, metricDefinitionNode*> allMIComponents(string::hash);
vector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

bool mdl_internal_metric_data(const string& metric_name, mdl_inst_data& result) {
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
metricDefinitionNode::metricDefinitionNode(process *p, const string& met_name, 
				   const vector< vector<string> >& foc,
				   const vector< vector<string> >& component_foc,
                                   const string& component_flat_name, int agg_style)
: aggregate_(false), 
  aggOp(agg_style), // CM5 metrics need aggOp to be set
  inserted_(false), installed_(false), met_(met_name),
  focus_(foc), component_focus(component_foc),
  flat_name_(component_flat_name),
  aggSample(0),
  cumulativeValue(0.0), samples(0),
  id_(-1), originalCost_(0.0), proc_(p)
{
  mdl_inst_data md;
  bool aflag;
  aflag=mdl_internal_metric_data(met_name, md);
  assert(aflag);
  style_ = md.style;
}

// for aggregate metrics
metricDefinitionNode::metricDefinitionNode(const string& metric_name,
                                           const vector< vector<string> >& foc,
                                           const string& cat_name, 
                                           vector<metricDefinitionNode*>& parts,
					   int agg_op)
: aggregate_(true), aggOp(agg_op), inserted_(false),  installed_(false),
  met_(metric_name), focus_(foc),
  flat_name_(cat_name), components(parts),
  aggSample(agg_op),
  id_(-1), originalCost_(0.0), proc_(NULL)
{
  unsigned p_size = parts.size();
  for (unsigned u=0; u<p_size; u++) {
    metricDefinitionNode *mi = parts[u];
    mi->aggregators += this;
    mi->samples += aggSample.newComponent();
  }
}

// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
metricDefinitionNode *doInternalMetric(vector< vector<string> >& canon_focus,
				       vector< vector<string> >& component_canon_focus,
                                       string& metric_name, string& flat_name,
                                       bool enable, bool& matched)
{
  // called by createMetricInstance, below.
  // return values:
  //   a valid metricDefinitionNode* when successful
  //   -1 --> enable was false
  //   -2 --> not legal to instrument this focus
  //   NULL --> a more serious error (probably metric-is-unknown)

  matched = false;
  metricDefinitionNode *mn = 0; 

  // check to see if this is an internal metric
  unsigned im_size = internalMetric::allInternalMetrics.size();
  for (unsigned im_index=0; im_index<im_size; im_index++){
    internalMetric *theIMetric = internalMetric::allInternalMetrics[im_index];
    if (theIMetric->name() == metric_name) {
      matched = true;
      if (!enable)
	 return (metricDefinitionNode*)-1;

      if (!theIMetric->legalToInst(canon_focus))
	 // Paradyn will handle this case and report appropriate error msg
         return (metricDefinitionNode*)-2;

      mn = new metricDefinitionNode(NULL, metric_name, canon_focus,
				    component_canon_focus,
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
	  if (!enable) return (metricDefinitionNode*)-1;
	  costMetric *nc = costMetric::allCostMetrics[i];
	  if (!nc->legalToInst(canon_focus)) return (metricDefinitionNode*)-2;

	  mn = new metricDefinitionNode(NULL, metric_name, canon_focus,
					component_canon_focus,
					flat_name, nc->aggregate());
          assert(mn);

          nc->enable(mn); 

	  return(mn);
     }
  }

  // No matches found among internal or cost metrics
  return NULL;
}

// the following should probably be made a public static member fn of class metric
string metricAndCanonFocus2FlatName(const string &metricName,
				    const vector< vector<string> > &canonFocus) {
   string result = metricName;

   for (unsigned hierarchy=0; hierarchy < canonFocus.size(); hierarchy++)
      for (unsigned component=0; component < canonFocus[hierarchy].size();
	   component++)
	 result += canonFocus[hierarchy][component];

   return result;
}

// the following should probably be made a public static member fn of class metric
static bool focus2CanonicalFocus(const vector<unsigned> &focus,
				 vector< vector<string> > &canonFocus,
				 bool important) {
   // takes in "focus", writes to "canonFocus".  Returns true iff successful.
   // if "important" is false, don't print error msg on failure (called by guessCost();
   // no mi is really being created)

   vector< vector<string> > unCanonFocus;
   if (!resource::foc_to_strings(unCanonFocus, focus, important)) { // writes to unCanonFocus
      if (important)
         cerr << "focus2CanonicalFocus failed since resource::foc_to_strings failed" << endl;
      return false;
   }

   resource::make_canonical(unCanonFocus, canonFocus);

   return true;
}

static void print_focus(debug_ostream &os, vector< vector<string> > &focus) {
   for (unsigned a=0; a < focus.size(); a++) {
      for (unsigned b=0; b < focus[a].size(); b++)
	 os << '/' << focus[a][b];

      if (a < focus.size()-1)
	 os << ',';
   }
   os << endl;
}

metricDefinitionNode *createMetricInstance(string& metric_name, 
                                           vector<u_int>& focus,
                                           bool enable, // true if for real; false for guessCost()
					   bool& internal)
{
    vector< vector<string> > canonicalFocus;
    if (!focus2CanonicalFocus(focus, canonicalFocus, enable)) {
       if (enable) // for real, so an error msg is appropriate
	  cerr << "createMetricInstance failed because focus2CanonicalFocus failed" << endl;

       return NULL;
    }

    string flat_name = metricAndCanonFocus2FlatName(metric_name, canonicalFocus);

    // first see if it is already defined.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi(allMIs);

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

    unsigned key;
    metricDefinitionNode *mi= NULL;

    // TODO -- a dictionary search here will be much faster
    while (mdi.next(key, mi))
      if (mi->getFullName() == flat_name) {
	metric_cerr << "createMetricInstance: mi with flat_name " << flat_name << " already exists! using it" << endl;
        return mi; // this metricDefinitionNode has already been defined
      }

    if (mdl_can_do(metric_name)) {
      internal = false;

      extern vector<process*> processVec;
      mi = mdl_do(canonicalFocus, metric_name, flat_name, processVec, false);
      if (mi == NULL) {
	 metric_cerr << "createMetricInstance failed since mdl_do failed" << endl;
	 metric_cerr << "metric name was " << metric_name << "; focus was ";
	 print_focus(metric_cerr, canonicalFocus);
      }
    } else {
      bool matched;
      mi=doInternalMetric(canonicalFocus,
			  canonicalFocus, // is this right for component_canon_focus???
			  metric_name,flat_name,enable,matched);
         // NULL on serious error; -1 if enable was false; -2 if illegal to instr with
         // given focus [many internal metrics work only for whole program]

      if (mi == (metricDefinitionNode*)-2) {
	 metric_cerr << "createMetricInstance: internal metric " << metric_name << " isn't defined for focus: ";
	 print_focus(metric_cerr, canonicalFocus);
	 mi = NULL; // straighten up the return value
      }
      else if (mi == (metricDefinitionNode*)-1) {
	 assert(!enable); // no error msg needed
	 mi = NULL; // straighten up the return value
      }
      else if (mi == NULL) {
	 // more serious error...do a printout
         metric_cerr << "createMetricInstance failed since doInternalMetric failed" << endl;
	 metric_cerr << "metric name was " << metric_name << "; focus was ";
	 print_focus(metric_cerr, canonicalFocus);
      }

      internal = true;
    }

    return mi;
}


// propagate this metric instance to process p.
// p is a process that started after the metric instance was created
// note: don't call this routine for a process started via fork or exec, just
// for processes started the "normal" way.
// "this" is an aggregate mi, not a component one.

void metricDefinitionNode::propagateToNewProcess(process *p) {
  unsigned comp_size = components.size();

  if (comp_size == 0)
    return; // if there are no components, shouldn't the mi be fried?

  for (unsigned u = 0; u < comp_size; u++) {
    if (components[u]->proc() == p) {
      // The metric is already enabled for this process. This case can 
      // happen when we are starting several processes at the same time.
      // (explain...?)
      return;
    }
  }

  bool internal = false;

  metricDefinitionNode *mi = NULL;
     // an aggregate (not component) mi, though we know that it'll contain just
     // one component.  It's that one component that we're really interested in.
  if (mdl_can_do(met_)) {
      // Make the unique ID for this metric/focus visible in MDL.
      string vname = "$globalId";
      mdl_env::add(vname, false, MDL_T_INT);
      mdl_env::set(this->getMId(), vname);

      vector<process *> vp(1,p);
      mi = mdl_do(focus_, met_, flat_name_, vp, false);
  } else {
    // internal and cost metrics don't need to be propagated (um, is this correct?)
    mi = NULL;
  }

  if (mi) { // successfully created new mi
    assert(mi->components.size() == 1);
    metricDefinitionNode *theNewComponent = mi->components[0];

    components += theNewComponent;
    theNewComponent->aggregators[0] = this;
    theNewComponent->samples[0] = aggSample.newComponent();
    if (!internal) {
      theNewComponent->insertInstrumentation();
      theNewComponent->checkAndInstallInstrumentation();
    }

    // update cost
    const float cost = mi->cost();
    if (cost > originalCost_) {
      currentPredictedCost += cost - originalCost_;
      originalCost_ = cost;
    }

    mi->components.resize(0); // protect the new component
    delete mi;
  }
}

metricDefinitionNode* metricDefinitionNode::handleExec() {
   // called by handleExec(), below.  See that routine for documentation.
   // "this" is a component mi.

   // If this component mi can be (re-)enabled in the new (post-exec) process, then do
   // so.  Else, remove the component mi from aggregators, etc.  Returns new component
   // mi if successful, NULL otherwise.

   assert(!aggregate_);

   // How can we tell if the mi can be inserted into the "new" (post-exec) process?
   // A component mi is basically a set of instReqNodes and dataReqNodes.  The latter
   // don't restrict what can be inserted (is this right?); the instReqNodes hold the
   // key -- we should look at the functions (instPoint's) where code (whose contents
   // are in AstNode's) would be inserted.  Now clearly, the instPoint's must be
   // checked -- if any one doesn't exist, then the instReqNode and hence the component
   // mi doesn't belong in the post-exec process.  But what about the AstNode's?
   // Should the code that gets inserted be subject to a similar test?  Probably, but
   // we currently don't do it.

   // BUT: Even if a process contains a function in both the pre-exec and post-exec
   // stages, we must assume that the function is IN A DIFFERENT LOCATION IN
   // THE ADDRESS SPACE.  Ick.  So the instPoint's can't be trusted and must
   // be recalculated from scratch.  In that regard, this routine is similar to
   // propagateToNewProcess(), which propagates aggregate mi's to a brand new
   // process (but which doesn't work for processes started via fork or exec).
   // The lesson learned is to (ick, ick, ick) call mdl_do() all over again.
   // This gets really confusing when you consider that a component mi can belong
   // to several aggregate mi's (e.g. if we represent cpu time for proc 100 then
   // we can belong to cpu/whole and cpu/proc-100); for which aggregate mi should
   // we run mdl_do?  Any will do, so we can pick arbitrarily (is this right?).

   // QUESTION: What about internal or cost metrics???  They have aggregate and
   //           component mi's just like normal metrics, right?  If that's so, then
   //           they must be propagated too!   NOT YET IMPLEMENTED!!!

   metricDefinitionNode *aggregateMI = this->aggregators[0];
   metricDefinitionNode *resultCompMI = NULL; // so far...

   const bool internal = !mdl_can_do(aggregateMI->met_);
   if (internal)
      return NULL; // NOT YET IMPLEMENTED

   // try to propagate the mi
   // note: the following code is mostly stolen from propagateToNewProcess(); blame
   //       it for any bugs :)

   // Make the unique ID for this metric/focus visible in MDL. (?)
   string vname = "$globalId";
   mdl_env::add(vname, false, MDL_T_INT);
   mdl_env::set(aggregateMI->getMId(), vname);

   vector<process*> vp(1, this->proc());
   metricDefinitionNode *tempAggMI = mdl_do(aggregateMI->focus_,
					    aggregateMI->met_,
					    aggregateMI->flat_name_,
					    vp,
					    true // --> fry existing component MI
					    );
   if (tempAggMI == NULL)
      return NULL; // failure

   assert(tempAggMI->aggregate_);

   // okay, it looks like we successfully created a new aggregate mi.
   // Of course, we're just interested in the (single) component mi contained
   // within it; it'll replace "this".

   assert(tempAggMI->components.size() == 1);
   resultCompMI = tempAggMI->components[0];

   resultCompMI->aggregators.resize(0);
   resultCompMI->samples.resize(0);

   // For each aggregator, go back and find where "this" was a component mi.
   // When found, replace the ptr to "this" with "theNewComponent".
   unsigned num_aggregators = aggregators.size();
   assert(num_aggregators > 0);
   for (unsigned agglcv=0; agglcv < num_aggregators; agglcv++) {
      metricDefinitionNode *aggMI = aggregators[agglcv];

      bool found=false;
      for (unsigned complcv=0; complcv < aggMI->components.size(); complcv++) {
	 if (aggMI->components[complcv] == this) {
	    aggMI->components[complcv] = resultCompMI;

	    resultCompMI->aggregators += aggMI;
	    resultCompMI->samples     += aggMI->aggSample.newComponent();
	    
	    aggMI->aggSample.removeComponent(this->samples[agglcv]);
	    
	    found=true;
	    break;
	 }
      }
      assert(found);
   }

   // Now let's actually insert the instrumentation:
   if (!internal) {
      resultCompMI->insertInstrumentation();
      resultCompMI->checkAndInstallInstrumentation();
   }

   // And fry "tempAggMI", but make sure "resultCompMI" isn't fried when we do so
   tempAggMI->components.resize(0); // protect resultCompMI
   delete tempAggMI; // good riddance; you were an ugly hack to begin with

   return resultCompMI;
}

void metricDefinitionNode::handleExec(process *proc) {
   // a static member fn.
   // handling exec is tricky.  At the time this routine is called, the "new" process
   // has been bootstrapped and is ready for stuff to get inserted.  No mi's have yet
   // been propagated, and the data structures (allMIs, allMIComponents, etc.) are still
   // in their old, pre-exec state, so they show component mi's enabled for this
   // process, even though they're not (at least not yet).  This routines brings things
   // up-to-date.
   //
   // Algorithm: loop thru all component mi's for this process.  If it is possible to
   // propagate it to the "new" (post-exec) process, then do so.  If not, fry the
   // component mi.  An example where a component mi can no longer fit is an mi
   // specific to, say, function foo(), which (thanks to the exec syscall) no longer
   // exists in this process.  Note that the exec syscall changed the addr space enough
   // so even if a given routine foo() is present in both the pre-exec and post-exec
   // process, we must assume that it has MOVED TO A NEW LOCATION, thus making
   // the component mi's instReqNode's instPoint out-of-date.  Ick.

   vector<metricDefinitionNode*> miComponents = allMIComponents.values();
   for (unsigned lcv=0; lcv < miComponents.size(); lcv++) {
      metricDefinitionNode *componentMI = miComponents[lcv];
      if (componentMI->proc() != proc)
	 continue;

      forkexec_cerr << "calling handleExec for component "
	            << componentMI->flat_name_ << endl;

      metricDefinitionNode *replaceWithComponentMI = componentMI->handleExec();

      if (replaceWithComponentMI == NULL) {
	 forkexec_cerr << "handleExec for component " << componentMI->flat_name_
	               << " failed, so not propagating it" << endl;
         componentMI->removeThisInstance(); // propagation failed; fry component mi
      }
      else {
	 forkexec_cerr << "handleExec for component " << componentMI->flat_name_
	               << " succeeded...it has been propagated" << endl;
	 // new component mi has already been inserted in place of old component mi
	 // in all of its aggregate's component lists.  So, not much left to do,
	 // except to update allMIComponents.

	 assert(replaceWithComponentMI->flat_name_ == componentMI->flat_name_);

	 delete componentMI; // old component mi (dtor removes it from allMIComponents)
	 assert(!allMIComponents.defines(replaceWithComponentMI->flat_name_));
	 allMIComponents[replaceWithComponentMI->flat_name_] = replaceWithComponentMI;
      }
   }
}

// called when all components have been removed (because the processes have exited
// or exec'd) from "this".  "this" is an aggregate mi.
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
// "this" is an aggregate mi; "comp" is a component mi.
void metricDefinitionNode::removeFromAggregate(metricDefinitionNode *comp) {
  unsigned size = components.size();
  for (unsigned u = 0; u < size; u++) {
    if (components[u] == comp) {
      delete components[u];
      components[u] = NULL;
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

// remove this component mi from all aggregators it is a component of.
// if the aggregate mi no longer has any components then fry the mi aggregate mi.
// called by removeFromMetricInstances, below, when a process exits (or exec's)
void metricDefinitionNode::removeThisInstance() {
  assert(!aggregate_);

  // first, remove from allMIComponents (this is new --- is it right?)
  assert(allMIComponents.defines(flat_name_));
  allMIComponents.undef(flat_name_);

  assert(aggregators.size() == samples.size());
  unsigned aggr_size = aggregators.size();
  assert(aggr_size > 0);

  for (unsigned u = 0; u < aggr_size; u++) {
    aggregators[u]->aggSample.removeComponent(samples[u]);
    aggregators[u]->removeFromAggregate(this); 
  }
}


// Called when a process exits, to remove the component associated to proc 
// from all metric instances.  (If, after an exec, we never want to carry over
// mi's from the pre-exec, then this routine will work there, too.  But we try to
// carry over mi's whenever appropriate.)
// Remove the aggregate metric instances that don't have any components left
void removeFromMetricInstances(process *proc) {
    // Loop through all of the _component_ mi's; for each with component process
    // of "proc", remove the component mi from its aggregate mi.
    // Note: imho, there should be a *per-process* vector of mi-components.

    vector<metricDefinitionNode *> MIs = allMIComponents.values();
    for (unsigned j = 0; j < MIs.size(); j++) {
      if (MIs[j]->proc() == proc)
	MIs[j]->removeThisInstance();
    }
    costMetric::removeProcessFromAll(proc); // what about internal metrics?
}

/* *************************************************************************** */

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

/* *************************************************************************** */

// called when a process forks (by handleFork(), below). "this" is a (component)
// mi in the parent process. Duplicate it for the child, with appropriate
// changes (i.e. the pid of the component focus name differs), and return the newly
// created child mi.  "map" maps all instInstance's of the parent to those copied into
// the child.
// 
// Note how beautifully everything falls into place.  Consider the case of alarm
// sampling with cpu/whole program.  Then comes the fork.  The parent process has
// (0) a tTimer structure allocated in a specific location in the inferior heap,
// (1) instrumentation @ main to call startTimer on that ptr, (2) instrumentation in
// DYNINSTsampleValues() to call DYNINSTreportTimer on that ptr.
// The child process of fork will have ALL of these things in the exact same locations,
// which is correct.  We want the timer to be located in the same spot; we want
// DYNINSTreportTimer to be called on the same pointer; and main() hasn't moved.
//
// So there's not much to do here.  We create a new component mi (with same flat name
// as in the parent, except for a different pid), and call "forkProcess" for all
// dataReqNodes and instReqNodes, none of which have to do anything titanic.

metricDefinitionNode *metricDefinitionNode::forkProcess(process *child,
			const dictionary_hash<instInstance*,instInstance*> &map) const {
    // The "focus_" member vrble stays the same, because it was always for the
    // metric as a whole, and not for some component.
    //
    // But two things must change, because they were component-specific (and the
    // component has changed processes):
    // (1) the flat name
    // (2) the component focus (not to be confused with plain focus_)
    //
    // For example, instead of
    // "/Code/foo.c/myfunc, /Process/100, ...", we should have
    // "/Code/foo.c/myfunc, /Process/101, ...", because the pid of the child
    // differs from that of the parent.

    // The resource structure of a given process is found in the "rid"
    // field of class process.
    const resource *parentResource = child->getParent()->rid;
    const string &parentPartName = parentResource->part_name();

    const resource *childResource = child->rid;
    const string &childPartName = childResource->part_name();

    vector< vector<string> > newComponentFocus = this->component_focus;
       // we'll change the process, but not the machine name.
    bool foundProcess = false;

    for (unsigned hier=0; hier < component_focus.size(); hier++) {
       if (component_focus[hier][0] == "Process") {
	  foundProcess = true;
	  assert(component_focus[hier].size() == 2);
	     // since a component focus is by definition specific to some process

	  assert(component_focus[hier][1] == parentPartName);

	  // change the process:
	  newComponentFocus[hier][1] = childPartName;
	  break;
       }
    }
    assert(foundProcess);
    
    string newComponentFlatName = metricAndCanonFocus2FlatName(met_, newComponentFocus);

    metricDefinitionNode *mi =
        new metricDefinitionNode(child,
			 met_, // metric name doesn't change
			 focus_, // focus doesn't change (tho component focus will)
			 newComponentFocus, // this is a change
			 newComponentFlatName, // this is a change
			 aggOp // no change
			 );
    assert(mi);

    forkexec_cerr << "metricDefinitionNode::forkProcess -- component flat name for parent is " << flat_name_ << "; for child is " << mi->flat_name_ << endl;

    assert(!allMIComponents.defines(newComponentFlatName));
    allMIComponents[newComponentFlatName] = mi;

    // Duplicate the dataReqNodes:
    for (unsigned u = 0; u < dataRequests.size(); u++) {
       // must add to midToMiMap[] before dup() to avoid some assert fails
       const int newCounterId = metricDefinitionNode::counterId++;
          // no relation to mi->getMId();
       forkexec_cerr << "forked dataReqNode going into midToMiMap with id " << newCounterId << endl;
       assert(!midToMiMap.defines(newCounterId));
       midToMiMap[newCounterId] = mi;
       
       dataReqNode *newNode = dataRequests[u]->dup(child, mi, newCounterId, map);
         // remember, dup() is a virtual fn, so the right dup() and hence the
         // right fork-ctor is called.
       assert(newNode);

       mi->dataRequests += newNode;
    }

    // Duplicate the instReqNodes:
    for (unsigned u = 0; u < instRequests.size(); u++) {
      mi->instRequests += instReqNode::forkProcess(instRequests[u], map);
    }

    mi->inserted_ = true;

    return mi;
}

bool metricDefinitionNode::unFork(dictionary_hash<instInstance*, instInstance*> &map,
				  bool unForkInstRequests,
				  bool unForkDataRequests) {
   // see below handleFork() for explanation of why this routine is needed.
   // "this" is a component mi for the parent process; we need to remove copied
   // instrumentation from the _child_ process.
   // Returns true iff the instrumentation was removed in the child (would be false
   // if it's not safe to remove the instrumentation in the child because it was
   // active.)

   // "map" maps instInstances from the parent process to instInstances in the child
   // process.

   // We loop thru the instReqNodes of the parent process, unforking each.
   // In addition, we need to unfork the dataReqNodes, because the alarm-sampled
   // ones instrument DYNINSTsampleValues.

   bool result = true;

   if (unForkInstRequests)
      for (unsigned lcv=0; lcv < instRequests.size(); lcv++)
         if (!instRequests[lcv].unFork(map))
	    result = false; // failure

   if (unForkDataRequests)
      for (unsigned lcv=0; lcv < dataRequests.size(); lcv++)
         if (!dataRequests[lcv]->unFork(map))
	    result = false; // failure

   return result;
}


// called by forkProcess of context.C, just after the fork-constructor was
// called for the child process.
void metricDefinitionNode::handleFork(const process *parent, process *child,
			      dictionary_hash<instInstance*,instInstance*> &map) {
   // "map" defines a mapping from all instInstance's of the parent process to
   // the copied one in the child process.  Some of the child process's ones may
   // get fried by this routine, as it detects that instrumentation has been copied
   // (by the fork syscall, which we have no control over) which doesn't belong in
   // the child process and therefore gets deleted manually.
  
   // Remember that a given component can be shared by multiple aggregator-mi's,
   // so be careful about duplicating a component twice.  Since we loop through
   // component mi's instead of aggregate mi's, it's no problem.  Note that it's
   // possible that only a subset of a component-mi's aggregators should get the newly
   // created child component mi.

   vector<metricDefinitionNode *> allComponents = allMIComponents.values();
   for (unsigned complcv=0; complcv < allComponents.size(); complcv++) {
      metricDefinitionNode *comp = allComponents[complcv];

      // duplicate the component (create a new one) if it belongs in the
      // child process.  It belongs if any of its aggregate mi's should be
      // propagated to the child process.  An aggregate mi should be propagated
      // if it wasn't refined to some process.

      bool shouldBePropagated = false; // so far
      bool shouldBeUnforkedIfNotPropagated = false; // so far
      assert(comp->aggregators.size() > 0);
      for (unsigned agglcv=0; agglcv < comp->aggregators.size(); agglcv++) {
	 metricDefinitionNode *aggMI = comp->aggregators[agglcv];

	 if (aggMI->focus_[resource::process].size() == 1) {
	    // wasn't specific to any process
	    shouldBeUnforkedIfNotPropagated = false; // we'll definitely be using it
	    shouldBePropagated = true;
	    break;
	 }
	 else if (comp->proc() == parent)
	    // was specific to parent process, so fork() copied it into the child,
	    // unless it was an internal or cost metric, in which case there was nothing
	    // for fork to copy.
	    if (!internalMetric::isInternalMetric(aggMI->getMetName()) &&
		!costMetric::isCostMetric(aggMI->getMetName()))
	       shouldBeUnforkedIfNotPropagated = true;
	 else
	    // was specific to other process, so nothing is in the child for it yet
	    ;
      }

      if (!shouldBePropagated && shouldBeUnforkedIfNotPropagated) {
	 // this component mi isn't gonna be propagated to the child process, but
	 // the fork syscall left some residue in the child.  Delete that residue now.
	 assert(comp->proc() == parent);
	 comp->unFork(map, true, true); // also modifies 'map' to remove items
      }

      if (!shouldBePropagated)
	 continue;

      // Okay, it's time to propagate this component mi to the subset of its aggregate
      // mi's which weren't refined to a specific process.  If we've gotten to this
      // point, then there _is_ at least one such aggregate.
      assert(shouldBePropagated);
      metricDefinitionNode *newComp = comp->forkProcess(child, map);
         // copies instr (well, fork() does this for us), allocs ctr/timer space,
         // initializes.  Basically, copies dataReqNode's and instReqNode's.

      bool foundAgg = false;
      for (unsigned agglcv=0; agglcv < comp->aggregators.size(); agglcv++) {
	 metricDefinitionNode *aggMI = comp->aggregators[agglcv];
	 if (aggMI->focus_[resource::process].size() == 1) {
	    // this aggregate mi wasn't specific to any process, so it gets the new
	    // child component.
	    aggMI->components += newComp;
	    newComp->aggregators += aggMI;
	    newComp->samples     += aggMI->aggSample.newComponent();
	    foundAgg = true;
	 }
      }
      assert(foundAgg);
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
    if (!mi) {
       cerr << "startCollecting failed because createMetricInstance failed" << endl;
       return(-1);
    }

    mi->id_ = id;

    assert(!allMIs.defines(mi->id_));
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

float guessCost(string& metric_name, vector<u_int>& focus) {
   // called by dynrpc.C (getPredictedDataCost())
    bool internal;
    metricDefinitionNode *mi = createMetricInstance(metric_name, focus, false, internal);
    if (!mi) {
       //metric_cerr << "guessCost returning 0.0 since createMetricInstance failed" << endl;
       return(0.0);
    }

    float cost = mi->cost();
    // delete the metric instance, if it is not being used 
    if (!allMIs.defines(mi->getMId()))
      delete mi;

    return(cost);
}

bool metricDefinitionNode::insertInstrumentation()
{
    // returns true iff successful
    if (inserted_)
       return true;

    inserted_ = true;

    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++)
          if (!components[u]->insertInstrumentation())
	     return false; // shouldn't we try to undo what's already put in?
    } else {
      bool needToCont = proc_->status() == running;
      bool res = proc_->pause();
      if (!res)
	return false;

      // Loop thru "dataRequests", an array of (ptrs to) dataReqNode:
      // Here we allocate ctrs/timers in the inferior heap but don't
      // stick in any code, except (if appropriate) that we'll instrument the
      // application's alarm-handler when not shm sampling.
      unsigned size = dataRequests.size();
      for (unsigned u=0; u<size; u++) {
	// the following allocs an object in inferior heap and arranges for
        // it to be alarm sampled, if appropriate.
        if (!dataRequests[u]->insertInstrumentation(proc_, this))
           return false; // shouldn't we try to undo what's already put in?

	unsigned mid = dataRequests[u]->getSampleId();
	assert(!midToMiMap.defines(mid));
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

	  if (retInst)
	    returnInsts += retInst;
      }

      if (needToCont)
	 proc_->continueProc();
    }

    return(true);
}

bool metricDefinitionNode::checkAndInstallInstrumentation() {
    bool needToCont = false;

    if (installed_) return(true);

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

	vector<Address> pc = proc_->walkStack();

	// for(u_int i=0; i < pc.size(); i++){
	//     printf("frame %d: pc = 0x%x\n",i,pc[i]);
	// }

        unsigned rsize = returnInsts.size();
	u_int max_index = 0;  // first frame where it is safe to install instr
	bool delay_install = false; // true if some instr. needs to be delayed 
	vector<bool> delay_elm(rsize); // wch instr. to delay
        // for each inst point walk the stack to determine if it can be
	// inserted now (it can if it is not currently on the stack)
	// If some can not be inserted, then find the first safe point on
	// the stack where all can be inserted, and set a break point  
        for (unsigned u=0; u<rsize; u++) {
            u_int index = 0;
            bool installSafe = returnInsts[u] -> checkReturnInstance(pc,index);
	    if ((!installSafe) && (index > max_index)) max_index = index;
	    
            if (installSafe) {
	        //cerr << "installSafe!" << endl;
                returnInsts[u] -> installReturnInstance(proc_);
		delay_elm[u] = false;
            } else {
		delay_install = true;
		delay_elm[u] = true;
            }
        }
	if(delay_install){
	    // get rid of pathological cases...caused by threaded applications 
	    // TODO: this should be fixed to do something smarter
	    if((max_index > 0) && ((max_index+1) >= pc.size())){
	       max_index--;
	       //printf("max_index changed: %d\n",max_index);
	    }
	    if((max_index > 0) && (pc[max_index+1] == 0)){
	       max_index--;
	       //printf("max_index changed: %d\n",max_index);
	    }
	    Address pc2 = pc[max_index+1];
	    for(u_int i=0; i < rsize; i++){
		if(delay_elm[i]){
                    returnInsts[i]->addToReturnWaitingList(pc2, proc_);
		}
	    }
	}

        if (needToCont) proc_->continueProc();
    }
    return(true);
}

float metricDefinitionNode::cost() const
{
    float ret = 0.0;
    if (aggregate_) {
        unsigned c_size = components.size();
        for (unsigned u=0; u<c_size; u++) {
          float nc = components[u]->cost();
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


// NOTE: This stuff (flush_batch_buffer() and batchSampleData()) belongs
//       in perfStream.C; this is an inappropriate file.

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

#ifdef FREEDEBUG
timeStamp t1,t2;
t1=getCurrentTime(false);
#endif

   // Now let's do the actual igen call!
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
    timeStamp sampleTime = wallTime / 1000000.0;
       // note: we can probably do integer division by million quicker

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
    // currently called (only) by the above routine
    sampleInterval ret = aggSample.aggregateValues();
    if (ret.valid) {
        assert(ret.end > ret.start);
        assert(ret.start + 0.000001 >= (firstRecordTime/MILLION));
        assert(ret.end >= (firstRecordTime/MILLION));
	batchSampleData(id_, ret.start, ret.end, ret.value,
			aggSample.numComponents(),false);
    }
//    else {
//        metric_cerr << "sorry, ret.valid false so not batching sample data" << endl;
//    }
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
    // we can probably do integer division by million quicker.
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
void processSample(int pid, traceHeader *h, traceSample *s)
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
       metric_cerr << "TR_SAMPLE id " << s->id.id << " not for valid mi...discarding" << endl;
       return;
    }

//    metric_cerr << "FROM pid " << pid << " got value " << s->value << " for id " << s->id.id << endl;

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
    instance = NULL; // set when insertInstrumentation() calls addInstFunc()
    ast = assignAst(iAst);
    manuallyTrigger = iManuallyTrigger;
    assert(point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parentNode,
			     const dictionary_hash<instInstance*,instInstance*> &map) {
    instReqNode ret = instReqNode(parentNode.point, parentNode.ast, parentNode.when,
				  parentNode.order,
				  false // don't manually trigger
				  );

    if (!map.find(parentNode.instance, ret.instance)) // writes to ret.instance
       assert(false);

    return ret;
}

bool instReqNode::unFork(dictionary_hash<instInstance*,instInstance*> &map) const {
   // The fork syscall duplicates all trampolines from the parent into the child. For
   // those mi's which we don't want to propagate to the child, this creates a
   // problem.  We need to remove instrumentation code from the child.  This routine
   // does that.
   //
   // "this" represents an instReqNode in the PARENT process.
   // "map" maps all instInstance*'s of the parent process to instInstance*'s in the
   // child process.  We modify "map" by setting a value to NULL.

   instInstance *parentInstance = getInstance();
   
   instInstance *childInstance;
   if (!map.find(parentInstance, childInstance)) // writes to childInstance
      assert(false);

   vector<unsigned> pointsToCheck; // is it right leaving this empty on a fork()???
   deleteInst(childInstance, pointsToCheck);

   map[parentInstance] = NULL; // since we've deleted...

   return true; // success
}

bool instReqNode::insertInstrumentation(process *theProc,
					returnInstance *&retInstance) {
    // NEW: We may manually trigger the instrumentation, via a call to postRPCtoDo()

    // addInstFunc() is one of the key routines in all paradynd.
    // It installs a base tramp at the point (if needed), generates code
    // for the tramp, calls inferiorMalloc() in the text heap to get space for it,
    // and actually inserts the instrumentation.
    instance = addInstFunc(theProc, point, ast, when, order,
			   false, // false --> don't exclude cost
			   retInstance);

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
   theSampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   counterPtr = NULL;
   sampler = NULL;
}

sampledIntCounterReqNode::sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
						   process *childProc,
						   metricDefinitionNode *,
						   int iCounterId,
						   const dictionary_hash<instInstance*,instInstance*> &map) {
   // a dup() routine (call after a fork())
   counterPtr = src.counterPtr; // assumes addr spaces have been dup()d.

   if (!map.find(src.sampler, this->sampler)) // writes to this->sampler
      assert(false);

   theSampleId = iCounterId;
 
   intCounter temp;
   temp.id.id = this->theSampleId;
   temp.value = initialValue;
   writeToInferiorHeap(childProc, temp);
}

dataReqNode *
sampledIntCounterReqNode::dup(process *childProc,
			      metricDefinitionNode *mi,
			      int iCounterId,
			      const dictionary_hash<instInstance*,instInstance*> &map
			      ) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new sampledIntCounterReqNode(*this, childProc, mi, iCounterId, map);
      // fork ctor
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
   temp.id.id = this->theSampleId;
   temp.value = this->initialValue;

   writeToInferiorHeap(theProc, temp);

   pdFunction *sampleFunction = theProc->findOneFunction("DYNINSTsampleValues");
   assert(sampleFunction);

   AstNode *ast, *tmp;
   tmp = new AstNode(AstNode::Constant, counterPtr);
   ast = new AstNode("DYNINSTreportCounter", tmp);
   removeAst(tmp);

   const instPoint *func_entry = sampleFunction->funcEntry(theProc);
   sampler = addInstFunc(theProc, func_entry,
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

bool sampledIntCounterReqNode::
unFork(dictionary_hash<instInstance*,instInstance*> &map) {
   instInstance *parentSamplerInstance = this->sampler;

   instInstance *childSamplerInstance;
   if (!map.find(parentSamplerInstance, childSamplerInstance))
      assert(false);

   vector<unsigned> pointsToCheck; // empty on purpose
   deleteInst(childSamplerInstance, pointsToCheck);

   map[parentSamplerInstance] = NULL;

   return true;
}
				      
#endif

/* ************************************************************************* */

#ifdef SHM_SAMPLING

sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(int iValue, int iCounterId) :
                                                  dataReqNode() {
   theSampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   allocatedIndex = UINT_MAX;
   inferiorCounterPtr = NULL;
}

sampledShmIntCounterReqNode::
sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
			    process *childProc, metricDefinitionNode *mi,
			    int iCounterId) {
   // a dup() routine (call after a fork())
   // Assumes that "childProc" has been copied already (e.g., the shm seg was copied).

   // Note that the index w/in the inferior heap remains the same, so setting the
   // new inferiorCounterPtr isn't too hard.  Actually, it's trivial, since other code
   // ensures that the new shm segment is placed in exactly the same virtual mem location
   // as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already copied the
   // actual data; we need to fill in new meta-data (new houseKeeping entries).

   this->allocatedIndex = src.allocatedIndex;

   this->theSampleId = iCounterId;  // this is different from the parent's value
   this->initialValue = src.initialValue;

   fastInferiorHeap<intCounterHK, intCounter> &theHeap =
                    childProc->getInferiorIntCounters();

   // since the new shm seg is placed in exactly the same memory location as the old
   // one, nothing here should change.
   intCounter *oldInferiorCounterPtr = src.inferiorCounterPtr;
   inferiorCounterPtr = theHeap.index2InferiorAddr(allocatedIndex);
   assert(inferiorCounterPtr == oldInferiorCounterPtr);

   // write to the raw item in the inferior heap:
   intCounter *localCounterPtr = theHeap.index2LocalAddr(allocatedIndex);
   const intCounter *localSrcCounterPtr = childProc->getParent()->getInferiorIntCounters().index2LocalAddr(allocatedIndex);
   localCounterPtr->value = initialValue;
   localCounterPtr->id.id = theSampleId;
   localCounterPtr->theSpinner = localSrcCounterPtr->theSpinner;
      // in case we're in the middle of an operation

   // write HK for this intCounter:
   // Note: we don't assert anything about mi->getMId(), because that id has no
   // relation to the ids we work with (theSampleId).  In fact, we (the sampling code)
   // just don't ever care what mi->getMId() is.
   assert(theSampleId >= 0);
   assert(midToMiMap.defines(theSampleId));
   assert(midToMiMap[theSampleId] == mi);
   intCounterHK iHKValue(theSampleId, mi);
      // the mi differs from the mi of the parent; theSampleId differs too.
   theHeap.initializeHKAfterFork(allocatedIndex, iHKValue);
}

dataReqNode *
sampledShmIntCounterReqNode::dup(process *childProc,
				 metricDefinitionNode *mi,
				 int iCounterId,
				 const dictionary_hash<instInstance*,instInstance*> &
				 ) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new sampledShmIntCounterReqNode(*this, childProc, mi, iCounterId);
      // fork ctor
}

bool sampledShmIntCounterReqNode::insertInstrumentation(process *theProc,
							metricDefinitionNode *iMi) {
   // Remember counterPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the intCounter in the inferior heap
   intCounter iValue;
   iValue.id.id = this->theSampleId;
   iValue.value = this->initialValue; // what about initializing 'theSpinner'???

   intCounterHK iHKValue(this->theSampleId, iMi);

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
   theSampleId = iCounterId;
   initialValue = iValue;

   // The following fields are NULL until insertInstrumentation()
   counterPtr = NULL;
}

nonSampledIntCounterReqNode::
nonSampledIntCounterReqNode(const nonSampledIntCounterReqNode &src,
			    process *childProc, metricDefinitionNode *,
			    int iCounterId) {
   // a dup() routine (call after a fork())
   counterPtr = src.counterPtr; // assumes addr spaces have been dup()d.
   initialValue = src.initialValue;
   theSampleId = iCounterId;
 
   intCounter temp;
   temp.id.id = this->theSampleId;
   temp.value = this->initialValue;
   writeToInferiorHeap(childProc, temp);
}

dataReqNode *
nonSampledIntCounterReqNode::dup(process *childProc,
				 metricDefinitionNode *mi,
				 int iCounterId,
				 const dictionary_hash<instInstance*,instInstance*> &
				 ) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new nonSampledIntCounterReqNode(*this, childProc, mi, iCounterId);
      // fork ctor
}

bool nonSampledIntCounterReqNode::insertInstrumentation(process *theProc,
							metricDefinitionNode *) {
   // Remember counterPtr is NULL until this routine gets called.
   counterPtr = (intCounter*)inferiorMalloc(theProc, sizeof(intCounter), dataHeap);
   if (counterPtr == NULL)
      return false; // failure!

   // initialize the intCounter in the inferior heap
   intCounter temp;
   temp.id.id = this->theSampleId;
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
   theSampleId = iCounterId;
   theTimerType = iType;

   // The following fields are NULL until insertInstrumentatoin():
   timerPtr = NULL;
   sampler  = NULL;
}

sampledTimerReqNode::sampledTimerReqNode(const sampledTimerReqNode &src,
					 process *childProc,
					 metricDefinitionNode *,
					 int iCounterId,
					 const dictionary_hash<instInstance*,instInstance*> &map) {
   // a dup()-like routine; call after a fork()
   timerPtr = src.timerPtr; // assumes addr spaces have been dup()'d

   if (!map.find(src.sampler, this->sampler)) // writes to this->sampler
      assert(false);

   assert(sampler); // makes sense; timers are always sampled, whereas intCounters
                    // might be just non-sampled predicates.
   
   theSampleId = iCounterId;
   theTimerType = src.theTimerType;

   tTimer temp;
   P_memset(&temp, '\0', sizeof(tTimer)); /* is this needed? */
   temp.id.id = this->theSampleId;
   temp.type = this->theTimerType;
   temp.normalize = 1000000;
   writeToInferiorHeap(childProc, temp);

   // WARNING: shouldn't we be resetting the raw value to count=0, start=0,
   //          total = src.initialValue ???  On the other hand, it's not that
   //          simple -- if the timer is active in the parent, then it'll be active
   //          in the child.  So how about setting count to src.count, start=now,
   //          total=0 ???
}

dataReqNode *
sampledTimerReqNode::dup(process *childProc, metricDefinitionNode *mi,
			 int iCounterId,
			 const dictionary_hash<instInstance*,instInstance*> &map
			 ) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new sampledTimerReqNode(*this, childProc, mi, iCounterId, map);
}

bool sampledTimerReqNode::insertInstrumentation(process *theProc,
						metricDefinitionNode *) {
   timerPtr = (tTimer *)inferiorMalloc(theProc, sizeof(tTimer), dataHeap);
   if (timerPtr == NULL)
      return false; // failure!

   // Now let's initialize the newly allocated tTimer in the inferior heap:
   tTimer temp;
   P_memset(&temp, '\0', sizeof(tTimer));
   temp.id.id = this->theSampleId;
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

   const instPoint *func_entry = sampleFunction->funcEntry(theProc);
   sampler = addInstFunc(theProc, func_entry, ast,
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

bool sampledTimerReqNode::
unFork(dictionary_hash<instInstance*,instInstance*> &map) {
   instInstance *parentSamplerInstance = sampler;

   instInstance *childSamplerInstance;
   if (!map.find(parentSamplerInstance, childSamplerInstance))
      assert(false);

   vector<unsigned> pointsToCheck; // empty
   deleteInst(childSamplerInstance, pointsToCheck);

   map[parentSamplerInstance] = NULL; // since we've deleted...

   return true;
}
				 
#endif

/* ****************************************************************** */

#ifdef SHM_SAMPLING
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(int iCounterId) :
                                                 dataReqNode() {
   theSampleId = iCounterId;

   // The following fields are NULL until insertInstrumentation():
   allocatedIndex = UINT_MAX;
   inferiorTimerPtr = NULL;
}

sampledShmWallTimerReqNode::
sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src,
			   process *childProc,
			   metricDefinitionNode *mi,
			   int iCounterId) {
   // a dup()-like routine; call after a fork().
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting the new
   // inferiorTimerPtr isn't too hard.  Actually, it's trivial, since other code
   // ensures that the new shm segment is placed in exactly the same virtual mem loc
   // as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already copied the
   // actual data; we need to fill in new meta-data (new houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   fastInferiorHeap<wallTimerHK, tTimer> &theHeap =
                    childProc->getInferiorWallTimers();

   // since the new shm seg is placed in exactly the same memory location as the old
   // one, nothing here should change.
   tTimer *oldInferiorTimerPtr = src.inferiorTimerPtr;
   inferiorTimerPtr = theHeap.index2InferiorAddr(allocatedIndex);
   assert(inferiorTimerPtr == oldInferiorTimerPtr);

   // Write new raw value in the inferior heap:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   tTimer *localTimerPtr = theHeap.index2LocalAddr(allocatedIndex);
   const tTimer *srcTimerPtr = childProc->getParent()->getInferiorWallTimers().index2LocalAddr(allocatedIndex);

   localTimerPtr->total = 0;
   localTimerPtr->counter = srcTimerPtr->counter;
   localTimerPtr->id.id   = theSampleId;
   localTimerPtr->protector1 = srcTimerPtr->protector1;
   localTimerPtr->protector2 = srcTimerPtr->protector2;

   if (localTimerPtr->counter == 0)
      // inactive timer...this is the easy case to copy
      localTimerPtr->start = 0; // undefined, really
   else
      // active timer...don't copy the start time from the source...make it 'now'
      localTimerPtr->start = getCurrWallTime();

   // write new HK for this tTimer:
   // Note: we don't assert anything about mi->getMId(), because that id has no
   // relation to the ids we work with (theSampleId).  In fact, we (the sampling code)
   // just don't ever care what mi->getMId() is.
   assert(theSampleId >= 0);
   assert(midToMiMap.defines(theSampleId));
   assert(midToMiMap[theSampleId] == mi);
   wallTimerHK iHKValue(theSampleId, mi, 0); // is last param right?
      // the mi should differ from the mi of the parent; theSampleId differs too.
   theHeap.initializeHKAfterFork(allocatedIndex, iHKValue);
}

dataReqNode *
sampledShmWallTimerReqNode::dup(process *childProc,
				metricDefinitionNode *mi,
				int iCounterId,
				const dictionary_hash<instInstance*,instInstance*> &
				) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new sampledShmWallTimerReqNode(*this, childProc, mi, iCounterId);
      // fork constructor
}

bool sampledShmWallTimerReqNode::insertInstrumentation(process *theProc,
						       metricDefinitionNode *iMi) {
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   wallTimerHK iHKValue(this->theSampleId, iMi, 0);

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

   fastInferiorHeap<wallTimerHK, tTimer> &theShmHeap = theProc->getInferiorWallTimers();

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
   theSampleId = iCounterId;

   // The following fields are NULL until insertInstrumentatoin():
   allocatedIndex = UINT_MAX;
   inferiorTimerPtr = NULL;
}

sampledShmProcTimerReqNode::
sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src,
			   process *childProc,
			   metricDefinitionNode *mi,
			   int iCounterId) {
   // a dup()-like routine; call after a fork()
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting the new
   // inferiorTimerPtr isn't too hard.  Actually, it's trivial, since other code
   // ensures that the new shm segment is placed in exactly the same virtual mem loc
   // as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already copied the
   // actual data; we need to fill in new meta-data (new houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   fastInferiorHeap<processTimerHK, tTimer> &theHeap =
                    childProc->getInferiorProcessTimers();

   // since the new shm seg is placed in exactly the same memory location as the old
   // one, nothing here should change.
   tTimer *oldInferiorTimerPtr = src.inferiorTimerPtr;
   inferiorTimerPtr = theHeap.index2InferiorAddr(allocatedIndex);
   assert(inferiorTimerPtr == oldInferiorTimerPtr);

   // Write new raw value:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   tTimer *localTimerPtr = theHeap.index2LocalAddr(allocatedIndex);
   const tTimer *srcTimerPtr = childProc->getParent()->getInferiorProcessTimers().index2LocalAddr(allocatedIndex);

   localTimerPtr->total = 0;
   localTimerPtr->counter = srcTimerPtr->counter;
   localTimerPtr->id.id   = theSampleId;
   localTimerPtr->protector1 = srcTimerPtr->protector1;
   localTimerPtr->protector2 = srcTimerPtr->protector2;

   if (localTimerPtr->counter == 0)
      // inactive timer...this is the easy case to copy
      localTimerPtr->start = 0; // undefined, really
   else
      // active timer...don't copy the start time from the source...make it 'now'
      localTimerPtr->start = childProc->getInferiorProcessCPUtime();

   // Write new HK for this tTimer:
   // Note: we don't assert anything about mi->getMId(), because that id has no
   // relation to the ids we work with (theSampleId).  In fact, we (the sampling code)
   // just don't ever care what mi->getMId() is.
   assert(theSampleId >= 0);
   assert(midToMiMap.defines(theSampleId));
   assert(midToMiMap[theSampleId] == mi);
   processTimerHK iHKValue(theSampleId, mi, 0); // is last param right?
      // the mi differs from the mi of the parent; theSampleId differs too.
   theHeap.initializeHKAfterFork(allocatedIndex, iHKValue);
}

dataReqNode *
sampledShmProcTimerReqNode::dup(process *childProc,
				metricDefinitionNode *mi,
				int iCounterId,
				const dictionary_hash<instInstance*,instInstance*> &
				) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   return new sampledShmProcTimerReqNode(*this, childProc, mi, iCounterId);
      // fork constructor
}

bool sampledShmProcTimerReqNode::insertInstrumentation(process *theProc,
						       metricDefinitionNode *iMi) {
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   processTimerHK iHKValue(this->theSampleId, iMi, 0);

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
