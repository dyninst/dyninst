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

// $Id: superVector.C,v 1.12 2002/02/21 21:48:32 bernat Exp $

#include <sys/types.h>
#include <limits.h>
#include "common/h/headers.h"
#include "paradynd/src/superVector.h"
#include "paradynd/src/shmSegment.h"
#include "paradynd/src/fastInferiorHeapMgr.h"
#include "paradynd/src/fastInferiorHeap.h"
#include "rtinst/h/rtinst.h" // for time64
#include "dyninstAPI/src/pdThread.h"

#if !defined(MT_THREAD)
template <class HK, class RAW>
superVector<HK, RAW>::
superVector(process *iInferiorProcess,
	    unsigned heapNumElems,
	    unsigned subHeapIndex,
	    unsigned numberOfColumns):
	    inferiorProcess(iInferiorProcess),
            statemap(heapNumElems),
	    houseKeeping(heapNumElems)
{
   assert(heapNumElems > 0);

   // Initialize statemap by setting all entries to state 'free'
   for (unsigned ndx=0; ndx < statemap.size(); ndx++)
     statemap[ndx] = FIHfree;

   firstFreeIndex = 0;

   RAW *iBaseAddrInParadynd;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   for (unsigned i=0; i<numberOfColumns; i++) {
       iBaseAddrInParadynd = (RAW *) iHeapMgr.getSubHeapInParadynd(subHeapIndex);

       fastInferiorHeap<HK, RAW> *theFastInferiorHeap = new fastInferiorHeap<HK, RAW>(iBaseAddrInParadynd+i*heapNumElems, iInferiorProcess);
       assert(theFastInferiorHeap != NULL);

       theSuperVector += theFastInferiorHeap;
   }

   // Note: houseKeeping[] values are uninitialized; each call to alloc() initializes
   //       one HK (here) and one RAW (in the inferior heap).

   // Note: permanentSamplingSet and currentSamplingSet are initialized to empty arrays
}

template <class HK, class RAW>
superVector<HK, RAW>::superVector(const superVector<HK, RAW> *parent,
				  process *newProc,
				  unsigned subHeapIndex) :
                             inferiorProcess(newProc),
			     statemap(parent->statemap), // copy statemap
			     houseKeeping(parent->houseKeeping.size()) // just size it
{
	unsigned lcv;


   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.

   for (unsigned i=0; i<parent->theSuperVector.size(); i++) {
       RAW *paradynd_attachedAt;
       RAW *appl_attachedAt;
       const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();

       appl_attachedAt     = (RAW*) iHeapMgr.getSubHeapInApplic(subHeapIndex);
       paradynd_attachedAt = (RAW*) iHeapMgr.getSubHeapInParadynd(subHeapIndex);

       unsigned offset = i*statemap.size();
       theSuperVector += new fastInferiorHeap<HK,RAW>(newProc,paradynd_attachedAt+offset,appl_attachedAt+offset);

       firstFreeIndex = parent->firstFreeIndex;

       // Now for the houseKeeping, which is quick tricky on a fork.
       // We (intentionally) leave every entry undefined for now.
       // (using a copy-ctor would be a very bad idea, for then we would end up sharing
       // mi's with the parent process, which is not at all the correct thing to do.
       // outside code (forkProcess(); context.C) should soon fill in what we left
       // undefined; for example, the call to metricDefinitionNode::handleFork().)
       // More specifically: on a fork, dataReqNode->dup() is called for everything in
       // the parent.  This virtual fn will end up calling the correct fork-ctor for
       // objects like "samedShmIntCounterReqNode", which will in turn think up a new
       // HK value for a single allocated entry, and call initializeHKAfterFork() to
       // fill it in. If, for whatevery reason, some allocated datareqnode doesn't have
       // its HK filled in by initializeAfterFork(), then the HK is left undefined
       // (in particular, its mi will be NULL), so we should get an assert error rather
       // soon...the next shm sample, in fact.

       // Furthermore: we initially turn each allocated entry in the new heap into type
       // maybeAllocated.  Why do we do this?
       // Because some of the allocated items of the parent process don't belong in the
       // child process after all...specifically, those with foci narrowed down to a
       // specific process --- the parent process.  In such cases, the new process
       // shouldn't get a copy of mi. On the other hand, what about foci which aren't
       // specific to any process, and thus should be copied to the new heap?  Well in
       // such cases, intializeHKAfterFork() will be called...so at that time, we'll
       // turn the item back to allocated.  A bit ugly, n'est-ce pas?.  --ari
   }

   for (lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == FIHallocated)
	 statemap[lcv] = FIHmaybeAllocatedByFork;
      if (statemap[lcv] == FIHallocatedButDoNotSample)
	 statemap[lcv] = FIHmaybeAllocatedByForkButDoNotSample;
   }

   for (lcv=0; lcv < houseKeeping.size(); lcv++) {
      // If we wanted, we could do this only for allocated items...but doing it for
      // every item should be safe (if unnecessary).
      const HK undefinedHK; // default ctor should leave things undefined
      houseKeeping[lcv] = undefinedHK;
   }

   // permanent and curr sampling sets init'd to null on purpose, since they
   // can differ from those of the parent.  forkHasCompleted() initializes them.
}

