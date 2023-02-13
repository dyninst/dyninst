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

#include "addressSpace.h"
#include "codeRange.h"
#include "dynProcess.h"
#include "function.h"
#include "binaryEdit.h"
#include "baseTramp.h"

#include "instPoint.h"
#include "debug.h"

// Two-level codeRange structure
#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include "Instruction.h"

#include "common/h/DynAST.h"
#include "Relocation/CodeMover.h"
#include "Relocation/Springboard.h"
#include "Relocation/Transformers/Include.h"
#include "Relocation/CodeTracker.h"

#include "parseAPI/h/CodeObject.h"
#include <boost/tuple/tuple.hpp>

#include "PatchMgr.h"
#include "Patching.h"
#include "Relocation/DynAddrSpace.h"
#include "Relocation/DynPointMaker.h"
#include "Relocation/DynObject.h"
#include "Relocation/DynInstrumenter.h"

#include <boost/bind/bind.hpp>

#include "dynThread.h"
#include "pcEventHandler.h"
#include "unaligned_memory_access.h"

// Implementations of non-virtual functions in the address space
// class.

using namespace Dyninst;

using PatchAPI::DynObject;
using PatchAPI::DynAddrSpace;
using PatchAPI::PatchMgr;
using PatchAPI::Patcher;
using PatchAPI::DynInstrumenter;
using PatchAPI::DynRemoveSnipCommand;

AddressSpace::AddressSpace () :
    trapMapping(this),
    new_func_cb(NULL),
    new_instp_cb(NULL),
    heapInitialized_(false),
    useTraps_(true),
    sigILLTrampoline_(false),
    trampGuardBase_(NULL),
    up_ptr_(NULL),
    costAddr_(0),
    installedSpringboards_(new Relocation::InstalledSpringboards()),
    delayRelocation_(false)
{
   // Historically, we only use SIGTRAP as the signal for tramopline.
   // However, SIGTRAP is always intercepted by GDB, causing it is 
   // almost impossible to debug through signal trampolines.
   // Here, we add a new environment variable DYNINST_SIGNAL_TRAMPOLINE_SIGILL
   // to control whether we use SIGILL as the signal for trampolines.
   // In the case of binary rewriting, DYNINST_SIGNAL_TRAMPOLINE_SIGILL should be 
   // consistently set or unset for rewriting the binary and running the rewritten binaries. 
   if (getenv("DYNINST_SIGNAL_TRAMPOLINE_SIGILL")) {
      sigILLTrampoline_ = true;
   }
}

AddressSpace::~AddressSpace() {
    if (mgr_)
       static_cast<DynAddrSpace*>(mgr_->as())->removeAddrSpace(this);

    deleteAddressSpace();
}

PCProcess *AddressSpace::proc() {
    return dynamic_cast<PCProcess *>(this);
}

BinaryEdit *AddressSpace::edit() {
    return dynamic_cast<BinaryEdit *>(this);
}
Address AddressSpace::getTOCoffsetInfo(func_instance *func) {
  // Symtab has this information on a per-Function basis. It's
  // kinda nontrivial to get a Function object out of a 
  // func_instance; instead we use its entry address which
  // is what all the TOC data structures are written in terms of
  // anyway

  bool toc64 = false;
  bool toc32 = false;
#if defined(cap_toc_64)
  toc64 = true;
#endif
#if defined(cap_toc_32)
  toc32 = true;
#endif
  if (getAddressWidth() == 8 && !toc64) return 0;
  if (getAddressWidth() == 4 && !toc32) return 0;

  // In PowerABI V2, each binary has a TOC
  Address baseTOC = func->obj()->getTOCBaseAddress();
  return baseTOC + func->obj()->dataBase();
}

// Fork constructor - and so we can assume a parent "process"
// rather than "address space"
//
// Actually, for the sake of abstraction, use an AddressSpace instead of process
void AddressSpace::copyAddressSpace(AddressSpace *parent) {
    deleteAddressSpace();

    // This is only defined for process->process copy
    // until someone can give a good reason for copying
    // anything else...
    assert(proc());

    mapped_object *par_aout = parent->getAOut();
    mapped_object *child_aout = new mapped_object(par_aout, proc());
    initPatchAPI();
    addMappedObject(child_aout);

    // Mapped objects first
    for (unsigned i = 0; i < parent->mapped_objects.size(); i++) {
        mapped_object *par_obj = parent->mapped_objects[i];
        if (parent->getAOut() != par_obj) {
          mapped_object *child_obj = new mapped_object(par_obj, proc());
          assert(child_obj);
          addMappedObject(child_obj);
        }
    }

    // Clone the tramp guard base
    if (parent->trampGuardBase_) 
      trampGuardBase_ = new int_variable(parent->trampGuardBase_, getAOut()->getDefaultModule());
    else
      trampGuardBase_ = NULL;

    /////////////////////////
    // Inferior heap
    /////////////////////////
    heap_ = inferiorHeap(parent->heap_);
    heapInitialized_ = parent->heapInitialized_;

    /////////////////////////
    // Trap mappings
    /////////////////////////
    trapMapping.copyTrapMappings(& (parent->trapMapping));

    /////////////////////////
    // Code tracking system
    /////////////////////////
    for (auto *ct : parent->relocatedCode_) {
       // Efficiency; this avoids a spurious copy of the entire CodeTracker.
       relocatedCode_.push_back(Relocation::CodeTracker::fork(ct, this));
    }
    
    // Let's assume we're not forking in the middle of instrumentation, so
    // leave modifiedFunctions_ alone.

    assert(parent->mgr());
    PatchAPI::CallModMap& cmm = parent->mgr()->instrumenter()->callModMap();
    for (PatchAPI::CallModMap::iterator iter = cmm.begin(); iter != cmm.end(); ++iter) {
       // Need to forward map the lot
      block_instance *newB = findBlock(SCAST_BI(iter->first)->llb());
      for (std::map<PatchFunction*, PatchFunction*>::iterator iter2 = iter->second.begin();
            iter2 != iter->second.end(); ++iter2) {
        func_instance *context = (SCAST_FI(iter2->first) == NULL) ? NULL : findFunction(SCAST_FI(iter2->first)->ifunc());
        func_instance *target = (SCAST_FI(iter2->second) == NULL) ? NULL : findFunction(SCAST_FI(iter2->second)->ifunc());
        cmm[newB][context] = target;
       }
    }

    PatchAPI::FuncModMap& frm = parent->mgr()->instrumenter()->funcRepMap();
    for (PatchAPI::FuncModMap::iterator iter = frm.begin(); iter != frm.end(); ++iter) {
      func_instance *from = findFunction(SCAST_FI(iter->first)->ifunc());
      func_instance *to = findFunction(SCAST_FI(iter->second)->ifunc());
      frm[from] = to;
    }

    PatchAPI::FuncWrapMap& fwm = parent->mgr()->instrumenter()->funcWrapMap();
    for (PatchAPI::FuncWrapMap::iterator iter = fwm.begin(); iter != fwm.end(); ++iter) {
      func_instance *from = findFunction(SCAST_FI(iter->first)->ifunc());
      func_instance *to = findFunction(SCAST_FI(iter->second.first)->ifunc());
      fwm[from] = std::make_pair(to, iter->second.second);
    }
}

void AddressSpace::deleteAddressSpace() {
   heapInitialized_ = false;
   heap_.clear();

   for (auto *mo : mapped_objects) {
      delete mo;
   }
   mapped_objects.clear();

   for (auto *rc : relocatedCode_) {
      delete rc;
   }
   relocatedCode_.clear();

   /*
   * NB: We do not own the contents of forwardDefensiveMap_, reverseDefensiveMap_,
   *     instrumentationInstances_, modifiedFunctions_, reverseDefensiveMap_,
   *     or runtime_lib
   */
   forwardDefensiveMap_.clear();
   reverseDefensiveMap_.clear();
   instrumentationInstances_.clear();
   modifiedFunctions_.clear();
   runtime_lib.clear();

   trampGuardBase_ = NULL;
   trampGuardAST_ = AstNodePtr();

   // up_ptr_ is untouched
   costAddr_ = 0;
}



// Returns the named symbol from the image or a shared object
bool AddressSpace::getSymbolInfo( const std::string &name, int_symbol &ret ) 
{
   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      if (mapped_objects[i]->getSymbolInfo( name, ret ))
         return true;
   }
   return false;
}

