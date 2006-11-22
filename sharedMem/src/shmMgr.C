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

/* $Id: shmMgr.C,v 1.1 2006/11/22 21:44:59 bernat Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#include "BPatch_Vector.h"
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "shmMgr.h"
#include "shmSegment.h"

extern void addLibraryCallback(BPatch_process *, BPatch_module *, bool);

shmMgr::shmMgr(BPatch_process *proc, key_t shmSegKey, unsigned shmSize_, bool freeWhenDel) :
        nextKeyToTry(shmSegKey), shmSegSize(shmSize_), totalShmSize(0), freespace(0),
        app(proc), freeWhenDeleted(freeWhenDel)
{
}

shmMgr *shmMgr::createSharedManager(BPatch_process *proc,
                                    key_t shmSegKey,
                                    unsigned defaultSegmentSize,
                                    bool automaticFree) {
    shmMgr *shm = new shmMgr(proc, shmSegKey, defaultSegmentSize, automaticFree);
    if (shm == NULL) return NULL;
    if (!shm->initialize()) {
        delete shm;
        return NULL;
    }
    return shm;
}


bool shmMgr::initialize() {
	// Get the name of the shared memory library (if specified in environment)
   const char *shm_lib_name = getenv("DYNINSTAPI_SHM_LIB");
   if (!shm_lib_name)
      shm_lib_name = SHARED_MUTATEE_LIB;

   //getBPatch().registerDynLibraryCallback((BPatchDynLibraryCallback) (addLibraryCallback));
   // Load the shared memory library via Dyninst's loadLibrary
   if (!app->loadLibrary(shm_lib_name, true)) {
      fprintf(stderr, "Failed to load shared memory library\n");
      return false;
   }
   BPatch_Vector<BPatch_module *> * mods = app->getImage()->getModules();
   static char mnamebuf[512];
   BPatch_module *shm_mod = NULL;
   for (unsigned int i = 0; i < mods->size(); ++i) {
      (*mods)[i]->getName(mnamebuf, 512);
      if (!strcmp(mnamebuf, shm_lib_name)) {
         shm_mod = (*mods)[i];
         break;
      }
   }
   if (!shm_mod) {
      fprintf(stderr, "%s[%d}:  Could not find module %s\n", __FILE__, __LINE__, shm_lib_name);
   } 

   // Proactively make the first segment
   ShmSegment *new_seg = ShmSegment::Create(nextKeyToTry, shmSegSize, freeWhenDeleted);
   if (new_seg == NULL) {
      // No luck
      fprintf(stderr, "%s[%d]: shared memory manager: failed creation\n");
      return false;
   }
   if (!new_seg->attach(app)) {
      fprintf(stderr, "%s[%d]: shared memory manager: failed attach\n");
      return false;
   }
   
   nextKeyToTry++;
   totalShmSize += shmSegSize;
   freespace += shmSegSize;
   theShms.push_back(new_seg);
   return true;
}


shmMgr::shmMgr(const shmMgr *par, BPatch_process *child) :
        nextKeyToTry(par->nextKeyToTry), shmSegSize(par->shmSegSize), 
        totalShmSize(par->totalShmSize), freespace(par->freespace),
        app(child), freeWhenDeleted(par->freeWhenDeleted)
{  

    for (unsigned iter = 0; iter < par->theShms.size(); iter++) {
        ShmSegment *new_seg = par->theShms[iter]->Copy(app, true);
        if (new_seg) {
            theShms.push_back(new_seg);
        }
        else {
            assert(0 && "Failed to copy parent's shared memory segment");            
        }
    }
}

shmMgr::~shmMgr()
{
  // free the preMalloced memory
  for (unsigned iter = 0; iter < theShms.size(); iter++) {
      delete theShms[iter];
  }  
}

void *shmMgr::malloc_int(unsigned size, bool /* align = true*/) {
    void *retAddr = NULL;
    
    if (freespace < size) {
        // No way to get any allocation, try to allocate new shared memory segment
        ShmSegment *new_seg = ShmSegment::Create(nextKeyToTry, shmSegSize, freeWhenDeleted);
        if (new_seg == NULL) {
            // No luck
            fprintf(stderr, "shmMgr: unable to allocate: unsufficient space, failed to allocate new segment\n");
            return NULL;
        }
        if (!new_seg->attach(app)) {
            fprintf(stderr, "shmMgr: unable to allocate: unsufficient space, failed to attach to new segment\n");
            return NULL;
        }
        
        nextKeyToTry++;
        totalShmSize += shmSegSize;
        freespace += shmSegSize;
        theShms.push_back(new_seg);
    }
    
    // Pass it to our shared memory segments

    for (unsigned iter = 0; iter < theShms.size(); iter++) {
        retAddr = (void *) theShms[iter]->malloc(size);
        if (retAddr != NULL) {
            freespace -= size;
            return retAddr;
        }
    }

    assert(retAddr == NULL);
    return NULL;
}

