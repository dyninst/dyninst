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

#include "paradynd/src/dataReqNode.h"
#include "dyninstAPI/src/process.h"
#include "common/h/Types.h"
#include "paradynd/src/superTable.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/init.h"

#if defined(MT_THREAD)
int sampledShmIntCounterReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}
#endif

#if defined(MT_THREAD)
sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(
                                pdThread *thr, rawTime64 iValue, 
				int iCounterId, process *proc,
				bool dontInsertData, bool doNotSample,
				unsigned p_allocatedIndex,
				unsigned p_allocatedLevel)
#else
sampledShmIntCounterReqNode::sampledShmIntCounterReqNode(
				rawTime64 iValue, int iCounterId, 
				process *proc, bool dontInsertData,
				bool doNotSample)
#endif
{
   theSampleId = iCounterId;
   initialValue = iValue;
#if defined(MT_THREAD)
   thr_ = thr;
#endif

   // The following fields are NULL until loadInstrIntoApp()
#if defined(MT_THREAD)
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UI32_MAX;
   allocatedLevel = UI32_MAX;
#endif

   position_=0;

   if (!dontInsertData) {
     bool isOk=false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, proc, doNotSample);
#else
     isOk = insertShmVar(proc, doNotSample);
#endif
     assert(isOk); 
   }
}

sampledShmIntCounterReqNode::
sampledShmIntCounterReqNode(const sampledShmIntCounterReqNode &src,
			    process *childProc,
			    int iCounterId, const process *parentProc)
{
   // a dup() routine (call after a fork()) Assumes that "childProc" has been
   // copied already (e.g., the shm seg was copied).

   // Note that the index w/in the inferior heap remains the same, so setting
   // the new inferiorCounterPtr isn't too hard.  Actually, it's trivial,
   // since other code ensures that the new shm segment is placed in exactly
   // the same virtual mem loc as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already
   // copied the actual data; we need to fill in new meta-data (new
   // houseKeeping entries).

   this->allocatedIndex = src.allocatedIndex;
   this->allocatedLevel = src.allocatedLevel;

   this->theSampleId = iCounterId;  // this is different from the parent's value
   this->initialValue = src.initialValue;

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(0,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(0,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     // write to the raw item in the inferior heap:
     intCounter *localCounterPtr = (intCounter *) theTable.index2LocalAddr(0,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     localCounterPtr->value = initialValue;
     localCounterPtr->id.id = theSampleId;
   }

   // write HK for this intCounter:
   assert(theSampleId >= 0);
   intCounterHK iHKValue(theSampleId, NULL);

      // the mi differs from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkIntCounter(allocatedIndex, allocatedLevel, 
					    iHKValue);
   position_=0;
}

dataReqNode *
sampledShmIntCounterReqNode::dup(process *childProc, int iCounterId,
	                  const dictionary_hash<instInstance*,instInstance*> &)
  const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmIntCounterReqNode *tmp;
   tmp = new sampledShmIntCounterReqNode(*this, childProc,
					 iCounterId, childProc->getParent());
      // fork ctor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmIntCounterReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmIntCounterReqNode::insertShmVar(process *theProc,
#endif
				  bool doNotSample)
{
   // Remember counterPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the intCounter in the inferior heap
   intCounter iValue;
   iValue.id.id = this->theSampleId;
   iValue.value = this->initialValue;

   intCounterHK iHKValue(this->theSampleId, NULL);

   superTable &theTable = theProc->getTable();
#if defined(MT_THREAD)
   if (thr==NULL) thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocIntCounter(thr_pos, iValue, iHKValue, 
				 this->allocatedIndex, this->allocatedLevel, 
				 doNotSample))
     return false;  // failure
#else
   if (!theTable.allocIntCounter(iValue, iHKValue, this->allocatedIndex, 
				 this->allocatedLevel, doNotSample))
     return false;  // failure
#endif

   return true; // success
}

void sampledShmIntCounterReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();
  
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 0, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActCounters_is>0) theProc->numOfActCounters_is--;
#else
  theTable.makePendingFree(0,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}

/* ************************************************************************* */

