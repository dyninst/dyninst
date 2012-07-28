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
#include "../h/SharedMem.h"
#include "sharedMemInternal.h"
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

Dyninst::Address shmMgr::malloc_int(unsigned size, bool /* align = true*/) {
    Dyninst::Address retAddr = 0;
    
    if (freespace < size) {
        // No way to get any allocation, try to allocate new shared memory segment
        ShmSegment *new_seg = ShmSegment::Create(nextKeyToTry, shmSegSize, freeWhenDeleted);
        if (new_seg == NULL) {
            // No luck
            fprintf(stderr, "shmMgr: unable to allocate: unsufficient space, failed to allocate new segment\n");
            return 0;
        }
        if (!new_seg->attach(app)) {
            fprintf(stderr, "shmMgr: unable to allocate: unsufficient space, failed to attach to new segment\n");
            return 0;
        }
        
        nextKeyToTry++;
        totalShmSize += shmSegSize;
        freespace += shmSegSize;
        theShms.push_back(new_seg);
    }
    
    // Pass it to our shared memory segments

    for (unsigned iter = 0; iter < theShms.size(); iter++) {
        retAddr = (Dyninst::Address) theShms[iter]->malloc(size);
        if (retAddr != 0) {
            freespace -= size;
            return retAddr;
        }
    }

    assert(retAddr == 0);
    return 0;
}

void shmMgr::free(shMallocHandle *handle) 
{
    assert(handle);
    Dyninst::Address addr = (Dyninst::Address) handle->addressInMutator();

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
    Dyninst::Address retAddr = malloc_int(size, align);
    if (retAddr == 0) return 0;

    char buffer[1024];
    snprintf(buffer, 1024, "%s-%p-%d", "shMemVar", retAddr, size);

    if (type == NULL) {
        assert(BPatch::getBPatch());
        type = BPatch::getBPatch()->createScalar(buffer, size);
        assert(type);
    }
    
    // Okay, we've got an address. Let's wrap it up and go.
    BPatch_variableExpr *expr = app->createVariable(buffer, 
                                                    daemonToApplic(retAddr),
                                                    type);
    if (expr == NULL) return NULL;

    return new shMallocHandle(retAddr, expr);
}

// Assumption: the vectors of shared segments in each manager
// are equivalent
Dyninst::Address  shmMgr::translateFromParentDaemon(Dyninst::Address vaddr, const shmMgr *parShmMgr) {
    Dyninst::Address addr = (Dyninst::Address) vaddr;
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentDaemon(addr)) {
            Dyninst::Address offset = parShmMgr->theShms[i]->offsetInDaemon(addr);
            assert(theShms.size() > i);
            return (Dyninst::Address )theShms[i]->getAddrInDaemon(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}



// Assumption: the vectors of shared segments in each manager
// are equivalent
Dyninst::Address shmMgr::translateFromParentApplic(Dyninst::Address vaddr, const shmMgr *parShmMgr) {
    Dyninst::Address addr = (Dyninst::Address) vaddr;
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentApplic(addr)) {
            Dyninst::Address offset = parShmMgr->theShms[i]->offsetInApplic(addr);
            assert(theShms.size() > i);
            return (Dyninst::Address )theShms[i]->getAddrInApplic(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}

Dyninst::Address  shmMgr::applicToDaemon(Dyninst::Address vapplic) const {
    Dyninst::Address applic = (Dyninst::Address) vapplic;
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentApplic(applic)) {
            Dyninst::Address ret = theShms[i]->applicToDaemon(applic);

            return (Dyninst::Address )ret;
        }
    return 0;
}

Dyninst::Address shmMgr::daemonToApplic(Dyninst::Address vdaemon) const {
    Dyninst::Address daemon = (Dyninst::Address) vdaemon;
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentDaemon(daemon)) {
            Dyninst::Address ret = theShms[i]->daemonToApplic(daemon);
            return (Dyninst::Address )ret;
        }
    return 0;
}

shmMgr *shmMgr::handleFork(BPatch_process *childProc) {
    return new shmMgr(this, childProc);
    // If we kept internal handles, we'd want to duplicate... but
    // for now we force the user to call getChildHandle(...)
}

Dyninst::Address shMallocHandle::addressInMutatee() const {
    if (!expr_) return 0;
    return (Dyninst::Address) expr_->getBaseAddr();
}

shMallocHandle::~shMallocHandle() {
    if (expr_)
        delete expr_;
}


