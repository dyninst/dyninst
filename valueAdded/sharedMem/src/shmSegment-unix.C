/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
//----------------------------------------------------------------------------
// $Id: shmSegment-unix.C,v 1.1 2006/11/22 21:45:02 bernat Exp $
//----------------------------------------------------------------------------
//
// Definition of the ShmSegment class.
// A ShmSegment object represents a shared memory segment, providing an
// OS-independent interface to management of shared memory segments to 
// paradynd.
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include "shmSegment.h"
#include "../h/SharedMem.h"
#include "sharedMemInternal.h"


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
    assert(baseAddrInDaemon != (Address) -1);
    // unmap the segment from our address space
    if( shmdt( (void *)baseAddrInDaemon ) == -1 )
    {
        perror( "ShmSegment dtor shmdt" );
    }
    baseAddrInDaemon = (Address) -1;

    if(! leaveSegmentAroundOnExit) {
       // destroy the segment
       if( shmctl( seg_id, IPC_RMID, 0 ) == -1 )
       {
          perror( "ShmSegment dtor shmctl" );
       }
       seg_id = -1;
       baseAddrInApplic = (Address) -1;
    }
}

//
// ShmSegment::Create
//
// Access a shared memory segment whose name is based on the value
// of the given key, of at least the indicated size, and mapped into
// our address space at the indicated address (if specified).
// 
ShmSegment*
ShmSegment::Create( key_t& key, unsigned int size, bool freeWhenDeleted )
{
    ShmSegment* seg = NULL;
    

    // create a shared memory segment of the desired size
    while (true) {
      errno = 0;

      shmid_t shmid = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);
      if (shmid == -1) {
         // failure...but why?  If because seg already exists, then increase
         // 'firstKeyToTry' and retry!  (But first, try to garbage collect it
         // and if successful, retry without increasing 'firstKeyToTry'!)

          if (errno == EINVAL) {
              perror("Failed to create shared memory segment");
              return NULL;
          }

	     if (errno == EEXIST) {
	        // The shm segment already exists; try to garbage collect it.
	        // If sucessful, then retry; else, retry after incrementing
	        // firstKeyToTry.
	        gcResult result = TryToReleaseShmSegment(key, size);

	        if (result == gcHappened) {
	           continue;
	        }
	        else if (result == gcDidntHappen) {
	           key++;
	           continue;
	        }
	        else if (result == noNeed) {

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

      void* mapAddr = shmat(shmid, NULL, 0);
      if (mapAddr == (void*)-1) {
	     if (errno == EMFILE) {
	        // the # of shm segments mapped to this process would exceed the
	        // system-imposed limit.  Ick, ick, ick.
	        // Would garbage collection help?  Probably not; it might reduce
	        // #segments system-wide, but not #segments we've shmat()'d to.
	        // So, give up: undo the shmget, then throw an exception.
                 perror("Failed to map shared segment");
                 
                 return NULL;
             }
             
	     perror("ShmSegment::Create shmat");
             
             // release the segment we created
	     (void)shmctl(shmid, IPC_RMID, NULL);
	     break; // throw an exception
      }

      // Okay, the shmget() worked, and the shmat() worked too.
      // build an object to represent the segment
      seg = new ShmSegment( shmid, key, size, (Address) mapAddr );
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

    if (!freeWhenDeleted)
        seg->markAsLeaveSegmentAroundOnExit();

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
    shmid_t shmid = shmget(key, size, 0666);
    if( shmid != -1 )
    {
        // map the shared memory segment into our address space
        void* mapAddr = shmat(shmid, addr, 0);
        if( mapAddr != (void*)-1 )
        {
            // we were able to access and map the shared memory
            // build an object to represent it
            seg = new ShmSegment( shmid, key, size, (Address) mapAddr );
        }
        else
        {
            perror("shmat");
        }
    }
    else
    {
        perror("shmget");
    }

    return seg;
}

// sameAddr: if true, attempt to attach at the same address and fail
// if different

ShmSegment *
ShmSegment::Copy(BPatch_process *child_thr, bool sameAddr) {
    // Now, create a new segment of the same size as this one,
    // and attach the child in the same place
    key_t key = GetKey();
    ShmSegment *child_seg = ShmSegment::Create(key, GetSize(), !leaveSegmentAroundOnExit);

    Address storedBaseAddr = baseAddrInApplic;
    // First, detach the shared segment from the child
    if (!detach(child_thr))
        return NULL;
    // detach zeroes out our base address in the application, which is undesireable
    // in this case. Fix that.
    baseAddrInApplic = storedBaseAddr;

    if (!child_seg)
        return NULL;
    if (!child_seg->attach(child_thr, (sameAddr) ? baseAddrInApplic : 0))
        return NULL;
    
    // Copy contents from this segment to the new one
    memcpy((void *)child_seg->baseAddrInDaemon, (void *)baseAddrInDaemon, GetSize());
    child_seg->freespace = freespace;
    child_seg->highWaterMark = highWaterMark;
    child_seg->leaveSegmentAroundOnExit = leaveSegmentAroundOnExit;

    // Return the new shared segment, attached and ready to go.
    return child_seg;
}

static
gcResult
TryToReleaseShmSegment( key_t keyToTry, unsigned int size )
{
   // Garbage collection strategy:
   // Try to shmget() the segment without the IPC_CREAT or IPC_EXCL flags.
   // If successful, then try to shmctl(IPC_RMID) to fry it.
   // Return true iff garbage collection succeeded.

   shmid_t shmid = shmget(keyToTry, size, 0666);

   if (shmid == -1 && errno == ENOENT)
      // the shm segment doesn't exist...which means garbage collection
      // vacuously succeeds...nothing to do.
      return noNeed;

   if (shmid == -1) {
      perror("tryToReleaseShmSegment");
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
      return gcDidntHappen;
   }

   if (shm_buf.shm_nattch > 0) {
      return gcDidntHappen;
   }

   if (kill(shm_buf.shm_cpid, 0) == 0) {
      return gcDidntHappen;
   }

   // Well, if we've gotten this far, then we know:
   // 1) noone is attached to the segment
   // 2) we are the owner, so we should have permission to delete it.
   // So if we get this far, we expect to succeed in garbage collecting
   // this segment.

   void *segPtr = shmat(shmid, NULL, 0);
   if (segPtr == (void*)-1) {
      // we can't examine the contents...give up.  No need to undo the effects
      // of shmget(), since we know that the segment already exists so that
      // doing a shmctl(IPC_RMID) would be a serious mistake.
      return gcDidntHappen;
   }

   const unsigned *ptr = (unsigned *)segPtr;

   // First, check for cookie.  If no match, then the segment wasn't created
   // by paradynd and we should not touch it!
   if (*ptr != ShmSegment::cookie) {
      (void)shmdt(segPtr);
      return gcDidntHappen;
   }

#if 0
   // Next, check the 2 pids (pid of applic, pid of paradynd)
   int pid1 = (int)(*ptr++);
   int pid2 = (int)(*ptr++);

   errno = 0;
   result = kill(pid1, 0);
   if (result == 0)
      // the process definitely exists.  So, don't garbage collect.
      return gcDidntHappen;
   else if (errno != ESRCH) {
      // this is rather unexpected.  Don't know what to do here.
      perror("kill");
      return gcDidntHappen;
   }

   errno = 0;
   result = kill(pid2, 0);
   if (result == 0)
      return gcDidntHappen; // process exists; unsafe to gc
   else if (errno != ESRCH) {
      perror("kill");
      return gcDidntHappen; // unexpected
   }
#endif
   // Okay, neither process exists.  Looks like we may fry the segment now!
   (void)shmdt(segPtr);
   (void)shmctl(shmid, IPC_RMID, NULL);

   return gcHappened; // but see above comment...so, is this always right?
}