#if defined(MT_THREAD)
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(
                                pdThread *thr, int iCounterId,
				process *proc, bool dontInsertData,
				unsigned p_allocatedIndex,
				unsigned p_allocatedLevel)
{
#else
sampledShmWallTimerReqNode::sampledShmWallTimerReqNode(
                                int iCounterId, process *proc,
				bool dontInsertData)
{
#endif
   theSampleId = iCounterId;

   // The following fields are NULL until insertShmVar():
#if defined(MT_THREAD)
   thr_ = thr;
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UI32_MAX;
   allocatedLevel = UI32_MAX;
#endif

   position_=0;

   if (!dontInsertData) {
     bool isOk = false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, proc);
#else
     isOk = insertShmVar(proc);
#endif
     assert(isOk); 
   }
}

sampledShmWallTimerReqNode::
sampledShmWallTimerReqNode(const sampledShmWallTimerReqNode &src,
			   process *childProc, int iCounterId, 
			   const process *parentProc)
{
   // a dup()-like routine; call after a fork().
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting
   // the new inferiorTimerPtr isn't too hard.  Actually, it's trivial, since
   // other code ensures that the new shm segment is placed in exactly the
   // same virtual mem loc as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already
   // copied the actual data; we need to fill in new meta-data (new
   // houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;

   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(1,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(1,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value in the inferior heap:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) theTable.index2LocalAddr(1,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     const tTimer *srcTimerPtr = (const tTimer *) childProc->getParent()->getTable().index2LocalAddr(1,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);

     localTimerPtr->total = 0;
     localTimerPtr->counter = srcTimerPtr->counter;
     localTimerPtr->id.id   = theSampleId;
     localTimerPtr->protector1 = srcTimerPtr->protector1;
     localTimerPtr->protector2 = srcTimerPtr->protector2;

     if (localTimerPtr->counter == 0)
        // inactive timer...this is the easy case to copy
        localTimerPtr->start = 0; // undefined, really
     else
        // active timer...don't copy the start time from the source...make it 'now'
        localTimerPtr->start = getRawWallTime();
   }

   // write new HK for this tTimer: 
   // Note: we don't assert anything about dataNode->getMetricID(), because that id
   // has no relation to the ids we work with (theSampleId).  In fact, we
   // (the sampling code) just don't ever care what dataNode->getMetricID() is.
   assert(theSampleId >= 0);
   // the NULL below should probably be the threadNode of the dup'd HK node
   wallTimerHK iHKValue(theSampleId, NULL, timeLength::Zero()); 
   // the mi should differ from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkWallTimer(allocatedIndex, allocatedLevel, 
					   iHKValue);

   position_=0;
}

dataReqNode *
sampledShmWallTimerReqNode::dup(process *childProc,
				int iCounterId,
				const dictionary_hash<instInstance*,
				instInstance*> &) const
{
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmWallTimerReqNode *tmp;
   tmp = new sampledShmWallTimerReqNode(*this, childProc, iCounterId,
					childProc->getParent());
      // fork constructor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmWallTimerReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmWallTimerReqNode::insertShmVar(process *theProc,
#endif
					      bool)
{
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   wallTimerHK iHKValue(this->theSampleId, NULL, timeLength::Zero());

   superTable &theTable = theProc->getTable();

#if defined(MT_THREAD)
   thr_ = thr;
   if (thr==NULL) thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocWallTimer(thr_pos, iValue, iHKValue, 
				this->allocatedIndex, this->allocatedLevel))
#else
   if (!theTable.allocWallTimer(iValue, iHKValue, this->allocatedIndex, 
				this->allocatedLevel))
#endif
      return false; // failure

   return true;
}

void sampledShmWallTimerReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();
  
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 1, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActWallTimers_is>0) theProc->numOfActWallTimers_is--;
#else
  theTable.makePendingFree(1,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}

/* ****************************************************************** */

#if defined(MT_THREAD)
int sampledShmProcTimerReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}
#endif

