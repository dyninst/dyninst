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

// $Id: superVector.h,v 1.8 2002/04/05 19:39:01 schendel Exp $
// The superVector is an array of vectors of counters and timers, or
// fastInferiorHeap objects. Part of the functionality of the fastInferiorHeap
// class has been moved to this new class - naim 3/26/97

#ifndef _SUPER_VECTOR__H_
#define _SUPER_VECTOR__H_

#if !defined(i386_unknown_nt4_0) // interface is a VC++ keyword
#pragma interface
#endif // !defined(i386_unknown_nt4_0)

#include "common/h/Vector.h"
#include "rtinst/h/rtinst.h"
#include "common/h/Types.h"
#include "fastInferiorHeap.h"

template <class HK, class RAW>
class superVector {

  private:
   process *inferiorProcess;
      // ptr instead of ref due to include file problems (if this file is
      // included w/in class process then class process isn't fully defined
      // when we reach this point so it won't let us use a ref).

   vector<states> statemap; // one entry per value (allocated or not) in the appl.
#if !defined(MT_THREAD)
   vector<HK> houseKeeping; // one entry per value (allocated or not) in the appl.
#endif
  
   unsigned firstFreeIndex;
      // makes allocation quick; UI32_MAX --> no free elems in heap (but
      // there could be pending-free items)

   vector < fastInferiorHeap<HK,RAW> *> theSuperVector;
      // the index here is the thread id (after it's been mapped)

   // since we don't define them, make sure they're not used:
   // (VC++ requires template members to be defined if they are declared)
   superVector(const superVector &)				{}
   superVector &operator=(const superVector &)	{ return *this; }

  public: 

#if defined(MT_THREAD)
   superVector(process *iInferiorProcess,
	       unsigned heapNumElems,
               unsigned level,
	       unsigned subHeapIndex,
               unsigned numberOfColumns,
               bool calledFromBaseTableConst=false);
#else
    superVector(process *iInferiorProcess,
		unsigned heapNumElems,
		unsigned subHeapIndex,
		unsigned numberOfColumns);
#endif
      // Note that the ctor has no way to pass in the baseAddrInApplic
      // because the applic hasn't yet attached to the segment.  When the
      // applic attaches and tells us where it attached, we can call
      // setBaseAddrInApplic() to fill it in.

   superVector(const superVector<HK, RAW> *parent, 
               process *newProc,
               unsigned subHeapIndex);
      // this copy-ctor is a fork()/dup()-like routine.  Call after a process
      // forks.  From the process' point of view after the fork(): the fork()
      // has attached it to all shm segments of its parent; so, it needs to
      // unattach() from them and then attach to a new segment.

  ~superVector();

   void setBaseAddrInApplic(RAW *addr, unsigned level);

#if defined(MT_THREAD)
   void updateThreadTable(RAW *shmAddr, unsigned pos, unsigned level);
#endif

   void handleExec();
      // call after the exec syscall has executed. Basically we need to redo
      // everything that we did in the (non-fork) constructor. Well, some
      // things don't change: the process ptr, the addr that paradynd
      // attached at, the shm seg key.

   void forkHasCompleted();
      // call when a fork has completed (i.e. after you've called the fork
      // ctor AND also metricDefinitionNode::handleFork, as forkProcess
      // [context.C] does).  performs some assertion checks, such as mi !=
      // NULL for all allocated HKs.  also recalculates some things, such as
      // the sampling sets.

   void initializeHKAfterFork(unsigned allocatedIndex, const HK &iHouseKeepingValue);
      // After a fork, the hk entry for this index will be copied, which is
      // probably not what you want.  (We don't provide a param for the raw
      // item since you can write the raw item easily enough by just writing
      // directly to shared memory...see baseAddrInParadynd.)

   bool alloc(                            
#if defined(MT_THREAD)
              unsigned thr_pos, const RAW &iRawValue, 
#else
              const RAW &iRawValue,
#endif
              const HK &iHouseKeepingValue,
	      unsigned &allocatedIndex,
	      bool doNotSample);
      // Allocate an entry in the inferior heap and initialize its raw value
      // with "iRawValue" and its housekeeping value with
      // "iHouseKeepingValue".  Returns true iff successful; false if not
      // (because inferior heap was full).  If true, then also sets
      // "allocatedIndex" to the index (0 thru heapNumElems-1) that was
      // allocated in param "allocatedIndex"; this info is probably needed by
      // the caller.  For example, the caller might be allocating a
      // mini-tramp that calls startTimer(); it needs "allocatedIndex" to
      // calculate the address of the timer structure (which is passed to
      // startTimer).

      // Note: we write to the inferior heap, in order to fill in the initial
      // value.  Since the inferior heap is in shared memory, we don't need
      // to write it using /proc lseek/write() combo.  Which means the
      // inferior process doesn't necessarily need to be paused (which is
      // slow as hell to do).  This is nice, but remember: (1) /proc writes
      // are still needed at least for patching code to cause a jump to a
      // base tramp.  Conceivably though, the base tramp and all mini-tramps
      // (and of course counters & timers they write to) can be in
      // shared-memory, which is nice.  (2) if you're not going to pause the
      // application, beware of race conditions.  A nice touch is to create
      // backwards; i.e. allocate the timer/counter in shared memory, then
      // the mini-tramp in shared memory, then the base tramp in shared
      // memory if need be, then initialize the code contents of the tramps,
      // and only then patch up some function to call our base tramp (the
      // last step using a pause; /proc write; unpause sequence).

#if defined(MT_THREAD)
   void makePendingFree(unsigned pd_pos, unsigned ndx, const vector<Address> &trampsUsing);
#else
   void makePendingFree(unsigned ndx, const vector<Address> &trampsUsing);
#endif
      // "free" an item in the shared-memory heap.  More specifically, change
      // its statemap type from allocated to pending-free. A later call to
      // garbageCollect() is the only way to truly free the item.  An item in
      // pending-free state will no longer be processed by processAll().
      // Note that we don't touch the shared-memory heap; we just play around
      // with statemap meta-data stuff.  Of course that doesn't mean that
      // it's okay to call this with the expectation that the raw item will
      // still be written to, except perhaps by a tramp that is itself in the
      // process of being freed up.

   void garbageCollect(const vector<Address> &PCs);
      // called by alloc() if it needs memory, but it's a good idea to call
      // this periodically; progressive preemptive garbage collection can
      // help make allocations requests faster.  The parameter is a stack
      // trace in the inferior process, containing PC-register values.

  RAW *index2LocalAddr(unsigned position, unsigned allocatedIndex) const;
  RAW *index2InferiorAddr(unsigned position, unsigned allocatedIndex) const;
  HK *getHouseKeeping(unsigned position, unsigned allocatedIndex) const;

  bool doMajorSample();
  bool doMinorSample();
#if defined(MT_THREAD)
  void addColumns(unsigned from, unsigned to, unsigned subHeapIndex, unsigned level);
  void addThread(unsigned pos, unsigned pd_pos, unsigned subHeapIndex, unsigned level);
  void deleteThread(unsigned pos, unsigned pd_pos, unsigned level);
#endif
};

#endif
