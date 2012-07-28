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

#include "dyntypes.h"

#include      <sys/types.h>
#include      <sys/shm.h>
#include "BPatch_Vector.h"


class shMallocHandle {
    friend class shmMgr;
 private:
    Dyninst::Address addrInMutator_;
    BPatch_variableExpr *expr_;
    shMallocHandle(Dyninst::Address a, BPatch_variableExpr *e) : addrInMutator_(a), expr_(e) {};
 public:
    Dyninst::Address addressInMutatee() const;
    Dyninst::Address addressInMutator() const { return addrInMutator_; }
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

    Dyninst::Address  applicToDaemon(Dyninst::Address  addr) const;
    Dyninst::Address  daemonToApplic(Dyninst::Address  addr) const;
    Dyninst::Address  getOffsetDaemon(Dyninst::Address  addr) const;
    Dyninst::Address  getOffsetApplic(Dyninst::Address  addr) const;

    /* Do all the work of creating a new shared memory segment
       and getting the RT lib to hook to it */
    void addShmSegment(Dyninst::Address  /*size*/);

    shmMgr(BPatch_process *proc, key_t shmSegKey, unsigned shmSize_, bool freeWhenDel = true);
    // Fork constructor
    shmMgr(const shmMgr *par, BPatch_process *child_proc);
    shmMgr() {};

    bool initialize();

    Dyninst::Address malloc_int(unsigned size, bool align = false);

    Dyninst::Address  translateFromParentDaemon(Dyninst::Address  addr, const shmMgr *parent);
    Dyninst::Address  translateFromParentApplic(Dyninst::Address  addr, const shmMgr *parent);
    
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
