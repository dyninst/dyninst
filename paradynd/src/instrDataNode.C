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

#if defined(MT_THREAD)
//
// reUseIndex only works at the presence of an aggregator.
// We always need a dummy aggregator for threads metrics, and it is 
// implemented in mdl.C apply_to_process
//
void instrThrDataNode::reUseIndexAndLevel(unsigned &p_allocatedIndex, 
					  unsigned &p_allocatedLevel)
{
   p_allocatedIndex = UI32_MAX;
   p_allocatedLevel = UI32_MAX;

   instrCodeNode_Val *codeNode = getParent();
   vector<instrThrDataNode *> dataNodes = codeNode->getDataNodes();

   for (unsigned i=0; i<dataNodes.size(); i++) {
      instrThrDataNode *dataNode = dataNodes[i];
      
      if (dataNode != this) {
	 dataReqNode *p_dataRequest;
	 // Assume for all metrics, data are allocated in the same order
	 // we get the one that was created the earliest
	 
	 if (dataNode->numDataRequests() > this->numDataRequests()) {
	    vector<dataReqNode *> otherThrDRNs = dataNode->getDataRequests();
	    p_dataRequest = otherThrDRNs[otherThrDRNs.size()-1];
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
#endif //MT_THREAD

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

dataReqNode *instrThrDataNode::makeIntCounter(pdThread *thr, 
					      rawTime64 initialValue,
					      unsigned *level, unsigned *index,
					      bool doNotSample)
{
   dataReqNode *result = NULL;

#ifdef MT_THREAD
   // shared memory sampling of a reported intCounter
   result = new sampledShmIntCounterReqNode(thr, initialValue,
                                            incrementCounterId(), proc(),
					    dontInsertData_, doNotSample,
					    *index, *level);
#else
   result = new sampledShmIntCounterReqNode(initialValue,
                                            incrementCounterId(), proc(),
					    dontInsertData_, doNotSample);
#endif //MT_THREAD
   *level = result->getAllocatedLevel();
   *index = result->getAllocatedIndex();
      // implicit conversion to base class
   assert(result);
   
   proc()->numOfActCounters_is++;

   return result;
}

					      // if ST app, thr == NULL
dataReqNode *instrThrDataNode::createSampledCounter(pdThread *thr, 
						    rawTime64 initialValue,
						    dataInstHandle *handle)
{
   dataReqNode *result = NULL;
   handle->level = UI32_MAX;
   handle->index = UI32_MAX;
   result = makeIntCounter(thr, initialValue, &handle->level, &handle->index, 
			   false);
   assert(sampledDataReq == NULL);  // shouldn't have been set yet
   sampledDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::createConstraintCounter(pdThread *thr, 
						       rawTime64 initialValue,
						       dataInstHandle *handle)
{
   dataReqNode *result = NULL;
   handle->level = UI32_MAX;
   handle->index = UI32_MAX;
   result = makeIntCounter(thr, initialValue, &handle->level, &handle->index, 
			   false);
   assert(constraintDataReq == NULL);  // shouldn't have been set yet
   constraintDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::createTemporaryCounter(pdThread *thr, 
						      rawTime64 initialValue,
						      dataInstHandle *handle)
{
   dataReqNode *result = NULL;
   handle->level = UI32_MAX;
   handle->index = UI32_MAX;
   result = makeIntCounter(thr, initialValue, &handle->level, &handle->index, 
			   true);
   tempCtrDataRequests.push_back(result);
   return result;
}

dataReqNode *instrThrDataNode::reuseSampledCounter(pdThread *thr, 
						   rawTime64 initialValue,
					     const dataInstHandle &dataHandle)
{
   dataReqNode *result = NULL;
   unsigned level = dataHandle.level;
   unsigned index = dataHandle.index;
   result = makeIntCounter(thr, initialValue, &level, &index, false);
   assert(sampledDataReq == NULL);  // shouldn't have been set yet
   sampledDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::reuseConstraintCounter(pdThread *thr, 
						      rawTime64 initialValue,
					     const dataInstHandle &dataHandle)
{
   dataReqNode *result = NULL;
   unsigned level = dataHandle.level;
   unsigned index = dataHandle.index;
   result = makeIntCounter(thr, initialValue, &level, &index, false);
   assert(constraintDataReq == NULL);  // shouldn't have been set yet
   constraintDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::reuseTemporaryCounter(pdThread *thr, 
						     rawTime64 initialValue,
					     const dataInstHandle &dataHandle)
{
   dataReqNode *result = NULL;
   unsigned level = dataHandle.level;
   unsigned index = dataHandle.index;
   result = makeIntCounter(thr, initialValue, &level, &index, true);
   tempCtrDataRequests.push_back(result);
   return result;
}


dataReqNode *instrThrDataNode::makeWallTimer(pdThread *thr, unsigned *level,
					     unsigned *index)
{
   dataReqNode *result = NULL;

#if defined(MT_THREAD)
   result = new sampledShmWallTimerReqNode(thr, incrementCounterId(), 
					   proc(), dontInsertData_, 
					   *index, *level);
      // implicit conversion to base class
#else
   result = new sampledShmWallTimerReqNode(incrementCounterId(), proc(), 
					   dontInsertData_);
#endif //MT_THREAD
   *level = result->getAllocatedLevel();
   *index = result->getAllocatedIndex();

   assert(result);
   proc()->numOfActWallTimers_is++;
   assert(sampledDataReq == NULL);
   sampledDataReq = result;
   return result;
}

dataReqNode *instrThrDataNode::createWallTimer(pdThread *thr, 
					       dataInstHandle *handle) {
  handle->level = UI32_MAX;
  handle->index = UI32_MAX;
  return makeWallTimer(thr, &handle->level, &handle->index);
}

dataReqNode *instrThrDataNode::reuseWallTimer(pdThread *thr, 
					      const dataInstHandle &handle) {
  unsigned level = handle.level;
  unsigned index = handle.index;
  return makeWallTimer(thr, &level, &index);
}

dataReqNode *instrThrDataNode::makeProcessTimer(pdThread *thr, unsigned *level,
						unsigned *index)
{
   dataReqNode *result = NULL;

#if defined(MT_THREAD)
   result = new sampledShmProcTimerReqNode(thr, incrementCounterId(), 
					   proc(), dontInsertData_, 
					   *index, *level);
      // implicit conversion to base class
#else
   result = new sampledShmProcTimerReqNode(incrementCounterId(), proc(),
					   dontInsertData_);
#endif //MT_THREAD
   *level = result->getAllocatedLevel();
   *index = result->getAllocatedIndex();

   assert(result);

   proc()->numOfActProcTimers_is++;

   assert(sampledDataReq == NULL);
   sampledDataReq = result;
   return result;
};

dataReqNode *instrThrDataNode::createProcessTimer(pdThread *thr, 
						  dataInstHandle *handle) {
  handle->level = UI32_MAX;
  handle->index = UI32_MAX;
  return makeProcessTimer(thr, &handle->level, &handle->index);
}
dataReqNode *instrThrDataNode::reuseProcessTimer(pdThread *thr, 
						 const dataInstHandle &handle) 
{
  unsigned level = handle.level;
  unsigned index = handle.index;
  return makeProcessTimer(thr, &level, &index);
}

void instrThrDataNode::print() {
   cerr << "D:" << (void*)this << "\n";
}

void instrThrDataNode::startSampling(threadMetFocusNode_Val *thrClient) { 
  thrNodeClientSet = true;
  assert(sampledDataReq!=NULL);
  sampledDataReq->setThrNodeClient(thrClient, proc());
}

void instrThrDataNode::stopSampling() {
  thrNodeClientSet = false;
  assert(sampledDataReq!=NULL);
  sampledDataReq->setThrNodeClient(NULL, proc());
}

void instrThrDataNode::disableAndDelete(vector<addrVecType> pointsToCheck) {
  if(! dontInsertData_) {
    if(sampledDataReq!=NULL)  sampledDataReq->disable(proc(), pointsToCheck);
    if(constraintDataReq!=NULL) 
       constraintDataReq->disable(proc(), pointsToCheck);

    for (unsigned u=0; u<tempCtrDataRequests.size(); u++) {
      tempCtrDataRequests[u]->disable(proc(), pointsToCheck); // deinstrument
    }
  }
  delete this;
}

int indivInstrThrDataNode::getThreadID() const {
   return thrObj->get_tid();
}