bool heapItemLessByAddr(const heapItem *a, const heapItem *b)
{
   if (a->addr < b->addr) {
      return true;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Memory allocation routines
//////////////////////////////////////////////////////////////////////////////


void AddressSpace::inferiorFreeCompact() {
   std::vector<heapItem *> &freeList = heap_.heapFree;
   unsigned i, nbuf = freeList.size();

   /* sort buffers by address */
    std::sort(freeList.begin(), freeList.end(), heapItemLessByAddr);

   /* combine adjacent buffers */
   bool needToCompact = false;
   for (i = 1; i < freeList.size(); i++) {
      heapItem *h1 = freeList[i-1];
      heapItem *h2 = freeList[i];
      assert(h1->length != 0);
      if (h1->addr + h1->length > h2->addr) {
         fprintf(stderr, "Error: heap 1 (%p) (0x%p to 0x%p) overlaps heap 2 (%p) (0x%p to 0x%p)\n",
                 (void*)h1,
                 (void *)h1->addr, (void *)(h1->addr + h1->length),
                 (void*)h2,
                 (void *)h2->addr, (void *)(h2->addr + h2->length));
      }
      assert(h1->addr + h1->length <= h2->addr);
      if (h1->addr + h1->length == h2->addr
          && h1->type == h2->type) {
         h2->addr = h1->addr;
         h2->length = h1->length + h2->length;
         h1->length = 0;
         nbuf--;
         needToCompact = true;
      }
   }

   /* remove any absorbed (empty) buffers */ 
   if (needToCompact) {
      std::vector<heapItem *> cleanList;
      unsigned end = freeList.size();
      for (i = 0; i < end; i++) {
         heapItem *h1 = freeList[i];
         if (h1->length != 0) {
            cleanList.push_back(h1);
         } else {
            delete h1;
         }
      }
      assert(cleanList.size() == nbuf);
      for (i = 0; i < nbuf; i++) {
         freeList[i] = cleanList[i];
      }
      freeList.resize(nbuf);
      assert(freeList.size() == nbuf);
   }
}
    
int AddressSpace::findFreeIndex(unsigned size, int type, Address lo, Address hi) {
   // type is a bitmask: match on any bit in the mask
   std::vector<heapItem *> &freeList = heap_.heapFree;
    
   int best = -1;
   for (unsigned i = 0; i < freeList.size(); i++) {
      heapItem *h = freeList[i];
      // check if free block matches allocation constraints
      // Split out to facilitate debugging
      infmalloc_printf("%s[%d]: comparing heap %u: 0x%lx-0x%lx/%d to desired %u bytes in 0x%lx-0x%lx/%d\n",
                       FILE__, __LINE__, 
                       i,
                       h->addr, 
                       h->addr + h->length,
                       h->type,
                       size,
                       lo, 
                       hi,
                       type);
      if (h->addr >= lo &&
          (h->addr + size - 1) <= hi &&
          h->length >= size &&
          h->type & type) {
         if (best == -1)
            best = i;
         // check for better match
         if (h->length < freeList[best]->length) best = i;
      }
   }
   infmalloc_printf("%s[%d]: returning match %d\n", FILE__, __LINE__, best);
   return best;
}

void AddressSpace::addHeap(heapItem *h) {
   heap_.bufferPool.push_back(h);
   heapItem *h2 = new heapItem(h);
   h2->status = HEAPfree;
   heap_.heapFree.push_back(h2);
    
   /* When we add an item to heapFree, make sure it remains in sorted order */
   std::sort(heap_.heapFree.begin(), heap_.heapFree.end(), heapItemLessByAddr);

   heap_.totalFreeMemAvailable += h2->length;
}

void AddressSpace::initializeHeap() {
   // (re)initialize everything 
   heap_.heapActive.clear();
   heap_.heapFree.resize(0);
   heap_.disabledList.resize(0);
   heap_.disabledListTotalMem = 0;
   heap_.freed = 0;
   heap_.totalFreeMemAvailable = 0;

   heapInitialized_ = true;
}

/* returns true if memory was allocated for a variable starting at address
   "block", otherwise returns false
*/
bool AddressSpace::isInferiorAllocated(Address block) {
   return (heap_.heapActive.find(block) != heap_.heapActive.end());
}

Address AddressSpace::inferiorMallocInternal(unsigned size,
                                             Address lo,
                                             Address hi,
                                             inferiorHeapType type) {
   infmalloc_printf("%s[%d]: inferiorMallocInternal, %u bytes, type %d, between 0x%lx - 0x%lx\n",
                    FILE__, __LINE__, size, type, lo, hi);
   int freeIndex = findFreeIndex(size, type, lo, hi);
   if (freeIndex == -1) return 0; // Failure is often an option


   // adjust active and free lists
   assert(freeIndex != -1);
   heapItem *h = heap_.heapFree[freeIndex];
   assert(h);
    
   // remove allocated buffer from free list
   if (h->length != size) {
      // size mismatch: put remainder of block on free list
      heapItem *rem = new heapItem(h);
      rem->addr += size;
      rem->length -= size;
      heap_.heapFree[freeIndex] = rem;
   } else {
      // size match: remove entire block from free list
      unsigned last = heap_.heapFree.size();
      heap_.heapFree[freeIndex] = heap_.heapFree[last-1];
      heap_.heapFree.resize(last-1);
   }

   /* When we update an item in heapFree, make sure it remains in sorted order */
   std::sort(heap_.heapFree.begin(), heap_.heapFree.end(), heapItemLessByAddr);

   // add allocated block to active list
   h->length = size;
   h->status = HEAPallocated;
   heap_.heapActive[h->addr] = h;
   // bookkeeping
   heap_.totalFreeMemAvailable -= size;
   assert(h->addr);
    
   return(h->addr);
}

void AddressSpace::inferiorFreeInternal(Address block) {
   // find block on active list
   infmalloc_printf("%s[%d]: inferiorFree for block at 0x%lx\n", FILE__, __LINE__, block);

   auto iter = heap_.heapActive.find(block);
   if (iter == heap_.heapActive.end()) return;
   heapItem *h = iter->second;
   assert(h);

   // Remove from the active list
   heap_.heapActive.erase(iter);
    
   // Add to the free list
   h->status = HEAPfree;
   heap_.heapFree.push_back(h);

   /* When we add an item to heapFree, make sure it remains in sorted order */
   std::sort(heap_.heapFree.begin(), heap_.heapFree.end(), heapItemLessByAddr);

   heap_.totalFreeMemAvailable += h->length;
   heap_.freed += h->length;
   infmalloc_printf("%s[%d]: Freed block from 0x%lx - 0x%lx, %u bytes, type %d\n",
                    FILE__, __LINE__,
                    h->addr,
                    h->addr + h->length,
                    h->length,
                    h->type);
}

void AddressSpace::inferiorMallocAlign(unsigned &size) {
   // Align to the process word
   unsigned alignment = (getAddressWidth() - 1);
   size = (size + alignment) & ~alignment;
}
    
bool AddressSpace::inferiorReallocInternal(Address block, unsigned newSize) {
  //#if defined (cap_dynamic_heap)
   // This is why it's not a reference...
   inferiorMallocAlign(newSize);
   //#endif
    
   infmalloc_printf("%s[%d]: inferiorRealloc for block 0x%lx, new size %u\n",
                    FILE__, __LINE__, block, newSize);

   auto iter = heap_.heapActive.find(block);
   if (iter == heap_.heapActive.end()) {
      // We can do this if we're at process teardown.
      infmalloc_printf("%s[%d]: inferiorRealloc unable to find block, returning\n", FILE__, __LINE__);
      return false;
   }
   heapItem *h = iter->second;
   assert(h);
   infmalloc_printf("%s[%d]: inferiorRealloc found block with addr 0x%lx, length %u\n",
                    FILE__, __LINE__, h->addr, h->length);

   if (h->length == newSize)
      return true;
   else if (h->length > newSize) {
      // Shrink the block
      return inferiorShrinkBlock(h, block, newSize);
   }
   else {
      // See if we can grow this block
      return inferiorExpandBlock(h, block, newSize);
   }
}

bool AddressSpace::inferiorShrinkBlock(heapItem *h, 
				       Address block, 
				       unsigned newSize) {

   // We make a new "free" block that is the end of this one.
   Address freeStart = block + newSize;
   Address succAddr = h->addr + h->length;
   int shrink = h->length - newSize;
    
   assert(shrink > 0);
    
   h->length = newSize;
    
   // New speedy way. Find the block that is the successor of the
   // active block; if it exists, simply enlarge it "downwards". Otherwise,
   // make a new block. 
   heapItem *succ = NULL;
   for (unsigned i = 0; i < heap_.heapFree.size(); i++) {
      heapItem *tmp = heap_.heapFree[i];
      assert(tmp);
      if (tmp->addr == succAddr) {
         succ = tmp;
         break;
      }
   }
   if (succ != NULL) {
      infmalloc_printf("%s[%d]: enlarging existing block; old 0x%lx - 0x%lx (%lu), new 0x%lx - 0x%lx (%u)\n",
                       FILE__, __LINE__,
                       succ->addr,
                       succ->addr + succ->length,
                       succ->addr,
                       succ->addr - shrink,
                       succ->addr + succ->length,
                       succ->length + shrink);


      succ->addr -= shrink;
      succ->length += shrink;
   }
   else {
      // Must make a new block to represent the free memory
      infmalloc_printf("%s[%d]: inferiorRealloc: creating new block 0x%lx to 0x%lx (%d), type %d\n",
                       FILE__, __LINE__, 
                       freeStart, 
                       freeStart + shrink,
                       shrink,
                       h->type);

      heapItem *freeEnd = new heapItem(freeStart,
                                       shrink,
                                       h->type,
                                       h->dynamic,
                                       HEAPfree);
      heap_.heapFree.push_back(freeEnd);

      /* When we add an item to heapFree, make sure it remains sorted */
      std::sort(heap_.heapFree.begin(), heap_.heapFree.end(), heapItemLessByAddr);
   }

   heap_.totalFreeMemAvailable += shrink;
   heap_.freed += shrink;

   return true;
}    

bool AddressSpace::inferiorExpandBlock(heapItem *h, 
				       Address, 
				       unsigned newSize) {
   // We attempt to find a free block that immediately succeeds
   // this one. If we find such a block we expand this block into
   // the next; if this is possible we return true. Otherwise
   // we return false.

   Address succAddr = h->addr + h->length;
   int expand = newSize - h->length;
   assert(expand > 0);
    
   // New speedy way. Find the block that is the successor of the
   // active block; if it exists, simply enlarge it "downwards". Otherwise,
   // make a new block. 
   heapItem *succ = NULL;
   for (unsigned i = 0; i < heap_.heapFree.size(); i++) {
      heapItem *tmp = heap_.heapFree[i];
      assert(tmp);
      if (tmp->addr == succAddr) {
         succ = tmp;
         break;
      }
   }
   if (succ != NULL) {
      if (succ->length < (unsigned) expand) {
         // Can't fit
         return false;
      }
      Address newFreeBase = succAddr + expand;
      int newFreeLen = succ->length - expand;
      succ->addr = newFreeBase;
      succ->length = newFreeLen;

      // If we've enlarged to exactly the end of the successor (succ->length == 0),
      // remove succ
      if (0x0 == succ->length) {
          std::vector<heapItem *> cleanList;
          unsigned end = heap_.heapFree.size();
          for (unsigned i = 0; i < end; i++) {
              heapItem * h1 = heap_.heapFree[i];
              if (h1->length != 0) {
                  cleanList.push_back(h1);
              } else {
                  delete h1;
              }
          }
          end--;
          for (unsigned i = 0; i < end; i++) {
              heap_.heapFree[i] = cleanList[i];  
          }
          // Remove the last (now unused) element of the freeList
          heap_.heapFree.pop_back();
      }
   }
   else {
      return false;
   }

   heap_.totalFreeMemAvailable -= expand;
  
   return true;
}    

/////////////////////////////////////////
// Function lookup...
/////////////////////////////////////////

bool AddressSpace::findFuncsByAll(const std::string &funcname,
                                  std::vector<func_instance *> &res,
                                  const std::string &libname) { // = "", btw
    
   unsigned starting_entries = res.size(); // We'll return true if we find something
   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      if (libname == "" ||
          mapped_objects[i]->fileName() == libname.c_str() ||
          mapped_objects[i]->fullName() == libname.c_str()) {
         const std::vector<func_instance *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
         if (pretty) {
            // We stop at first match...
            for (unsigned pm = 0; pm < pretty->size(); pm++) {
               res.push_back((*pretty)[pm]);
            }
         }
         else {
            const std::vector<func_instance *> *mangled = mapped_objects[i]->findFuncVectorByMangled(funcname);
            if (mangled) {
               for (unsigned mm = 0; mm < mangled->size(); mm++) {
                  res.push_back((*mangled)[mm]);
               }
            }
         }
      }
   }

   return (res.size() != starting_entries);
}


bool AddressSpace::findFuncsByPretty(const std::string &funcname,
                                     std::vector<func_instance *> &res,
                                     const std::string &libname) { // = "", btw

   unsigned starting_entries = res.size(); // We'll return true if we find something

   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      if (libname == "" ||
          mapped_objects[i]->fileName() == libname.c_str() ||
          mapped_objects[i]->fullName() == libname.c_str()) {
         const std::vector<func_instance *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
         if (pretty) {
            // We stop at first match...
            for (unsigned pm = 0; pm < pretty->size(); pm++) {
               res.push_back((*pretty)[pm]);
            }
         }
      }
   }
   return res.size() != starting_entries;
}


