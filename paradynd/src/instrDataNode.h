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

#ifndef INSTR_THR_DATA_NODE
#define INSTR_THR_DATA_NODE

#include "paradynd/src/metric.h"

class dataReqNode;
class instrCodeNode_Val;


class instrThrDataNode {
 private:
  dataReqNode *sampledDataReq;
  dataReqNode *constraintDataReq;
  vector<dataReqNode*> tempCtrDataRequests;
  instrCodeNode_Val *parentNode;
  bool thrNodeClientSet;
  bool dontInsertData_;

  /* unique id for a counter or timer */
  static int counterId;

  dataReqNode *makeIntCounter(inst_var_index index, pdThread *thr, 
			      rawTime64 initialValue);
 protected:
  virtual ~instrThrDataNode(); // use disableAndDelete() to delete

 public:
  static int incrementCounterId() {  return ++counterId;  }
  // styles are enumerated in aggregation.h
  instrThrDataNode(instrCodeNode_Val *_parentNode, bool arg_dontInsertData);

  vector<dataReqNode *> getDataRequests();
  unsigned numDataRequests() const { 
    return tempCtrDataRequests.size() + ((sampledDataReq!=NULL)? 1:0)
                                      + ((constraintDataReq!=NULL)? 1:0);
  }
  bool dontInsertData() { return dontInsertData_; }
  bool nonNull() const { return (numDataRequests() > 0); }
  void disableAndDelete(vector< vector<Address> > pointsToCheck);
  const dataReqNode *getConstraintDRN() const { return constraintDataReq; }
  void cleanup_drn();
  instrCodeNode_Val *getParent() {
    return parentNode;
  }
  process *proc();
  void print();
  void startSampling(threadMetFocusNode_Val *thrClient);
  void stopSampling();

  // --- Counters -------------
  static inst_var_index allocateForCounter(process *proc,bool bDontInsertData);
  dataReqNode *createSampledCounter(inst_var_index index, pdThread *thr, 
				    rawTime64 initialValue);
  dataReqNode *createConstraintCounter(inst_var_index index, pdThread *thr, 
				       rawTime64 initialValue);
  dataReqNode *createTemporaryCounter(inst_var_index index, pdThread *thr, 
				      rawTime64 initialValue);

  // --- Wall Timers ----------
  static inst_var_index allocateForWallTimer(process *proc,
					     bool bDontInsertData);
  dataReqNode *createWallTimer(inst_var_index index, pdThread *thr);

  // --- Process Timers -------
  static inst_var_index allocateForProcTimer(process *proc,
					     bool bDontInsertData);
  dataReqNode *createProcessTimer(inst_var_index index, pdThread *thr);

  virtual int getThreadID() const = 0;
};

class collectInstrThrDataNode : public instrThrDataNode {
 protected:
  ~collectInstrThrDataNode();  // use disableAndDelete() to delete
 public:
  enum { collectThreadID = -1 };
  collectInstrThrDataNode(instrCodeNode *_parentNode, 
			  bool arg_dontInsertData);
  int getThreadID() const { return collectThreadID; }
};

class indivInstrThrDataNode : public instrThrDataNode {
 private:
  pdThread *thrObj;
 protected:
  ~indivInstrThrDataNode();  // use disableAndDelete() to delete
 public:
  indivInstrThrDataNode(instrCodeNode *_parentNode, 
			bool arg_dontInsertData, pdThread *thrObj_);
  int getThreadID() const;
  pdThread *getThreadObj() { return thrObj; }
};


#endif
