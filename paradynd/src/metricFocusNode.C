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

// $Id: metricFocusNode.C,v 1.217 2002/02/12 23:50:26 schendel Exp $

#include "common/h/headers.h"
#include <limits.h>
#include <assert.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "pdutil/h/sampleAggregator.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/pdThread.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/main.h"
#include "dyninstAPI/src/stats.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/mdld.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/showerror.h"
#include "paradynd/src/costmetrics.h"
#include "paradynd/src/metric.h"
#include "common/h/debugOstream.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/timing.h"
#include "paradyn/src/met/mdl_data.h"
#ifdef FREEDEBUG
#include <strstream.h>  // in flush_batch_buffer
#endif

#include "dyninstAPI/src/instPoint.h"

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern pdDebug_ostream metric_cerr;
extern pdDebug_ostream sampleVal_cerr;

extern unsigned inferiorMemAvailable;
extern vector<Address> getAllTrampsAtPoint(instInstance *instance);

void flush_batch_buffer();
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value, 
		     bool internal_metric);

timeLength currentPredictedCost = timeLength::Zero();

unsigned mdnHash(const metricDefinitionNode *&mdn) {
  return ((unsigned)(Address)mdn) >> 2; // assume all addrs are 4-byte aligned
  //  return ((unsigned) mdn);
}

unsigned componentMdnPtrHash(metricDefinitionNode * const &ptr) {
  // maybe assert that "ptr" isn't for an aggregate mi
  return string::hash(ptr->getFullName());
}

dictionary_hash<unsigned, metricDefinitionNode*> allMIs(uiHash);
dictionary_hash<string, metricDefinitionNode*> allMIComponents(string::hash);
dictionary_hash<string, metricDefinitionNode*> allMIPrimitives(string::hash);

// === machineMetFocus data ============================
bool isKeyDef_machineMetFocusBuf(const unsigned &key) {
  return allMIs.defines(key);
}

void undefKey_machineMetFocusBuf(const unsigned &key) {
  allMIs.undef(key);
}

void setVal_machineMetFocusBuf(const unsigned &key, machineMetFocusNode *val) {
  allMIs[key] = static_cast<metricDefinitionNode*>(val);
}

bool lookup_machineMetFocusBuf(const unsigned &key,machineMetFocusNode **val) {
  metricDefinitionNode** n = reinterpret_cast<metricDefinitionNode**>(val);
  metricDefinitionNode* &mfn = *n;
  return allMIs.find(key, mfn);
}

dictionary_hash_iter<unsigned, metricDefinitionNode*>
getIter_machineMetFocusBuf() {
     return allMIs;
}


// === processMetFocus data ============================
bool isKeyDef_processMetFocusBuf(const string &key) {
  return allMIComponents.defines(key);
}

void undefKey_processMetFocusBuf(const string &key) {
  allMIComponents.undef(key);
}

void setVal_processMetFocusBuf(const string &key, processMetFocusNode *val) {
  allMIComponents[key] = static_cast<metricDefinitionNode*>(val);
}

bool lookup_processMetFocusBuf(const string &key, processMetFocusNode **val) {
  metricDefinitionNode** n = reinterpret_cast<metricDefinitionNode**>(val);
  metricDefinitionNode* &mfn = *n;
  return allMIComponents.find(key, mfn);
}

dictionary_hash_iter<string, metricDefinitionNode*>
getIter_processMetFocusBuf() {
     return allMIComponents;
}

// === sampleMetFocus data =============================
bool isKeyDef_sampleMetFocusBuf(const string &key) {
  return allMIPrimitives.defines(key);
}

void undefKey_sampleMetFocusBuf(const string &key) {
  allMIPrimitives.undef(key);
}

void setVal_sampleMetFocusBuf(const string &key, sampleMetFocusNode *val) {
  allMIPrimitives[key] = static_cast<metricDefinitionNode*>(val);
}

bool lookup_sampleMetFocusBuf(const string &key, sampleMetFocusNode **val) {
  metricDefinitionNode** n = reinterpret_cast<metricDefinitionNode**>(val);
  metricDefinitionNode* &mfn = *n;
  return allMIPrimitives.find(key, mfn);
}

dictionary_hash_iter<string, metricDefinitionNode*>
getIter_sampleMetFocusBuf() {
  return allMIPrimitives;
}

vector<defInst*> instrumentationToDo;
vector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

sampleAggregator DummyAggSample(aggregateOp(0), getCurrSamplingRate());

/* No longer used
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
*/


// for NON_MT_THREAD:  PRIM_MDN is non-aggregate
// for MT_THREAD:  THR_LEV could be aggregate (component being PROC_COMP)
metricDefinitionNode::metricDefinitionNode(process *p, const string& met_name, 
					   const vector< vector<string> >& foc,
					   const vector< vector<string> >& component_foc,
					   const string& component_flat_name, 
					   aggregateOp agg_op,
                                           MDN_TYPE mdntype)
: mdn_type_(mdntype),
  aggOp(agg_op),
  // CM5 metrics need aggOp to be set
  inserted_(false), instrDeferred_(false), installed_(false), met_(met_name), 
  focus_(foc), component_focus(component_foc), flat_name_(component_flat_name),
  aggregator(agg_op, getCurrSamplingRate()), _sentInitialActualValue(false),
  cumulativeValue(pdSample::Zero()),
  partsNeedingInitializing(true),
  id_(-1), originalCost_(timeLength::Zero()), proc_(p)
{
#if defined(MT_THREAD)
  needData_ = true ;
#endif
}

// for AGG_MDN or AGG_MDN metrics
metricDefinitionNode::metricDefinitionNode(const string& metric_name,
                                           const vector< vector<string> >& foc,
                                           const string& cat_name, 
                                          vector<metricDefinitionNode*>& parts,
                                           aggregateOp agg_op,
                                           MDN_TYPE mdntype)
: mdn_type_(mdntype),
  aggOp(agg_op), inserted_(false), instrDeferred_(false), installed_(false), 
  met_(metric_name), focus_(foc), flat_name_(cat_name), components(parts), 
  aggregator(aggregateOp(agg_op), getCurrSamplingRate()), 
  _sentInitialActualValue(false), cumulativeValue(pdSample::Zero()), 
  partsNeedingInitializing(true),
  id_(-1), originalCost_(timeLength::Zero()), proc_(NULL)
{
  unsigned p_size = parts.size();
  metric_cerr << " [MDN constructor:  part size = " << p_size << "]" << endl;
  for (unsigned u=0; u<p_size; u++) {
    metricDefinitionNode *mi = parts[u];
    mi->aggregators += this;
    mi->samples += aggregator.newComponent();
    // mi's comp_flat_names is updated in apply_to_process in mdl.C
  }
#if defined(MT_THREAD)
  needData_ = true ;
#endif

  // as before, only have fields:
  //     components, aggregator
}

machineMetFocusNode::machineMetFocusNode(const string& metric_name, 
		      const vector< vector<string> >& foc,
		      const string& cat_name,
		      vector<metricDefinitionNode*>& parts,
		      aggregateOp agg_op)
  : metricDefinitionNode(metric_name, foc, cat_name, parts, agg_op, AGG_MDN)
{

}

processMetFocusNode::processMetFocusNode(process *p,const string& metric_name,
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op)
  : metricDefinitionNode(p, metric_name, foc, component_foc,
			 component_flat_name, agg_op, COMP_MDN)
{
}

sampleMetFocusNode::sampleMetFocusNode(process *p, const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op)
  : metricDefinitionNode(p, metric_name, foc, component_foc,
			 component_flat_name, agg_op, PRIM_MDN)
{
}

threadMetFocusNode::threadMetFocusNode(process *p, const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op)
  : metricDefinitionNode(p, metric_name, foc, component_foc,
			 component_flat_name, agg_op, THR_LEV)
{ 
}

collectThreadMetFocusNode::collectThreadMetFocusNode(process *p, 
		       const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op)
  : threadMetFocusNode(p, metric_name, foc, component_foc,
			 component_flat_name, agg_op)
{ 
}

indivThreadMetFocusNode::indivThreadMetFocusNode(process *p, 
		       const string& metric_name, 
                       const vector< vector<string> >& foc,
                       const vector< vector<string> >& component_foc,
                       const string& component_flat_name, 
		       aggregateOp agg_op)
  : threadMetFocusNode(p, metric_name, foc, component_foc,
		       component_flat_name, agg_op)
{ 
}

const char *typeStr(int i) {
  static const char* typeName[] = { "AGG", "COMP", "PRIM", "THR" };  
  if(! (i>=0 && i<=3))
    cerr << "i == " << i << "\n";
  assert(i>=0 && i<=3);
  return typeName[i];
}

