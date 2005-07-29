/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

#ifndef PROC_MET_FOCUS_NODE
#define PROC_MET_FOCUS_NODE

#include "paradynd/src/metricFocusNode.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/pd_thread.h"
#include "common/h/Vector.h"

class machineMetFocusNode;
class instrCodeNode;
class threadMetFocusNode;
class threadMetFocusNode_Val;
class instrDataNode;
class catchupReq;
class AstNode;
class dyn_thread;
class pd_thread;
class instReqNode;
class dyn_lwp;

struct catchup_t {
    AstNode *ast;
    dyn_thread *thread;
};

struct sideEffect_t {
  Frame frame;
  instReqNode *reqNode;
};

class processMetFocusNode : public metricFocusNode {
 private:
  static pdvector<processMetFocusNode *> procNodesToDeleteLater;
  instrCodeNode *metricVarCodeNode;
  pdvector<instrCodeNode *> constraintCodeNodes;
  pdvector<threadMetFocusNode *> thrNodes;
  sampleAggregator aggregator;
  machineMetFocusNode *parentNode;
  aggComponent *aggInfo;  // for machNode <-> procNode link
  pd_process *proc_;

  aggregateOp aggOp;
  timeStamp procStartTime;    // the time that this metric started
                              // need this in updateWithDeltaValue()
  const pdstring metric_name;
  const Focus focus;

  bool dontInsertData_;
  bool currentlyPaused;
  bool instrInserted_;  // ie. instr:  loaded & tramps hookedup & catchuped
  pdvector<catchup_t >   catchupASTList;
  pdvector<unsigned> rpc_id_buf;

  bool isBeingDeleted_;

  processMetFocusNode(pd_process *p, const pdstring &metname,
		      const Focus &component_foc, aggregateOp agg_op, 
		      bool arg_dontInsertData);

  void manuallyTrigger(int mid);
  void prepareCatchupInstr(pdvector<pdvector<Frame> >&stackWalks);  // do catchup on given thread
  bool postCatchupRPCs();

  threadMetFocusNode *getThrNode(unsigned tid);
  threadMetFocusNode *getThrNode(pd_thread *thr) {
     return getThrNode(thr->get_tid());
  }
  const threadMetFocusNode *getThrNode(unsigned tid) const;
  const threadMetFocusNode *getThrNode(pd_thread *thr) const {
     return getThrNode(thr->get_tid());
  }

 public:
  bool isBeingDeleted() {return isBeingDeleted_; };

  // use this function to create a new processMetFocusNode in the general case
  static processMetFocusNode *newProcessMetFocusNode(pd_process *p, 
				 const pdstring &metname, const Focus &focus_,
				 aggregateOp agg_op, bool arg_dontInsertData);

  // a copy constructor variant, used when handling fork
  processMetFocusNode(const processMetFocusNode &par, pd_process *childProc);

  ~processMetFocusNode();

  static void removeProcNodesToDeleteLater();
  static void addProcNodesToDeleteLater(processMetFocusNode *procNode) {
     procNodesToDeleteLater.push_back(procNode);
  }
  const pdstring getFullName() const {
     return (metric_name + focus.getName());
  }
  const Focus getFocus() const { 
    return focus;
  }
  void setMetricVarCodeNode(instrCodeNode* part);
  void addConstraintCodeNode(instrCodeNode* part);
  void propagateToNewThread(pd_thread *thr);
  void updateForExitedThread(pd_thread *thr);
  
  timeLength cost() const;
  instrCodeNode *getMetricVarCodeNode() {
    return metricVarCodeNode;
  }
  int getMetricID();
  unsigned numThrNodes() { return thrNodes.size(); }
  machineMetFocusNode *getParent() { return parentNode; }
  void recordAsParent(machineMetFocusNode *machNode,
		      aggComponent *childAggInfo);

  void getAllCodeNodes(pdvector<instrCodeNode *> *vecPtr) {
    (*vecPtr).push_back(metricVarCodeNode);
    for(unsigned i=0; i<constraintCodeNodes.size(); i++)
      (*vecPtr).push_back(constraintCodeNodes[i]);
  }
  void getAllCodeNodes(pdvector<const instrCodeNode *> *vecPtr) const {
    (*vecPtr).push_back(metricVarCodeNode);
    for(unsigned i=0; i<constraintCodeNodes.size(); i++)
      (*vecPtr).push_back(constraintCodeNodes[i]);
  }
  aggregateOp getAggOp() { return aggOp; }
  void addThrMetFocusNode(threadMetFocusNode *thrNode);
  void removeThrMetFocusNode(threadMetFocusNode *thrNode);
  void updateAggInterval(timeLength width) {
    aggregator.changeAggIntervalWidth(width);
  }
  void updateWithDeltaValue(timeStamp startTime, timeStamp sampleTime, 
			    pdSample value);
  void tryAggregation();
  pd_process *proc() { return proc_; }
  bool dontInsertData() { return dontInsertData_; }
  void print();
  void initializeForSampling(timeStamp timeOfCont, pdSample initValue);
  void initAggInfoObjects(timeStamp timeOfCont, pdSample initValue);
  bool hasBeenCatchuped();
  bool trampsHookedUp();
  bool hasDeferredInstr();
  bool insertJumpsToTramps(pdvector<pdvector<Frame > >&stackWalks);
  bool instrInserted() { return instrInserted_; }
  bool instrLoaded();

  inst_insert_result_t loadInstrIntoApp();
  void doCatchupInstrumentation(pdvector<pdvector<Frame> > &stackWalks);
  void doInstrumentationFixup(pdvector<pdvector<Frame> > &stackWalks);
  inst_insert_result_t insertInstrumentation();
  
  pdvector<const instrDataNode *> getFlagDataNodes() const;
  void prepareForSampling();
  void prepareForSampling(threadMetFocusNode *thrNode);
  void stopSamplingThr(threadMetFocusNode_Val *thrNodeVal);
  //bool needToWalkStack() ;  // const;  , make this const in the future

  timeStamp getStartTime() { return procStartTime; }

  // static void catchupRPC_Complete(process *, unsigned rpc_id,
  //                                 void * /*data*/, void * /*ret*/);

  void removeDataNodes();

  void pauseProcess();
  void continueProcess();
  void unFork();
  bool cancelPendingRPC(unsigned rpc_id);
  void cancelPendingRPCs();
  bool anyPendingRPCs() { return (rpc_id_buf.size() > 0); }
};



#endif