template <class HK, class RAW>
superVector<HK, RAW>::~superVector() {
   // destructors for statemap[] and houseKeeping[] are called automatically
   for (unsigned i=0; i<theSuperVector.size(); i++) {
     delete theSuperVector[i];
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::handleExec() {
   // after the exec syscall...the applic has detached from the segment, but paradynd
   // is still attached to one...and we'll reuse it.
   
   // inferiorProcess doesn't change
   // statemap and houseKeeping sizes don't change (but we need to reset their contents)
   assert(statemap.size() == houseKeeping.size());

   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      statemap[lcv] = FIHfree;

   // here's a quick-and-dirty way to reinitialize the houseKeeping vector (I think):
   houseKeeping.resize(0);
   houseKeeping.resize(statemap.size());

   firstFreeIndex = 0;

   // baseAddrInParadynd doesn't change

   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       // baseAddrInApplic: reset to NULL since applic isn't attached any more
       // (it needs to re-run DYNINSTinit)
       assert(theSuperVector[idx] != NULL);
       theSuperVector[idx]->setBaseAddrInApplic(NULL);

       // sampling sets: reset
       theSuperVector[idx]->clearPermanentSamplingSet();
       theSuperVector[idx]->clearCurrentSamplingSet();
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::forkHasCompleted() {
   // call when a fork has completed (i.e. after you've called the fork ctor AND
   // also metricDefinitionNode::handleFork, as forkProcess [context.C] does).
   // performs some assertion checks, such as mi != NULL for all allocated HKs.

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == FIHmaybeAllocatedByFork ||
	  statemap[lcv] == FIHmaybeAllocatedByForkButDoNotSample)
         // this guy isn't being carried over, because the focus was specific to a
	 // process -- some other process -- before the fork.  (Can we check this?)
	 statemap[lcv] = FIHfree;

      if (statemap[lcv] == FIHallocated || 
	  statemap[lcv] == FIHallocatedButDoNotSample) {
	 const HK &theHK = houseKeeping[lcv];
	 theHK.assertWellDefined();
      }
   }


   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);
       // entries that were allocated (and thus in the sampling set) of the parent
       // may not carry over to the child, in which case the sampling set(s) can become
       // invalid.
       theSuperVector[idx]->reconstructPermanentSamplingSet(statemap);
       theSuperVector[idx]->updateCurrentSamplingSet();
   }
}

template <class HK, class RAW>
bool superVector<HK, RAW>::alloc(const RAW &iValue,
				 const HK &iHKValue,
				 unsigned &allocatedIndex,
				 bool doNotSample) {
   // See the .h file for extensive documentation on this routine...
   unsigned i;

   if (firstFreeIndex == UINT_MAX) {
      // heap is full!  Garbage collect and try a second time.
      cout << "fastInferiorHeap alloc: heap is full; about to garbage collect" << endl;

      for (unsigned lcv=0; lcv < statemap.size(); lcv++)
         assert(statemap[lcv] != FIHfree);

      const vector<Address> PCs = inferiorProcess->walkStack(); // prob expensive
      garbageCollect(PCs);
      if (firstFreeIndex == UINT_MAX) {
         // oh no; inferior heap is still full!  Garbage collection has failed.
	 cout << "fastInferiorHeap alloc: heap is full and garbage collection FAILED" << endl;
         return false; // failure
      }
   }

   assert(firstFreeIndex < statemap.size());

   allocatedIndex = firstFreeIndex; // allocatedIndex is a ref param
   assert(statemap[allocatedIndex] == FIHfree);
   if (doNotSample) 
     statemap[allocatedIndex] = FIHallocatedButDoNotSample;
   else
     statemap[allocatedIndex] = FIHallocated;

   houseKeeping[allocatedIndex] = iHKValue; // HK::operator=()

   // Write "iValue" to the inferior heap, by writing to the shared memory segment.
   // Should we grab the mutex lock before writing?  Right now we don't, on the
   // assumption that no trampoline in the inferior process is yet writing to
   // this just-allocated memory.  (Mem should be allocated: data first, then initialize
   // tramps, then actually insert tramps)

   for (i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);
       // baseAddrInApplic: reset to NULL since applic isn't attached any more
       // (it needs to re-run DYNINSTinit)
       RAW *destRawPtr = theSuperVector[idx]->getBaseAddrInParadynd() + allocatedIndex; // ptr arith
       *destRawPtr = iValue; // RAW::operator=(const RAW &) if defined, else a bit copy
   }

   // update firstFreeIndex to point to next free entry; UINT_MAX if full
   unsigned numberOfIter = 0;
   firstFreeIndex++; 
   while (++numberOfIter < statemap.size()) {
      if (firstFreeIndex == statemap.size()) 
         firstFreeIndex = 0; // wrapping around to keep looking for a free 
                             // index
      if (statemap[firstFreeIndex] == FIHfree)
         break;
      else
	 firstFreeIndex++;
   }

   if (numberOfIter == statemap.size()) {
      // inferior heap is now full (but the allocation succeeded)
      firstFreeIndex = UINT_MAX;
      cout << "fastInferiorHeap alloc: alloc succeeded but now full" << endl;
   }

   // sampling set: add to permanent; no need to add to current
   // note: because allocatedIndex is not necessarily larger than all of the current
   //       entries in permanentSamplingSet[] (non-increasing allocation indexes can
   //       happen all the time, once holes are introduced into the statemap due to
   //       deallocation & garbage collection), we reconstruct the set from scratch;
   //       it's the only easy way to maintain our invariant that the permanent
   //       sampling set is sorted.
   for (i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);
       const unsigned oldPermanentSamplingSetSize = theSuperVector[idx]->getPermanentSamplingSetSize();
       theSuperVector[idx]->reconstructPermanentSamplingSet(statemap);
       if (doNotSample) {
         assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize);
       } else {
         assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize + 1);
       }
   }

   return true;
}

