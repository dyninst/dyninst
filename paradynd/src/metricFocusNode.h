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

// $Id: metricFocusNode.h,v 1.89 2002/05/02 21:29:04 schendel Exp $ 

#ifndef METRIC_H
#define METRIC_H

#include "common/h/String.h"
// trace data streams
#include "common/h/Vector.h"
#include "pdutil/h/sampleAggregator.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "rtinst/h/trace.h"
#include "dyninstAPI/src/inst.h" // for "enum callWhen"
#include "dyninstRPC.xdr.SRVR.h" // for flag_cons
#include "common/h/Time.h"
#include "pdutil/h/metricStyle.h"
#include "pdutil/h/ByteArray.h"
#include "common/h/Dictionary.h"

#define OPT_VERSION 1

class threadMetFocusNode_Val;

class instInstance; // enough since we only use instInstance* in this file
#if defined(MT_THREAD)
class pdThread; // enough since we only use pdThread* in this file
#endif

#include "paradynd/src/variableMgr.h"


/* ************************************************************************ */

class processMetFocusNode;
class threadMetFocusNode;
class instrCodeNode;

class metricDefinitionNode {
friend timeLength guessCost(string& metric_name, vector<u_int>& focus) ;

friend int startCollecting(string&, vector<u_int>&, int id); // called by dynrpc.C
#if defined(MT_THREAD)
friend bool checkMetricMIPrimitives(string metric_flat_name, 
				    instrCodeNode *& metric_prim,
				    string name, 
				    vector< vector<string> >& comp_focus, 
				    int processIdx);
#endif

public:
  // NON_MT_THREAD version:
  // for primitive (real non-aggregate, per constraint var or metric var) mdn's
  // flat name should include process id
  //
  // for component (per-process) (non-aggregate, now aggregate) mdn's
  // difference: it now has parts too (become aggregate)

  metricDefinitionNode();

  // NON_MT_THREAD version:
  // for aggregate (not component) mdn's

  virtual ~metricDefinitionNode() { };

  virtual void print() { };

  timeLength originalCost() const { return originalCost_; }

  // propagate this aggregate mi to a newly started process p (not for
  // processes started via fork or exec, just for those started "normally")
  void propagateToNewProcess(process *p);  

  metricDefinitionNode *forkProcess(process *child,
                                    const dictionary_hash<instInstance*,instInstance*> &map) const;
     // called when it's determined that an mi should be propagated from the
     // parent to the child.  "this" is a component mi, not an aggregator mi.
  bool unFork(dictionary_hash<instInstance*, instInstance*> &map,
	      bool unForkInstRequests, bool unForkDataRequests);
     // the fork() sys call copies all trampoline code, so the child process
     // can be left with code that writes to counters/timers that don't exist
     // (in the case where we don't propagate the mi to the new process).  In
     // such cases, we must remove instrumentation from the child process.
     // That's what this routine is for.  It looks at the instReqNodes of the
     // mi, which are in place in the parent process, and removes them from
     // the child process.  "this" is a component mi representing the parent
     // process.  "map" maps instInstance's of the parent to those of the
     // child.

  static void handleFork(const process *parent, process *child,
                         dictionary_hash<instInstance*, instInstance*> &map);
     // called once per fork.  "map" maps all instInstance's of the parent
     // process to the corresponding copy in the child process...we'll delete
     // some instrumentation in the child process if we find that some
     // instrumentation in the parent doesn't belong in the child.

  static void handleExec(process *);
     // called once per exec, once the "new" process has been bootstrapped.
     // We decide which mi's that were present in the pre-exec process should
     // be carried over to the new process.  For those that should, the
     // instrumentation is actually inserted.  For others, the component mi
     // in question is removed from the system.

#if defined(MT_THREAD)

  // --- ---
  void rmCompFlatName(unsigned u);

  void addCompFlatName(string proc_flat_name) {
    // assert(COMP_MDN == mdn_type_);  --bhs
    comp_flat_names += proc_flat_name;
  }
#endif
  
protected:
  // Since we don't define these, make sure they're not used:
  metricDefinitionNode &operator=(const metricDefinitionNode &src);
  metricDefinitionNode(const metricDefinitionNode &src);

 protected:

  // this function checks if we need to do stack walk
  // if all returnInstance's overwrite only 1 instruction, no stack walk necessary

