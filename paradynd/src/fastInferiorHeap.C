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
			     houseKeeping(parent.houseKeeping), // copy hk
			     permanentSamplingSet(parent.permanentSamplingSet), // copy
			     currentSamplingSet(parent.currentSamplingSet) // copy
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
   // Here is what has already been done:

   baseAddrInApplic   = (RAW*)appl_attachedAt;
   baseAddrInParadynd = (RAW*)paradynd_attachedAt;

   firstFreeIndex = parent.firstFreeIndex;
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::~fastInferiorHeap() {
   // destructor for statemap[] and houseKeeping[] is called automatically

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

   houseKeeping[allocatedIndex] = iHKValue;

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

   // Write "iValue" to the inferior heap, by writing to the shared memory segment.
   // Should we grab the mutex lock before writing?  Right now we don't, on the
   // assumption that no trampoline in the inferior process is yet writing to
   // this just-allocated memory.  (Mem should be allocated: data first, then initialize
   // tramps, then actually insert tramps)

   // Don't forget to leave the first 16 bytes of meta-data alone...
   // (ergo, we use baseAddrInParadynd instead of trueBaseAddrInParadynd)
   RAW *destRawPtr = baseAddrInParadynd + allocatedIndex; // ptr arith
   *destRawPtr = iValue; // RAW::operator=(const RAW &) if defined, else a bit copy

   // sampling set: add to permanent; no need to add to current
   // note: because allocatedIndex is not necessarily larger than all of the current
   //       entries in permanentSamplingSet[] (non-increasing allocation indexes can
   //       happen all the time, once holes are introduced into the statemap due to
   //       deallocation & garbage collection), we reconstruct the set from scratch;
   //       it's the only easy way to maintain our invariant that the set is sorted.
   const unsigned oldPermanentSamplingSetSize = permanentSamplingSet.size();

   permanentSamplingSet.resize(0);
   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      if (statemap[lcv] == allocated)
         permanentSamplingSet += lcv;
   assert(permanentSamplingSet.size() == oldPermanentSamplingSetSize + 1);

   return true;
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
   // Remember that both sets need to stay in sorted order, so we'd have to shift items
   // to the right after removing from both sets, instead of just swapping with the
   // last item and reducing the size by 1.  Since it can't be done fast, we just
   // reconstruct from scratch, for simplicity.

   const unsigned oldPermanentSamplingSetSize = permanentSamplingSet.size();
   assert(oldPermanentSamplingSetSize > 0);

   // Verify: ndx should be in permanentSamplingSet[], before we reconstruct the set.
   bool found = false;
   for (unsigned lcv=0; lcv < permanentSamplingSet.size() && !found; lcv++)
      if (permanentSamplingSet[lcv] == ndx)
	 found = true;
   assert(found);

   permanentSamplingSet.resize(0);
   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
      if (statemap[lcv] == allocated)
         permanentSamplingSet += lcv;

   assert(permanentSamplingSet.size() == oldPermanentSamplingSetSize - 1);

   // What about the current sampling set?  We can conservatively set it to contain more
   // entries than need be.
   currentSamplingSet = permanentSamplingSet;
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::garbageCollect(const vector<unsigned> &PCs) {
   // tries to set some pending-free items to free, giving preference to freeing items
   // at the end of the heap, thus tending keeping the end of the heap entirely
   // free, which reduces the # of items which need to be read during grabFromApplic().

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
bool fastInferiorHeap<HK, RAW>::doMajorSample(unsigned long long, // wall time
					      unsigned long long // process time
					      ) {
   // return true iff a complete sample was made
   // theWallTime passed in should be in microsecs (since what time?)

   // We used to take in a process (virtual) time as the last param, passing it
   // on to HK::perform().  But we've found that the process time (when used as
   // fudge factor when sampling an active process timer) must be taken at the
   // same time the sample is taken to avoid incorrectly scaled fudge factors,
   // leading to jagged spikes in the histogram (i.e. incorrect samples).
   // The same applies to wall-time, so we ignore the 2d-to-last param too.

   currentSamplingSet = permanentSamplingSet;
      // not a fast operation; vector::operator=()

   // Verify that every item in the sampling set is allocated (i.e. should be sampled)
   for (unsigned lcv=0; lcv < currentSamplingSet.size(); lcv++)
      assert(statemap[currentSamplingSet[lcv]] == allocated);

   // Verify that every allocated statemap item is in the current sampling set,
   // and that the current sampling set is ordered. (A very strong set of asserts.)
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
	 currentSamplingSet[lcv] = currentSamplingSet[--numLeftInMinorSample];
      }
   }

   const bool result = (numLeftInMinorSample == 0);

   return result;
}