// check for "special" metrics that are computed directly by paradynd 
// if a cost of an internal metric is asked for, enable=false
machineMetFocusNode *doInternalMetric(vector< vector<string> >& canon_focus,
				      vector< vector<string> >&,
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
  machineMetFocusNode *mn = 0; 

  // check to see if this is an internal metric
  unsigned im_size = internalMetric::allInternalMetrics.size();

  for (unsigned im_index=0; im_index<im_size; im_index++){
    internalMetric *theIMetric = internalMetric::allInternalMetrics[im_index];
    if (theIMetric->name() == metric_name) {
      matched = true;
      if (!enable)
	return (machineMetFocusNode*)-1;

      if (!theIMetric->legalToInst(canon_focus))
	// Paradyn will handle this case and report appropriate error msg
	return (machineMetFocusNode*)-2;

      // it's required that the internal metric's mdn be a "top level node"
      // (ie. AGG_MDN or AGG_MDN) in order for setInitialActualValue to send
      // the value the the front-end
      vector<metricDefinitionNode*> noParts;
      mn = new machineMetFocusNode(metric_name, canon_focus,
		    flat_name, noParts, theIMetric->aggregate());
      assert(mn);
      
      unsigned instIndex = theIMetric->enableNewInstance(mn);
      theIMetric->getEnabledInstance(instIndex).setStartTime(getWallTime());
      return(mn);
    }
  }

  // check to see if this is a cost metric
  for (unsigned i=0; i < costMetric::allCostMetrics.size(); i++){
    if(costMetric::allCostMetrics[i]->name() == metric_name){
      matched = true;
      if (!enable)
	return (machineMetFocusNode*)-1;

      costMetric *nc = costMetric::allCostMetrics[i];

      if (!nc->legalToInst(canon_focus))
	return (machineMetFocusNode*)-2;
      vector<metricDefinitionNode*> noParts;
      mn = new machineMetFocusNode(metric_name, canon_focus,
		       flat_name, noParts, nc->aggregate());
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

static void print_focus(pdDebug_ostream &os, vector< vector<string> > &focus) {
  for (unsigned a=0; a < focus.size(); a++) {
    for (unsigned b=0; b < focus[a].size(); b++)
      os << '/' << focus[a][b];

    if (a < focus.size()-1)
      os << ',';
  }
  os << endl;
}

machineMetFocusNode *createMetricInstance(string& metric_name, 
					  vector<u_int>& focus,
				     // true if for real; false for guessCost()
					  bool enable, 
					  bool& internal)
{
    vector< vector<string> > canonicalFocus;
    // we make third parameter false to avoid printing warning messages in
    // focus2CanonicalFocus ("enable" was here previously) - naim
    if (!focus2CanonicalFocus(focus, canonicalFocus, false)) {
       //if (enable) cerr << "createMetricInstance failed because focus2CanonicalFocus failed" << endl;
       return NULL;
    }

    for(unsigned z = 0; z < canonicalFocus.size(); z++) {
        vector<string> temp_strings = canonicalFocus[z];
        for(unsigned y = 0; y < temp_strings.size(); y++) {
        }
    }

    string flat_name = metricAndCanonFocus2FlatName(metric_name, canonicalFocus);


    // first see if it is already defined.
    dictionary_hash_iter<unsigned, metricDefinitionNode*> mdi =
                                             getIter_machineMetFocusBuf();

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

    // first see if it is already defined.
    for (; mdi; mdi++) {
       machineMetFocusNode *mi = 
	                    dynamic_cast<machineMetFocusNode*>(mdi.currval());

       if (mi->getFullName() == flat_name) {
          return mi; // this metricDefinitionNode has already been defined
       }
    }

    if (mdl_can_do(metric_name)) {
      internal = false;

      /* select the processes that should be instrumented. We skip process
	 that have exited, and processes that have been created but are not
	 completely initialized yet.
	 If we try to insert instrumentation in a process that is not ready
	 yet, we get a core dump.
	 A process is ready when it is not in neonatal state and the 
	 isBootstrappedYet returns true.
      */
      vector<process*> procs;
      vector< vector<pdThread *> > threadsVec;

      for (unsigned u = 0; u < processVec.size(); u++) {
	if (processVec[u]->status()==exited 
	    || processVec[u]->status()==neonatal || processVec[u]->isBootstrappedYet()) 
	{
	  procs += processVec[u];
#if defined(MT_THREAD)
	  threadsVec += processVec[u]->threads;
#endif
	}
      }

#if defined(MT_THREAD)
      if (procs.size() == 0 || threadsVec.size() == 0) {
	// there are no processes or threads to instrument
#else
      if (procs.size() == 0) {
	// there are no processes to instrument
#endif

	//printf("createMetricInstance failed, no processes to instrument\n");
	return NULL;
      }

      bool computingCost;
      if (enable) computingCost = false;
      else computingCost = true;
      machineMetFocusNode *mi = mdl_do(canonicalFocus, metric_name, flat_name,
				       procs, threadsVec, false, computingCost);
      //cerr << "  mdl_do returned ";
      //if (mi == NULL) {
      //    cerr << "NULL" << endl;
      //} else {
      //    cerr << "Non-NULL" << endl;
      //}

      if (mi == NULL) {
	 metric_cerr << "createMetricInstance failed since mdl_do failed" << endl;
	 metric_cerr << "metric name was " << metric_name << "; focus was ";
	 print_focus(metric_cerr, canonicalFocus);
      }
#if defined(TEST_DEL_DEBUG)
      // print mdn info
      if (mi != NULL) {
	logLine("*** AGGREGATE LEVEL\n");
	sprintf(errorLine,"*** METRIC: %s\n",mi->getMetName().string_of());
	logLine(errorLine);
	sprintf(errorLine,"*** FLAT NAME: %s\n",mi->getFullName().string_of());
	logLine(errorLine);
	for (unsigned i=0;i<(mi->getComponents()).size();i++) {
	  metricDefinitionNode *proc_mi = (mi->getComponents())[i];
	  if (i==0) {
	    if (proc_mi->getMdnType() == COMP_MDN)
	      logLine("****** PROCESS LEVEL\n");
	    if (proc_mi->getMdnType() == PRIM_MDN)
	      logLine("****** PROCESS PRIM LEVEL\n");
	    if (proc_mi->getMdnType() == THR_LEV)
	      logLine("****** THREAD LEVEL\n");
	  }
	  sprintf(errorLine,"****** METRIC: %s\n",proc_mi->getMetName().string_of());
	  logLine(errorLine);
	  sprintf(errorLine,"****** FLAT NAME: %s\n",proc_mi->getFullName().string_of());
	  logLine(errorLine);
	  for (unsigned j=0;j<(proc_mi->getComponents()).size();j++) {
	    metricDefinitionNode *thr_mi = (proc_mi->getComponents())[j];
	    if (j==0) {
	      if (thr_mi->getMdnType() == COMP_MDN)
		logLine("********* PROCESS LEVEL\n");
	      if (thr_mi->getMdnType() == PRIM_MDN)
		logLine("********* PROCESS PRIM LEVEL\n");
	      if (thr_mi->getMdnType() == THR_LEV)
		logLine("********* THREAD LEVEL\n");
	    }
	    sprintf(errorLine,"********* METRIC: %s\n",thr_mi->getMetName().string_of());
	    logLine(errorLine);
	    sprintf(errorLine,"********* FLAT NAME: %s\n",thr_mi->getFullName().string_of());
	    logLine(errorLine);	  
	  }
	}
      }
#endif

      return mi;
    } else {
      bool matched;
      machineMetFocusNode *mi=doInternalMetric(canonicalFocus, canonicalFocus,
					 metric_name,flat_name,enable,matched);
      // NULL on serious error; -1 if enable was false; -2 if illegal to instr with
      // given focus [many internal metrics work only for whole program]

      if (mi == (machineMetFocusNode*)-2) {
	metric_cerr << "createMetricInstance: internal metric " << metric_name << " isn't defined for focus: ";
	print_focus(metric_cerr, canonicalFocus);
	mi = NULL; // straighten up the return value
      }
      else if (mi == (machineMetFocusNode*)-1) {
	metric_cerr << " createMetricInstance: internal metric not enable: " << metric_name << endl;
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
      return mi;
    }
}


// propagate this metric instance to process p.
// p is a process that started after the metric instance was created
// note: don't call this routine for a process started via fork or exec, just
// for processes started the "normal" way.
// "this" is an aggregate(AGG_MDN or AGG_MDN) mi, not a component one.

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

  machineMetFocusNode *mi = NULL;
     // an aggregate (not component) mi, though we know that it'll contain just
     // one component.  It's that one component that we're really interested in.
  if (mdl_can_do(met_)) {
      // Make the unique ID for this metric/focus visible in MDL.
      string vname = "$globalId";
      mdl_env::add(vname, false, MDL_T_INT);
      mdl_env::set(this->getMId(), vname);

      vector<process *> vp(1,p);
      vector< vector<pdThread *> > threadsVec;
#if defined(MT_THREAD)
      threadsVec += p->threads;
#endif
      mi = mdl_do(focus_, met_, flat_name_, vp, threadsVec, false, false);
  } else {
    // internal and cost metrics don't need to be propagated (um, is this correct?)
    mi = NULL;
  }

  if (mi) { // successfully created new mi
    assert(mi->components.size() == 1);

    metricDefinitionNode *theNewComponent = mi->components[0];

    components += theNewComponent;
#if defined(MT_THREAD)
    unsigned aggr_size = theNewComponent->aggregators.size();
    theNewComponent->aggregators[aggr_size-1] = this;       // overwrite
    theNewComponent->samples[aggr_size-1] = aggregator.newComponent();
                                                            // overwrite
    // theNewComponent->comp_flat_names[aggr_size-1]  has the correct value
#else
    theNewComponent->aggregators[0] = this;
    theNewComponent->samples[0] = aggregator.newComponent();
#endif

    if (!internal) {
      // dummy parameters for insertInstrumentation
      pd_Function *func = NULL;
      theNewComponent->insertInstrumentation(&func);
      theNewComponent->checkAndInstallInstrumentation();
    }

    // update cost
    const timeLength cost = mi->cost();
    if (cost > originalCost_) {
      addCurrentPredictedCost(cost - originalCost_);
      originalCost_ = cost;
    }

    mi->components.resize(0); // protect the new component
    delete mi;
  }
}

/* This only called by MDN of type COMP_MDN, it meddles with too many
   members of MDN to make it a member of processMetFocusNode
*/
metricDefinitionNode* metricDefinitionNode::handleExec() {
   // called by handleExec(), below.  See that routine for documentation.
   // "this" is a component mi.

   // If this component mi can be (re-)enabled in the new (post-exec)
   // process, then do so.  Else, remove the component mi from aggregators,
   // etc.  Returns new component mi if successful, NULL otherwise.

   // How can we tell if the mi can be inserted into the "new" (post-exec)
   // process?  A component mi is basically a set of instReqNodes and
   // dataReqNodes.  The latter don't restrict what can be inserted (is this
   // right?); the instReqNodes hold the key -- we should look at the
   // functions (instPoint's) where code (whose contents are in AstNode's)
   // would be inserted.  Now clearly, the instPoint's must be checked -- if
   // any one doesn't exist, then the instReqNode and hence the component mi
   // doesn't belong in the post-exec process.  But what about the AstNode's?
   // Should the code that gets inserted be subject to a similar test?
   // Probably, but we currently don't do it.

   // BUT: Even if a process contains a function in both the pre-exec and
   // post-exec stages, we must assume that the function is IN A DIFFERENT
   // LOCATION IN THE ADDRESS SPACE.  Ick.  So the instPoint's can't be
   // trusted and must be recalculated from scratch.  In that regard, this
   // routine is similar to propagateToNewProcess(), which propagates
   // aggregate mi's to a brand new process (but which doesn't work for
   // processes started via fork or exec).  The lesson learned is to (ick,
   // ick, ick) call mdl_do() all over again.  This gets really confusing
   // when you consider that a component mi can belong to several aggregate
   // mi's (e.g. if we represent cpu time for proc 100 then we can belong to
   // cpu/whole and cpu/proc-100); for which aggregate mi should we run
   // mdl_do?  Any will do, so we can pick arbitrarily (is this right?).

   // QUESTION: What about internal or cost metrics???  They have aggregate
   // and component mi's just like normal metrics, right?  If that's so, then
   // they must be propagated too!  NOT YET IMPLEMENTED!!!

#if defined(MT_THREAD)
   machineMetFocusNode *aggregateMI = NULL;
   
   for (unsigned u=0; u<aggregators.size(); u++)
     if (aggregators[u]->getMdnType() == AGG_MDN) {
       aggregateMI = dynamic_cast<machineMetFocusNode*>(aggregators[u]);
       break;
     }

   if (!aggregateMI)                                        // abort if all aggregators are thr_lev's
     return NULL;                                           // could replic thr_lev's agg, not for now
#else
   machineMetFocusNode *aggregateMI = dynamic_cast<machineMetFocusNode*>(
                                                 this->aggregators[0]);
#endif
   metricDefinitionNode *resultCompMI = NULL; // so far...

   const bool internal = !mdl_can_do(const_cast<string&>(getMetName()));
   if (internal)
      return NULL; // NOT YET IMPLEMENTED

   // try to propagate the mi note: the following code is mostly stolen from
   // propagateToNewProcess(); blame it for any bugs :)

   // Make the unique ID for this metric/focus visible in MDL. (?)
   string vname = "$globalId";
   mdl_env::add(vname, false, MDL_T_INT);
   mdl_env::set(aggregateMI->getMId(), vname);

   vector<process*> vp(1, this->proc());
   vector< vector<pdThread *> > threadsVec;
#if defined(MT_THREAD)
   threadsVec += this->proc()->threads;
#endif
   machineMetFocusNode *tempAggMI = mdl_do(
              const_cast<vector< vector<string> > &>(aggregateMI->getFocus()),
	                      const_cast<string &>(aggregateMI->getMetName()),
	                      const_cast<string &>(aggregateMI->getFullName()),
	                      vp, threadsVec,
                              true, // fry existing component MI
	                      false);
   if (tempAggMI == NULL)
      return NULL; // failure

   assert(tempAggMI->isTopLevelMDN());

   // okay, it looks like we successfully created a new aggregate mi.
   // Of course, we're just interested in the (single) component mi contained
   // within it; it'll replace "this".

   vector<metricDefinitionNode *> comp = tempAggMI->getComponents();
   assert(comp.size() == 1);
   resultCompMI = comp[0];

#if defined(MT_THREAD)
   unsigned size = resultCompMI->aggregators.size();
   resultCompMI->aggregators.resize(size-1);
   resultCompMI->samples.resize(size-1);
   resultCompMI->comp_flat_names.resize(size-1);
#else
   resultCompMI->aggregators.resize(0);
   resultCompMI->samples.resize(0);
#endif

   // For each aggregator, go back and find where "this" was a component mi.
   // When found, replace the ptr to "this" with "theNewComponent".
   unsigned num_aggregators = aggregators.size();
   assert(num_aggregators > 0);
   for (unsigned agglcv=0; agglcv < num_aggregators; agglcv++) {
      metricDefinitionNode *aggMI = aggregators[agglcv];

#if defined(MT_THREAD)
      if (THR_LEV == aggregators[agglcv]->getMdnType())
	continue;
#endif

      bool found=false;
      vector<metricDefinitionNode *> aggComp = aggMI->getComponents();
      for (unsigned complcv=0; complcv < aggComp.size(); complcv++) {
	 if (aggComp[complcv] == this) {
	    aggComp[complcv] = 
	      static_cast<metricDefinitionNode*>(resultCompMI);

	    resultCompMI->aggregators += aggMI;
	    resultCompMI->samples     += aggMI->getAggregator().newComponent();
#if defined(MT_THREAD)
	    resultCompMI->comp_flat_names += comp_flat_names[agglcv];
#endif

	    this->samples[agglcv]->requestRemove();
	    
	    found=true;
	    break;
	 }
      }
      assert(found);
   }

   // Now let's actually insert the instrumentation:
   if (!internal) {
      // dummy parameters for insertInstrumentation 
      pd_Function *func = NULL;
      resultCompMI->insertInstrumentation(&func);
      resultCompMI->checkAndInstallInstrumentation();
   }

   // And fry "tempAggMI", but make sure "resultCompMI" isn't fried when we
   // do so
   tempAggMI->removeAllComponents();
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

   // note the two loops; we can't safely combine into one since the second loop modifies
   // the dictionary.
   vector<metricDefinitionNode*> allcomps;
   dictionary_hash_iter<string,metricDefinitionNode*> iter =
                                             getIter_processMetFocusBuf();
   for (; iter; iter++)
      allcomps += iter.currval();
   
   for (unsigned i=0; i < allcomps.size(); i++) {
     processMetFocusNode* componentMI = dynamic_cast<processMetFocusNode*>(
                                             allcomps[i]);
      if (componentMI->proc() != proc)
	 continue;

      forkexec_cerr << "calling handleExec for component "
	            << componentMI->flat_name_ << endl;

      processMetFocusNode *replaceWithComponentMI = 
	dynamic_cast<processMetFocusNode*>(
	static_cast<metricDefinitionNode*>(componentMI)->handleExec());

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

#if defined(MT_THREAD)
	 for (unsigned u1=0; u1<componentMI->comp_flat_names.size(); u1++)
	   if (isKeyDef_processMetFocusBuf(componentMI->comp_flat_names[u1]))
	     undefKey_processMetFocusBuf(componentMI->comp_flat_names[u1]);

	 for (unsigned u2=0; u2<componentMI->components.size(); u2++)
	   componentMI->removeComponent(componentMI->components[u2]);
	 componentMI->components.resize(0);
#else
	 assert(replaceWithComponentMI->flat_name_ == componentMI->flat_name_);
#endif
	 delete componentMI; // old component mi (dtor removes it from allMIComponents)
	 // This is redundant, see mdl.C, apply_to_process 
	 // assert(!allMIComponents.defines(replaceWithComponentMI->flat_name_));
#if defined(MT_THREAD)
	 for (unsigned u=0; u<replaceWithComponentMI->comp_flat_names.size(); 
	      u++) 
	 {
	   string &key = replaceWithComponentMI->comp_flat_names[u];
	   setVal_processMetFocusBuf(key, replaceWithComponentMI);
	 }
#else
	 setVal_processMetFocusBuf(replaceWithComponentMI->flat_name_,
				   replaceWithComponentMI);
#endif
      }
   }
}


// called when all components have been removed (because the processes have exited
// or exec'd) from "this".  "this" is an aggregate (AGG_MDN or AGG_MDN) mi.
void metricDefinitionNode::endOfDataCollection() {
  assert(isTopLevelMDN());

  flush_batch_buffer();
  // trace data streams
  extern dictionary_hash<unsigned, unsigned> traceOn;
  for (dictionary_hash_iter<unsigned,unsigned> iter=traceOn; iter; iter++) {
     unsigned key = iter.currkey();
     unsigned val = iter.currval();

     if (val) {
        extern void batchTraceData(int, int, int, char *);
	extern bool TRACE_BURST_HAS_COMPLETED;
	TRACE_BURST_HAS_COMPLETED = true;
	batchTraceData(0, key, 0, (char *)NULL);
	traceOn[key] = 0;
     }
  }
  // we're not done until this metric doesn't have any metrics
  if(components.size() == 0)
    tp->endOfDataCollection(id_);
}

#if defined(MT_THREAD)
void metricDefinitionNode::rmCompFlatName(unsigned u) {
  assert(COMP_MDN == mdn_type_);
  unsigned size = comp_flat_names.size();
  assert(u < size);
    
  if (isKeyDef_processMetFocusBuf(comp_flat_names[u])) {
    undefKey_processMetFocusBuf(comp_flat_names[u]);
  }
  
  comp_flat_names[u] = comp_flat_names[size-1];
  comp_flat_names.resize(size-1);
}
#endif

// called in removeThisInstance
// calls removeComponent and endOfDataCollection

// for NON_MT_THREAD:
// remove a component from an aggregate.
// "this" is an aggregate mi; "comp" is a component mi.

// for MT_THREAD: (remove comp from aggregate "this")
// this could be agg_lev(comp proc_comp or thr_lev), thr_lev(comp proc_comp)
// or proc_prim(comp thr_lev)
void metricDefinitionNode::removeFromAggregate(metricDefinitionNode *comp, 
					       int deleteComp) {
#if defined(MT_THREAD)

  unsigned size = components.size();
  for (unsigned u=0; u<size; u++) {
    if (components[u] == comp) {
      if (deleteComp) {
	metric_cerr << "   === removeFromAggregate: call removeComponet for " << u << "th component " << endl;
	removeComponent(components[u]);
	//delete components[u];
      }

      components[u] = components[size-1];
      components.resize(size-1);

      if (PRIM_MDN == mdn_type_) {
	assert(size == thr_names.size());
	thr_names[u] = thr_names[size-1];
	thr_names.resize(size-1);

	if (1 == size)
	  if (isKeyDef_sampleMetFocusBuf(flat_name_))
	    undefKey_sampleMetFocusBuf(flat_name_);
      }

      if (size == 1 && id_ != -1) {
	if (mdn_type_ == AGG_MDN)
	  endOfDataCollection();
      }

      // metric_cerr << "   === removeFromAggregate: " << u << "th component removed" << endl;
      return;
    }
  }
#else
  unsigned size = components.size();
  for (unsigned u=0; u<size; u++) {
    if (components[u] == comp) {
      if (deleteComp) {
	metric_cerr << "   === removeFromAggregate: call removeComponet for " << u << "th component " << endl;
	removeComponent(components[u]);
	//delete components[u];
      }
      components.erase(u);

      if (PRIM_MDN == mdn_type_ && 1 == size)
	if (isKeyDef_sampleMetFocusBuf(flat_name_))
	  undefKey_sampleMetFocusBuf(flat_name_);

      if (size == 1 && id_ != -1) {
	assert(mdn_type_ == AGG_MDN);
	endOfDataCollection();
      }

      return;
    }
  }
#endif  
  // should always find the right component 
  // assert(0);
}

// in the future should change it so constraint PRIM MDNs don't have
// aggComponents tied to them, since they don't have their values
// sampled or aggregated
// don't have to worry about 
void metricDefinitionNode::removePrimAggComps() {
  for(unsigned i=0; i<samples.size(); i++) {
    samples[0]->requestRemove();
  }
}

// DELETE AND DISABLE AND REMOVE: 
// start from ~metricDefinitionNode, disable() and removeThisInstance()

// called in handleExec, removeFromMetricInstances, deleteThread
// called for proc_comp in 1st 2 case, and for thr_lev in last case
// calls removeFromAggregate

// for NON_MT_THREAD:
// remove this component mi from all aggregators it is a component of.
// if the aggregate mi no longer has any components then fry the mi aggregate mi.
// called by removeFromMetricInstances, below, when a process exits (or exec's)

// for MT_THREAD: 
// remove proc_comp mi from all its aggregators, which could be agg_lev or thr_lev
//   if the agg_lev no longer has components, fry the agg_lev
//   if it's a thr_lev, fry it
//   undef in MIComponents
// or remove thr_lev mi from all its aggregators, which could be proc_prim or agg_lev
//   if the proc_prim no longer has components, fry the proc_prim
//   if the agg_lev no longer has components, fry the agg_lev 
//   undef in MIComponents if it's there

void metricDefinitionNode::removeThisInstance() {
  assert(! isTopLevelMDN());
  metric_cerr << "removeThisInstance() - " << this << ", sampleAgg: " 
	      << aggregator << "\n";
#if defined(MT_THREAD)

  assert(mdn_type_ == COMP_MDN || mdn_type_ == THR_LEV);
  
  unsigned aggr_size = aggregators.size();
  assert(aggr_size == samples.size());
  
  // remove aggregators first
  for (unsigned u=0; u<aggr_size; u++) {
    samples[u]->markAsFinished();

    metric_cerr << "   removeThisInstance: " << u 
		<< "th aggregator calls removeFromAggregate " << endl;
    aggregators[u]->removeFromAggregate(this, false);
  }
  aggregators.resize(0);
  samples.resize(0);

  if (COMP_MDN == mdn_type_) {
    assert(aggr_size == comp_flat_names.size());
    for (unsigned t=0; t<aggr_size; t++)
      rmCompFlatName(0);
  }
  assert(0 == comp_flat_names.size());
  // remove comp_flat_names, undef from allMIComponents

  // now remove components
  for (unsigned u1=0; u1<components.size(); u1++) {
    metric_cerr << "   removeThisInstance: this calls removeComponent for " 
		<< u1 << "th component " << endl;
    removeComponent(components[u1]);

    // if (0 == component[u1]->aggregators.size())
    // delete components[u1];
  }
  components.resize(0);

  // we must be careful here, since the removeFromAggregate call
  // could delete us.  If it does, we have to be sure that we don't
  // refer to any of our member variables once we've been deleted.
  // unsigned int agsz = tmpaggs.size();

#else
  assert(mdn_type_ == COMP_MDN);

  // The dummy component metricDefinitionNodes also comes through here.  I
  // don't understand these dummy mdn's but I see that they don't have any
  // components (in the cases I see atleast).

  // first, remove from allMIComponents (this is new --- is it right?)
  if (isKeyDef_processMetFocusBuf(flat_name_)) {
    undefKey_processMetFocusBuf(flat_name_);
  }

  for(unsigned pi=0; pi<components.size(); pi++) {
    metricDefinitionNode *primMdn = components[pi];
    primMdn->removePrimAggComps();
  }

  assert(aggregators.size() == samples.size());

  // This call to tryAggregation needs to come here because the
  // tryAggregation call will flush samples out of the PRIM<->COMP
  // aggComponents and get them passed up through any corresponding
  // COMP<->AGG aggComponents before they get marked as finished, and hence
  // unable to have samples added to them.
  tryAggregation();
  for (unsigned u=0; u<samples.size(); u++) {
    samples[u]->markAsFinished();
  }

  for (unsigned i=0; i<aggregators.size(); i++) {
    aggregators[i]->removeFromAggregate(this, false);
    aggregators[i]->endOfDataCollection();
  }  

  aggregators.resize(0);
  samples.resize(0);

  // now remove components
  for (unsigned u1=0; u1<components.size(); u1++) {
    metric_cerr << "   removeThisInstance: this calls removeComponent for " 
		<< u1 << "th component: " << components[u1] << endl;
    removeComponent(components[u1]);

    // if (0 == component[u1]->aggregators.size())
    // delete components[u1];
  }
  components.resize(0);

  // we must be careful here, since the removeFromAggregate call
  // could delete us.  If it does, we have to be sure that we don't
  // refer to any of our member variables once we've been deleted.
  /*
  unsigned int agsz = aggregators.size();
  while( agsz )
    {
      aggregators[0]->removeFromAggregate(this,true);
      agsz--;
    }
  */
  // we have been deleted - be sure not to refer to any member variables 
  // from here till the end of the method
#endif
}


// Called when a process exits, to remove the component associated to proc 
// from all metric instances.  (If, after an exec, we never want to carry over
// mi's from the pre-exec, then this routine will work there, too.  But we try to
// carry over mi's whenever appropriate.)
// Remove the aggregate metric instances that don't have any components left
void removeFromMetricInstances(process *proc) {
  metric_cerr << "removeFromMetricInstances- proc: " << proc << ", pid: " 
	      << proc->getPid() << "\n";
    // Loop through all of the _component_ mi's; for each with component process
    // of "proc", remove the component mi from its aggregate mi.
    // Note: imho, there should be a *per-process* vector of mi-components.

   // note 2 loops for safety (2d loop may modify dictionary?)
    vector<metricDefinitionNode *> MIs;
    dictionary_hash_iter<string,metricDefinitionNode*> iter =
                                            getIter_processMetFocusBuf();
    for (; iter; iter++)
       MIs += iter.currval();

#if defined(TEST_DEL_DEBUG)
    sprintf(errorLine,"=====> removeFromMetricInstances, MIs size=%d\n",MIs.size());
    logLine(errorLine);
#endif
    metric_cerr << "removeFromMetricInstance- numComp: " << MIs.size() << "\n";
    for (unsigned j = 0; j < MIs.size(); j++) {
      metric_cerr << "  j: " << j << ", comp: " << MIs[j] << ", mi->proc: "
		  << MIs[j]->proc() << ", numComp: " 
		  << (MIs[j]->getComponents()).size() << "\n";
      if (MIs[j] != NULL) {   // this check is necessary, MIs[j] could have been deleted if aliased
	
	if (MIs[j]->proc() == proc) 
	  MIs[j]->removeThisInstance();
      }
    }
    costMetric::removeProcessFromAll(proc); // what about internal metrics?
}

/* *************************************************************************** */

#if defined(MT_THREAD)
//
// reUseIndex only works at the presence of an aggregator.
// We always need a dummy aggregator for threads metrics, and it is 
// implemented in mdl.C apply_to_process
//
void threadMetFocusNode::reUseIndexAndLevel(unsigned &p_allocatedIndex, 
					    unsigned &p_allocatedLevel)
{
  p_allocatedIndex = UINT_MAX;
  p_allocatedLevel = UINT_MAX;
  unsigned agg_size = aggregators.size();

  assert(mdn_type_ == THR_LEV);

  sampleMetFocusNode *prim_mn = NULL;
  for (unsigned uu=0; uu < agg_size; uu++)
    if (aggregators[uu]->getMdnType() == PRIM_MDN) {
      prim_mn = dynamic_cast<sampleMetFocusNode*>(aggregators[uu]);
      break;
    }

  if (prim_mn != NULL) {
    vector<metricDefinitionNode *> comps = prim_mn->getComponents();
    unsigned c_size = comps.size();
    for (unsigned i=0; i<c_size; i++) {
      threadMetFocusNode *curThrNode = 
	dynamic_cast<threadMetFocusNode*>(comps[i]);

      if (curThrNode != this) {
	dataReqNode *p_dataRequest;
	// Assume for all metrics, data are allocated in the same order
	// we get the one that was created the earliest

	if (curThrNode->dataRequests.size() > this->dataRequests.size()) {
	  p_dataRequest = curThrNode->dataRequests[this->dataRequests.size()];
	  p_allocatedIndex = p_dataRequest->getAllocatedIndex();
	  p_allocatedLevel = p_dataRequest->getAllocatedLevel();
#if defined(TEST_DEL_DEBUG)
	  sprintf(errorLine,"=====> re-using level=%d, index=%d\n",
		  p_allocatedLevel, p_allocatedIndex);
	  cerr << errorLine << endl;
#endif
	  break;
	}
      }
    }
  }
}
#endif //MT_THREAD

// obligatory definition of static member vrble:
int metricDefinitionNode::counterId=0;

#if defined(MT_THREAD)
dataReqNode *threadMetFocusNode::addSampledIntCounter(pdThread *thr, 
						      rawTime64 initialValue,
#else
dataReqNode *threadMetFocusNode::addSampledIntCounter(rawTime64 initialValue,
#endif
                                                        bool computingCost,
							bool doNotSample)
{
   dataReqNode *result=NULL;

#ifdef MT_THREAD
   // shared memory sampling of a reported intCounter
   unsigned p_allocatedIndex, p_allocatedLevel;
   reUseIndexAndLevel(p_allocatedIndex, p_allocatedLevel);
   result = new sampledShmIntCounterReqNode(thr, initialValue,
                                            incrementCounterId(),
                                            this, computingCost, doNotSample,
					    p_allocatedIndex,p_allocatedLevel);
#else
   result = new sampledShmIntCounterReqNode(initialValue,
                                            incrementCounterId(),
                                            this, computingCost, doNotSample);
#endif //MT_THREAD
      // implicit conversion to base class

   assert(result);
   
   proc_->numOfActCounters_is++;

   dataRequests += result;
   return result;
}

#if defined(MT_THREAD)
dataReqNode *threadMetFocusNode::addWallTimer(bool computingCost, 
					      pdThread *thr) {
#else
dataReqNode *threadMetFocusNode::addWallTimer(bool computingCost) {
#endif
   dataReqNode *result = NULL;

#if defined(MT_THREAD)
   unsigned p_allocatedIndex, p_allocatedLevel;
   reUseIndexAndLevel(p_allocatedIndex, p_allocatedLevel);
   result = new sampledShmWallTimerReqNode(thr, incrementCounterId(), this, 
			    computingCost, p_allocatedIndex, p_allocatedLevel);
      // implicit conversion to base class
#else
   result = new sampledShmWallTimerReqNode(incrementCounterId(), this, 
					   computingCost);
#endif //MT_THREAD

   assert(result);
   proc_->numOfActWallTimers_is++;

   dataRequests += result;
   return result;
}

#if defined(MT_THREAD)
dataReqNode *threadMetFocusNode::addProcessTimer(bool computingCost, 
						 pdThread *thr) {
#else
dataReqNode *threadMetFocusNode::addProcessTimer(bool computingCost) {
#endif
   dataReqNode *result = NULL;

#if defined(MT_THREAD)
   unsigned p_allocatedIndex, p_allocatedLevel;
   reUseIndexAndLevel(p_allocatedIndex, p_allocatedLevel);
   result = new sampledShmProcTimerReqNode(thr, incrementCounterId(), this, 
			    computingCost, p_allocatedIndex, p_allocatedLevel);
      // implicit conversion to base class
#else
   result = new sampledShmProcTimerReqNode(incrementCounterId(), this, 
					   computingCost);
#endif //MT_THREAD

   assert(result);

   proc_->numOfActProcTimers_is++;

   dataRequests += result;
   return result;
};

/* *************************************************************************** */

// called when a process forks (by handleFork(), below). "this" is a
// (component) mi in the parent process. Duplicate it for the child, with
// appropriate changes (i.e. the pid of the component focus name differs),
// and return the newly created child mi.  "map" maps all instInstance's of
// the parent to those copied into the child.
// 
// Note how beautifully everything falls into place.  Consider the case of
// alarm sampling with cpu/whole program.  Then comes the fork.  The parent
// process has (0) a tTimer structure allocated in a specific location in the
// inferior heap, (1) instrumentation @ main to call startTimer on that ptr,
// (2) instrumentation in DYNINSTsampleValues() to call DYNINSTreportTimer on
// that ptr.  The child process of fork will have ALL of these things in the
// exact same locations, which is correct.  We want the timer to be located
// in the same spot; we want DYNINSTreportTimer to be called on the same
// pointer; and main() hasn't moved.
//
// So there's not much to do here.  We create a new component mi (with same
// flat name as in the parent, except for a different pid), and call
// "forkProcess" for all dataReqNodes and instReqNodes, none of which have to
// do anything titanic.

// duplicate the dataReqNodes and duplicate the instReqNodes: only for
// non-threaded

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
       if (component_focus[hier][0] == "Machine") {
	  foundProcess = true;
	  assert(component_focus[hier].size() >= 3);
	     // since a component focus is by definition specific to some process

	  //assert(component_focus[hier][2] == parentPartName); -- DAN
	  if( component_focus[hier][2] != parentPartName )
		  return NULL;

	  // change the process:
	  newComponentFocus[hier][2] = childPartName;

	  break;
       }
    }
    assert(foundProcess);
    
    string newComponentFlatName = metricAndCanonFocus2FlatName(met_, newComponentFocus);

    processMetFocusNode *mi = new processMetFocusNode(child,
			 met_, // metric name doesn't change
			 focus_, // focus doesn't change (tho component focus will)
			 newComponentFocus, // this is a change
			 newComponentFlatName, // this is a change
			 aggregateOp(aggOp));
    assert(mi);

    incrementCounterId();

    forkexec_cerr << "metricDefinitionNode::forkProcess -- component flat name for parent is " << flat_name_ << "; for child is " << mi->flat_name_ << endl;

    // not attempt to register all names
    assert(! isKeyDef_processMetFocusBuf(newComponentFlatName));
    setVal_processMetFocusBuf(newComponentFlatName, mi);

    /*
    // Duplicate the dataReqNodes:
    // If it's going to duplicate the dataReqNodes, these are members only of
    // the threadMetFocusNode.  So it would have to go through all of mi's
    // threadMetFocusNodes and duplicate all of it's data request nodes.
    // Then I suppose it would assign those duplicated dataRequestNodes to be
    // used by this metricDefinitionNode's threadMetFocusNodes.

    for (unsigned u1 = 0; u1 < dataRequests.size(); u1++) {
       // must add to drnIdToMdnMap[] before dup() to avoid some assert fails
       const int newCounterId = incrementCounterId();
          // no relation to mi->getMId();
       forkexec_cerr << "forked dataReqNode going into drnIdToMdnMap with id " << newCounterId << endl;
       assert(!drnIdToMdnMap.defines(newCounterId));
       drnIdToMdnMap[newCounterId] = static_cast<metricDefinitionNode*>(mi);
       
       dataReqNode *newNode = dataRequests[u1]->dup(child, 
                   static_cast<metricDefinitionNode*>(mi), newCounterId, map);
         // remember, dup() is a virtual fn, so the right dup() and hence the
         // right fork-ctor is called.
       assert(newNode);

       mi->dataRequests += newNode;
    }

    // Duplicate the instReqNodes:
    for (unsigned u2 = 0; u2 < instRequests.size(); u2++) {
      mi->instRequests += instReqNode::forkProcess(instRequests[u2], map);
    }

    mi->inserted_ = true;
    */
    return static_cast<metricDefinitionNode*>(mi);
}

// unforkInstRequests and unforkDataRequests only for non-threaded

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
   /*
   unsigned lcv;
   // Needs to be implemented
   if (unForkInstRequests)
      for (lcv=0; lcv < instRequests.size(); lcv++)
         if (!instRequests[lcv].unFork(map))
	    result = false; // failure

   if (unForkDataRequests)
      for (lcv=0; lcv < dataRequests.size(); lcv++)
         if (!dataRequests[lcv]->unFork(map))
	    result = false; // failure

   */
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

   // 2 loops for safety (2d loop may modify dictionary?)
   vector<metricDefinitionNode *> allComponents;
   dictionary_hash_iter<string,metricDefinitionNode*> iter =
                                                 getIter_processMetFocusBuf();
   for (; iter; iter++)
      allComponents += iter.currval();

   for (unsigned complcv=0; complcv < allComponents.size(); complcv++) {
      metricDefinitionNode *comp = allComponents[complcv];

      // duplicate the component (create a new one) if it belongs in the
      // child process.  It belongs if any of its aggregate mi's should be
      // propagated to the child process.  An aggregate mi should be propagated
      // if it wasn't refined to some process.

      bool shouldBePropagated = false; // so far
      bool shouldBeUnforkedIfNotPropagated = false; // so far
      assert(comp->aggregators.size() > 0);
      for (unsigned agglcv1=0; agglcv1 < comp->aggregators.size(); agglcv1++) {
	 metricDefinitionNode *aggMI = comp->aggregators[agglcv1];

	 if (aggMI->focus_[resource::machine].size() <= 2) {
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

	  if( !newComp )
		  continue;
         // copies instr (well, fork() does this for us), allocs ctr/timer space,
         // initializes.  Basically, copies dataReqNode's and instReqNode's.

      bool foundAgg = false;
      for (unsigned agglcv2=0; agglcv2 < comp->aggregators.size(); agglcv2++) {
	 metricDefinitionNode *aggMI = comp->aggregators[agglcv2];
	 if (aggMI->focus_[resource::machine].size() <= 2) {
	    // this aggregate mi wasn't specific to any process, so it gets the new
	    // child component.
	    aggMI->components += newComp;
	    newComp->aggregators += aggMI;
	    newComp->samples     += aggMI->aggregator.newComponent();
	    foundAgg = true;
	 }
      }
      assert(foundAgg);
   }
}


bool metricDefinitionNode::anythingToManuallyTrigger() const {

#if defined(MT_THREAD)
  if (mdn_type_ != PRIM_MDN) {
#else
  if ((mdn_type_ == AGG_MDN) || (mdn_type_ == COMP_MDN)) {
#endif
    for (unsigned i=0; i < components.size(); i++)
      if (components[i]->anythingToManuallyTrigger())
	return true;
    return false;
  }
  else {
    assert(mdn_type_ == PRIM_MDN);

    // Should we do this?
    //
    if( manuallyTriggerNodes.size() > 0 )
      return true;
    return false;
  }
  
  assert(false);
}

//
// Added to allow metrics affecting a function F to be triggered when
//  a program is executing (in a stack frame) under F:
// The basic idea here is similar to Ari's(?) hack to start "whole 
//  program" metrics requested while the program is executing 
//  (see T_dyninstRPC::mdl_instr_stmt::apply in mdl.C).  However,
//  in this case, the manuallyTrigger flag is set based on the 
//  program stack (the call sequence implied by the sequence of PCs
//  in the program stack), and the types of the set of inst points 
//  which the metricDefinitionNode corresponds to, as opposed to
//  a hacked interpretation of the original MDL statement syntax.
// The basic algorithm is as follows:
//  1. Construct function call sequence leading to the current 
//     stack frame (yields vector of pf_Function hopefully equivalent
//     to yield of "backtrace" functionality in gdb.
//  2. Look at each instReqNode in *this (call it node n):
//     Does n correspond to a function currently on the stack?
//       No -> don't manually trigger n's instrumentation.
//       Yes ->
//         

void metricDefinitionNode::adjustManuallyTrigger()
{
  // aggregate metricDefinitionNode - decide whether to manually trigger 
  //  instrumentation corresponding to each component node individually.
#if defined(MT_THREAD)
  if (mdn_type_ == AGG_MDN || mdn_type_ == THR_LEV)
#else
  if (mdn_type_ == AGG_MDN)
#endif
  {
    for (unsigned i=0; i < components.size(); i++) {
      components[i]->adjustManuallyTrigger();
    }
  }
  // non-aggregate:
  else if (mdn_type_ == COMP_MDN)
  {
//
#if defined(MT_THREAD)
    
    vector<vector<Address> > pc_s = proc_->walkAllStack();
    // WalkAllStack _MUST_ return the list of stacks sorted in the same
    // order as the list of threads in the process

    for (unsigned stack_i=0; stack_i< pc_s.size(); stack_i++) {
      for (unsigned component_i=0; component_i < components.size(); 
	   component_i++) {
	sampleMetFocusNode *curPrim = dynamic_cast<sampleMetFocusNode*>(
                                                    components[component_i]);
	curPrim->adjustManuallyTrigger(pc_s[stack_i], 
				       proc_->threads[stack_i]->get_tid());
      }
    }
//
#else
    vector<Address> stack_pcs = proc_->walkStack();
    for (unsigned i1=0; i1 < components.size(); i1++) {
      sampleMetFocusNode *curPrim = dynamic_cast<sampleMetFocusNode*>(
                                                       components[i1]);
      curPrim->adjustManuallyTrigger(stack_pcs, -1);
    }
#endif

  }
  else {
    assert(0);  // PRIM_MDN or PRIM_MDN
  }
}

// A debug function for adjustManuallyTrigger

void adjustManuallyTrigger_debug(instReqNode &iRN)
{
  //
  //
  switch (iRN.When()) {
  case callPreInsn:
    cerr << " callPreInsn for ";
    break;
  case callPostInsn:
    cerr << " callPostInsn for ";
    break;
  }
#if defined(mips_sgi_irix6_4)
  if( iRN.Point()->type() == IPT_ENTRY )
    cerr << " FunctionEntry " << endl;
  else if (iRN.Point()->type() == IPT_EXIT )
    cerr << " FunctionExit " << endl;
  else if (iRN.Point()->type() == IPT_CALL )
    cerr << " callSite " << endl;
  
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
  if( iRN.Point()->ipType == functionEntry )
    cerr << " Function Entry " << endl;
  else if( iRN.Point()->ipType == functionExit )
    cerr << " FunctionExit " << endl;
  else if( iRN.Point()->ipType == callSite )
    cerr << " callSite " << endl;
  
#elif defined(rs6000_ibm_aix4_1)
  if( iRN.Point()->ipLoc == ipFuncEntry )
    cerr << " FunctionEntry " << endl;
  if( iRN.Point()->ipLoc == ipFuncReturn )
    cerr << " FunctionExit " << endl;
  if( iRN.Point()->ipLoc == ipFuncCallPoint )
    cerr << " callSite " << endl;
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
  if( iRN.Point()->iPgetAddress() == iRN.Point()->iPgetFunction()->addr() )
    cerr << " FunctionEntry " << endl;
  else if ( iRN.Point()->insnAtPoint().isCall() ) 
    cerr << " calSite " << endl;
  else
    cerr << " FunctionExit " << endl;
#else
#error Check for instPoint type == entry not implemented on this platform
#endif
  
//
//
}

void sampleMetFocusNode::adjustManuallyTrigger(vector<Address> stack_pcs, 
					       int tid)
{
  assert(mdn_type_ == PRIM_MDN);

  vector<instPoint*> instPts;
  unsigned j, k;
  pd_Function *stack_func;
  instPoint *point;
  Address stack_pc;
  
  string prettyName; // not really a good name
  if (pd_debug_catchup) {
    prettyName = met_ + string(": <");;

    bool first = true;
    for (unsigned h=0; h<focus_.size(); h++) {
      if (focus_[h].size() > 1) {
	if (!first) prettyName += string(",");
	first = false;
	for (unsigned c=0; c< focus_[h].size(); c++) {
	  prettyName += string("/");
	  prettyName += focus_[h][c];
	}
      }
    }
    prettyName += string(">");
  }

  if( stack_pcs.size() == 0 ) {
    cerr << "WARNING -- adjustManuallyTrigger was handed an empty stack" << endl;
  }
  vector<pd_Function *> stack_funcs = proc_->convertPCsToFuncs(stack_pcs);
  proc_->correctStackFuncsForTramps( stack_pcs, stack_funcs );
  bool badReturnInst = false;

  unsigned i = stack_funcs.size();
  //for(i=0;i<stack_funcs.size();i++) {
  if (i!=0)
    do {
      --i;
      stack_func = stack_funcs[i];
      stack_pc = stack_pcs[i];
      if (pd_debug_catchup) {
	if( stack_func != NULL ) {
	  instPoint *ip = findInstPointFromAddress(proc_, stack_pc);
	  if (ip) {// this is an inst point
	    cerr << i << ": " << stack_func->prettyName() << "@" << (void*) ip->iPgetAddress() << "[iP]"
		 << "@" << (void*)stack_pc << endl;
	  } else 
	    cerr << i << ": " << stack_func->prettyName() << "@" << (void*)stack_pc << endl;
	} else
	  cerr << i << ": skipped (unknown function) @" << (void*)stack_pc << endl;
      }
      if (stack_func == NULL) continue;
      instPts.resize(0);
      instPts += const_cast<instPoint*>( stack_func->funcEntry(proc_) );
      instPts += stack_func->funcCalls(proc_);
      instPts += stack_func->funcExits(proc_);

#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
      if (stack_func->isInstalled(proc_)) {
        instPts += const_cast<instPoint*>( stack_func->funcEntry(0) );
        instPts += stack_func->funcCalls(0);
        instPts += stack_func->funcExits(0);
      }
#endif

#if !(defined(i386_unknown_nt4_0) \
  || defined(i386_unknown_solaris2_5) \
  || defined(i386_unknown_linux2_0) \
  || defined(ia64_unknown_linux2_4) ) /* Temporary duplication - TLM */
      // If there is a function on the stack with relevant instPoints which we were
      // not able to install return instances for, we don't want to manually trigger
      // anything else further on the stack, as it could cause inconsistencies with
      // metrics which rely on matching pairs of actions. - DAN
      // **NOTE** we don't do this on x86, because we can always immediately insert
      // a 'jump' to base tramp via. trap.  In addition, when we use a trap rather
      // than a branch, the return instance is still NULL, and this code breaks. - DAN

      //
      // If any instrumentation attempt failed for this function, we shouldn't
      // be doing catchup instrumentation for this function. - ZHICHEN
      //
      for(j=1;j<instPts.size()&&!badReturnInst;j++) {
	for(k=0;k<instRequests.size();k++) {
	  if( instPts[j] == instRequests[k].Point() ) {
	    if( instRequests[k].getRInstance() != NULL
		&& !(instRequests[k].getRInstance()->Installed()) 
	      )
	    {
	      if (pd_debug_catchup) {
	        cerr << "AdjustManuallyTrigger -- Bad return instance in "
		     << stack_func->prettyName()
		     << ", not manually triggering for this stack frame." << endl;
	      }
	      badReturnInst = true;
	      break;
	    }
	  }
	}
      }
#endif
      if( badReturnInst )
	continue;
      
      // For all of the inst points in all the functions on the stack...

      for(j=0;j<instPts.size();j++) {
	point = instPts[j];
	for(k=0;k<instRequests.size();k++) {
	  if (point == instRequests[k].Point()) {
	    // If we have an instrumentation request for that point...
	    
 	    if (instRequests[k].Ast()->accessesParam()) {
	      // and it accesses parameters for the function, break (parameters are unknown)
	      break;
	    }
	    if (instRequests[k].triggeredInStackFrame(stack_func, stack_pc, proc_))
	      {
		// Otherwise, add it to the list to be manually triggered
		if (pd_debug_catchup) {
		  instReqNode &iRN = instRequests[k];
		  cerr << "--- catch-up needed for "
		       << prettyName << " @ " << stack_func->prettyName() 
		       << " @ " << (void*) stack_pc << endl;
		  adjustManuallyTrigger_debug(iRN);
		}
		manuallyTriggerNodes += &(instRequests[k]);
		instRequests[k].friesWithThat(tid);
	      }
	  }
	}
      }
    } while (i!=0);

#if defined(MT_THREAD)

  oldCatchUp(tid);

#endif  // not OLD_CATCHUP, but MT_THREAD
}


void sampleMetFocusNode::oldCatchUp(int tid) {

  unsigned j, k;

  assert(mdn_type_ == PRIM_MDN);
  
  assert(proc_); // proc_ should always be correct for non-aggregates
  const function_base *mainFunc = proc_->getMainFunction();
  assert(mainFunc); // processes should always have mainFunction defined
                    // Instead of asserting we could call adjustManuallyTrigger0,
                    // which could handle a pseudo function.

  // The following code is used in the case where the new catchup code is disabled.
  // It is replicated in the adjustManuallyTrigger0 function and at some point in the
  // future could be moved into a single separate function.  This code could also
  // useful in the case where mainFunc is undefined.

  // This code is a kludge which will catch the case where the WHOLE_PROGRAM metrics
  // have not been set to manjually trigger by the above code.  Look at the 
  // component_focus for the "Code" element, and see if there is any constraint.
  // Then, for each InstReqNode in this MetricDefinitionNode which is at the entry
  // point of main, and which has not been added to the manuallyTriggerNodes list,
  // add it to the list.
  for( j = 0; j < component_focus.size(); ++j )
    if( component_focus[j][0] == "Code" && 
	( component_focus[j].size() == 1 ||
	  ( component_focus[j].size() == 2 && component_focus[j][1] == "" ) ) )
      for( k = 0; k < instRequests.size(); ++k ) {
	if( instRequests[ k ].Point()->iPgetFunction() == mainFunc ) {
	  unsigned dummy;
	  if( !find( manuallyTriggerNodes, &(instRequests[k]), dummy ) ) {
#if defined(mips_sgi_irix6_4)
	  if( instRequests[ k ].Point()->type() == IPT_ENTRY )
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
	  if( instRequests[ k ].Point()->ipType == functionEntry )
#elif defined(rs6000_ibm_aix4_1)
	  if( instRequests[ k ].Point()->ipLoc == ipFuncEntry )
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) \
      || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */
	    if( instRequests[ k ].Point()->iPgetAddress() == mainFunc->addr() )
#else
#error Check for instPoint type == entry not implemented on this platform
#endif
	    {
              if ( pd_debug_catchup ) {
		metric_cerr << "AdjustManuallyTrigger -- "
			    << "(WHOLE_PROGRAM kludge) catch-up needed for "
			    << flat_name_ << " @ " << mainFunc->prettyName() << endl;
              }
	      //manuallyTriggerNodes.insert( 0, &(instRequests[k]) );
	      manuallyTriggerNodes.push_back( &(instRequests[k]) );
	      instRequests[k].friesWithThat(tid);
	    }
	  }
	}
      }
}
//
// degenerated version
//
#if defined(MT_THREAD)
void metricDefinitionNode::adjustManuallyTrigger0(int tid)
{
  vector<instPoint*> instPts;
  unsigned i;
  
  // aggregate metricDefinitionNode - decide whether to manually trigger 
  //  instrumentation corresponding to each component node individually.
  if (mdn_type_ == AGG_MDN || mdn_type_ == THR_LEV || mdn_type_ == COMP_MDN)  // ! COMP_MDN
  {
    for (i=0; i < components.size(); i++) {
      components[i]->adjustManuallyTrigger0(tid);
    }
  } else {
    assert(false);
  }
}

void sampleMetFocusNode::adjustManuallyTrigger0(int tid) {
  // This code is a kludge which will catch the case where the WHOLE_PROGRAM
  // metrics have not been set to manjually trigger by the above code.  Look
  // at the component_focus for the "Code" element, and see if there is any
  // contraint.  Then, for each InstReqNode in this MetricDefinitionNode
  // which is at the entry point of main, and which has not been added to the
  // manuallyTriggerNodes list, add it to the list.

  oldCatchUp(tid);
}
#endif 
 
// Look at the inst point corresponding to *this, and the stack.
// If inst point corresponds a location which is conceptually "over"
// the current execution frame, then set the manuallyTrigger flag
// (of *this) to true, (hopefully) causing the AST corresponding
// to the inst point to be executed (before the process resumes
// execution).
// What does conceptually "over" mean?
// An inst point is "over" the current execution state if that inst
//  point would have had to have been traversed to get to the current
//  execution state, had the inst point existed since before the
//  program began execution.
// In practise, inst points are categorized as function entry, exit, and call
//  site instrumentation.  entry instrumentation is "over" the current 
//  execution frame if the function it is applied to appears anywhere
//  on the stack.  exit instrumentation is never "over" the current 
//  execution frame.  call site instrumentation is "over" the current
//  execution frame if   

void metricDefinitionNode::manuallyTrigger(int parentMId) {
   assert(anythingToManuallyTrigger());

   bool aggr = true;

   if (mdn_type_ == PRIM_MDN)
     aggr = false;

   if( aggr ) {
     for ( unsigned i=0; i < components.size(); ++i )
       if (components[i]->anythingToManuallyTrigger())
	 components[i]->manuallyTrigger(parentMId);
     return;
   }

   for ( unsigned i=0; i < manuallyTriggerNodes.size(); ++i ) {
     if (!manuallyTriggerNodes[i]->triggerNow(proc(),parentMId)) {
       cerr << "manual trigger failed for an inst request" << endl;
     }
   }
   manuallyTriggerNodes.resize( 0 );
}

#if defined(MT_THREAD)
void metricDefinitionNode::propagateId(int id) {
  if (mdn_type_ != THR_LEV) {
    for (unsigned i=0;i<components.size();i++) 
      if (components[i])
        components[i]->propagateId(id);
  }
  if (id_ == -1) id_ = id;
}
#endif

// startCollecting is called by dynRPC::enableDataCollection 
// (or enableDataCollection2) in dynrpc.C
// startCollecting is a friend of metricDefinitionNode; can it be
// made a member function of metricDefinitionNode instead?
// Especially since it clearly is an integral part of the class;
// in particular, it sets the crucial vrble "id_"
//
int startCollecting(string& metric_name, vector<u_int>& focus, int id,
		    vector<process *> &procsToCont)
{
    bool internal = false;
    // Make the unique ID for this metric/focus visible in MDL.
    string vname = "$globalId";
    mdl_env::add(vname, false, MDL_T_INT);
    mdl_env::set(id, vname);

    machineMetFocusNode *mi = createMetricInstance(metric_name, focus,
						   true, internal);

    // calls mdl_do()

    if (!mi) {
       metric_cerr << "startCollecting for " << metric_name 
                << " failed because createMetricInstance failed" << endl;
       return(-1);
    }

    mi->id_ = id;

#if defined(MT_THREAD)
    mi->propagateId(id);
#endif

#if defined(TEST_DEL_DEBUG)
    sprintf(errorLine,"-----> in startCollecting, id=%d\n",mi->id_);
    logLine(errorLine);
#endif
    // No longer assert -- if we're retrying instrumentation, then
    // the MDN may already exist. 
    //assert(!allMIs.defines(mi->id_));
    setVal_machineMetFocusBuf(mi->id_, mi);

    const timeLength cost = mi->cost();
    mi->originalCost_ = cost;

    addCurrentPredictedCost(cost);

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
	  if (p->status() == running) {
#ifdef DETACH_ON_THE_FLY
	    if (p->reattachAndPause())
#else
	    if (p->pause())
#endif
	      procsToCont += p;
	  }
	}

        // Array to hold boolean check if any of the component mdn's
        // already have there instrementation inserted and installed 
        bool *alreadyThere = new bool[mi->components.size()];

        // initialize to false
        for (unsigned j=0; j < mi->components.size(); j++) {
	  alreadyThere[j] = false;
	}

        // Check if components' instrumentation has been inserted and 
        // installed        
        for (unsigned i=0; i < mi->components.size(); i++) {
            metricDefinitionNode *comp = mi->components[i];

	    if (comp->instrInserted() && comp->installed()) {
	      alreadyThere[i] = true;
	    }
	}

        int ret = 0;

        pd_Function *func = NULL;
	bool inserted = mi->insertInstrumentation(&func);

	// Instrumentation insertion may have failed. We should handle this case
	// better than an assert. Currently we handle the case where (on x86) we
	// couldn't insert the instrumentation due to relocation failure (deferred
	// instrumentation). If it's deferred, come back later. 

        if(inserted == false) {

          // instrumentation was deferred
          if (mi->hasDeferredInstr()) {
            ret = mi->id_;            

            // Check if we have already created the defInst object for 
            // instrumenting the function later 
            bool previouslyDeferred = false;
	    // number of functions for which instrumentation was deferred
	    int numDeferred = instrumentationToDo.size();

            for (int i=0; i < numDeferred; i++) {
              if (instrumentationToDo[i]->id() == id) {
                previouslyDeferred = true;
                instrumentationToDo[i]->failedAttempt();

                if (instrumentationToDo[i]->numAttempts() == 0) {
                  instrumentationToDo[i]->func()->setRelocatable(false);
		}

                // latest attempt to insert instrumentation failed.
                ret = 0;
	        break;
	      }
	    }

            // Create defInst object so instrumentation can be inserted later.
            // number of attempts at relocation is 1000
            if (!previouslyDeferred) {
              assert(func != NULL);
              defInst *di = new defInst(metric_name, focus, id, func, 1000);
              instrumentationToDo.push_back(di);
  	    }

	    // Don't delete the metricDefinitionNode because we want to
	    // reuse it later. 
	    return ret;
	  } // end deferred handling
	  // Instrumentation failed for an unknown reason. Very bad case.
	  else {
	    assert(false);
          // disable remnants of failed attempt to instrument function
	    
	    undefKey_machineMetFocusBuf(mi->id_);
	    assert(! isKeyDef_machineMetFocusBuf(mi->id_));
	    
	    mi->disable();
	    delete mi;
	    return ret;
	  }
	}

	mi->checkAndInstallInstrumentation();

	//
        // Now that the timers and counters have been allocated on the heap, and
	// the instrumentation added, we can manually execute instrumentation
	// we may have missed at function entry points and pre-instruction call 
	// sites which have already executed.
	//

	// Adjust mi's manuallyTrigger fields to try to execute function entry
	// and call site instrumentation according to the program stack....
	//cerr << " (startCollecting) about to call mi->adjustManuallyTrigger" << endl;

        // Do catch-up instrumentation for individual components that need it 
        for (unsigned k=0; k < mi->components.size(); k++) {
          metricDefinitionNode *comp = mi->components[k];

	  if (!alreadyThere[k]) { // trigger only if it is fresh request
	    comp->adjustManuallyTrigger();
	  }
          else {
	    if (pd_debug_catchup)
	      cerr << "SKIPPED  adjustManuallyTrigger for "
		   << comp->getFullName().string_of()
		   << ", instrumentation is already there" << endl;
	  }

  	  if (comp->anythingToManuallyTrigger()) {
	    process *theProc = comp->proc();
	    assert(theProc);

	    bool alreadyRunning = (theProc->status_ == running);

	    if (alreadyRunning) {
#ifdef DETACH_ON_THE_FLY
	      theProc->reattachAndPause();
#else
	      theProc->pause();
#endif
	    }

	    comp->manuallyTrigger(comp->id_);

	    if (alreadyRunning) {
#ifdef DETACH_ON_THE_FLY
	      theProc->detachAndContinue();
#else
	      theProc->continueProc();
#endif
	    }
	  }
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

timeLength guessCost(string& metric_name, vector<u_int>& focus) {
    // called by dynrpc.C (getPredictedDataCost())

    bool internal;
    machineMetFocusNode *mi = createMetricInstance(metric_name, focus, false, 
						   internal);

    if (!mi) {
       metric_cerr << "guessCost returning 0.0 since createMetricInstance failed" << endl;
       return timeLength::Zero();
    }

    timeLength cost = mi->cost();

    // delete the metric instance, if it is not being used 
    if (! isKeyDef_machineMetFocusBuf(mi->getMId()) && 
	mi->aggregators.size()==0) {
      metric_cerr << "guessCost deletes <" <<  mi->getFullName().string_of()
	          << ">  since it is not being used" << endl << endl ;

      for (unsigned u=0; u < mi->components.size(); u++) {
	mi->removeComponent(mi->components[u]);
      }
      mi->components.resize(0);
      delete mi;
    }

    return cost;
}

ostream& operator<<(ostream&s, const metricDefinitionNode &m) {
  s << "mdn: " << &m << ", id: " << m.getMId() << ", type: " 
    << typeStr(int(m.getMdnType())) << "\n";
  s << "name: " << m.getFullName() << "\n";
  s << "  components -----\n";
  for(unsigned i=0; i<m.components.size(); i++) {
    metricDefinitionNode* curCompPtr = m.components[i];
    s << "  " << curCompPtr << ", type: " << typeStr(curCompPtr->getMdnType()) 
      << "\n";
  }
  s << "  aggregator: " << &m.aggregator << "\n";
  s << "  local aggComponents -----\n";
  for(unsigned j=0; j<m.samples.size(); j++) {
    cerr << "  " << m.samples[j] << ", parent: " 
	 << m.samples[j]->getParentAggregator() << "\n";
  }
  s << "\n";
  for(unsigned k=0; k<m.components.size(); k++) {
    metricDefinitionNode* curCompPtr = m.components[k];
    s << *curCompPtr;
  }
  s << "\n";
  return s;
}

// in regards to the line:     mdn->setInitialActualValue(pdSample::Zero());
// This has zero for an initial value.  This is because for cpu_time and
// wall_time, we just want to total the cpu_time and wall_time for this
// process and no others (but if we want someone to get an actual cpu
// time for this program even if they start the cpu_time metric after the
// start of the process, the initial actual value could be the actual cpu
// time at the start of this metric).  For the counter metrics
// (eg. proc_calls, io_bytes), we also want zero (we have no way of
// getting the total proc_calls & io_bytes of the process before the
// metric was enabled, so we have to use zero).  However, it is possible
// that in the future we'll create a metric that it makes sense to send
// an initial actual value.

// The purpose of this call back function is to initialize the aggComponents
// of the mdn's.  It loops through all of the top level AGG mdn's and will go
// down the graph of mdn's from this AGG mdn if the flag returned by the
// function childrenMdnNeedingInitializing indicates that there are child mdn
// nodes (& their corresponding aggComponents) that haven't been initialized
// (ie. the startTime and the initialActualValue haven't been initialized).
// The childrenMdnNeedingInitializing is set when the mdn is first set up and
// also by the addPart and addParts function when an mdn gets added to a
// (parent) mdn.  In this case, all of the mdn's above the added mdn will get
// this flag set.  This callback will then know to travel down the graph to
// set the appropriate uninitialized mdn.  Because of the described data
// structure and algorithm, there should be little cost of calling this
// function in general, which occurs everytime a process is continued.  It's
// true that this could initialize a mdn for a process that's not applicable
// for a given mdn.  However, I don't think it's an issue.  Essentially,
// mdn's will get added.  This function will get called after a process
// continues and all mdn's added will get initialized with the current time.
void mdnContinueCallback(timeStamp timeOfCont) {
  dictionary_hash_iter<unsigned,metricDefinitionNode*> iter = 
                                         getIter_machineMetFocusBuf();
  for (; iter; iter++) {
    metricDefinitionNode *mdn = iter.currval();
    sampleVal_cerr << "mdnContinueCallback: comparing mdn: " << mdn 
		   << ", inserted: " << mdn->instrInserted()
		   << ", partsNeedingInit: " 
		   << mdn->childrenMdnNeedingInitializing()
    		   << ", type: " << typeStr(mdn->getMdnType()) << "\n";
    if(! mdn->instrInserted())   // only want to setup initialization times
      continue;               // if the instrumentation code has been inserted
    if(! mdn->isStartTimeSet())
      mdn->setStartTime(timeOfCont);

    if(!mdn->isInitialActualValueSet())
      mdn->setInitialActualValue(pdSample::Zero());

    if(mdn->childrenMdnNeedingInitializing())
      mdn->initChildrenMdnPartsWhereNeeded(timeOfCont, pdSample::Zero());

    mdn->partsNeedingInitializing = false;
  }
}

void metricDefinitionNode::setStartTime(timeStamp t) {
  mdnStartTime = t;
  sampleVal_cerr << "setStartTime for mdn: " << this << " to " << t
		 << ", type: " << typeStr(getMdnType()) << ", numComp: "
		 << aggregator.numComponents() << ", numSamples: "
		 << samples.size() << "\n"; 
  for(unsigned i = 0; i < aggregator.numComponents(); i++) {
    aggComponent *curComp = aggregator.getComponent(i);
    if(! curComp->isInitialStartTimeSet()) {
      // I guess could be set since order of mdn could be AGG-THR-COMP-PRIM
      // so aggComp could be attempted to be initialized twice
      sampleVal_cerr << "            for comp: " << curComp << ", time: " 
		     << t << "\n";
      curComp->setInitialStartTime(t);
    }
  }
  // an aggComponent pointing up could be new and not initialized if 
  // this is a new mdn added through addPart or addParts
  for(unsigned u=0; u<samples.size() ; u++) {
    aggComponent *curComp = samples[u];
    if(! curComp->isInitialStartTimeSet()) {
      curComp->setInitialStartTime(t);      
      sampleVal_cerr << "                     for aggComp (up) " << curComp
		     << "\n";
    }
  }
}

// Will set the initial actual value for all the components of this metric.
// This is actually sort of a kludge, since it assumes that a AGG_MDN mdn
// will have components that all have the same initial actual sample value.
// For now this is fine, since currently only internal metrics have non-zero
// initial actual values.  All these (non-internal) metrics have zero for an
// initial actual value.
void metricDefinitionNode::setInitialActualValue(pdSample s) {
  mdnInitActualVal = s;
  sampleVal_cerr << "setInitialActualValue() for mdn " << this
		 << ", numComp: " << aggregator.numComponents()
		 << ", numSamples: " << samples.size() << "\n";
  for(unsigned i = 0; i < aggregator.numComponents(); i++) {
    aggComponent *curComp = aggregator.getComponent(i);
    if(! curComp->isInitialActualValueSet()) {
      // I guess could be set since order of mdn could be AGG-THR-COMP-PRIM
      // so aggComp could be attempted to be initialized twice
      sampleVal_cerr << "                       for aggComp " << curComp 
		     << "\n";
      curComp->setInitialActualValue(s);
    }
  }
  // an aggComponent pointing up could be new and not initialized if 
  // this is a new mdn added through addPart or addParts
  for(unsigned u=0; u<samples.size() ; u++) {
    aggComponent *curComp = samples[u];
    if(! curComp->isInitialActualValueSet()) {
      curComp->setInitialActualValue(s);
      sampleVal_cerr << "                     for aggComp (up) " << curComp
		     << "\n";
    }
  }
}

void metricDefinitionNode::initChildrenMdnPartsWhereNeeded(timeStamp t, 
							   pdSample s) {
  sampleVal_cerr << "setChildrenStartTimeWhereNeeded- " << this << "\n";
  for(unsigned j=0; j<components.size(); j++) {
    metricDefinitionNode *compMdn = components[j];
    sampleVal_cerr << "     compMdn: " << compMdn << ", initTimeSet: "
	 << compMdn->isStartTimeSet() << "\n";

    if(!compMdn->isStartTimeSet())
      compMdn->setStartTime(t);

    if(! compMdn->isInitialActualValueSet())
      compMdn->setInitialActualValue(s);
  }
  partsNeedingInitializing = false;

  // need to handle these seperately so can unset partsNeedingInitializing
  // for this mdn, there are cycles and this is the only way to 
  // stop an infinite loop.
  for(unsigned k=0; k<components.size(); k++) {
    metricDefinitionNode *compMdn = components[k];
    sampleVal_cerr << "     compMdn: " << compMdn << ", childMdnNeedingInit: "
	 << compMdn->childrenMdnNeedingInitializing() << "\n";
    
    if(compMdn->childrenMdnNeedingInitializing())
      compMdn->initChildrenMdnPartsWhereNeeded(t, s);
  }
}

void metricDefinitionNode::sendInitialActualValue(pdSample s) {
  double valToSend = static_cast<double>(s.getValue());
  tp->setInitialActualValueFE(getMId(), valToSend);
  sentInitialActualValue(true);
}

void metricDefinitionNode::updateAllAggInterval(timeLength width) {
  dictionary_hash_iter<unsigned,metricDefinitionNode*> iter =
                                     getIter_machineMetFocusBuf();
  for (; iter; iter++) {
    metricDefinitionNode *mdn = iter.currval();
    mdn->updateAggInterval(width);
  }
}

bool sampleMetFocusNode::insertInstrumentation(pd_Function **func) {
  // Loop thru "dataRequests", an array of (ptrs to) dataReqNode: Here we
  // allocate ctrs/timers in the inferior heap but don't stick in any code,
  // except (if appropriate) that we'll instrument the application's
  // alarm-handler when not shm sampling.
  //unsigned size = dataRequests.size();
  //for (u=0; u<size; u++) {
  // the following allocs an object in inferior heap and arranges for it to
  // be alarm sampled, if appropriate.  Note: this is not necessary anymore
  // because we are allocating the space when the constructor for dataReqNode
  // is called. This was done for the dyninstAPI - naim 2/18/97
  //if (!dataRequests[u]->insertInstrumentation(proc_, this))
  //  return false; // shouldn't we try to undo what's already put in?
  //}
  
  // Loop thru "instRequests", an array of instReqNode:
  // (Here we insert code instrumentation, tramps, etc. via addInstFunc())
  unsigned int inst_size = instRequests.size();

  for (unsigned u1=0; u1<inst_size; u1++) {
    // code executed later (adjustManuallyTrigger) may also manually trigger 
    // the instrumentation via inferiorRPC.
    returnInstance *retInst=NULL;
    bool deferredFlag = false;
    if (!instRequests[u1].insertInstrumentation(proc_, retInst, 
						&deferredFlag)) {
      unmarkAsDeferred();
      if (deferredFlag == true) {
	*func = dynamic_cast<pd_Function *>(const_cast<function_base *>(
                            instRequests[u1].Point()->iPgetFunction()));
	markAsDeferred();
      }

      //assert (*func != NULL);
      return false; // shouldn't we try to undo what's already put in?
    }
    if (retInst) {
      returnInsts += retInst;
    }
  }
  inserted_ = true;
  return true;
}

bool metricDefinitionNode::insertInstrumentation(pd_Function **func)
{
   // cerr << "mdn: " << this << ", insertInstrum, type: " 
   // << typeStr(getMdnType()) << "), mid: " << getMId() << ", inserted: " 
   // << inserted_ << "\n";
   // returns true iff successful
   if (instrInserted()) {
      return true;
   }
   
   if(isTopLevelMDN()) {
      unsigned c_size = components.size();
      bool aCompFailedToInsert = false;
      for (unsigned u=0; u<c_size; u++)
	 if (!components[u]->insertInstrumentation(func)) {
	    //assert (*func != NULL);
	    aCompFailedToInsert = true;
	 }
      if(aCompFailedToInsert) return false;
      inserted_ = true;
   }
   else if (mdn_type_ == COMP_MDN) 
   { // similar to agg case
      
      // PAUSE inferior process ONCE FOR ALL PRIMITIVES
      bool needToCont = proc_->status() == running;
#ifdef DETACH_ON_THE_FLY
      bool res = proc_->reattachAndPause();
#else
      bool res = proc_->pause();
#endif
      if (!res) {
	 cerr << "returning since pause failed\n";
	 return false;
      }
      
      unsigned c_size = components.size();
      // mark remaining prim. components as deferred if we come upon
      // one deferred component
      bool aCompWasDeferred = false;
      for (unsigned u1=0; u1<c_size; u1++) {
	 sampleMetFocusNode *primNode = 
	    dynamic_cast<sampleMetFocusNode*>(components[u1]);
	 primNode->unmarkAsDeferred();
	 if(aCompWasDeferred == true)
	    primNode->markAsDeferred();
	 else {
	    if (!primNode->insertInstrumentation(func)) {
	       //assert (*func != NULL);
	       if(primNode->hasDeferredInstr()) {
		  aCompWasDeferred = true;
	       }
	    }
	 }
      }
      if(aCompWasDeferred) {
	 return false;
      }
      inserted_ = true;
      if (needToCont) {
#ifdef DETACH_ON_THE_FLY
	 proc_->detachAndContinue();
#else
	 proc_->continueProc();
#endif
      }      
   }
   else {
      // shouldn't get a PRIM or THR metFocus here
      assert(false);
   }
   return(true);
}

bool metricDefinitionNode::instrInserted(void) {
  bool isSpecialHangingThrType = (getMdnType()==THR_LEV && 
				  components.size()>0 &&
				  components[0]->getMdnType()!=COMP_MDN);
  if(getMdnType() == THR_LEV && !isSpecialHangingThrType) {
    // the code for a THR_LEV node has been inserted if the instr code
    // contained within it's parent PRIM mdn has been inserted
    sampleMetFocusNode *prim = NULL;
    for(unsigned i=0; i<aggregators.size(); i++) {
      if(aggregators[i]->getMdnType() == PRIM_MDN) {
	prim = dynamic_cast<sampleMetFocusNode*>(aggregators[i]);
      }
    }
    assert(prim != NULL);
    return prim->instrInserted();
  }
  return inserted_;
}


bool sampleMetFocusNode::needToWalkStack() const {
   for (unsigned u1=0; u1<returnInsts.size(); u1++) {
      if (returnInsts[u1]->needToWalkStack())
	 return true;
   }
   return false;
}

// this function checks if we need to do stack walk.  If all returnInstance's
// overwrite only 1 instruction, no stack walk necessary
bool metricDefinitionNode::needToWalkStack() const
{
  assert(COMP_MDN == mdn_type_);
  for (unsigned u=0; u<components.size(); u++) {
     sampleMetFocusNode *primNode = 
	dynamic_cast<sampleMetFocusNode*>(components[u]);
     if (primNode->needToWalkStack())
	return true;
  }
  return false;
}


bool metricDefinitionNode::checkAndInstallInstrumentation() {
   // Patch up the application to make it jump to the base trampoline(s) of
   // this metric.  (The base trampoline and mini-tramps have already been
   // installed in the inferior heap).  We must first check to see if it's
   // safe to install by doing a stack walk, and determining if anything on
   // it overlaps with any of our desired jumps to base tramps.  The key
   // variable is "returnsInsts", which was created for us when the base
   // tramp(s) were created.  Essentially, it contains the details of how
   // we'll jump to the base tramp (where in the code to patch, how many
   // instructions, the instructions themselves).  Note that it seems this
   // routine is misnamed: it's not instrumentation that needs to be
   // installed (the base & mini tramps are already in place); it's just the
   // last step that is still needed: the jump to the base tramp.  If one or
   // more can't be added, then a TRAP insn is inserted in the closest common
   // safe return point along the stack walk, and some structures are
   // appended to the process' "wait list", which is then checked when a TRAP
   // signal arrives.  At that time, the jump to the base tramp is finally
   // done.  WARNING: It seems to me that such code isn't thread-safe...just
   // because one thread hits the TRAP, there may still be other threads that
   // are unsafe.  It seems to me that we should be doing this check again
   // when a TRAP arrives...but for each thread (right now, there's no stack
   // walk for other threads).  --ari
 
   bool needToCont = false;

   if (installed_) return(true);

   installed_ = true;

#if defined(MT_THREAD)
   //if (mdn_type_ != THR_LEV)
   if((mdn_type_ == AGG_MDN) || (mdn_type_ == THR_LEV))
#else
   if(mdn_type_ == AGG_MDN)
#endif
   {
      unsigned c_size = components.size();
      for (unsigned u1=0; u1<c_size; u1++)
	 components[u1]->checkAndInstallInstrumentation();
      // why no checking of the return value?
   }
   else if (mdn_type_ == COMP_MDN) {
	 
      // pause once for all primitives for this component
      needToCont = proc_->status() == running;
#ifdef DETACH_ON_THE_FLY
      if (!proc_->reattachAndPause())
#else
      if (!proc_->pause())
#endif
      {
	 cerr << "checkAndInstallInstrumentation -- pause failed" <<endl;
	 cerr.flush();
	 return false;
      }
      
      // only overwrite 1 instruction on power arch (2 on mips arch)
      // always safe to instrument without stack walk
      if (!needToWalkStack()) {
	 // NO stack walk necessary
	 
	 vector<Address> pc;  // empty
	 unsigned c_size = components.size();
	 
	 for (unsigned u=0; u<c_size; u++) {
	    sampleMetFocusNode *primnode =
	       dynamic_cast<sampleMetFocusNode*>(components[u]);
	    primnode->checkAndInstallInstrumentation(pc);
	    // why no checking of the return value?
	 }

      }
      else {
	 // stack walk necessary, do stack walk only ONCE for all
	 // primitives
	 // NOTE: walkStack should walk all the threads' staks! It
	 // doesn't do that right now... naim 1/28/98
	 vector<Address> pc = proc_->walkStack();
	 // ndx 0 is where the pc is now; ndx 1 is the call site;
	 // ndx 2 is the call site's call site, etc...
	 
	 // for(u_int i=0; i < pc.size(); i++){
	 //     printf("frame %d: pc = 0x%x\n",i,pc[i]);
	 // }
#ifdef WALK_ALL_STACKS
	 // deleted
#endif  // WALK_ALL_STACKS
	 
	 unsigned c_size = components.size();
	 for (unsigned u2=0; u2<c_size; u2++) {
	    sampleMetFocusNode *primnode =
	       dynamic_cast<sampleMetFocusNode*>(components[u2]);
	    primnode->checkAndInstallInstrumentation(pc);
	 // why no checking of the return value?
	 }
	 
      } // else of needToWalkStack();

      if (needToCont) {
#ifdef DETATCH_ON_THE_FLY
	 proc_->detachAndContinue();
#else
	 proc_->continueProc();
#endif
      }
   }
   else
      metric_cerr << " ### checkAndInstall: prim type, should call the other checkAndInstall! " << endl;
   
   return true;
}  

bool sampleMetFocusNode::checkAndInstallInstrumentation(vector<Address>& pc) {
    if (installed_) return(true);

    installed_ = true;

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
#if defined(MT_THREAD) 
      bool installSafe = true; 
#else
      // only overwrite 1 instruction on power arch (2 on mips arch)
      // always safe to instrument without stack walk
      // pc is empty for those didn't do a stack walk, will return safe.
      bool installSafe = returnInsts[u] -> checkReturnInstance(pc,index);
#endif
      // if unsafe, index will be set to the first unsafe stack walk ndx
      // (0 being top of stack; i.e. the current pc)

      if (!installSafe && index > max_index)
	max_index = index;

      if (installSafe) {
	returnInsts[u] -> installReturnInstance(proc_);
	delay_elm[u] = false;
      } else {
	delay_install = true;
	delay_elm[u] = true;
      }
    }

    if (delay_install) {
      // get rid of pathological cases...caused by threaded applications 
      // TODO: this should be fixed to do something smarter
      if(max_index > 0 && max_index+1 >= pc.size()){
	max_index--;
	//printf("max_index changed: %d\n",max_index);
      }
      if(max_index > 0 && pc[max_index+1] == 0){
	max_index--;
	//printf("max_index changed: %d\n",max_index);
      }
      Address pc2 = pc[max_index+1];
      for (u_int i=0; i < rsize; i++)
	if (delay_elm[i]) {
	  returnInsts[i]->addToReturnWaitingList(pc2, proc_);
	}
    }

#if defined(MT_THREAD)
    for (unsigned u=0; u<components.size(); u++) {
      assert(THR_LEV == components[u]->getMdnType());
      if (!components[u]->installed())
	components[u]->setInstalled(true);
    }
#endif

    return(true);
}

timeLength sampleMetFocusNode::cost() const {
   timeLength ret = timeLength::Zero();
  if (1 < aggregators.size())
    return ret;

  // if (originalCost_ > ret)  // > 0, already computed
  // ret = originalCost_;
  // else {
  for (unsigned u2=0; u2<instRequests.size(); u2++)
    ret += instRequests[u2].cost(proc_);
  // originalCost_ = ret;
  //}
  return ret;
}

timeLength metricDefinitionNode::cost() const {
   timeLength ret = timeLength::Zero();
#if defined(MT_THREAD)
   if (AGG_MDN == mdn_type_ || THR_LEV == mdn_type_)
#else
   if (AGG_MDN == mdn_type_)
#endif
   {
#if defined(MT_THREAD)
      if (THR_LEV == mdn_type_ && 2 < aggregators.size())   // 1 of them for thr_lev's proc_prim
	 return ret;
#endif
      unsigned c_size = components.size();
      for (unsigned u=0; u<c_size; u++) {
	 timeLength nc = components[u]->cost();
	 if (nc > ret) ret = nc;
      }
   }
   else if (COMP_MDN == mdn_type_) {
      if (1 < aggregators.size())
	 return ret;
    
      unsigned c_size = components.size();
      for (unsigned u1=0; u1<c_size; u1++) {
	 ret += components[u1]->cost();
      }
   }

   return(ret);
}

int metricDefinitionNode::unhookParent(metricDefinitionNode *parent) {
   unsigned aggr_size = aggregators.size();
   assert(aggr_size == samples.size());
   
   int u1 = -1;
   for (u1=0; u1<static_cast<int>(aggr_size); u1++) {
      if (aggregators[u1] == parent) {
	 aggregators.erase(u1);
	 samples.erase(u1);
	 break;
      }
   }// for u1
   return u1;
}

#if !defined(MT_THREAD)
void threadMetFocusNode::disable(vector<addrVecType> pointsToCheck) {
  for (unsigned u=0; u<dataRequests.size(); u++) {
    dataRequests[u]->getSampleId();
    dataRequests[u]->disable(proc_, pointsToCheck); // deinstrument
  }
}

void sampleMetFocusNode::disable() {
  static pdDebug_ostream discerr(cerr, false);
  if (numParents() == 0)
    inserted_ = false;

  assert(aggregators.size() == 0);
  // start over with the points to check, since collecting this list
  // for this PRIM_MDN's children thread nodes  
  vector<addrVecType> pointsToCheck;
  for (unsigned u1=0; u1<instRequests.size(); u1++) {
    addrVecType pointsForThisRequest =
      getAllTrampsAtPoint(instRequests[u1].getInstance());
    pointsToCheck += pointsForThisRequest;
    
    instRequests[u1].disable(pointsForThisRequest); // calls deleteInst()
  }
  if (isKeyDef_sampleMetFocusBuf(flat_name_))
    undefKey_sampleMetFocusBuf(flat_name_);

  /* disable components of aggregate metrics */
  for (int u=(int)components.size()-1; u>=0; u--) {
    threadMetFocusNode *thrnode = 
      dynamic_cast<threadMetFocusNode*>(components[u]);
    
    thrnode->unhookParent(this);

    // disable component only if it is not being shared
    if (thrnode->numParents() == 0) {
      thrnode->disable(pointsToCheck);
      
      discerr << "  deleting " << typeStr(thrnode->getMdnType())
	      << ", addr: " << (void*)thrnode << " [" << (void*)this << "]\n";
      delete thrnode; // @@
      discerr << "  done deleting " << ", addr: " << (void*)thrnode 
	      << " [" << (void*)this << "]\n";
    }
    components.erase(u);
  }
}

void metricDefinitionNode::disable()
{
  static pdDebug_ostream discerr(cerr, false);
  discerr << "mdn::disable- " << this << "(" << typeStr(getMdnType()) << "), "
	  << ", mid: " << getMId() << ", addr: [" << (void*)this
	  << "], " << getFullName() << "\n";

  // check for internal metrics
  unsigned ai_size = internalMetric::allInternalMetrics.size();
  for (unsigned t=0; t<ai_size; t++) {
    internalMetric *theIMetric = internalMetric::allInternalMetrics[t];
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
    }
  }

  if(!hasDeferredInstr() && getMdnType()==AGG_MDN)
    if (! instrInserted()) {
      discerr << "returning since !inserted, (only for non-deferred instr) ["
	      << (void*)this << "]\n";
      return;
    }

  if (numParents() == 0)
    inserted_ = false;

  if((mdn_type_ == AGG_MDN) || (mdn_type_ == COMP_MDN)) 
  {
    /* disable components of aggregate metrics */
    for (int u=(int)components.size()-1; u>=0; u--) {
      metricDefinitionNode *comp = components[u];
      comp->unhookParent(this);

      // disable component only if it is not being shared
      if(comp->numParents() == 0) {
	comp->disable();
	discerr << "  deleting " << typeStr(comp->getMdnType())
		<< ", addr: " << (void*)comp << " [" << (void*)this << "]\n";
	delete comp; // @@
	discerr << "  done deleting " << ", addr: " << (void*)comp 
		<< " [" << (void*)this << "]\n";
      }
      components.erase(u);
    }
    // we DON'T UNDEF this aggregate mdn from allMIs here, it's UNDEFed 
    // and deleted in disableDataCollection (dynrpc.C) which calls this func

    // should we UNDEF this component mdn from allMIComponents?
    // should be here and should be ok (but check) to UNDEF here
    // because it only has name equivalence (hashed by name)

    if (COMP_MDN == mdn_type_) {
      if (isKeyDef_processMetFocusBuf(flat_name_)) {
	// "proc_" is coded in flat_name_
	undefKey_processMetFocusBuf(flat_name_);
      }
    }
  }
}

#else
void threadMetFocusNode::disable(vector<addrVecType> pointsToCheck) {
   bool disable = false ;
   if (numParents()==0) {
      disable = true ;
      inserted_ = false;  // the only place to set "inserted_" to false
   }
   
   // if disable == false (aggregators size == 1, for proc_prim), delete
   // from agg_lev 
   // if disable == true (aggregators size == 0), delete from proc_prim
   
   // first case, delete from agg_lev
   
   if (disable) {
      for (unsigned u=0; u<dataRequests.size(); u++) {
	 dataRequests[u]->disable(proc_, pointsToCheck); // deinstrument
      }
   }
}

void sampleMetFocusNode::disable() {
  vector<addrVecType> pointsToCheck;
    
  for (unsigned u1=0; u1<instRequests.size(); u1++) {
    addrVecType pointsForThisRequest =
      getAllTrampsAtPoint(instRequests[u1].getInstance());
    
    pointsToCheck += pointsForThisRequest;
    
    instRequests[u1].disable(pointsForThisRequest); // calls deleteInst()
  }
  
  for (unsigned u=0; u<components.size(); u++) {
    threadMetFocusNode *thrnode = 
       dynamic_cast<threadMetFocusNode*>(components[u]);
    thrnode->unhookParent(this);

    if (thrnode->numParents() == 0) {
      thrnode->disable(pointsToCheck);
    }
  }//for u
  
  components.resize(0);
  thr_names.resize(0);
  if (isKeyDef_sampleMetFocusBuf(flat_name_)) { // THIS IS IMPORTANT
    metric_cerr << " UNDEF " << flat_name_ << " in allMIPrimitives " << endl;
    undefKey_sampleMetFocusBuf(flat_name_);
  }
}

// first call always for agg_lev mi's, the agg_lev mi will be deleted (in
// dynrpc.C) also need to undef allmiprimitives.
void metricDefinitionNode::disable()
{
   // check for internal metrics
   unsigned ai_size = internalMetric::allInternalMetrics.size();
   for (unsigned t=0; t<ai_size; t++) {
      internalMetric *theIMetric = internalMetric::allInternalMetrics[t];
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
      }
   }
   
   if(!hasDeferredInstr())
      if (!inserted_) {  // should change to instrInserted() ?
	 //cerr <<"returning since !inserted, (only for non-deferred instr)\n";
	 return;
      }
   
   if (mdn_type_ == AGG_MDN || mdn_type_ == THR_LEV) {  // case 1: AGG_MDN
      /* disable components of aggregate metrics */
      for (unsigned u=0; u<components.size(); u++) {
	 metricDefinitionNode *m = components[u];
	 int matchIndex = m->unhookParent(this);
	 assert(matchIndex != -1);
	 if (COMP_MDN == m->mdn_type_) {  // NEED TO UNDEF HERE
	    m->rmCompFlatName(static_cast<unsigned>(matchIndex));
	 }
	 
	 // disable component only if it is not being shared
	 if (m->numParents() == 0) {
	    m->disable();
	 }

	 // the above has removed the AGG_MDN from m's aggregators list
	 // in the case that m is THR_LEV, we want to do the following
	 if (THR_LEV==m->mdn_type_ && m->numParents()==1)  
	    // one for this agg_lev mi, one for its proc_prim
	    m->disable();
      }//for u
      components.resize(0);
   }
   else if (mdn_type_ == COMP_MDN) {  // case 2: COMP_MDN
      for (unsigned u=0; u<components.size(); u++) {
	 metricDefinitionNode *m = components[u];

	 m->unhookParent(this);
	 
	 assert(0 == m->comp_flat_names.size());  // m is proc_prim
	 if (m->numParents() == 0) 
	    m->disable();
      }//for u
      components.resize(0);
   }
}
#endif

// (add removePrimitive part, so not needed in removeFromAggregate)
// (not in removeFromAggregate, but in removeThisInstance)

// called in removeFromAggregate and ~metricDefinitionNode
// calls itself

// for MT_THREAD:
// this could be agg_lev(comp proc_comp or thr_lev), thr_lev(comp proc_comp)
// or proc_prim(comp thr_lev)
// so, comp is proc_comp or thr_lev
void metricDefinitionNode::removeComponent(metricDefinitionNode *comp) {
    unsigned u;
    metric_cerr << "calling removeComponent[" << this << "] on " << comp 
		<< "\n";
#if defined(MT_THREAD)
    if ( !comp ) {
      metric_cerr << "   --- removeComponent: component does not exist " << endl;
      return;
    }

    unsigned aggr_size = comp->aggregators.size();
    unsigned found = aggr_size;

    if (aggr_size == 0) {
      if (PRIM_MDN == comp->mdn_type_)
	if (isKeyDef_sampleMetFocusBuf(comp->flat_name_))
	  undefKey_sampleMetFocusBuf(comp->flat_name_);

      for (u=0; u<comp->components.size(); u++) {
	comp->removeComponent(comp->components[u]);
      }
      (comp->components).resize(0);
      delete comp;
      return;
    }

    // component has more than one aggregator. Remove this from list of aggregators
    for (u = 0; u < aggr_size; u++) {
      if (comp->aggregators[u] == this) {
	found = u;
	break;
      }
    }
    if (found == aggr_size) {
      metric_cerr << "   --- removeComponent: this not found in component's aggregators " << endl;
      return;
    }

    assert(found < aggr_size);
    assert(aggr_size == comp->samples.size());

    comp->aggregators[found] = comp->aggregators[aggr_size-1];
    comp->aggregators.resize(aggr_size-1);

    comp->samples[found] = comp->samples[aggr_size-1];
    comp->samples.resize(aggr_size-1);

    if (COMP_MDN == comp->mdn_type_) {  // NEED TO UNDEF HERE
      comp->rmCompFlatName(found);
    }
    // metric_cerr << "   --- removeComponent: this removed from component's " << found << "th aggregator " << endl;

    if (1 == aggr_size) {
      for (u=0; u<comp->components.size(); u++) {
	comp->removeComponent(comp->components[u]);
      }
      (comp->components).resize(0);

      // metric_cerr << "   --- removeCompoent: now component's aggr size == 0, delete " << endl;
      if (PRIM_MDN == comp->mdn_type_)
	if (isKeyDef_sampleMetFocusBuf(comp->flat_name_))
	  undefKey_sampleMetFocusBuf(comp->flat_name_);
      delete comp;
      return;
    }

    // newly added
    if (THR_LEV==comp->mdn_type_ && 2>=aggr_size) {
      if (AGG_MDN == comp->aggregators[0]->mdn_type_) { // first PRIM_MDN has been removed
	metric_cerr << " remove this thr_lev mn's agg_lev aggregators " << endl;

	for (u=0; u<aggr_size-1; u++) {
	  comp->samples[u]->requestRemove();
	  comp->aggregators[u]->removeFromAggregate(comp, false);
	}

	comp->aggregators.resize(0);
	comp->samples.resize(0);
      }
    }
    
    /*
    if (AGG_MDN==mdn_type_ && THR_LEV==comp->mdn_type_ && 2>=aggr_size)
      {
	if (comp->components.size()>0) {
	  metricDefinitionNode *pcomp = comp->components[0];
	  comp->components.resize(0);
	  comp->removeComponent(pcomp);
	}
      }
    */
#else
    metric_cerr << "removeComponent(" << this << "), deleting " << comp <<"\n";
    metric_cerr << "    aggregator: " << aggregator << "\n";
    
    if ( !comp ) {
      metric_cerr << "   --- removeComponent: component does not exist " << endl;
      return;
    }

    assert(comp->mdn_type_ != AGG_MDN);

    unsigned aggr_size = comp->aggregators.size();
    unsigned found = aggr_size;

    if (aggr_size == 0) {
      if (COMP_MDN == comp->mdn_type_)
	if (isKeyDef_processMetFocusBuf(comp->flat_name_))
	  undefKey_processMetFocusBuf(comp->flat_name_);

      if (PRIM_MDN == comp->mdn_type_)
	if (isKeyDef_sampleMetFocusBuf(comp->flat_name_))
	  undefKey_sampleMetFocusBuf(comp->flat_name_);

      for (u=0; u<comp->components.size(); u++) {
        comp->removeComponent(comp->components[u]);
      }
      (comp->components).resize(0);
      delete comp;
      return;
    }

    // component has more than one aggregator. Remove this from list of aggregators
    for (u = 0; u < aggr_size; u++) {
      if (comp->aggregators[u] == this) {
	found = u;
	break;
      }
    }
    if (found == aggr_size) {
      metric_cerr << "   --- removeComponent: this not found in component's aggregators " << endl;
      return;
    }

    assert(found < aggr_size);
    assert(aggr_size == comp->samples.size());

    comp->aggregators[found] = comp->aggregators[aggr_size-1];
    comp->aggregators.resize(aggr_size-1);

    comp->samples[found] = comp->samples[aggr_size-1];
    comp->samples.resize(aggr_size-1);

    if (aggr_size == 1) {
      // newly added
      for (u=0; u<comp->components.size(); u++) {
	comp->removeComponent(comp->components[u]);
      }
      (comp->components).resize(0);

      if (COMP_MDN == comp->mdn_type_)
	if (isKeyDef_processMetFocusBuf(comp->flat_name_))
	  undefKey_processMetFocusBuf(comp->flat_name_);

      if (PRIM_MDN == comp->mdn_type_)
	if (isKeyDef_sampleMetFocusBuf(comp->flat_name_))
	  undefKey_sampleMetFocusBuf(comp->flat_name_);
      delete comp;
      return;
    }
#endif
}

threadMetFocusNode::~threadMetFocusNode() {
  for (unsigned u2=0; u2<dataRequests.size(); u2++) {
    delete dataRequests[u2];
  }
  dataRequests.resize(0);
}

// call removeComponent before delete
metricDefinitionNode::~metricDefinitionNode()  
{
  assert(0 == aggregators.size());
  assert(0 == components.size());
}

processMetFocusNode::~processMetFocusNode() {
#if defined(MT_THREAD)
  if (proc_) {
    unsigned tSize = proc_->allMIComponentsWithThreads.size();
    for (unsigned u=0; u<tSize; u++) 
      if (proc_->allMIComponentsWithThreads[u] == this) {
	proc_->allMIComponentsWithThreads[u] = proc_->allMIComponentsWithThreads[tSize-1];
	proc_->allMIComponentsWithThreads.resize(tSize-1);
	break ;
      }
  }
#endif
}

void sampleMetFocusNode::cleanup_drn() {
   for (unsigned u=0; u<components.size(); u++) {
      threadMetFocusNode *thrNode = 
	 dynamic_cast<threadMetFocusNode*>(components[u]);
      thrNode->cleanup_drn();
   }
}

void threadMetFocusNode::cleanup_drn() {
    vector<addrVecType> pointsToCheck;
    for (unsigned u=0; u<dataRequests.size(); u++) {
      metric_cerr << " clean " << u << "th data request " << endl;
      dataRequests[u]->disable(proc_, pointsToCheck); // deinstrument
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

  if (batch_buffer_next == 0) {
    return;
  }


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
   timeStamp t1 = getWallTime();
#endif

   // Now let's do the actual igen call!
   tp->batchSampleDataCallbackFunc(0, copyBatchBuffer);

#ifdef FREEDEBUG
   timeStamp t2 = getWallTime();
   if (t2-t1 > 15*timeLength::sec()) {
     ostrstream errorLine;
     errorLine << "++--++ TEST ++--++ batchSampleDataCallbackFunc took " << 
       t2-t1 << ", size= << " << sizeof(T_dyninstRPC::batch_buffer_entry) << 
       ", Kbytes=", << (sizeof(T_dyninstRPC::batch_buffer_entry) * 
			copyBatchBuffer.size()/1024.0F);
     logLine(errorLine);
   }
#endif

   BURST_HAS_COMPLETED = false;
   batch_buffer_next = 0;
}

// temporary until front-end's pipeline gets converted
u_int isMetricTimeType(const string& met_name) {
  unsigned size = mdl_data::all_metrics.size();
  T_dyninstRPC::metricInfo element;
  unsigned u;
  for (u=0; u<size; u++) {
    //cerr << "checking " << met_name << " against " << mdl_data::all_metrics[u]->name_ << "\n";
    if (mdl_data::all_metrics[u]->name_ == met_name) {
      u_int mtype = mdl_data::all_metrics[u]->type_;
      return (mtype == MDL_T_PROC_TIMER || mtype == MDL_T_WALL_TIMER);
    }
  }

  unsigned isize = internalMetric::allInternalMetrics.size();
  for (u=0; u<isize; u++) {
    T_dyninstRPC::metricInfo metInfo;
    metInfo = internalMetric::allInternalMetrics[u]->getInfo();
    //cerr << "checking " << met_name << " against " << metInfo.name << "\n";
    if(met_name == metInfo.name) {
      return (metInfo.unitstype == Normalized);
    }
  }
  for (unsigned u2=0; u2< costMetric::allCostMetrics.size(); u2++) {
    T_dyninstRPC::metricInfo metInfo;
    metInfo = costMetric::allCostMetrics[u2]->getInfo();
    //cerr << "checking " << met_name << " against " << metInfo.name << "\n";
    if(met_name == metInfo.name) {
      return (metInfo.unitstype == Normalized);
    }
  }

  //  cerr << "mdl_get_type: mid " << met_name << " not found\n";
  assert(0);
  return 0;
}

// the metname is temporary, get rid of this 
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value) 
{
   // This routine is called where we used to call tp->sampleDataCallbackFunc.
   // We buffer things up and eventually call tp->batchSampleDataCallbackFunc

#ifdef notdef
   char myLogBuffer[120] ;
   sprintf(myLogBuffer, "mid %d, value %g\n", mid, value.getValue()) ;
   logLine(myLogBuffer) ;
#endif

   sampleVal_cerr << "batchSampleData - metric: " << metname.string_of() 
		  << "  mid: " << mid << ", startTimeStamp: " <<startTimeStamp
		  << ", endTimeStamp: " << endTimeStamp << "value: " 
		  << value << "\n";

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (batch_buffer_next >= SAMPLE_BUFFER_SIZE || BURST_HAS_COMPLETED)
      flush_batch_buffer();

   // Now let's batch this entry.
   T_dyninstRPC::batch_buffer_entry &theEntry = theBatchBuffer[batch_buffer_next];
   theEntry.mid = mid;
   theEntry.startTimeStamp = startTimeStamp.getD(timeUnit::sec(), 
						 timeBase::bStd());
   theEntry.endTimeStamp = endTimeStamp.getD(timeUnit::sec(),timeBase::bStd());

   double bval = static_cast<double>(value.getValue());
   if(isMetricTimeType(metname)) {
     sampleVal_cerr << metname.string_of() << " is a time metric type: normalizing\n";
     bval /= 1000000000.0;
   }
   theEntry.value = bval;

   sampleVal_cerr << ">b2 startTimeStamp d: " << theEntry.startTimeStamp
   		  << ", endTimeStamp d: " << theEntry.endTimeStamp
   		  << ", value d: " << theEntry.value << "\n";

   batch_buffer_next++;
}

//////////////////////////////////////////////////////////////////////////////
// Buffer the traces before we actually send it                            //
//      Send it when the buffers are full                                   //
//      or, send it when the last sample in the interval has arrived.       //
//////////////////////////////////////////////////////////////////////////////

const unsigned TRACE_BUFFER_SIZE = 10;
bool TRACE_BURST_HAS_COMPLETED = false;
   // set to true after a burst (after a processTraceStream(), or sampleNodes for
   // the CM5), which will force the buffer to be flushed before it fills up
   // (if not, we'd have bad response time)

vector<T_dyninstRPC::trace_batch_buffer_entry> theTraceBatchBuffer (TRACE_BUFFER_SIZE);
unsigned int trace_batch_buffer_next=0;

void flush_trace_batch_buffer(int program) {
   // don't need to flush if the batch had no data (this does happen; see
   // perfStream.C)
   if (trace_batch_buffer_next == 0)
      return;

   vector<T_dyninstRPC::trace_batch_buffer_entry> copyTraceBatchBuffer(trace_batch_buffer_next);
   for (unsigned i=0; i< trace_batch_buffer_next; i++)
      copyTraceBatchBuffer[i] = theTraceBatchBuffer[i];

   // Now let's do the actual igen call!

   tp->batchTraceDataCallbackFunc(program, copyTraceBatchBuffer);

   TRACE_BURST_HAS_COMPLETED = false;
   trace_batch_buffer_next = 0;
}

void batchTraceData(int program, int mid, int recordLength,
                     char *recordPtr)
{
   // Now let's batch this entry.
   T_dyninstRPC::trace_batch_buffer_entry &theEntry = theTraceBatchBuffer[trace_batch_buffer_next];
   theEntry.mid = mid;
   theEntry.length = recordLength;
   theEntry.traceRecord = byteArray(recordPtr,recordLength);
   trace_batch_buffer_next++;

   // We buffer things up and eventually call tp->batchTraceDataCallbackFunc

   // Flush the buffer if (1) it is full, or (2) for good response time, after
   // a burst of data:
   if (trace_batch_buffer_next >= TRACE_BUFFER_SIZE || TRACE_BURST_HAS_COMPLETED) {
      flush_trace_batch_buffer(program);
   }

}

void metricDefinitionNode::forwardSimpleValue(timeStamp start, timeStamp end,
					     pdSample value)
{
  // TODO mdc
  sampleVal_cerr << "forwardSimpleValue - st: " << start << "  end: " << end 
		 << "  value: " << value << "\n";

  assert(start >= getFirstRecordTime());
  assert(end > start);

  batchSampleData(met_, id_, start, end, value);
}

bool threadMetFocusNode::isReadyForUpdates() {
  if(! instrInserted()) {
    sampleVal_cerr << "mi " << this << " hasn't been inserted\n";
    return false;
  }
  if(! isStartTimeSet()) {
    sampleVal_cerr << "mi " << this << " doesn't have an initialized"
		   << " start time\n";
    return false;
  }
  if(! isInitialActualValueSet()) {
    sampleVal_cerr << "mi " << this << " doesn't have an initialized"
		   << " initial actual value\n";
    return false;
  }
  return true;
}

// takes an actual value as input, as opposed to a change in sample value
void threadMetFocusNode::updateValue(timeStamp sampleTime, pdSample value)
{
  assert(value >= pdSample::Zero());
 
  sampleVal_cerr << "updateValue() - mdn: " << this << ", " 
		 << getFullName() << "value: " << value << ", cumVal: " 
		 << cumulativeValue << "\n";

  if(hasDeferredInstr()) {
    sampleVal_cerr << "returning since has deferred instrumentation\n";
    return;
  }

  if (value < cumulativeValue) {
  // only use delta from last sample.
    value = cumulativeValue;
  }

  value -= cumulativeValue;
  cumulativeValue += value;
  timeStamp uninitializedTime;  // we don't know the start of the interval here
  updateWithDeltaValue(uninitializedTime, sampleTime, value);
}

// will adjust the value of the sample if the referred to mdn wasn't active
// wholly during the length of the sample
// returns false if this sample should not be "added" to the mdn

// here's an example that shows why this function is relevant:
//
//      18s992ms / 19s192ms     AGG     AGG'  (AGG'-starttime: 19s387ms)
//   (AGG-starttime: 19s592ms)   |    /  |
//                               |  /    |
//      18s992ms / 19s192ms     COMP    COMP'
//                               |       |
//                               |       |
//      un / 19s761ms           PRIM    PRIM'
//
//   AGG is a cpu/process metric-focus, AGG' is a cpu/wholeprog metric-focus
//   added later.  The time before the '/' represents the start-time of the
//   sample, the time after the '/' represents the sample-time (or end-time)
//   of the sample.  The "AGG-starttime:" represents the start time of the
//   mdn set according to mdnContinue.
//   In the above example, the sample coming out of the COMP's aggregator
//   did not occur while the AGG' was active, so this sample should be
//   skipped or not added to AGG'.

bool adjustSampleIfNewMdn(pdSample *sampleVal, timeStamp startTime,
			  timeStamp sampleTime, timeStamp mdnStartTime) {
  if(! mdnStartTime.isInitialized()) {
    sampleVal_cerr << "  skipping sample since startTime- is uninitialized\n";
    return false;
  }
  
  // the sample did not occur while (but before) the mdn was active
  if(sampleTime < mdnStartTime) {
    sampleVal_cerr << "  skipping sample since sample did not occur while"
		   << " the mdn was active\n";
    return false;
  }
  
  // interpolate the value of the sample for the time that the mdn was 
  // active during the length of the sample
  if(!startTime.isInitialized() || startTime >= mdnStartTime) {
    // the whole sample occurred while the mdn was active
    // keep the sampleVal just as it is
  } else if(startTime < mdnStartTime) {
    // the sample partially occurred while the mdn was active
    // ie. startTime < mdnStartTime < sampleTime
    // we want to determine the amount of the sample within the interval
      // that this new mdn was active
    timeLength timeActiveForNewMdn = sampleTime - mdnStartTime;
    timeLength sampleLen = sampleTime - startTime;
    assert(sampleLen >= timeActiveForNewMdn);
    double pcntIntvlActiveForNewMdn = timeActiveForNewMdn / sampleLen;
    *sampleVal = pcntIntvlActiveForNewMdn * (*sampleVal);
    sampleVal_cerr << "  using " << pcntIntvlActiveForNewMdn
		   << ", new val = " << *sampleVal << "\n";
  }
  
  return true;
}

void metricDefinitionNode::updateWithDeltaValue(timeStamp startTime,
						timeStamp sampleTime, 
						pdSample value) {
  sampleVal_cerr << "updateWithDeltaValue() - mdn: " << this << ", " 
		 << getFullName() //<< ", type: " << typeStr(getMdnType())
		 << "start: " << startTime << ", sampleTime: " << sampleTime
		 << ", value:" 
		 << value << ", cumVal: " << cumulativeValue << "\n";
  
  if(isTopLevelMDN()) {
    assert(startTime.isInitialized());
    sampleVal_cerr << "Batching st: " << startTime << ", end: " << sampleTime 
		   <<", val: " << value << "\n";
    batchSampleData(met_, id_, startTime, sampleTime, value);
    return;
  }

  unsigned numAggregators = aggregators.size();
  if(numAggregators != samples.size()) {
    cerr << "type: " << typeStr(getMdnType()) << ", numAgg: " << numAggregators
	 << ", numSamples: " << samples.size() << "\n";
  }
  assert(numAggregators == samples.size());
  //cerr << "  numAggregators: " << numAggregators << "\n";
  for (unsigned u = 0; u < numAggregators; u++) {
    aggComponent &curComp = *samples[u];
    metricDefinitionNode &curParentMdn = *aggregators[u];
    sampleVal_cerr << "  handling agg#: " << u << ", *: " << &curParentMdn
		   << ", id: " << curParentMdn.getMId() << ", mdn- " 
		   << curParentMdn.getFullName()
		   << "  mdnStartTime: " << curParentMdn.getStartTime() <<"\n";
    // For some reason the mdn graph can be like AGG->THR->COMP->PRIM->THR
    // with a connection also of AGG->COMP ...  

    // The AGG->THR I suppose works for the case where one is looking at a
    // per thread metric and the AGG->COMP->PRIM->THR works for the case
    // where the metric includes all of the threads.  I don't understand the
    // purpose though of AGG->THR->COMP.  We don't want to batch samples
    // through this path so ignore parents of THR_LEV when this mdn is a
    // COMP_MDN.
    if(mdn_type_ == COMP_MDN && curParentMdn.mdn_type_ == THR_LEV)
      continue;

    pdSample valToPass = value;
    bool shouldAddSample = adjustSampleIfNewMdn(&valToPass, startTime, 
				  sampleTime, curParentMdn.getStartTime());
    if(!shouldAddSample)   continue;

    sampleVal_cerr << "  addSamplePt (" << &curComp << ")- " << sampleTime 
		   << ", val: " << valToPass << "\n";
    curComp.addSamplePt(sampleTime, valToPass);

    curParentMdn.tryAggregation();
    sampleVal_cerr <<"  back in updateWithDeltaValue, finished handling agg#: "
		   << u << ", *: " << &curParentMdn << ", id: " 
		   << curParentMdn.getMId() << ", mdn- " 
		   << curParentMdn.getFullName() << "\n";
  }
}

void metricDefinitionNode::tryAggregation() {
    sampleInterval aggSample;
    sampleVal_cerr << "Calling aggregate for mdn: " << this << ", id: "
		   << getMId() << ", sampleAgg: " << &aggregator << "\n";
    while(aggregator.aggregate(&aggSample) == true) {
      if(isTopLevelMDN() && !sentInitialActualValue()) {
	sendInitialActualValue(aggregator.getInitialActualValue());
      }
      updateWithDeltaValue(aggSample.start, aggSample.end, aggSample.value);
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

/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(instPoint *iPoint,
                         AstNode *iAst,
                         callWhen  iWhen,
                         callOrder o) {
    point = iPoint;
    when = iWhen;
    order = o;
    instance = NULL; // set when insertInstrumentation() calls addInstFunc()
    ast = assignAst(iAst);
    assert(point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parentNode,
			     const dictionary_hash<instInstance*,instInstance*> &map) {
    instReqNode ret = instReqNode(parentNode.point, parentNode.ast, parentNode.when,
				  parentNode.order
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

   vector<Address> pointsToCheck; // is it right leaving this empty on a fork()???
   deleteInst(childInstance, pointsToCheck);

   map[parentInstance] = NULL; // since we've deleted...

   return true; // success
}

bool instReqNode::insertInstrumentation(process *theProc,
					returnInstance *&retInstance,
                                        bool *deferred) 
{
    // NEW: We may manually trigger the instrumentation, via a call to postRPCtoDo()

    // addInstFunc() is one of the key routines in all paradynd.
    // It installs a base tramp at the point (if needed), generates code
    // for the tramp, calls inferiorMalloc() in the text heap to get space for it,
    // and actually inserts the instrumentation.
    instance = addInstFunc(theProc, point, ast, when, order,
			   false, // false --> don't exclude cost
			   retInstance,
                           *deferred,
			   false // false --> do not allow recursion
			   );

    //if( !retInstance )
	//cerr << "addInstFunc returned a NULL retInstance" << endl;
    rinstance = retInstance;

    return (instance != NULL);
}

void instReqNode::disable(const vector<Address> &pointsToCheck)
{
#if defined(MT_THREAD)
    if (instance) 
      deleteInst(instance, pointsToCheck);
#else
    deleteInst(instance, pointsToCheck);
#endif
    instance = NULL;
}

instReqNode::~instReqNode()
{
    instance = NULL;
    removeAst(ast);
}

timeLength instReqNode::cost(process *theProc) const
{
    // Currently the predicted cost represents the maximum possible cost of
    // the snippet.  For instance, for the statement "if(cond) <stmtA> ..."
    // the cost of <stmtA> is currently included, even if it's actually not
    // called.  Feel free to change the maxCost call below to ast->minCost or
    // ast->avgCost if the semantics need to be changed.
    int unitCostInCycles = ast->maxCost() + getPointCost(theProc, point) +
                       getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
    // printf("unit cost = %d cycles\n", unitCostInCycles);
    timeLength unitCost(unitCostInCycles, getCyclesPerSecond());
    float frequency = getPointFrequency(point);
    timeLength value = unitCost * frequency;
    return(value);
}

extern void checkProcStatus();

bool instReqNode::triggerNow(process *theProc, int mid) {

   bool needToCont = theProc->status() == running;
#ifdef DETACH_ON_THE_FLY
   if ( !theProc->reattachAndPause() )
#else
   if ( !theProc->pause() )
#endif
     {
	   cerr << "instReqNode::triggerNow -- pause failed" << endl;
	   return false;
     }
   
   // If multithreaded, we need to trigger for each thread necessary
   for (unsigned i = 0; i < fries.size(); i++) {
#if defined(MT_THREAD)
     int thrID = fries[i];
     /*
     fprintf(stderr, "Starting instrumentation for thread %d\n", thrID);
     */
     // trigger the instrumentation

     for (unsigned i=0;i<manuallyTriggerTIDs.size();i++) {
       if (manuallyTriggerTIDs[i]==thrID) {
	 continue;
       }
     }
     // Avoid multiply triggering, since the thread IDs stored in "fries"
     // may not be unique
     manuallyTriggerTIDs += thrID;
     
#if defined(TEST_DEL_DEBUG)
     sprintf(errorLine,"***** posting inferiorRPC for mid=%d and tid=%d\n",mid,thrId);
     logLine(errorLine);
#endif
#endif

     theProc->postRPCtoDo(ast, false, // don't skip cost
			  instReqNode::triggerNowCallbackDispatch, this,
#if defined(MT_THREAD)
			  mid,
			  thrID,
			  false
#else
                          mid
#endif
			  ); //false --> regular RPC, true-->SAFE RPC
     metric_cerr << "   inferiorRPC has been launched for this thread. " << endl;
     rpcCount = 0;
     
     if (pd_debug_catchup) {
       cerr << "launched catchup instrumentation, waiting rpc to finish ..." << endl;
     }
     // Launch RPC now since we're playing around with timers and such.
     // We should really examine the stack to be sure that it's safe, but
     // for now assume it is.
     do { 
       // Make sure that we are not currently in an RPC to avoid race
       // conditions between catchup instrumentation and waitProcs()
       // loops
       if ( !theProc->isRPCwaitingForSysCallToComplete() ) 
	 theProc->launchRPCifAppropriate(false, false); 
       checkProcStatus(); 
       
     } while ( !rpcCount && theProc->status() != exited );
     if ( pd_debug_catchup ) {
       metric_cerr << "catchup instrumentation finished ..." << endl;
     }
   }     
   if( needToCont && (theProc->status() != running)) {
#ifdef DETACH_ON_THE_FLY
     theProc->detachAndContinue();
#else
     theProc->continueProc();
#endif
   }
   else if ( !needToCont && theProc->status()==running ) {
#ifdef DETACH_ON_THE_FLY
     theProc->reattachAndPause();
#else
     theProc->pause();
#endif
   }
   return true;
}

void instReqNode::triggerNowCallback(void * /*returnValue*/ ) {
	++rpcCount;
}


bool instReqNode::triggeredInStackFrame(pd_Function *stack_fn,
				      Address pc,
				      process *p)
{
    return p->triggeredInStackFrame(point, stack_fn, pc, when, order);
}

/* ************************************************************************* */

#if defined(MT_THREAD)
int sampledShmIntCounterReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}
#endif

#if defined(MT_THREAD)
sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(
                                pdThread *thr, rawTime64 iValue, 
				int iCounterId, threadMetFocusNode *thrmf,
				bool computingCost, bool doNotSample,
				unsigned p_allocatedIndex,
				unsigned p_allocatedLevel)
#else
sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(
				rawTime64 iValue, int iCounterId, 
				threadMetFocusNode *thrmf, bool computingCost,
				bool doNotSample)
#endif
{
   theSampleId = iCounterId;
   initialValue = iValue;
#if defined(MT_THREAD)
   thr_ = thr;
#endif

   // The following fields are NULL until insertInstrumentation()
#if defined(MT_THREAD)
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UINT_MAX;
   allocatedLevel = UINT_MAX;
#endif

   position_=0;

   if (!computingCost) {
     bool isOk=false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, thrmf->proc(), thrmf, doNotSample);
#else
     isOk = insertShmVar(thrmf->proc(), thrmf, doNotSample);
#endif
     assert(isOk); 
   }
}

sampledShmIntCounterReqNode::
sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
			    process *childProc, threadMetFocusNode *thrmf,
			    int iCounterId, const process *parentProc)
{
   // a dup() routine (call after a fork())
   // Assumes that "childProc" has been copied already (e.g., the shm seg was copied).

   // Note that the index w/in the inferior heap remains the same, so setting the
   // new inferiorCounterPtr isn't too hard.  Actually, it's trivial, since other code
   // ensures that the new shm segment is placed in exactly the same virtual mem loc
   // as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already copied the
   // actual data; we need to fill in new meta-data (new houseKeeping entries).

   this->allocatedIndex = src.allocatedIndex;
   this->allocatedLevel = src.allocatedLevel;

   this->theSampleId = iCounterId;  // this is different from the parent's value
   this->initialValue = src.initialValue;

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(0,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(0,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     // write to the raw item in the inferior heap:
     intCounter *localCounterPtr = (intCounter *) theTable.index2LocalAddr(0,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     localCounterPtr->value = initialValue;
     localCounterPtr->id.id = theSampleId;
   }

   // write HK for this intCounter:
   // Note: we don't assert anything about thrmf->getMId(), because that id
   // has no relation to the ids we work with (theSampleId).  In fact, we
   // (the sampling code) just don't ever care what thrmf->getMId() is.
   assert(theSampleId >= 0);
   intCounterHK iHKValue(theSampleId, thrmf);

      // the mi differs from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkIntCounter(allocatedIndex, allocatedLevel, 
					    iHKValue);
   position_=0;
}

dataReqNode *
sampledShmIntCounterReqNode::dup(process *childProc, threadMetFocusNode *thrmf,
			         int iCounterId,
	                  const dictionary_hash<instInstance*,instInstance*> &)
  const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmIntCounterReqNode *tmp;
   tmp = new sampledShmIntCounterReqNode(*this, childProc, thrmf, iCounterId,
					 childProc->getParent());
      // fork ctor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmIntCounterReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmIntCounterReqNode::insertShmVar(process *theProc,
#endif
				  threadMetFocusNode *thrmf, bool doNotSample)
{
   // Remember counterPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the intCounter in the inferior heap
   intCounter iValue;
   iValue.id.id = this->theSampleId;
   iValue.value = this->initialValue;

   intCounterHK iHKValue(this->theSampleId, thrmf);

   superTable &theTable = theProc->getTable();
#if defined(MT_THREAD)
   if (thr==NULL) thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocIntCounter(thr_pos, iValue, iHKValue, this->allocatedIndex, this->allocatedLevel, doNotSample))
     return false;  // failure
#else
   if (!theTable.allocIntCounter(iValue, iHKValue, this->allocatedIndex, this->allocatedLevel, doNotSample))
     return false;  // failure
#endif
   return true; // success
}

void sampledShmIntCounterReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();

  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 0, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActCounters_is>0) theProc->numOfActCounters_is--;
#else
  theTable.makePendingFree(0,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}

/* ************************************************************************* */

#if defined(MT_THREAD)
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(
                                pdThread *thr, int iCounterId,
				threadMetFocusNode *thrmf, bool computingCost,
				unsigned p_allocatedIndex,
				unsigned p_allocatedLevel)
{
#else
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(
                                int iCounterId, threadMetFocusNode *thrmf,
				bool computingCost)
{
#endif
   theSampleId = iCounterId;

   // The following fields are NULL until insertShmVar():
#if defined(MT_THREAD)
   thr_ = thr;
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UINT_MAX;
   allocatedLevel = UINT_MAX;
#endif

   position_=0;

   if (!computingCost) {
     bool isOk = false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, thrmf->proc(), thrmf);
#else
     isOk = insertShmVar(thrmf->proc(), thrmf);
#endif
     assert(isOk); 
   }
}

sampledShmWallTimerReqNode::
sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src,
			   process *childProc,
			   threadMetFocusNode *thrmf,
			   int iCounterId, const process *parentProc)
{
   // a dup()-like routine; call after a fork().
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting
   // the new inferiorTimerPtr isn't too hard.  Actually, it's trivial, since
   // other code ensures that the new shm segment is placed in exactly the
   // same virtual mem loc as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already
   // copied the actual data; we need to fill in new meta-data (new
   // houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;

   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(1,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(1,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value in the inferior heap:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) theTable.index2LocalAddr(1,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     const tTimer *srcTimerPtr = (const tTimer *) childProc->getParent()->getTable().index2LocalAddr(1,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);

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
        localTimerPtr->start = getRawWallTime();
   }

   // write new HK for this tTimer: 
   // Note: we don't assert anything about thrmf->getMId(), because that id
   // has no relation to the ids we work with (theSampleId).  In fact, we
   // (the sampling code) just don't ever care what thrmf->getMId() is.
   assert(theSampleId >= 0);
   wallTimerHK iHKValue(theSampleId, thrmf, timeLength::Zero()); 
      // the mi should differ from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkWallTimer(allocatedIndex, allocatedLevel, 
					   iHKValue);

   position_=0;
}

