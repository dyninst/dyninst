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

/* $Id: shmMgr.h,v 1.1 2006/11/22 21:45:00 bernat Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#ifndef shmMgr_h
#define shmMgr_h

// these defaults can be overidden by "SHARED_MUTATEE_LIB" env variable
#if defined(os_windows)
#define SHARED_MUTATEE_LIB "libdynShmMutatee.dll"
#else
#define SHARED_MUTATEE_LIB "libdynShmMutatee.so"
#endif

class BPatch_process;
class ShmSegment;
class shmMgrPreallocInternal;
class BPatch_type;
class BPatch_variableExpr;

#include      <sys/types.h>
#include      <sys/shm.h>
#include "BPatch_Vector.h"


class shMallocHandle {
    friend class shmMgr;
 private:
    void * addrInMutator_;
    BPatch_variableExpr *expr_;
    shMallocHandle(void *a, BPatch_variableExpr *e) : addrInMutator_(a), expr_(e) {};
 public:
    void *addressInMutatee() const;
    void *addressInMutator() const { return addrInMutator_; }
    BPatch_variableExpr *expr() const { return expr_; }
    ~shMallocHandle();
};

class shmMgr {
 private:
    key_t nextKeyToTry;
    unsigned shmSegSize;
    
    BPatch_Vector<ShmSegment *> theShms;
    unsigned totalShmSize;
    unsigned freespace;

    BPatch_process *app;

    bool freeWhenDeleted;

    void * applicToDaemon(void * addr) const;
    void * daemonToApplic(void * addr) const;
    void * getOffsetDaemon(void * addr) const;
    void * getOffsetApplic(void * addr) const;

    /* Do all the work of creating a new shared memory segment
       and getting the RT lib to hook to it */
    void addShmSegment(void * /*size*/);

    shmMgr(BPatch_process *proc, key_t shmSegKey, unsigned shmSize_, bool freeWhenDel = true);
    // Fork constructor
    shmMgr(const shmMgr *par, BPatch_process *child_proc);
    shmMgr() {};

    bool initialize();

    void *malloc_int(unsigned size, bool align = false);

    void * translateFromParentDaemon(void * addr, const shmMgr *parent);
    void * translateFromParentApplic(void * addr, const shmMgr *parent);
    
  public:
    
    // Create a new shared memory manager and return it. 
    // proc: the associated BPatch process
    // shmSegKey: the shared memory key
    // defaultSegmentSize: the initial size for segments (multiple segments are supported)
    // automaticFree: delete shared memory when this object is freed?
    static shmMgr *createSharedManager(BPatch_process *proc,
                                       key_t shmSegKey,
                                       unsigned defaultSegmentSize,
                                       bool automaticFree);
    ~shmMgr();
    
    // Malloc and free equivalents
    shMallocHandle *malloc(unsigned size, bool align = true, BPatch_type *type = NULL);
    void free(shMallocHandle *handle);
    
    // CALL ON EXEC
    // TODO: if Dyninst ever supports chaining of handlers,
    // then register this explicitly.
    //void handleExec();
    // Not implemented yet.

    // Call on fork to get a shmMgr for the child process.
    shmMgr *handleFork(BPatch_process *childProc);    
    
    shMallocHandle *getChildHandle(shMallocHandle *parentHandle);

};




#endif /*shmMgr_h*/
