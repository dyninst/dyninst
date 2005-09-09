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

/* $Id: shmMgr.C,v 1.25 2005/09/09 18:07:59 legendre Exp $
 * shmMgr: an interface to allocating/freeing memory in the 
 * shared segment. Will eventually support allocating a new
 * shared segment and attaching to it.
 */

#include "common/h/std_namesp.h"
#include "shmMgr.h"
#include "shmSegment.h"
#include "paradynd/src/pd_process.h"
#include "dyninstAPI/h/BPatch_process.h"
#include "paradynd/src/init.h"
extern void addLibraryCallback(BPatch_process *, BPatch_module *, bool);

shmMgr::shmMgr()
{
}

shmMgr::shmMgr(BPatch_process *thr, key_t shmSegKey, unsigned shmSize_, bool freeWhenDel) :
        nextKeyToTry(shmSegKey), shmSegSize(shmSize_), totalShmSize(0), freespace(0),
        app_thread(thr), freeWhenDeleted(freeWhenDel)
{
}

bool shmMgr::initialize() {

    // Get the name of the shared memory library (if specified in environment)
   const char *shm_lib_name = getenv("PARADYN_LIB_SHARED");
   if (!shm_lib_name)
      shm_lib_name = SHARED_MUTATEE_LIB;

   //getBPatch().registerDynLibraryCallback((BPatchDynLibraryCallback) (addLibraryCallback));
   // Load the shared memory library via Dyninst's loadLibrary
   assert(app_thread->isStopped());
   if (!app_thread->loadLibrary(shm_lib_name, true)) {
      fprintf(stderr, "Failed to load shared memory library\n");
      return false;
   }
   BPatch_Vector<BPatch_module *> * mods = app_thread->getImage()->getModules();
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
      fprintf(stderr, "Failed creation\n");
      return false;
   }
   if (!new_seg->attach(app_thread)) {
      fprintf(stderr, "Failed attach\n");
      return false;
   }
   
   nextKeyToTry++;
   totalShmSize += shmSegSize;
   freespace += shmSegSize;
   theShms.push_back(new_seg);
   assert(app_thread->isStopped());
   
   return true;
}


shmMgr::shmMgr(const shmMgr *par, BPatch_process *child_thr, bool sameAddress) :
        nextKeyToTry(par->nextKeyToTry), shmSegSize(par->shmSegSize), 
        totalShmSize(par->totalShmSize), freespace(par->freespace),
        app_thread(child_thr), freeWhenDeleted(par->freeWhenDeleted)
{  

    for (unsigned iter = 0; iter < par->theShms.size(); iter++) {
        ShmSegment *new_seg = par->theShms[iter]->Copy(app_thread, sameAddress);
        if (new_seg) {
            theShms.push_back(new_seg);
        }
        else {
            assert(0 && "Failed to copy parent's shared memory segment");            
        }
    }

    for(unsigned i=0; i<par->prealloc.size(); i++) {
        Address newBaseAddr = translateFromParentDaemon(par->prealloc[i]->baseAddr_, 
                                                        par);
        shmMgrPreallocInternal *new_prealloc = 
        new shmMgrPreallocInternal(par->prealloc[i], newBaseAddr);
        prealloc.push_back(new_prealloc);
    }
}

shmMgr::~shmMgr()
{
  // free the preMalloced memory
  for (unsigned i = 0; i < prealloc.size(); i++) {
      delete prealloc[i];
  }
  
  for (unsigned iter = 0; iter < theShms.size(); iter++) {
      delete theShms[iter];
  }  
}