dataReqNode *
sampledShmWallTimerReqNode::dup(process *childProc,
				threadMetFocusNode *thrmf,
				int iCounterId,
				const dictionary_hash<instInstance*,instInstance*> &
				) const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmWallTimerReqNode *tmp;
   tmp = new sampledShmWallTimerReqNode(*this, childProc, thrmf, iCounterId,
					childProc->getParent());
      // fork constructor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmWallTimerReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmWallTimerReqNode::insertShmVar(process *theProc,
#endif
					       threadMetFocusNode *thrmf, bool)
{
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   wallTimerHK iHKValue(this->theSampleId, thrmf, timeLength::Zero());

   superTable &theTable = theProc->getTable();

#if defined(MT_THREAD)
   thr_ = thr;
   if (thr==NULL) thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocWallTimer(thr_pos, iValue, iHKValue, 
				this->allocatedIndex, this->allocatedLevel))
#else
   if (!theTable.allocWallTimer(iValue, iHKValue, this->allocatedIndex, 
				this->allocatedLevel))
#endif
      return false; // failure

   return true;
}

void sampledShmWallTimerReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();
  
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 1, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActWallTimers_is>0) theProc->numOfActWallTimers_is--;
#else
  theTable.makePendingFree(1,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}

/* ****************************************************************** */

