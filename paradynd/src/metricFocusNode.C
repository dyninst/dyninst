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

// $Id: metricFocusNode.C,v 1.222 2002/04/09 04:19:49 schendel Exp $

#include "common/h/headers.h"
#include "common/h/Types.h"
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
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
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

vector<internalMetric*> internalMetric::allInternalMetrics;

// used to indicate the mi is no longer used.
#define DELETED_MI 1
#define MILLION 1000000.0

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
metricDefinitionNode::metricDefinitionNode()
: originalCost_(timeLength::Zero())
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
machineMetFocusNode *doInternalMetric(int mid, 
				      vector< vector<string> >& canon_focus,
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
      vector<processMetFocusNode*> noParts;
      mn = new machineMetFocusNode(mid, metric_name, canon_focus,
		    flat_name, noParts, theIMetric->aggregate(), enable);
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
      vector<processMetFocusNode*> noParts;
      mn = new machineMetFocusNode(mid, metric_name, canon_focus,
		       flat_name, noParts, nc->aggregate(), enable);
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

machineMetFocusNode *createMetricInstance(int mid, string& metric_name, 
		        vector<u_int>& focus,
		        bool enable, // true if for real; false for guessCost()
		        bool *internal)
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

   string flat_name = metricAndCanonFocus2FlatName(metric_name,canonicalFocus);
   
   if (mdl_can_do(metric_name)) {
      *internal = false;
      
      /* select the processes that should be instrumented. We skip process
	 that have exited, and processes that have been created but are not
	 completely initialized yet.  If we try to insert instrumentation in
	 a process that is not ready yet, we get a core dump.  A process is
	 ready when it is not in neonatal state and the isBootstrappedYet
	 returns true.
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
      if (procs.size() == 0 || threadsVec.size() == 0)
	 // there are no processes or threads to instrument
#else
	 if (procs.size() == 0)
	    // there are no processes to instrument
#endif
	 {	    
	    //printf("createMetricInstance failed, no processes to instrument\n");
	    return NULL;
	 }

      machineMetFocusNode *machNode = 
	 mdl_do(mid, canonicalFocus, metric_name, flat_name, procs, 
		threadsVec, false, enable);
      //cerr << "  mdl_do returned ";
      //if (machNode == NULL) {
      //    cerr << "NULL" << endl;
      //} else {
      //    cerr << "Non-NULL" << endl;
      //}
      
      if (machNode == NULL) {
	//	 metric_cerr << "createMetricInstance failed since mdl_do failed" << endl;
	// metric_cerr << "metric name was " << metric_name << "; focus was ";
      metric_cerr << "createMetricInstance failed since mdl_do failed" << endl;
      metric_cerr << "metric name was " << metric_name << "; focus was ";
	 print_focus(metric_cerr, canonicalFocus);
      }
      return machNode;
   } else {
      bool matched;
      machineMetFocusNode *machNode = 
	 doInternalMetric(mid, canonicalFocus, canonicalFocus, metric_name, 
			  flat_name, enable, matched);
      // NULL on serious error; -1 if enable was false; -2 if illegal to
      // instr with given focus [many internal metrics work only for whole
      // program]

      if (machNode == (machineMetFocusNode*)-2) {
	 metric_cerr << "createMetricInstance: internal metric " << metric_name << " isn't defined for focus: ";
	 print_focus(metric_cerr, canonicalFocus);
	 machNode = NULL; // straighten up the return value
      }
      else if (machNode == (machineMetFocusNode*)-1) {
	 metric_cerr << " createMetricInstance: internal metric not enable: " << metric_name << endl;
	 assert(!enable); // no error msg needed
	 machNode = NULL; // straighten up the return value
      }
      else if (machNode == NULL) {
	 // more serious error...do a printout
	 metric_cerr << "createMetricInstance failed since doInternalMetric failed" << endl;
	 metric_cerr << "metric name was " << metric_name << "; focus was ";
	 print_focus(metric_cerr, canonicalFocus);
      }

      *internal = true;
      return machNode;
   }
}


// propagate this metric instance to process p.
// p is a process that started after the metric instance was created
// note: don't call this routine for a process started via fork or exec, just
// for processes started the "normal" way.
// "this" is an aggregate(AGG_MDN or AGG_MDN) mi, not a component one.

void metricDefinitionNode::propagateToNewProcess(process *) {
  /*
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

  machineMetFocusNode *machNode = NULL;
     // an aggregate (not component) machNode, though we know that it'll
     // contain just one component.  It's that one component that we're
     // really interested in.
  if (mdl_can_do(met_)) {
      // Make the unique ID for this metric/focus visible in MDL.
      string vname = "$globalId";
      mdl_env::add(vname, false, MDL_T_INT);
      mdl_env::set(this->getMetricID(), vname);

      vector<process *> vp(1,p);
      vector< vector<pdThread *> > threadsVec;
#if defined(MT_THREAD)
      threadsVec += p->threads;
#endif
      cerr << "mdl_do - B\n";
      machNode = mdl_do(mid, focus_, met_, flat_name_, vp, threadsVec, false,false);
  } else {
    // internal and cost metrics don't need to be propagated (is this correct?)
    machNode = NULL;
  }

  if (machNode) { // successfully created new machNode
    assert(machNode->components.size() == 1);

    processMetFocusNode *procNode = 
      dynamic_cast<processMetFocusNode*>(machNode->components[0]);

    components += procNode;
#if defined(MT_THREAD)
    unsigned aggr_size = procNode->aggregators.size();
    procNode->aggregators[aggr_size-1] = this;       // overwrite
    procNode->samples[aggr_size-1] = aggregator.newComponent();
                                                            // overwrite
    // procNode->comp_flat_names[aggr_size-1]  has the correct value
#else
    procNode->aggregators[0] = this;
    procNode->samples[0] = aggregator.newComponent();
#endif

    if (!internal) {
      // dummy parameters for loadInstrIntoApp
      pd_Function *func = NULL;
      procNode->loadInstrIntoApp(&func);
      procNode->insertJumpsToTramps();
    }

    // update cost
    const timeLength cost = machNode->cost();
    if (cost > originalCost_) {
      addCurrentPredictedCost(cost - originalCost_);
      originalCost_ = cost;
    }

    machNode->components.resize(0); // protect the new component
    delete machNode;
  }
  */
}



void metricDefinitionNode::handleExec(process *) {
   // a static member fn.  handling exec is tricky.  At the time this routine
   // is called, the "new" process has been bootstrapped and is ready for
   // stuff to get inserted.  No mi's have yet been propagated, and the data
   // structures (allMIs, allMIComponents, etc.) are still in their old,
   // pre-exec state, so they show component mi's enabled for this process,
   // even though they're not (at least not yet).  This routines brings
   // things up-to-date.
   //
   // Algorithm: loop thru all component mi's for this process.  If it is
   // possible to propagate it to the "new" (post-exec) process, then do so.
   // If not, fry the component mi.  An example where a component mi can no
   // longer fit is an mi specific to, say, function foo(), which (thanks to
   // the exec syscall) no longer exists in this process.  Note that the exec
   // syscall changed the addr space enough so even if a given routine foo()
   // is present in both the pre-exec and post-exec process, we must assume
   // that it has MOVED TO A NEW LOCATION, thus making the component mi's
   // instReqNode's instPoint out-of-date.  Ick.

   // note the two loops; we can't safely combine into one since the second
   // loop modifies the dictionary.
  /*
   vector<metricDefinitionNode*> allcomps;
   dictionary_hash_iter<string,metricDefinitionNode*> iter =
                                             getIter_processMetFocusBuf();
   for (; iter; iter++)
      allcomps += iter.currval();
   
   for (unsigned i=0; i < allcomps.size(); i++) {
      processMetFocusNode* procnode = dynamic_cast<processMetFocusNode*>(
                                                                allcomps[i]);
      if (procnode->proc() != proc)
	 continue;

      forkexec_cerr << "calling handleExec for component "
	            << procnode->flat_name_ << endl;

      processMetFocusNode *replaceWithComponentMI = procnode->handleExec();
      
      if (replaceWithComponentMI == NULL) {
	 forkexec_cerr << "handleExec for component " << procnode->flat_name_
	               << " failed, so not propagating it" << endl;
         procnode->removeThisInstance(); // propagation failed; fry component mi
      }
      else {
	 forkexec_cerr << "handleExec for component " << procnode->flat_name_
	               << " succeeded...it has been propagated" << endl;
	 // new component mi has already been inserted in place of old
	 // component mi in all of its aggregate's component lists.  So, not
	 // much left to do, except to update allMIComponents.

#if defined(MT_THREAD)
	 for (unsigned u1=0; u1<procnode->comp_flat_names.size(); u1++)
	    if (isKeyDef_processMetFocusBuf(procnode->comp_flat_names[u1]))
	       undefKey_processMetFocusBuf(procnode->comp_flat_names[u1]);

	 for (unsigned u2=0; u2<procnode->components.size(); u2++)
	    procnode->removeComponent(procnode->components[u2]);
	 procnode->components.resize(0);
#else
	 assert(replaceWithComponentMI->flat_name_ == procnode->flat_name_);
#endif
	 delete procnode; // old component mi (dtor removes it from allMIComponents)
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
  */
}



#if defined(MT_THREAD)
void metricDefinitionNode::rmCompFlatName(unsigned u) {
  //assert(COMP_MDN == mdn_type_);  -- bhs
  unsigned size = comp_flat_names.size();
  assert(u < size);
    
  // -- bhs
  //if (isKeyDef_processMetFocusBuf(comp_flat_names[u])) {
  //  undefKey_processMetFocusBuf(comp_flat_names[u]);
  //}
  
  comp_flat_names[u] = comp_flat_names[size-1];
  comp_flat_names.resize(size-1);
}
#endif

// Remove the aggregate metric instances that don't have any components left
void removeFromMetricInstances(process *proc) {
   metric_cerr << "removeFromMetricInstances- proc: " << proc << ", pid: " 
	       << proc->getPid() << "\n";

   // Loop through all of the _component_ mi's; for each with component
   // process of "proc", remove the component mi from its aggregate mi.
   // Note: imho, there should be a *per-process* vector of mi-components.
   
   // note 2 loops for safety (2d loop may modify dictionary?)
   
   vector<processMetFocusNode *> greppedProcNodes;
   processMetFocusNode::getProcNodes(&greppedProcNodes, proc->getPid());
   for(unsigned i=0; i<greppedProcNodes.size(); i++) {
      machineMetFocusNode *machNode = greppedProcNodes[i]->getParent();
      machNode->deleteProcNode(greppedProcNodes[i]);
      costMetric::removeProcessFromAll(proc); // what about internal metrics?
   }
}


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

//before
//metricDefinitionNode *metricDefinitionNode::forkProcess(process *child,
//			const dictionary_hash<instInstance*,instInstance*> &map) const {
metricDefinitionNode *metricDefinitionNode::forkProcess(process *,
		   const dictionary_hash<instInstance*,instInstance*> &) const
{
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
  /*
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
			 newComponentFocus, // this is a change
			 aggregateOp(aggOp));
    assert(mi);

    //  need to reimplement, in instrThrDataNode
    //incrementCounterId();

    forkexec_cerr << "metricDefinitionNode::forkProcess -- component flat name for parent is " << flat_name_ << "; for child is " << mi->flat_name_ << endl;

    // not attempt to register all names
    assert(! isKeyDef_processMetFocusBuf(newComponentFlatName));
    setVal_processMetFocusBuf(newComponentFlatName, mi);

    // Duplicate the dataReqNodes:
    // If it's going to duplicate the dataReqNodes, these are members only of
    // the threadMetFocusNode.  So it would have to go through all of mi's
    // threadMetFocusNodes and duplicate all of it's data request nodes.
    // Then I suppose it would assign those duplicated dataRequestNodes to be
    // used by this metricDefinitionNode's threadMetFocusNodes.

    for (unsigned u1 = 0; u1 < dataRequests.size(); u1++) {
       // must add to drnIdToMdnMap[] before dup() to avoid some assert fails
       const int newCounterId = incrementCounterId();
          // no relation to mi->getMetricID();
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

    mi->instrLoaded_ = true;
    return static_cast<metricDefinitionNode*>(mi);
  */
  return NULL;  // just while function body is commented out
}

// unforkInstRequests and unforkDataRequests only for non-threaded

//bool metricDefinitionNode::unFork(dictionary_hash<instInstance*, instInstance*> &map,
//				  bool unForkInstRequests,
//				  bool unForkDataRequests) {
bool metricDefinitionNode::unFork(dictionary_hash<instInstance*, instInstance*> &,
				  bool, bool) {
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
//void metricDefinitionNode::handleFork(const process *parent, process *child,
//			      dictionary_hash<instInstance*,instInstance*> &map) {
void metricDefinitionNode::handleFork(const process *, process *,
			     dictionary_hash<instInstance*,instInstance*> &)
{
  /*
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
*/
}


// startCollecting is called by dynRPC::enableDataCollection 
// (or enableDataCollection2) in dynrpc.C
// startCollecting is a friend of metricDefinitionNode; can it be
// made a member function of metricDefinitionNode instead?
// Especially since it clearly is an integral part of the class;
// in particular, it sets the crucial vrble "id_"
//
int startCollecting(string& metric_name, vector<u_int>& focus, int id)
{
   bool internal = false;
   // Make the unique ID for this metric/focus visible in MDL.
   string vname = "$globalId";
   mdl_env::add(vname, false, MDL_T_INT);
   mdl_env::set(id, vname);
   
   machineMetFocusNode *machNode = 
      createMetricInstance(id, metric_name, focus, true, &internal);
   
   if (!machNode) {
      metric_cerr << "startCollecting for " << metric_name 
		  << " failed because createMetricInstance failed" << endl;
      return(-1);
   }
   
   const timeLength cost = machNode->cost();
   machNode->originalCost_ = cost;
   
   addCurrentPredictedCost(cost);
   
   if (internal) {
      metResPairsEnabled++;
      return(machNode->getMetricID());
   }

   machNode->pauseProcesses();

   if(! machNode->insertInstrumentation()) {
      machNode->continueProcesses();
      return machNode->getMetricID();
   }
   
   // This has zero for an initial value.  This is because for cpu_time and
   // wall_time, we just want to total the cpu_time and wall_time for this
   // process and no others (but if we want someone to get an actual cpu time
   // for this program even if they start the cpu_time metric after the start
   // of the process, the initial actual value could be the actual cpu time
   // at the start of this metric).  For the counter metrics (eg. proc_calls,
   // io_bytes), we also want zero (we have no way of getting the total
   // proc_calls & io_bytes of the process before the metric was enabled, so
   // we have to use zero).  However, it is possible that in the future we'll
   // create a metric where it makes sense to send an initial actual value.
   machNode->initializeForSampling(getWallTime(), pdSample::Zero());

   machNode->continueProcesses();

   metResPairsEnabled++;
   return machNode->getMetricID();
}

timeLength guessCost(string& metric_name, vector<u_int>& focus) {
    // called by dynrpc.C (getPredictedDataCost())
   static int tempMetFocus_ID = -1;
   bool internal;
   machineMetFocusNode *mi = 
      createMetricInstance(tempMetFocus_ID, metric_name, focus, false, 
			   &internal);
   tempMetFocus_ID--;

    if (!mi) {
       metric_cerr << "guessCost returning 0.0 since createMetricInstance failed" << endl;
       return timeLength::Zero();
    }

    timeLength cost = mi->cost();
    delete mi;

    return cost;
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


//
// Costs are now reported to paradyn like other metrics (ie. we are not
// calling reportInternalMetrics to deliver cost values, instead we wait
// until we have received a new interval of cost data from each process)
// note: this only works for the CM5 because all cost metrics are sumed
// at the daemons and at paradyn, otherwise the CM5 needs its own version
// of this routine that uses the same aggregate method as the one for paradyn 
//

/* ************************************************************************* */

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
	tp->endOfDataCollection(theInstance.getMetricID());
	theIMetric->disableInstance(0);
      }
    }  
}




// ======================


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

defInst::defInst(int id, pd_Function *func, unsigned attempts)
  : id_(id), func_(func), attempts_(attempts)
{

}