  // comments only for NON_MT_THREAD version:
  // for aggregate metrics and component (non-aggregate) metrics
  // for component metrics: the last is "base", others are all constraints
  //  aggregator should be consistent with components to some extents
  //  should be added or removed if necessary;
  //  also added in AGG_LEV constructor and addPart

#if defined(MT_THREAD)
                                       //  following 5 memorizing stuff --- for PROC_COMP only
  // data required to add threads - naim
  unsigned type_thr;
  bool dontInsertData_thr;
  vector<string> temp_ctr_thr;
  vector<T_dyninstRPC::mdl_constraint*> flag_cons_thr;
  // could be multiple mdl_constraints
  T_dyninstRPC::mdl_constraint*  base_use_thr;

                                       //  following 4 --- for PROC_COMP only
  vector<string> comp_flat_names;      //  should be consistent with PROC_COMP's aggregators

                                       //  should be consistent with PROC_PRIM's components
#endif
  
  timeLength originalCost_;
 
  // CONSISTENCY GROUPS
  // aggregators, samples (comp_flat_names for PROC_COMP)  \  complicated relations between them
  // components, (aggregator) (thr_names for THR_LEV)           /  should be kept cleanly if possible
  // SPECIALS:  AGG_LEV  --- id_
  //          PROC_COMP  --- temp_ctr_thr... , comp_flat_names
  //          PROC_PRIM  --- instRequests... , thr_names
  //            THR_LEV  --- dataRequests, cumulativeValue
  
  // called by static void handleExec(process *), for each component mi
  // returns new component mi if propagation succeeded; NULL if not.
};


class defInst {
 public:
  defInst(int id, pd_Function *func, unsigned attempts);

  int id() { return id_; }
  pd_Function *func() { return func_; }
  unsigned numAttempts() { return attempts_; }
  void failedAttempt() { if (attempts_ > 0) attempts_--; }
  
 private:
  int id_;
  pd_Function *func_;
  unsigned attempts_;
};

//class defInst {
// public:
//  defInst(unsigned, vector<T_dyninstRPC::focusStruct>&, vector<string>&,
//	  vector<u_int>&, u_int&, u_int&);

//  unsigned index() { return index_; }
//  vector<T_dyninstRPC::focusStruct> focus() { return focus_; }
//  vector<string>& metric() { return metric_; }
//  vector<u_int>& ids() { return ids_; }
//  u_int did() { return did_; }
//  u_int rid() { return rid_; }

// private:
//  unsigned index_;
//  vector<T_dyninstRPC::focusStruct> focus_; 
//  vector<string> metric_;
//  vector<u_int> ids_; 
//  u_int did_;
//  u_int rid_;
//};

#if defined(MT_THREAD)
extern dictionary_hash<string, metricDefinitionNode*> allMIinstalled;
#endif

// don't access this directly, consider private
extern timeLength currentPredictedCost;

// Access currentPredictedCost through these functions, should probably
// be included in some class or namespace in the future
inline timeLength &getCurrentPredictedCost() {
  return currentPredictedCost;
}
inline void setCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost = tl;
}
inline void addCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost += tl;
}
inline void subCurrentPredictedCost(const timeLength &tl) {
  currentPredictedCost -= tl;
}

extern void reportInternalMetrics(bool force);

/*
 * Routines to control data collection.
 *
 * focus		- a list of resources
 * metricName		- what metric to collect data for
 * id                   - metric id
 * procsToContinue      - a list of processes that had to be stopped to insert
 *                        instrumentation. The caller must continue these processes.
 */
int startCollecting(string& metricName, vector<u_int>& focus, int id); 

/*
 * Return the expected cost of collecting performance data for a single
 *    metric at a given focus.  The value returned is the fraction of
 *    perturbation expected (i.e. 0.10 == 10% slow down expected).
 */
timeLength guessCost(string& metric_name, vector<u_int>& focus);

void flush_batch_buffer();
void batchSampleData(string metname, int mid, timeStamp startTimeStamp, 
		     timeStamp endTimeStamp, pdSample value);

bool AstNode_condMatch(AstNode* a1, AstNode* a2,
		       vector<dataReqNode*> &data_tuple1, // initialization?
		       vector<dataReqNode*> &data_tuple2,
		       vector<dataReqNode*> datareqs1,
		       vector<dataReqNode*> datareqs2);

#endif


// need to make dataReqNode return their type info


 