#if defined(MT_THREAD)
int sampledShmProcTimerReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}
#endif

#if defined(MT_THREAD)
sampledShmProcTimerReqNode::sampledShmProcTimerReqNode(
                              pdThread *thr, int iCounterId,
			      threadMetFocusNode *thrmf, bool computingCost,
			      unsigned p_allocatedIndex,
			      unsigned p_allocatedLevel) {
#else
sampledShmProcTimerReqNode::sampledShmProcTimerReqNode(
			      int iCounterId, threadMetFocusNode *thrmf,
			      bool computingCost) {
#endif
   theSampleId = iCounterId;

   // The following fields are NULL until insertInstrumentatoin():
#if defined(MT_THREAD)
   thr_ = thr;
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UINT_MAX;
   allocatedLevel = UINT_MAX;
#endif

   position_=0;

   if (!computingCost) {
     bool isOk=false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, thrmf->proc(), thrmf);
#else
     isOk = insertShmVar(thrmf->proc(), thrmf);
#endif
     assert(isOk); 
   }
}

sampledShmProcTimerReqNode::
sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src,
			   process *childProc,
			   threadMetFocusNode *thrmf,
			   int iCounterId, const process *parentProc)
{
   // a dup()-like routine; call after a fork()
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting
   // the new inferiorTimerPtr isn't too hard.  Actually, it's trivial, since
   // other code ensures that the new shm segment is placed in exactly the
   // same virtual mem loc as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already
   // copied the actual data; we need to fill in new meta-data (new
   // houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;
   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(2,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(2,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) theTable.index2LocalAddr(2,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     const tTimer *srcTimerPtr = (const tTimer *) childProc->getParent()->getTable().index2LocalAddr(2,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);

     localTimerPtr->total = 0;
     localTimerPtr->counter = srcTimerPtr->counter;
     localTimerPtr->id.id   = theSampleId;
     localTimerPtr->protector1 = srcTimerPtr->protector1;
     localTimerPtr->protector2 = srcTimerPtr->protector2;

     if (localTimerPtr->counter == 0) {
        // inactive timer...this is the easy case to copy
        localTimerPtr->start = 0; // undefined, really
     } else {
        // active timer...don't copy the start time from the source...make it 'now'
#if defined(MT_THREAD)
        localTimerPtr->start = childProc->getRawCpuTime(localTimerPtr->lwp_id);
#else
        localTimerPtr->start = childProc->getRawCpuTime(-1);
#endif
     }
   }

   // Write new HK for this tTimer:

   // Note: we don't assert anything about thrmf->getMId(), because that id
   // has no relation to the ids we work with (theSampleId).  In fact, we
   // (the sampling code) just don't ever care what thrmf->getMId() is.
   assert(theSampleId >= 0);
   processTimerHK iHKValue(theSampleId, thrmf, timeLength::Zero());
      // the mi differs from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkProcTimer(allocatedIndex, allocatedLevel, 
					   iHKValue);

   position_=0;
}

