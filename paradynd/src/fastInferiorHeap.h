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

// fastInferiorHeap.h
// Ari Tamches
// This class is intended to be used only in paradynd.
// This templated class manages a heap of objects in a UNIX shared-memory segment
// (currently, the template types that are used are either intCounter or tTimer).
// Note that the shm segment itself is managed by our "parent" class,
// fastInferiorHeapMgr.h/.C.  This class doesn't actually use any shared-memory
// primitives...

// Previously, this class would manage its own shared-memory segment, so that
// if a process creates, say, a fastInferiorHeap<intCounter> and two
// fastInferiorHeap<tTimer>, then 3 shm segments are created (and 6 shmat()'s
// are done, since both paradynd and the application need to shmat() to a given
// segment).  To cut down on the number of segments floating around, and to reduce
// and hopefully avoid the dreaded EMFILE errno when shmat()'ing to a segment,
// there's now just 1 shm segment per process (and 2 shmat()'s are done).
// The actual shm seg management is now done in file fastInferiorHeapMgr.h/.C

#ifndef _FAST_INFERIOR_HEAP_H_
#define _FAST_INFERIOR_HEAP_H_

#pragma interface

#include "util/h/Vector.h"

class process; // avoids need for an expensive #include

template <class HK, class RAW>
   // where HK is the housekeeping information for something like "counter" or "timer"
   // (with entries for everything except the actual counter or timer value) and where
   // RAW is the same raw type used in the appl heap; presumably "int", etc.
class fastInferiorHeap {
 private:
   enum states {allocated, free, pendingfree, maybeAllocatedByFork};

   process *inferiorProcess;
      // ptr instead of ref due to include file problems (if this file is included w/in
      // class process then class process isn't fully defined when we reach this point
      // so it won't let us use a ref).

   // Let's take a moment to think about whether a 'baseAddrInApplic' is needed.
   // 1) It's not needed in order for paradynd to write to the shared seg;
   //    baseAddrInParadynd is used for that.
   // 2) But it is needed for paradynd to add instrumentation code that uses
   //    an object.  For example, say paradynd wants to add code instrumentation
   //    which calls startTimer() with (this is what's important here) the argument of
   //    a ptr to the tTimer.  To do this, it needs the addr in the inferior appl, not
   //    the addr in paradynd's attachment.

   RAW * baseAddrInApplic;
      // When ctor #1 (not the fork ctor) is used, this vrble is undefined until
      // setBaseAddrInApplic() is called.

   RAW * baseAddrInParadynd;

   vector<states> statemap; // one entry per value (allocated or not) in the appl.
   vector<HK> houseKeeping; // one entry per value (allocated or not) in the appl.

   unsigned firstFreeIndex;
      // makes allocation quick; UINT_MAX --> no free elems in heap (but there could be
      // pending-free items)

   // Keeps track of what needs sampling, needed to sort out major/minor sampling
   vector<unsigned> permanentSamplingSet; // all allocated indexes
   vector<unsigned> currentSamplingSet; // a subset of permanentSamplingSet

   // Since we don't define these, making them private makes sure they're not used:
   fastInferiorHeap &operator=(const fastInferiorHeap &);
   fastInferiorHeap(const fastInferiorHeap &);

   void reconstructPermanentSamplingSet();

 public:

   fastInferiorHeap(RAW *iBaseAddrInParadynd,
                    process *iInferiorProcess,
		    unsigned heapNumElems);
      // Note that the ctor has no way to pass in the baseAddrInApplic because
      // the applic hasn't yet attached to the segment.  When the applic attaches
      // and tells us where it attached, we can call setBaseAddrInApplic() to fill
      // it in.

   fastInferiorHeap(const fastInferiorHeap &parent, process *newProc,
		    void *paradynd_attachedAt,
		    void *appl_attachedAt);
      // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
      // From the process' point of view after the fork(): the fork() has attached it
      // to all shm segments of its parent; so, it needs to unattach() from them
      // and then attach to a new segment.

  ~fastInferiorHeap();

   void handleExec();
      // call after the exec syscall has executed.  Basically we need to redo everything
      // that we did in the (non-fork) constructor.  Well, some things don't change: the
      // process ptr, the addr that paradynd attached at, the shm seg key.

   void forkHasCompleted();
      // call when a fork has completed (i.e. after you've called the fork ctor AND
      // also metricDefinitionNode::handleFork, as forkProcess [context.C] does).
      // performs some assertion checks, such as mi != NULL for all allocated HKs.
      // also recalculates some things, such as the sampling sets.

