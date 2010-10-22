/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include "process.h"
#include "function.h"
#include "binaryEdit.h"
#include "miniTramp.h"
#include "baseTramp.h"

#include "instPoint.h"

// Two-level codeRange structure
#include "mapped_object.h"
#include "mapped_module.h"
#if defined(cap_instruction_api)
#include "InstructionDecoder.h"
#include "Instruction.h"
#else
#include "parseAPI/src/InstrucIter.h"
#endif //defined(cap_instruction_api)

#include "Relocation/CodeMover.h"
#include "Relocation/Springboard.h"
#include "Relocation/Transformers/Include.h"
#include "Relocation/CodeTracker.h"

// Implementations of non-virtual functions in the address space
// class.

AddressSpace::AddressSpace () :
    trapMapping(this),
    useTraps_(true),
    trampGuardBase_(NULL),
    up_ptr_(NULL),
    costAddr_(0)
{}

AddressSpace::~AddressSpace() {}

process *AddressSpace::proc() {
    return dynamic_cast<process *>(this);
}

BinaryEdit *AddressSpace::edit() {
    return dynamic_cast<BinaryEdit *>(this);
}

// Fork constructor - and so we can assume a parent "process"
// rather than "address space"
void AddressSpace::copyAddressSpace(process *parent) {
    deleteAddressSpace();

    // This is only defined for process->process copy
    // until someone can give a good reason for copying
    // anything else...

    assert(proc());

    // Mapped objects first
    for (unsigned i = 0; i < parent->mapped_objects.size(); i++) {
        mapped_object *par_obj = parent->mapped_objects[i];
        mapped_object *child_obj = new mapped_object(par_obj, proc());
        assert(child_obj);
        
        mapped_objects.push_back(child_obj);
        addOrigRange(child_obj);
        
        // This clones funcs, which then clone instPoints, which then 
        // clone baseTramps, which then clones miniTramps.
    }
    // Clone the tramp guard base
    trampGuardBase_ = new int_variable(parent->trampGuardBase_, getAOut()->getDefaultModule());
    

    /////////////////////////
    // Inferior heap
    /////////////////////////

    heap_ = inferiorHeap(parent->heap_);
    heapInitialized_ = parent->heapInitialized_;

    /////////////////////////
    // Trap mappings
    /////////////////////////
    trapMapping.copyTrapMappings(& (parent->trapMapping));
}

void AddressSpace::deleteAddressSpace() {
    // Methodically clear everything we have - it all went away
    // We have the following member variables:

    // bool heapInitialized_
    // inferiorHeap heap_
    // codeRangeTree textRanges_
    // codeRangeTree modifiedCodeRanges_

    heapInitialized_ = false;
    heap_.clear();

    // We can just clear the originalCodeRanges structure - everything else
    // is deleted elsewhere, and we won't leak.
    textRanges_.clear();

    // Delete only the wrapper objects
    pdvector<codeRange *> ranges;
    dataRanges_.elements(ranges);
    for (unsigned i = 0; i < ranges.size(); i++) {
        delete ranges[i];
    }
    ranges.clear();
    dataRanges_.clear();

    for (unsigned i = 0; i < ranges.size(); i++) {
        // This is either a:
        // instArea
        // replacedFunctionCall
        // replacedFunction

        // Either way, we can nuke 'em.
        delete ranges[i];
    }

    for (unsigned i = 0; i < mapped_objects.size(); i++) 
        delete mapped_objects[i];

    mapped_objects.clear();

    trampGuardBase_ = NULL;

    // up_ptr_ is untouched
    costAddr_ = 0;
}


// findRangeByAddr: finds the object (see below) that corresponds with
// a given absolute address. This includes:
//   Functions (non-relocated)
//   Base tramps
//   Mini tramps
//   Relocated functions
//
// The process class keeps a tree of objects occupying the address space.
// This top-level tree includes trampolines, relocated functions, and the
// application and shared objects. The search starts at the top of this tree.
// If the address resolves to a base tramp, mini tramp, or relocated function,
// that is returned. If the address resolves within the range of an shared
// object, the search recurses into the object (the offset into the object
// is calculated and the function lookup works from the offset). If the offset
// is within the a.out, we look up the function assuming the address given is
// the offset. 

// This function gives only "original" code - that is, code that either existed
// in the binary or that we added to unallocated space. If you're looking for places
// where things were overwritten (jumps to instrumentation, etc.) look at
// modifiedRanges_.

