
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

// fastInferiorHeapMgr.C
// Ari Tamches
// A class that manages several fastInferiorHeaps (fastInferiorHeap.h/.C)
// Formerly, each fastInferiorHeap would create its own shm-segment.
// But this created too many segments.  This class creates a single shm segment,
// containing several fastInferiorHeaps.

#include <iostream.h>
#include "fastInferiorHeapMgr.h"

unsigned fastInferiorHeapMgr::cookie = 0xabcdefab;

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

fastInferiorHeapMgr::gcResult
fastInferiorHeapMgr::tryToGarbageCollectShmSeg(key_t keyToTry, unsigned size) {
   // Garbage collection strategy:
   // Try to shmget() the segment without the IPC_CREAT or IPC_EXCL flags.
   // If successful, then try to shmctl(IPC_RMID) to fry it.
   // Return true iff garbage collection succeeded.

   int shmid = P_shmget(keyToTry, size, 0666);

   if (shmid == -1 && errno == ENOENT)
      // the shm segment doesn't exist...which means garbage collection
      // vacuously succeeds...nothing to do.
      return noNeed;

   if (shmid == -1) {
      perror("shmget");
      return gcDidntHappen;
   }

   // Okay, we've got a shmid for this key.  Let's get some information on
   // it (such as the owner's uid; if it's not us, then don't try to shmat() to
   // it!)

   shmid_ds shm_buf;
   errno = 0;
   int result = shmctl(shmid, IPC_STAT, &shm_buf);
   if (result == -1) {
      // some possible errors:
      // EACCES -- read access was denied; this shm seg must not belong to us!
      // EINVAL -- invalid shmid (certainly not expected here)
      perror("shmctl IPC_STAT");
      return gcDidntHappen;
   }

   // Some fields of interest:
   // -- shm_cpid gives the pid of the creator
   // -- shm_nattch gives # of current attaches
   // -- shm_perm.cuid gives uid of the creator

   if (shm_buf.shm_perm.cuid != getuid()) {
#ifdef SHM_SAMPLING_DEBUG
      cerr << "note: shm segment key " << keyToTry << " was created by uid "
	   << shm_buf.shm_perm.cuid << ", so we can't touch it" << endl;
#endif
      return gcDidntHappen;
   }

   if (shm_buf.shm_nattch > 0) {
#ifdef SHM_SAMPLING_DEBUG
      cerr << "note: shm segment key " << keyToTry << " has a non-zero attach value ("
           << shm_buf.shm_nattch << "), so we will leave it alone" << endl;
#endif
      return gcDidntHappen;
   }

   if (kill(shm_buf.shm_cpid, 0) == 0) {
#ifdef SHM_SAMPLING_DEBUG
      cerr << "note: the creating process of shm segment key " << keyToTry
	   << " is still running, so we will leave the segment alone." << endl;
#endif
      return gcDidntHappen;
   }

   // Well, if we've gotten this far, then we know:
   // 1) noone is attached to the segment
   // 2) we are the owner, so we should have permission to delete it.
   // So if we get this far, we expect to succeed in garbage collecting
   // this segment.

   void *segPtr = P_shmat(shmid, NULL, 0);
   if (segPtr == (void*)-1) {
      // we can't examine the contents...give up.  No need to undo the effects
      // of shmget(), since we know that the segment already exists so that
      // doing a shmctl(IPC_RMID) would be a serious mistake.
#ifdef SHM_SAMPLING_DEBUG
      cerr << "while trying to garbage collect shm seg key, " << keyToTry << endl;
      perror("shmat");
#endif
      return gcDidntHappen;
   }

   const unsigned *ptr = (unsigned *)segPtr;

   // First, check for cookie.  If no match, then the segment wasn't created
   // by paradynd and we should not touch it!
   if (*ptr++ != cookie) {
#ifdef SHM_SAMPLING_DEBUG
      cerr << "no match on cookie, so leaving shm seg key " << keyToTry << " alone." << endl;
#endif
      (void)P_shmdt(segPtr);
      return gcDidntHappen;
   }

   // Next, check the 2 pids (pid of applic, pid of paradynd)
   int pid1 = (int)(*ptr++);
   int pid2 = (int)(*ptr++);

   errno = 0;
   result = P_kill(pid1, 0);
   if (result == 0)
      // the process definitely exists.  So, don't garbage collect.
      return gcDidntHappen;
   else if (errno != ESRCH) {
      // this is rather unexpected.  Don't know what to do here.
      perror("kill");
      return gcDidntHappen;
   }

   errno = 0;
   result = P_kill(pid2, 0);
   if (result == 0)
      return gcDidntHappen; // process exists; unsafe to gc
   else if (errno != ESRCH) {
      perror("kill");
      return gcDidntHappen; // unexpected
   }

   // Okay, neither process exists.  Looks like we may fry the segment now!
   (void)P_shmdt(segPtr);
   (void)P_shmctl(shmid, IPC_RMID, NULL);

   return gcHappened; // but see above comment...so, is this always right?
}

