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
//----------------------------------------------------------------------------
// $Id: shmSegment-unix.C,v 1.4 2000/07/28 17:22:33 pcroth Exp $
//----------------------------------------------------------------------------
//
// Definition of the ShmSegment class.
// A ShmSegment object represents a shared memory segment, providing an
// OS-independent interface to management of shared memory segments to 
// paradynd.
//
//----------------------------------------------------------------------------
#include "common/h/headers.h"
#include "dyninstAPI/src/showerror.h"
#include "shmSegment.h"
#include "main.h"



// enumeration for return values from TryToReleaseShmSegment function,
// describing status of its operation
enum gcResult {noNeed, gcHappened, gcDidntHappen};


//----------------------------------------------------------------------------
// prototypes of functions used in this file
//----------------------------------------------------------------------------
static gcResult TryToReleaseShmSegment( key_t, unsigned int size );



//
// ShmSegment::~ShmSegment
//
// Destory a shared memory segment
//
ShmSegment::~ShmSegment( void )
{
    assert( seg_addr != NULL );

    // unmap the segment from our address space
    if( P_shmdt( seg_addr ) == -1 )
    {
        perror( "ShmSegment dtor shmdt" );
    }
    seg_addr = NULL;

    // destroy the segment
    if( P_shmctl( seg_id, IPC_RMID, 0 ) == -1 )
    {
        perror( "ShmSegment dtor shmctl" );
    }
    seg_id = -1;
}



//
// ShmSegment::Create
//
// Access a shared memory segment whose name is based on the value
// of the given key, of at least the indicated size, and mapped into
// our address space at the indicated address (if specified).
// 
ShmSegment*
ShmSegment::Create( key_t& key, unsigned int size, void* /*addr*/ )
{
    ShmSegment* seg = NULL;
    

    // create a shared memory segment of the desired size
    while (true) {
      errno = 0;

      shmid_t shmid = P_shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);
      if (shmid == -1) {
         // failure...but why?  If because seg already exists, then increase
         // 'firstKeyToTry' and retry!  (But first, try to garbage collect it
         // and if successful, retry without increasing 'firstKeyToTry'!)

             if (errno == EINVAL) {
                cerr << "ShmSegment::Create failed for segment of size "
                     << size << " bytes, due to system-imposed limits." << endl;
                string msg = "Size of shared memory segment requested mapped"
                    " to this process (" + string(size) + ") exceeds OS limit.";
	        showErrorCallback(102,msg);
	        cleanUpAndExit(-1);
             }

	     if (errno == EEXIST) {
	        // The shm segment already exists; try to garbage collect it.
	        // If sucessful, then retry; else, retry after incrementing
	        // firstKeyToTry.
	        gcResult result = TryToReleaseShmSegment(key, size);
	        if (result == gcHappened) {
#ifdef SHM_SAMPLING_DEBUG
	           cerr << "ShmSegment::Create successfully gc'd key " 
                        << (int)key << endl;
#endif
	           continue;
	        }
	        else if (result == gcDidntHappen) {
#ifdef SHM_SAMPLING_DEBUG
	           cerr << "ShmSegment::Create couldn't gc key " 
                        << (int)key << endl;
#endif
	           key++;
	           continue;
	        }
	        else if (result == noNeed) {
	           cerr << "unexpected noNeed" << endl;
	        }
	        else
	           assert(false); // invalid result code
	     }

	     if (errno == ENOSPC) {
                // Would have created the shm segment, but the system-imposed
                // limit on the max number of allowed shm identifiers [shmmni?]
                // system-wide would have been exceeded.  What to do here? 
                // At first glance, there's nothing we can possibly do. 
                // But we could try to garbage collect some segments, and
                // if successful with at least one collection, then retry.
	        bool success = false;
	        for (key_t k= key; k <= key+20; k++) {
	           if (TryToReleaseShmSegment(k, size)==gcHappened)
		            success = true;
	        }

	        if (success)
	           // cool!  An unexpected piece of good news.
	           continue;

	        // else, fall through to error
	     }

             perror("ShmSegment::Create shmget");

	     break; // should probably throw an exception!
      }

      // shmget succeeded -- the shm segment has been created.  Now we need
      // to attach to it.
      assert(errno == 0);

      void* mapAddr = P_shmat(shmid, NULL, 0);
      if (mapAddr == (void*)-1) {
	     if (errno == EMFILE) {
	        // the # of shm segments mapped to this process would exceed the
	        // system-imposed limit.  Ick, ick, ick.
	        // Would garbage collection help?  Probably not; it might reduce
	        // #segments system-wide, but not #segments we've shmat()'d to.
	        // So, give up: undo the shmget, then throw an exception.

	        cerr << "ShmSegment::Create: shmat failed -- "
                     << "number of shm segments attached to paradynd" << endl;
	        cerr << "would exceed a system-imposed limit." << endl;
	        
	        showErrorCallback(102,"Number of shared memory segments"
                        " mapped to this process exceeds OS limit.");
	        cleanUpAndExit(-1);
	     }

	     perror("ShmSegment::Create shmat");

             // release the segment we created
	     (void)P_shmctl(shmid, IPC_RMID, NULL);
	     break; // throw an exception
      }

      // Okay, the shmget() worked, and the shmat() worked too.
      // build an object to represent the segment
      seg = new ShmSegment( shmid, key, size, mapAddr );
      break;

    } // while (true)


    if( seg != NULL )
    {
        // we successfully created and mapped a shared memory segment
        // of the desired size
        //
        // attempt to garbage-collect segments we find at nearby
        // key values
        while( true )
        {
            bool anySuccesses = false; // so far...

            for( key_t k = key+1; k <= key + 10; k++ )
            {
                if( TryToReleaseShmSegment( k, size ) == gcHappened )
                {
                    anySuccesses = true;
                }
            }

            if( !anySuccesses )
            {
                // we didn't find any segments to release
                break;
            }
        }
    }

    return seg;
}


//
// ShmSegment::Open
//
// Open and attach to an existing shared memory segment, whose name
// is based on the given key, of at least the indicated size, and 
// map it into our address space at the indcated address (if given).
//
ShmSegment*
ShmSegment::Open( key_t key, unsigned int size, void* addr )
{
    ShmSegment* seg = NULL;


    // access an existing shared memory segment with the given key
    errno = 0;
    shmid_t shmid = P_shmget(key, size, 0666);
    if( shmid != -1 )
    {
        // map the shared memory segment into our address space
        void* mapAddr = P_shmat(shmid, addr, 0);
        if( mapAddr != (void*)-1 )
        {
            // we were able to access and map the shared memory
            // build an object to represent it
            seg = new ShmSegment( shmid, key, size, mapAddr );
        }
        else
        {
            cerr << "ShmSegment::Open: could not shmat existing segment" << endl;
            perror("shmat");
        }
    }
    else
    {
        cerr << "ShmSegment::Open: could not shmget existing segment" << endl;
        perror("shmget");
    }

    return seg;
}



static
gcResult
TryToReleaseShmSegment( key_t keyToTry, unsigned int size )
{
   // Garbage collection strategy:
   // Try to shmget() the segment without the IPC_CREAT or IPC_EXCL flags.
   // If successful, then try to shmctl(IPC_RMID) to fry it.
   // Return true iff garbage collection succeeded.

   shmid_t shmid = P_shmget(keyToTry, size, 0666);

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
   int result = P_shmctl(shmid, IPC_STAT, &shm_buf);
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
   if (*ptr++ != fastInferiorHeapMgr::cookie) {
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

