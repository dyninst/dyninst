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

// fastInferiorHeap.C
// Ari Tamches

#include <sys/types.h>
#include <limits.h>
#include "util/h/headers.h"
#include "fastInferiorHeap.h"
#include "rtinst/h/rtinst.h" // for time64

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::handleExec() {
   // after the exec syscall...the applic has detached from the segment, but paradynd
   // is still attached to one...and we'll reuse it.
   
   // inferiorProcess doesn't change
   // statemap and houseKeeping sizes don't change (but we need to reset their contents)
   assert(statemap.size() == houseKeeping.size());

   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      statemap[lcv] = free;

   // here's a quick-and-dirty way to reinitialize the houseKeeping vector (I think):
   houseKeeping.resize(0);
   houseKeeping.resize(statemap.size());

   firstFreeIndex = 0;

   // baseAddrInParadynd doesn't change

   // baseAddrInApplic: reset to NULL since applic isn't attached any more (it needs
   // to re-run DYNINSTinit)
   baseAddrInApplic = NULL;

   // sampling sets: reset
   permanentSamplingSet.resize(0);
   currentSamplingSet.resize(0);
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::
fastInferiorHeap(RAW *iBaseAddrInParadynd,
		 process *iInferiorProcess,
		 unsigned heapNumElems) :
	      inferiorProcess(iInferiorProcess),
              statemap(heapNumElems),
	      houseKeeping(heapNumElems) {
   assert(heapNumElems > 0);

   baseAddrInApplic   = NULL; // until setBaseAddrInApplic() gets called...
   baseAddrInParadynd = iBaseAddrInParadynd;

   // Initialize statemap by setting all entries to state 'free'
   for (unsigned ndx=0; ndx < statemap.size(); ndx++)
      statemap[ndx] = free;

   firstFreeIndex = 0;

   // Note: houseKeeping[] values are uninitialized; each call to alloc() initializes
   //       one HK (here) and one RAW (in the inferior heap).

   // Note: permanentSamplingSet and currentSamplingSet are initialized to empty arrays
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::fastInferiorHeap(const fastInferiorHeap<HK, RAW> &parent,
					    process *newProc,
					    void *paradynd_attachedAt,
					    void *appl_attachedAt) :
                             inferiorProcess(newProc),
			     statemap(parent.statemap), // copy statemap
			     houseKeeping(parent.houseKeeping.size()) // just size it
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.

   baseAddrInApplic   = (RAW*)appl_attachedAt;
   baseAddrInParadynd = (RAW*)paradynd_attachedAt;

   firstFreeIndex = parent.firstFreeIndex;

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
   // fill it in.  If, for whatevery reason, some allocated datareqnode doesn't have
   // its HK filled in by initializeAfterFork(), then the HK is left undefined
   // (in particular, its mi will be NULL), so we should get an assert error rather
   // soon...the next shm sample, in fact.

   // Furthermore: we initially turn each allocated entry in the new heap into type
   // maybeAllocated.  Why do we do this?
   // Because some of the allocated items of the parent process don't belong in the
   // child process after all...specifically, those with foci narrowed down to a
   // specific process --- the parent process.  In such cases, the new process
   // shouldn't get a copy of the mi.  On the other hand, what about foci which aren't
   // specific to any process, and thus should be copied to the new heap?  Well in such
   // cases, intializeHKAfterFork() will be called...so at that time, we'll turn the
   // item back to allocated.  A bit ugly, n'est-ce pas?.  --ari

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == allocated)
	 statemap[lcv] = maybeAllocatedByFork;
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
void fastInferiorHeap<HK, RAW>::forkHasCompleted() {
   // call when a fork has completed (i.e. after you've called the fork ctor AND
   // also metricDefinitionNode::handleFork, as forkProcess [context.C] does).
   // performs some assertion checks, such as mi != NULL for all allocated HKs.

   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == maybeAllocatedByFork)
         // this guy isn't being carried over, because the focus was specific to a
	 // process -- some other process -- before the fork.  (Can we check this?)
	 statemap[lcv] = free;

      if (statemap[lcv] == allocated) {
	 const HK &theHK = houseKeeping[lcv];
	 theHK.assertWellDefined();
      }
   }

   // entries that were allocated (and thus in the sampling set) of the parent
   // may not carry over to the child, in which case the sampling set(s) can become
   // invalid.
   reconstructPermanentSamplingSet();
   currentSamplingSet = permanentSamplingSet; // just being conservative
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::~fastInferiorHeap() {
   // destructors for statemap[], houseKeeping[], permanentSamplingSet[], and
   // currentSamplingSet[] are called automatically
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::alloc(const RAW &iValue,
				      const HK &iHKValue,
				      unsigned &allocatedIndex) {
   // See the .h file for extensive documentation on this routine...

   if (firstFreeIndex == UINT_MAX) {
      // heap is full!  Garbage collect and try a second time.
      cout << "fastInferiorHeap alloc: heap is full; about to garbage collect" << endl;

      for (unsigned lcv=0; lcv < statemap.size(); lcv++)
         assert(statemap[lcv] != free);

      const vector<unsigned> PCs = inferiorProcess->walkStack(); // prob expensive
      garbageCollect(PCs);
      if (firstFreeIndex == UINT_MAX) {
         // oh no; inferior heap is still full!  Garbage collection has failed.
	 cout << "fastInferiorHeap alloc: heap is full and garbage collection FAILED" << endl;
         return false; // failure
      }
   }

   assert(firstFreeIndex < statemap.size());

   allocatedIndex = firstFreeIndex; // allocatedIndex is a ref param
   assert(statemap[allocatedIndex] == free);
   statemap[allocatedIndex] = allocated;

   houseKeeping[allocatedIndex] = iHKValue; // HK::operator=()

   // Write "iValue" to the inferior heap, by writing to the shared memory segment.
   // Should we grab the mutex lock before writing?  Right now we don't, on the
   // assumption that no trampoline in the inferior process is yet writing to
   // this just-allocated memory.  (Mem should be allocated: data first, then initialize
   // tramps, then actually insert tramps)

   RAW *destRawPtr = baseAddrInParadynd + allocatedIndex; // ptr arith
   *destRawPtr = iValue; // RAW::operator=(const RAW &) if defined, else a bit copy

   // update firstFreeIndex to point to next free entry; UINT_MAX if full
   while (++firstFreeIndex < statemap.size()) {
      if (statemap[firstFreeIndex] == free)
         break;
   }

   if (firstFreeIndex >= statemap.size()) {
      // inferior heap is now full (but the allocation succeeded)
      assert(firstFreeIndex == statemap.size());
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
   const unsigned oldPermanentSamplingSetSize = permanentSamplingSet.size();

   reconstructPermanentSamplingSet();

   assert(permanentSamplingSet.size() == oldPermanentSamplingSetSize + 1);

   return true;
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::reconstructPermanentSamplingSet() {
   permanentSamplingSet.resize(0);

   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      if (statemap[lcv] == allocated)
         permanentSamplingSet += lcv;
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::initializeHKAfterFork(unsigned index,
						      const HK &iHKValue) {
   // should be called only for a maybe-allocated-by-fork value
   assert(statemap[index] == maybeAllocatedByFork);
   statemap[index] = allocated;

   // write HK:
   houseKeeping[index] = iHKValue; // HK::operator=()
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::makePendingFree(unsigned ndx,
						const vector<unsigned> &trampsUsing) {
//   cout << "fastInferiorHeap: welcome to makePendingFree" << endl;
   assert(ndx < statemap.size());
   assert(statemap[ndx] == allocated);
   statemap[ndx] = pendingfree;

   houseKeeping[ndx].makePendingFree(trampsUsing);

   // firstFreeIndex doesn't change

   // Sampling sets: a pending-free item should no longer be sampled, so remove
   // it from both permanentSamplingSet and currentSamplingSet.
   // Remember that the permanent set needs to stay sorted.  Since it can't be done
   // fast, we just reconstruct from scratch, for simplicity.

   const unsigned oldPermanentSamplingSetSize = permanentSamplingSet.size();
   assert(oldPermanentSamplingSetSize > 0);

#ifdef SHM_SAMPLING_DEBUG
   // Verify: ndx should be in permanentSamplingSet[], before we reconstruct the set.
   bool found = false;
   for (unsigned lcv=0; lcv < permanentSamplingSet.size() && !found; lcv++)
      if (permanentSamplingSet[lcv] == ndx)
	 found = true;
   assert(found);
#endif

   permanentSamplingSet.resize(0);
   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      if (statemap[lcv] == allocated)
         permanentSamplingSet += lcv;

   assert(permanentSamplingSet.size() == oldPermanentSamplingSetSize - 1);

   // What about the current sampling set?  We can conservatively set it to contain more
   // entries than need be, which we do.  (We could also set it to the empty set;
   // at worst, one bucket of sampling data is lost, which is no big deal).
   currentSamplingSet = permanentSamplingSet;
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::garbageCollect(const vector<unsigned> &PCs) {
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
      if (statemap[ndx] == pendingfree && houseKeeping[ndx].tryGarbageCollect(PCs)) {
         statemap[ndx] = free;

         // update firstFreeIndex:
         if (ndx < firstFreeIndex)
            firstFreeIndex = ndx;
      }
   } while (ndx);
 
   // We've gone thru every item in the inferior heap so there's no more garbage
   // collection we can do at this time.

   // Note that garbage collection doesn't affect either of the sampling sets.
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::doMajorSample(time64, // wall time
					      time64  // process time
					      ) {
   // return true iff a complete sample was made
   // theWallTime passed in should be in microsecs (since what time?)

   // We used to take in a process (virtual) time as the last param, passing it
   // on to HK::perform().  But we've found that the process time (when used as
   // fudge factor when sampling an active process timer) must be taken at the
   // same time the sample is taken to avoid incorrectly scaled fudge factors,
   // leading to jagged spikes in the histogram (i.e. incorrect samples).
   // The same applies to wall-time, so we ignore both params.

   currentSamplingSet = permanentSamplingSet;
      // not a fast operation; vector::operator=()

#ifdef SHM_SAMPLING_DEBUG
   // Verify that every item in the sampling set is allocated (i.e. should be sampled)
   for (unsigned lcv=0; lcv < currentSamplingSet.size(); lcv++)
      assert(statemap[currentSamplingSet[lcv]] == allocated);
#endif

   // Verify that every allocated statemap item is in the current sampling set,
   // and that the current sampling set is ordered. (A very strong set of asserts.)
   // (Note: The curr sampling set is sorted now (at the start of a major sample), but
   //        it likely won't remain so for long...which is OK.)
   // It's a bit too expensive to do casually, however, so we ifdef it:
#ifdef SHM_SAMPLING_DEBUG
   unsigned cssIndex=0; // current-sampling-set index
   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == allocated)
	 assert(currentSamplingSet[cssIndex++] == lcv);
   }
   assert(cssIndex == currentSamplingSet.size());
#endif

   const bool result = doMinorSample();

   return result;
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::doMinorSample() {
   // samples items in currentSamplingSet[]; returns false if all done successfully

   unsigned numLeftInMinorSample = currentSamplingSet.size();

   unsigned lcv = 0;
   while (lcv < numLeftInMinorSample) {
      // try to sample this item
      const unsigned index = currentSamplingSet[lcv];
      assert(statemap[index] == allocated);

      // HK::perform() returns true iff sampling succeeded without having to
      // wait; false otherwise.  It handles all needed synchronization.
      if (!houseKeeping[index].perform(*(baseAddrInParadynd + index), // ptr arith
				       inferiorProcess)) {
	 // the item remains in "currentSamplingSet[]"; move on to the next candidate
	 lcv++;
      }
      else {
	 // remove the item from "currentSamplingSet[]"; note that lcv doesn't change
         // Note also that the current sampling set, unlike the permanent sampling
         // set, does not remain sorted.
	 currentSamplingSet[lcv] = currentSamplingSet[--numLeftInMinorSample];
      }
   }

   const bool result = (numLeftInMinorSample == 0);

   return result;
}