fastInferiorHeapMgr::fastInferiorHeapMgr(key_t firstKeyToTry,
					 const vector<oneHeapStats> &iHeapStats,
					 pid_t inferiorPid) :
                         theHeapStats(iHeapStats) {
   
   const unsigned heapNumBytes = getHeapTotalNumBytes();
   
   // Try to allocate shm segment now...garbage collecting as we go...
   while (true) {
      errno = 0;
      int shmid = P_shmget(firstKeyToTry, heapNumBytes, 0666 | IPC_CREAT | IPC_EXCL);
      if (shmid == -1) {
	 // failure...but why?  If because seg already exists, then increase
         // 'firstKeyToTry' and retry!  (But first, try to garbage collect it and
	 // if successful, retry without increasing 'firstKeyToTry'!)

	 if (errno == EEXIST) {
	    // The shm segment already exists; try to garbage collect it.
	    // If sucessful, then retry; else, retry after incrementing
	    // firstKeyToTry.
	    const fastInferiorHeapMgr::gcResult result = tryToGarbageCollectShmSeg(firstKeyToTry, heapNumBytes);
	    if (result == gcHappened) {
#ifdef SHM_SAMPLING_DEBUG
	       cerr << "fastInferiorHeapMgr successfully gc'd key " << (int)firstKeyToTry << endl;
#endif
	       continue;
	    }
	    else if (result == gcDidntHappen) {
#ifdef SHM_SAMPLING_DEBUG
	       cerr << "fastInferiorHeapMgr couldn't gc key " << (int)firstKeyToTry << endl;
#endif
	       firstKeyToTry++;
	       continue;
	    }
	    else if (result == noNeed) {
	       cerr << "unexpected noNeed" << endl;
	    }
	    else
	       assert(false); // invalid result code
	 }

	 if (errno == ENOSPC) {
	    // Would have created the shm segment, but the system-imposed limit
	    // on the max number of allowed shm identifiers system-wide would have
	    // been exceeded.  What to do here?  At first glance, there's nothing
	    // we can possibly do.  But we could try to garbage collect some
	    // segments, and if successful with at least one collection, then retry.
	    bool success = false;
	    for (key_t k= firstKeyToTry; k <= firstKeyToTry+20; k++) {
	       if (tryToGarbageCollectShmSeg(k, heapNumBytes)==gcHappened)
		  success = true;
	    }

	    if (success)
	       // cool!  An unexpected piece of good news.
	       continue;

	    // else, fall through to error
	 }

	 perror("shmget");

	 theShmKey = 0;
	 theShmId = 0;
	 paradyndAttachedAt = applicAttachedAt = NULL;

	 return; // should probably throw and exception!
      }

      // shmget succeeded -- the shm segment has been created.  Now we need
      // to attach to it.

      assert(errno == 0);

      theShmKey = firstKeyToTry;
      theShmId = shmid;
      applicAttachedAt = NULL; // for now...

      errno = 0;
      paradyndAttachedAt = P_shmat(shmid, NULL, 0);
      if (paradyndAttachedAt == (void*)-1) {
	 if (errno == EMFILE) {
	    // the # of shm segments mapped to this process would exceed the
	    // system-imposed limit.  Ick, ick, ick.
	    // Would garbage collection help?  Probably not; it might reduce the
	    // # of segments system-wide, but not the # of segments we've shmat()'d
	    // to.  So, give up.  First, undo the shmget.  Then, throw an exception.

	    cerr << "fastInferiorHeapMgr: shmat failed -- number of shm segments attached to paradynd" << endl;
	    cerr << "would exceed a system-imposed limit." << endl;
	    
	    // fall through...
	 }

	 perror("fastInferiorHeapMgr shmat");

	 (void)P_shmctl(shmid, IPC_RMID, NULL);
	    
	 theShmKey = 0;
	 theShmId = 0;
	 paradyndAttachedAt = applicAttachedAt = NULL;

	 return; // throw an exception
      }

      // Okay, the shmget() worked, and the shmat() worked too.
      break; // successful
   } // while (true)


   // Now, let's initialize some meta-data: cookie, process pid, paradynd pid,
   // cost.
   unsigned *ptr = (unsigned *)paradyndAttachedAt;
   *ptr++ = cookie;
   *ptr++ = (unsigned)inferiorPid;
   *ptr++ = (unsigned)getpid();     // paradynd pid
   *ptr++ = 0;            // initialize observed cost

   // We're done, successfully.  Just for fun, let's do some garbage
   // collection of higher-numbered segments...ignoring any errors...

   while (true) {
      bool anySuccesses = false; // so far...

      for (key_t k = theShmKey+1; k <= theShmKey + 10; k++)
	 if (tryToGarbageCollectShmSeg(k, heapNumBytes)==gcHappened)
	    anySuccesses = true;

      if (!anySuccesses)
         break;
   }
}

