
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
 
// $Id: function.C,v 1.10 2005/03/02 19:44:45 bernat Exp 

#include "function.h"
#include "process.h"
#include "instPoint.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include "parseAPI/src/InstrucIter.h"
#include "MemoryEmulator/memEmulator.h"
#include "Relocation/Transformers/Movement-analysis.h"

//std::string int_function::emptyString("");

#include "Parsing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::Relocation;

int int_function_count = 0;

// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
    ifunc_(f),
    mod_(mod),
    handlerFaultAddr_(0),
    handlerFaultAddrAddr_(0), 
    isBeingInstrumented_(false)
#if defined(os_windows) 
   , callingConv(unknown_call)
   , paramSize(0)
#endif
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_function_count++;
    if ((int_function_count % 1000) == 0)
        fprintf(stderr, "int_function_count: %d (%d)\n",
                int_function_count, int_function_count*sizeof(int_function));
#endif

    addr_ = f->getOffset() + baseAddr;
    ptrAddr_ = (f->getPtrOffset() ? f->getPtrOffset() + baseAddr : 0);

    parsing_printf("%s: creating new proc-specific function at 0x%lx\n",
                   symTabName().c_str(), addr_);

     // We delay the creation of instPoints until they are requested;
    // this saves memory, and really, until something is asked for we
    // don't need it.  TODO: creation of an arbitrary instPoint should
    // trigger instPoint creation; someone may create an arbitrary at
    // a entry/exit/callsite.

    // Same with the flowGraph; we clone it from the image_func when
    // we need it.
    
    /* IA-64: create the cached allocs lazily. */
}

int_function::int_function(const int_function *parFunc,
                           mapped_module *childMod,
                           process *childP) :
    addr_(parFunc->addr_),
    ptrAddr_(parFunc->ptrAddr_),
    ifunc_(parFunc->ifunc_),
    mod_(childMod),
    handlerFaultAddr_(0),
    handlerFaultAddrAddr_(0), 
    isBeingInstrumented_(parFunc->isBeingInstrumented_)
 {
     unsigned i; // Windows hates "multiple definitions"

     // Construct the raw blocks_;
     for (BlockSet::const_iterator bIter = parFunc->blocks_.begin();
          bIter != parFunc->blocks_.end(); bIter++) 
     {
         createBlockFork((*bIter));
     }
     
     for (i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         entryPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->exitPoints_.size(); i++) {
         instPoint *parP = parFunc->exitPoints_[i];
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         exitPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->callPoints_.size(); i++) {
         instPoint *parP = parFunc->callPoints_[i];
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         callPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->arbitraryPoints_.size(); i++) {
         instPoint *parP = parFunc->arbitraryPoints_[i];
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         arbitraryPoints_.push_back(childIP);
     }

     set<instPoint*>::const_iterator pIter;
     for(pIter = parFunc->unresolvedPoints_.begin(); 
         pIter != parFunc->unresolvedPoints_.end(); pIter++) 
     {
         instPoint *parP = *pIter;
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         unresolvedPoints_.insert(childIP);
     }

     for(pIter = parFunc->abruptEnds_.begin(); 
         pIter != parFunc->abruptEnds_.end(); pIter++) 
     {
         instPoint *parP = *pIter;
         int_block *block = findBlockByEntry(parP->block()->start());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         abruptEnds_.insert(childIP);
     }
}

int_function::~int_function() { 
    // ifunc_ doesn't keep tabs on us, so don't need to let it know.
    // mod_ is cleared by the mapped_object
    // blocks_ isn't allocated

    // instPoints are process level (should be deleted here and refcounted)
    // DEMO: incorrectly delete instPoints here

    // some points are in multiple categories, keep the pointers around
    // in delPoints so we don't double-free
    std::set<instPoint*> delPoints;

    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        delPoints.insert(entryPoints_[i]);
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        delPoints.insert(exitPoints_[i]);
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        delPoints.insert(callPoints_[i]);
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        delPoints.insert(arbitraryPoints_[i]);
    }
    set<instPoint*>::iterator pIter = unresolvedPoints_.begin();
    for(; pIter != unresolvedPoints_.end(); ++pIter) {
        delPoints.insert(*pIter);
    }
    for (pIter = abruptEnds_.begin(); pIter != abruptEnds_.end(); ++pIter) {
        delPoints.insert(*pIter);
    }
    for (pIter = delPoints.begin(); delPoints.end() != pIter; ++pIter) {
       delete (*pIter);
    }

    // int_blocks
    BlockSet::iterator bIter = blocks_.begin();
    for (; bIter != blocks_.end(); bIter++) {
        delete *bIter;
    }

    for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
      
}

int_block *int_function::createBlock(image_basicBlock *ib) {
    int_block *block = new int_block(ib, this);
    blocks_.insert(block);
    blockMap_[ib] = block;

       
    return block;
}

int_block *int_function::createBlockFork(const int_block *parent)
{
    int_block *block = new int_block(parent, this);
    blocks_.insert(block);
    blockMap_[parent->llb()] = block;
    return block;
}

Address int_function::baseAddr() const {
    return obj()->codeBase();
}


// This needs to go away: how is "size" defined? Used bytes? End-start?

