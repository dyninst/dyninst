/*
 * Copyright (c) 1996-2000 Barton P. Miller
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

// $Id: fastInferiorHeapMgr.C,v 1.9 2000/07/28 17:22:11 pcroth Exp $
// A class that manages several fastInferiorHeaps (fastInferiorHeap.h/.C)
// Formerly, each fastInferiorHeap would create its own shm-segment.
// But this created too many segments.  This class creates a single shm segment,
// containing several fastInferiorHeaps.

#include <iostream.h>
#include "common/h/headers.h"
#include "shmSegment.h"
#include "fastInferiorHeapMgr.h"
#include "dyninstAPI/src/showerror.h"
#include "rtinst/h/rtinst.h" //MAX_NUMBER_OF_THREADS
#include "main.h"

const unsigned fastInferiorHeapMgr::cookie = 0xabcdefab;

static void align(unsigned &num, unsigned alignmentFactor) {
   if (num % alignmentFactor != 0)
      num += alignmentFactor - (num % alignmentFactor);
         // is this right?

   assert(num % alignmentFactor == 0);
}

unsigned fastInferiorHeapMgr::getSubHeapOffset(unsigned subHeapIndex) const {
   // return # bytes offset from shmat()
   // passing subHeapIndex equal to the # of subHeaps (i.e. 1 larger than the
   // last valid index) will return the # of bytes to shmget().

   unsigned result = 0;

   result += sizeof(unsigned); // cookie
   result += 2 * sizeof(int);  // process pid, paradynd pid
   result += sizeof(unsigned); // observed cost low

#if defined(MT_THREAD)
   result += sizeof(RTINSTsharedData) ; //RTsharedData
#endif

   align(result, 8);

   for (unsigned lcv = 0; lcv < subHeapIndex; lcv++) {
      assert(result % 8 == 0);
      result += (theHeapStats[lcv].elemNumBytes * theHeapStats[lcv].maxNumElems);

      align(result, 8); // make sure the next one starts off on a nice, aligned addr.
   }

   assert(result % 8 == 0);

   return result;
}

unsigned fastInferiorHeapMgr::getHeapTotalNumBytes() const {
   return getSubHeapOffset(theHeapStats.size());
}

fastInferiorHeapMgr::fastInferiorHeapMgr(key_t firstKeyToTry,
					 const vector<oneHeapStats> &iHeapStats,
					 pid_t inferiorPid) :
                         theHeapStats(iHeapStats),
                         theShm(NULL) {
   
   const unsigned heapNumBytes = getHeapTotalNumBytes();

   // Try to allocate shm segment now
   key_t key = firstKeyToTry;
   theShm = ShmSegment::Create( key, heapNumBytes );
   if( theShm == NULL )
   {
      // we failed to create the shared memory segment
       return;
   }

   // Now, let's initialize some meta-data: cookie, process pid, paradynd pid,
   // cost.
   unsigned *ptr = (unsigned *)theShm->GetMappedAddress();
   *ptr++ = cookie;
   *ptr++ = (unsigned)inferiorPid;
   *ptr++ = (unsigned)getpid();     // paradynd pid
   *ptr++ = 0;            // initialize observed cost

}


fastInferiorHeapMgr::~fastInferiorHeapMgr( void )
{
    delete theShm;
}


void fastInferiorHeapMgr::handleExec() {
   // detach and delete old shm segment; create new seg & attach to it; leave
   // applic-attached-at undefined.  Much like (non-fork) ctor, really.
   // There's a cute twist: although the exec syscall has caused the appl to detach
   //    from the shm seg, we (paradynd) are still attached to it.  So there's no
   //    need to re-create a brand new shm seg; it still exists, and we reuse it!
   //    Therefore, the shm seg key doesn't change.
   assert( theShm != NULL );

   // theHeapStats doesn't change
   // since we re-use the segment, theShmKey, theShmId, and paradyndAttachedAt
   //    don't change.
   applicAttachedAt = NULL;

   // do we need to change any meta-data in the 1st 16 bytes of the shm seg?
   // Well, the cookie doesn't change.  The inferior-pid doesn't change after an exec.
   // the pid of paradynd doesn't change.  But we should reset the observed cost to
   // zero.

   unsigned *ptr = (unsigned *)theShm->GetMappedAddress();
   ptr++; // skip past cookie
   ptr++; // skip past inferior pid
   ptr++; // skip past paradynd pid
   *ptr++ = 0; // reset observed cost
}

fastInferiorHeapMgr::fastInferiorHeapMgr(const fastInferiorHeapMgr &parent,
					 void *applicShmSegPtr,
					 key_t theKey,
					 const vector<oneHeapStats> &iHeapStats,
					 pid_t inferiorPid) :
                           theHeapStats(iHeapStats) {
   // This is the fork-constructor; the application (DYNINSTfork()) has already
   // created a new shared-mem segment, attached to it, copied data from the
   // old segment, and detached from the old segment.
   //
   // It is tempting to think that we (paradynd) should detach from the old segment
   // and then destroy it.  In fact, we should do neither.  Remember that in a fork,
   // one should trace both the parent and the child.  If we detached and/or destroyed
   // the old segment, the parent would no longer work!  The fork itself incremented the
   // reference count in the application, but not for paradynd (since paradynd didn't
   // fork).
   //
   // We need to (1) shmget to the new already-existing shm segment (it was already
   // created by the child process), (2) attach to it, (3) make a note where the child
   // attached to the new segment [this info is passed in as 'applicShmSegPtr'],
   // (4) check cookie and update pids embedded into 1st 12 bytes.
   //
   // QUESTION: the observed cost is present near the beginning of the shm segment.
   //           On a fork, should we reset it to zero?  If so, do it here!
   //

   // IMPORTANT: We certainly hope that the application has managed to attach to the new
   //            shm segment at the same virtual address as the old segment; for if not,
   //            then all mini-trampolines will refer to data in the wrong location in
   //            the child process!!!  Hence the following check right off the bat:
   if (applicShmSegPtr != parent.applicAttachedAt) {
      cerr << "Serious error in fastInferiorHeapMgr fork-constructor" << endl;
      cerr << "It appears that the child process hasn't attached the shm seg" << endl;
      cerr << "in the same location as in the parent process.  This leaves all" << endl;
      cerr << "mini-tramps data references pointing to invalid memory locs!" << endl;
      abort();
   }

   const unsigned heapNumBytes = getHeapTotalNumBytes();
   
   // open the existing segment
   theShm = ShmSegment::Open( theKey, heapNumBytes );
   if( theShm == NULL )
   {
       // we were unable to open the existing segment
       return;
   }
   applicAttachedAt = applicShmSegPtr;

   // We now need to do a memcpy() to copy all data in the old segment to the
   // new one.
#if defined(SHM_SAMPLING_DEBUG) || defined(FORK_EXEC_DEBUG)
   cerr << "paradynd doing memcpy from old seg to new one" << endl;
   cerr << "dest addr is " << (void*)theShm->GetMappedAddress() << endl;
   cerr << "src  addr is " << (void*)parent.theShm->GetMappedAddress() << endl;
   cerr << "num bytes is " << heapNumBytes << endl;
#endif
   memcpy(theShm->GetMappedAddress(), parent.theShm->GetMappedAddress(), heapNumBytes);


   // Now let's play some games with the meta-data at the start of the segment
   unsigned *ptr = (unsigned *)theShm->GetMappedAddress();

   // The memcpy should have left the cookie unchanged:
   assert(*ptr == cookie);
   ptr++;
   
   // The memcpy should have left the pid as the pid of the child proc, which needs to
   // be updated to the pid of the parent proc.
   assert((pid_t)(*ptr) != inferiorPid);
   *ptr++ = inferiorPid;

   // The memcpy should have left the pid of paradynd unchanged.
   assert((pid_t)(*ptr++) == getpid());

   // Finally, the observed cost in the child process should be 0.
   *ptr = 0;
}



void *fastInferiorHeapMgr::getSubHeapInParadynd(unsigned subHeapIndex) const {
   return getSubHeapInSpace(subHeapIndex, paradynd);
}

void *fastInferiorHeapMgr::getSubHeapInApplic(unsigned subHeapIndex) const {
   return getSubHeapInSpace(subHeapIndex, applic);
}

void *fastInferiorHeapMgr::getSubHeapInSpace(unsigned subHeapIndex,
					     space theSpace) const {

   assert( theShm != NULL );

   void *basePtr = (theSpace == 
                    paradynd ? theShm->GetMappedAddress() : applicAttachedAt);
   if (basePtr == NULL) {
      // we're not ready to perform this operation yet!
      assert(false);
      return NULL;
   }

   unsigned offset = getSubHeapOffset(subHeapIndex);
   return (void*)((char*)basePtr + offset);
}