dataReqNode *sampledShmProcTimerReqNode::dup(
                 process *childProc, threadMetFocusNode *thrmf, int iCounterId,
		 const dictionary_hash<instInstance*,instInstance*> &) const
{
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmProcTimerReqNode *tmp;
   tmp = new sampledShmProcTimerReqNode(*this, childProc, thrmf, iCounterId, 
					childProc->getParent());
      // fork constructor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmProcTimerReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmProcTimerReqNode::insertShmVar(
				         process *theProc,
#endif
				         threadMetFocusNode *thrmf, bool)
{
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   processTimerHK iHKValue(this->theSampleId, thrmf, timeLength::Zero());

   superTable &theTable = theProc->getTable();
#if defined(MT_THREAD)
   if (thr==NULL) 
     thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocProcTimer(thr_pos, iValue, iHKValue, 
				this->allocatedIndex, this->allocatedLevel))
#else
   if (!theTable.allocProcTimer(iValue, iHKValue, this->allocatedIndex,
				this->allocatedLevel))
#endif
      return false; // failure

   return true;
}

void sampledShmProcTimerReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();
  
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 2, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActProcTimers_is>0) theProc->numOfActProcTimers_is--;
#else
  theTable.makePendingFree(2,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}

/* **************************** */

void reportInternalMetrics(bool force) 
{
  if (isApplicationPaused())
    return; // we don't sample when paused (is this right?)

  // see if we have a sample to establish time base.
  if (!isInitFirstRecordTime())
    return;

  static timeStamp lastSampleTime;
  const  timeStamp now = getWallTime();

  if(! lastSampleTime.isInitialized()) {
    lastSampleTime = now;
    return;
  }

  //  check if it is time for a sample
  if (!force && now < lastSampleTime + getCurrSamplingRate())
    return;

  unsigned ai_size = internalMetric::allInternalMetrics.size();
  for (unsigned u2=0; u2<ai_size; u2++) {
    internalMetric *theIMetric = internalMetric::allInternalMetrics[u2];
    // Loop thru all enabled instances of this internal metric...
    
    for (unsigned v=0; v < theIMetric->num_enabled_instances(); v++) {
      internalMetric::eachInstance &theInst =theIMetric->getEnabledInstance(v);
      theInst.updateValue(now, theInst.calcValue());
    }
  }
  lastSampleTime = now;
}