template <class HK, class RAW>
void superVector<HK, RAW>::makePendingFree(unsigned ndx,
					   const vector<Address> &trampsUsing)
{
   bool oldEqualsPermanent = false;
   if (statemap[ndx] == FIHallocatedButDoNotSample) oldEqualsPermanent=true;

   assert(ndx < statemap.size());
   assert((statemap[ndx] == FIHallocated) || (statemap[ndx] == FIHallocatedButDoNotSample));
   statemap[ndx] = FIHpendingfree;

   houseKeeping[ndx].makePendingFree(trampsUsing);

   // firstFreeIndex doesn't change

   // Sampling sets: a pending-free item should no longer be sampled, so remove
   // it from both permanentSamplingSet and currentSamplingSet.
   // Remember that the permanent set needs to stay sorted.  Since it can't be done
   // fast, we just reconstruct from scratch, for simplicity.

   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);

       const unsigned oldPermanentSamplingSetSize = theSuperVector[idx]->getPermanentSamplingSetSize();

       if (oldEqualsPermanent)
         ; //assert(oldPermanentSamplingSetSize >= 0);
       else
         assert(oldPermanentSamplingSetSize > 0);

       theSuperVector[idx]->clearPermanentSamplingSet();
       for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
          if (statemap[lcv] == FIHallocated)
             theSuperVector[idx]->addToPermanentSamplingSet(lcv);

       }

       if (oldEqualsPermanent)
         assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize);
       else
         assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize - 1);

       // What about the current sampling set?  We can conservatively set it to contain
       // more entries than need be, which we do.  (We could also set it to the empty
       // set at worst, one bucket of sampling data is lost, which is no big deal).
       theSuperVector[idx]->updateCurrentSamplingSet();
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::garbageCollect(const vector<Address> &PCs) {
   // tries to set some pending-free items to free.

   // PCs represents a stack trace (list of PC-register values) in the inferior process,
   // presumably obtained by calling process::walkStack() (which needs to use /proc to
   // obtain lots of information, hence it pauses then unpauses the process, which is
   // quite slow...~70ms)

   // Question: should there be a maximum time limit?
   // Should there be a time where we are satisfied that we've freed up enough garbage? 
   // How should that be specified?
   // The current implementation goes through the entire heap, freeing up everything it
   // can.

   cout << "fastInferiorHeap: welcome to garbageCollect()" << endl;

   unsigned ndx = statemap.size();
   do {
      ndx--;

      // Try to garbage collect this item.
      if (statemap[ndx] == FIHpendingfree && houseKeeping[ndx].tryGarbageCollect(PCs)) {
         statemap[ndx] = FIHfree;

         // update firstFreeIndex:
         if (ndx < firstFreeIndex)
            firstFreeIndex = ndx;
      }
   } while (ndx);
 
   // We've gone thru every item in the inferior heap so there's no more garbage
   // collection we can do at this time.

   // Note that garbage collection doesn't affect either of the sampling sets.
}

#if defined(MT_THREAD)
template <class HK, class RAW>
void superVector<HK, RAW>::updateThreadTable(RAW *shmAddr, unsigned pos,
					     unsigned level) 
{
    unsigned addr;
    bool err;

    assert(inferiorProcess);

    // Getting thread table address
    addr = inferiorProcess->findInternalAddress("DYNINSTthreadTable",true, err);
    assert(!err);

    // Find the right position for this thread in the threadTable
    addr += pos*sizeof(unsigned);

    // Find the right level...
    addr += level*sizeof(unsigned)*MAX_NUMBER_OF_THREADS;

    unsigned tmp_addr = (unsigned) shmAddr;

    // Save pointer to vector of counter/timers in thread table
    inferiorProcess->writeDataSpace((caddr_t) addr, sizeof(unsigned),
				    (caddr_t) &tmp_addr);
}
#endif

template <class HK, class RAW>
void superVector<HK, RAW>::setBaseAddrInApplic(RAW *addr, 
#if defined(MT_THREAD)
        unsigned level)
