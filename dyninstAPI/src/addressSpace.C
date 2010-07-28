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
#include "multiTramp.h" // multiTramp and instArea
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

// Implementations of non-virtual functions in the address space
// class.

AddressSpace::AddressSpace () :
    trapMapping(this),
    useTraps_(true),
    multiTrampsById_(intHash),
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
    // Instrumentation (multiTramps on down)
    /////////////////////////

    pdvector<codeRange *> ranges;
    parent->modifiedRanges_.elements(ranges);

    for (unsigned i = 0; i < ranges.size(); i++) {
        instArea *area = dynamic_cast<instArea *>(ranges[i]);
        replacedFunctionCall *rfc = dynamic_cast<replacedFunctionCall *>(ranges[i]);
        // Not used yet
        //functionReplacement *fr = dynamic_cast<functionReplacement *>(ranges[i]);
        
       if (area) {
            // Create a new multiTramp and add it
            multiTramp *pMulti = area->multi;
            multiTramp *cMulti = new multiTramp(pMulti, proc());
            addMultiTramp(cMulti);
        }
        else if (rfc) {
            replacedFunctionCall *cRFC = new replacedFunctionCall((*rfc));
            addReplacedCall(cRFC);
        }
#if 0
// Apparently this was never implemented... it probably should be, though.
        else if (fr) {
            functionReplacement *cFR = new functionReplacement(cFR);
            addFuncReplacement(cFR);
        }
#endif
    }
    // That will also create all baseTramps, miniTramps, ...

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
    // dictionary_hash multiTrampsById_

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

    // Modified code ranges we have to take care of. 
    modifiedRanges_.elements(ranges);
    modifiedRanges_.clear();

    for (unsigned i = 0; i < ranges.size(); i++) {
        // This is either a:
        // instArea
        // replacedFunctionCall
        // replacedFunction

        // Either way, we can nuke 'em.
        delete ranges[i];
    }

    multiTrampsById_.clear();

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
    if (range->is_mapped_object()) {
        // Hack... add data range
        mappedObjData *data = new mappedObjData(range->is_mapped_object());
        dataRanges_.insert(data);
    }
}

void AddressSpace::addModifiedRange(codeRange *range) {
    modifiedRanges_.insert(range);
}

void AddressSpace::removeOrigRange(codeRange *range) {
    codeRange *tmp = NULL;
    
    if (!textRanges_.find(range->get_address(), tmp))
        return;

    assert (range == tmp);

    textRanges_.remove(range->get_address());
}

