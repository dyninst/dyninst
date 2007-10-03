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

// $Id: addressSpace.C,v 1.6 2007/10/03 21:18:16 bernat Exp $

#include "addressSpace.h"
#include "codeRange.h"
#include "process.h"
#include "binaryEdit.h"

#include "miniTramp.h"
#include "baseTramp.h"
#include "multiTramp.h" // multiTramp and instArea
#include "instPoint.h"

// Two-level codeRange structure
#include "mapped_object.h"

#include "InstrucIter.h"

// Implementations of non-virtual functions in the address space
// class.

AddressSpace::AddressSpace() :
    trampTrapMapping(addrHash4),
    multiTrampsById_(intHash),
    trampGuardBase_(0),
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
        if (!child_obj) {
            delete child_obj;
        }
        
        mapped_objects.push_back(child_obj);
        addOrigRange(child_obj);
        
        // This clones funcs, which then clone instPoints, which then 
        // clone baseTramps, which then clones miniTramps.
    }
    

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

    trampGuardBase_ = 0;

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
    
    if (!textRanges_.find(range->get_address_cr(), tmp))
        return;

    assert (range == tmp);

    textRanges_.remove(range->get_address_cr());
}

void AddressSpace::removeModifiedRange(codeRange *range) {
    codeRange *tmp = NULL;
    
    if (!modifiedRanges_.find(range->get_address_cr(), tmp))
        return;

    assert (range == tmp);

    modifiedRanges_.remove(range->get_address_cr());
}

codeRange *AddressSpace::findOrigByAddr(Address addr) {
    codeRange *range = NULL;
    
    if (!textRanges_.find(addr, range)) {
        return NULL;
    }
    
    assert(range);
    
    bool in_range = (addr >= range->get_address_cr() &&
                     addr <= (range->get_address_cr() + range->get_size_cr()));
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
    
    bool in_range = (addr >= range->get_address_cr() &&
                     addr <= (range->get_address_cr() + range->get_size_cr()));
    assert(in_range); // Supposed to return NULL if this is the case

    return range;
}