#else
        unsigned /*level*/)
#endif
{
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     idx = inferiorProcess->threads[i]->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL);
     // addr should be different for each fastInferiorHeap in theSuperVector.
     // we need tu pass addr+offset and not just addr - naim 3/21/97
     unsigned offset = idx*statemap.size();
     theSuperVector[idx]->setBaseAddrInApplic(addr+offset);
#if defined(MT_THREAD)
     updateThreadTable(addr+offset,inferiorProcess->threads[i]->get_pos(),level);
#endif
   }
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMajorSample()
{
   bool ok=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     idx = inferiorProcess->threads[i]->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL);
     ok = ok && theSuperVector[idx]->doMajorSample(statemap,houseKeeping);
   }
   return(ok);
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMinorSample()
{
   bool ok=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     idx = inferiorProcess->threads[i]->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL); 
     ok = ok && theSuperVector[idx]->doMinorSample(statemap,houseKeeping);
   }
   return(ok);
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2LocalAddr(unsigned position, unsigned allocatedIndex) const
{
  return(theSuperVector[position]->index2LocalAddr(allocatedIndex));
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2InferiorAddr(unsigned position, 
					      unsigned allocatedIndex) const
{
  assert(theSuperVector[position] != NULL);
  return(theSuperVector[position]->index2InferiorAddr(allocatedIndex));
}

template <class HK, class RAW>
void superVector<HK, RAW>::initializeHKAfterFork(unsigned allocatedIndex, 
						 const HK &iHouseKeepingValue)
{
   // should be called only for a maybe-allocated-by-fork value
   assert(statemap[allocatedIndex] == FIHmaybeAllocatedByFork ||
	  statemap[allocatedIndex] == FIHmaybeAllocatedByForkButDoNotSample);
   if (statemap[allocatedIndex] == FIHmaybeAllocatedByFork)
     statemap[allocatedIndex] = FIHallocated;
   if (statemap[allocatedIndex] == FIHmaybeAllocatedByForkButDoNotSample)
     statemap[allocatedIndex] = FIHallocatedButDoNotSample;

   // write HK:
   houseKeeping[allocatedIndex] = iHouseKeepingValue; // HK::operator=()
}
#else
template <class HK, class RAW>
superVector<HK, RAW>::
superVector(process *iInferiorProcess,
	    unsigned heapNumElems,
	    unsigned level,
	    unsigned subHeapIndex,
	    unsigned numberOfColumns,
	    bool calledFromBaseTableConst):
	    inferiorProcess(iInferiorProcess),
            statemap(heapNumElems)
{
   assert(heapNumElems > 0);

   // Initialize statemap by setting all entries to state 'free'
   for (unsigned ndx=0; ndx < statemap.size(); ndx++)
     statemap[ndx] = FIHfree;

   firstFreeIndex = 0;

   RAW *iBaseAddrInParadynd;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   unsigned adr_offset, col, maxCol;
   maxCol = inferiorProcess->getTable().getMaxSize();
   iBaseAddrInParadynd = (RAW *) iHeapMgr.getSubHeapInParadynd(subHeapIndex);
   assert(iBaseAddrInParadynd);
#if defined(TEST_DEL_DEBUG)
   sprintf(errorLine,"---> superVector, numberOfColumns=%d, subHeapIndex=%d\n",numberOfColumns,subHeapIndex);
   logLine(errorLine);
#endif
   for (unsigned i=0; i<numberOfColumns; i++) {
     col = i;
     adr_offset = (level*maxCol + col)*heapNumElems;
     fastInferiorHeap<HK, RAW> *theFastInferiorHeap = new fastInferiorHeap<HK, RAW>(iBaseAddrInParadynd+adr_offset, iInferiorProcess, statemap.size());
     assert(theFastInferiorHeap != NULL);

#if defined(MT_THREAD)
     if (!calledFromBaseTableConst) {
       // update DYNINSTthreadTable with new fastInferiorHeap address - naim
       RAW *iBaseAddrInApplic = (RAW *) iHeapMgr.getSubHeapInApplic(subHeapIndex);
       assert(iBaseAddrInApplic);
#if defined(TEST_DEL_DEBUG)
       logLine("---> calledFromBaseTableConst is FALSE\n");
#endif
       for (unsigned j=0;j<inferiorProcess->threads.size();j++) {
	 unsigned idx;
	 pdThread *thr = inferiorProcess->threads[j];
	 assert(thr);
	 idx = thr->get_pos();
	 if (col == thr->get_pd_pos()) {
	   updateThreadTable(iBaseAddrInApplic+adr_offset,idx,level);
	   break;
	 }
       }
     }
#endif

     theSuperVector += theFastInferiorHeap;
   }

   // Note: permanentSamplingSet and currentSamplingSet are initialized to empty arrays
}

template <class HK, class RAW>
superVector<HK, RAW>::superVector(const superVector<HK, RAW> *parent,
				  process *newProc,
				  unsigned subHeapIndex) :
                             inferiorProcess(newProc),
			     statemap(parent->statemap) // copy statemap
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
   for (unsigned i=0; i<parent->theSuperVector.size(); i++) {
       RAW *paradynd_attachedAt;
       RAW *appl_attachedAt;
       const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();

       appl_attachedAt     = (RAW*) iHeapMgr.getSubHeapInApplic(subHeapIndex);
       paradynd_attachedAt = (RAW*) iHeapMgr.getSubHeapInParadynd(subHeapIndex);

       unsigned offset = i*statemap.size();
       theSuperVector += new fastInferiorHeap<HK,RAW>(parent->theSuperVector[i],
	 newProc, 
	 paradynd_attachedAt+offset,
	 appl_attachedAt+offset);

       firstFreeIndex = parent->firstFreeIndex;
   }

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == FIHallocated)
	 statemap[lcv] = FIHmaybeAllocatedByFork;
      if (statemap[lcv] == FIHallocatedButDoNotSample)
	 statemap[lcv] = FIHmaybeAllocatedByForkButDoNotSample;
   }
}