void disableAllInternalMetrics() {
    flush_batch_buffer();
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

// will set the partsNeedingInitializing flag in current mdn and in all 
// parent mdns
void metricDefinitionNode::notifyMdnsOfNewParts() {
  sampleVal_cerr << "notifyParentOfMdnsOfNewParts- " << this << "\n";
  partsNeedingInitializing = true;
  for (unsigned u = 0; u < aggregators.size(); u++) {
    metricDefinitionNode &curParentMdn = *aggregators[u];
    // if parent already notified, don't need to continue up tree
    if(! curParentMdn.childrenMdnNeedingInitializing()) {
      curParentMdn.notifyMdnsOfNewParts();
    }
  }
}

bool metricDefinitionNode::hasDeferredInstr() {
  bool retVal = false;
  if(getMdnType() == AGG_MDN || getMdnType() == COMP_MDN) {
    unsigned csize = components.size();
    bool hasDeferredComp = false;
    for(unsigned i=0; i<csize; i++) {
      if(components[i]->hasDeferredInstr()) {
	hasDeferredComp = true;
	break;
      }
    }
    retVal = hasDeferredComp;
  } else if(getMdnType() == PRIM_MDN || getMdnType() == THR_LEV) {
    retVal = instrDeferred_;
  }
  return retVal;
}

// ======================
void metricDefinitionNode::addPart(metricDefinitionNode* mi)
{
    components += mi;
    mi->aggregators += this; 
    aggComponent *newAggComp = aggregator.newComponent();
    mi->samples += newAggComp;
    sampleVal_cerr << "addPart- " << this << " adding mdn " << mi 
		   << " aggComp: " << newAggComp << "\n";
    notifyMdnsOfNewParts();
    // it appears that the application doesn't need to be stopped when
    // threads are captured and added by daemon, so emulate the starting
    // of the "process" and thread ourself
    mdnContinueCallback(getWallTime());
}

// for adding constraints primitives
void metricDefinitionNode::addPartDummySample(metricDefinitionNode* mi)
{
    components += mi;
    mi->aggregators += this; 
    mi->samples += DummyAggSample.newComponent();
}

#if defined(MT_THREAD)
void metricDefinitionNode::addParts(vector<threadMetFocusNode*>& parts)
{
  sampleVal_cerr << "addParts- " << this << "\n";
  for (unsigned i=0;i<parts.size();i++) {
    metricDefinitionNode *mi = static_cast<metricDefinitionNode*>(parts[i]);
    components += mi;
    mi->aggregators += this; 
    aggComponent *newAggComp = aggregator.newComponent();
    mi->samples += newAggComp;
    sampleVal_cerr << "         adding mdn " << mi 
		   << " aggComp: " << newAggComp << "\n";
  }
  notifyMdnsOfNewParts();
  // it appears that the application doesn't need to be stopped when
  // threads are captured and added by daemon, so emulate the starting
  // of the "process" and thread ourself
  mdnContinueCallback(getWallTime());
}

void processMetFocusNode::addThread(pdThread *thr)
{
  int tid;
  assert(thr);
  assert(mdn_type_ == COMP_MDN) ;
  tid = thr->get_tid();

  metric_cerr << "+++++ adding thread " << tid << " to component " << flat_name_;

  /*  
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"+++++ adding thread %d to component %s",tid,flat_name_);
  cerr << errorLine << endl ;
#endif
  */

  string pretty_name = string(thr->get_start_func()->prettyName().string_of()) ;
  string thrName = string("thr_") + tid + string("{") + pretty_name + string("}");

  metricDefinitionNode * metric_prim = getMetricPrim();
  threadMetFocusNode * thr_mn = 
     dynamic_cast<metricDefinitionNode*>(metric_prim)->getThrComp(thrName);
  
  if (thr_mn) {
    cerr << "+++ metric already exist in "  << flat_name_.string_of() << "::addThread, " << endl;
    // << component_flat_name_thr.string_of() << endl ;
    return;
  }

  // component hasn't been defined previously. If it has, then we will have
  // reused it - naim

  // use stuff memorized in COMP_MDN: type_thr, temp_ctr_thr, flag_cons_thr and base_use_thr

  if ( base_use_thr.size() == 0 ) {
    // allocate constraints that is used as flags
    unsigned flag_size = flag_cons_thr.size(); // could be zero
    // for flags only
    assert(flag_size+1 == components.size());

    if (flag_size > 0) { 
      //unsigned thr_size = components[flag_size]->components.size();
      for (unsigned fs=0; fs<flag_size; fs++) {
	metricDefinitionNode * cons_prim = components[fs];

	// the following tests if flag prims have already been updated
	// assume one thread is added at a time and at that time all related metrics are updated
	// if (thr_size != cons_prim->components.size()) {
	// assert(thr_size+1 == cons_prim->components.size());
	// continue;
	// }
	sampleMetFocusNode *cons_prim_alt = dynamic_cast<sampleMetFocusNode*>(
							        cons_prim);
	if (cons_prim_alt->getThrComp(thrName)) {
	  // assert(thr_size+1 == cons_prim->components.size());
	  continue;
	}

	// if (!(flag_cons_thr[fs]->replace()))
	string cons_met_thr(cons_prim->getMetName());
	vector< vector<string> > cons_component_focus_thr(cons_prim->getComponentFocus());
	vector< vector<string> > cons_focus_thr(cons_prim->getFocus());

	for (unsigned i=0;i<cons_component_focus_thr.size();i++) {
	  if (cons_component_focus_thr[i][0] == "Machine")
	    cons_component_focus_thr[i] += thrName;
	}
	for (unsigned j=0;j<cons_focus_thr.size();j++) {
	  if (cons_focus_thr[j][0] == "Machine")
	    cons_focus_thr[j] += thrName;
	}
	string cons_flat_name_thr = metricAndCanonFocus2FlatName(cons_met_thr,cons_component_focus_thr);

	thr_mn = new threadMetFocusNode(proc_,
					  cons_met_thr,
					  cons_focus_thr,
					  cons_component_focus_thr,
					  cons_flat_name_thr,
					  cons_prim->getAggOp());
	assert(thr_mn);

	string tmp_tname(thrName);
	components[fs]->addThrName(tmp_tname);
	components[fs]->addPart(static_cast<metricDefinitionNode*>(thr_mn));

	thr_mn->setInserted(true);
	thr_mn->setInstalled(true);

	thr_mn->addSampledIntCounter(thr, 0, computingCost_thr, true) ; // should be false?
      }
    }
  }

  // for metric only (or base_use)
  // if base_use_thr != NULL;  NEED TO CHECK if base_use_thr[?]->replace() ??
  // add to components[components.size()-1]

  string met_thr(metric_prim->getMetName());
  vector< vector<string> > component_focus_thr(
					    metric_prim->getComponentFocus());
  vector< vector<string> > focus_thr(metric_prim->getFocus());

  for (unsigned i=0;i<component_focus_thr.size();i++) {
    if (component_focus_thr[i][0] == "Machine")
      component_focus_thr[i] += thrName;
  }
  for (unsigned j=0;j<focus_thr.size();j++) {
    if (focus_thr[j][0] == "Machine")
      focus_thr[j] += thrName;
  }
  string component_flat_name_thr = metricAndCanonFocus2FlatName(met_thr,component_focus_thr);

  thr_mn = new threadMetFocusNode(proc_,
				    met_thr,
				    focus_thr,
				    component_focus_thr,
				    component_flat_name_thr,
				    metric_prim->getAggOp());

  assert(thr_mn);

  string tmp_tname(thrName);
  metric_prim->addThrName(tmp_tname);
  metric_prim->addPart(static_cast<metricDefinitionNode*>(thr_mn));

  thr_mn->setInserted(true);
  thr_mn->setInstalled(true);

  // Create the timer, counter for this thread
  extern dataReqNode *create_data_object(unsigned, threadMetFocusNode *,
					 bool, pdThread *);
  create_data_object(type_thr, thr_mn, computingCost_thr, thr);

  // Create the temporary counters - are these useful
  if (temp_ctr_thr) {
    unsigned tc_size = temp_ctr_thr->size();
    for (unsigned tc=0; tc<tc_size; tc++) {
      // "true" means that we are going to create a sampled int counter but
      // we are *not* going to sample it, because it is just a temporary
      // counter - naim 4/22/97
      // By default, the last parameter is false - naim 4/23/97
      thr_mn->addSampledIntCounter(thr, 0, computingCost_thr, true);
    }
  }

  propagateId(id_);

  //
  //
  //

  adjustManuallyTrigger0(tid);

  if (anythingToManuallyTrigger()) {
    process *theProc = proc_;
    assert(theProc);
    
    bool needToContinue = (theProc->status_ == running);
    bool ok;
    if (needToContinue) {
#ifdef DETACH_ON_THE_FLY
      ok = theProc->reattachAndPause();
#else
      ok = theProc->pause();
#endif
    }

    manuallyTrigger(id_);

    if (needToContinue) {
      // the continue will trigger our code
#ifdef DETACH_ON_THE_FLY
      ok = theProc->detachAndContinue();
#else
      ok = theProc->continueProc();
#endif
    }
  }

}

threadMetFocusNode *metricDefinitionNode::getThrComp(string tname) {
  assert(mdn_type_ == PRIM_MDN);
  unsigned csize = components.size();
  assert(csize == thr_names.size());
  
  for (unsigned u=0; u<csize; u++)
    if (tname == thr_names[u]) {
      metricDefinitionNode *retNode = components[u];
      return dynamic_cast<threadMetFocusNode*>(retNode);
    }
  return NULL;
}

void processMetFocusNode::deleteThread(pdThread *thr)
{
  assert(mdn_type_ == COMP_MDN);
  int tid;
  assert(thr);
  tid = thr->get_tid();

  for (unsigned u=0; u<components.size(); u++) {
    metricDefinitionNode * prim = components[u];
    assert(prim->getMdnType() == PRIM_MDN);

    unsigned tsize = prim->getComponents().size();
    assert(tsize == prim->getThrNames().size());

    string pretty_name = string(thr->get_start_func()->prettyName().string_of()) ;
    string thrName = string("thr_") + tid + string("{") + pretty_name + string("}");
    sampleMetFocusNode *prim_alt = dynamic_cast<sampleMetFocusNode*>(prim);
    threadMetFocusNode *thr_mi = prim_alt->getThrComp(thrName);

    if (thr_mi) {
      assert(thr_mi->getMdnType() == THR_LEV);

      thr_mi->removeThisInstance();  // removeThisInstance
      // delete thr_mi
    }
  }
  
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"----- deleting thread %d to component %s\n",tid,flat_name_.string_of());
  logLine(errorLine);
#endif

}

int sampledShmWallTimerReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}

#endif //MT_THREAD

Address sampledShmIntCounterReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UINT_MAX || allocatedLevel == UINT_MAX) {
      return 0;
    }    
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(0,0,allocatedIndex,allocatedLevel));
}

Address sampledShmWallTimerReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UINT_MAX || allocatedLevel == UINT_MAX) {
      return 0;
    }
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(1,0,allocatedIndex,allocatedLevel));
}

Address sampledShmProcTimerReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UINT_MAX || allocatedLevel == UINT_MAX) {
      return 0;
    }
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(2,0,allocatedIndex,allocatedLevel));
}


//#if !defined(MT_THREAD)
// going to dyninstAPI/src/instPoint-alpha, mips, power, sparc, x86.h

// instPoint-alpha.h, instPoint-power.h, instPoinst-sparc.h
// addr_ in instPoint-mips.h and instPoint-x86.h
// moved to individual files
/*
bool instPoint::match(instPoint *p)
{
  if (this == p)
    return true;

  // should we check anything else?
  if (addr == p->addr)
    return true;
  
  return false;
}
*/


#if defined(MT_THREAD)
bool level_index_match(unsigned level1, unsigned level2,
		       unsigned index1, unsigned index2,
		       vector<dataReqNode*> &data_tuple1, // initialization?
		       vector<dataReqNode*> &data_tuple2,
		       vector<dataReqNode*> datareqs1,
		       vector<dataReqNode*> datareqs2)
{
  // defined in mdl.C
  extern int index_in_data(unsigned lev, unsigned ind, vector<dataReqNode*>& data);

  int match_index1 = index_in_data(level1, index1, data_tuple1);
  int match_index2 = index_in_data(level2, index2, data_tuple2);
  
  // CHECK IS THE SAME AS IN variable_address_match
  // v1 get matched in data_tuple1
  if (match_index1 >= 0) {
    // v2 get matched in data_tuple2
    if (match_index2 >= 0) {
      return (match_index1 == match_index2);
    }
    // v2 not get matched in data_tuple2
    else {
      return false;
    }
  }
  
  // now v1 not matched in data_tuple1
  match_index1 = index_in_data(level1, index1, datareqs1);
  match_index2 = index_in_data(level2, index2, datareqs2);
  

  // CHECK IS THE SAME AS IN variable_address_match
  // v1 get matched in datareqs1
  if (match_index1 >= 0) {
    // v2 get matched in datareqs2
    if (match_index2 >= 0) {
      // get a new match pair @@
      data_tuple1 += datareqs1[match_index1];
      data_tuple2 += datareqs2[match_index2];
      return true;
    }
    // v2 not get matched in datareqs2
    else {
      return false;
    }
  }
  
  // neither v1 or v2 get matched in their data tuples or reqs, respectively
  return ((level1 == level2) && (index1 == index2));
}
#else
bool variable_address_match(Address v1, Address v2, 
			    vector<dataReqNode*> &data_tuple1, // initialization?
			    vector<dataReqNode*> &data_tuple2,
			    vector<dataReqNode*> datareqs1,
			    vector<dataReqNode*> datareqs2)
{
  // defined in mdl.C
  extern int index_in_data(Address v, vector<dataReqNode*>& data);

  int match_index1 = index_in_data(v1, data_tuple1);
  int match_index2 = index_in_data(v2, data_tuple2);
  
  // v1 get matched in data_tuple1
  if (match_index1 >= 0) {
    // v2 get matched in data_tuple2
    if (match_index2 >= 0) {
      return (match_index1 == match_index2);
    }
    // v2 not get matched in data_tuple2
    else {
      return false;
    }
  }
  
  // now v1 not matched in data_tuple1
  match_index1 = index_in_data(v1, datareqs1);
  match_index2 = index_in_data(v2, datareqs2);
  
  // v1 get matched in datareqs1
  if (match_index1 >= 0) {
    // v2 get matched in datareqs2
    if (match_index2 >= 0) {
      // get a new match pair @@
      data_tuple1 += datareqs1[match_index1];
      data_tuple2 += datareqs2[match_index2];
      return true;
    }
    // v2 not get matched in datareqs2
    else {
      return false;
    }
  }
  
  // neither v1 or v2 get matched in their data tuples or reqs, respectively
  return (v1 == v2);
}
#endif

