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

/* $Id: shmMgr.h,v 1.5 2002/07/11 19:45:45 bernat Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#ifndef shmMgr_h
#define shmMgr_h

#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

class ShmSegment;
class process;

class shmMgrPreallocInternal
{
  friend class shmMgr;
 private:
  Address baseAddr_; // Where the preallocated segment starts
  unsigned size_; // size of each element
  unsigned numElems_; // Number of elements
  char *bitmap_;
  unsigned bitmap_size_;
  unsigned currAlloc_;

 public:
  shmMgrPreallocInternal(unsigned size, unsigned num, Address baseAddr);
  ~shmMgrPreallocInternal();

  Address malloc();
  void free(Address addr);
  bool oneAvailable();
};


class shmMgr {

  key_t keyUsed;
  ShmSegment* theShm;
  unsigned shmSize;
  unsigned freespace;
  Address baseAddrInDaemon;
  Address baseAddrInApplic;
  Address highWaterMark;
  vector<shmMgrPreallocInternal *> prealloc;
  unsigned num_allocated; // Allocated tracker

 public:
  static const unsigned cookie;

  shmMgr();
  shmMgr(process *proc, key_t shmSegKey, unsigned shmSize_);
  ~shmMgr();

  // Tell the manager to preallocate a chunk of space
  // Can be called repeatedly to allocate multiple sizes
  void preMalloc(unsigned size, unsigned num);

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

  void *applicAddrToDaemon(void *addressInApplic) {
    unsigned offset = reinterpret_cast<Address>(addressInApplic) - baseAddrInApplic;
    Address retAddr = baseAddrInDaemon + offset;
    return reinterpret_cast<void *>(retAddr);
  }

  void *getBaseAddrInDaemon() {
    return (void *)baseAddrInDaemon;
  }

  /* Reserve a set piece of space */
  void malloc(Address base, Address size)
  {
    if (highWaterMark < base+size) {
      freespace -= (base+size)-highWaterMark;
      highWaterMark = base+size;
    }
  }

  void registerInferiorAttachedAt(void *applicAttachedAt) { 
    baseAddrInApplic = reinterpret_cast<Address>(applicAttachedAt);
  }

  Address malloc(unsigned size);
  void free(Address addr);

  void handleExec() { }
  // detach from old segment (and delete it); create new segment & attach to
  // it...inferior attached at undefined.  Pretty much like a call to the
  // dtor following by a call to constructor (maybe this should be
  // operator=() for maximum cuteness)
};




#endif /*shmMgr_h*/
