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

#ifndef PROC_MET_FOCUS_NODE
#define PROC_MET_FOCUS_NODE

#include "paradynd/src/metric.h"
#include "common/h/Dictionary.h"

class machineMetFocusNode;
class instrCodeNode;
class threadMetFocusNode;
class threadMetFocusNode_Val;
class dataReqNode;

class processMetFocusNode : public metricDefinitionNode {
 private:
  instrCodeNode *metricVarCodeNode;
  vector<instrCodeNode *> constraintCodeNodes;
  vector<threadMetFocusNode *> thrNodes;
  sampleAggregator aggregator;
  machineMetFocusNode *parentNode;
  aggComponent *aggInfo;  // for machNode <-> procNode link
  process *proc_;

  aggregateOp aggOp;
  bool aggInfoInitialized;
  timeStamp procStartTime;    // the time that this metric started
                              // need this in updateWithDeltaValue()
  vector< vector<string> > component_focus;
  bool dontInsertData_;
  bool catchupNotDoneYet_;

  processMetFocusNode(process *p,
		      const vector< vector<string> >& component_foc,
		      aggregateOp agg_op, bool arg_dontInsertData);

  bool catchupNotDoneYet() { return catchupNotDoneYet_; }
  void manuallyTrigger(int mid);
  bool catchupInstrNeeded() const;

  // don't have a hash indexed by pid because can have multiple procNodes
  // with same pid
  static vector<processMetFocusNode*> allProcNodes;

 public:
  static void getProcNodes(vector<processMetFocusNode*> *machNodes);
  static void getProcNodes(vector<processMetFocusNode*> *machNodes, int pid);

  static processMetFocusNode *newProcessMetFocusNode(process *p, 
                                 const vector< vector<string> >& component_foc,
				 aggregateOp agg_op, bool arg_dontInsertData);
  ~processMetFocusNode();

  const vector< vector<string> > &getComponentFocus() const { 
    return component_focus;
  }
  void setMetricVarCodeNode(instrCodeNode* part);
  void addConstraintCodeNode(instrCodeNode* part);
  void addThread(pdThread *thr);
  void deleteThread(pdThread *thr);
  processMetFocusNode* handleExec();
  timeLength cost() const;
  instrCodeNode *getMetricVarCodeNode() {
    return metricVarCodeNode;
  }
  int getMetricID();
  unsigned numThrNodes() { return thrNodes.size(); }
  machineMetFocusNode *getParent() { return parentNode; }
  void recordAsParent(machineMetFocusNode *machNode,
		      aggComponent *childAggInfo);

  void getAllCodeNodes(vector<instrCodeNode *> *vecPtr) {
    (*vecPtr).push_back(metricVarCodeNode);
    for(unsigned i=0; i<constraintCodeNodes.size(); i++)
      (*vecPtr).push_back(constraintCodeNodes[i]);
  }
  void getAllCodeNodes(vector<const instrCodeNode *> *vecPtr) const {
    (*vecPtr).push_back(metricVarCodeNode);
    for(unsigned i=0; i<constraintCodeNodes.size(); i++)
      (*vecPtr).push_back(constraintCodeNodes[i]);
  }
  aggregateOp getAggOp() { return aggOp; }
  void addPart(threadMetFocusNode* thrNode);
  void updateAggInterval(timeLength width) {
    aggregator.changeAggIntervalWidth(width);
  }
  void updateWithDeltaValue(timeStamp startTime, timeStamp sampleTime, 
			    pdSample value);
  void tryAggregation();
  process *proc() { return proc_; }
  bool dontInsertData() { return dontInsertData_; }
  void print();
  void initAggInfoObjects(timeStamp timeOfCont, pdSample initValue);
  bool hasDeferredInstr();
  bool hookupJumpsToBaseTramps();
  bool baseTrampsHookedUp();
  bool instrLoaded();
  bool instrInserted() { return (instrLoaded() & baseTrampsHookedUp()); }
  bool loadInstrIntoApp(pd_Function **func);
  void doCatchupInstrumentation();

  vector<const dataReqNode*> getFlagDRNs(int thr_id) const;
  void mapSampledDRNs2ThrNodes();
  void stopSamplingThr(threadMetFocusNode_Val *thrNodeVal);
  bool needToWalkStack() ;  // const;  , make this const in the future

  bool hasAggInfoBeenInitialized() { return aggInfoInitialized; }
  timeStamp getStartTime() { return procStartTime; }

#if defined(MT_THREAD)
  void prepareCatchupInstr0(int tid);
#endif
  void prepareCatchupInstr();

#if defined(MT_THREAD)
  void setMetricRelated(unsigned type, bool arg_dontInsertData, 
			const vector<string> &temp_ctr, 
			vector<T_dyninstRPC::mdl_constraint*> flag_cons,
			T_dyninstRPC::mdl_constraint* repl_cons) {
    type_thr          = type;
    dontInsertData_thr = arg_dontInsertData;
    temp_ctr_thr      = temp_ctr;
    flag_cons_thr     = flag_cons;
    base_use_thr      = repl_cons;
  }
#endif
};



#endif
