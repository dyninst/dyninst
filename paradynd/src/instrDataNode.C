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

#include "common/h/Types.h"
#include "paradynd/src/instrThrDataNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/dataReqNode.h"


extern pdDebug_ostream sampleVal_cerr;
extern pdDebug_ostream metric_cerr;


instrThrDataNode::instrThrDataNode(instrCodeNode_Val *_parentNode, 
				   bool arg_dontInsertData)
  : sampledDataReq(NULL), constraintDataReq(NULL), parentNode(_parentNode), 
    thrNodeClientSet(false), dontInsertData_(arg_dontInsertData)
{ 
}

collectInstrThrDataNode::collectInstrThrDataNode(instrCodeNode *_parentNode,
					        bool arg_dontInsertData)
  : instrThrDataNode(_parentNode->getInternalData(), arg_dontInsertData)
{ 
}

indivInstrThrDataNode::indivInstrThrDataNode(instrCodeNode *_parentNode,
				          bool arg_dontInsertData, 
					     pdThread *thrObj_)
  : instrThrDataNode(_parentNode->getInternalData(), arg_dontInsertData), 
                     thrObj(thrObj_)
{ 
}

collectInstrThrDataNode::~collectInstrThrDataNode() { }

indivInstrThrDataNode::~indivInstrThrDataNode() { }

// the disable method should be called before this destructor
instrThrDataNode::~instrThrDataNode() {
  vector<addrVecType> pointsToCheck;
  delete sampledDataReq;
  sampledDataReq = NULL;
  delete constraintDataReq;
  constraintDataReq = NULL;

  for(unsigned i=0; i<tempCtrDataRequests.size(); i++) {
    delete tempCtrDataRequests[i];
  }
}

process *instrThrDataNode::proc() { 
  return parentNode->proc();
}

vector<dataReqNode *> instrThrDataNode::getDataRequests() { 
  vector<dataReqNode*> buff;
  if(sampledDataReq != NULL)  buff.push_back(sampledDataReq);
  if(constraintDataReq != NULL)  buff.push_back(constraintDataReq);
  for(unsigned i=0; i<tempCtrDataRequests.size(); i++) {
    buff.push_back(tempCtrDataRequests[i]);
  }
  return buff;
}

// obligatory definition of static member vrble:
int instrThrDataNode::counterId=0;
extern process *global_proc;

// --- Counters --------------------

inst_var_index instrThrDataNode::
allocateForCounter(process *proc, bool bDontInsertData) {
  inst_var_index retIndex;
  if(bDontInsertData) {
    retIndex = 0;  // this isn't actually used, but need to return something
  } else {
    retIndex = dataReqNode::allocateForInstVar(proc, Counter);
  }
  return retIndex;
}

dataReqNode *instrThrDataNode::makeIntCounter(inst_var_index index, 
					      pdThread *thr, 
					      rawTime64 initialValue)
{
   dataReqNode *result = 
     new sampledIntCounterReqNode(proc(), index, thr, initialValue,
				  incrementCounterId(), dontInsertData_);
   assert(result);
   proc()->numOfActCounters_is++;
   return result;
}
					      // if ST app, thr == NULL
dataReqNode *instrThrDataNode::createSampledCounter(inst_var_index index,
						    pdThread *thr, 
						    rawTime64 initialValue)
{
   dataReqNode *result = makeIntCounter(index, thr, initialValue);
   assert(sampledDataReq == NULL);  // shouldn't have been set yet
   sampledDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::createConstraintCounter(inst_var_index index,
						       pdThread *thr, 
						       rawTime64 initialValue)
{
   dataReqNode *result = makeIntCounter(index, thr, initialValue);
   assert(constraintDataReq == NULL);  // shouldn't have been set yet
   constraintDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::createTemporaryCounter(inst_var_index index,
						      pdThread *thr, 
						      rawTime64 initialValue)
{
   dataReqNode *result = makeIntCounter(index, thr, initialValue);
   tempCtrDataRequests.push_back(result);
   return result;
}

// --- Wall Timers -------------
inst_var_index instrThrDataNode::
allocateForWallTimer(process *proc, bool bDontInsertData) {
  inst_var_index retIndex;
  if(bDontInsertData) {
    retIndex = 0;  // this isn't actually used, but need to return something
  } else {
    retIndex = dataReqNode::allocateForInstVar(proc, WallTimer);
  }
  return retIndex;
}

dataReqNode *instrThrDataNode::createWallTimer(inst_var_index index,
					       pdThread *thr)
{
   dataReqNode *result = 
     new sampledWallTimerReqNode(proc(), index, thr, incrementCounterId(), 
				 dontInsertData_);
   assert(result);
   proc()->numOfActWallTimers_is++;
   assert(sampledDataReq == NULL);
   sampledDataReq = result;
   return result;
}

// --- Proc Timers -------------
inst_var_index instrThrDataNode::
allocateForProcTimer(process *proc, bool bDontInsertData) {
  inst_var_index retIndex;
  if(bDontInsertData) {
    retIndex = 0;  // this isn't actually used, but need to return something
  } else {
    retIndex = dataReqNode::allocateForInstVar(proc, ProcTimer);
  }
  return retIndex;
}

dataReqNode *instrThrDataNode::createProcessTimer(inst_var_index index,
						  pdThread *thr)
{
  dataReqNode *result = 
    new sampledProcTimerReqNode(proc(), index, thr, incrementCounterId(),
				dontInsertData_);
  assert(result);
  
  proc()->numOfActProcTimers_is++;
   
  assert(sampledDataReq == NULL);
  sampledDataReq = result;
  return result;
};


void instrThrDataNode::print() {
   cerr << "D:" << (void*)this << "\n";
}

void instrThrDataNode::startSampling(threadMetFocusNode_Val *thrClient) { 
  thrNodeClientSet = true;
  assert(sampledDataReq!=NULL);
  sampledDataReq->setThrNodeClient(thrClient);
  sampledDataReq->markAsSampled();
}

void instrThrDataNode::stopSampling() {
  thrNodeClientSet = false;
  assert(sampledDataReq!=NULL);
  sampledDataReq->setThrNodeClient(NULL);
  sampledDataReq->markAsNotSampled();
}

void instrThrDataNode::disableAndDelete(vector<addrVecType> pointsToCheck) {
  if(! dontInsertData_) {
    if(sampledDataReq!=NULL)  sampledDataReq->disable(pointsToCheck);
    if(constraintDataReq!=NULL) 
       constraintDataReq->disable(pointsToCheck);

    for (unsigned u=0; u<tempCtrDataRequests.size(); u++) {
      tempCtrDataRequests[u]->disable(pointsToCheck); // deinstrument
    }
  }
  delete this;
}

int indivInstrThrDataNode::getThreadID() const {
   return thrObj->get_tid();
}

