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

// $Id: superVector.C,v 1.18 2002/04/23 18:58:40 schendel Exp $

#include <sys/types.h>
#include "common/h/Types.h"
#include "common/h/headers.h"
#include "paradynd/src/superVector.h"
#include "paradynd/src/shmSegment.h"
#include "paradynd/src/fastInferiorHeapMgr.h"
#include "paradynd/src/fastInferiorHeap.h"
#include "rtinst/h/rtinst.h" // for time64
#include "dyninstAPI/src/pdThread.h"


template <class HK, class RAW>
superVector<HK, RAW>::
superVector(process *iInferiorProcess,
	    unsigned heapNumElems,
	    unsigned level,
	    unsigned subHeapIndex,
	    unsigned numberOfColumns,
	    bool calledFromBaseTableConst):
	    inferiorProcess(iInferiorProcess),
            varStates(heapNumElems)
{
   assert(heapNumElems > 0);

   // Initialize varStates by setting all entries to state 'free'
   for (unsigned ndx=0; ndx < varStates.size(); ndx++)
      varStates[ndx] = varFree;
   
   firstFreeIndex = 0;

   RAW *iBaseAddrInParadynd;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   unsigned adr_offset, col, maxCol;
   maxCol = inferiorProcess->getTable().getMaxSize();
   iBaseAddrInParadynd = (RAW *) iHeapMgr.getSubHeapInParadynd(subHeapIndex);
   assert(iBaseAddrInParadynd);
   
   for (unsigned i=0; i<numberOfColumns; i++) {
      col = i;
      if(inferiorProcess->is_multithreaded()) {
	 adr_offset = (level*maxCol + col) * heapNumElems;
      } else {
	 adr_offset = i * heapNumElems;
      }
      fastInferiorHeap<HK, RAW> *theFastInferiorHeap = 
	 new fastInferiorHeap<HK, RAW>(iBaseAddrInParadynd+adr_offset, 
				       iInferiorProcess, varStates.size());
      assert(theFastInferiorHeap != NULL);

      fastInferiorHeapBuf.push_back(theFastInferiorHeap);
      if(!calledFromBaseTableConst) {
	 // update DYNINSTthreadTable with new fastInferiorHeap address - naim
	 RAW *iBaseAddrInApplic = (RAW *) 
	    iHeapMgr.getSubHeapInApplic(subHeapIndex);
	 assert(iBaseAddrInApplic);

	 for (unsigned j=0;j<inferiorProcess->threads.size();j++) {
	    unsigned idx;
	    pdThread *thr = inferiorProcess->threads[j];
	    assert(thr);
	    idx = thr->get_pos();
	    if (col == thr->get_pd_pos()) {
	       updateThreadTable(iBaseAddrInApplic+adr_offset, idx, level);
	       break;
	    }
	 }
      }
   }
   // Note: permanentSamplingSet and currentSamplingSet are initialized to
   // empty arrays
}

template <class HK, class RAW>
superVector<HK, RAW>::superVector(const superVector<HK, RAW> *parent,
				  process *newProc,
				  unsigned subHeapIndex) :
   inferiorProcess(newProc),
   varStates(parent->varStates) // copy varStates
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
   for (unsigned i=0; i<parent->fastInferiorHeapBuf.size(); i++) {
      RAW *paradynd_attachedAt;
      RAW *appl_attachedAt;
      const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
      
      appl_attachedAt     = (RAW*) iHeapMgr.getSubHeapInApplic(subHeapIndex);
      paradynd_attachedAt = (RAW*) iHeapMgr.getSubHeapInParadynd(subHeapIndex);

      unsigned offset = i*varStates.size();
      fastInferiorHeap<HK,RAW> *newFIH = 
	new fastInferiorHeap<HK,RAW>(parent->fastInferiorHeapBuf[i],
				     newProc, paradynd_attachedAt + offset,
				     appl_attachedAt+offset);

      fastInferiorHeapBuf.push_back(newFIH);
      firstFreeIndex = parent->firstFreeIndex;
   }
   
   // Furthermore: we initially turn each allocated entry in the new heap
   // into type maybeAllocated.  Why do we do this?  Because some of the
   // allocated items of the parent process don't belong in the child process
   // after all...specifically, those with foci narrowed down to a specific
   // process --- the parent process.  In such cases, the new process
   // shouldn't get a copy of mi. On the other hand, what about foci which
   // aren't specific to any process, and thus should be copied to the new
   // heap?  Well in such cases, intializeHKAfterFork() will be called...so
   // at that time, we'll turn the item back to allocated.  A bit ugly,
   // n'est-ce pas?.  --ari

   for (unsigned lcv=0; lcv < varStates.size(); lcv++) {
      if (varStates[lcv] == varAllocated)
	 varStates[lcv] = varMaybeAllocatedByFork;
   }
}

