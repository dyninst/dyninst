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
fastInferiorHeap(process *iInferiorProcess,
		 const key_t &iShmKey,
		 unsigned heapNumElems) :
	      inferiorProcess(iInferiorProcess),
              statemap(heapNumElems), houseKeeping(heapNumElems) {
   assert(heapNumElems > 0);

   // Create our side of the shared memory segment.
   // Don't forget the 16 bytes of meta-data
   const unsigned sharedMemSegNumBytes = 16 + sizeof(RAW) * heapNumElems;
   this->theShmKey = iShmKey;
   shmid = P_shmget(theShmKey, sharedMemSegNumBytes, 0666 | IPC_CREAT);
      // should we also add IPC_EXCL to the flags???
   if (shmid == -1) {
      perror("fastInferiorHeap: shmget()");
      exit(5);
   }

   baseAddrInApplic = NULL; // until setBaseAddrInApplic() gets called...

   trueBaseAddrInParadynd = P_shmat(shmid,
				    NULL, // let OS choose where in our virtual addr
				          // space to map the shared-mem segment
				    0);
   if (trueBaseAddrInParadynd == (void*)-1) {
      perror("fastInferiorHeap: shmat()");
      if (errno == EMFILE)
	 cerr << "Perhaps the # of shm segments attached to paradynd would exceed the system-imposed limit? (EMFILE)" << endl;
      else if (errno == ENOMEM)
	 cerr << "Not enough memory? (ENOMEM)" << endl;

      cerr << "paradynd (pid " << getpid() << ") for process pid="
	   << iInferiorProcess->getPid() << ": failed to attach to shm segment!  Exiting paradynd now..." << endl;
      exit(5);
   }

   baseAddrInParadynd = (RAW *)((unsigned char *)trueBaseAddrInParadynd + 16);
      // account for 16 bytes of meta-data at the start of the segment

   const unsigned cookie = 0xabcdefab;
   memcpy(trueBaseAddrInParadynd, &cookie, sizeof(cookie));

   int temp_pid = inferiorProcess->getPid();
   memcpy((unsigned char *)trueBaseAddrInParadynd + sizeof(cookie),
	  &temp_pid, sizeof(temp_pid));
   
   temp_pid=getpid();
   memcpy((unsigned char *)trueBaseAddrInParadynd + sizeof(cookie) + sizeof(temp_pid),
	  &temp_pid, sizeof(temp_pid));

   // Initialize statemap by setting all entries to state 'free'
   for (unsigned ndx=0; ndx < statemap.size(); ndx++)
      statemap[ndx] = free;

   firstFreeIndex = 0;

   // Note: houseKeeping[] values are uninitialized; each call to alloc() initializes
   //       one HK (here) and one RAW (in the inferior heap).
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::fastInferiorHeap(const fastInferiorHeap<HK, RAW> &parent,
					    process *newProc, key_t iShmKey,
					    void *appl_attachedAt) :
                             inferiorProcess(newProc),
			     statemap(parent.statemap), // copy statemap
			     houseKeeping(parent.houseKeeping) // copy hk
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after a process forks.
   // Here is what has already been done:
   // 1) the inferior has chosen a key, created 3 shm segments, copied data
   //    from the previous shm segments, and detached from the old shm segments.

   // So we need to:
   // 1) attach to the new shm segments
   // 2) make note of where the inferior has attached [pass in as appl_attachedAt]
   //    i.e. call setBaseAddrInApplic now.

   const unsigned sharedMemSegNumBytes = 16 + sizeof(RAW) * statemap.size();
      // don't forget the 16 bytes of meta-data
   theShmKey = iShmKey;
   shmid = P_shmget(theShmKey, sharedMemSegNumBytes, 0666);
      // no create and no excl flags --> the shm segment must already exist (as it should be)
   if (shmid == -1) {
      perror("fastInferiorHeap fork: shmget()");
      exit(5);
   }

   baseAddrInApplic = NULL;
   setBaseAddrInApplic((RAW*)((char*)appl_attachedAt + 16));

   trueBaseAddrInParadynd = P_shmat(shmid,
				    NULL, // let OS choose where to put
				    0);
   if (trueBaseAddrInParadynd == (void*)-1) {
      perror("fastInferiorHeap fork: shmat()");
      exit(5);
   }

   baseAddrInParadynd = (RAW *)((unsigned char *)trueBaseAddrInParadynd + 16);
      // account for 16 bytes of meta-data at the start of the segment

   // The cookie should be unchanged; verify.
   const unsigned cookie = 0xabcdefab;
   unsigned actual_cookie_contents;
   memcpy(&actual_cookie_contents, trueBaseAddrInParadynd, sizeof(cookie));
   assert(actual_cookie_contents = cookie);

   // The pid is now out of date (will contain the pid of the parent process; we change
   // it now to the pid of the child process).
   // First, verify that the pid is that of the parent
   int actual_pid_contents;
   memcpy(&actual_pid_contents, (char*)trueBaseAddrInParadynd + sizeof(cookie), sizeof(int));
   assert(actual_pid_contents == newProc->getParent()->getPid());

   // Change the pid to that of the child:
   int temp_pid = inferiorProcess->getPid();
   memcpy((unsigned char *)trueBaseAddrInParadynd + sizeof(cookie),
	  &temp_pid, sizeof(temp_pid));

   // The pid of paradynd should remain unchanged
   unsigned actual_paradyndpid_contents;
   memcpy(&actual_paradyndpid_contents,
	  (char*)trueBaseAddrInParadynd + sizeof(cookie) + sizeof(int),
	  sizeof(int));
   assert(actual_paradyndpid_contents == getpid());

//   temp_pid=getpid();
//   memcpy((unsigned char *)trueBaseAddrInParadynd + sizeof(cookie) + sizeof(temp_pid),
//	  &temp_pid, sizeof(temp_pid));
   
   firstFreeIndex = parent.firstFreeIndex;

   // NOTE: This routine doesn't disconnect from the old segment for
   // a very good reason: we (paradynd) don't want to disconnect, because
   // we're now tracking both the parent and new process.  It's the
   // process itself (rtinst) that needs to disconnect from the old and
   // attach to the new!
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::~fastInferiorHeap() {
   // destructor for statemap[] and houseKeeping[] is called automatically

   // Detach from the shared-memory inferior heap...
   if (P_shmdt(trueBaseAddrInParadynd) == -1)
      perror("fastInferiorHeap dtor could not shmdt()");
      // let's not make this a fatal error; no "exit()" or abort()

   // ...and destroy the shared-mem seg:
   if (P_shmctl(shmid, IPC_RMID, 0) == -1)
      perror("fastInferiorHeap dtor could not shmctl(IPC_RMID)");
      // let's not make this a fatal error; no "exit()" or abort()
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
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::processAll(unsigned long long theWallTime,
					   unsigned long long theProcTime) {
   // theWallTime and theProcTime passed in should be in microsecs.

   vector<unsigned> indexesLeftToSample(statemap.size());
   unsigned numIndexesLeftToSample=0; // so far
   for (unsigned index=0; index < statemap.size(); index++)
      if (statemap[index] == allocated)
         indexesLeftToSample[numIndexesLeftToSample++] = index;

   while (numIndexesLeftToSample > 0)
      processAll(indexesLeftToSample, numIndexesLeftToSample,
		 theWallTime, theProcTime);
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::processAll(vector<unsigned> &indexesLeftToSample,
					   unsigned &numIndexesLeftToSample,
					   unsigned long long theWallTime,
					   unsigned long long theVirtualTime) {
   unsigned lcv=0;
   while (lcv < numIndexesLeftToSample) {
      const unsigned index = indexesLeftToSample[lcv];
      assert(statemap[index] == allocated);

      // What about synchronization here?  baseAddrInParadynd + index refers to
      // an object in shared memory; we have not paused the process; so the potential
      // for race conditions is rather obvious.  We'll let HK::process() handle
      // synchronization.  It should return true if it was able to grab a lock without
      // waiting and do the processing; otherwise it returns false, and we'll try again
      // later, under the assumption that the lock will probably become free by then.
      // In other words, we're assuming the locks are held for a very short time.

      if (!houseKeeping[index].perform(*(baseAddrInParadynd + index), // ptr arith
				       theWallTime, theVirtualTime))
         // this item must remain on "indexesLeftToSample"; move to next candidate
	 lcv++;
      else
         // We've successfully processed this index; remove it from "indexesLeftToSample"
	 // by swapping [lcv] with [last], while intentionally _not_ incrementing lcv.
         // Also reduce numIndexesLeftToSample by 1.
	 indexesLeftToSample[lcv] = indexesLeftToSample[--numIndexesLeftToSample];
   }

   // We've gone thru the entire list of things to process.
   // If numIndexesLeftToSample is > 0 then we haven't been able to process everything...
}
