/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: shmMgr.C,v 1.3 2002/05/22 19:03:24 bernat Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#include <iostream.h>
#include "shmMgr.h"
#include "shmSegment.h"

const unsigned shmMgr::cookie = 0xabcdefab;

shmMgr::shmMgr()
{
}

shmMgr::shmMgr(process *proc, key_t shmSegKey, unsigned shmSize_) :
  shmSize(shmSize_), baseAddrInDaemon(0), baseAddrInApplic(0),
  highWaterMark(0)
{
  // Try to allocate shm segment now
  key_t key = shmSegKey;

  theShm = ShmSegment::Create( key, shmSize);
  if( theShm == NULL )
  {
    cerr << "  we failed to create the shared memory segment\n";
    return;
  }
  keyUsed = key;

  // Now, let's initialize some meta-data: cookie, process pid, paradynd pid,
  // cost.
  baseAddrInDaemon = reinterpret_cast<Address>(theShm->GetMappedAddress());
  unsigned *ptr = reinterpret_cast<unsigned *>(baseAddrInDaemon);
  *ptr++ = cookie;
  *ptr++ = (unsigned)proc;
  *ptr++ = (unsigned)getpid();     // paradynd pid
  *ptr++ = 0;            // initialize observed cost
#if defined(MT_THREAD)
  // HACK, FIX!
  ptr += 4096;
#endif
  Address endAddr = reinterpret_cast<Address>(ptr);
  highWaterMark +=  endAddr - baseAddrInDaemon;
  
}

static unsigned align(unsigned num, unsigned alignmentFactor) {
  unsigned retnum;
  if (num % alignmentFactor != 0)
    retnum = num - (alignmentFactor - (num % alignmentFactor));

  assert(retnum % alignmentFactor == 0);
  return retnum;
}

Address shmMgr::malloc(unsigned size) {
  Address retAddr = 0;
  if ((highWaterMark+size) <= shmSize) {
    retAddr = baseAddrInDaemon + highWaterMark;
    highWaterMark += size;
  } else {
    cerr << "Not enough space to allocate memory chunk of size " << size 
	 << "\n";
  }
  return retAddr;
}

void shmMgr::free(Address /* addr */) {
  // not implemented yet
}
 

    

