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

// $Id: fastInferiorHeap.h,v 1.11 2000/07/28 17:22:11 pcroth Exp $
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

#if !defined(i386_unknown_nt4_0)    // interface is a keyword to VC++
#pragma interface
#endif // !defined(i386_unknown_nt4_0)

#include "heapStates.h" // enum states
#include "common/h/Vector.h"
#include "rtinst/h/rtinst.h" // for time64
#include "common/h/Types.h"    // for Address

class process; // avoids need for an expensive #include

template <class HK, class RAW>
   // where HK is the housekeeping information for something like "counter" or "timer"
   // (with entries for everything except the actual counter or timer value) and where
   // RAW is the same raw type used in the appl heap; presumably "int", etc.
class fastInferiorHeap {
 private:
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

   // Keeps track of what needs sampling, needed to sort out major/minor sampling
   vector<unsigned> permanentSamplingSet; // all allocated indexes
   vector<unsigned> currentSamplingSet; // a subset of permanentSamplingSet
   // since we don't define them, make sure they're not used:
   // (VC++ requires template members to be defined if they are declared)
   fastInferiorHeap(const fastInferiorHeap &)   {}
#if defined(MT_THREAD)
   vector<sampling_states> activemap;
   vector<HK> houseKeeping; // one entry per value (allocated or not) in the appl.
#endif

 public:   
   fastInferiorHeap &operator=(const fastInferiorHeap &);

   fastInferiorHeap(){};

   fastInferiorHeap(RAW *iBaseAddrInParadynd,
#if defined(MT_THREAD)
                    process *iInferiorProcess,
                    unsigned mapsize);
#else
                    process *iInferiorProcess);
#endif
      // Note that the ctor has no way to pass in the baseAddrInApplic because
      // the applic hasn't yet attached to the segment.  When the applic attaches
      // and tells us where it attached, we can call setBaseAddrInApplic() to fill
      // it in.

#if defined(MT_THREAD)
   fastInferiorHeap(fastInferiorHeap<HK, RAW> *parent, // 6/3/99 zhichen
		    process* newPorc,
#else
   fastInferiorHeap(process *newProc,
#endif
		    void *paradynd_attachedAt,
		    void *appl_attachedAt);
      // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
      // From the process' point of view after the fork(): the fork() has attached it
      // to all shm segments of its parent; so, it needs to unattach() from them
      // and then attach to a new segment.

  ~fastInferiorHeap();

#if defined(MT_THREAD)
   void set_houseKeeping(unsigned idx, const HK &iHKValue);
   void initialize_houseKeeping(unsigned mapsize);
   void initialize_activemap(unsigned mapsize);
   void set_activemap(unsigned idx, sampling_states value);
#endif // defined(MT_THREAD)

   void setBaseAddrInApplic(RAW *addr) {
      // should call _very_ soon after the ctor, right after the applic has
      // attached to the shm segment.
      //assert(baseAddrInApplic == NULL); // this is not true any more during
                                          // an "exec" - naim
      baseAddrInApplic = addr;
   }


   void updateCurrentSamplingSet() {
      currentSamplingSet = permanentSamplingSet;
   }

   unsigned getPermanentSamplingSetSize() {
      return(permanentSamplingSet.size());
   }

   void clearPermanentSamplingSet() {
      permanentSamplingSet.resize(0);
   }

   void clearCurrentSamplingSet() {
      currentSamplingSet.resize(0);
   }

   RAW *getBaseAddrInApplic() const {
      assert(baseAddrInApplic != NULL);
      return baseAddrInApplic;
   }

   RAW *getBaseAddrInParadynd() const {
      assert(baseAddrInParadynd != NULL);
      return baseAddrInParadynd;
   }

   RAW *index2InferiorAddr(unsigned allocatedIndex) const {
      assert(baseAddrInApplic != NULL);
      return baseAddrInApplic + allocatedIndex;
   }

   RAW *index2LocalAddr(unsigned allocatedIndex) const {
      assert(baseAddrInParadynd != NULL);
      return baseAddrInParadynd + allocatedIndex;
   }

#if defined(MT_THREAD)
   void handleExec();
   void forkHasCompleted(vector<states> &statemap);

   void makePendingFree(unsigned ndx, const vector<Address> &trampsUsing);
   bool checkIfInactive(unsigned ndx);
   bool tryGarbageCollect(const vector<Address> &PCs, unsigned ndx);
   void initializeHKAfterFork(unsigned allocatedIndex, 
                              const HK &iHouseKeepingValue);
#endif // defined(MT_THREAD)
   void addToPermanentSamplingSet(unsigned lcv);

   // Reads data values (in the shared memory heap) and processes allocated
   // item by calling HK::perform() on it.
   // Note: doesn't pause the application; instead, reads from shared memory.
   // returns true iff the sample completed successfully.
#if defined(MT_THREAD)
   bool doMajorSample(time64, time64, const vector<states> &statemap);
#else
   bool doMajorSample(time64, time64, 
                     const vector<states> &statemap,
                     const vector<HK> &houseKeeping);
#endif

   // call every once in a while after a call to doMajorSample returned false.
   // It'll resample and return true iff the resample finished the job. We keep
   // around state (currentSamplingSet) to keep track of what needs resampling;
   // this is reset to 'everything' (permanentSamplingSet) upon a call to
   // doMajorSample() and is reduced by that routine and by calls to 
   // doMinorSample().
#if defined(MT_THREAD)
   bool doMinorSample(const vector<states> &statemap);
#else
   bool doMinorSample(const vector<states> &statemap,
                      const vector<HK> &houseKeeping);
#endif
   void reconstructPermanentSamplingSet(const vector<states> &statemap);

};

#endif