#if defined(MT_THREAD)
sampledShmProcTimerReqNode::sampledShmProcTimerReqNode(
                              pdThread *thr, int iCounterId,
			      process *proc, bool dontInsertData,
			      unsigned p_allocatedIndex,
			      unsigned p_allocatedLevel) {
#else
sampledShmProcTimerReqNode::sampledShmProcTimerReqNode(
			      int iCounterId, process *proc,
			      bool dontInsertData) {
#endif
   theSampleId = iCounterId;

   // The following fields are NULL until insertInstrumentatoin():
#if defined(MT_THREAD)
   thr_ = thr;
   allocatedIndex = p_allocatedIndex;
   allocatedLevel = p_allocatedLevel;
#else
   allocatedIndex = UI32_MAX;
   allocatedLevel = UI32_MAX;
#endif

   position_=0;

   if (!dontInsertData) {
     bool isOk=false;
#if defined(MT_THREAD)
     isOk = insertShmVar(thr, proc);
#else
     isOk = insertShmVar(proc);
#endif
     assert(isOk); 
   }
}

sampledShmProcTimerReqNode::
sampledShmProcTimerReqNode(const sampledShmProcTimerReqNode &src,
			   process *childProc, int iCounterId, 
			   const process *parentProc)
{
   // a dup()-like routine; call after a fork()
   // Assumes that the "childProc" has been duplicated already

   // Note that the index w/in the inferior heap remains the same, so setting
   // the new inferiorTimerPtr isn't too hard.  Actually, it's trivial, since
   // other code ensures that the new shm segment is placed in exactly the
   // same virtual mem loc as the previous one.
   //
   // Note that the fastInferiorHeap class's fork ctor will have already
   // copied the actual data; we need to fill in new meta-data (new
   // houseKeeping entries).

   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;
   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &theTable = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(theTable.index2InferiorAddr(2,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(2,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value:
   // we set localTimerPtr as follows: protector1 and procetor2 should be copied from
   //    src. total should be reset to 0.  start should be set to now if active else 0.
   //    counter should be copied from the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK, BOTH THE
   //       PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT THE CHILD IS
   //       PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) theTable.index2LocalAddr(2,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     const tTimer *srcTimerPtr = (const tTimer *) childProc->getParent()->getTable().index2LocalAddr(2,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);

     localTimerPtr->total = 0;
     localTimerPtr->counter = srcTimerPtr->counter;
     localTimerPtr->id.id   = theSampleId;
     localTimerPtr->protector1 = srcTimerPtr->protector1;
     localTimerPtr->protector2 = srcTimerPtr->protector2;

     if (localTimerPtr->counter == 0) {
        // inactive timer...this is the easy case to copy
        localTimerPtr->start = 0; // undefined, really
     } else {
        // active timer...don't copy the start time from the source...make it 'now'
#if defined(MT_THREAD)
        localTimerPtr->start = childProc->getRawCpuTime(localTimerPtr->lwp_id);
#else
        localTimerPtr->start = childProc->getRawCpuTime(-1);
#endif
     }
   }

   // Write new HK for this tTimer:

   // Note: we don't assert anything about dataNode->getMetricID(), because that id
   // has no relation to the ids we work with (theSampleId).  In fact, we
   // (the sampling code) just don't ever care what dataNode->getMetricID() is.
   assert(theSampleId >= 0);
   // the NULL below should probably be the threadNode of the dup'd HK node
   processTimerHK iHKValue(theSampleId, NULL, timeLength::Zero());
      // the mi differs from the mi of the parent; theSampleId differs too.
   theTable.initializeHKAfterForkProcTimer(allocatedIndex, allocatedLevel, 
					   iHKValue);

   position_=0;
}

dataReqNode *sampledShmProcTimerReqNode::dup(
                process *childProc, int iCounterId,
		const dictionary_hash<instInstance*,instInstance*> &) const
{
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledShmProcTimerReqNode *tmp;
   tmp = new sampledShmProcTimerReqNode(*this, childProc, iCounterId,
					childProc->getParent());
      // fork constructor

   return tmp;
}

#if defined(MT_THREAD)
bool sampledShmProcTimerReqNode::insertShmVar(pdThread *thr, process *theProc,
#else
bool sampledShmProcTimerReqNode::insertShmVar(
				         process *theProc,
#endif
				         bool)
{
   // Remember inferiorTimerPtr is NULL until this routine gets called.
   // WARNING: there will be an assert failure if the applic hasn't yet attached to the
   //          shm segment!!!

   // initialize the tTimer in the inferior heap
   tTimer iValue;
   P_memset(&iValue, '\0', sizeof(tTimer));
   iValue.id.id = this->theSampleId;

   processTimerHK iHKValue(this->theSampleId, NULL, timeLength::Zero());

   superTable &theTable = theProc->getTable();
#if defined(MT_THREAD)
   if (thr==NULL) 
     thr = theProc->threads[0]; // default value for thread - naim
   assert(thr!=NULL);
   unsigned thr_pos = thr->get_pd_pos();
#endif

#if defined(MT_THREAD)
   if (!theTable.allocProcTimer(thr_pos, iValue, iHKValue, 
				this->allocatedIndex, this->allocatedLevel))
#else
   if (!theTable.allocProcTimer(iValue, iHKValue, this->allocatedIndex,
				this->allocatedLevel))
#endif
      return false; // failure

   return true;
}

void sampledShmProcTimerReqNode::disable(process *theProc,
				    const vector<addrVecType> &pointsToCheck) {
  superTable &theTable = theProc->getTable();
  
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

#if defined(MT_THREAD)
  theTable.makePendingFree(thr_, 2, allocatedIndex, allocatedLevel,
			   trampsMaybeUsing);
  if (theProc->numOfActProcTimers_is>0) theProc->numOfActProcTimers_is--;
#else
  theTable.makePendingFree(2,allocatedIndex,allocatedLevel,trampsMaybeUsing);
#endif
}


#if defined(MT_THREAD)
int sampledShmWallTimerReqNode::getThreadId() const {
  assert(thr_);
  return(thr_->get_tid());
}
#endif //MT_THREAD

Address sampledShmIntCounterReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UI32_MAX || allocatedLevel == UI32_MAX) {
      return 0;
    }    
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(0,0,allocatedIndex,allocatedLevel));
}

Address sampledShmWallTimerReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UI32_MAX || allocatedLevel == UI32_MAX) {
      return 0;
    }
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(1,0,allocatedIndex,allocatedLevel));
}