// Returns the named symbol from the image or a shared object
bool AddressSpace::getSymbolInfo( const pdstring &name, Symbol &ret ) 
{
  for (unsigned i = 0; i < mapped_objects.size(); i++) {
    bool sflag;
    sflag = mapped_objects[i]->getSymbolInfo( name, ret );
    
    if( sflag ) {
      return true;
    }
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
    if (!multi) return;

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
    Address currAddr = rep->get_address_cr();
    codeRange *range = findModByAddr(currAddr);
    while (range) {
        //overwrittenObjs.push_back(range);
        removeModifiedRange(range);

        currAddr += range->get_size_cr();
        range = findModByAddr(currAddr);
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
    codeRange *range = findModByAddr(repcall->get_address_cr());
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

//////////////////////////////////////////////////////////////////////////////
// Memory allocation routines
//////////////////////////////////////////////////////////////////////////////


void AddressSpace::inferiorFreeCompact() {
  pdvector<heapItem *> &freeList = heap_.heapFree;
  unsigned i, nbuf = freeList.size();

  /* sort buffers by address */
  VECTOR_SORT(freeList, heapItemCmpByAddr);

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

void AddressSpace::inferiorFree(Address block) {
    // find block on active list
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
}

void AddressSpace::inferiorMallocAlign(unsigned &size) {
    // Align to the process word
#if !defined(arch_ia64)
    unsigned alignment = (getAddressWidth() - 1);
#else
    unsigned alignment = (128 - 1);
#endif

    size = (size + alignment) & ~alignment;
}
    
bool AddressSpace::inferiorRealloc(Address block, unsigned newSize) {
#if defined (cap_dynamic_heap)
    // This is why it's not a reference...
    inferiorMallocAlign(newSize);
#endif
    
    // find block on active list
    heapItem *h = NULL;  
    if (!heap_.heapActive.find(block, h)) {
        // We can do this if we're at process teardown.
        return false;
    }
    assert(h);
    
    if (h->length < newSize)
        return false; // Cannot make bigger...
    if (h->length == newSize)
        return true;
    
    // We make a new "free" block that is the end of this one.
    Address freeStart = block + newSize;
    int shrink = h->length - newSize;
    
    assert(shrink > 0);
    
    h->length = newSize;
    
    heapItem *freeEnd = new heapItem(freeStart,
                                     shrink,
                                     h->type,
                                     h->dynamic,
                                     HEAPfree);
    heap_.heapFree.push_back(freeEnd);
    
    heap_.totalFreeMemAvailable += shrink;
    heap_.freed += shrink;
    
    // And run a compact; otherwise we'll end up with hugely fragmented memory.
    inferiorFreeCompact();
    
    return true;
}

/////////////////////////////////////////
// Function lookup...
/////////////////////////////////////////

bool AddressSpace::findFuncsByAll(const pdstring &funcname,
                             pdvector<int_function *> &res,
                             const pdstring &libname) { // = "", btw
    
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


bool AddressSpace::findFuncsByPretty(const pdstring &funcname,
                             pdvector<int_function *> &res,
                             const pdstring &libname) { // = "", btw

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


bool AddressSpace::findFuncsByMangled(const pdstring &funcname,
                                 pdvector<int_function *> &res,
                                 const pdstring &libname) { // = "", btw
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
                                           const string &lib) 
{
    assert(mapped_objects.size());

    pdvector<int_function *> allFuncs;
    if (!findFuncsByAll(name.c_str(), allFuncs, lib.c_str()))
        return NULL;
    if (allFuncs.size() > 1) {
        cerr << "Warning: multiple matches for " << name << ", returning first" << endl;
    }
    return allFuncs[0];
}

/////////////////////////////////////////
// Variable lookup...
/////////////////////////////////////////

bool AddressSpace::findVarsByAll(const pdstring &varname,
                            pdvector<int_variable *> &res,
                            const pdstring &libname) { // = "", btw
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

void *AddressSpace::getPtrToInstruction(Address addr) const {
    codeRange *range;

    if (textRanges_.find(addr, range)) {
        return range->getPtrToInstruction(addr);
    }
    else if (dataRanges_.find(addr, range)) {
        mappedObjData *data = dynamic_cast<mappedObjData *>(range);
        assert(data);
        return data->obj->getPtrToData(addr);
    }
    return NULL;
}

bool AddressSpace::isValidAddress(const Address& addr) const{
    // "Is this part of the process address space?"
    codeRange *dontcare;
    if (textRanges_.find(addr, dontcare))
        return true;
    if (dataRanges_.find(addr, dontcare))
        return true;
    fprintf(stderr, "Warning: address 0x%p not valid!\n",
            (void *)addr);
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

    if(func_ptr) {
       return func_ptr;
    }
    else if (multi) {
        return multi->func();
    }
    else if (mti) {
        return mti->baseTI->multiT->func();
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
mapped_module *AddressSpace::findModule(const pdstring &mod_name, bool wildcard)
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
mapped_object *AddressSpace::findObject(const pdstring &obj_name, bool wildcard)
{
    for(u_int j=0; j < mapped_objects.size(); j++){
        if (mapped_objects[j]->fileName() == obj_name.c_str() ||
            mapped_objects[j]->fullName() == obj_name.c_str() ||
            (wildcard &&
             (obj_name.wildcardEquiv(mapped_objects[j]->fileName().c_str()) ||
              obj_name.wildcardEquiv(mapped_objects[j]->fullName().c_str()))))
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
    
    InstrucIter ii(addr, this);
    if (ii.isAJumpInstruction())
        addr2 = ii.getBranchTargetAddress();

    return findFuncByAddr(addr2);
}

AstNodePtr AddressSpace::trampGuardAST() {
    if (!trampGuardBase_) {
        // Don't have it yet....
        return AstNodePtr();
    }
    if (trampGuardAST_) return trampGuardAST_;

    trampGuardAST_ = AstNode::operandNode(AstNode::DataAddr, 
                                          (void *)trampGuardBase_);

    return trampGuardAST_;
}

// Add it at the bottom...
void AddressSpace::deleteGeneratedCode(generatedCodeObject *delInst)
{
#if 0
    fprintf(stderr, "Deleting generated code %p, which is a:\n",
            delInst);
    if (dynamic_cast<multiTramp *>(delInst)) {
        fprintf(stderr, "   multiTramp\n");
    }
    else if (dynamic_cast<baseTrampInstance *>(delInst)) {
        fprintf(stderr, "   baseTramp\n");
    } 
    else if (dynamic_cast<miniTrampInstance *>(delInst)) {
        fprintf(stderr, "   miniTramp\n");
    }
    else {
        fprintf(stderr, "   unknown\n");
    }
#endif    

    delete delInst;
}
