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
   enum states {allocated, free, pendingfree};

   process *inferiorProcess;
      // ptr instead of ref due to include file problems (if this file is included w/in
      // class process then class process isn't fully defined when we reach this point
      // so it won't let us use a ref.

   // Let's take a moment to think about whether a 'baseAddrInApplic' is needed.
   // 1) It's not needed in order for paradynd to write to the shared seg; paradynd
   //    should be using baseAddrInParadynd for that anyway.
   // 2) But it is needed for paradynd to add instrumentation code that uses
   //    an object.  For example, say paradynd wants to add code instrumentation
   //    at a certain point; the instrumentation will call startTimer() with
   //    (this is what's important here) the argument of a ptr to the tTimer.  In
   //    order for paradynd to do this, it needs the addr in the inferior appl, not
   //    the addr in paradynd's attachment.

   key_t theShmKey; // retrieve with call to getShmSegKey(); pass result to inferior
                    // process so it may attach to the segment.
   RAW * baseAddrInApplic; // not set in ctor; filled in later (but hopefully soon!)
      // note: like baseAddrInParadynd, this ptr is, for convenience, already
      // skipped ahead of the 16 bytes of meta-data at the start of the shm seg

   void * trueBaseAddrInParadynd;
   RAW * baseAddrInParadynd; // same as above but add 16 bytes for meta-data

   int   shmid; // for trueBaseAddrInParadynd
      // These variables indicate where in virtual memory the shared segment (where
      // the raw data values, not meta-data housekeeping, are stored) resides.
      // trueBaseAddrInParadynd points to where in our (paradynd's) virtual address space
      // this segment starts.
      // baseAddrInApplic points to where in the application's virtual address
      // space the segment starts.

   vector<states> statemap; // one entry per value (allocated or not) in the appl.
   vector<HK> houseKeeping; // one entry per value (allocated or not) in the appl.

   unsigned firstFreeIndex;
      // makes allocation quick; UINT_MAX --> no free elems in heap

   // Since we don't define these, make sure they're not used:
   fastInferiorHeap &operator=(const fastInferiorHeap &);
   fastInferiorHeap(const fastInferiorHeap &);
   
   void processAll(vector<unsigned> &indexesToProcess,
		   unsigned &numIndexesLeftToProcess,
		   unsigned long long theWallTime,
		   unsigned long long theCpuTime);

 public:

   fastInferiorHeap(process *iInferiorProcess,
		    const key_t &theShmKey,
                    unsigned heapNumElems);
      // Note that the ctor has no way to pass in the baseAddrInApplic.
      // That's because paradynd creates the seg first, so it's just not possible
      // yet to know where the shared seg resides in the appl.  That gets filled in
      // with a call to setBaseAddrInApplic(), which should be asap.

   fastInferiorHeap(const fastInferiorHeap &parent, process *newProc, key_t iShmKey,
		    void *appl_attachedAt // we add 16 bytes for you
		    );
      // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
      // From the process' point of view after the fork(): the fork() has attached it
      // to all shm segments of its parent; so, it needs to unattach() from them
      // and then attach to a segment newly created by us, which we do here.

  ~fastInferiorHeap();
      // Fries the shm segment

   key_t getShmSegKey() const {return theShmKey;}
   unsigned getShmSegNumBytes() const {return sizeof(RAW) * statemap.size();}

   void setBaseAddrInApplic(RAW *iBaseAddrInApplic) {
      // should call _very_ soon after the ctor, right after the applic has
      // attached to the shm segment.  Of course, in order to do that, the applic
      // needs to be sent the key; a call getShmSegKey() is assumed.
      assert(baseAddrInApplic == NULL); // not for long...
      baseAddrInApplic = iBaseAddrInApplic;
   }

   RAW *getBaseAddrInApplic() const {
      assert(baseAddrInApplic != NULL);
      return baseAddrInApplic;
   }

   void *getTrueBaseAddrInApplic() const {
      assert(baseAddrInApplic != NULL);
         // baseAddrInApplic is 16 bytes beyond where we want

      unsigned char *result = (unsigned char *)baseAddrInApplic;
      result -= 16; // byte-size ptr arith

      return result; // implicit conversion to void * allowed
   }

   void *getTrueBaseAddrInParadynd() const {
      assert(trueBaseAddrInParadynd);

      return trueBaseAddrInParadynd; // 16 bytes beyond this is where the data elems starts
   }

   RAW *index2InferiorAddr(unsigned allocatedIndex) const {
      assert(baseAddrInApplic != NULL);
      assert(allocatedIndex < statemap.size());    
      return baseAddrInApplic + allocatedIndex;
   }

   bool alloc(const RAW &iRawValue, const HK &iHouseKeepingValue,
	      unsigned &allocatedIndex);
      // Allocate an entry in the inferior heap and initialize its raw value with
      // "iRawValue" and its housekeeping value with "iHouseKeepingValue".
      // Returns true iff successful; false if not (because inferior heap was full).
      // If true, then also "returns" the index (0 thru heapNumElems-1) that was
      // allocated in param "allocatedIndex"; this info is probably needed by the
      // caller.  For example, the caller might be allocating a mini-tramp
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
      // this with the expectation that the item will still be written to, except
      // perhaps by a tramp that is itself in the process of being freed up.

   void garbageCollect(const vector<unsigned> &PCs);
      // called by alloc() if it needs memory, but it's a good idea to call this
      // periodically; progressive preemptive garbage collection can help make
      // allocations requests faster.
      // The parameter is a stack trace in the inferior process, containing PC-register
      // values.

   void processAll(unsigned long long wallTime,
		   unsigned long long procTime);
      // Reads data values (in the shared memory heap) and processes allocated item
      // by calling HK::perform() on it.
      // Note: doesn't pause the application; instead, uses fast inline asm mutex
      // locks.

};

#endif