bool AddressSpace::findFuncsByMangled(const std::string &funcname,
                                      std::vector<func_instance *> &res,
                                      const std::string &libname) { // = "", btw
   unsigned starting_entries = res.size(); // We'll return true if we find something

   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      if (libname == "" ||
          mapped_objects[i]->fileName() == libname.c_str() ||
          mapped_objects[i]->fullName() == libname.c_str()) {
         const std::vector<func_instance *> *mangled = 
            mapped_objects[i]->findFuncVectorByMangled(funcname);
         if (mangled) {
            for (unsigned mm = 0; mm < mangled->size(); mm++) {
               res.push_back((*mangled)[mm]);
            }
         }
      }
   }
   return res.size() != starting_entries;
}

func_instance *AddressSpace::findOnlyOneFunction(const string &name,
                                                 const string &lib,
                                                 bool /*search_rt_lib*/) 
{
   assert(mapped_objects.size());

   std::vector<func_instance *> allFuncs;

   if (!findFuncsByAll(name.c_str(), allFuncs, lib.c_str()))
      return NULL;

   if (allFuncs.size() > 1) 
   {
      //cerr << "Warning: multiple matches for " << name << ", returning first" << endl;
   }

   return allFuncs[0];
}

/////////////////////////////////////////
// Variable lookup...
/////////////////////////////////////////

bool AddressSpace::findVarsByAll(const std::string &varname,
                                 std::vector<int_variable *> &res,
                                 const std::string &libname) { // = "", btw
   unsigned starting_entries = res.size(); // We'll return true if we find something
    
   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      if (libname == "" ||
          mapped_objects[i]->fileName() == libname.c_str() ||
          mapped_objects[i]->fullName() == libname.c_str()) {
         const std::vector<int_variable *> *pretty = mapped_objects[i]->findVarVectorByPretty(varname);
         if (pretty) {
            // We stop at first match...
            for (unsigned pm = 0; pm < pretty->size(); pm++) {
               res.push_back((*pretty)[pm]);
            }
         }
         else {
            const std::vector<int_variable *> *mangled = mapped_objects[i]->findVarVectorByMangled(varname);
            if (mangled) {
               for (unsigned mm = 0; mm < mangled->size(); mm++) {
                  res.push_back((*mangled)[mm]);
               }
            }
         }
      }
   }

   return res.size() != starting_entries;
}



// Get me a pointer to the instruction: the return is a local
// (mutator-side) store for the mutatee. This may duck into the local
// copy for images, or a relocated function's self copy.
// TODO: is this really worth it? Or should we just use ptrace?

void *AddressSpace::getPtrToInstruction(const Address addr) const {
   mapped_object *obj = findObject(addr);
   if (obj) return obj->getPtrToInstruction(addr);

   fprintf(stderr,"[%s:%d] failed to find matching range for address %lx\n",
           FILE__,__LINE__,addr);
   assert(0);
   return NULL;
}

bool AddressSpace::isCode(const Address addr) const {
   mapped_object *obj = findObject(addr);
   if (!obj) return false;

   Address objStart = obj->codeAbs();
   Address objEnd;
   if (BPatch_defensiveMode == obj->hybridMode()) {
      objEnd = obj->memoryEnd();
   } else {
      objEnd = objStart + obj->imageSize();
   }

   return (addr >= objStart && addr <= objEnd);

}
bool AddressSpace::isData(const Address addr) const {
   mapped_object *obj = findObject(addr);
   if (!obj) return false;
   
   Address dataStart = obj->dataAbs();
   if (addr >= dataStart &&
       addr < (dataStart + obj->dataSize())) return true;
   return false;
}

bool AddressSpace::isReadOnly(const Address ) const {
   return false;
}

bool AddressSpace::isValidAddress(const Address addr) const {
   mapped_object *obj = findObject(addr);
   if (!obj) return false;
   
   if (obj->isCode(addr) || obj->isData(addr)) return true;
   return false;
}

mapped_object *AddressSpace::findObject(Address addr) const {
   for (unsigned i=0; i<mapped_objects.size(); i++)
   {
      Address objStart = mapped_objects[i]->codeAbs();
      Address objEnd; // calculate objEnd
      if (BPatch_defensiveMode == mapped_objects[i]->hybridMode()) {
         objEnd = mapped_objects[i]->memoryEnd();
      } else {
         objEnd = objStart + mapped_objects[i]->imageSize();
      }

      if (addr >= objStart && addr < objEnd)
      {
         return mapped_objects[i];
      }
   }
   return NULL;
}

mapped_object *AddressSpace::findObject(const ParseAPI::CodeObject *co) const {
   mapped_object *obj = 
      findObject(static_cast<ParseAPI::SymtabCodeSource*>(co->cs())->
                 getSymtabObject()->file());
   return obj;
}

func_instance *AddressSpace::findFunction(parse_func *ifunc) {
   assert(ifunc);

   return findObject(ifunc->obj())->findFunction(ifunc);
}

block_instance *AddressSpace::findBlock(parse_block *iblk) {
   assert(iblk);

   return findObject(iblk->obj())->findBlock(iblk);
}

edge_instance *AddressSpace::findEdge(ParseAPI::Edge *iedge) {
   assert(iedge);
   return findObject(iedge->src()->obj())->findEdge(iedge);
}

// findModule: returns the module associated with mod_name 
// this routine checks both the a.out image and any shared object
// images for this resource
mapped_module *AddressSpace::findModule(const std::string &mod_name, bool wildcard)
{
   // KLUDGE: first search any shared libraries for the module name 
   //  (there is only one module in each shared library, and that 
   //  is the library name)
   for(u_int j=0; j < mapped_objects.size(); j++){
      mapped_module *mod = mapped_objects[j]->findModule(mod_name.c_str(), wildcard);
      if (mod) {
         return (mod);
      }
   }

   return NULL;
}