fastInferiorHeapMgr::fastInferiorHeapMgr(void *applicShmSegPtr,
					 key_t theKey,
					 const vector<oneHeapStats> &iHeapStats,
					 pid_t inferiorPid) :
                           theHeapStats(iHeapStats) {
   // This is the fork-constructor; the application (DYNINSTfork()) has already
   // created a new shared-mem segment, attached to it, copied data from the
   // old segment, and detached from the old segment.

   const unsigned heapNumBytes = getHeapTotalNumBytes();
   
   theShmKey = theKey;

   errno = 0;
   theShmId = P_shmget(theKey, heapNumBytes, 0666);
      // note the flags: no IPC_CREAT or IPC_EXCL --> the shm seg must already exist
   if (theShmId == -1) {
      cerr << "fastInferiorHeapMgr fork: could not shmget to new segment" << endl;
      perror("shmget");
      
      paradyndAttachedAt = applicAttachedAt = NULL;
      return; // throw an exception
   }

   paradyndAttachedAt = P_shmat(theShmId, NULL, 0);
   if (paradyndAttachedAt == (void*)-1) {
      cerr << "fastInferiorHeapMgr fork: could not shmat to new segment" << endl;
      perror("shmat");

      paradyndAttachedAt = applicAttachedAt = NULL;
      return; // throw an exception
   }

   applicAttachedAt = applicShmSegPtr;

   // Now let's play some games with the meta-data at the start of the segment
   unsigned *ptr = (unsigned *)paradyndAttachedAt;

   // The memcpy done by the applic should leave the cookie unchanged
   assert(*ptr == cookie);
   ptr++;
   
   // The memcpy done by the applic should leave the pid as the pid of the
   // child, which is now out of date.  Update it to the pid of the parent.
   assert((pid_t)(*ptr) != inferiorPid);
   *ptr++ = inferiorPid;

   // The memcpy done by the applic should leave the pid of paradynd unchanged.
   assert((pid_t)(*ptr) == getpid());
}

fastInferiorHeapMgr::~fastInferiorHeapMgr() {
   // Detach from shm segment
   if (P_shmdt(paradyndAttachedAt) == -1)
      perror("fastInferiorHeapMgr dtor shmdt");

   // Destroy the shm segment
   if (P_shmctl(theShmId, IPC_RMID, 0) == -1)
      perror("fastInferiorHeapMgr dtor shmctl IPC_RMID");
}

void *fastInferiorHeapMgr::getSubHeapInParadynd(unsigned subHeapIndex) const {
   return getSubHeapInSpace(subHeapIndex, paradynd);
}

void *fastInferiorHeapMgr::getSubHeapInApplic(unsigned subHeapIndex) const {
   return getSubHeapInSpace(subHeapIndex, applic);
}

void *fastInferiorHeapMgr::getSubHeapInSpace(unsigned subHeapIndex,
					     space theSpace) const {

   void *basePtr = (theSpace == paradynd ? paradyndAttachedAt : applicAttachedAt);
   if (basePtr == NULL) {
      // we're not ready to perform this operation yet!
      assert(false);
      return NULL;
   }

   unsigned offset = getSubHeapOffset(subHeapIndex);
   return (void*)((char*)basePtr + offset);
}