template <class HK, class RAW>
superVector<HK, RAW>::~superVector() {
   // destructors for statemap[] are called automatically
   for (unsigned i=0; i<theSuperVector.size(); i++) {
     delete theSuperVector[i];
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::handleExec() {
   // after the exec syscall...the applic has detached from the segment, but paradynd
   // is still attached to one...and we'll reuse it.
   
   // inferiorProcess doesn't change
   // statemap size doesn't change (but we need to reset its content)

   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      statemap[lcv] = FIHfree;

   for (unsigned i=0;i<theSuperVector.size();i++) {
     theSuperVector[i]->handleExec();
   }

   firstFreeIndex = 0;

   // baseAddrInParadynd doesn't change

   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       // baseAddrInApplic: reset to NULL since applic isn't attached any more
       // (it needs to re-run DYNINSTinit)
       assert(theSuperVector[idx] != NULL);
       theSuperVector[idx]->setBaseAddrInApplic(NULL);

       // sampling sets: reset
       theSuperVector[idx]->clearPermanentSamplingSet();
       theSuperVector[idx]->clearCurrentSamplingSet();
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::forkHasCompleted() {
   // call when a fork has completed (i.e. after you've called the fork ctor AND
   // also metricDefinitionNode::handleFork, as forkProcess [context.C] does).
   // performs some assertion checks, such as mi != NULL for all allocated HKs.

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == FIHmaybeAllocatedByFork ||
	  statemap[lcv] == FIHmaybeAllocatedByForkButDoNotSample)
         // this guy isn't being carried over, because the focus was specific to a
	 // process -- some other process -- before the fork.  (Can we check this?)
	 statemap[lcv] = FIHfree;
   }

   for (unsigned i=0;i<theSuperVector.size();i++) {
     theSuperVector[i]->forkHasCompleted(statemap);
   }

   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);
       // entries that were allocated (and thus in the sampling set) of the parent
       // may not carry over to the child, in which case the sampling set(s) can become
       // invalid.
       theSuperVector[idx]->reconstructPermanentSamplingSet(statemap);
       theSuperVector[idx]->updateCurrentSamplingSet();
   }
}

