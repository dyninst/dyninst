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

// fastInferiorHeapMgr.h
// Ari Tamches
// A class that manages several fastInferiorHeaps (fastInferiorHeap.h/.C)
// Formerly, each fastInferiorHeap would create its own shm-segment.
// But this created too many segments.  This class creates a single shm segment,
// containing several fastInferiorHeaps.

#ifndef _FAST_INFERIOR_HEAP_MGR_H_
#define _FAST_INFERIOR_HEAP_MGR_H_

#include "util/h/headers.h"
#include "fastInferiorHeap.h"

class fastInferiorHeapMgr {
 private:
   enum space {paradynd, applic};

   // The following structure contains all that this class needs to know about
   // a given fastInferiorHeap contained within us:
   struct oneHeapStats {
      unsigned elemNumBytes;
      unsigned maxNumElems;
   };
   vector<oneHeapStats> theHeapStats;

   static unsigned cookie;

   // And now for the actual low-level shm segment:
   key_t theShmKey;
   int theShmId; // result of our shmget()
   void *paradyndAttachedAt; // result of our (paradynd's) shmat()
   void *applicAttachedAt;   // result of applic's (DYNINSTinit's) shmat()

   // since we don't define them, make sure they're not used:
   fastInferiorHeapMgr(const fastInferiorHeapMgr &);
   fastInferiorHeapMgr &operator=(const fastInferiorHeapMgr &);

   unsigned getSubHeapOffset(unsigned subHeapIndex) const;
   void *getSubHeapInSpace(unsigned subHeapIndex, space) const;

   enum gcResult {noNeed, gcHappened, gcDidntHappen};
   static gcResult tryToGarbageCollectShmSeg(key_t keyToTry, unsigned size);

 public:
   // normal constructor:
   fastInferiorHeapMgr(key_t firstKeyToTry, const vector<oneHeapStats> &iHeapStats,
                       pid_t inferiorPid);

   // fork constructor:
   fastInferiorHeapMgr(const fastInferiorHeapMgr &parent,
                       void *applicShmSegPtr, key_t theKey,
                       const vector<oneHeapStats> &iHeapStats,
                       pid_t inferiorPid);

  ~fastInferiorHeapMgr();

   void handleExec();
      // detach from old segment (and delete it); create new segment & attach
      // to it...inferior attached at undefined.  Pretty much like a call to the dtor
      // following by a call to constructor (maybe this should be operator=() for
      // maximum cuteness)

   void registerInferiorAttachedAt(void *iApplicAttachedAt) {
      applicAttachedAt = iApplicAttachedAt;
   }

   key_t getShmKey() const { return theShmKey; }
   unsigned getHeapTotalNumBytes() const;

   void *getObsCostAddrInApplicSpace() {
      if (applicAttachedAt == NULL) {
         assert(false);
         return NULL;
      }

      return (void *)((char*)applicAttachedAt + 12);
   }

   unsigned *getObsCostAddrInParadyndSpace() {
      if (paradyndAttachedAt == NULL) {
         assert(false);
         return NULL;
      }

      return (unsigned *)((char*)paradyndAttachedAt + 12);
   }

   void *getSubHeapInParadynd(unsigned subHeapIndex) const;
   void *getSubHeapInApplic(unsigned subHeapIndex) const;
};

#endif