Address shmMgr::malloc(unsigned size, bool /* align = true*/) {
    if (freespace < size) {
        // No way to get any allocation, try to allocate new shared memory segment
        ShmSegment *new_seg = ShmSegment::Create(nextKeyToTry, shmSegSize, freeWhenDeleted);
        if (new_seg == NULL) {
            // No luck
            fprintf(stderr, "Failed creation\n");
            return 0;
        }
        if (!new_seg->attach(app_thread)) {
            fprintf(stderr, "Failed attach\n");
            return 0;
        }
        
        nextKeyToTry++;
        totalShmSize += shmSegSize;
        freespace += shmSegSize;
        theShms.push_back(new_seg);
    }
    
    // Next, check to see if this size matches any of the preallocated
    // chunks
    bool size_prealloced = false;
    for (unsigned i = 0; i < prealloc.size(); i++) {
        if ((size == prealloc[i]->size_)) {
            size_prealloced = true; // used to determine if we should prealloc more
            if ((prealloc[i]->oneAvailable())) {
                Address ret = prealloc[i]->malloc();
                freespace -= size;
                return ret;
            }
        }
    }

    // If the size matches a preallocation cluster, but there was never space, preallocate
    // more (figuring that we're going to need more
    if (size_prealloced) {
        // preMalloc calls ::malloc... recursive calls should be okay, as 
        // we continue increasing allocation size (and will either succeed
        // or fail)
        if (preMalloc(size, 50)) {
            // Allocated a new one
            if (prealloc[prealloc.size()-1]->oneAvailable()) {
                Address ret = prealloc[prealloc.size()-1]->malloc();
                freespace -= size;
                return ret;
            }
            else {
                // Made a new prealloc and nothing available?
                assert(0 && "Odd case in shmMgr::malloc");
            }
        }
        // preMalloc failed, fall through
    }

    // Grump. Nothing available, so pass it to our shared memory
    // segments

    for (unsigned iter = 0; iter < theShms.size(); iter++) {
        Address addr = theShms[iter]->malloc(size);
        if (addr) {
            freespace -= size;
            return addr;
        }
    }
    return 0;
}

void shmMgr::free(Address addr) 
{
  // First, check if the addr is within any of the preallocated
  // chunks
   for (unsigned i = 0; i < prealloc.size(); i++) {
      if ((addr >= prealloc[i]->baseAddr_) &&
          (addr < (prealloc[i]->baseAddr_ + (prealloc[i]->size_*prealloc[i]->numElems_)))) {
         prealloc[i]->free(addr);
         freespace += prealloc[i]->size_;
         break;
      }
   }
   // Pass along to the shared segments
   for (unsigned iter = 0; iter < theShms.size(); iter++) {
       if (theShms[iter]->addrInSegmentDaemon(addr)) {
           theShms[iter]->free(addr);
       }
   }
}

// Preallocates a chunk of memory for a certain number of elements,
// all of size 'size'. This is a mechanism for speeding up common
// cases for malloc and free.
// Question... is allocating more than one 'chunk' with a given 
// data size allowable? For now I'll say yes.

bool shmMgr::preMalloc(unsigned size, unsigned num)
{
    Address baseAddr = this->malloc((num+1)*size, false); // We'll align
    // We allocate an extra for purposes of alignment -- we should align on
    // a <size> boundary
    if (!baseAddr) return false;
    baseAddr += size - (baseAddr % size);

    shmMgrPreallocInternal *new_prealloc = new shmMgrPreallocInternal(size, num, baseAddr);
    prealloc.push_back(new_prealloc);
    return true;
}

shmMgrPreallocInternal::shmMgrPreallocInternal(unsigned size, unsigned num, 
                                               Address baseAddr)
{
  baseAddr_ = baseAddr;
  size_ = size;
  numElems_ = num;
  currAlloc_ = 0;
  
  // Round up to 8 for purposes of bitmapping.
  unsigned rounded_num = num;
  if (rounded_num % 8) {
    rounded_num = num + 8;
    rounded_num -= (rounded_num % 8);
  }
  bitmap_size_ = rounded_num/8;
  assert((bitmap_size_ * 8) >= rounded_num);
  bitmap_ = new char[bitmap_size_];
  for (unsigned i = 0; i < rounded_num/8; i++)
    bitmap_[i] = 0;
  // If num wasn't a multiple of 8, mark the final slots as taken.
  if (rounded_num != num) {
    // rounded is going to be bigger...
    int difference = rounded_num - num;
    int counter = 7;
    while (difference > 0) {
      bitmap_[(rounded_num/8)-1] += 1 << counter;
      counter--;
      difference--;
    }
  }
}