template <class HK, class RAW>
bool superVector<HK, RAW>::alloc(unsigned thr_pos, const RAW &iValue,
				 const HK &iHKValue,
				 unsigned &allocatedIndex,
				 bool doNotSample) {
   bool updateFreeIndex = true;
   // See the .h file for extensive documentation on this routine...
   if (allocatedIndex == UINT_MAX) {
     // this counter/timer has not been allocated before - naim
     if (firstFreeIndex == UINT_MAX) {
       // heap is full!  Garbage collect and try a second time.
       //cout << "fastInferiorHeap alloc: heap is full; about to garbage collect" << endl;
       
       for (unsigned lcv=0; lcv < statemap.size(); lcv++)
         assert(statemap[lcv] != FIHfree);

      const vector<Address> PCs = inferiorProcess->walkStack(); // prob expensive
      garbageCollect(PCs);
      if (firstFreeIndex == UINT_MAX) {
         // oh no; inferior heap is still full!  Garbage collection has failed.
	 cerr << "fastInferiorHeap alloc: heap is full and garbage collection FAILED" << endl;
         return false; // failure
       }
     }

     assert(firstFreeIndex < statemap.size());

     allocatedIndex = firstFreeIndex; // allocatedIndex is a ref param
     assert(statemap[allocatedIndex] == FIHfree);
   } else {
     updateFreeIndex = false;
   }

   if (doNotSample) { 
     statemap[allocatedIndex] = FIHallocatedButDoNotSample;
   } else {
     statemap[allocatedIndex] = FIHallocated;
   }

   // Write "iValue" to the inferior heap, by writing to the shared memory segment.
   // Should we grab the mutex lock before writing?  Right now we don't, on the
   // assumption that no trampoline in the inferior process is yet writing to
   // this just-allocated memory.  (Mem should be allocated: data first, then initialize
   // tramps, then actually insert tramps)

   unsigned idx;
   idx = thr_pos;
   assert(idx < theSuperVector.size());
   assert(theSuperVector[idx] != NULL);
   // baseAddrInApplic: reset to NULL since applic isn't attached any more
   // (it needs to re-run DYNINSTinit)

   RAW *destRawPtr = theSuperVector[idx]->getBaseAddrInParadynd() + allocatedIndex; 
   *destRawPtr = iValue; // RAW::operator=(const RAW &) if defined, else a bit copy

   if (doNotSample==false) {
#if defined(TEST_DEL_DEBUG)
     sprintf(errorLine,"=====> setting activemap[%d]=ACTIVE, theSuperVector[%d]",allocatedIndex,idx);
     cerr << errorLine << endl;
#endif
     theSuperVector[idx]->set_activemap(allocatedIndex, FIHactive);
   }
#if defined(TEST_DEL_DEBUG)
   else logLine("=====> doNotSample is TRUE\n");
#endif

  //used to be inside
  theSuperVector[idx]->set_houseKeeping(allocatedIndex, iHKValue);

  if (updateFreeIndex) {
     // update firstFreeIndex to point to next free entry; UINT_MAX if full
     unsigned numberOfIter = 0;
     firstFreeIndex++; 
     while (++numberOfIter < statemap.size()) {
       if (firstFreeIndex == statemap.size()) 
         firstFreeIndex = 0; // wrapping around to keep looking for a free 
       // index
       if (statemap[firstFreeIndex] == FIHfree)
         break;
       else
	 firstFreeIndex++;
     }

     if (numberOfIter == statemap.size()) {
       // inferior heap is now full (but the allocation succeeded)
       firstFreeIndex = UINT_MAX;
       //cout << "fastInferiorHeap alloc: alloc succeeded but now full" << endl;
     }
  }

  // sampling set: add to permanent; no need to add to current
  // note: because allocatedIndex is not necessarily larger than all of the current
  //       entries in permanentSamplingSet[] (non-increasing allocation indexes can
  //       happen all the time, once holes are introduced into the statemap due to
  //       deallocation & garbage collection), we reconstruct the set from scratch;
  //       it's the only easy way to maintain our invariant that the permanent
  //       sampling set is sorted.
  const unsigned oldPermanentSamplingSetSize = theSuperVector[idx]->getPermanentSamplingSetSize();
  theSuperVector[idx]->reconstructPermanentSamplingSet(statemap);
  if (updateFreeIndex) {
     if (doNotSample) {
       assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize);
     } else {
       // debugging information. this condition should not happen! - naim
       if (theSuperVector[idx]->getPermanentSamplingSetSize() != oldPermanentSamplingSetSize + 1) {
         sprintf(errorLine,"=====> newSamplingSetSize=%d, oldSamplingSetSize=%d, updateFreeIndex=%d\n",
	    theSuperVector[idx]->getPermanentSamplingSetSize(),
	    oldPermanentSamplingSetSize, 
	    updateFreeIndex);
         logLine(errorLine);
       }
       assert(theSuperVector[idx]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize + 1);
     }
  }

  return true;
}

template <class HK, class RAW>
void superVector<HK, RAW>::makePendingFree(unsigned pd_pos,
					   unsigned ndx,
					   const vector<Address> &trampsUsing)
{
   bool oldEqualsPermanent = false;
   enum states sm_ndx = statemap[ndx];
   if (sm_ndx == FIHallocatedButDoNotSample || sm_ndx == FIHpendingfree) 
       oldEqualsPermanent=true;

   assert(ndx < statemap.size());
   assert((sm_ndx == FIHallocated) || 
	  (sm_ndx == FIHallocatedButDoNotSample) || 
	  (sm_ndx == FIHpendingfree));

   // debugging information. this condition should not happen - naim
   if (pd_pos >= theSuperVector.size()) {
     sprintf(errorLine,"WARNING: pd_pos=%d, superVector.size=%d, index=%d\n",pd_pos,theSuperVector.size(),ndx);
     logLine(errorLine);
   }
   assert(pd_pos < theSuperVector.size());
   assert(theSuperVector[pd_pos] != NULL);
   theSuperVector[pd_pos]->makePendingFree(ndx,trampsUsing);

   bool updatemap=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
       unsigned idx;
       idx = inferiorProcess->threads[i]->get_pd_pos();
       assert(idx < theSuperVector.size());
       assert(theSuperVector[idx] != NULL);
       updatemap = updatemap && theSuperVector[idx]->checkIfInactive(ndx);
       if (!updatemap) break;
   }
   if (updatemap) {
     statemap[ndx] = FIHpendingfree;
     sprintf(errorLine, "----- pd_pos=%u, statemap[%u]=FIHpendingfree", pd_pos, ndx);
     cerr << errorLine << endl ;
   }
   // firstFreeIndex doesn't change

   // Sampling sets: a pending-free item should no longer be sampled, so remove
   // it from both permanentSamplingSet and currentSamplingSet.
   // Remember that the permanent set needs to stay sorted.  Since it can't be done
   // fast, we just reconstruct from scratch, for simplicity.

   const unsigned oldPermanentSamplingSetSize = theSuperVector[pd_pos]->getPermanentSamplingSetSize();

   if (!oldEqualsPermanent)
     assert(oldPermanentSamplingSetSize > 0);

   theSuperVector[pd_pos]->clearPermanentSamplingSet();
   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
     if (statemap[lcv] == FIHallocated) {
       theSuperVector[pd_pos]->addToPermanentSamplingSet(lcv);
     }
   }
     
   // What about the current sampling set?  We can conservatively set it to contain
   // more entries than need be, which we do.  (We could also set it to the empty
   // set at worst, one bucket of sampling data is lost, which is no big deal).
   theSuperVector[pd_pos]->updateCurrentSamplingSet();
     
   if (oldEqualsPermanent)
     assert(theSuperVector[pd_pos]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize);
   else if(updatemap) // XXX
     assert(theSuperVector[pd_pos]->getPermanentSamplingSetSize() == oldPermanentSamplingSetSize - 1);
}

