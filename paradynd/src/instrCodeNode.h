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

#ifndef INSTR_CODE_NODE
#define INSTR_CODE_NODE

#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/instReqNode.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/metricFocusNode.h"

class instrDataNode;
class Focus;
class instrCodeNode;
class processMetFocusNode;
class threadMetFocusNode;
class threadMetFocusNode_Val;
class HwEvent;
class pd_thread;

inline unsigned ui_hash__(const unsigned &u) { return u; }

class instrCodeNode_Val {
  friend class instrCodeNode;

  instrDataNode *sampledDataNode;
  instrDataNode *constraintDataNode;
  pdvector<instrDataNode*> tempCtrDataNodes;

  const pdstring name;  // could be either a metric name or a constraint name
  const Focus focus;
  pdvector<instReqNode *> instRequests;
  pdvector<returnInstance *> baseTrampInstances;
  bool trampsNeedHookup_;

  // instrCodeNodes (actually instrCodeNode_Vals) can be shared so we want to
  // store catchup tracking information in this class, as opposed to storing
  // it in a processMetFocusNode
  bool needsCatchup_;
  bool instrDeferred_;
  bool instrLoaded_;
  pd_process *proc_;
  bool dontInsertData_;
  int referenceCount;
  HwEvent* hwEvent;

 public:
  static pdstring construct_key_name(const pdstring &metricStr, 
				   const pdstring &focusStr) {
    return (metricStr + "-" + focusStr);
  }

  instrCodeNode_Val(const pdstring &name_, const Focus &f, pd_process *p,
		    bool dontInsertData, HwEvent* hw) : 
     sampledDataNode(NULL), constraintDataNode(NULL), name(name_), focus(f), 
     trampsNeedHookup_(false), needsCatchup_(false), instrDeferred_(false), 
     instrLoaded_(false), proc_(p), dontInsertData_(dontInsertData), 
     referenceCount(0), hwEvent(hw)
  { }

  instrCodeNode_Val(const instrCodeNode_Val &par, pd_process *childProc);
  
  ~instrCodeNode_Val();

  pdstring getKeyName();
  pdvector<instReqNode*> &getInstRequests() { return instRequests; }
  pdvector<returnInstance *> &getBaseTrampInstances() { 
    return baseTrampInstances;
  }
  bool getDontInsertData() const { return dontInsertData_; }
  void incrementRefCount() { referenceCount++; }
  void decrementRefCount() { referenceCount--; }
  int getRefCount() { return referenceCount; }
  void getDataNodes(pdvector<instrDataNode *> *saveBuf);
  pd_process *proc() {  return proc_;  }
  pdstring getName() const { return name; }
};

class instrCodeNode {
  static dictionary_hash<pdstring, instrCodeNode_Val*> allInstrCodeNodeVals;

  instrCodeNode_Val &V;

 private:
  instrCodeNode(instrCodeNode_Val *val) : V(*val) { }
  static void registerCodeNodeVal(instrCodeNode_Val *nodeVal);

  // a copy constructor variation
  instrCodeNode(const instrCodeNode &par, pd_process *childProc);

 public:
  static instrCodeNode *newInstrCodeNode(pdstring name_, const Focus &f,
				   pd_process *proc, bool arg_dontInsertData, 
                                   pdstring hw_cntr_str = "");
  static instrCodeNode *copyInstrCodeNode(const instrCodeNode &par,
					  pd_process *childProc);

  ~instrCodeNode();
  //bool condMatch(instrCodeNode *mn, pdvector<dataReqNode*> &data_tuple1,
  //               pdvector<dataReqNode*> &data_tuple2);
  pdvector<instReqNode*> getInstRequests() { return V.getInstRequests(); }
  inst_insert_result_t loadInstrIntoApp();
  long getID() { return reinterpret_cast<long>(&V); }
  instrCodeNode_Val *getInternalData() { return &V; }
  // should make it private

  void prepareCatchupInstr(pdvector<catchupReq *> &); 

  pdstring getName() const { return V.getName(); }
  int numDataNodes() { 
    return (((V.sampledDataNode != NULL) ? 1 : 0) +
	    ((V.constraintDataNode != NULL) ? 1 : 0) +
	    (V.tempCtrDataNodes.size()));
  }
  void setSampledDataNode(instrDataNode *dataNode) { 
    assert(V.sampledDataNode == NULL);
    V.sampledDataNode = dataNode;
  }
  void setConstraintDataNode(instrDataNode *dataNode) { 
    assert(V.constraintDataNode == NULL);
    V.constraintDataNode = dataNode;
  }
  void addTempCtrDataNode(instrDataNode *dataNode) { 
    V.tempCtrDataNodes.push_back(dataNode);
  }

  // ---------------------------------------
  void getDataNodes(pdvector<instrDataNode *> *saveBuf) { 
    V.getDataNodes(saveBuf);
  }
  void manuallyTrigger(int mid);
  void prepareForSampling(const pdvector<threadMetFocusNode *> &thrNodes);
  void prepareForSampling(threadMetFocusNode *thrNode);
  void stopSamplingThr(threadMetFocusNode_Val *thrNodeVal);
  bool hasDeferredInstr() { return V.instrDeferred_; }
  void markAsDeferred() {
    V.instrDeferred_ = true;
  }
  pd_process *proc() {  return V.proc(); }
  pd_process *proc() const {  return V.proc(); }
  void print();
  const Focus& getFocus() const { return V.focus; }
  static pdstring collectThreadName;
  const instrDataNode* getFlagDataNode() const;
  void markTrampsAsHookedUp() { V.trampsNeedHookup_ = false; }
  void markAsCatchupDone() { V.needsCatchup_ = false; }
  void unmarkAsDeferred() {
    V.instrDeferred_ = false;
  }

  // these aren't thread specific because the instrumentation deals with the
  // code and the code is shared by all the threads
  bool instrLoaded() { return V.instrLoaded_; }
  bool trampsNeedHookup() { return V.trampsNeedHookup_; }
  bool needsCatchup() { return V.needsCatchup_; }

  bool needToWalkStack(); // const;
  bool insertJumpsToTramps(pdvector<pdvector<Frame> > &stackWalks);
  void addInst(instPoint *point, AstNode *, callWhen when, callOrder o);
  timeLength cost() const;
  void oldCatchUp(int tid);
  void disable();

  void cleanup_drn();
  bool nonNull() const { return (V.instRequests.size() > 0);  }
  HwEvent* getHwEvent() { return V.hwEvent; }

};

#endif