// findObject: returns the object associated with obj_name 
// This just iterates over the mapped object vector
mapped_object *AddressSpace::findObject(std::string obj_name, bool wildcard) const
{
   // Update: check by full name first because we may have non-unique fileNames. 
   for(u_int j=0; j < mapped_objects.size(); j++){
      if (mapped_objects[j]->fullName() == obj_name ||
          (wildcard &&
           wildcardEquiv(obj_name, mapped_objects[j]->fullName())))
         return mapped_objects[j];
   }

   // get rid of the directory in the path
   std::string orig_name = obj_name;
   std::string::size_type dir = obj_name.rfind('/');
   if (dir != std::string::npos) {
      // +1, as that finds the slash and we don't want it. 
      obj_name = obj_name.substr(dir+1);
   } else {
	   dir = obj_name.rfind('\\');
	   if (dir != std::string::npos){
		 obj_name = obj_name.substr(dir+1);
	   }
   }

   for(u_int j=0; j < mapped_objects.size(); j++){
      if (mapped_objects[j]->fileName() == obj_name ||
          (wildcard &&
           wildcardEquiv(obj_name, mapped_objects[j]->fileName())))
         return mapped_objects[j];
   }
#if 0
   cerr << "Warning: failed to find mapped_object matching " << obj_name
        << " (originally " << orig_name << ")"
        << " with wildcard " << (wildcard ? "<on>" : "<off>") << endl;
   for (unsigned int i = 0; i < mapped_objects.size(); ++i) {
      cerr << "\t" << mapped_objects[i]->fileName() << " / " << mapped_objects[i]->fullName() << endl;
   }
#endif
   return NULL;
}

// findObject: returns the object associated with obj_name 
// This just iterates over the mapped object vector
mapped_object *AddressSpace::findObject(fileDescriptor desc) const
{
   for(u_int j=0; j < mapped_objects.size(); j++){
      if (desc == mapped_objects[j]->getFileDesc()) 
         return mapped_objects[j];
   }
   return NULL;
}

// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects

void AddressSpace::getAllFunctions(std::vector<func_instance *> &funcs) {
   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      mapped_objects[i]->getAllFunctions(funcs);
   }
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects

void AddressSpace::getAllModules(std::vector<mapped_module *> &mods){
   for (unsigned i = 0; i < mapped_objects.size(); i++) {
      const std::vector<mapped_module *> &obj_mods = mapped_objects[i]->getModules();
      for (unsigned j = 0; j < obj_mods.size(); j++) {
         mods.push_back(obj_mods[j]);
      }
   }
}

//Acts like findTargetFuncByAddr, but also finds the function if addr
// is an indirect jump to a function.
//I know this is an odd function, but darn I need it.
func_instance *AddressSpace::findJumpTargetFuncByAddr(Address addr) {

   Address addr2 = 0;
   func_instance *f = findOneFuncByAddr(addr);
   if (f)
      return f;

   if (!findObject(addr)) return NULL;

   using namespace Dyninst::InstructionAPI;
   InstructionDecoder decoder((const unsigned char*)getPtrToInstruction(addr),
                              InstructionDecoder::maxInstructionLength,
                              getArch());
   Instruction curInsn = decoder.decode();
    
   Expression::Ptr target = curInsn.getControlFlowTarget();
   RegisterAST thePC = RegisterAST::makePC(getArch());
   target->bind(&thePC, Result(u32, addr));
   Result cft = target->eval();
   if(cft.defined)
   {
      switch(cft.type)
      {
         case u32:
            addr2 = cft.val.u32val;
            break;
         case s32:
            addr2 = cft.val.s32val;
            break;
         default:
            assert(!"Not implemented for non-32 bit CFTs yet!");
            break;
      }
   }
   return findOneFuncByAddr(addr2);
}

AstNodePtr AddressSpace::trampGuardAST() {
   if (!trampGuardBase_) {
      // Don't have it yet....
      return AstNodePtr();
   }

   if (trampGuardAST_) return trampGuardAST_;

   trampGuardAST_ = AstNode::operandNode(AstNode::operandType::variableAddr, trampGuardBase_->ivar());
   return trampGuardAST_;
}


trampTrapMappings::trampTrapMappings(AddressSpace *a) :
   needs_updating(false),
   as(a),
   trapTableUsed(NULL),
   trapTableVersion(NULL),
   trapTable(NULL),
   trapTableSorted(NULL),
   table_version(0),
   table_used(0),
   table_allocated(0),
   table_mutatee_size(0),
   current_table(0x0),
   table_header(0x0),
   blockFlushes(false)
{
}

void trampTrapMappings::copyTrapMappings(trampTrapMappings *parent)
{
   needs_updating = parent->needs_updating;
   trapTableUsed = NULL;
   trapTableVersion = NULL;
   trapTable = NULL;
   trapTableSorted = NULL;
   table_version = parent->table_version;
   table_used = parent->table_used;
   table_allocated = parent->table_allocated;
   table_mutatee_size = parent->table_mutatee_size;
   current_table = parent->current_table;
   mapping = parent->mapping;
}

void trampTrapMappings::clearTrapMappings()
{
   needs_updating = false;
   trapTableUsed = NULL;
   trapTableVersion = NULL;
   trapTable = NULL;
   trapTableSorted = NULL;
   table_version = 0;
   table_used = 0;
   table_allocated = 0;
   table_mutatee_size = 0;
   current_table = 0;
   mapping.clear();
}

void trampTrapMappings::addTrapMapping(Address from, Address to, 
                                       bool write_to_mutatee)
{
#if defined(arch_x86) || defined(arch_x86_64)
   //x86 traps occur at +1 addr
   from++;
#endif
   tramp_mapping_t m;
   bool existing_trap = (mapping.count(from) != 0);
   m.from_addr = from;
   m.to_addr = to;
   m.written = false;
   m.cur_index = existing_trap ? mapping[from].cur_index : INDEX_INVALID;
   m.mutatee_side = write_to_mutatee;
   mapping[from] = m;
#if defined(cap_mutatee_traps)
   updated_mappings.insert(& mapping[from]);
   if (write_to_mutatee && !existing_trap) {
      table_mutatee_size++;
   }
   needs_updating = true;
#endif
#if defined(arch_x86) || defined(arch_x86_64)
   from--;
#endif
}

bool trampTrapMappings::definesTrapMapping(Address from)
{
   return mapping.count(from) != 0;
}

Address trampTrapMappings::getTrapMapping(Address from)
{
   if (!mapping.count(from))
      return 0;
   return mapping[from].to_addr;
}

bool trampTrapMappings::needsUpdating()
{
   return needs_updating;
}

bool trampTrapMappings::empty() {
   return mapping.empty();
}


AddressSpace *trampTrapMappings::proc() const {
   return as;
}

bool mapping_sort(const trampTrapMappings::tramp_mapping_t *lhs,
                  const trampTrapMappings::tramp_mapping_t *rhs)
{
   return lhs->from_addr < rhs->from_addr;
}

void trampTrapMappings::writeToBuffer(unsigned char *buffer, unsigned long val,
                                      unsigned addr_width)
{
   //Deal with the case when mutatee word size != mutator word size
   if (addr_width != sizeof(Address)) {
      //Currently only support 64-bit mutators with 32-bit mutatees
      assert(addr_width == 4);
      assert(sizeof(Address) == 8);
#if defined(cap_32_64)
      assert(val <= numeric_limits<uint32_t>::max() && "val more than 32 bits");
      write_memory_as(buffer, static_cast<uint32_t>(val));
      return;
#endif
   }
   write_memory_as(buffer, static_cast<uint64_t>(val));
}

void trampTrapMappings::writeTrampVariable(const int_variable *var, 
                                           unsigned long val)
{
   unsigned char buffer[16];
   unsigned aw = proc()->getAddressWidth();

   writeToBuffer(buffer, val, aw);
   bool result = proc()->writeDataSpace((void *) var->getAddress(), aw, buffer);
   assert(result);
}
                  
void trampTrapMappings::arrange_mapping(tramp_mapping_t &m, bool should_sort,
                                        std::vector<tramp_mapping_t*> &mappings_to_add,
                                        std::vector<tramp_mapping_t*> &mappings_to_update)
{
   if (!m.mutatee_side || (m.written && !should_sort))
      return;
   m.written = true;
   if (should_sort || m.cur_index == INDEX_INVALID)
      mappings_to_add.push_back(&m);
   else if (m.cur_index != INDEX_INVALID)
      mappings_to_update.push_back(&m);
}