unsigned int_function::getSize_NP()  {
    blocks();
    if (blocks_.size() == 0) return 0;
            
    return ((*blocks_.rbegin())->end() - 
            (*blocks_.begin())->start());
}

void int_function::addArbitraryPoint(instPoint *insp) {
    arbitraryPoints_.push_back(insp);
}

const pdvector<instPoint *> &int_function::funcEntries() {
    pdvector<image_instPoint *> img_entries;
    if (entryPoints_.size() == 0 || obj()->isExploratoryModeOn()) {
        entryPoints_.clear();

        ifunc_->funcEntries(img_entries);
	    if (img_entries.empty()) {
	      cerr << "Warning: function " << prettyName() << " has no parsed entry points" << endl;
	    }
#if defined (cap_use_pdvector)
        entryPoints_.reserve_exact(img_entries.size());
#endif
        for (unsigned i = 0; i < img_entries.size(); i++) {
            instPoint *point = instPoint::createParsePoint(this,
                                                            img_entries[i]);
            if (!point) continue; // Can happen if we double-create
            assert(point);
            entryPoints_.push_back(point);
        }
#if defined (cap_use_pdvector)
        entryPoints_.reserve_exact(entryPoints_.size());
#endif
    }
    if (entryPoints_.size() != 1) {
       cerr << "Error: function " << prettyName() << ":" << obj()->fullName() <<" has " << entryPoints_.size() << " points!" << endl;
    }
    assert(entryPoints_.size() == 1);
    return entryPoints_;
}