void AddressSpace::addOrigRange(codeRange *range) {
    textRanges_.insert(range);
#if 0
    if (range->is_mapped_object()) {
        // Hack... add data range
        mappedObjData *data = new mappedObjData(range->is_mapped_object());
        dataRanges_.insert(data);
    }
#endif
}

void AddressSpace::removeOrigRange(codeRange *range) {
    codeRange *tmp = NULL;
    
    if (!textRanges_.find(range->get_address(), tmp))
        return;

    assert (range == tmp);

    textRanges_.remove(range->get_address());
}


codeRange *AddressSpace::findOrigByAddr(Address addr) {
    codeRange *range = NULL;
    
    if (!textRanges_.find(addr, range)) {
        return NULL;
    }
    
    assert(range);
    
    bool in_range = (addr >= range->get_address() &&
                     addr <= (range->get_address() + range->get_size()));
    assert(in_range); // Supposed to return NULL if this is the case
    
    // The top level tree doesn't go into mapped_objects, which is not
    // what we want; so if we're in a mapped_object, poke inside.
    // However, if we're in a function (int_function), minitramp,
    // basetramp, ... return that right away.
    
    mapped_object *mobj = range->is_mapped_object();
    if (mobj) {
        codeRange *obj_range = mobj->findCodeRangeByAddress(addr);
        if (obj_range) range = obj_range;
    }
    
    return range;
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


bool AddressSpace::getOrigRanges(pdvector<codeRange *> &ranges) {
    textRanges_.elements(ranges);
    return true;
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
   pdvector<heapItem *> &freeList = heap_.heapFree;
   unsigned i, nbuf = freeList.size();

   /* sort buffers by address */
#if defined (cap_use_pdvector)
   VECTOR_SORT(freeList, heapItemCmpByAddr);
#else
   VECTOR_SORT(freeList, heapItemLessByAddr);
#endif


  /* combine adjacent buffers */
  bool needToCompact = false;
  for (i = 1; i < freeList.size(); i++) {
      heapItem *h1 = freeList[i-1];
      heapItem *h2 = freeList[i];
      assert(h1->length != 0);
      if (h1->addr + h1->length > h2->addr) {
          fprintf(stderr, "Error: heap 1 (%p) (0x%p to 0x%p) overlaps heap 2 (%p) (0x%p to 0x%p)\n",
                  h1,
                  (void *)h1->addr, (void *)(h1->addr + h1->length),
                  h2,
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
    pdvector<heapItem *> cleanList;
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
    pdvector<heapItem *> &freeList = heap_.heapFree;
    
    int best = -1;
    for (unsigned i = 0; i < freeList.size(); i++) {
        heapItem *h = freeList[i];
        // check if free block matches allocation constraints
        // Split out to facilitate debugging
        infmalloc_printf("%s[%d]: comparing heap %d: 0x%lx-0x%lx/%d to desired %d bytes in 0x%lx-0x%lx/%d\n",
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
    heap_.totalFreeMemAvailable += h2->length;

    if (h->dynamic) {
       addAllocatedRegion(h->addr, h->length);
    }
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
    heapItem *h = NULL;  
    return heap_.heapActive.find(block, h);
}

Address AddressSpace::inferiorMallocInternal(unsigned size,
                                             Address lo,
                                             Address hi,
                                             inferiorHeapType type) {
    infmalloc_printf("%s[%d]: inferiorMallocInternal, %d bytes, type %d, between 0x%lx - 0x%lx\n",
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
    heapItem *h = NULL;  
    if (!heap_.heapActive.find(block, h)) {
        // We can do this if we're at process teardown.
        return;
    }
    assert(h);
    
    // Remove from the active list
    heap_.heapActive.undef(block);
    
    // Add to the free list
    h->status = HEAPfree;
    heap_.heapFree.push_back(h);
    heap_.totalFreeMemAvailable += h->length;
    heap_.freed += h->length;
    infmalloc_printf("%s[%d]: Freed block from 0x%lx - 0x%lx, %d bytes, type %d\n",
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
#if defined (cap_dynamic_heap)
    // This is why it's not a reference...
    inferiorMallocAlign(newSize);
#endif
    
    infmalloc_printf("%s[%d]: inferiorRealloc for block 0x%lx, new size %d\n",
                     FILE__, __LINE__, block, newSize);

    // find block on active list
    heapItem *h = NULL;  
    if (!heap_.heapActive.find(block, h)) {
        // We can do this if we're at process teardown.
        infmalloc_printf("%s[%d]: inferiorRealloc unable to find block, returning\n", FILE__, __LINE__);
        return false;
    }
    assert(h);
    infmalloc_printf("%s[%d]: inferiorRealloc found block with addr 0x%lx, length %d\n",
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
        infmalloc_printf("%s[%d]: enlarging existing block; old 0x%lx - 0x%lx (%d), new 0x%lx - 0x%lx (%d)\n",
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
                             pdvector<int_function *> &res,
                             const std::string &libname) { // = "", btw
    
    unsigned starting_entries = res.size(); // We'll return true if we find something
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname.c_str() ||
            mapped_objects[i]->fullName() == libname.c_str()) {
            const pdvector<int_function *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
            if (pretty) {
                // We stop at first match...
                for (unsigned pm = 0; pm < pretty->size(); pm++) {
                    res.push_back((*pretty)[pm]);
                }
            }
            else {
                const pdvector<int_function *> *mangled = mapped_objects[i]->findFuncVectorByMangled(funcname);
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
                             pdvector<int_function *> &res,
                             const std::string &libname) { // = "", btw

    unsigned starting_entries = res.size(); // We'll return true if we find something

    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname.c_str() ||
            mapped_objects[i]->fullName() == libname.c_str()) {
            const pdvector<int_function *> *pretty = mapped_objects[i]->findFuncVectorByPretty(funcname);
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
                                 pdvector<int_function *> &res,
                                 const std::string &libname) { // = "", btw
    unsigned starting_entries = res.size(); // We'll return true if we find something

    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname.c_str() ||
            mapped_objects[i]->fullName() == libname.c_str()) {
            const pdvector<int_function *> *mangled = 
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

int_function *AddressSpace::findOnlyOneFunction(const string &name,
                                                const string &lib,
                                                bool /*search_rt_lib*/) 
{
    assert(mapped_objects.size());

    pdvector<int_function *> allFuncs;

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
                            pdvector<int_variable *> &res,
                            const std::string &libname) { // = "", btw
    unsigned starting_entries = res.size(); // We'll return true if we find something
    
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        if (libname == "" ||
            mapped_objects[i]->fileName() == libname.c_str() ||
            mapped_objects[i]->fullName() == libname.c_str()) {
            const pdvector<int_variable *> *pretty = mapped_objects[i]->findVarVectorByPretty(varname);
            if (pretty) {
                // We stop at first match...
                for (unsigned pm = 0; pm < pretty->size(); pm++) {
                    res.push_back((*pretty)[pm]);
                }
            }
            else {
                const pdvector<int_variable *> *mangled = mapped_objects[i]->findVarVectorByMangled(varname);
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
    codeRange *range;

    if (textRanges_.find(addr, range)) {
        return range->getPtrToInstruction(addr);
    }
    else if (dataRanges_.find(addr, range)) {
        mappedObjData *data = dynamic_cast<mappedObjData *>(range);
        assert(data);
        return data->obj->getPtrToData(addr);
    }
    fprintf(stderr,"[%s:%d] failed to find matching range for address %lx\n",
        FILE__,__LINE__,addr);
    assert(0);
    return NULL;
}

void *
AddressSpace::getPtrToData(const Address addr) const {
    codeRange *range = NULL;
    if(dataRanges_.find(addr,range)) {
        mappedObjData * data = dynamic_cast<mappedObjData *>(range);
        assert(data);
        return data->obj->getPtrToData(addr);
    }
    assert(0);
    return NULL;
}

bool AddressSpace::isCode(const Address addr) const {
    codeRange *dontcare;
    if (textRanges_.find(addr, dontcare))
        return true;
    else
        return false;
}
bool AddressSpace::isData(const Address addr) const {
    return !isCode(addr) && isValidAddress(addr);
}

bool AddressSpace::isValidAddress(const Address addr) const{
    // "Is this part of the process address space, and if so, 
    //  does it correspond to an address that we can get a 
    //  valid pointer to?"

    codeRange *range;
    if (textRanges_.find(addr, range)) {
        mapped_object *obj = range->is_mapped_object();

        if ( !obj ) 
            return true;
 
        if ( obj->parse_img()->getObject()->isCode(addr - obj->codeBase()) ||
             obj->parse_img()->getObject()->isData(addr - obj->dataBase())   )
            return true;
 
        return false;
    }

    if (dataRanges_.find(addr, range))
        return true;

    return false;
}        

mapped_object *AddressSpace::findObject(Address addr) {
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

int_function *AddressSpace::findFuncByAddr(Address addr) {
    codeRange *range = findOrigByAddr(addr);
    if (!range) return NULL;
    
    int_function *func_ptr = range->is_function();
    mapped_object *obj = range->is_mapped_object();

    if(func_ptr) {
       return func_ptr;
    }
    else if (obj) {
        if ( ! obj->isAnalyzed() ) {
            obj->analyze();
        }
        return obj->findFuncByAddr( addr );
    }
    else {
        return NULL;
    }
}

int_basicBlock *AddressSpace::findBasicBlockByAddr(Address addr) {
    codeRange *range = findOrigByAddr(addr);
    if (!range) return NULL;

    int_basicBlock *b = range->is_basicBlock();
    int_function *f = range->is_function();

    if(b) {
        return b;
    }
    else if(f) {
        return f->findBlockByAddr(addr);
    }
    else
        return NULL;
}

int_function *AddressSpace::findFuncByInternalFunc(image_func *ifunc) {
    assert(ifunc);
  
    // Now we have to look up our specialized version
    // Can't do module lookup because of DEFAULT_MODULE...
    pdvector<int_function *> possibles;
    if (!findFuncsByMangled(ifunc->symTabName().c_str(),
                            possibles))
        return NULL;

    assert(possibles.size());
  
    for (unsigned i = 0; i < possibles.size(); i++) {
        if (possibles[i]->ifunc() == ifunc) {
            return possibles[i];
        }
    }
    return NULL;
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
mapped_object *AddressSpace::findObject(const std::string &obj_name, bool wildcard)
{
    for(u_int j=0; j < mapped_objects.size(); j++){
        if (mapped_objects[j]->fileName() == obj_name.c_str() ||
            mapped_objects[j]->fullName() == obj_name.c_str() ||
           (wildcard &&
             (wildcardEquiv(obj_name, mapped_objects[j]->fileName()) ||
              wildcardEquiv(obj_name, mapped_objects[j]->fullName()))))
            return mapped_objects[j];
    }
    return NULL;
}

// findObject: returns the object associated with obj_name 
// This just iterates over the mapped object vector
mapped_object *AddressSpace::findObject(fileDescriptor desc)
{
    for(u_int j=0; j < mapped_objects.size(); j++){
       if (desc == mapped_objects[j]->getFileDesc()) 
            return mapped_objects[j];
    }
    return NULL;
}

// getAllFunctions: returns a vector of all functions defined in the
// a.out and in the shared objects

void AddressSpace::getAllFunctions(pdvector<int_function *> &funcs) {
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        mapped_objects[i]->getAllFunctions(funcs);
    }
}
      
// getAllModules: returns a vector of all modules defined in the
// a.out and in the shared objects

void AddressSpace::getAllModules(pdvector<mapped_module *> &mods){
    for (unsigned i = 0; i < mapped_objects.size(); i++) {
        const pdvector<mapped_module *> &obj_mods = mapped_objects[i]->getModules();
        for (unsigned j = 0; j < obj_mods.size(); j++) {
            mods.push_back(obj_mods[j]);
        }
    }
}

//Acts like findTargetFuncByAddr, but also finds the function if addr
// is an indirect jump to a function.
//I know this is an odd function, but darn I need it.
int_function *AddressSpace::findJumpTargetFuncByAddr(Address addr) {

    Address addr2 = 0;
    int_function *f = findFuncByAddr(addr);
    if (f)
        return f;

    codeRange *range = findOrigByAddr(addr);
    if (!range->is_mapped_object()) 
        return NULL;
#if defined(cap_instruction_api)
    using namespace Dyninst::InstructionAPI;
    InstructionDecoder decoder((const unsigned char*)getPtrToInstruction(addr),
            InstructionDecoder::maxInstructionLength,
            getArch());
    Instruction::Ptr curInsn = decoder.decode();
    
    Expression::Ptr target = curInsn->getControlFlowTarget();
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
#else
    InstrucIter ii(addr, this);
    if (ii.isAJumpInstruction())
        addr2 = ii.getBranchTargetAddress();
#endif //defined(cap_instruction_api)
    return findFuncByAddr(addr2);
}

AstNodePtr AddressSpace::trampGuardAST() {
    if (!trampGuardBase_) {
        // Don't have it yet....
        return AstNodePtr();
    }

    if (trampGuardAST_) return trampGuardAST_;

    trampGuardAST_ = AstNode::operandNode(AstNode::variableAddr, trampGuardBase_->ivar());
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

#if defined(cap_32_64)
void trampTrapMappings::writeToBuffer(unsigned char *buffer, unsigned long val,
                                      unsigned addr_width)
{
   //Deal with the case when mutatee word size != mutator word size
   if (addr_width != sizeof(Address)) {
      //Currently only support 64-bit mutators with 32-bit mutatees
      assert(addr_width == 4);
      assert(sizeof(Address) == 8);
      *((uint32_t *) buffer) = (uint32_t) val;
      return;
   }
   *((unsigned long *) buffer) = val;
}
#else
void trampTrapMappings::writeToBuffer(unsigned char *buffer, unsigned long val,
                                      unsigned)
{
   *((unsigned long *)(void*) buffer) = val;
}
#endif


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
   bool should_sort = (dynamic_cast<process *>(proc()) == NULL ||
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

   Address write_addr = 0x0;

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
      write_addr = current_table + (table_used * entry_size);
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
   if (dynamic_cast<process *>(proc())) 
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

   if (dynamic_cast<process *>(proc()))
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
   header.signature = TRAP_HEADER_SIG;
   header.num_entries = table_mutatee_size;
   header.pos = -1;
   header.low_entry = 0;
   header.high_entry = 0;

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
   }
}

// only works for unrelocated addresses
bool AddressSpace::findFuncsByAddr(Address addr, vector<int_function*> &funcs)
{
   codeRange *range = findOrigByAddr(addr);
   if (!range)
      return false;

   bblInstance *bblInst = range->is_basicBlockInstance();
   if (!bblInst) {
       return false;
   }
   
   if (!bblInst->block()->llb()->isShared()) {
       funcs.push_back(bblInst->func());
       return true;
   }


   // the function is shared, get the multiple functions from the image block, 
   // convert to int_functions and return them.
   image_basicBlock *img_block = bblInst->block()->llb();
   vector<ParseAPI::Function *> img_funcs;
   img_block->getFuncs(img_funcs);
   assert(img_funcs.size());
   vector<ParseAPI::Function *>::iterator fit = img_funcs.begin();
   for( ; fit != img_funcs.end(); ++fit) {
       int_function *int_func = findFuncByInternalFunc((image_func*)(*fit));
       funcs.push_back(int_func);
   }

   return true;
}

bool AddressSpace::canUseTraps()
{
   BinaryEdit *binEdit = dynamic_cast<BinaryEdit *>(this);
   if (binEdit && binEdit->getMappedObject()->parse_img()->getObject()->isStaticBinary())
   	return false;

#if !defined(cap_mutatee_traps)
   return false;
#endif
   
   return useTraps_;
}

void AddressSpace::setUseTraps(bool usetraps)
{
   useTraps_ = usetraps;
}

bool AddressSpace::needsPIC(int_variable *v)
{
   return needsPIC(v->mod()->proc());
}

bool AddressSpace::needsPIC(int_function *f)
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
    Address loadAddr = mobj->parse_img()->desc().loadAddr();

    SymtabAPI::Region *reg1 = 
        mobj->parse_img()->getObject()->findEnclosingRegion( addr1 - loadAddr );

    if (!reg1 || reg1 != mobj->parse_img()->getObject()->
                         findEnclosingRegion( addr2 - loadAddr )) {
        return false;
    }
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////

void AddressSpace::replaceFunctionCall(instPoint *point, int_function *newFunc) {
  // Just register it for later code generation
  callReplacements_[point] = newFunc;
  addModifiedFunction(point->func());
}

void AddressSpace::replaceFunction(int_function *oldfunc, int_function *newfunc) {
  functionReplacements_[oldfunc] = newfunc;
  addModifiedFunction(oldfunc);
}

void AddressSpace::removeFunctionCall(instPoint *point) {
  callRemovals_.insert(point);
  addModifiedFunction(point->func());
}

// Why not be able to revert?
void AddressSpace::revertReplacedCall(instPoint *point) {
  callReplacements_.erase(point);
  // TODO: need a "remove modified function"
}
void AddressSpace::revertReplacedFunction(int_function *oldfunc) {
  functionReplacements_.erase(oldfunc);
}
void AddressSpace::revertRemovedFunctionCall(instPoint *point) {
  callRemovals_.erase(point);
}


using namespace Dyninst;
using namespace Relocation;

bool AddressSpace::relocate() {
  relocation_cerr << "ADDRSPACE::Relocate called!" << endl;
  bool ret = true;
  for (std::map<mapped_object *, FuncSet>::const_iterator iter = modifiedFunctions_.begin();
       iter != modifiedFunctions_.end(); ++iter) {
    assert(iter->first);

    if (!relocateInt(iter->second.begin(), iter->second.end(), iter->first->codeAbs())) {
      ret = false;
    }
    addModifiedRegion(iter->first);
  }

  updateMemEmulator();

  modifiedFunctions_.clear();
  return ret;
}

// iter is some sort of functions
bool AddressSpace::relocateInt(FuncSet::const_iterator begin, FuncSet::const_iterator end, Address nearTo) {
  if (begin == end) {
    return true;
  }

  // Create a CodeMover covering these functions
  //cerr << "Creating a CodeMover" << endl;
  
  // Attempting to reduce copies...
  CodeTracker t;
  relocatedCode_.push_back(t);
  CodeMover::Ptr cm = CodeMover::create(relocatedCode_.back());
  if (!cm->addFunctions(begin, end)) return false;

  SpringboardBuilder::Ptr spb = SpringboardBuilder::createFunc(begin, end, this);

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
      using namespace InstructionAPI;
      // Print out the buffer we just created
      cerr << "DUMPING RELOCATION BUFFER " << hex 
           << cm->blockMap().begin()->first->firstInsnAddr() << dec << endl;
      Address base = baseAddr;
      InstructionDecoder deco
        (cm->ptr(),cm->size(),getArch());
      Instruction::Ptr insn = deco.decode();
      while(insn) {
        cerr << "\t" << hex << base << ": " << insn->format() << endl;
        base += insn->size();
        insn = deco.decode();
      }
      cerr << dec;
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
      cerr << "Error: patching in jumps failed, ret false!" << endl;
    return false;
  }

  // Build the address mapping index
  relocatedCode_.back().createIndices();
    
  // Kevin's stuff
  cm->extractDefensivePads(this);

  if (proc() && BPatch_defensiveMode == proc()->getHybridMode()) {
      // adjust PC if active frame is in a modified function,
      // this is an important case for code overwrites
      vector<dyn_thread*>::const_iterator titer;
      for (titer=proc()->threads.begin();
           titer != proc()->threads.end(); 
           titer++) 
      {
          // translate thread's active PC to orig addr
          Frame tframe = (*titer)->getActiveFrame();
          Address pcOrig=0;
          vector<int_function *> origFuncs;
          baseTrampInstance *bti=NULL;
          if (! getAddrInfo(tframe.getPC(), pcOrig, origFuncs, bti)) {
              continue;
          }
          int_function *origFunc;
          if (origFuncs.size() == 1) {
              origFunc = origFuncs[0];
          } else {
              mal_printf("WARNING: active pc %lx is in a shared function we've"
                         " modified but we don't know which, stackwalking to "
                         "find out %s[%d]\n", pcOrig, FILE__,__LINE__);
              origFunc = proc()->findActiveFuncByAddr(pcOrig);
          }
          // if the PC matches a modified function, change the PC
          for (FuncSet::const_iterator fit = begin; fit != end; fit++) {
              if ((*fit)->findBlockInstanceByAddr(pcOrig)) {
                  list<Address> relocPCs;
                  getRelocAddrs(pcOrig, origFunc, relocPCs, false);
                  (*titer)->get_lwp()->changePC(relocPCs.back(),NULL);
                  mal_printf("Pulling active frame PC into newest relocation "
                             "orig[%lx]cur[%lx]new[%lx] %s[%d]\n", pcOrig, 
                             tframe.getPC(), relocPCs.back(),FILE__,__LINE__);
                  break;
              }
          }
      }
  }

  return true;
}

bool AddressSpace::transform(CodeMover::Ptr cm) {
  // Ensure each block ends with an appropriate CFElement
  CFAtomCreator c;
  cm->transform(c);

  //cerr << "Applying PCSens transformer" << endl;
  //PCSensitiveTransformer v(this, cm->priorityMap());
  //cm->transform(v);

  adhocMovementTransformer a(this);
  cm->transform(a);

  //cerr << "Memory emulator" << endl;
  //MemEmulatorTransformer m;
  //cm->transform(m);

  // Insert whatever binary modifications are desired
  // Right now needs to go before Instrumenters because we use
  // instrumentation for function replacement.
  Modification mod(callReplacements_, functionReplacements_, callRemovals_);
  cm->transform(mod);

  // Localize control transfers
  //cerr << "  Applying control flow localization" << endl;
  LocalizeCF t(cm->blockMap(), 
	       cm->priorityMap());
  cm->transform(t);

  // Add instrumentation
  // For ease of edge instrumentation this should occur post-LocalCFTransformer-age
  //cerr << "Inst transformer" << endl;
  Instrumenter i;
  cm->transform(i);

  // And remove unnecessary jumps. This needs to be last.
  Fallthroughs f;
  cm->transform(f);

  // Kevin's stuff
  Defensive d(cm->priorityMap());
  cm->transform(d);

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
    baseAddr = inferiorMalloc(cm->size(), anyHeap, nearTo);
    relocation_cerr << "   inferiorMalloc returned " 
		    << std::hex << baseAddr << std::dec << endl;


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

  relocation_cerr << "   ... fixpoint finished, returning baseAddr " 
		  << std::hex << baseAddr << std::dec << endl;

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
      cerr << "Failed springboard generation, ret false" << endl;
    return false;
  }

  for (std::list<codeGen>::iterator iter = patches.begin();
       iter != patches.end(); ++iter) {
    if (!writeTextSpace((void *)iter->startAddr(),
			iter->used(),
            iter->start_ptr())) {
        cerr << "Failed writing a springboard branch, ret false" << endl;
            return false;
    }
    }


  return true;
};

void AddressSpace::causeTemplateInstantiations() {
}

void AddressSpace::getRelocAddrs(Address orig, 
                                 int_function *func,
                                 std::list<Address> &relocs,
                                 bool getInstrumentationAddrs) const {
  for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
       iter != relocatedCode_.end(); ++iter) {
    Relocation::CodeTracker::RelocatedElements reloc;
    if (iter->origToReloc(orig, func, reloc)) {
      // Pick instrumentation if it's there, otherwise use the reloc instruction
      if (reloc.instrumentation && getInstrumentationAddrs) {
        relocs.push_back(reloc.instrumentation);
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
				vector<int_function *> &origFuncs,
				baseTrampInstance *&baseT) 
{
    // retrieve if unrelocated address
    bblInstance *bbi = findOrigByAddr(relocAddr)->is_basicBlockInstance();
    if ( bbi ) {
        origAddr = relocAddr;
        findFuncsByAddr(origAddr, origFuncs);
        baseT = NULL;
        return true;
    }

    // retrieve if relocated address
    int_function *func;
    if (getRelocInfo(relocAddr, origAddr, func, baseT)) {
        origFuncs.push_back(func);
        return true;
    }

    return false;

}


bool AddressSpace::getRelocInfo(Address relocAddr,
				Address &origAddr,
				int_function *&origFunc,
				baseTrampInstance *&baseT) 
{
  baseT = NULL;
  origFunc = NULL;

  // address is relocated (or bad), check relocation maps
  for (CodeTrackers::const_iterator iter = relocatedCode_.begin();
       iter != relocatedCode_.end(); ++iter) 
  {
     if (iter->relocToOrig(relocAddr, origAddr, origFunc, baseT)) {
        return true;
     }
  }

  return false;
}

void AddressSpace::addModifiedFunction(int_function *func) {
  assert(func->obj());

  modifiedFunctions_[func->obj()].insert(func);
}

void AddressSpace::addDefensivePad(bblInstance *callBlock, Address padStart, unsigned size) {
  // We want to register these in terms of a bblInstance that the pad ends, but 
  // the CFG can change out from under us; therefore, for lookup we use an instPoint
  // as they are invariant. 
   
   instPoint *point = callBlock->func()->findInstPByAddr(callBlock->lastInsnAddr());
   if (!point) {
       callBlock->func()->funcCalls();
       point = callBlock->func()->findInstPByAddr(callBlock->lastInsnAddr());
   }
   if (!point) {
       // Kevin didn't instrument it so we don't care :)
       return;
   }
   
   forwardDefensiveMap_[point].insert(std::make_pair(padStart, size));
   reverseDefensiveMap_.insert(padStart, padStart+size, point);
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

void AddressSpace::updateMemEmulator() {
   // 1) Create shadow copies for any MappedObject we
   // have modified.
   // 2) Update the runtime's MemoryMapper structure
   // to correspond to this.

   std::vector<int_variable *> memoryMapperTable;
   if (!findVarsByAll("RTmemoryMapper", memoryMapperTable)) {
      //assert(0);
      return; // For testing with static rewriter
   }

   // First step: nonblocking synchro.
   int guardValue;
   readDataSpace((void *)memoryMapperTable[0]->getAddress(),
                 sizeof(int),
                 &guardValue,
                 false);
   guardValue++;
   writeDataSpace((void *)memoryMapperTable[0]->getAddress(),
                  sizeof(int),
                  &guardValue);
   
   cerr << "UpdateMemEmulator: writing guard value " << guardValue << endl;

   // 64->32 bit is annoying...
   if (getAddressWidth() == 4) {
      struct MemoryMapper32 newMapper;
      
      readDataSpace((void *)memoryMapperTable[0]->getAddress(),
                    sizeof(newMapper),
                    &newMapper,
                    false);

      // First step: 
      newMapper.guard1 = guardValue;
      newMapper.guard2 = guardValue;
      newMapper.size = memoryMapTree.size();
      cerr << "\t new values: " << newMapper.guard1 << "/" << newMapper.guard2 << "/" << newMapper.size << endl;
      std::vector<MemoryMapTree::Entry> elements;
      memoryMapTree.elements(elements);
      for (unsigned i = 0; i < elements.size(); ++i) {
         newMapper.elements[i].lo = elements[i].first.first;
         newMapper.elements[i].hi = elements[i].first.second;
         assert(newMapper.elements[i].hi > newMapper.elements[i].lo);
         newMapper.elements[i].shift = elements[i].second;
         cerr << "\t\t Element: " << hex << newMapper.elements[i].lo << "->" << newMapper.elements[i].hi << ": " << newMapper.elements[i].shift << dec << endl;
      }
      writeDataSpace((void *)memoryMapperTable[0]->getAddress(),
                     sizeof(newMapper),
                     &newMapper);
   }
   else {
      // TODO copy
      //assert(0);
   }
}

void AddressSpace::addAllocatedRegion(Address start, unsigned size) {
   if (size == 0) return;

   Address end = start + size;
   assert(end > start);

   // Okay. For efficiency, we want to merge this if possible with an existing
   // range. We do this because our allocation tends to be contiguous.
   // Two options: we're immediately above an existing range or we're immediately
   // below. Check both. 

   Address lb, ub;
   long val;
   if (memoryMapTree.find(start, lb, ub, val)) {
      // Check to see if it's another allocated range
      if (val != -1) {
         // Not an allocated range (we use -1 to indicate that),
         // so insert and finish
         memoryMapTree.insert(start, end, -1);
         return;
      }

      if (start == ub) {
         memoryMapTree.remove(lb);
         memoryMapTree.insert(lb, end, -1);
         return;
      }

      // Otherwise start is within the range. Oops.
      assert(0);
   }
   if (memoryMapTree.find(end, lb, ub, val)) {
      if (val != -1) {
         memoryMapTree.insert(start, end, -1);
         return;
      }
      if (end == lb) {
         memoryMapTree.remove(lb);
         memoryMapTree.insert(start, ub, -1);
         return;
      }
      assert(0);
   }

   memoryMapTree.insert(start, end, -1);
   return;
}

void AddressSpace::addModifiedRegion(mapped_object *obj) {

   // We just add mapped objects, so if we've already added this one then nifty.
   Address lb, ub;
   long val;
   if (memoryMapTree.find(obj->codeAbs(), lb, ub, val)) {
      // Done
      return;
   }
   Address base = createShadowCopy(obj);
   memoryMapTree.insert(obj->codeAbs(),
                        obj->codeAbs() + obj->imageSize(),
                        obj->codeAbs() - base);
   return;
}

Address AddressSpace::createShadowCopy(mapped_object *obj) {
   // TODO
   return obj->codeAbs();
}
