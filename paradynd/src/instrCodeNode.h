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
#include "paradynd/src/instReqNode.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/instrDataNode.h"

class instrDataNode;
class Focus;
class instrCodeNode;
class processMetFocusNode;
class threadMetFocusNode;
class threadMetFocusNode_Val;


class instrCodeNode_Val {
  friend class instrCodeNode;

  instrDataNode *sampledDataNode;
  instrDataNode *constraintDataNode;
  vector<instrDataNode*> tempCtrDataNodes;

  const string name;  // could be either a metric name or a constraint name
  const Focus focus;
  vector<instReqNode> instRequests;
  vector<returnInstance *> baseTrampInstances;
  bool _trampsHookedUp;
  bool instrDeferred_;
  bool instrLoaded_;
  bool hasBeenCatchuped_;
  process *proc_;
  bool dontInsertData_;
  int referenceCount;
#if defined(MT_THREAD)
  // remember names of each of its threads (tid + start_func_name)
  vector<string> thr_names;  
#endif

 public:
  static string construct_key_name(const string &metricStr, 
				   const string &focusStr) {
    return (metricStr + "-" + focusStr);
  }

  instrCodeNode_Val(const string &name_, const Focus &f, process *p,
		    bool dontInsertData) : 
    sampledDataNode(NULL), constraintDataNode(NULL), name(name_), focus(f), 
    _trampsHookedUp(false), instrDeferred_(false), instrLoaded_(false), 
    hasBeenCatchuped_(false), proc_(p), dontInsertData_(dontInsertData),
    referenceCount(0)
  { }

  instrCodeNode_Val(const instrCodeNode_Val &par, process *childProc);
  
  ~instrCodeNode_Val();

  string getKeyName();
  vector<instReqNode> &getInstRequests() { return instRequests; }
  vector<returnInstance *> &getBaseTrampInstances() { 
    return baseTrampInstances;
  }
  bool getDontInsertData() const { return dontInsertData_; }
  void incrementRefCount() { referenceCount++; }
  void decrementRefCount() { referenceCount--; }
  int getRefCount() { return referenceCount; }
  void getDataNodes(vector<instrDataNode *> *saveBuf);
  process *proc() {  return proc_;  }
  string getName() const { return name; }
};

class instrCodeNode {
  static dictionary_hash<string, instrCodeNode_Val*> allInstrCodeNodeVals;

  instrCodeNode_Val &V;

 private:
  instrCodeNode(instrCodeNode_Val *val) : V(*val) { }
  static void registerCodeNodeVal(instrCodeNode_Val *nodeVal);

  // a copy constructor variation
  instrCodeNode(const instrCodeNode &par, process *childProc);

 public:
  static instrCodeNode *newInstrCodeNode(string name_, const Focus &f,
				       process *proc, bool arg_dontInsertData);
  static instrCodeNode *copyInstrCodeNode(const instrCodeNode &par,
					  process *childProc);

  ~instrCodeNode();
  //bool condMatch(instrCodeNode *mn, vector<dataReqNode*> &data_tuple1,
  //               vector<dataReqNode*> &data_tuple2);
  vector<instReqNode> getInstRequests() { return V.getInstRequests(); }
  bool loadInstrIntoApp(pd_Function **func);
  int getID() { return reinterpret_cast<int>(&V); }
  instrCodeNode_Val *getInternalData() { return &V; }
  // should make it private

  bool hasBeenCatchuped() const { return V.hasBeenCatchuped_;};
  void prepareCatchupInstr(vector<vector<catchupReq *> >&); 

  string getName() const { return V.getName(); }
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
  void getDataNodes(vector<instrDataNode *> *saveBuf) { 
    V.getDataNodes(saveBuf);
  }
  void manuallyTrigger(int mid);
  void prepareForSampling(const vector<threadMetFocusNode *> &thrNodes);
  void prepareForSampling(threadMetFocusNode *thrNode);
  void stopSamplingThr(threadMetFocusNode_Val *thrNodeVal);
  bool hasDeferredInstr() { return V.instrDeferred_; }
  void markAsDeferred() {
    V.instrDeferred_ = true;
  }
  process *proc() {  return V.proc(); }
  process *proc() const {  return V.proc(); }
  void print();
  const Focus& getFocus() const { return V.focus; }
  static string collectThreadName;
  const instrDataNode* getFlagDataNode() const;
  void markTrampsAsHookedUp() { V._trampsHookedUp = true; }
  void unmarkAsDeferred() {
    V.instrDeferred_ = false;
  }
  bool instrLoaded() { return V.instrLoaded_; }
  bool trampsHookedUp() { return V._trampsHookedUp; }

  bool needToWalkStack(); // const;
  bool insertJumpsToTramps(vector<Frame> stackWalk);
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