void trampTrapMappings::flush() {
   if (!needs_updating || blockFlushes)
      return;

   set<mapped_object *> &rtlib = proc()->runtime_lib;

   //We'll sort addresses in the binary rewritter (when writting only happens
   // once and we may do frequent lookups)
   //If we're using the dynamic instrumentor and creating a new table we might
   // as well sort it.
   //If we're just adding a few entries to the table which already fit, then
   // we'll just append them to the end of the table.
   //
   //If we're sorting, then everytime we update we'll generate a whole new table
   //If we're not sorting, then each update will just append to the end of the
   // table.
   bool should_sort = (dynamic_cast<PCProcess *>(proc()) == NULL ||
                       table_mutatee_size > table_allocated);

   if (should_sort) {
      table_used = 0; //We're rebuilding the table, nothing's used.
   }

   /**
    * Fill in the mappings_to_add and mappings_to_update vectors.
    * As an optimization, we keep a list of the mappings that have
    * changed in updated_mappings.  If we're not completly regenerating
    * the table we'll get our trap list out of updated_mappings, 
    * otherwise we'll get our change list out of the entire dyn_hash_map.
    **/
   std::vector<tramp_mapping_t*> mappings_to_add;
   std::vector<tramp_mapping_t*> mappings_to_update;
   if (should_sort) {
      dyn_hash_map<Address, tramp_mapping_t>::iterator i;
      for (i = mapping.begin(); i != mapping.end(); i++) {
         arrange_mapping((*i).second, should_sort, 
                         mappings_to_add, mappings_to_update);
      }
   } 
   else {
      std::set<tramp_mapping_t *>::iterator i;
      for (i = updated_mappings.begin(); i != updated_mappings.end(); i++) {
         arrange_mapping(**i, should_sort, 
                         mappings_to_add, mappings_to_update);
      }
   }
   updated_mappings.clear();

   assert(mappings_to_add.size() + table_used == table_mutatee_size);

   for (unsigned k=0; k<mappings_to_add.size(); k++)
   {
      mappings_to_add[k]->written = true;
   }

   //Sort the mappings (if needed) 
   if (should_sort) 
      std::sort(mappings_to_add.begin(), mappings_to_add.end(), mapping_sort);

   // Assign the cur_index field of each entry in the new mappings we're adding
   for (unsigned j=0; j<mappings_to_add.size(); j++) {
      mappings_to_add[j]->cur_index = table_used + j;
   }
   
   //Each table entry has two pointers.
   unsigned entry_size = proc()->getAddressWidth() * 2;

   allocateTable();

   //Add any new entries to the table
   unsigned char *buffer = NULL;
   if (mappings_to_add.size()) {
      //Create a buffer containing the new entries we're going to write.
      unsigned long bytes_to_add = mappings_to_add.size() * entry_size;
      buffer = (unsigned char *) malloc(bytes_to_add);
      assert(buffer);
      
      unsigned char *cur = buffer;
      std::vector<tramp_mapping_t*>::iterator j;
      for (j = mappings_to_add.begin(); j != mappings_to_add.end(); j++) {
         tramp_mapping_t &tm = **j;
         writeToBuffer(cur, tm.from_addr, proc()->getAddressWidth());
         cur += proc()->getAddressWidth();
         writeToBuffer(cur, tm.to_addr, proc()->getAddressWidth());
         cur += proc()->getAddressWidth();
      }
      assert(cur == buffer + bytes_to_add);
      
      //Write the new entries into the process
      Address write_addr = current_table + (table_used * entry_size);
      bool result = proc()->writeDataSpace((void *) write_addr, bytes_to_add, 
                                           buffer);
      assert(result);
      free(buffer);
      buffer = NULL;

      table_used += mappings_to_add.size();
   }

   //Now we get to update existing entries that have been modified.
   if (mappings_to_update.size()) {
      assert(!should_sort);
      unsigned aw = proc()->getAddressWidth();
      buffer = (unsigned char *) malloc(aw);
      assert(buffer);

      //For each entry, use its cur_index field to figure out where in the
      // process it is, and write it.
      //We only need to update the to_addr, since this is an update of an
      // existing from_addr
      std::vector<tramp_mapping_t*>::iterator j;
      for (j = mappings_to_update.begin(); j != mappings_to_update.end(); j++) {
         tramp_mapping_t &tm = **j;
         writeToBuffer(buffer, tm.to_addr, aw);

         Address write_addr = current_table + (tm.cur_index * entry_size) + aw;
         bool result = proc()->writeDataSpace((void *) write_addr, aw, buffer);
         assert(result);
      }
      
      free(buffer);
      buffer = NULL;
   }

   //This function just keeps going... Now we need to take all of those 
   // mutatee side variables and update them.
   if (dynamic_cast<PCProcess *>(proc())) 
   {
      if (!trapTable) {
         //Lookup all variables that are in the rtlib
         set<mapped_object *>::iterator rtlib_it;
         for(rtlib_it = rtlib.begin(); rtlib_it != rtlib.end(); ++rtlib_it) {
            if( !trapTableUsed ) trapTableUsed = (*rtlib_it)->getVariable("dyninstTrapTableUsed");
            if( !trapTableVersion ) trapTableVersion = (*rtlib_it)->getVariable("dyninstTrapTableVersion");
            if( !trapTable ) trapTable = (*rtlib_it)->getVariable("dyninstTrapTable");
            if( !trapTableSorted ) trapTableSorted = (*rtlib_it)->getVariable("dyninstTrapTableIsSorted");
         }
         
         if (!trapTableUsed) {
            fprintf(stderr, "Dyninst is about to crash with an assert.  Either your dyninstAPI_RT library is stripped, or you're using an older version of dyninstAPI_RT with a newer version of dyninst.  Check your DYNINSTAPI_RT_LIB enviroment variable.\n");
         }
         assert(trapTableUsed);
         assert(trapTableVersion);
         assert(trapTable);
         assert(trapTableSorted);
      }
   
      writeTrampVariable(trapTableUsed, table_used);
      writeTrampVariable(trapTableVersion, ++table_version);
      writeTrampVariable(trapTable, (unsigned long) current_table);
      writeTrampVariable(trapTableSorted, should_sort ? 1 : 0);
   }

   needs_updating = false;
}

void trampTrapMappings::allocateTable()
{
   unsigned entry_size = proc()->getAddressWidth() * 2;

   if (dynamic_cast<PCProcess *>(proc()))
   {
      //Dynamic rewriting

      //Allocate the space for the tramp mapping table, or make sure that enough
      // space already exists.
      if (table_mutatee_size > table_allocated) {
         //Free old table
         if (current_table) {
            proc()->inferiorFree(current_table);
         }
         
         //Calculate size of new table
         table_allocated = (unsigned long) (table_mutatee_size * 1.5);
         if (table_allocated < MIN_TRAP_TABLE_SIZE)
            table_allocated = MIN_TRAP_TABLE_SIZE;
         
         //allocate
         current_table = proc()->inferiorMalloc(table_allocated * entry_size);
         assert(current_table);
      }
      return;
   }

   //Static rewriting
   BinaryEdit *binedit = dynamic_cast<BinaryEdit *>(proc());
   assert(!current_table);
   assert(binedit);
   
   table_allocated = (unsigned long) table_mutatee_size;
   table_header = proc()->inferiorMalloc(table_allocated * entry_size + 
                                         sizeof(trap_mapping_header));
   trap_mapping_header header;
   memset(&header, 0, sizeof(header));
   header.signature = TRAP_HEADER_SIG;
   header.num_entries = table_mutatee_size;
   header.pos = -1;

   bool result = proc()->writeDataSpace((void *) table_header, 
                                        sizeof(trap_mapping_header),
                                        &header);
   assert(result);   
   current_table = table_header + sizeof(trap_mapping_header);

   SymtabAPI::Symtab *symtab = 
      binedit->getMappedObject()->parse_img()->getObject();
   if( !symtab->isStaticBinary() ) {
       symtab->addSysVDynamic(DT_DYNINST, table_header);
       symtab->addLibraryPrereq(proc()->dyninstRT_name);
#if defined (os_windows)
      symtab->addTrapHeader_win((Address)table_header);
#endif
   }
}

bool AddressSpace::findFuncsByAddr(Address addr, std::set<func_instance*> &funcs, bool includeReloc)
{
   if (includeReloc) {
      RelocInfo ri;
      if (getRelocInfo(addr, ri)) {

         if (proc() && BPatch_defensiveMode == proc()->getHybridMode()) {
            // check that the block & function still exist
            mapped_object *obj = findObject(ri.orig);
            if (!obj) {
               return false;
            }
            std::set<block_instance*> blocks;
            if (!obj->findBlocksByAddr(ri.orig, blocks)) {
               return false; // block no longer exists
            }
         }
         if (ri.func) {
            // We cloned for some reason. Nifty.
            funcs.insert(ri.func);
         }
         else {
            // We copied a block sans function context, so...
            // grab everyone
            ri.block->getFuncs(std::inserter(funcs, funcs.end()));
         }
         return true;
      }
   }
   mapped_object *obj = findObject(addr);
   if (!obj) return false;
   return obj->findFuncsByAddr(addr, funcs);
}

bool AddressSpace::findBlocksByAddr(Address addr, std::set<block_instance *> &blocks, bool includeReloc) {
   if (includeReloc) {
      RelocInfo ri;
      if (getRelocInfo(addr, ri)) {
         blocks.insert(ri.block);
         return true;
      }
   }

   mapped_object *obj = findObject(addr);
   if (!obj) return false;
   bool ret = obj->findBlocksByAddr(addr, blocks);
   if (ret || !includeReloc)
      return ret;
   return false;
}

func_instance *AddressSpace::findFuncByEntry(const block_instance *block) {
   mapped_object *obj = findObject(block->start());
   if (!obj) return NULL;
   return obj->findFuncByEntry(block);
}