void shmMgr::free(shMallocHandle *handle) 
{
    assert(handle);
    Address addr = (Address) handle->addressInMutator();

    delete handle;

    // Pass along to the shared segments
    for (unsigned iter = 0; iter < theShms.size(); iter++) {
        if (theShms[iter]->addrInSegmentDaemon(addr)) {
            theShms[iter]->free(addr);
            return;
        }
    }
}

shMallocHandle *shmMgr::malloc(unsigned size, bool align /* = true*/, BPatch_type *type /* = NULL */) {
    void *retAddr = malloc_int(size, align);
    if (retAddr == NULL) return NULL;

    char buffer[1024];
    snprintf(buffer, 1024, "%s-%p-%d", "shMemVar", retAddr, size);

    if (type == NULL) {
        assert(BPatch::getBPatch());
        type = BPatch::getBPatch()->createScalar(buffer, size);
        assert(type);
    }
    
    // Okay, we've got an address. Let's wrap it up and go.
    BPatch_variableExpr *expr = new BPatch_variableExpr(buffer, app, 
                                       daemonToApplic(retAddr), 
                                       type);

    return new shMallocHandle(retAddr, expr);
}

// Assumption: the vectors of shared segments in each manager
// are equivalent
void * shmMgr::translateFromParentDaemon(void *vaddr, const shmMgr *parShmMgr) {
    Address addr = (Address) vaddr;
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentDaemon(addr)) {
            Address offset = parShmMgr->theShms[i]->offsetInDaemon(addr);
            assert(theShms.size() > i);
            return (void *)theShms[i]->getAddrInDaemon(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}



// Assumption: the vectors of shared segments in each manager
// are equivalent
void *shmMgr::translateFromParentApplic(void *vaddr, const shmMgr *parShmMgr) {
    Address addr = (Address) vaddr;
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentApplic(addr)) {
            Address offset = parShmMgr->theShms[i]->offsetInApplic(addr);
            assert(theShms.size() > i);
            return (void *)theShms[i]->getAddrInApplic(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}

void * shmMgr::applicToDaemon(void *vapplic) const {
    Address applic = (Address) vapplic;
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentApplic(applic)) {
            Address ret = theShms[i]->applicToDaemon(applic);

            return (void *)ret;
        }
    return 0;
}

void *shmMgr::daemonToApplic(void *vdaemon) const {
    Address daemon = (Address) vdaemon;
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentDaemon(daemon)) {
            Address ret = theShms[i]->daemonToApplic(daemon);
            return (void *)ret;
        }
    return 0;
}

shmMgr *shmMgr::handleFork(BPatch_process *childProc) {
    return new shmMgr(this, childProc);
    // If we kept internal handles, we'd want to duplicate... but
    // for now we force the user to call getChildHandle(...)
}

void *shMallocHandle::addressInMutatee() const {
    if (!expr_) return NULL;
    return expr_->getBaseAddr();
}

shMallocHandle::~shMallocHandle() {
    if (expr_)
        delete expr_;
}


