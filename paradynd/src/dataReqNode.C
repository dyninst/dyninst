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
#include "paradynd/src/variableMgr.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/init.h"


void dataReqNode::markAsSampled(threadMetFocusNode_Val *thrNval) {
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsSampled(varType, varIndex, threadPos, thrNval);
}

void dataReqNode::markAsNotSampled() {
  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.markVarAsNotSampled(varType, varIndex, threadPos);
}

void dataReqNode::disable(const vector<addrVecType> &pointsToCheck) {
  // Remove from inferior heap; make sure we won't be sampled any more:
  vector<Address> trampsMaybeUsing;
  for (unsigned pointlcv=0; pointlcv < pointsToCheck.size(); pointlcv++)
    for (unsigned tramplcv=0; tramplcv < pointsToCheck[pointlcv].size(); 
	 tramplcv++) {
      trampsMaybeUsing += pointsToCheck[pointlcv][tramplcv];
    }

  variableMgr &varMgr = proc->getVariableMgr();
  varMgr.makePendingFree(varType, varIndex, threadPos, trampsMaybeUsing);

  //if (MT && theProc->numOfActCounters_is>0) theProc->numOfActCounters_is--;
}

Address dataReqNode::getInferiorPtr() const {
  // counterPtr could be NULL if we are building AstNodes just to compute
  // the cost - naim 2/18/97
  // NOTE:
  // this routine will dissapear because we can't compute the address
  // of the counter/timer without knowing the thread id - naim 3/17/97
  //
  Address varAddr;
  if(getDontInsertData()) {
    varAddr = 0;
  } else {
    variableMgr &varMgr = proc->getVariableMgr();
    // we assume there is only one thread
    varAddr = (Address)varMgr.shmVarApplicAddr(varType, varIndex, threadPos);
  }
  return varAddr;
}

/* ************************************************************************* */


sampledIntCounterReqNode::
sampledIntCounterReqNode(process *proc, inst_var_index varIndex,
			 pdThread *thr, rawTime64 iValue, 
			 int iCounterId, bool dontInsertData_) :
  dataReqNode(proc, Counter, varIndex, ((thr==NULL) ? 0 : thr->get_pd_pos()),
	      iCounterId, dontInsertData_),
  initialValue(iValue)
{
}

sampledIntCounterReqNode::
sampledIntCounterReqNode(const sampledIntCounterReqNode &src,
			 process */*childProc*/,
			 int /*iCounterId*/, const process */*parentProc*/) :
  dataReqNode(src)
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

  /*
   this->allocatedIndex = src.allocatedIndex;
   this->allocatedLevel = src.allocatedLevel;

   this->theSampleId = iCounterId;  // this is different from the parent's value
   this->initialValue = src.initialValue;

   superTable &varMgr = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(varMgr.index2InferiorAddr(0,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(0,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     // write to the raw item in the inferior heap:
     intCounter *localCounterPtr = (intCounter *) varMgr.index2LocalAddr(0,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
     localCounterPtr->value = initialValue;
     localCounterPtr->id.id = theSampleId;
   }

   // write HK for this intCounter:
   assert(theSampleId >= 0);
   intCounterHK iHKValue(theSampleId, NULL);

      // the mi differs from the mi of the parent; theSampleId differs too.
   varMgr.initializeHKAfterForkIntCounter(allocatedIndex, allocatedLevel, 
					    iHKValue);
   //position_=0;
   */
}

dataReqNode *
sampledIntCounterReqNode::dup(process *childProc, int iCounterId,
	                  const dictionary_hash<instInstance*,instInstance*> &)
  const {
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledIntCounterReqNode *tmp;
   tmp = new sampledIntCounterReqNode(*this, childProc,
				      iCounterId, childProc->getParent());
      // fork ctor

   return tmp;
}


/* ************************************************************************* */

sampledWallTimerReqNode::
sampledWallTimerReqNode(process *proc, inst_var_index varIndex,
			pdThread *thr, int iCounterId, bool dontInsertData_) :
  dataReqNode(proc, WallTimer, varIndex, ((thr==NULL) ? 0 : thr->get_pd_pos()),
	      iCounterId, dontInsertData_)
{
}