block_instance *AddressSpace::findBlockByEntry(Address a) {
   mapped_object *obj = findObject(a);
   if (!obj) return NULL;
   return obj->findBlockByEntry(a);
}

func_instance *AddressSpace::findOneFuncByAddr(Address addr) {
   std::set<func_instance *> funcs;
   if (!findFuncsByAddr(addr, funcs)) return NULL;
   if (funcs.empty()) return NULL;
   if (funcs.size() == 1) return *(funcs.begin());
   // Arbitrarily pick one...
   Address last = 0;
   func_instance *ret = NULL;
   for (std::set<func_instance *>::iterator iter = funcs.begin();
        iter != funcs.end(); ++iter) {
      if (ret == NULL ||
          ((*iter)->entryBlock()->start() > last)) {
         ret = *iter;
         last = (*iter)->entryBlock()->start();
      }
   }
   return ret;
}

func_instance *AddressSpace::findFuncByEntry(Address addr) {
   std::set<func_instance *> funcs;
   if (!findFuncsByAddr(addr, funcs)) return NULL;
   if (funcs.empty()) return NULL;

   for (std::set<func_instance *>::iterator iter = funcs.begin();
        iter != funcs.end(); ++iter) {
      if ((*iter)->entryBlock()->start() == addr) {
         return *iter;
      }
   }
   return NULL;
}


bool AddressSpace::canUseTraps()
{
#if !defined(cap_mutatee_traps)
   return false;
#else
   return useTraps_;
#endif
}

void AddressSpace::setUseTraps(bool usetraps)
{
   useTraps_ = usetraps;
}

bool AddressSpace::needsPIC(int_variable *v)
{
   return needsPIC(v->mod()->proc());
}

bool AddressSpace::needsPIC(func_instance *f)
{
   return needsPIC(f->proc());
}

bool AddressSpace::needsPIC(AddressSpace *s)
{
   if (proc())
      return false; //Never PIC for dynamic
   if (this != s)
      return true; //Use PIC cross module
   return s->needsPIC(); //Use target module
}

bool AddressSpace::sameRegion(Address addr1, Address addr2)
{
   mapped_object *mobj = findObject(addr1);
   if (!mobj || mobj != findObject(addr2)) {
      return false;
   }
   Address baseAddr = mobj->codeBase();

   SymtabAPI::Region *reg1 = 
      mobj->parse_img()->getObject()->findEnclosingRegion( addr1 - baseAddr );

   if (!reg1 || reg1 != mobj->parse_img()->getObject()->
       findEnclosingRegion( addr2 - baseAddr )) {
      return false;
   }
   return true;
}
////////////////////////////////////////////////////////////////////////////////////////

void AddressSpace::modifyCall(block_instance *block, func_instance *newFunc, func_instance *context) {
   // Just register it for later code generation
   //callModifications_[block][context] = newFunc;
   mgr()->instrumenter()->modifyCall(block, newFunc, context);
   if (context) addModifiedFunction(context);
   else addModifiedBlock(block);
}

void AddressSpace::replaceFunction(func_instance *oldfunc, func_instance *newfunc) {
   mgr()->instrumenter()->replaceFunction(oldfunc, newfunc);
   addModifiedFunction(oldfunc);
}

bool AddressSpace::wrapFunction(func_instance *original, 
                                func_instance *wrapper,
                                SymtabAPI::Symbol *clone) {
   if (!original) return false;
   if (!wrapper) return false;
   if (!clone) return false;

   if (original->proc() != this) {
      return original->proc()->wrapFunction(original, wrapper, clone);
   }
   assert(original->proc() == this);


   // 1) Replace original with wrapper via entry point jumps;
   //    this is handled in the instrumenter. 
   // 2) Create a copy of original with the new provided name. 
   // 3) (binary editing): update the various Symtab tables
   //      with the new name.
   //    (process) replace any PLT stubs to the clone with intermodule
   //      branches to this copy. 

   // TODO: once we have PatchAPI updated a bit, break this into
   // steps 1-3. For now, keep it together. 
   mgr()->instrumenter()->wrapFunction(original, wrapper, clone->getMangledName());
   addModifiedFunction(original);

   wrappedFunctionWorklist_[original] = clone;

   return true;
}

void AddressSpace::wrapFunctionPostPatch(func_instance *func, Dyninst::SymtabAPI::Symbol *clone) {
   if (edit()) {
      func->addSymbolsForCopy();
   }
   else {
      Address newAddr = func->getWrapperSymbol()->getOffset();
      // We have copied the original function and given it the address
      // newAddr. We now need to update any references calling the clone
      // symbol and point them at newAddr. Effectively, we're acting as
      // a proactive loader. 
      
      for (unsigned i = 0; i < mapped_objects.size(); ++i) {
         // Need original to get intermodule working right. 
         mapped_objects[i]->replacePLTStub(clone, func, newAddr);
      }
   }
}

void AddressSpace::removeCall(block_instance *block, func_instance *context) {
  mgr()->instrumenter()->removeCall(block, context);
  if (context) addModifiedFunction(context);
  else addModifiedBlock(block);
}

void AddressSpace::revertCall(block_instance *block, func_instance *context) {
  /*
   if (callModifications_.find(block) != callModifications_.end()) {
      callModifications_[block].erase(context);
   }
  */
  mgr()->instrumenter()->revertModifiedCall(block, context);
  if (context) addModifiedFunction(context);
  else addModifiedBlock(block);
}

void AddressSpace::revertReplacedFunction(func_instance *oldfunc) {
  //functionReplacements_.erase(oldfunc);
  mgr()->instrumenter()->revertReplacedFunction(oldfunc);
  addModifiedFunction(oldfunc);
}

void AddressSpace::revertWrapFunction(func_instance *wrappedfunc) {
   // Undo the instrumentation component
   mgr()->instrumenter()->revertWrappedFunction(wrappedfunc);
   addModifiedFunction(wrappedfunc);
   wrappedFunctionWorklist_.erase(wrappedfunc);
}

const func_instance *AddressSpace::isFunctionReplacement(func_instance *func) const
{
    PatchAPI::FuncModMap repFuncs = mgr_->instrumenter()->funcRepMap();
    PatchAPI::FuncModMap::const_iterator frit = repFuncs.begin();
    for (; frit != repFuncs.end(); frit++) {
        if (func == frit->second) {
            return static_cast<const func_instance*>(frit->first);
        }
    }
    return NULL;
}


using namespace Dyninst;
using namespace Relocation;

bool AddressSpace::delayRelocation() const {
   return delayRelocation_;
}

bool AddressSpace::relocate() {
   if (delayRelocation()) return true;
   
   relocation_cerr << "ADDRSPACE::Relocate called; modified functions reports "
                   << modifiedFunctions_.size() << " objects to relocate." << endl;
  if (!mapped_objects.size()) {
    relocation_cerr << "WARNING: No mapped_object in this addressSpace!\n";
    return false;
  }

  bool ret = true;
  for (std::map<mapped_object *, FuncSet>::iterator iter = modifiedFunctions_.begin();
       iter != modifiedFunctions_.end(); ++iter) {
     FuncSet &modFuncs = iter->second;

     bool repeat = false;

     do { // add overlapping functions in a fixpoint calculation
        repeat = false;
        unsigned int num = modFuncs.size();
        FuncSet overlappingFuncs;
        for (FuncSet::iterator iter2 = modFuncs.begin(); iter2 != modFuncs.end(); ++iter2) {
//           block_instance *entry = (*iter2)->entryBlock();
//           entry->getFuncs(std::inserter(overlappingFuncs,overlappingFuncs.begin()));
           // Check whether any blocks in the function are are members of any other functions
            func_instance* curFunc = *iter2;
            for (auto iter3 = curFunc->blocks().begin(); iter3 != curFunc->blocks().end(); ++iter3) {
                block_instance* curBlock = SCAST_BI(*iter3);
                curBlock->getFuncs(std::inserter(overlappingFuncs,overlappingFuncs.begin()));
           }
        }
        modFuncs.insert(overlappingFuncs.begin(), overlappingFuncs.end());
        if (num < modFuncs.size()) {
           repeat = true;
        }
     } while (repeat);

     if (getArch() == Arch_ppc64) {
         // The PowerPC new ABI typically generate two entries per function.
         // Need special hanlding for them
         FuncSet actualModFuncs;
         for (auto fit = modFuncs.begin(); fit != modFuncs.end(); ++fit) {
             func_instance* funct = *fit;
             if (funct->getPowerPreambleFunc() != NULL) {
                 relocation_cerr << "Ignore function " << funct->name() << " at " << hex << funct->entryBlock()->GetBlockStartingAddress() << " as it has the power preabmle" << endl;
                 continue;
             }
             actualModFuncs.insert(funct);
             if (funct->ifunc()->containsPowerPreamble()) {
                 funct->entryBlock()->_ignorePowerPreamble = true;
             }
         }
         modFuncs = actualModFuncs;
     }

     Address middle = (iter->first->codeAbs() + (iter->first->imageSize() / 2));
     
     if (!relocateInt(iter->second.begin(), iter->second.end(), middle)) {
        ret = false;
     }
  }


     
  


  modifiedFunctions_.clear();

  for (std::map<func_instance *, Dyninst::SymtabAPI::Symbol *>::iterator foo = wrappedFunctionWorklist_.begin();
       foo != wrappedFunctionWorklist_.end(); ++foo) {
      wrapFunctionPostPatch(foo->first, foo->second);
  }
  wrappedFunctionWorklist_.clear();

  return ret;
}

