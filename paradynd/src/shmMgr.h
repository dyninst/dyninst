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

/* $Id: shmMgr.h,v 1.2 2002/05/03 15:50:04 schendel Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#ifndef shmMgr_h
#define shmMgr_h

#include "common/h/headers.h"
#include "common/h/Types.h"

class ShmSegment;
class process;

class shmMgr {
  key_t keyUsed;
  ShmSegment* theShm;
  unsigned shmSize;
  Address baseAddrInDaemon;
  Address baseAddrInApplic;
  Address highWaterMark;  

 public:
  static const unsigned cookie;

  shmMgr();

  shmMgr(process *proc, key_t shmSegKey, unsigned shmSize_);

  ~shmMgr() {};

  /* Do all the work of creating a new shared memory segment
     and getting the RT lib to hook to it */
  void addShmSegment(Address /*size*/) {assert(0);};

  key_t getShmKey() const { return keyUsed; };
  unsigned getHeapTotalNumBytes() const { return shmSize; }
  unsigned memAllocated() { return highWaterMark; }
  void *getAddressInApplic(void *addressInDaemon) {
    unsigned offset = reinterpret_cast<Address>(addressInDaemon) - 
                      baseAddrInDaemon;
    Address retAddr = baseAddrInApplic + offset;
    return reinterpret_cast<void*>(retAddr);
  }
  void *getObsCostAddrInApplicSpace() { 
    if(baseAddrInApplic == 0)
      return NULL;
    
    return reinterpret_cast<void*>(baseAddrInApplic + 12);
  }
  void *getObsCostAddrInParadyndSpace() { 
    if(baseAddrInApplic == 0)
      return NULL;
    
    return reinterpret_cast<void*>(baseAddrInDaemon + 12);
  }
  void registerInferiorAttachedAt(void *applicAttachedAt) { 
    baseAddrInApplic = reinterpret_cast<Address>(applicAttachedAt);
  }

  Address malloc(unsigned size);
  void handleExec() { }
  // detach from old segment (and delete it); create new segment & attach to
  // it...inferior attached at undefined.  Pretty much like a call to the
  // dtor following by a call to constructor (maybe this should be
  // operator=() for maximum cuteness)


  /* Don't do anything as yet. */
  void free(Address addr);
};




#endif /*shmMgr_h*/