shmMgrPreallocInternal::shmMgrPreallocInternal(
    const shmMgrPreallocInternal *par, Address newBaseAddr) :
        baseAddr_(newBaseAddr),
        size_(par->size_), numElems_(par->numElems_),
        bitmap_size_(par->bitmap_size_), currAlloc_(par->currAlloc_)
{
    bitmap_ = new char[bitmap_size_];
    memcpy(bitmap_, par->bitmap_, bitmap_size_);
}

shmMgrPreallocInternal::~shmMgrPreallocInternal()
{
   if(currAlloc_ != 0)
      cerr << "WARNING, shmMgrPreallocInternal contains unfreed allocations\n";
   delete [] bitmap_;
}

bool shmMgrPreallocInternal::oneAvailable()
{
  return (currAlloc_ < numElems_);
}

Address shmMgrPreallocInternal::malloc()
{
   if (!oneAvailable()) return 0;

   // Well, there's one here... let's try and find it. Scan the bitmaps
   // for one that is less than 0xff
   unsigned next_free_block = 0;
   for (next_free_block = 0; next_free_block<bitmap_size_; next_free_block++)
      if (bitmap_[next_free_block] != (char) 0xff)
	 break;
   int foo = 0;
   foo = bitmap_[next_free_block];
   // Should have been bounced by oneAvailable
   
   assert(next_free_block < bitmap_size_); 
   
   // Next: find slot within that is empty
   unsigned next_free_slot = 0;
   for (next_free_slot = 0; next_free_slot < 8; next_free_slot++) {
      // logical AND, right?
      if (!(bitmap_[next_free_block] &(0x1 << next_free_slot))) 
	 break;
   }

  assert(next_free_slot < 8);
  // Okay, we have our man... mark it as active, decrease available count,
  // and calculate the return address
  bitmap_[next_free_block] += 0x1 << next_free_slot;
  currAlloc_++;
  Address retAddr = baseAddr_ + 
                    (((next_free_block * 8) + next_free_slot) * size_);
  //  fprintf(stderr, "Returning addr 0x%x\n", (unsigned) retAddr);
  return retAddr;
}

void shmMgrPreallocInternal::free(Address addr)
{
   // Reverse engineer allocation, above...
   unsigned freed_block;
   unsigned freed_slot;
   freed_block = ((addr - baseAddr_)/size_)/8;
   freed_slot =  ((addr - baseAddr_)/size_) % 8;
   assert(bitmap_[freed_block] & (0x1 << freed_slot));
   bitmap_[freed_block] = bitmap_[freed_block] - (0x1 << freed_slot);
   currAlloc_--;
}

Address shmMgr::applicToDaemon(Address applic) const {
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentApplic(applic)) {
            Address ret = theShms[i]->applicToDaemon(applic);

            return ret;
        }
    return 0;
}

Address shmMgr::daemonToApplic(Address daemon) const {
    for (unsigned i = 0; i < theShms.size(); i++)
        if (theShms[i]->addrInSegmentDaemon(daemon)) {
            Address ret = theShms[i]->daemonToApplic(daemon);
            return ret;
        }
    return 0;
}

#if 0
void *shmMgetBaseAddrInDaemon() {
    return (void *)baseAddrInDaemon;
}

Address getBaseAddrInDaemon2() const {
    return baseAddrInDaemon;
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

#endif

void shmMgr::handleExec() {
    assert(0);
}

// Given an address from a different shared manager, translate
// to the current (used when cloning shared segments)

// Assumption: the vectors of shared segments in each manager
// are equivalent
Address shmMgr::translateFromParentDaemon(Address addr, const shmMgr *parShmMgr) {
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentDaemon(addr)) {
            Address offset = parShmMgr->theShms[i]->offsetInDaemon(addr);
            assert(theShms.size() > i);
            return theShms[i]->getAddrInDaemon(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}



// Assumption: the vectors of shared segments in each manager
// are equivalent
Address shmMgr::translateFromParentApplic(Address addr, const shmMgr *parShmMgr) {
    for (unsigned i = 0; i < parShmMgr->theShms.size(); i++) {
        if (parShmMgr->theShms[i]->addrInSegmentApplic(addr)) {
            Address offset = parShmMgr->theShms[i]->offsetInApplic(addr);
            assert(theShms.size() > i);
            return theShms[i]->getAddrInApplic(offset);
        }
    }
    assert(0 && "Translation failed");
    return 0;
}