template <class HK, class RAW>
void superVector<HK, RAW>::garbageCollect(const vector<Address> &PCs) {
   // tries to set some pending-free items to free.

   // PCs represents a stack trace (list of PC-register values) in the inferior process,
   // presumably obtained by calling process::walkStack() (which needs to use /proc to
   // obtain lots of information, hence it pauses then unpauses the process, which is
   // quite slow...~70ms)

   // Question: should there be a maximum time limit?
   // Should there be a time where we are satisfied that we've freed up enough garbage? 
   // How should that be specified?
   // The current implementation goes through the entire heap, freeing up everything it
   // can.

   //cout << "fastInferiorHeap: welcome to garbageCollect()" << endl;

   unsigned ndx = statemap.size();
   do {
      ndx--;

      bool tryGarbageCollectOK = true;
      for (unsigned i=0;i<theSuperVector.size();i++) {
	tryGarbageCollectOK = tryGarbageCollectOK && 
	                      theSuperVector[i]->tryGarbageCollect(PCs,ndx);
	if (!tryGarbageCollectOK) break;
      }

      // Try to garbage collect this item.
      if (statemap[ndx] == FIHpendingfree && tryGarbageCollectOK) {
         statemap[ndx] = FIHfree;

         // update firstFreeIndex:
         if (ndx < firstFreeIndex)
            firstFreeIndex = ndx;
      }
   } while (ndx);
 
   // We've gone thru every item in the inferior heap so there's no more garbage
   // collection we can do at this time.

   // Note that garbage collection doesn't affect either of the sampling sets.
}

#if defined(MT_THREAD)
template <class HK, class RAW>
void superVector<HK, RAW>::updateThreadTable(RAW *shmAddr, unsigned pos,
					     unsigned level) 
{
    unsigned addr;
    bool err;

    assert(inferiorProcess);
    // Getting thread table address
    addr = inferiorProcess->findInternalAddress("DYNINSTthreadTable",true, err);
    assert(!err);

    // Find the right position for this thread in the threadTable
    addr += pos*sizeof(unsigned);

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
#if defined(TEST_DEL_DEBUG)
      int tid=-1;
      for (unsigned i=0;i<inferiorProcess->threads.size();i++) {
	if ((inferiorProcess->threads)[i]->get_pos()==pos) {
	  tid=(inferiorProcess->threads)[i]->get_tid();
	  break;
	}
      }
      sprintf(errorLine,"-----> in updateThreadTable for tid=%d, pos=%d, thr-addr=0x%x, fih-addr=0x%x\n",tid,pos,addr,tmp_addr);
      logLine(errorLine);
#endif
      
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
#endif

template <class HK, class RAW>
void superVector<HK, RAW>::setBaseAddrInApplic(RAW *addr, 
#if defined(MT_THREAD)
        unsigned level)
#else
        unsigned /*level*/)
#endif
{
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx, maxCol;
     idx = inferiorProcess->threads[i]->get_pd_pos();
     maxCol = inferiorProcess->getTable().getMaxSize();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL);
     // addr should be different for each fastInferiorHeap in theSuperVector.
     // we need tu pass addr+offset and not just addr - naim 3/21/97
     unsigned offset = (level*maxCol + idx)*statemap.size();
     theSuperVector[idx]->setBaseAddrInApplic(addr+offset);
#if defined(MT_THREAD)
#if defined(TEST_DEL_DEBUG)
     sprintf(errorLine,"---> setBaseAddrInApplic, updating thread table for tid=%d\n",inferiorProcess->threads[i]->get_tid());
     logLine(errorLine);
#endif
     updateThreadTable(addr+offset,inferiorProcess->threads[i]->get_pos(),level);
#endif
   }
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMajorSample()
{
   bool ok=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     pdThread *thr;
     thr = inferiorProcess->threads[i];
     assert(thr);
     idx = thr->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL);
     ok = ok && theSuperVector[idx]->doMajorSample(statemap);
   }
   return(ok);
}

