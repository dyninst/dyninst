/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: shmMgr.h,v 1.14 2004/07/15 16:52:32 gquinn Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#ifndef shmMgr_h
#define shmMgr_h

#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

// these defaults can be overidden by "SHARED_MUTATEE_LIB" env variable
#if defined(os_windows)
#define SHARED_MUTATEE_LIB "libsharedMutatee.dll"
#else
#define SHARED_MUTATEE_LIB "libsharedMutatee.so.1"
#endif

class BPatch_thread;
class ShmSegment;

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

  // don't use the default one
  shmMgrPreallocInternal(const shmMgrPreallocInternal &);

 public:
  shmMgrPreallocInternal(unsigned size, unsigned num, Address baseAddr);
  ~shmMgrPreallocInternal();
  shmMgrPreallocInternal(const shmMgrPreallocInternal *par,
                         Address newBaseAddr);

  Address malloc();
  void free(Address addr);
  bool oneAvailable();
};


class shmMgr {
    key_t nextKeyToTry;
    unsigned shmSegSize;
    
    pdvector<ShmSegment *> theShms;
    unsigned totalShmSize;
    unsigned freespace;
    pdvector<shmMgrPreallocInternal *> prealloc;

    BPatch_thread *app_thread;

    bool freeWhenDeleted;
    
  public:
    
    shmMgr();
    shmMgr(BPatch_thread *thr, key_t shmSegKey, unsigned shmSize_, bool freeWhenDel = true);
    shmMgr(const shmMgr *par, BPatch_thread *child_thr, bool sameAddress);
    ~shmMgr();

    bool initialize();
    
    // Tell the manager to preallocate a chunk of space
    // Can be called repeatedly to allocate multiple sizes
    bool preMalloc(unsigned size, unsigned num);
    
    /* Do all the work of creating a new shared memory segment
       and getting the RT lib to hook to it */
    void addShmSegment(Address /*size*/);
    
    Address malloc(unsigned size, bool align = true);
    void free(Address addr);
    
    // detach from old segment (and delete it); create new segment & attach to
    // it...inferior attached at undefined.  Pretty much like a call to the
    // dtor following by a call to constructor (maybe this should be
    // operator=() for maximum cuteness)
    void handleExec();

    Address applicToDaemon(Address addr) const;
    Address daemonToApplic(Address addr) const;
    Address getOffsetDaemon(Address addr) const;
    Address getOffsetApplic(Address addr) const;

    Address translateFromParentDaemon(Address addr, const shmMgr *parent);
    Address translateFromParentApplic(Address addr, const shmMgr *parent);
    
};




#endif /*shmMgr_h*/
