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

// $Id: superVector.C,v 1.4 1999/05/21 17:25:19 wylie Exp $

#include <sys/types.h>
#include <limits.h>
#include "util/h/headers.h"
#include "paradynd/src/superVector.h"
#include "paradynd/src/fastInferiorHeapMgr.h"
#include "paradynd/src/fastInferiorHeap.h"
#include "rtinst/h/rtinst.h" // for time64
#include "dyninstAPI/src/pdThread.h"

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

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == FIHallocated)
	 statemap[lcv] = FIHmaybeAllocatedByFork;
      if (statemap[lcv] == FIHallocatedButDoNotSample)
	 statemap[lcv] = FIHmaybeAllocatedByForkButDoNotSample;
   }

   for (unsigned lcv=0; lcv < houseKeeping.size(); lcv++) {
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

   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
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
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
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
bool superVector<HK, RAW>::doMajorSample(time64 wallTime, time64 procTime)
{
   bool ok=true;
   for (unsigned i=0; i<inferiorProcess->threads.size(); i++) {
     unsigned idx;
     idx = inferiorProcess->threads[i]->get_pd_pos();
     assert(idx < theSuperVector.size());
     assert(theSuperVector[idx] != NULL);
     ok = ok && theSuperVector[idx]->doMajorSample(wallTime,procTime,statemap,houseKeeping);
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