sampledWallTimerReqNode::
sampledWallTimerReqNode(const sampledWallTimerReqNode &src,
			process */*childProc*/, int /*iCounterId*/, 
			const process */*parentProc*/) :
  dataReqNode(src)
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

  /*
   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;

   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &varMgr = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(varMgr.index2InferiorAddr(1,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(1,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value in the inferior heap: we set localTimerPtr as
   // follows: protector1 and procetor2 should be copied from src. total
   // should be reset to 0.  start should be set to now if active else 0.
   // counter should be copied from the source.

   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK,
   // BOTH THE PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT
   // THE CHILD IS PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS
   // NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) varMgr.index2LocalAddr(1,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
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

   // write new HK for this tTimer: Note: we don't assert anything about
   // dataNode->getMetricID(), because that id has no relation to the ids we
   // work with (theSampleId).  In fact, we (the sampling code) just don't
   // ever care what dataNode->getMetricID() is.
   assert(theSampleId >= 0);
   // the NULL below should probably be the threadNode of the dup'd HK node
   wallTimerHK iHKValue(theSampleId, NULL, timeLength::Zero()); 
   // the mi should differ from the mi of the parent; theSampleId differs too.
   varMgr.initializeHKAfterForkWallTimer(allocatedLevel, allocatedIndex, 
					   iHKValue);
  */
   //position_=0;
}

dataReqNode *
sampledWallTimerReqNode::dup(process *childProc,
			     int iCounterId,
			     const dictionary_hash<instInstance*,
			     instInstance*> &) const
{
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledWallTimerReqNode *tmp;
   tmp = new sampledWallTimerReqNode(*this, childProc, iCounterId,
					childProc->getParent());
      // fork constructor

   return tmp;
}

/* ****************************************************************** */

sampledProcTimerReqNode::
sampledProcTimerReqNode(process *proc, inst_var_index varIndex,
			pdThread *thr, int iCounterId, bool dontInsertData_) :
  dataReqNode(proc, ProcTimer, varIndex, ((thr==NULL) ? 0 : thr->get_pd_pos()),
	      iCounterId, dontInsertData_)
{
}

sampledProcTimerReqNode::
sampledProcTimerReqNode(const sampledProcTimerReqNode &src,
			process */*childProc*/, int /*iCounterId*/, 
			const process */*parentProc*/) :
  dataReqNode(src)
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

  /*
   allocatedIndex = src.allocatedIndex;
   allocatedLevel = src.allocatedLevel;
   theSampleId = iCounterId;
   assert(theSampleId != src.theSampleId);

   superTable &varMgr = childProc->getTable();

   // since the new shm seg is placed in exactly the same memory location as
   // the old one, nothing here should change.
   const superTable &theParentTable = parentProc->getTable();
   assert(varMgr.index2InferiorAddr(2,childProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel)==theParentTable.index2InferiorAddr(2,parentProc->threads[0]->get_pd_pos(),allocatedIndex,allocatedLevel));

   // Write new raw value: we set localTimerPtr as follows: protector1 and
   // procetor2 should be copied from src. total should be reset to 0.  start
   // should be set to now if active else 0.  counter should be copied from
   // the source.
   // NOTE: SINCE WE COPY FROM THE SOURCE, IT'S IMPORTANT THAT ON A FORK,
   // BOTH THE PARENT AND CHILD ARE PAUSED UNTIL WE COPY THINGS OVER.  THAT
   // THE CHILD IS PAUSED IS NOTHING NEW; THAT THE PARENT SHOULD BE PAUSED IS
   // NEW NEWS!

   for (unsigned i=0; i<childProc->threads.size(); i++) {
     tTimer *localTimerPtr = (tTimer *) varMgr.index2LocalAddr(2,childProc->threads[i]->get_pd_pos(),allocatedIndex,allocatedLevel);
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
       localTimerPtr->start =childProc->getRawCpuTime(localTimerPtr->lwp_id);
#else
       localTimerPtr->start = childProc->getRawCpuTime(-1);
#endif
     }
   }

   // Write new HK for this tTimer:

   // Note: we don't assert anything about dataNode->getMetricID(), because
   // that id has no relation to the ids we work with (theSampleId).  In
   // fact, we (the sampling code) just don't ever care what
   // dataNode->getMetricID() is.

   assert(theSampleId >= 0);
   // the NULL below should probably be the threadNode of the dup'd HK node
   processTimerHK iHKValue(theSampleId, NULL, timeLength::Zero());
      // the mi differs from the mi of the parent; theSampleId differs too.
   varMgr.initializeHKAfterForkProcTimer(allocatedLevel, allocatedIndex, 
					   iHKValue);
  */
   //position_=0;
}

dataReqNode *sampledProcTimerReqNode::dup(
                process *childProc, int iCounterId,
		const dictionary_hash<instInstance*,instInstance*> &) const
{
   // duplicate 'this' (allocate w/ new) and return.  Call after a fork().

   sampledProcTimerReqNode *tmp;
   tmp = new sampledProcTimerReqNode(*this, childProc, iCounterId,
				     childProc->getParent());
      // fork constructor

   return tmp;
}