   void setBaseAddrInApplic(RAW *addr) {
      // should call _very_ soon after the ctor, right after the applic has
      // attached to the shm segment.
      assert(baseAddrInApplic == NULL); // not for long...
      baseAddrInApplic = addr;
   }

   RAW *getBaseAddrInApplic() const {
      assert(baseAddrInApplic != NULL);
      return baseAddrInApplic;
   }

   RAW *index2InferiorAddr(unsigned allocatedIndex) const {
      assert(baseAddrInApplic != NULL);
      assert(allocatedIndex < statemap.size());    
      return baseAddrInApplic + allocatedIndex;
   }

   RAW *index2LocalAddr(unsigned allocatedIndex) const {
      assert(baseAddrInParadynd != NULL);
      assert(allocatedIndex < statemap.size());    
      return baseAddrInParadynd + allocatedIndex;
   }

   void initializeHKAfterFork(unsigned index, const HK &iHKValue);
      // After a fork, the hk entry for this index will be copied, which is probably
      // not what you want.  (We don't provide a param for the raw item since you
      // can write the raw item easily enough by just writing directly to shared
      // memory...see baseAddrInParadynd.)
                            
   bool alloc(const RAW &iRawValue, const HK &iHouseKeepingValue,
	      unsigned &allocatedIndex);
      // Allocate an entry in the inferior heap and initialize its raw value with
      // "iRawValue" and its housekeeping value with "iHouseKeepingValue".
      // Returns true iff successful; false if not (because inferior heap was full).
      // If true, then also sets "allocatedIndex" to the index (0 thru heapNumElems-1)
      // that was allocated in param "allocatedIndex"; this info is probably needed by
      // the caller.  For example, the caller might be allocating a mini-tramp
      // that calls startTimer(); it needs "allocatedIndex" to calculate the
      // address of the timer structure (which is passed to startTimer).

      // Note: we write to the inferior heap, in order to fill in the initial value.
      //       Since the inferior heap is in shared memory, we don't need to write it
      //       using /proc lseek/write() combo.  Which means the inferior process
      //       doesn't necessarily need to be paused (which is slow as hell to do).
      //       This is nice, but remember:
      //       (1) /proc writes are still needed at least for patching code to
      //       cause a jump to a base tramp.  Conceivably though, the base tramp and
      //       all mini-tramps (and of course counters & timers they write to) can be
      //       in shared-memory, which is nice.
      //       (2) if you're not going to pause the application, beware of race
      //       conditions.  A nice touch is to create backwards; i.e. allocate the
      //       timer/counter in shared memory, then the mini-tramp in shared memory,
      //       then the base tramp in shared memory if need be, then initialize the
      //       code contents of the tramps, and only then patch up some function
      //       to call our base tramp (the last step using a pause; /proc write; unpause
      //       sequence).

   void makePendingFree(unsigned ndx, const vector<unsigned> &trampsUsing);
      // "free" an item in the shared-memory heap.  More specifically, change its
      // statemap type from allocated to pending-free.  A later call to garbageCollect()
      // is the only way to truly free the item.  An item in pending-free
      // state will no longer be processed by processAll().
      // Note that we don't touch the shared-memory heap; we just play around with
      // statemap meta-data stuff.  Of course that doesn't mean that it's okay to call
      // this with the expectation that the raw item will still be written to, except
      // perhaps by a tramp that is itself in the process of being freed up.

   void garbageCollect(const vector<unsigned> &PCs);
      // called by alloc() if it needs memory, but it's a good idea to call this
      // periodically; progressive preemptive garbage collection can help make
      // allocations requests faster.
      // The parameter is a stack trace in the inferior process, containing PC-register
      // values.

   bool doMajorSample(time64 wallTime, time64 procTime);
      // Reads data values (in the shared memory heap) and processes allocated item
      // by calling HK::perform() on it.
      // Note: doesn't pause the application; instead, reads from shared memory.
      // returns true iff the sample completed successfully.

   bool doMinorSample();
      // call every once in a while after a call to doMajorSample() returned false.
      // It'll resample and return true iff the re-sample finished the job.  We keep
      // around state (currentSamplingSet) to keep track of what needs re-sampling;
      // this is reset to 'everything' (permanentSamplingSet) upon a call to
      // doMajorSample() and is reduced by that routine and by calls to doMinorSample().
};

#endif