template <class HK, class RAW>
bool superVector<HK, RAW>::doMinorSample()
{
   bool ok=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     pdThread *thr;
     thr = inferiorProcess->threads[i];
     assert(thr);
     idx = thr->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL); 
     ok = ok && theSuperVector[idx]->doMinorSample(statemap);
   }
   return(ok);
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2LocalAddr(unsigned position, unsigned allocatedIndex) const
{
  return(theSuperVector[position]->index2LocalAddr(allocatedIndex));
}

template <class HK, class RAW>
RAW *superVector<HK, RAW>::index2InferiorAddr(unsigned position, 
					      unsigned allocatedIndex) const
{
  assert(theSuperVector[position] != NULL);
  return(theSuperVector[position]->index2InferiorAddr(allocatedIndex));
}

template <class HK, class RAW>
void superVector<HK, RAW>::initializeHKAfterFork(unsigned allocatedIndex, 
						 const HK &iHouseKeepingValue)
{
   // should be called only for a maybe-allocated-by-fork value
   assert(statemap[allocatedIndex] == FIHmaybeAllocatedByFork ||
	  statemap[allocatedIndex] == FIHmaybeAllocatedByForkButDoNotSample);
   if (statemap[allocatedIndex] == FIHmaybeAllocatedByFork)
     statemap[allocatedIndex] = FIHallocated;
   if (statemap[allocatedIndex] == FIHmaybeAllocatedByForkButDoNotSample)
     statemap[allocatedIndex] = FIHallocatedButDoNotSample;

   // write HK:
   for (unsigned i=0;i<theSuperVector.size();i++) {
     theSuperVector[i]->initializeHKAfterFork(allocatedIndex,
					      iHouseKeepingValue);
   }
}

template <class HK, class RAW>
void superVector<HK, RAW>::addColumns(unsigned from, unsigned to,
				      unsigned subHeapIndex, 
				      unsigned level)
{
   RAW *iBaseAddrInParadynd;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   unsigned adr_offset, col, maxCol;
   maxCol = inferiorProcess->getTable().getMaxSize();
   iBaseAddrInParadynd = (RAW *) iHeapMgr.getSubHeapInParadynd(subHeapIndex);
   assert(iBaseAddrInParadynd);
   for (unsigned i=from; i<to; i++) {
       col = i;
       adr_offset = (level*maxCol + col)*statemap.size();
       fastInferiorHeap<HK, RAW> *theFastInferiorHeap = new fastInferiorHeap<HK, RAW>(iBaseAddrInParadynd+adr_offset, inferiorProcess, statemap.size());
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
#if defined(TEST_DEL_DEBUG)
	   sprintf(errorLine,"---> addColumns, updating thread table for tid=%d\n",thr->get_tid());
	   logLine(errorLine);
#endif
	   updateThreadTable(iBaseAddrInApplic+adr_offset,idx,level);
	   break;
	 }
       }

       theSuperVector += theFastInferiorHeap;
   }

   // Note: do we need to "cleanup" columns first? - naim 10/10/97
}

template <class HK, class RAW>
void superVector<HK, RAW>::addThread(unsigned pos, unsigned pd_pos, 
				     unsigned subHeapIndex, 
				     unsigned level)
{
   unsigned adr_offset, maxCol;
   const fastInferiorHeapMgr &iHeapMgr = inferiorProcess->getShmHeapMgr();
   maxCol = inferiorProcess->getTable().getMaxSize();
   adr_offset = (level*maxCol + pd_pos)*statemap.size();

   // update DYNINSTthreadTable with new fastInferiorHeap address - naim
   RAW *iBaseAddrInApplic = (RAW *) iHeapMgr.getSubHeapInApplic(subHeapIndex);
   assert(iBaseAddrInApplic);
   assert(theSuperVector[pd_pos]!=NULL);
   theSuperVector[pd_pos]->setBaseAddrInApplic(iBaseAddrInApplic+adr_offset);
#if defined(TEST_DEL_DEBUG)
   sprintf(errorLine,"---> addThread, updating thread table for pos=%d, pd_pos=%d, index=%d, level=%d, addr-appl=0x%x\n",pos,pd_pos,subHeapIndex,level,iBaseAddrInApplic+adr_offset);
   logLine(errorLine);
#endif
   updateThreadTable(iBaseAddrInApplic+adr_offset,pos,level);

   // reconstruct permanent sampling set. The new thread inherits all the
   // instrumentation valid for this fastInferiorHeap - naim
   theSuperVector[pd_pos]->reconstructPermanentSamplingSet(statemap);
}

template <class HK, class RAW>
void superVector<HK, RAW>::deleteThread(unsigned pos, unsigned pd_pos, 
					unsigned level)
{
   assert(theSuperVector[pd_pos]!=NULL);
   theSuperVector[pd_pos]->setBaseAddrInApplic(NULL);
   updateThreadTable(NULL,pos,level);
   theSuperVector[pd_pos]->initialize_activemap(statemap.size());
   theSuperVector[pd_pos]->initialize_houseKeeping(statemap.size());
}
#endif //MT_THREAD