const pdvector<instPoint*> &int_function::funcExits() {
    if (exitPoints_.size() == 0 || obj()->isExploratoryModeOn()) {
        exitPoints_.clear();
        pdvector<image_instPoint *> img_exits;
        ifunc_->funcExits(img_exits);
#if defined (cap_use_pdvector)
        exitPoints_.reserve_exact(img_exits.size());
#endif
        
        for (unsigned i = 0; i < img_exits.size(); i++) {
            instPoint *point = instPoint::createParsePoint(this,
                                                            img_exits[i]);
            if (!point) continue; // Can happen if we double-create

            assert(point);
            exitPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    exitPoints_.reserve_exact(exitPoints_.size());
#endif
    return exitPoints_;
}

const pdvector<instPoint*> &int_function::funcCalls() {
    if (callPoints_.size() == 0 || obj()->isExploratoryModeOn()) {
        //KEVINTODO: exploratory mode can be made more efficient by making 
        // callPoints a set and not clearing it in that mode
        callPoints_.clear();
        pdvector<image_instPoint *> img_calls;
        ifunc_->funcCalls(img_calls);
#if defined (cap_use_pdvector)
        callPoints_.reserve_exact(img_calls.size());
#endif
        
        for (unsigned i = 0; i < img_calls.size(); i++) {
            instPoint *point = instPoint::createParsePoint(this,
                                                           img_calls[i]);
            if (!point) continue; // Can happen if we double-create
            callPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    callPoints_.reserve_exact(callPoints_.size());
#endif
    return callPoints_;
}

const pdvector<instPoint*> &int_function::funcArbitraryPoints() {
  // We add these per-process, so there's no chance to have
  // a parse-level list
    return arbitraryPoints_;
}

const std::set<instPoint*> &int_function::funcUnresolvedControlFlow() 
{
    if ( ! unresolvedPoints_.empty() && ! obj()->isExploratoryModeOn() ) {
        return unresolvedPoints_;
    }
    pdvector<image_instPoint*> imgPoints;
    ifunc()->funcUnresolvedControlFlow( imgPoints );
    if ( unresolvedPoints_.size() != imgPoints.size() ) {

        // convert image_instPoints to instPoints, add to set
        pdvector<image_instPoint*>::iterator pIter = imgPoints.begin();
        while (pIter != imgPoints.end()) {
#if 0
            // KEVIN TODO: this is horribly wrong in implementation, but decent in theory
            // 1) the call target is _wrong_ because it's an offset from the instPoint
            //    base. 
            // 2) It's insufficient to check whether we're going to a known object;
            //    we need to check whether we know the target _function_.
            if ( ! (*pIter)->isDynamic() ) {
                if (proc()->findObject((*pIter)->callTarget())) {
                    pIter++;
                    continue;
                }
            }
#endif
            // find or create the new instPoint and add it to the vector
            instPoint *curPoint = instPoint::createParsePoint(this, *pIter);
            if (curPoint) unresolvedPoints_.insert(curPoint); // std::set eliminates duplicates
            pIter++;
        }
    }
    return unresolvedPoints_;
}

const set<instPoint*> &int_function::funcAbruptEnds() 
{
    if ( ! abruptEnds_.size() && ! obj()->isExploratoryModeOn() ) {
        return abruptEnds_;
    }

    pdvector<image_instPoint*> imgPoints;
    ifunc()->funcAbruptEnds( imgPoints );
    if (abruptEnds_.size() != imgPoints.size()) {

        // convert image_instPoints to instPoints, add to set
        pdvector<image_instPoint*>::iterator pIter = imgPoints.begin();
        while (pIter != imgPoints.end()) {

            // find or create the new instPoint and add it to the vector
            instPoint *curPoint;
            Address ptAddr = (*pIter)->offset() 
                                   + getAddress() 
                                   - ifunc()->getOffset();
            if (instPsByAddr_.find(ptAddr) != instPsByAddr_.end()) {
                curPoint = instPsByAddr_[ptAddr];
            } else {
                curPoint = instPoint::createParsePoint(this, *pIter);
            }
            abruptEnds_.insert(curPoint); // std::set eliminates duplicates
            pIter++;
        }
    }
    return abruptEnds_;
}

// TODO: POINT TYPES CANNOT BE SINGULAR! We end up looking in the wrong vector and failing...

bool int_function::removePoint(instPoint *point) 
{
    bool foundPoint = false;
    instPsByAddr_.erase(point->addr());

    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        if (entryPoints_[i] == point) {
            entryPoints_[i] = entryPoints_.back();
            entryPoints_.pop_back();
            foundPoint = true;
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        if (exitPoints_[i]->addr() == point->addr()) {
            exitPoints_[i] = exitPoints_.back();
            exitPoints_.pop_back();
            foundPoint = true;
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        if (callPoints_[i] == point) {
            callPoints_[i] = callPoints_.back();
            callPoints_.pop_back();
            foundPoint = true;
        }
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        if (arbitraryPoints_[i] == point) {
            arbitraryPoints_[i] = arbitraryPoints_.back();
            arbitraryPoints_.pop_back();
            foundPoint = true;
        }
    }

    if (abruptEnds_.find(point) != abruptEnds_.end()) {
        abruptEnds_.erase(point);
        foundPoint = true;
    }

    if (unresolvedPoints_.find(point) != unresolvedPoints_.end()) {
        unresolvedPoints_.erase(point);
        foundPoint = true;
    }

    return foundPoint;
}

void int_function::setPointResolved(instPoint *point, bool resolve)
{
    // look in unresolved points
    set<instPoint*>::iterator pIter = unresolvedPoints_.find( point );
    if (unresolvedPoints_.end() != pIter) {
        if (resolve) {
            unresolvedPoints_.erase(pIter);
        }
    } else {
        if (resolve) {
            // check among the abruptEnd points
            pIter = abruptEnds_.find( point );
            if (abruptEnds_.end() != pIter) {
                abruptEnds_.erase(pIter);
            }
        } else {
            unresolvedPoints_.insert(point);
        }
    }
    // make sure the point is still accessible
    assert( point == findInstPByAddr(point->addr()) );
}

// finds new entry point, sets the argument to the new 
int_block * int_function::setNewEntryPoint()
{
    int_block *newEntry = NULL;

    // find block with no intraprocedural entry edges
    assert(blocks_.size());
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) 
        {
            SingleContext epred(ifunc(),true,true);
            Block::edgelist & ib_ins = (*bIter)->llb()->sources();
            Block::edgelist::iterator eit = ib_ins.begin(&epred);
            if (eit == ib_ins.end()) {
                if (NULL != newEntry) {
                    fprintf(stderr,"ERROR: multiple blocks in function %lx "
                        "have no incoming edges: [%lx %lx) and [%lx %lx)\n",
                        getAddress(), newEntry->llb()->start(),
                        newEntry->llb()->start() + newEntry->llb()->end(),
                        (*bIter)->llb()->start(),
                        (*bIter)->llb()->start() + (*bIter)->llb()->end());
                } else {
                    newEntry = *bIter;
                }
            }
        }
    if( ! newEntry ) {
        newEntry = *blocks_.begin();
    }
    ifunc()->setEntryBlock(newEntry->llb());
    this->addr_ = newEntry->start();

    assert(!newEntry->llb()->isShared()); //KEVINTODO: unimplemented case

    image_instPoint *imgPoint = new image_instPoint(
        newEntry->llb()->firstInsnOffset(),
        newEntry->llb(),
        ifunc()->img(),
        functionEntry);
    ifunc()->img()->addInstPoint(imgPoint);

    // create and add an entry point for the int_func
    instPoint *point = 
        findInstPByAddr( newEntry->start() );
    if (NULL == point) {
        point = instPoint::createParsePoint(this, imgPoint);
    }
	assert(point);
    entryPoints_.push_back(point);

    // change function base address
    addr_ = newEntry->start();
    return newEntry;
}

void int_function::setHandlerFaultAddr(Address fa) 
{ 
    handlerFaultAddr_ = fa;
}

// Sets the address in the structure at which the fault instruction's
// address is stored if "set" is true.  Accesses the fault address and 
// translates it back to an original address if it corresponds to 
// relocated code in the Dyninst heap 
void int_function::setHandlerFaultAddrAddr(Address faa, bool set) 
{ 
    if (set) {
        // save the faultAddrAddr
        handlerFaultAddrAddr_ = faa; 
    }

    // get the faultAddr 
    assert(proc()->proc());
    assert(sizeof(Address) == proc()->getAddressWidth());
    Address faultAddr=0;
    if (!proc()->readDataSpace
        ((void*)faa, proc()->getAddressWidth(), (void*)&faultAddr, true)) 
    {
        assert(0);
    }

    // translate the faultAddr back to an original address, and if
    // that translation was necessary, save it to the faultAddrAddr in the 
    // CONTEXT struct
    if (proc()->proc()->isRuntimeHeapAddr(faultAddr)) {

        Address origAddr = faultAddr;
        vector<int_function *> tmps;
        baseTrampInstance *bti = NULL;
        bool success = proc()->getAddrInfo(faultAddr, origAddr, tmps, bti);
        assert(success);
        assert( proc()->writeDataSpace((void*)faa, 
                                       sizeof(Address), 
                                       (void*)&origAddr) );
    }
}

// Set the handler return addr to the most recent instrumented or
// relocated address, similar to instPoint::instrSideEffect.
// Also, make sure that we update our mapped view of memory, 
// we may have overwritten memory that was previously not code
void int_function::fixHandlerReturnAddr(Address /*faultAddr*/)
{
    if ( !proc()->proc() || ! handlerFaultAddrAddr_ ) {
        assert(0);
        return;
    }
#if 0 //KEVINTODO: this function doesn't work, I tried setting newPC to 0xdeadbeef and it had no impact on the program's behavior.  If the springboards work properly this code is unneeded
    // Do a straightfoward forward map of faultAddr
    // First, get the original address
    int_function *func;
    int_block *block; baseTrampInstance *ignored;
    Address origAddr;
    if (!proc()->getRelocInfo(faultAddr, origAddr, block, ignored)) {
       func = dynamic_cast<process *>(proc())->findActiveFuncByAddr(faultAddr);
       origAddr = faultAddr;
    }
    else {
       func = block->func();
    }
    std::list<Address> relocAddrs;
    proc()->getRelocAddrs(origAddr, func, relocAddrs, true);
    Address newPC = (!relocAddrs.empty() ? relocAddrs.back() : origAddr);
    
    if (newPC != faultAddr) {
       if(!proc()->writeDataSpace((void*)handlerFaultAddrAddr_, 
                                  sizeof(Address), 
                                  (void*)&newPC)) {
          assert(0);
       }
    }
#endif
}

void int_function::findPoints(int_block *block,
                              std::set<instPoint *> &foundPoints) const {
   // What's better - iterate over all our point contaners 
   // looking for options, or run through instPsByAddr_...

   for (Address a = block->start(); a < block->end(); ++a) {
      std::map<Address, instPoint *>::const_iterator p_iter = instPsByAddr_.find(a);
      if ((p_iter != instPsByAddr_.end()) &&
          (p_iter->second->block() == block)) {
         foundPoints.insert(p_iter->second);
      }
   }
}

// doesn't delete the ParseAPI::Block's, those are removed in a batch
// call to the parseAPI
void int_function::deleteBlock(int_block* block) 
{

   // init stuff
   assert(block && this == block->func());
   
    // Find the points that reside in this block. 
    std::set<instPoint *> foundPoints;
    findPoints(block, foundPoints);
    
    // And delete them
    for (std::set<instPoint *>::iterator iter = foundPoints.begin();
         iter != foundPoints.end(); ++iter) {
       removePoint(*iter);
    }

    blocks_.erase(block);
    blockMap_.erase(block->llb());

    // It appears that we delete the int_block before the image_basicBlock...
    //assert(consistency());
    triggerModified();
}

void int_function::splitBlock(image_basicBlock *img_orig, 
                              image_basicBlock *img_new) {
    blocks();
   int_block *origBlock = blockMap_[img_orig];
   assert(origBlock);
   
   int_block *newBlock = blockMap_[img_new];
   if (!newBlock) {
       newBlock = createBlock(img_new);
   }
   // Also adds to trackers
   
   // Move all instPoints that were contained in orig and should be
   // contained in newBlock...
   for (Address a = newBlock->start(); a < newBlock->end(); ++a) {
      std::map<Address, instPoint *>::const_iterator p_iter = instPsByAddr_.find(a);
      if ((p_iter != instPsByAddr_.end()) &&
          (p_iter->second->block() == origBlock)) {
            p_iter->second->setBlock(newBlock);
      }
   }
   	triggerModified();
}

// Remove funcs from:
//   mapped_object & mapped_module datastructures
//   addressSpace::textRanges codeRangeTree<int_function*> 
//   image-level & SymtabAPI datastructures
//   BPatch_addressSpace::BPatch_funcMap <int_function -> BPatch_function>
void int_function::removeFromAll() 
{
    mal_printf("purging blocks_ of size = %d from func at %lx\n",
               blocks_.size(), getAddress());

    BlockSet::const_iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) 
    {
        int_block *bbi = (*bIter);
        mal_printf("purged block [%lx %lx]\n",bbi->start(), bbi->end());
    }

    // delete blocks 
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end();
         bIter = blocks_.begin()) 
    {
        deleteBlock(*bIter);// removes block from blocks_ too
    }
    // remove from mapped_object & mapped_module datastructures
    obj()->removeFunction(this);
    mod()->removeFunction(this);

    // remove points
    entryPoints_.clear();
    callPoints_.clear();
    exitPoints_.clear();
    arbitraryPoints_.clear();
    abruptEnds_.clear();
    unresolvedPoints_.clear();
    instPsByAddr_.clear();

    // invalidates analyses related to this function
	triggerModified();

    // remove func & blocks from image, ParseAPI, & SymtabAPI datastructures
    ifunc()->img()->deleteFunc(ifunc());

    delete(this);
}

void int_function::addMissingBlock(image_basicBlock *missingB)
{
   int_block *bbi = findBlock(missingB);
   if (bbi) {
      assert(bbi->llb() == missingB);
      return;
    }
   
    cerr << "Function " << hex << this << " @ " << getAddress() << " adding block " << missingB->start() << " (" << missingB << ")" << dec << endl;
   createBlock(missingB);
}


/* Find image_basicBlocks that are missing from these datastructures and add
 * them.  The int_block constructor does pretty much all of the work in
 * a chain of side-effects extending all the way into the mapped_object class
 * 
 * We have to take into account that additional parsing may cause basic block splitting,
 * in which case it is necessary not only to add new int-level blocks, but to update 
 * int_block and BPatch_basicBlock objects. 
 */
void int_function::addMissingBlocks()
{
    // A bit of a hack, but be sure that we've re-checked the blocks in the
    // image_func as well.
    ifunc_->invalidateCache();

   blocks();
   cerr << "addMissingBlocks for function " << hex << this << dec << endl;
   // Add new blocks

   const vector<image_basicBlock*> & nblocks = obj()->parse_img()->getNewBlocks();
   // add blocks by looking up new blocks, if it promises to be more 
   // efficient than looking through all of the llfunc's blocks
   vector<image_basicBlock*>::const_iterator nit = nblocks.begin();
   for( ; nit != nblocks.end(); ++nit) {
       if (ifunc()->contains(*nit)) {
           addMissingBlock(*nit);
       }
   }

   if (ifunc()->blocks().size() > blocks_.size()) { //not just the else case!
       // we may have parsed into an existing function and added its blocks 
       // to ours, or this may just be a more efficient lookup method
       Function::blocklist & iblks = ifunc()->blocks();
       for (Function::blocklist::iterator bit = iblks.begin(); 
            bit != iblks.end(); 
            bit++) 
       {
           if (!findBlock(*bit)) {
               addMissingBlock(static_cast<image_basicBlock*>(*bit));
           }
       }
   }
}

/* trigger search in image_layer points vectors to be added to int_level 
 * datastructures
 */
void int_function::addMissingPoints()
{
    // the "true" parameter causes the helper functions to search for new 
    // points in the image, bypassing cached points
    funcEntries();
    funcExits();
    funcCalls();
    funcUnresolvedControlFlow();
    funcAbruptEnds();
}

// get instPoints of known function calls into this one
void int_function::getCallerPoints(std::vector<instPoint*>& callerPoints)
{
    image_basicBlock *entryLLB = entryBlock()->llb();
    const ParseAPI::Block::edgelist &sources = entryLLB->sources();
    for (ParseAPI::Block::edgelist::iterator iter = sources.begin();
        iter != sources.end(); ++iter) {
        std::vector<ParseAPI::Function *> llFuncs;
        (*iter)->src()->getFuncs(llFuncs);
        for (std::vector<ParseAPI::Function *>::iterator f_iter = llFuncs.begin();
            f_iter != llFuncs.end(); ++f_iter) {
            int_function *caller = proc()->findFuncByInternalFunc(static_cast<image_func *>(*f_iter));
            int_block *callerBlock = caller->findBlock((*iter)->src());
            caller->funcCalls();
            instPoint *callPoint = caller->findInstPByAddr(callerBlock->last());
            if (callPoint) {
                callerPoints.push_back(callPoint);
            }
        }
    }
}


instPoint *int_function::findInstPByAddr(Address addr) {
    // This only finds instPoints that have been previously created...
    // so don't bother parsing. 
    
    if (instPsByAddr_.find(addr) != instPsByAddr_.end())
        return instPsByAddr_[addr];

    // The above should have been sufficient... however, if we forked and have
    // a baseTramp that does not contain instrumentation, then there will never
    // be a instPointInstance created, and so no entry in instPsByAddr_. Argh.
    // So, if the lookup above failed, do the slow search through entries, 
    // exits, and calls - arbitraries should already exist.
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
	if (entryPoints_[i]->addr() == addr) {
            instPsByAddr_[addr] = entryPoints_[i];
            return entryPoints_[i];
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
	if (exitPoints_[i]->addr() == addr) {
            instPsByAddr_[addr] = exitPoints_[i];
            return exitPoints_[i];
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
	if (callPoints_[i]->addr() == addr) {
            instPsByAddr_[addr] = callPoints_[i];
            return callPoints_[i];
        }
    }
    std::set< instPoint* >::iterator pIter = unresolvedPoints_.begin();
    while ( pIter != unresolvedPoints_.end() ) {
        if ( (*pIter)->addr() == addr ) {
            instPsByAddr_[addr] = *pIter;
            return *pIter;
        }
        pIter++;
    }
    pIter = abruptEnds_.begin();
    while ( pIter != abruptEnds_.end() ) {
        if ( (*pIter)->addr() == addr ) {
            instPsByAddr_[addr] = *pIter;
            return *pIter;
        }
        pIter++;
    }

    return NULL;
}

void int_function::getReachableBlocks(const set<int_block*> &exceptBlocks,
                                      const list<int_block*> &seedBlocks,
                                      set<int_block*> &reachBlocks)//output
{
    list<image_basicBlock*> imgSeeds;
    for (list<int_block*>::const_iterator sit = seedBlocks.begin();
         sit != seedBlocks.end(); 
         sit++) 
    {
        imgSeeds.push_back((*sit)->llb());
    }
    set<image_basicBlock*> imgExcept;
    for (set<int_block*>::const_iterator eit = exceptBlocks.begin();
         eit != exceptBlocks.end(); 
         eit++) 
    {
        imgExcept.insert((*eit)->llb());
    }

    // image-level function does the work
    set<image_basicBlock*> imgReach;
    ifunc()->getReachableBlocks(imgExcept,imgSeeds,imgReach);

    for (set<image_basicBlock*>::iterator rit = imgReach.begin();
         rit != imgReach.end(); 
         rit++) 
    {
        reachBlocks.insert( findBlock(*rit) );
    }
}

bool int_function::findBlocksByAddr(Address addr, 
                                    std::set<int_block *> &blocks) 
{
   this->blocks();
   bool ret = false;    
   Address offset = addr-baseAddr();
   std::set<ParseAPI::Block *> iblocks;
   if (!ifunc()->img()->findBlocksByAddr(offset, iblocks)) {
       return false;
   }
   for (std::set<ParseAPI::Block *>::iterator iter = iblocks.begin(); 
        iter != iblocks.end(); ++iter) {
      int_block *bbl = findBlock(*iter);
      if (bbl) {
         ret = true; 
         blocks.insert(bbl);
      }
      // It's okay for this to fail if we're in the middle of a CFG
      // modification. 

   }
   return ret;
}

int_block *int_function::findOneBlockByAddr(Address addr) {
    std::set<int_block *> blocks;
    if (!findBlocksByAddr(addr, blocks)) {
        return NULL;
    }
    for (std::set<int_block *>::iterator iter = blocks.begin();
        iter != blocks.end(); ++iter) {
        // Walk this puppy and see if it matches our address.
        int_block::InsnInstances insns;
        (*iter)->getInsnInstances(insns);
        for (int_block::InsnInstances::iterator i_iter = insns.begin(); i_iter != insns.end(); ++i_iter) {
            if (i_iter->second == addr) return *iter;
        }
    }
    return NULL;
}

int_block *int_function::findBlockByEntry(Address addr) {
    std::set<int_block *> blocks;
    if (!findBlocksByAddr(addr, blocks)) {
        return NULL;
    }
    for (std::set<int_block *>::iterator iter = blocks.begin();
        iter != blocks.end(); ++iter) {
        int_block *cand = *iter;
        if (cand->start() == addr) return cand;
    }
    return NULL;
}

int_block *int_function::findBlock(ParseAPI::Block *block) {
   blocks();
    image_basicBlock *iblock = static_cast<image_basicBlock *>(block);
    BlockMap::iterator iter = blockMap_.find(iblock);
    if (iter != blockMap_.end()) return iter->second;
    return NULL;
}


void int_function::registerInstPointAddr(Address addr, instPoint *inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    if (oldInstP) assert(inst == oldInstP);

    instPsByAddr_[addr] = inst;
}

void int_function::unregisterInstPointAddr(Address addr, instPoint* inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    assert(oldInstP == inst);

    instPsByAddr_.erase(addr);
}

void print_func_vector_by_pretty_name(std::string prefix,
				      pdvector<int_function *>*funcs) {
    unsigned int i;
    int_function *func;
    for(i=0;i<funcs->size();i++) {
      func = ((*funcs)[i]);
      cerr << prefix << func->prettyName() << endl;
    }
}

mapped_module *int_function::mod() const { return mod_; }
mapped_object *int_function::obj() const { return mod()->obj(); }
AddressSpace *int_function::proc() const { return obj()->proc(); }

const int_function::BlockSet &int_function::blocks()
{
    if (blocks_.empty()) {
        // defensiveMode triggers premature block list creation when it
        // checks that the targets of control transfers have not been
        // tampered with.  
        Function::blocklist & img_blocks = ifunc_->blocks();
        Function::blocklist::iterator sit = img_blocks.begin();

        for( ; sit != img_blocks.end(); ++sit) {
            image_basicBlock *b = (image_basicBlock*)(*sit);
            createBlock(b);
        }
    }
    return blocks_;
}



int_block *int_function::entryBlock() {
    if (!ifunc_->parsed()) {
        ifunc_->blocks();
    }
	ParseAPI::Block *iEntry = ifunc_->entry();
	if (!iEntry) return NULL;
	return findBlock(iEntry);
}

unsigned int_function::getNumDynamicCalls()
{
   unsigned count=0;
   pdvector<instPoint *> callPoints = funcCalls();

   for (unsigned i=0; i<callPoints.size(); i++)
   {
      if (callPoints[i]->isDynamic())
          count++;
   }
   return count;
}

const string &int_function::symTabName() const { 
    return ifunc_->symTabName(); 
}

void int_function::debugPrint() const {
    fprintf(stderr, "Function debug dump (%p):\n", this);
    fprintf(stderr, "  Symbol table names:\n");
    for (unsigned i = 0; i < symTabNameVector().size(); i++) {
        fprintf(stderr, "    %s\n", symTabNameVector()[i].c_str());
    }
    fprintf(stderr, "  Demangled names:\n");
    for (unsigned j = 0; j < prettyNameVector().size(); j++) {
        fprintf(stderr, "    %s\n", prettyNameVector()[j].c_str());
    }
    fprintf(stderr, "  Typed names:\n");
    for (unsigned k = 0; k < typedNameVector().size(); k++) {
        fprintf(stderr, "    %s\n", typedNameVector()[k].c_str());
    }
    fprintf(stderr, "  Address: 0x%lx\n", getAddress());
    fprintf(stderr, "  Internal pointer: %p\n", ifunc_);
    fprintf(stderr, "  Object: %s (%p), module: %s (%p)\n", 
            obj()->fileName().c_str(), 
            obj(),
            mod()->fileName().c_str(),
            mod());
    for (BlockSet::const_iterator 
             cb = blocks_.begin();
         cb != blocks_.end(); 
         cb++) 
    {
        int_block* orig = (*cb);
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->start(),
                orig->end());
    }
}

// Add to internal
// Add to mapped_object if a "new" name (true return from internal)
void int_function::addSymTabName(const std::string name, bool isPrimary) {
    if (ifunc()->addSymTabName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::mangledName);
}

void int_function::addPrettyName(const std::string name, bool isPrimary) {
    if (ifunc()->addPrettyName(name, isPrimary))
        obj()->addFunctionName(this, name, mapped_object::prettyName);
}

void int_function::getStaticCallers(pdvector< int_function * > &callers)
{
    pdvector<image_edge *> ib_ins;

    if(!ifunc_ || !ifunc_->entryBlock())
        return;

    Block::edgelist & ins = ifunc_->entryBlock()->sources();
    Block::edgelist::iterator eit = ins.begin();
    for( ; eit != ins.end(); ++eit) {
        if((*eit)->type() == CALL)
        {   
            vector<Function *> ifuncs;
            (*eit)->src()->getFuncs(ifuncs);
            vector<Function *>::iterator ifit = ifuncs.begin();
            for( ; ifit != ifuncs.end(); ++ifit)
            {   
                int_function * f;
                f = obj()->findFunction((image_func*)*ifit);
                
                callers.push_back(f);
            }
        }
    }
}

image_func *int_function::ifunc() {
    return ifunc_;
}

// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool int_function::getSharingFuncs(int_block *b,
                                   std::set<int_function *> & funcs)
{
    bool ret = false;
    if(!b->hasSharedBase())
        return ret;

    vector<Function *> lfuncs;
    b->llb()->getFuncs(lfuncs);
    vector<Function *>::iterator fit = lfuncs.begin();
    for( ; fit != lfuncs.end(); ++fit) {
        image_func *ll_func = static_cast<image_func*>(*fit);
        int_function *hl_func = obj()->findFunction(ll_func);
        assert(hl_func);

        if (hl_func == this) continue;

        if (funcs.find(hl_func) == funcs.end()) ret = true;
        funcs.insert(hl_func);
    }

    return ret;
}

// Find sharing functions via checking all basic blocks. We might be
// able to check only exit points; but we definitely need to check _all_
// exits so for now we're checking everything.

bool int_function::getSharingFuncs(std::set<int_function *> &funcs) {
    bool ret = false;

    // Create the block list.
    blocks();
    
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) {
       if (getSharingFuncs(*bIter,funcs))
          ret = true;
    }

    return ret;
}