template <class HK, class RAW>
superVector<HK, RAW>::~superVector() {
   // destructors for varStates[] are called automatically
   for (unsigned i=0; i<fastInferiorHeapBuf.size(); i++) {
     delete fastInferiorHeapBuf[i];
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::handleExec() {
   // after the exec syscall...the applic has detached from the segment, but
   // paradynd is still attached to one...and we'll reuse it.
   
   // inferiorProcess doesn't change
   // varStates size doesn't change (but we need to reset its content)

   for(unsigned lcv=0; lcv < varStates.size(); lcv++)
      varStates[lcv] = varFree;

   for(unsigned u=0; u<fastInferiorHeapBuf.size(); u++) {
     fastInferiorHeapBuf[u]->handleExec();
   }

   firstFreeIndex = 0;

   // baseAddrInParadynd doesn't change

   for(unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < fastInferiorHeapBuf.size());
       // baseAddrInApplic: reset to NULL since applic isn't attached any more
       // (it needs to re-run DYNINSTinit)
       assert(fastInferiorHeapBuf[idx] != NULL);
       fastInferiorHeapBuf[idx]->setBaseAddrInApplic(NULL);

       // sampling sets: reset
       fastInferiorHeapBuf[idx]->clearPermanentSamplingSet();
       fastInferiorHeapBuf[idx]->clearCurrentSamplingSet();
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::forkHasCompleted() {
   // call when a fork has completed (i.e. after you've called the fork ctor
   // AND also metricDefinitionNode::handleFork, as forkProcess [context.C]
   // does).  performs some assertion checks, such as mi != NULL for all
   // allocated HKs.

   for(unsigned lcv=0; lcv < varStates.size(); lcv++) {
      if (varStates[lcv] == varMaybeAllocatedByFork) {
         // this guy isn't being carried over, because the focus was specific
	 // to a process -- some other process -- before the fork.  (Can we
	 // check this?)
	 varStates[lcv] = varFree;
      }
   }

   for(unsigned u=0; u<fastInferiorHeapBuf.size(); u++) {
     fastInferiorHeapBuf[u]->forkHasCompleted(varStates);
   }

   for(unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < fastInferiorHeapBuf.size());
       assert(fastInferiorHeapBuf[idx] != NULL);
       // entries that were allocated (and thus in the sampling set) of the
       // parent may not carry over to the child, in which case the sampling
       // set(s) can become invalid.
       fastInferiorHeapBuf[idx]->reconstructPermanentSamplingSet(varStates);
       fastInferiorHeapBuf[idx]->updateCurrentSamplingSet();
   }
}

// returns the allocated instrumentation variable index
template <class HK, class RAW>
inst_var_index superVector<HK, RAW>::allocateForInstVar()
{
   bool updateFreeIndex = true;

   // See the .h file for extensive documentation on this routine...
   // this counter/timer has not been allocated before - naim
   if (firstFreeIndex == UI32_MAX) {
     // heap is full!  Garbage collect and try a second time.
     //cout << "fastInferiorHeap alloc: heap is full; about to garbage collect" << endl;
     for (unsigned lcv=0; lcv < varStates.size(); lcv++)
       assert(varStates[lcv] != varFree);

     vector<Frame> stackWalk;
     inferiorProcess->walkStack(inferiorProcess->getActiveFrame(), stackWalk);
     garbageCollect(stackWalk);

     if (firstFreeIndex == UI32_MAX) {
       // oh no; inferior heap is still full!  Garbage collection has
       // failed.
       cerr << "fastInferiorHeap alloc: heap is full and garbage "
	    << "collection FAILED" << endl;
       assert(false);
       // perhaps throw an exception in the future
     }
   }
   
   assert(firstFreeIndex < varStates.size());
   
   inst_var_index varIndex = firstFreeIndex;
   assert(varStates[varIndex] == varFree);
   varStates[varIndex] = varAllocated;

   if (updateFreeIndex) {
      // update firstFreeIndex to point to next free entry; UI32_MAX if full
      unsigned numberOfIter = 0;
      firstFreeIndex++; 
      while (++numberOfIter < varStates.size()) {
	 if (firstFreeIndex == varStates.size()) 
	    firstFreeIndex = 0; // wrapping around to keep looking for a free 
	 // index
	 if (varStates[firstFreeIndex] == varFree)
	    break;
	 else
	    firstFreeIndex++;
      }

      if (numberOfIter == varStates.size()) {
	 // inferior heap is now full (but the allocation succeeded)
	 firstFreeIndex = UI32_MAX;
	 //cerr << "fastInferiorHeap alloc: alloc succeeded but now full\n";
      }
   }
   
   return varIndex;
}


template <class HK, class RAW>
void superVector<HK, RAW>::createThrInstVar(inst_var_index varIndex,
					    unsigned thrPos, 
					    const RAW &iValue,
					    const HK &iHKValue)
{
   assert(varIndex != UI32_MAX);
   assert(varStates[varIndex] == varAllocated);
   
   // Write "iValue" to the inferior heap, by writing to the shared memory
   // segment.  Should we grab the mutex lock before writing?  Right now we
   // don't, on the assumption that no trampoline in the inferior process is
   // yet writing to this just-allocated memory.  (Mem should be allocated:
   // data first, then initialize tramps, then actually insert tramps)
   assert(thrPos < fastInferiorHeapBuf.size());
   fastInferiorHeap<HK,RAW> *FIH = fastInferiorHeapBuf[thrPos];
   assert(FIH != NULL);
   // baseAddrInApplic: reset to NULL since applic isn't attached any more
   // (it needs to re-run DYNINSTinit)
   
   RAW *destRawPtr = FIH->getBaseAddrInParadynd() + varIndex; 
   // RAW::operator=(const RAW &) if defined, else a bit copy
   *destRawPtr = iValue; 

   //used to be inside
   FIH->set_houseKeeping(varIndex, iHKValue);
}

template <class HK, class RAW>
void superVector<HK, RAW>::markVarAsSampled(inst_var_index varIndex,
					    unsigned threadPos)
					    
{
   fastInferiorHeapBuf[threadPos]->set_activemap(varIndex, varDoSample);

   // sampling set: add to permanent; no need to add to current
   // note:
   // because varIndex is not necessarily larger than all of the
   // current entries in permanentSamplingSet[] (non-increasing allocation
   // indexes can happen all the time, once holes are introduced into the
   // varStates due to deallocation & garbage collection), we reconstruct the
   // set from scratch; it's the only easy way to maintain our invariant that
   // the permanent sampling set is sorted.
   fastInferiorHeapBuf[threadPos]->reconstructPermanentSamplingSet(varStates);
}

template <class HK, class RAW>
void superVector<HK, RAW>::markVarAsNotSampled(inst_var_index varIndex,
					       unsigned threadPos)
					       
{
   fastInferiorHeap<HK,RAW> *fih = fastInferiorHeapBuf[threadPos];
   fih->set_activemap(varIndex, varDontSample);
   fih->reconstructPermanentSamplingSet(varStates);
   fih->removeFromCurrentSamplingSet(varIndex);
}

template <class HK, class RAW>
void superVector<HK, RAW>::makePendingFree(unsigned varIndex,
					   unsigned threadPos,
					   const vector<Address> &trampsUsing)
{
   enum states sm_ndx = varStates[varIndex];

   assert(varIndex < varStates.size());
   assert((sm_ndx == varAllocated) || 
	  (sm_ndx == varPendingfree));
   
   // debugging information. this condition should not happen - naim
   if (threadPos >= fastInferiorHeapBuf.size()) {
      fprintf(stderr,"WARNING: threadPos=%d, superVector.size=%d, index=%d\n",
	      threadPos, fastInferiorHeapBuf.size(), varIndex);
   }
   assert(threadPos < fastInferiorHeapBuf.size());
   assert(fastInferiorHeapBuf[threadPos] != NULL);
   fastInferiorHeap<HK,RAW> *fih = fastInferiorHeapBuf[threadPos];
   fih->makePendingFree(varIndex, trampsUsing);

   bool allNotBeingSampled = true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
      unsigned idx;
      idx = inferiorProcess->threads[i]->get_pd_pos();
      assert(idx < fastInferiorHeapBuf.size());
      assert(fih != NULL);
      if(fih->isMarkedAsSampled(varIndex)) {
	 allNotBeingSampled = false;
	 break;
      }
   }
   
   if (allNotBeingSampled) {
      varStates[varIndex] = varPendingfree;
      //fprintf(stderr, "----- threadPos=%u, varStates[%u]=varPendingfree", 
      //      threadPos, varIndex);
   }
   // firstFreeIndex doesn't change

   // Sampling sets: a pending-free item should no longer be sampled, so
   // remove it from both permanentSamplingSet and currentSamplingSet.
   // Remember that the permanent set needs to stay sorted.  Since it can't
   // be done fast, we just reconstruct from scratch, for simplicity.

   fih->reconstructPermanentSamplingSet(varStates);
   fih->removeFromCurrentSamplingSet(varIndex);
}

template <class HK, class RAW>
void superVector<HK, RAW>::garbageCollect(const vector<Frame> &stackWalk) {
   // tries to set some pending-free items to free.

   // Question: should there be a maximum time limit?  Should there be a time
   // where we are satisfied that we've freed up enough garbage?  How should
   // that be specified?  The current implementation goes through the entire
   // heap, freeing up everything it can.

   //cout << "fastInferiorHeap: welcome to garbageCollect()" << endl;

   unsigned ndx = varStates.size();
   do {
      ndx--;
      
      bool tryGarbageCollectOK = true;
      for (unsigned i=0; i<fastInferiorHeapBuf.size(); i++) {
	 tryGarbageCollectOK = tryGarbageCollectOK && 
	                    fastInferiorHeapBuf[i]->tryGarbageCollect(stackWalk,ndx);
	 if (!tryGarbageCollectOK) break;
      }

      // Try to garbage collect this item.
      if (varStates[ndx] == varPendingfree && tryGarbageCollectOK) {
         varStates[ndx] = varFree;

         // update firstFreeIndex:
         if (ndx < firstFreeIndex)
            firstFreeIndex = ndx;
      }
   } while (ndx);
 
   // We've gone thru every item in the inferior heap so there's no more
   // garbage collection we can do at this time.

   // Note that garbage collection doesn't affect either of the sampling sets.
}

template <class HK, class RAW>
void superVector<HK, RAW>::updateThreadTable(RAW *shmAddr, unsigned pos,
					     unsigned level) 
{
   unsigned addr;
   bool err;

   assert(inferiorProcess);
   if(! inferiorProcess->is_multithreaded()) {
      // don't need this yet
      return;
   }

   // Getting thread table address
   addr = inferiorProcess->findInternalAddress("DYNINSTthreadTable",true,err);
   assert(!err);
   
   // Find the right position for this thread in the threadTable
   addr += pos*sizeof(unsigned);

#if !defined(MT_THREAD)   
   #define MAX_NUMBER_OF_THREADS  1 // temporary
#endif
   // Find the right level...
   addr += level*sizeof(unsigned)*MAX_NUMBER_OF_THREADS;

   unsigned tmp_addr = (unsigned) shmAddr;

   // we make sure that the process is stopped before writing to it - naim
   bool needToCont;
   needToCont = false;
   if (inferiorProcess->status() == running) {
      needToCont = true;
#ifdef DETACH_ON_THE_FLY
      inferiorProcess->reattachAndPause();
#else
      inferiorProcess->pause();
#endif
   }
   // process should be stopped now - naim
   if (inferiorProcess->status() == stopped) {
      // Save pointer to vector of counter/timers in thread table
      inferiorProcess->writeDataSpace((caddr_t) addr, sizeof(unsigned),
				      (caddr_t) &tmp_addr);
      
      if (needToCont) {
#ifdef DETACH_ON_THE_FLY
	 inferiorProcess->detachAndContinue();
#else
	 inferiorProcess->continueProc();
#endif
      }
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::setBaseAddrInApplic(RAW *addr, unsigned level)
{
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
      unsigned idx, maxCol;
      idx = inferiorProcess->threads[i]->get_pd_pos();
      maxCol = inferiorProcess->getTable().getMaxSize();
      assert(idx < fastInferiorHeapBuf.size());
      assert(fastInferiorHeapBuf[idx] != NULL);
      // addr should be different for each fastInferiorHeap in
      // fastInferiorHeapBuf.  we need tu pass addr+offset and not just addr
      unsigned offset;
      if(inferiorProcess->is_multithreaded()) {
	 offset = (level*maxCol + idx) * varStates.size();
      } else {
	 offset = i * varStates.size();
	 assert(offset == 0);  // if this isn't MT, then should only be 1 thr
      }
      fastInferiorHeapBuf[idx]->setBaseAddrInApplic(addr + offset);
      updateThreadTable(addr+offset, inferiorProcess->threads[i]->get_pos(),
			level);
   }
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMajorSample()
{
   //cerr << "  superVector::doMajorSample\n";
   bool allFIH_SamplesSuccessful = true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     pdThread *thr;
     thr = inferiorProcess->threads[i];
     assert(thr);
     idx = thr->get_pd_pos();

     assert(idx < fastInferiorHeapBuf.size());
     assert(fastInferiorHeapBuf[idx] != NULL);
     if(! fastInferiorHeapBuf[idx]->doMajorSample(varStates)) {
	allFIH_SamplesSuccessful = false;
     }
   }
   //cerr << "  superVector::doMajorSample, result = " 
   //     << allFIH_SamplesSuccessful << "\n";
   return allFIH_SamplesSuccessful;
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMinorSample()
{
   //cerr << "  superVector::doMinorSample\n";
   bool allFIH_SamplesSuccessful = true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
      unsigned idx;
      pdThread *thr;
      thr = inferiorProcess->threads[i];
      assert(thr);
      idx = thr->get_pd_pos();
      assert(idx < fastInferiorHeapBuf.size());
      assert(fastInferiorHeapBuf[idx] != NULL); 
      if(! fastInferiorHeapBuf[idx]->doMinorSample(varStates)) {
	 allFIH_SamplesSuccessful = false;
      }
   }
   //cerr << "  superVector::doMajorSample, result = " 
   //     << allFIH_SamplesSuccessful << "\n";
   return allFIH_SamplesSuccessful;
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2LocalAddr(inst_var_index varIndex,
					   unsigned threadPos) const
{
  return(fastInferiorHeapBuf[threadPos]->index2LocalAddr(varIndex));
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2InferiorAddr(inst_var_index varIndex,
					      unsigned threadPos) const
{
  assert(fastInferiorHeapBuf[threadPos] != NULL);
  return(fastInferiorHeapBuf[threadPos]->index2InferiorAddr(varIndex));
}

template <class HK, class RAW>
HK *superVector<HK, RAW>::getHouseKeeping(inst_var_index varIndex,
					  unsigned threadPos) const
					  
{
  assert(fastInferiorHeapBuf[threadPos] != NULL);
  return(fastInferiorHeapBuf[threadPos]->getHouseKeeping(varIndex));
}

template <class HK, class RAW>
void superVector<HK, RAW>::initializeHKAfterFork(inst_var_index varIndex, 
						 const HK &iHouseKeepingValue)
{
   // should be called only for a maybe-allocated-by-fork value
   assert(varStates[varIndex] == varMaybeAllocatedByFork);
   if (varStates[varIndex] == varMaybeAllocatedByFork)
     varStates[varIndex] = varAllocated;

   // write HK:
   for (unsigned i=0;i<fastInferiorHeapBuf.size();i++) {
     fastInferiorHeapBuf[i]->initializeHKAfterFork(varIndex,
					      iHouseKeepingValue);
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::addColumns(unsigned from, unsigned to,
				      unsigned subHeapIndex, 
				      unsigned level)
{
   assert(inferiorProcess->is_multithreaded());
   RAW *iBaseAddrInParadynd;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   unsigned adr_offset, col, maxCol;
   maxCol = inferiorProcess->getTable().getMaxSize();
   iBaseAddrInParadynd = (RAW *) iHeapMgr.getSubHeapInParadynd(subHeapIndex);
   assert(iBaseAddrInParadynd);
   for (unsigned i=from; i<to; i++) {
      col = i;
      adr_offset = (level*maxCol + col)*varStates.size();
      fastInferiorHeap<HK, RAW> *theFastInferiorHeap = 
	 new fastInferiorHeap<HK, RAW>(iBaseAddrInParadynd + adr_offset, 
				       inferiorProcess, varStates.size());
      assert(theFastInferiorHeap != NULL);
      
      // update DYNINSTthreadTable with new fastInferiorHeap address - naim
      RAW *iBaseAddrInApplic = (RAW *) iHeapMgr.getSubHeapInApplic(subHeapIndex);
      assert(iBaseAddrInApplic);
      for (unsigned j=0;j<inferiorProcess->threads.size();j++) {
	 unsigned idx;
	 pdThread *thr = inferiorProcess->threads[j];
	 assert(thr);
	 idx = thr->get_pos();
	 if (col == thr->get_pd_pos()) {
	    updateThreadTable(iBaseAddrInApplic+adr_offset, idx, level);
	    break;
	 }
      }

      fastInferiorHeapBuf.push_back(theFastInferiorHeap);
   }
   
   // Note: do we need to "cleanup" columns first? - naim 10/10/97
}

template <class HK, class RAW>
void superVector<HK, RAW>::addThread(unsigned pos, unsigned threadPos, 
				     unsigned subHeapIndex, 
				     unsigned level)
{
   unsigned adr_offset, maxCol;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   maxCol = inferiorProcess->getTable().getMaxSize();
   adr_offset = (level*maxCol + threadPos)*varStates.size();

   // update DYNINSTthreadTable with new fastInferiorHeap address - naim
   RAW *iBaseAddrInApplic = (RAW *) iHeapMgr.getSubHeapInApplic(subHeapIndex);
   assert(iBaseAddrInApplic);
   assert(fastInferiorHeapBuf[threadPos]!=NULL);
   fastInferiorHeapBuf[threadPos]->setBaseAddrInApplic(iBaseAddrInApplic +
						    adr_offset);
   updateThreadTable(iBaseAddrInApplic+adr_offset,pos,level);

   // reconstruct permanent sampling set. The new thread inherits all the
   // instrumentation valid for this fastInferiorHeap - naim
   fastInferiorHeapBuf[threadPos]->reconstructPermanentSamplingSet(varStates);
}

template <class HK, class RAW>
void superVector<HK, RAW>::deleteThread(unsigned pos, unsigned threadPos, 
					unsigned level)
{
   assert(fastInferiorHeapBuf[threadPos]!=NULL);
   fastInferiorHeapBuf[threadPos]->setBaseAddrInApplic(NULL);
   updateThreadTable(NULL,pos,level);
   fastInferiorHeapBuf[threadPos]->initialize_activemap(varStates.size());
   fastInferiorHeapBuf[threadPos]->initialize_houseKeeping(varStates.size());
}

