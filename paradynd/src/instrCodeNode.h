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

#ifndef INSTR_CODE_NODE
#define INSTR_CODE_NODE

#include "common/h/String.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/metric.h"
#include "paradynd/src/instReqNode.h"

class instrThrDataNode;

class instrCodeNode_Val {
 public:
  string name;
  vector<instReqNode> instRequests;
  vector<returnInstance *> baseTrampInstances;
  vector<instInstance *> miniTrampInstances;
  vector<instrThrDataNode *> dataNodes;
  vector<processMetFocusNode *> parentNodes;
  vector<instReqNode *> manuallyTriggerNodes;
  bool _baseTrampsHookedUp;
  bool instrDeferred_;
  bool instrLoaded_;
  process *proc_;
  bool dontInsertData_;
  int referenceCount;
#if defined(MT_THREAD)
  // remember names of each of its threads (tid + start_func_name)
  vector<string> thr_names;  
#endif

  instrCodeNode_Val(string name_, process *p) : name(name_), 
    _baseTrampsHookedUp(false), instrDeferred_(false), instrLoaded_(false), 
    proc_(p), referenceCount(0)
  { }
  ~instrCodeNode_Val();
  vector<instReqNode> &getInstRequests() { return instRequests; }
  vector<returnInstance *> &getBaseTrampInstances() { 
    return baseTrampInstances;
  }
  vector<instInstance *> &getMiniTrampInstances() { 
    return miniTrampInstances;
  }
  void incrementRefCount() { referenceCount++; }
  void decrementRefCount() { referenceCount--; }
  int getRefCount() { return referenceCount; }
  vector<instrThrDataNode *>& getDataNodes() { return dataNodes; }
  process *proc() {  return proc_;  }
};

class instrCodeNode {
  static dictionary_hash<string, instrCodeNode_Val*> allInstrCodeNodeVals;

  instrCodeNode_Val &V;

 private:
  instrCodeNode(instrCodeNode_Val *val) : V(*val) { }
 public:
  static instrCodeNode *newInstrCodeNode(string name_, process *proc,
					 bool arg_dontInsertData);
  ~instrCodeNode();
  bool condMatch(instrCodeNode *mn, vector<dataReqNode*> &data_tuple1,
		 vector<dataReqNode*> &data_tuple2);
  vector<dataReqNode *> getDataRequests();
  vector<instReqNode> getInstRequests() { return V.getInstRequests(); }
  bool loadInstrIntoApp(pd_Function **func);
  int getID() { return reinterpret_cast<int>(&V); }
  instrCodeNode_Val *getInternalData() { return &V; }
  // should make it private
#if defined(MT_THREAD)
  void prepareCatchupInstr0(int tid);
#endif
  void prepareCatchupInstr(vector<Address> stack_pcs, int tid); 
  bool catchupInstrNeeded() const {
    if( V.manuallyTriggerNodes.size() > 0 )  return true;
    else  return false;
  }
  void recordAsParent(processMetFocusNode *procobj);
  unsigned numParents() { return V.parentNodes.size(); }
  unsigned numDataNodes() { return V.dataNodes.size(); }
  // --- JUNK, REMOVE THIS SOMETIME SOON ---
  processMetFocusNode *getFirstParent() { 
    assert(V.parentNodes.size() > 0);
    return V.parentNodes[0];
  }
  // ---------------------------------------
  vector<instrThrDataNode *>& getDataNodes() { return V.getDataNodes(); }
  void manuallyTrigger(int mid);
  void mapSampledDRNs2ThrNodes(const vector<threadMetFocusNode *> &thrNodes);
  void stopSamplingThr(threadMetFocusNode_Val *thrNodeVal);
  bool hasDeferredInstr() { return V.instrDeferred_; }
  void markAsDeferred() {
    V.instrDeferred_ = true;
  }
  process *proc() {  return V.proc(); }
  process *proc() const {  return V.proc(); }
  void print();

  static string collectThreadName;
  instrThrDataNode* getThrDataNode(const string &tname);
  instrThrDataNode* getThrDataNode(int thr_id);
  const instrThrDataNode* getThrDataNode(int thr_id) const;
  const dataReqNode* getFlagDRN(int thr_id) const;
  void markBaseTrampsAsHookedUp() { V._baseTrampsHookedUp = true; }
  void unmarkAsDeferred() {
    V.instrDeferred_ = false;
  }
  bool instrLoaded() { return V.instrLoaded_; }
  bool baseTrampsHookedUp() { return V._baseTrampsHookedUp; }

  void addDataNode(instrThrDataNode* part) { V.dataNodes.push_back(part); }
  bool needToWalkStack(); // const;
  bool insertJumpsToTramps(vector<Address>& pc);
  void addInst(instPoint *point, AstNode *, callWhen when, callOrder o);
  timeLength cost() const;
  void oldCatchUp(int tid);
  void disable();
  void cleanup_drn();
  bool nonNull() const { return (V.instRequests.size() > 0);  }
#if defined(MT_THREAD)
  void addThrName(string thr_name) {  V.thr_names += thr_name;  }
  vector<string> getThrNames() {  return V.thr_names;  }
#endif

};

#endif