// iter is some sort of functions
bool AddressSpace::relocateInt(FuncSet::const_iterator begin, FuncSet::const_iterator end, Address nearTo) {

  if (begin == end) {
    return true;
  }


  // Create a CodeMover covering these functions
  //cerr << "Creating a CodeMover" << endl;

  relocatedCode_.push_back(new CodeTracker());
  CodeMover::Ptr cm = CodeMover::create(relocatedCode_.back());
  if (!cm->addFunctions(begin, end)) return false;

  SpringboardBuilder::Ptr spb = SpringboardBuilder::createFunc(begin, end, this);

  relocation_cerr << "Debugging CodeMover (pre-transform)" << endl;
  relocation_cerr << cm->format() << endl;
  transform(cm);

  relocation_cerr << "Debugging CodeMover" << endl;
  relocation_cerr << cm->format() << endl;

  relocation_cerr << "  Entering code generation loop" << endl;
  Address baseAddr = generateCode(cm, nearTo);
  if (!baseAddr) {
    relocation_cerr << "  ERROR: generateCode returned baseAddr of " << baseAddr << ", exiting" << endl;
    return false;
  }

  if (dyn_debug_reloc || dyn_debug_write) {
      cerr << "DUMPING RELOCATION BUFFER" << endl;
      cerr << cm->gen().format() << endl;
  }

  // Copy it in
  relocation_cerr << "  Writing " << cm->size() << " bytes of data into program at "
		  << std::hex << baseAddr << std::dec << endl;
  if (!writeTextSpace((void *)baseAddr,
		      cm->size(),
		      cm->ptr()))
    return false;

  // Now handle patching; AKA linking
  relocation_cerr << "  Patching in jumps to generated code" << endl;

  if (!patchCode(cm, spb)) {
      relocation_cerr << "Error: patching in jumps failed, ret false!" << endl;
    return false;
  }

  // Build the address mapping index
  relocatedCode_.back()->createIndices();
    
  // Kevin's stuff
  cm->extractDefensivePads(this);

  if (proc()) {
      // adjust PC if active frame is in a modified function, this 
      // forces the instrumented version of the code to execute right 
      // away and is needed for code overwrites
      
      vector<PCThread *> threads;
      proc()->getThreads(threads);

      vector<PCThread *>::const_iterator titer;
      for (titer = threads.begin();
           titer != threads.end(); 
           titer++) 
      {
          // translate thread's active PC to orig addr
          Frame tframe = (*titer)->getActiveFrame();
          Address curAddr = tframe.getPC();

          Address orig = 0;
          block_instance *block = NULL;
          func_instance *func = NULL;
          unsigned offset = 0;

          // Fill in the above
          // First, check in instrumentation
          RelocInfo ri;
          if (getRelocInfo(curAddr, ri)) {
             orig = ri.orig;
             block = ri.block;
             func = ri.func;
             // HACK: if we're in the middle of an emulation block, add that
             // offset to where we transfer to. 
             TrackerElement *te = NULL;
             for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
                  iter != relocatedCode_.end(); ++iter) {
                te = (*iter)->findByReloc(curAddr);
                if (te) break;
             }
             
             if (te && te->type() == TrackerElement::emulated) {
                offset = curAddr - te->reloc();
                assert(offset < te->size());
             }
          } else {
             // In original code; do a slow and painful lookup. 
             orig = curAddr;
             mapped_object *obj = findObject(curAddr);
             if (!obj) break;
			 if(!(obj->parse_img()->isParsed())) break;
             block = obj->findOneBlockByAddr(curAddr);
             func = tframe.getFunc();
             offset = 0;
          }             
			if (!block || !func) continue;

          list<Address> relocPCs;
          getRelocAddrs(orig, block, func, relocPCs, true);
          mal_printf("Found %lu matches for address 0x%lx\n", relocPCs.size(), orig);
          if (!relocPCs.empty()) {
             (*titer)->changePC(relocPCs.back() + offset);
             mal_printf("Pulling active frame PC into newest relocation "
                        "orig[%lx], cur[%lx], new[%lx (0x%lx + 0x%x)]\n", orig, 
                        tframe.getPC(), relocPCs.back() + offset, relocPCs.back(), offset);
             break;
          }
      }
  }
  
  return true;
}

bool AddressSpace::transform(CodeMover::Ptr cm) {

   if (0 && proc() && BPatch_defensiveMode != proc()->getHybridMode()) {
       adhocMovementTransformer a(this);
       cm->transform(a);
   }
   else {
       PCSensitiveTransformer pc(this, cm->priorityMap());
        cm->transform(pc);
   }

  // Add instrumentation
  relocation_cerr << "Inst transformer" << endl;
  Instrumenter i;
  cm->transform(i);

   Modification mod(mgr()->instrumenter()->callModMap(),
                    mgr()->instrumenter()->funcRepMap(),
                    mgr()->instrumenter()->funcWrapMap());
   cm->transform(mod);

  return true;

}

Address AddressSpace::generateCode(CodeMover::Ptr cm, Address nearTo) {
  // And now we start the relocation process.
  // This is at heart an iterative process, using the following
  // algorithm
  // size = code size estimate
  // done = false
  // While (!done), do
  //   addr = inferiorMalloc(size)
  //   cm.relocate(addr)
  //   if ((cm.size <= size) || (inferiorRealloc(addr, cm.size)))
  //     done = true
  //   else
  //     size = cm.size
  //     inferiorFree(addr)
  // In effect, we keep trying until we get a code generation that fits
  // in the space we have allocated.
  Address baseAddr = 0;

  codeGen genTemplate;
  genTemplate.setAddrSpace(this);
  // Set the code emitter?
  
  if (!cm->initialize(genTemplate)) {
    return 0;
  }

  while (1) {
     relocation_cerr << "   Attempting to allocate " << cm->size() << "bytes" << endl;
    unsigned size = cm->size();
    if (!size) {
        // This can happen if the only thing being moved are control flow instructions
        // (or other things that are _only_ patches)
        // inferiorMalloc horks if we hand it zero, so make sure it's non-zero.
        size = 1;
    }
    baseAddr = inferiorMalloc(size, anyHeap, nearTo);
    
    
    relocation_cerr << "   Calling CodeMover::relocate" << endl;
    if (!cm->relocate(baseAddr)) {
       // Whoa
       relocation_cerr << "   ERROR: CodeMover failed relocation!" << endl;
       return 0;
    }
    
    // Either attempt to expand or shrink...
    relocation_cerr << "   Calling inferiorRealloc to fit new size " << cm->size() 
		    << ", current base addr is " 
		    << std::hex << baseAddr << std::dec << endl;
    if (!inferiorRealloc(baseAddr, cm->size())) {
      relocation_cerr << "   ... inferiorRealloc failed, trying again" << endl;
      inferiorFree(baseAddr);
      continue;
    }
    else {
      relocation_cerr << "   ... inferiorRealloc succeeded, finished" << endl;
      break;
    }
  }
  
  if (!cm->finalize()) {
     return 0;
  }

  //addrMap.debug();

  return baseAddr;
}

bool AddressSpace::patchCode(CodeMover::Ptr cm,
			     SpringboardBuilder::Ptr spb) {
   SpringboardMap &p = cm->sBoardMap(this);
  
  // A SpringboardMap has three priority sets: Required, Suggested, and
  // NotRequired. We care about:
  // Required: all
  // Suggested: function entries
  // NotRequired: none

  std::list<codeGen> patches;

  if (!spb->generate(patches, p)) {
      springboard_cerr << "Failed springboard generation, ret false" << endl;
    return false;
  }

  springboard_cerr << "Installing " << patches.size() << " springboards!" << endl;
  for (std::list<codeGen>::iterator iter = patches.begin();
       iter != patches.end(); ++iter) 
  {
      springboard_cerr << "Writing springboard @ " << hex << iter->startAddr() << endl;
      if (!writeTextSpace((void *)iter->startAddr(),
          iter->used(),
          iter->start_ptr())) 
      {
	springboard_cerr << "\t FAILED to write springboard @ " << hex << iter->startAddr() << endl;
         // HACK: code modification will make this happen...
         return false;
      }
  }

  return true;
}

void AddressSpace::getRelocAddrs(Address orig, 
                                 block_instance *block,
                                 func_instance *func,
                                 std::list<Address> &relocs,
                                 bool getInstrumentationAddrs) const {
  springboard_cerr << "getRelocAddrs for orig addr " << hex << orig << " /w/ block start " << block->start() << dec << endl;
  for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
       iter != relocatedCode_.end(); ++iter) {
    Relocation::CodeTracker::RelocatedElements reloc;
    //springboard_cerr << "\t Checking CodeTracker " << hex << *iter << dec << endl;
    if ((*iter)->origToReloc(orig, block, func, reloc)) {
      // Pick instrumentation if it's there, otherwise use the reloc instruction
       //springboard_cerr << "\t\t ... match" << endl;
       if (!reloc.instrumentation.empty() && getInstrumentationAddrs) {
          for (std::map<instPoint *, Address>::iterator iter2 = reloc.instrumentation.begin();
               iter2 != reloc.instrumentation.end(); ++iter2) {
             relocs.push_back(iter2->second);
          }
      }
      else {
        assert(reloc.instruction);
        relocs.push_back(reloc.instruction);
      }
    }
  }
}      