Address sampledShmProcTimerReqNode::getInferiorPtr(process *proc) const {
    // counterPtr could be NULL if we are building AstNodes just to compute
    // the cost - naim 2/18/97
    // NOTE:
    // this routine will dissapear because we can't compute the address
    // of the counter/timer without knowing the thread id - naim 3/17/97
    //
    if (allocatedIndex == UI32_MAX || allocatedLevel == UI32_MAX) {
      return 0;
    }
    assert(proc != NULL);
    superTable &theTable = proc->getTable();
    // we assume there is only one thread
    return((Address) theTable.index2InferiorAddr(2,0,allocatedIndex,allocatedLevel));
}

void sampledShmIntCounterReqNode::setThrNodeClient(
					      threadMetFocusNode_Val *thrNval,
					      process *proc)
{
  superTable &theTable = proc->getTable();
  void *ptr = theTable.getHouseKeeping(0, 
#if defined(MT_THREAD)
				       getThread(), 
#endif 
				       allocatedIndex, allocatedLevel);
  intCounterHK *hkPtr = reinterpret_cast<intCounterHK*>(ptr);
  hkPtr->setThrClient(thrNval);
}

void sampledShmWallTimerReqNode::setThrNodeClient(
					      threadMetFocusNode_Val *thrNval, 
					      process *proc)
{
  superTable &theTable = proc->getTable();
  void *ptr = theTable.getHouseKeeping(1, 
#if defined(MT_THREAD)
				       getThread(), 
#endif
				       allocatedIndex, allocatedLevel);
  wallTimerHK *hkPtr = reinterpret_cast<wallTimerHK*>(ptr);
  hkPtr->setThrClient(thrNval);
}

void sampledShmProcTimerReqNode::setThrNodeClient(
                                               threadMetFocusNode_Val *thrNval,
					       process *proc)
{
  superTable &theTable = proc->getTable();
  void *ptr = theTable.getHouseKeeping(2, 
#if defined(MT_THREAD)
				       getThread(),
#endif
                                       allocatedIndex, allocatedLevel);
  processTimerHK *hkPtr = reinterpret_cast<processTimerHK*>(ptr);
  hkPtr->setThrClient(thrNval);
}