// Find functions that have overlapping, but disjoint, blocks in 
// addition to shared blocks. Very slow!

bool int_function::getOverlappingFuncs(int_block *block,
										std::set<int_function *> &funcs) 
{
	ParseAPI::Block *llB = block->llb();
	std::set<ParseAPI::Block *> overlappingBlocks;
	for (Address i = llB->start(); i < llB->end(); ++i) {
		llB->obj()->findBlocks(llB->region(), i, overlappingBlocks);
	}
	// We now have all of the overlapping ParseAPI blocks. Get the set of 
	// ParseAPI::Functions containing each and up-map to int_functions
	for (std::set<ParseAPI::Block *>::iterator iter = overlappingBlocks.begin();
		iter != overlappingBlocks.end(); ++iter) {
		std::vector<ParseAPI::Function *> llFuncs;
		(*iter)->getFuncs(llFuncs);
		for (std::vector<ParseAPI::Function *>::iterator iter2 = llFuncs.begin();
			iter2 != llFuncs.end(); ++iter2) 
        {
            funcs.insert(obj()->findFunction(*iter2));
		}
	}
	return (funcs.size() > 1);
}

bool int_function::getOverlappingFuncs(std::set<int_function *> &funcs) 
{
    bool ret = false;

    // Create the block list.
    blocks();
    BlockSet::iterator bIter;
    for (bIter = blocks_.begin(); 
         bIter != blocks_.end(); 
         bIter++) {
       if (getOverlappingFuncs(*bIter,funcs))
          ret = true;
    }

    return ret;
}