bool AddressSpace::getAddrInfo(Address relocAddr,
                               Address &origAddr,
                               vector<func_instance *> &origFuncs,
                               baseTramp *&baseT) 
{
   CodeTracker::RelocInfo ri;
   if (getRelocInfo(relocAddr, ri)) {
      origAddr = ri.orig;
      baseT = ri.bt;
      if (ri.func)
         origFuncs.push_back(ri.func);
      else {
         // We copied a block sans function context, so...
         // grab everyone
         ri.block->getFuncs(std::back_inserter(origFuncs));
      }
      return true;
   }      
   
   std::set<func_instance *> tmpFuncs;
   if (findFuncsByAddr(relocAddr, tmpFuncs)) {
      origAddr = relocAddr;
      origFuncs.insert(origFuncs.end(), tmpFuncs.begin(), tmpFuncs.end());

      baseT = NULL;
      return true;
   }
      
   return false;
}

// KEVINTODO: not clearing out entries when deleting code, and extremely slow in defensive mode, over 300 codetrackers for Yoda
bool AddressSpace::getRelocInfo(Address relocAddr,
                                RelocInfo &ri) {
  bool ret = false;
  // address is relocated (or bad), check relocation maps
  for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
       iter != relocatedCode_.end(); ++iter) {
     if ((*iter)->relocToOrig(relocAddr, ri)) {
        assert(!ret);
        ret = true;
     }
  }
  return ret;
}

bool AddressSpace::inEmulatedCode(Address addr) {
  // address is relocated (or bad), check relocation maps
  for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
       iter != relocatedCode_.end(); ++iter)  {
     TrackerElement *te = (*iter)->findByReloc(addr);
     if (te) {
        if (te->type() == TrackerElement::emulated ||
            te->type() == TrackerElement::instrumentation) {
           return true;
        }
     }
  }
  return false;
}

void AddressSpace::addModifiedFunction(func_instance *func) {
  assert(func->obj());

  modifiedFunctions_[func->obj()].insert(func);
}

void AddressSpace::addModifiedBlock(block_instance *block) {
   // TODO someday this will decouple from functions. Until
   // then...
   std::list<func_instance *> tmp;
   block->getFuncs(std::back_inserter(tmp));
   for (std::list<func_instance *>::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      addModifiedFunction(*iter);
   }
}


void AddressSpace::addDefensivePad(block_instance *callBlock, func_instance *callFunc,
                                   Address padStart, unsigned size) {
  // We want to register these in terms of a block_instance that the pad ends, but 
  // the CFG can change out from under us; therefore, for lookup we use an instPoint
  // as they are invariant. 
   instPoint *point = instPoint::preCall(callFunc, callBlock);
   if (!point) {
      mal_printf("Error: no preCall point for %s\n",
                 callBlock->long_format().c_str());
      return;
   }

   mal_printf("Adding pad for callBlock [%lx %lx), pad at 0%lx\n", 
              callBlock->start(), callBlock->end(), padStart);

   forwardDefensiveMap_[callBlock->last()][callFunc].insert(std::make_pair(padStart, size));
   std::pair<func_instance*,Address> padContext;
   padContext.first = callFunc;
   padContext.second = callBlock->last();
   reverseDefensiveMap_.insert(padStart, padStart+size, padContext);
}

void AddressSpace::getPreviousInstrumentationInstances(baseTramp *bt,
						       std::set<Address>::iterator &b,
						       std::set<Address>::iterator &e) {
  b = instrumentationInstances_[bt].begin();
  e = instrumentationInstances_[bt].end();
  return;
}

void AddressSpace::addInstrumentationInstance(baseTramp *bt,
					      Address a) {
  instrumentationInstances_[bt].insert(a);
}

void updateSrcListAndVisited(ParseAPI::Edge* e,
			     std::list<ParseAPI::Edge*>& srcList,
			     std::set<ParseAPI::Edge*>& visited)			     
{
  if (visited.find(e) == visited.end()) {
    srcList.push_back(e);
    visited.insert(e);
  }
}
			     
// create stub edge set which is: all edges such that: 
//     e->trg() in owBlocks and e->src() not in delBlocks, 
//     in which case, choose stub from among e->src()->sources()
std::map<func_instance*,vector<edgeStub> > 
AddressSpace::getStubs(const std::list<block_instance *> &owBlocks,
                       const std::set<block_instance*> &delBlocks,
                       const std::list<func_instance*> &deadFuncs)
{
    std::map<func_instance*,vector<edgeStub> > stubs;
    std::set<ParseAPI::Edge*> stubEdges;
    std::set<ParseAPI::Edge*> visited;

    for (list<block_instance*>::const_iterator bit = owBlocks.begin();
         bit != owBlocks.end(); 
         bit++) 
    {
        // if the overwritten block is in a dead func, we won't find a stub
        bool inDeadFunc = false;
        set<func_instance*> bFuncs;
        (*bit)->getFuncs(std::inserter(bFuncs,bFuncs.end()));
        for (list<func_instance*>::const_iterator dfit = deadFuncs.begin();
             dfit != deadFuncs.end(); dfit++) 
        {
           if (bFuncs.end() != bFuncs.find(*dfit)) {
              inDeadFunc = true;
              break;
           }
        }
        if (inDeadFunc) {
           mal_printf("block [%lx %lx) is in a dead function, will not reparse\n", 
                      (*bit)->start(), (*bit)->end());
           continue;
        }

        using namespace ParseAPI;
        bool foundStub = false;
        parse_block *curImgBlock = (*bit)->llb();
        Address base = (*bit)->start() - curImgBlock->firstInsnOffset();
        const Block::edgelist & sourceEdges = curImgBlock->sources();

        // search for stubs in all functions containing the overwritten block
        for (set<func_instance*>::iterator fit = bFuncs.begin();
             !foundStub && fit != bFuncs.end();
             fit++)
        {
            // build "srcList" worklist
            SingleContext epred_((*fit)->ifunc(),true,true);
            //Intraproc epred(&epred_);
            std::list<ParseAPI::Edge*> srcList;
	    std::for_each(boost::make_filter_iterator(epred_, sourceEdges.begin(), sourceEdges.end()),
			  boost::make_filter_iterator(epred_, sourceEdges.end(), sourceEdges.end()),
			  boost::bind(updateSrcListAndVisited,
				      boost::placeholders::_1,
				      boost::ref(srcList),
				      boost::ref(visited)));


            // find all stub blocks for this edge
            for (list<ParseAPI::Edge*>::iterator eit = srcList.begin(); eit != srcList.end(); eit++) {
                parse_block *isrc = (parse_block*)((*eit)->src());
                block_instance *src = (*fit)->obj()->findBlockByEntry(base + isrc->start());
                assert(src);

                if ( delBlocks.end() == delBlocks.find(src) ) {
                   if (stubEdges.find(*eit) == stubEdges.end()) {
                      edgeStub st(src, (*eit)->trg()->start() + base, (*eit)->type());
                      stubs[*fit].push_back(st);
                      foundStub = true;
                   }
                } 
                else {
                   const Block::edgelist &srcSrcs = isrc->sources();
		   std::for_each(boost::make_filter_iterator(epred_, srcSrcs.begin(), srcSrcs.end()),
				 boost::make_filter_iterator(epred_, srcSrcs.end(), srcSrcs.end()),
				 boost::bind(updateSrcListAndVisited,
						 boost::placeholders::_1,
					     boost::ref(srcList),
					     boost::ref(visited)));
		   

                }
            }
        }
    }
    return stubs;
}

/* PatchAPI Stuffs */
void AddressSpace::initPatchAPI() {
   DynAddrSpace* addr_space = DynAddrSpace::create();
   assert(addr_space);
   
   mgr_ = PatchMgr::create(addr_space,
                           new DynInstrumenter,
                           new DynPointMaker);
   
   patcher_ = Patcher::create(mgr_);
   
   assert(mgr());
   mgr()->instrumenter()->callModMap().clear();
   mgr()->instrumenter()->funcRepMap().clear();
   mgr()->instrumenter()->funcWrapMap().clear();
}

bool AddressSpace::patch(AddressSpace* as) {
   return as->patcher()->commit();
}

void AddressSpace::addMappedObject(mapped_object* obj) {
  mapped_objects.push_back(obj);
  dynamic_cast<DynAddrSpace*>(mgr_->as())->loadLibrary(obj);
}


bool uninstrument(Dyninst::PatchAPI::Instance::Ptr inst) {
   instPoint *point = IPCONV(inst->point());
   bool ret = point->remove(inst);
   if (!ret) return false;
   point->markModified();
   return true;

}


unsigned AddressSpace::getAddressWidth() const {
    if( mapped_objects.size() > 0 ) {
        return mapped_objects[0]->parse_img()->codeObject()->cs()->getAddressWidth();
    }
    // We can call this before we've attached...best effort guess
    return sizeof(Address);
}