void AddressSpace::removeModifiedRange(codeRange *range) {
    codeRange *tmp = NULL;
    
    if (dynamic_cast<multiTramp*>(range) && 
        dynamic_cast<multiTramp*>(range)->getIsActive()) 
    {
        fprintf(stderr,"ERROR: removeModifiedRange tried to remove active "
                "multiTramp range [%lx %lx] %s[%d]\n",
                range->get_address(), range->get_address() + range->get_size(),
                FILE__,__LINE__);
        assert(0);
    }

    if (!modifiedRanges_.find(range->get_address(), tmp))
        return;

    assert (range == tmp);

    modifiedRanges_.remove(range->get_address());

    instArea *area = dynamic_cast<instArea *>(range);
    if (area) {
      // We have just removed a multiTramp. If the dictionary
      // entry is the same as the instArea pointer, remove it
      // from the dictionary as well so we can't accidentally
      // access it that way.

      // If the pointers aren't equal, squawk because that shouldn't
      // happen.
      
       if (area->multi) {
          if (multiTrampsById_[area->multi->id()] == area->multi)
             multiTrampsById_[area->multi->id()] = NULL;
          else {
             fprintf(stderr, "%s[%u]: Warning: odd case in removing instArea\n",
                     FILE__, __LINE__);
          }
      }
    }
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

codeRange *AddressSpace::findModByAddr(Address addr) {
    codeRange *range = NULL;
    
    if (!modifiedRanges_.find(addr, range)) {
        return NULL;
    }
    
    assert(range);
    
    bool in_range = (addr >= range->get_address() &&
                     addr <= (range->get_address() + range->get_size()));
    assert(in_range); // Supposed to return NULL if this is the case

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

bool AddressSpace::getModifiedRanges(pdvector<codeRange *> &ranges) {
    modifiedRanges_.elements(ranges);
    return true;
}

multiTramp *AddressSpace::findMultiTrampByAddr(Address addr) {    
    codeRange *range = findModByAddr(addr);
    if (range) {
        instArea *area = dynamic_cast<instArea *>(range);
        
        if (area)
            return area->multi;
    }
    range = findOrigByAddr(addr);

    multiTramp *multi = dynamic_cast<multiTramp *>(range);
    return multi;
}

multiTramp *AddressSpace::findMultiTrampById(unsigned int id) {
    multiTramp *multi = NULL;
    multiTrampsById_.find(id, multi);

    return multi;
}

// Check to see if we're replacing an earlier multiTramp,
// add to the unmodifiedAreas list
void AddressSpace::addMultiTramp(multiTramp *multi) {
    assert(multi);
    assert(multi->instAddr());

    // Actually... we haven't copied it yet, so don't add anything. 
    //addOrigRange(multi);

    codeRange *range = findModByAddr(multi->instAddr());
    if (range) {
        // We're overriding. Keep the instArea but update pointer.
        instArea *area = dynamic_cast<instArea *>(range);
        // It could be something else, which should have been
        // caught already
        if (!area) {
            // Oops, someone already here... and multiTramps have
            // the lowest priority.
            return;
        }
        area->multi = multi;
    }
    else {
        instArea *area = new instArea(multi);

        modifiedRanges_.insert(area);
    }

    multiTrampsById_[multi->id()] = multi;
}

void AddressSpace::removeMultiTramp(multiTramp *multi) {

    if (!multi || multi->getIsActive()) {
        fprintf(stderr,"ERROR: Trying to remove active or NULL multitramp %lx "
                "[%lx %lx], should not be doing this %s[%d]\n", 
                multi->instAddr(), multi->getAddress(), 
                multi->getAddress() + multi->get_size(), FILE__,__LINE__);
        assert(0);
    }

    assert(multi->instAddr());

    removeOrigRange(multi);

    // For multiTramps we also have the "wrapper" that
    // represents the jump to the multiTramp. If that's
    // still associated with this one, then nuke it
    // as well. If not, ignore.

    instArea *jump = dynamic_cast<instArea *>(findModByAddr(multi->instAddr()));

    if (jump && jump->multi == multi) {
        removeModifiedRange(jump);
        delete jump;
    }
}

functionReplacement *AddressSpace::findFuncReplacement(Address addr) {
    codeRange *range = findModByAddr(addr);
    if (!range) return NULL;
    
    functionReplacement *rep = dynamic_cast<functionReplacement *>(range);
    return rep;
}

void AddressSpace::addFuncReplacement(functionReplacement *rep) {
    assert(rep);
    Address currAddr = rep->get_address();

    while (currAddr < (rep->get_address() + rep->get_size())) {
        codeRange *range = findModByAddr(currAddr);

        if (range) removeModifiedRange(range);
        currAddr += 1;
    }

    addModifiedRange(rep);
}

void AddressSpace::removeFuncReplacement(functionReplacement *rep) {
    removeModifiedRange(rep);
}

replacedFunctionCall *AddressSpace::findReplacedCall(Address addr) {
    codeRange *range = findModByAddr(addr);
    if (!range) return NULL;
    
    replacedFunctionCall *rep = dynamic_cast<replacedFunctionCall *>(range);
    return rep;
}

void AddressSpace::addReplacedCall(replacedFunctionCall *repcall) {
    codeRange *range = findModByAddr(repcall->get_address());
    if (range) {
        // Can't replace instrumentation right now...
        assert(dynamic_cast<replacedFunctionCall *>(range));
        removeModifiedRange(range);
        delete range;
    }
    assert(repcall);
    addModifiedRange(repcall);
}

void AddressSpace::removeReplacedCall(replacedFunctionCall *repcall) {
    removeModifiedRange(repcall);
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
    
    if (h->length < newSize) {
        return false; // Cannot make bigger...
    }
    if (h->length == newSize)
        return true;

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
    // "Is this part of the process address space?"
    codeRange *dontcare;
    if (textRanges_.find(addr, dontcare))
        return true;
    if (dataRanges_.find(addr, dontcare))
        return true;
    return false;
}        

mapped_object *AddressSpace::findObject(Address addr) {
    for (unsigned i=0; i<mapped_objects.size(); i++)
    {
        if (addr >= mapped_objects[i]->codeAbs() &&
            addr < mapped_objects[i]->codeAbs() + mapped_objects[i]->imageSize())
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
    multiTramp *multi = range->is_multitramp();
    miniTrampInstance *mti = range->is_minitramp();
    mapped_object *obj = range->is_mapped_object();

    if(func_ptr) {
       return func_ptr;
    }
    else if (multi) {
        return multi->func();
    }
    else if (mti) {
        return mti->baseTI->multiT->func();
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
    multiTramp *mt = range->is_multitramp();
    miniTrampInstance *mti = range->is_minitramp();

    if(b) {
        return b;
    }
    else if(f) {
        return f->findBlockByAddr(addr);
    }
    else if(mt) {
        return mt->func()->findBlockByAddr(addr);
    }
    else if(mti) {
        return mti->baseTI->multiT->func()->findBlockByAddr(addr);
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

// Add it at the bottom...
void AddressSpace::deleteGeneratedCode(generatedCodeObject *delInst)
{
    delete delInst;
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

bool AddressSpace::findFuncsByAddr(Address addr, std::vector<int_function *> &funcs)
{
   codeRange *range = findOrigByAddr(addr);
   if (!range)
      return false;

   bblInstance *bblInst = NULL;
   int_basicBlock *int_block = NULL;
   int_function *int_func = NULL;
   image_basicBlock *img_block = NULL;

   bblInst = range->is_basicBlockInstance();
   if (bblInst) {
      int_block = bblInst->block();
      if (int_block->origInstance() != bblInst) {
         //If we're not the original instance of a block, then this
         // is a relocated copy.  Since we un-share when relocating
         // we only have one function to return.
         int_func = int_block->func();
         funcs.push_back(int_func);
         return true;
      }
   }

   if (!int_block)
      int_block = range->is_basicBlock();
   if (int_block) 
   {
      //int_basicBlocks are unique to a function and don't contain
      // sharing information.  Move down to the image_basicBlock and
      // use that in the next if statement to look up sharing.
      img_block = int_block->llb();
   }

   if (!img_block) 
      img_block = range->is_image_basicBlock();
   if (img_block)
   {
      //Get the multiple functions from the image block, convert to
      // int_functions and return them.
      vector<ParseAPI::Function *> img_funcs;
      img_block->getFuncs(img_funcs);
      assert(img_funcs.size());
      vector<ParseAPI::Function *>::iterator fit = img_funcs.begin();
      for( ; fit != img_funcs.end(); ++fit) {
         int_func = findFuncByInternalFunc(static_cast<image_func*>(*fit));
         funcs.push_back(int_func);
      }
      return true;
   }

   assert(!range->is_function());
   return false;
   
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