bool AstNode::condMatch(AstNode* a,
			vector<dataReqNode*> &data_tuple1, // initialization?
			vector<dataReqNode*> &data_tuple2,
			vector<dataReqNode*> datareqs1,
			vector<dataReqNode*> datareqs2)
{

  unsigned i;


  if (this == a)
    return true;
  
  // compare node type
  if (type != a->type)
    return false;
  
  // what to do with "size" (size of the operations in bytes)
  
  switch (type)
    {
    case sequenceNode:
      assert ((loperand != NULL) &&
	      (roperand != NULL) &&
	      (a->loperand != NULL) &&
	      (a->roperand != NULL));
      
      if ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
	  (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)))
	return true;
      else
	return false;

      break;
      
    case opCodeNode:
      // compare operator code
      if (op != a->op)
	return false;
      
      switch (op)
	{
	case funcJumpOp:
	  if ((callee == a->callee) &&
	      (calleefunc->match(a->calleefunc)))  // NEED WORK
	    return true;
	  else
	    return false;
	  
	  break;

	default:
	  // assume:
	  // if 1 operand,  it is loperand
	  // if 2 operands, they are loperand and roperand
	  // if 3 operands, they are loperand, roperand and eoperand
	  if ((loperand != NULL) && (a->loperand != NULL) &&
	      (roperand == NULL) && (a->roperand == NULL) &&
	      (eoperand == NULL) && (a->eoperand == NULL))
	    return (loperand->condMatch(a->loperand, data_tuple1, data_tuple2, 
					datareqs1, datareqs2));
	  
	  else if ((loperand != NULL) && (a->loperand != NULL) &&
		   (roperand != NULL) && (a->roperand != NULL) &&
		   (eoperand == NULL) && (a->eoperand == NULL))
	    return ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)));

	  else if ((loperand != NULL) && (a->loperand != NULL) &&
		   (roperand != NULL) && (a->roperand != NULL) &&
		   (eoperand != NULL) && (a->eoperand != NULL))
	    return ((loperand->condMatch(a->loperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (roperand->condMatch(a->roperand, data_tuple1, data_tuple2, datareqs1, datareqs2)) &&
		    (eoperand->condMatch(a->eoperand, data_tuple1, data_tuple2, datareqs1, datareqs2)));
	  else
	    return false;

	  break;
	}

      break;
      
    case operandNode:
      // compare operand type
      if (oType != a->oType)
	return false;

      switch (oType)
	{ // need to know the EXACT ovalue type to compare
	case Constant:  // NEED WORK, WHAT ABOUT ADDRESSES, or other types
	  return ((int) oValue == (int) a->oValue);
	  
	case ConstantPtr:  // doesn't seem to be used anywhere
	  break;

	case ConstantString:
	  return (strcmp((char *)oValue, (char *)a->oValue) == 0);
	  
#if defined(MT_THREAD)
	case OffsetConstant:
	  return ((isLevel == a->isLevel) &&
		  (level_index_match(lvl, a->lvl, idx, a->idx,
				     data_tuple1, data_tuple2, datareqs1, datareqs2)));
#else
	  // restore DataValue and DataPtr
	case DataPtr:  // used in: createTimer, mdl.C MDL_ADDRESS &v
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
	  
	case DataValue:  // used in: createCounter, mdl.C MDL_T_DRN, if (flag)
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
#endif

	case DataId:  // doesn't seem to be used anywhere
	  break;

	case DataIndir:
	  assert((loperand != NULL) &&
		 (a->loperand != NULL));

	  return (loperand->condMatch(a->loperand, data_tuple1, data_tuple2, 
				      datareqs1, datareqs2));
	  
	case DataReg:  // used once in ast.C (computeAddress)
	  return ((unsigned) oValue == (unsigned) a->oValue);

	case Param:  // or should it be "unsigned"
	  return ((int) oValue == (int) a->oValue);

	case ReturnVal:  // or should it be "unsigned"
	  return ((int) oValue == (int) a->oValue);
	  
#if defined(MT_THREAD) // Need work: check type!!
	case DataAddr:
	  return ((void *) oValue == (void *) a->oValue);
#else  // Need work: check for first 2 or 3 cases!!!
	case DataAddr:  // Address is unsigned; used in mdl.C -- arg to func
	  // return ((Address) oValue == (Address) a->oValue);
	  return (variable_address_match((Address)oValue, (Address)a->oValue,
					 data_tuple1, data_tuple2, datareqs1, datareqs2));
#endif
	  
	case FrameAddr:  // used once in BPatch_snippet.C (BPatch_variableExpr constructor)
	  return ((void *) oValue == (void *) a->oValue);

	case SharedData:  // doesn't seem to be used anywhere
	  break;

	case PreviousStackFrameDataReg:  // Register or some other unsigned type
	  return ((unsigned) oValue == (unsigned) a->oValue);

	default:
	  return false;
	}

      break;
      
    case callNode:
      if (callee != a->callee)
	return false;
      
      if (!calleefunc->match(a->calleefunc))
	return false;
      
      // unsigned osize = operands.size();
      if (operands.size() != a->operands.size())
	return false;
      
      for (i=0; i<operands.size(); i++) {
	if (!operands[i]->condMatch(a->operands[i], data_tuple1, data_tuple2,
				    datareqs1, datareqs2))
	  return false;

	  }
      
      return true;
      
    default:
      return false;
    }

  return false;
}

// Check if "mn" and "this" correspond to the same instrumentation?
bool sampleMetFocusNode::condMatch(sampleMetFocusNode *mn,
				   vector<dataReqNode*> &data_tuple1,
				   vector<dataReqNode*> &data_tuple2) {
  vector<dataReqNode *> datanodes1, datanodes2;
  datanodes1 = getDataRequests();
  datanodes2 = mn->getDataRequests();

  unsigned datareqs_size = datanodes1.size();
  unsigned instreqs_size = instRequests.size();

  
  // Both "this" metricDefinitionNode and the passed in metricDefinitionNode
  // "mn" have the same number of dataRequestNodes 
  if ((datareqs_size != datanodes2.size()) ||
      (instreqs_size != mn->instRequests.size())) {
    return false;
  }
  
  // need to return this match?
  // vector<dataReqNode*> data_tuple1, data_tuple2; // initialization?

  // Check that instRequestNodes in "mn" and "this" are the same. 
  for (unsigned i=0; i<instreqs_size; i++) {

    // what to do with "callWhen when" and "callOrder order"??
    bool match_flag = (instRequests[i].Point())->match(mn->instRequests[i].Point());
    if (!match_flag) {
      return false;
    }
    
    match_flag = (instRequests[i].Ast())->condMatch(mn->instRequests[i].Ast(),
						    data_tuple1, data_tuple2,
						    datanodes1, datanodes2);
    if (!match_flag) {
      return false;
    }
  }

  return true;
}

// going to here

// incremental code generation optimization
// check if match BEFORE add into allMIPrimitives
sampleMetFocusNode* sampleMetFocusNode::matchInMIPrimitives() {
  // note the two loops; we can't safely combine into one since the second 
  // loop modifies the dictionary. 
  vector<metricDefinitionNode*> allprims;
  dictionary_hash_iter<string, metricDefinitionNode*> iter = 
                                                getIter_sampleMetFocusBuf();
  for (; iter; iter++) {
    allprims += iter.currval();
  }
  
  for (unsigned i=0; i < allprims.size(); i++) {
    sampleMetFocusNode* primitiveMI = dynamic_cast<sampleMetFocusNode*>(
                                                               allprims[i]);

    if ((primitiveMI->proc() != proc_) || 
        (primitiveMI->metStyle() != style_)) {
      continue;
    }

    // what about? -- not meaningful to primitive mdn at all!
    //   met_: met_.., internalMetric::isInternalMetric(aggMI->getMetName())
    //   focus_, component_focus, flat_name_
    //   id_, metric_name_, originalCost_? aggOp should be default
    
    // A NEW PROBLEM TO DO:
    //   component mdn needs to remember all names of its primitive mdns

    vector<dataReqNode*> data_tuple1;
    vector<dataReqNode*> data_tuple2;
    bool match_flag =condMatch(primitiveMI,
			       data_tuple1, data_tuple2);
    if (match_flag) {
      return primitiveMI;
    }
  }
  
  return NULL;
}
//#endif

// assume constraint variable is always the first in primitive
dataReqNode* metricDefinitionNode::getFlagDRN(void) 
{ 
  assert(mdn_type_ == PRIM_MDN);
  assert(components.size() > 0);
  threadMetFocusNode *thrNode = 
     dynamic_cast<threadMetFocusNode*>(components[0]);
  vector<dataReqNode *> dataReqs = thrNode->getDataRequests();
  assert(dataReqs.size() > 0);
  return dataReqs[0];
}

void sampleMetFocusNode::addInst(instPoint *point, AstNode *ast,
				 callWhen when, callOrder o)
{
  if (!point) return;

  instReqNode temp(point, ast, when, o);
  instRequests += temp;
}

vector<dataReqNode *> sampleMetFocusNode::getDataRequests()
{
   vector<metricDefinitionNode *> comps = getComponents();
   assert(0 < comps.size());
   threadMetFocusNode *thrNode =
     dynamic_cast<threadMetFocusNode*>(comps[0]);
   return thrNode->getDataRequests();
}

defInst::defInst(string& metric_name, vector<u_int>& focus, int id, 
                 pd_Function *func, unsigned attempts)
  : metric_name_(metric_name), focus_(focus), id_(id), 
    func_(func), attempts_(attempts)
{

}