Address int_function::get_address() const 
{
   return getAddress();
}

unsigned int_function::get_size() const 
{
   assert(0);
   return 0x0;
}

std::string int_function::get_name() const
{
   return symTabName();
}

void int_function::getNewInstrumentation(std::set<instPoint *> &ret) {
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        if (entryPoints_[i]->hasNewInstrumentation()) {
            ret.insert(entryPoints_[i]);
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        if (exitPoints_[i]->hasNewInstrumentation()) {
            ret.insert(exitPoints_[i]);
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        if (callPoints_[i]->hasNewInstrumentation()) {
            ret.insert(callPoints_[i]);
        }
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        if (arbitraryPoints_[i]->hasNewInstrumentation()) {
            ret.insert(arbitraryPoints_[i]);
        }
    }
    std::set< instPoint* >::iterator pIter = unresolvedPoints_.begin();
    while ( pIter != unresolvedPoints_.end() ) {
        if ( (*pIter)->hasNewInstrumentation() ) {
            ret.insert( *pIter );
        }
        pIter++;
    }
    pIter = abruptEnds_.begin();
    while ( pIter != abruptEnds_.end() ) {
        if ( (*pIter)->hasNewInstrumentation() ) {
            ret.insert( *pIter );
        }
        pIter++;
    }
}

void int_function::getAnyInstrumentation(std::set<instPoint *> &ret) {
    for (unsigned i = 0; i < entryPoints_.size(); i++) {
        if (entryPoints_[i]->hasAnyInstrumentation()) {
            ret.insert(entryPoints_[i]);
        }
    }
    for (unsigned i = 0; i < exitPoints_.size(); i++) {
        if (exitPoints_[i]->hasAnyInstrumentation()) {
            ret.insert(exitPoints_[i]);
        }
    }
    for (unsigned i = 0; i < callPoints_.size(); i++) {
        if (callPoints_[i]->hasAnyInstrumentation()) {
            ret.insert(callPoints_[i]);
        }
    }
    for (unsigned i = 0; i < arbitraryPoints_.size(); i++) {
        if (arbitraryPoints_[i]->hasAnyInstrumentation()) {
            ret.insert(arbitraryPoints_[i]);
        }
    }
    std::set< instPoint* >::iterator pIter = unresolvedPoints_.begin();
    while ( pIter != unresolvedPoints_.end() ) {
        if ( (*pIter)->hasAnyInstrumentation() ) {
            ret.insert( *pIter );
        }
        pIter++;
    }
    pIter = abruptEnds_.begin();
    while ( pIter != abruptEnds_.end() ) {
        if ( (*pIter)->hasAnyInstrumentation() ) {
            ret.insert( *pIter );
        }
        pIter++;
    }
}


Offset int_function::addrToOffset(const Address addr) const { 
  return addr - (getAddress() - ifunc_->getOffset());
}

Address int_function::offsetToAddr(const Offset off ) const { 
  return off + (getAddress() - ifunc_->getOffset());
}

const pdvector< int_parRegion* > &int_function::parRegions()
{
  if (parallelRegions_.size() > 0)
    return parallelRegions_;

  for (unsigned int i = 0; i < ifunc_->parRegions().size(); i++)
    {
      image_parRegion * imPR = ifunc_->parRegions()[i];
      //int_parRegion * iPR = new int_parRegion(imPR, baseAddr, this); 
      int_parRegion * iPR = new int_parRegion(imPR, addr_, this); 
      parallelRegions_.push_back(iPR);
    }
  return parallelRegions_;
}

bool int_function::validPoint(instPoint *p) const {
   // check whether the instPoint is correct
   assert(p->block()->start() <= p->addr());
   assert(p->block()->end() > p->addr());
   return true;
}

bool int_function::consistency() const {
   // 1) Check for 1:1 block relationship in
   //    the block list and block map
   // 2) Check that all instPoints are in the
   //    correct block.

   const ParseAPI::Function::blocklist &img_blocks = ifunc_->blocks();
   assert(img_blocks.size() == blocks_.size());
   assert(blockMap_.size() == blocks_.size());
   for (ParseAPI::Function::blocklist::iterator iter = img_blocks.begin();
        iter != img_blocks.end(); ++iter) {
      image_basicBlock *img_block = static_cast<image_basicBlock *>(*iter);
      BlockMap::const_iterator m_iter = blockMap_.find(img_block);
      assert(m_iter != blockMap_.end());
      assert(blocks_.find(m_iter->second) != blocks_.end());
   }

   // Instpoints
   for (unsigned i = 0; i < entryPoints_.size(); ++i) {
      assert(validPoint(entryPoints_[i]));
   }
   for (unsigned i = 0; i < exitPoints_.size(); ++i) {
      assert(validPoint(exitPoints_[i]));
   }
   for (unsigned i = 0; i < callPoints_.size(); ++i) {
      assert(validPoint(callPoints_[i]));
   }
   for (unsigned i = 0; i < arbitraryPoints_.size(); ++i) {
      assert(validPoint(arbitraryPoints_[i]));
   }
   for (std::set<instPoint *>::const_iterator iter = unresolvedPoints_.begin();
        iter != unresolvedPoints_.end(); ++iter) {
      assert(validPoint(*iter));
   }
   for (std::set<instPoint *>::const_iterator iter = abruptEnds_.begin();
        iter != abruptEnds_.end(); ++iter) {
      assert(validPoint(*iter));
   }
   return true;
}

void int_function::triggerModified() {
	// A single location to drop anything that needs to be
	// informed when an int_function was updated.

    // invalidate liveness calculations
    ifunc()->invalidateLiveness();
}
