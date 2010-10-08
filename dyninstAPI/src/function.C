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
#include "multiTramp.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include "parseAPI/src/InstrucIter.h"

#if defined(cap_relocation)
#include "reloc-func.h"
#endif

//std::string int_function::emptyString("");

#include "Parsing.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;


int int_function_count = 0;

// 
int_function::int_function(image_func *f,
			   Address baseAddr,
			   mapped_module *mod) :
    ifunc_(f),
    mod_(mod),
    blockIDmap(intHash),
    handlerFaultAddr_(0),
    handlerFaultAddrAddr_(0), 
    isBeingInstrumented_(false),
    instPsByAddr_(addrHash4),
#if defined(cap_relocation)
    generatedVersion_(0),
    installedVersion_(0),
    linkedVersion_(0),
#endif
    version_(0)
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
    blockIDmap(intHash),
    handlerFaultAddr_(0),
    handlerFaultAddrAddr_(0), 
    isBeingInstrumented_(parFunc->isBeingInstrumented_),
    instPsByAddr_(addrHash4),
#if defined(cap_relocation)
    generatedVersion_(parFunc->generatedVersion_),
    installedVersion_(parFunc->installedVersion_),
    linkedVersion_(parFunc->linkedVersion_),
#endif
    version_(parFunc->version_)
 {
     unsigned i; // Windows hates "multiple definitions"

     // Construct the raw blocklist;
     set< int_basicBlock* , int_basicBlock::compare >::const_iterator 
         bIter = parFunc->blockList.begin();
     for (i=0; bIter != parFunc->blockList.end(); i++,bIter++) {
         int_basicBlock *block = new int_basicBlock((*bIter), this,i);
         blockList.insert(block);
     }
     nextBlockID = i;
     // got the same blocks in the same order as the parent, so this is safe:
     blockIDmap = parFunc->blockIDmap;
     
     for (i = 0; i < parFunc->entryPoints_.size(); i++) {
         instPoint *parP = parFunc->entryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         entryPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->exitPoints_.size(); i++) {
         instPoint *parP = parFunc->exitPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         exitPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->callPoints_.size(); i++) {
         instPoint *parP = parFunc->callPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         callPoints_.push_back(childIP);
     }

     for (i = 0; i < parFunc->arbitraryPoints_.size(); i++) {
         instPoint *parP = parFunc->arbitraryPoints_[i];
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         arbitraryPoints_.push_back(childIP);
     }

     set<instPoint*>::const_iterator pIter;
     for(pIter = parFunc->unresolvedPoints_.begin(); 
         pIter != parFunc->unresolvedPoints_.end(); pIter++) 
     {
         instPoint *parP = *pIter;
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         unresolvedPoints_.insert(childIP);
     }

     for(pIter = parFunc->abruptEnds_.begin(); 
         pIter != parFunc->abruptEnds_.end(); pIter++) 
     {
         instPoint *parP = *pIter;
         int_basicBlock *block = findBlockByAddr(parP->addr());
         assert(block);
         instPoint *childIP = instPoint::createForkedPoint(parP, block, childP);
         abruptEnds_.insert(childIP);
     }

     // TODO: relocated functions
}

int_function::~int_function() { 
    // ifunc_ doesn't keep tabs on us, so don't need to let it know.
    // mod_ is cleared by the mapped_object
    // blockList isn't allocated

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
    for(; pIter != unresolvedPoints_.end(); pIter++) {
        delPoints.insert(*pIter);
    }
    for (pIter = abruptEnds_.begin(); pIter != abruptEnds_.end(); pIter++) {
        delPoints.insert(*pIter);
    }
    for (pIter = delPoints.begin(); delPoints.end() != pIter; pIter++) {
        delete (*pIter);
    }

    // int_basicBlocks
    set< int_basicBlock* , int_basicBlock::compare >::iterator
        bIter = blockList.begin();
    for (; bIter != blockList.end(); bIter++) {
        delete *bIter;
    }

#if defined(cap_relocation)
    for (unsigned i = 0; i < enlargeMods_.size(); i++)
        delete enlargeMods_[i];
#if defined (cap_use_pdvector)
    enlargeMods_.zap();
#else
    enlargeMods_.clear();
#endif
#endif
    
    for (unsigned i = 0; i < parallelRegions_.size(); i++)
      delete parallelRegions_[i];
      
}

// This needs to go away: how is "size" defined? Used bytes? End-start?

unsigned int_function::getSize_NP()  {
    blocks();
    if (blockList.size() == 0) return 0;
            
    return ((*blockList.rbegin())->origInstance()->endAddr() - 
            (*blockList.begin())->origInstance()->firstInsnAddr());
}

void int_function::addArbitraryPoint(instPoint *insp) {
    arbitraryPoints_.push_back(insp);
}

const pdvector<instPoint *> &int_function::funcEntries() {
    if (entryPoints_.size() == 0 || obj()->isExploratoryModeOn()) {
        entryPoints_.clear();
        pdvector<image_instPoint *> img_entries;
        ifunc_->funcEntries(img_entries);
#if defined (cap_use_pdvector)
        entryPoints_.reserve_exact(img_entries.size());
#endif
        for (unsigned i = 0; i < img_entries.size(); i++) {

            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_entries[i]->offset()-ifunc_->getOffset();
            // add points that we've already seen
            if ( instPsByAddr_.find( offsetInFunc + getAddress() ) ) {
                entryPoints_.push_back( instPsByAddr_[offsetInFunc + getAddress()] );
                continue;
            }
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for entry point "
                        "at 0x%lx (0x%lx) (func 0x%lx to 0x%lx\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP());
                
                continue;
            }

            instPoint *point = instPoint::createParsePoint(this,
                                                           img_entries[i]);
			if (!point) continue; // Can happen if we double-create
			assert(point);
            entryPoints_.push_back(point);
        }
    }
#if defined (cap_use_pdvector)
    entryPoints_.reserve_exact(entryPoints_.size());
#endif
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
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_exits[i]->offset()-ifunc_->getOffset();
            // add points that we've already seen
            if ( instPsByAddr_.find( offsetInFunc + getAddress() ) ) {
                exitPoints_.push_back( instPsByAddr_[offsetInFunc + getAddress()] );
                continue;
            }
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for exit point at 0x%lx (0x%lx) (func 0x%lx to 0x%lx\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP());
                
                continue;
            }

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
        callPoints_.clear();
        pdvector<image_instPoint *> img_calls;
        ifunc_->funcCalls(img_calls);
#if defined (cap_use_pdvector)
        callPoints_.reserve_exact(img_calls.size());
#endif
        
        for (unsigned i = 0; i < img_calls.size(); i++) {
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_calls[i]->offset()-ifunc_->getOffset();
            // add points that we've already seen
            if ( instPsByAddr_.find( offsetInFunc + getAddress() ) ) {
                callPoints_.push_back( instPsByAddr_[offsetInFunc + getAddress()] );
                continue;
            }
            if (!findBlockByOffset(offsetInFunc)) {
                fprintf(stderr, "Warning: unable to find block for call point "
                        "at 0x%lx (0x%lx) (func 0x%lx to 0x%lx, %s/%s)\n",
                        offsetInFunc,
                        offsetInFunc+getAddress(),
                        getAddress(),
                        getAddress() + getSize_NP(),
                        symTabName().c_str(),
                        obj()->fileName().c_str());
                debugPrint();
                
                continue;
            }
            instPoint *point = instPoint::createParsePoint(this,
                                                           img_calls[i]);
            if (!point) continue; // Can happen if we double-create

            assert(point);
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

            // skip static transfers to known code
            if ( ! (*pIter)->isDynamic() ) {
                codeRange *range = 
                    proc()->findOrigByAddr((*pIter)->callTarget());
                if ( range && ! (range->is_mapped_object()) ) {
                    pIter++;
                    continue; 
                }
            }

            // find or create the new instPoint and add it to the vector
            instPoint *curPoint;
            Address ptAddr = (*pIter)->offset() 
                                   + getAddress() 
                                   - ifunc()->getOffset();
            if (instPsByAddr_.find(ptAddr)) {
                curPoint = instPsByAddr_[ptAddr];
            } else {
                curPoint = instPoint::createParsePoint(this, *pIter);
            }
            unresolvedPoints_.insert(curPoint); // std::set eliminates duplicates
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
            if (instPsByAddr_.find(ptAddr)) {
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

void int_function::findBlocksByRange(std::vector<int_basicBlock*> &rangeBlocks, 
                                     Address start, Address end)
{
    std::set< int_basicBlock* , int_basicBlock::compare >::iterator biter;
    biter = blockList.begin();
    while (biter != blockList.end()) 
    {
        Address bstart = (*biter)->origInstance()->firstInsnAddr();
        if (start <= bstart && bstart < end) 
        {
           rangeBlocks.push_back(*biter);
        }
        biter++;
    }
    if (rangeBlocks.size() == 0 ) {
        //make sure we got an un-relocated range, haven't implemented this
        // for relocated ranges
        assert ( obj()->codeBase() <= start && 
                 start < obj()->codeBase() + obj()->get_size() );
    }
}

bool int_function::removePoint(instPoint *point) 
{
    bool foundPoint = false;
    if (instPsByAddr_.find(point->addr()))
        instPsByAddr_.undef(point->addr());
    switch(point->getPointType()) {
    case functionEntry:
        for (unsigned i = 0; !foundPoint && i < entryPoints_.size(); i++) {
            if (entryPoints_[i] == point) {
                if (i < entryPoints_.size()-1) {
                    entryPoints_[i] = entryPoints_[entryPoints_.size()-1];
                }
                entryPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case functionExit:
        for (unsigned i = 0; !foundPoint && i < exitPoints_.size(); i++) {
            if (exitPoints_[i]->addr() == point->addr()) {
                if (i < exitPoints_.size()-1) {
                    exitPoints_[i] = exitPoints_[exitPoints_.size()-1];
                }
                exitPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case callSite:
        for (unsigned i = 0; !foundPoint && i < callPoints_.size(); i++) {
            if (callPoints_[i] == point) {
                if (i < callPoints_.size()-1) {
                    callPoints_[i] = callPoints_[callPoints_.size()-1];
                }
                callPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case otherPoint:
        for (unsigned i = 0; !foundPoint && i < arbitraryPoints_.size(); i++) {
            if (arbitraryPoints_[i] == point) {
                if (i < arbitraryPoints_.size()-1) {
                    arbitraryPoints_[i] = arbitraryPoints_[arbitraryPoints_.size()-1];
                }
                arbitraryPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    default: // includes noneType
        assert(0); // unhandled case!
    }
    if (unresolvedPoints_.find(point) != unresolvedPoints_.end()) {
        unresolvedPoints_.erase(point);
        foundPoint = true;
    }
    if (abruptEnds_.find(point) != abruptEnds_.end()) {
        abruptEnds_.erase(point);
        foundPoint = true;
    }
    if (point->imgPt()) {
        ifunc()->img()->removeInstPoint(point->imgPt());
    }
    assert(foundPoint);
    return foundPoint;
}

// returns true if a change was made, image layer is called independently
bool int_function::setPointResolved(instPoint *point)
{
    bool foundPoint = false;
    // look in unresolved points
    set<instPoint*>::iterator pIter = unresolvedPoints_.find( point );
    if (unresolvedPoints_.end() != pIter) {
        unresolvedPoints_.erase(pIter);
        foundPoint = true;
    } else {
        // check among the abruptEnd points
        pIter = abruptEnds_.find( point );
        if (abruptEnds_.end() != pIter) {
            abruptEnds_.erase(pIter);
            foundPoint = true;
        }
    }

    // make sure the point is still accessible
    assert( point == findInstPByAddr(point->addr()) );

    if (!foundPoint) {
        fprintf(stderr,"WARNING: Tried to resolve point at offset %lx "
                "that was already resolved %s[%d]\n",
                point->addr(),FILE__,__LINE__);
    }
    return foundPoint;
}

// finds new entry point, sets the argument to the new 
Address int_function::setNewEntryPoint(int_basicBlock *& newEntry)
{
    newEntry = NULL;

    // find block with no intraprocedural entry edges
    assert(blockList.size());
    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); 
         bIter != blockList.end(); 
         bIter++) 
        {
            SingleContext epred(ifunc(),true,true);
            Block::edgelist & ib_ins = (*bIter)->llb()->sources();
            Block::edgelist::iterator eit = ib_ins.begin(&epred);
            if (eit != ib_ins.end()) {
                assert(!newEntry);
                newEntry = *bIter;
            }
        }
    if( ! newEntry) {
        newEntry = *blockList.begin();
    }

    assert(!newEntry->llb()->isShared()); //KEVINTODO: unimplemented case

    //create and add an entry point for the image_func
    int insn_size = 0;
    unsigned char * insn_buf = (unsigned char *) obj()->getPtrToInstruction
        (newEntry->origInstance()->firstInsnAddr());
#if defined(cap_instruction_api)
    using namespace InstructionAPI;
    InstructionDecoder dec
        (insn_buf,InstructionDecoder::maxInstructionLength,proc()->getArch());
    Instruction::Ptr insn = dec.decode();
    if(insn)
        insn_size = insn->size();
#else
    InstrucIter ah(newEntry->origInstance()->firstInsnAddr(),
                   newEntry->origInstance()->getSize(),
                   proc());
    instruction insn = ah.getInstruction();
    insn_size = insn.size();
#endif
    image_instPoint *imgPoint = new image_instPoint(
        newEntry->llb()->firstInsnOffset(),
        insn_buf,
        insn_size,
        ifunc()->img(),
        functionEntry);
    ifunc()->img()->addInstPoint(imgPoint);

    // create and add an entry point for the int_func
    instPoint *point = 
        findInstPByAddr( newEntry->origInstance()->firstInsnAddr() );
    if (NULL == point) {
        point = instPoint::createParsePoint(this, imgPoint);
    }
	assert(point);
    entryPoints_.push_back(point);

    // change function base address
    addr_ = newEntry->origInstance()->firstInsnAddr();
    return newEntry->origInstance()->firstInsnAddr();
}

/* 0. The target and source must be in the same mapped region, make sure memory
 *    for the target is up to date
 * 1. Parse from target address, add new edge at image layer
 * 2. Add image blocks as int_basicBlocks
 * 3. Add image points, as instPoints, fix up mapping of split blocks with points
 * 4. Register all newly created functions as a result of new edge parsing
*/
bool int_function::parseNewEdges( std::vector<ParseAPI::Block*> &sources, 
                                  std::vector<Address> &targets, 
                                  std::vector<EdgeTypeEnum> &edgeTypes)
{
    using namespace SymtabAPI;

/* 0. The target and source must be in the same mapped region, make sure memory
      for the target is up to date */

    // Update set of active multiTramps before parsing 
    if (NULL != proc()->proc()) {// if it's a process and not a binEdit
        proc()->proc()->updateActiveMultis();
    }

    Address loadAddr = getAddress() - ifunc()->getOffset();
    std::set<Region*> targRegions;
    for (unsigned idx=0; idx < targets.size(); idx++) {
        Region *targetRegion = ifunc()->img()->getObject()->
            findEnclosingRegion( targets[idx]-loadAddr );

        // same region check
        if (NULL != sources[idx]) {
            Region *sourceRegion = ifunc()->img()->getObject()->
                findEnclosingRegion( sources[idx]->start() );
            assert(targetRegion == sourceRegion );

        }
        // update target region
        if (targRegions.end() == targRegions.find(targetRegion)) {
            obj()->updateMappedFileIfNeeded(targets[idx],targetRegion);
            targRegions.insert(targetRegion);
        }
        // translate targets to memory offsets rather than absolute addrs
        targets[idx] -= loadAddr;
    }

/* 1. Parse from target address, add new edge at image layer  */
    assert( !ifunc()->img()->hasSplitBlocks() && 
            !ifunc()->img()->hasNewBlocks());
    // parses and adds new blocks to image-layer datastructures
    ifunc()->img()->codeObject()->parseNewEdges(sources, targets, edgeTypes);

/* 2. Add function blocks to int-level datastructures         */
    //vector<int_basicBlock*> newBlocks;
    addMissingBlocks(); // also adds to mapped_object and re-sizes function

/* 3. Add image points to int-level datastructures, fix up mapping of split blocks with points */
    addMissingPoints();
    // See if block splitting has caused problems with existing points
    if (ifunc()->img()->hasSplitBlocks()) {
        obj()->splitIntLayer();
        ifunc()->img()->clearSplitBlocks();
    }

/* 4. Register all newly created image_funcs as a result of new edge parsing */
    pdvector<int_function*> intfuncs;
    obj()->getAllFunctions(intfuncs);

    return true;
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
    assert (proc()->readDataSpace
        ((void*)faa, proc()->getAddressWidth(), (void*)&faultAddr, true));

    // translate the faultAddr back to an original address, and if
    // that translation was necessary, save it to the faultAddrAddr in the 
    // CONTEXT struct
    if (proc()->proc()->isRuntimeHeapAddr(faultAddr)) {
        codeRange *range = proc()->findOrigByAddr(faultAddr);
        if (range->is_multitramp()) {
            faultAddr = range->is_multitramp()->instToUninstAddr(faultAddr);
            range = proc()->findOrigByAddr( faultAddr );
        }
        bblInstance *curbbi = range->is_basicBlockInstance();
        assert(curbbi);
        faultAddr = curbbi->equivAddr(0,faultAddr);
        assert( proc()->writeDataSpace((void*)faa, 
                                       sizeof(Address), 
                                       (void*)&faultAddr) );
    }
}

// Set the handler return addr to the most recent instrumented or
// relocated address, similar to instPoint::instrSideEffect.
// Also, make sure that we update our mapped view of memory, 
// we may have overwritten memory that was previously not code
void int_function::fixHandlerReturnAddr(Address faultAddr)
{
    if ( !proc()->proc() || ! handlerFaultAddrAddr_ ) {
        assert(0);
        return;
    }
    bblInstance *bbi = proc()->findOrigByAddr(faultAddr)->
        is_basicBlockInstance();
    if (bbi) {
        // get relocated PC address
        Address newPC = bbi->equivAddr(bbi->func()->version(), faultAddr);
        multiTramp *multi = proc()->findMultiTrampByAddr(newPC);
        if (multi) {
            newPC = multi->uninstToInstAddr(newPC);
        }
        assert(newPC);
        assert(proc()->proc()->getAddressWidth() == sizeof(Address));
        if (newPC != faultAddr) {
            assert( proc()->writeDataSpace((void*)handlerFaultAddrAddr_, 
                                           sizeof(Address), 
                                           (void*)&newPC) );
        }
    }
}

// doesn't delete the ParseAPI::Block's, those are removed in a batch
// call to the parseAPI
void int_function::deleteBlock(int_basicBlock* block) 
{
    // init stuff
    assert(block && this == block->func());
    bblInstance *origbbi = block->origInstance();
    image_basicBlock *imgBlock = block->llb();
    assert( ! imgBlock->isShared() ); //KEVINTODO: unimplemented case
    Address baseAddr = ifunc()->img()->desc().loadAddr();

    // remove points
    pdvector<image_instPoint*> imgPoints;
    ifunc()->img()->getInstPoints( origbbi->firstInsnAddr(), 
                                   origbbi->endAddr(), 
                                   imgPoints );
    for (unsigned pidx=0; pidx < imgPoints.size(); pidx++) {
        image_instPoint *imgPt = imgPoints[pidx];
        instPoint *point = findInstPByAddr( imgPt->offset() + baseAddr );
        if (!point) {
            addMissingBlocks();
            point = findInstPByAddr( imgPt->offset() + baseAddr );
        }
        point->removeMultiTramps();
        removePoint( point );
    }


    // Remove block from int-level datastructures 
    pdvector<bblInstance*> bbis = block->instances();
    for (unsigned bIdx = 1; bIdx < bbis.size(); bIdx++) 
    {   // the original instance is not in the process range
        proc()->removeOrigRange(bbis[bIdx]);
    }
    for (unsigned bidx=0; bidx < block->instances_.size(); bidx++) {
        deleteBBLInstance(block->instances_[bidx]);
    }
    blockList.erase(block);
    obj()->removeRange(block->origInstance());
    
    // delete block? 
    delete(block);
}

// Remove funcs from:
//   mapped_object & mapped_module datastructures
//   addressSpace::textRanges codeRangeTree<int_function*> 
//   image-level & SymtabAPI datastructures
//   BPatch_addressSpace::BPatch_funcMap <int_function -> BPatch_function>
void int_function::removeFromAll() 
{
    mal_printf("purging blocklist of size = %d\n",blockList.size());
    set< int_basicBlock* , int_basicBlock::compare >::const_iterator bIter;
    for (bIter = blockList.begin(); 
         bIter != blockList.end(); 
         bIter++) 
    {
        bblInstance *bbi = (*bIter)->origInstance();
        mal_printf("block [%lx %lx]\n",bbi->firstInsnAddr(), bbi->endAddr());
    }
    // delete blocks 
    for (bIter = blockList.begin(); 
         bIter != blockList.end();
         bIter = blockList.begin()) 
    {
        deleteBlock(*bIter);// removes block from blockList too
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

    // remove func & blocks from image, ParseAPI, & SymtabAPI datastructures
    ifunc()->img()->deleteFunc(ifunc());
}

void int_function::addMissingBlock(image_basicBlock & imgBlock)
{
    Address baseAddr = this->getAddress() - ifunc()->getOffset();
    int_basicBlock *intBlock = findBlockByAddr( 
        imgBlock.firstInsnOffset() + baseAddr );

    if ( intBlock && &imgBlock != intBlock->llb() ) 
    {
        // the block was split during parsing, (or there's real block 
        // overlapping) adjust the end and lastInsn fields of both 
        // bblInstances 
        bblInstance *curInst = intBlock->origInstance();
        image_basicBlock *curImgB = intBlock->llb();
        Address blockBaseAddr = curInst->firstInsnAddr() - 
            curImgB->firstInsnOffset();
        curInst->setEndAddr( curImgB->endOffset() + blockBaseAddr );
        curInst->setLastInsnAddr( curImgB->lastInsnOffset() + blockBaseAddr );
        // instance 2
        bblInstance *otherInst = findBlockInstanceByAddr
                        (imgBlock.firstInsnOffset() + baseAddr);
        if (otherInst && otherInst != curInst) {
            curInst = otherInst;
            curImgB = intBlock->llb();
            blockBaseAddr = curInst->firstInsnAddr() - 
                curImgB->firstInsnOffset();
            curInst->setEndAddr( curImgB->endOffset() + blockBaseAddr );
            curInst->setLastInsnAddr(curImgB->lastInsnOffset() + blockBaseAddr);
        }

        // now try and find the block again
        int_basicBlock *newIntBlock = findBlockByAddr( 
            imgBlock.firstInsnOffset() + baseAddr );
        if (intBlock == newIntBlock) {
            // there's real overlapping going on, find the intBlock 
            // that starts at the right address (if there is one) 
            mal_printf("WARNING: overlapping blocks, major obfuscation or "
                    "bad parse [%lx %lx] [%lx %lx] %s[%d]\n",
                    intBlock->origInstance()->firstInsnAddr(), 
                    intBlock->origInstance()->endAddr(), 
                    baseAddr + imgBlock.firstInsnOffset(), 
                    baseAddr + imgBlock.endOffset(), 
                    FILE__,__LINE__);
            
            intBlock = NULL;
            for (set<int_basicBlock*,int_basicBlock::compare>::iterator 
                 bIter = blockList.begin();
                 bIter != blockList.end(); 
                 bIter++) 
            {
                if ( (baseAddr + imgBlock.firstInsnOffset()) == 
                     (*bIter)->origInstance()->firstInsnAddr() )
                {
                    intBlock = (*bIter);
                    break;
                }
            }
        } else {
            intBlock = newIntBlock;
        }
    }

    if ( ! intBlock ) {
        // create new int_basicBlock and add it to our datastructures
        intBlock = new int_basicBlock
            ( &imgBlock, baseAddr, this, nextBlockID );
        bblInstance *bbi = intBlock->origInstance();
        assert(bbi);
        blocksByAddr_.insert(bbi);
        nextBlockID++;
        blockList.insert(intBlock);
        blockIDmap[imgBlock.id()] = blockIDmap.size();
    } 
}


/* Find image_basicBlocks that are missing from these datastructures and add
 * them.  The int_basicBlock constructor does pretty much all of the work in
 * a chain of side-effects extending all the way into the mapped_object class
 * 
 * We have to take into account that additional parsing may cause basic block splitting,
 * in which case it is necessary not only to add new int-level blocks, but to update 
 * int_basicBlock, bblInstance, and BPatch_basicBlock objects. 
 */
void int_function::addMissingBlocks()
{
    if ( blockList.empty() ) 
        blocks();

    Function::blocklist & imgBlocks = ifunc_->blocks();
    Function::blocklist::iterator sit = imgBlocks.begin();
    for( ; sit != imgBlocks.end(); ++sit) {
        addMissingBlock( *dynamic_cast<image_basicBlock*>(*sit) );
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

// get instPoints of known function callsinto this one
void int_function::getCallerPoints(std::vector<instPoint*>& callerPoints)
{
    int_basicBlock *entryBlock = findBlockByAddr(getAddress());
    assert(entryBlock);
    pdvector<int_basicBlock*> sourceBlocks;
    entryBlock->getSources(sourceBlocks);
    for (unsigned bIdx=0; bIdx < sourceBlocks.size(); bIdx++) {
        instPoint *callPoint = sourceBlocks[bIdx]->func()->findInstPByAddr
            (sourceBlocks[bIdx]->origInstance()->lastInsnAddr());
        if (!callPoint) {
            sourceBlocks[bIdx]->func()->funcCalls();
            callPoint = sourceBlocks[bIdx]->func()->findInstPByAddr
                (sourceBlocks[bIdx]->origInstance()->lastInsnAddr());
        }
        if (callPoint) {
            callerPoints.push_back(callPoint);
        }
    }
}


instPoint *int_function::findInstPByAddr(Address addr) {
    // This only finds instPoints that have been previously created...
    // so don't bother parsing. 
    
    if (instPsByAddr_.find(addr))
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

void int_function::registerInstPointAddr(Address addr, instPoint *inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    if (oldInstP) assert(inst == oldInstP);

    instPsByAddr_[addr] = inst;
}

void int_function::unregisterInstPointAddr(Address addr, instPoint* inst) {
    instPoint *oldInstP = findInstPByAddr(addr);
    assert(oldInstP == inst);

    instPsByAddr_.undef(addr);
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

bblInstance *int_function::findBlockInstanceByAddr(Address addr) {
    codeRange *range;
    if (blockList.empty()) {
        // Will make the block list...
        blocks();
    }
    
    if (blocksByAddr_.find(addr, range)) {
        assert(range->is_basicBlockInstance());
        return range->is_basicBlockInstance();
    }
    return NULL;
}

int_basicBlock *int_function::findBlockByAddr(Address addr) {
    bblInstance *inst = findBlockInstanceByAddr(addr);
    if (inst)
        return inst->block();
    else
        return NULL;
}


const std::set<int_basicBlock*,int_basicBlock::compare> &int_function::blocks()
{
    int i = 0;

    if (blockList.empty()) {
        Address base = getAddress() - ifunc_->getOffset();

        Function::blocklist & img_blocks = ifunc_->blocks();
        Function::blocklist::iterator sit = img_blocks.begin();

        for( ; sit != img_blocks.end(); ++sit) {
            image_basicBlock *b = (image_basicBlock*)*sit;
            blockList.insert( new int_basicBlock(b, base, this, i) );
            blockIDmap[b->id()] = i;
            ++i;
        }
        nextBlockID = i;
    }
    return blockList;
}

AddressSpace *int_basicBlock::proc() const {
    return func()->proc();
}

// Note that code sharing is masked at this level. That is, edges
// to and from a block that do not originate from the low-level function
// that this block's int_function represents will not be included in
// the returned block collection
void int_basicBlock::getSources(pdvector<int_basicBlock *> &ins) const {

    /* Only allow edges that are within this current function; hide sharing */
    /* Also avoid CALL and RET edges */
    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);

    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred2);

    for( ; eit != ib_ins.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);

        image_basicBlock * sb = (image_basicBlock*)(*eit)->src();
        int_basicBlock *sblock = func()->findBlockByAddr
            ( sb->start() + 
              func()->getAddress() - 
              func()->ifunc()->getOffset() );
        if (!sblock) {
            fprintf(stderr,"ERROR: no corresponding intblock for "
                    "imgblock #%d at 0x%lx %s[%d]\n", ib_->id(),
                    ib_->firstInsnOffset(),FILE__,__LINE__); 
            assert(0);
        }
        ins.push_back( sblock );
    }
}

void int_basicBlock::getTargets(pdvector<int_basicBlock *> &outs) const {
    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);
    NoSinkPredicate epred3(&epred2);

    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred3);

    for( ; eit != ib_outs.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);
        image_basicBlock * tb = (image_basicBlock*)(*eit)->trg();
        int_basicBlock* tblock = func()->findBlockByAddr
            ( tb->start() + 
              func()->getAddress() - 
              func()->ifunc()->getOffset() );
        if (!tblock) {
            fprintf(stderr,"ERROR: no corresponding intblock for "
                    "imgblock #%d at 0x%lx %s[%d]\n", ib_->id(),
                    ib_->firstInsnOffset(),FILE__,__LINE__);                    
            assert(0);
        }
        outs.push_back(tblock);
    }
}

EdgeTypeEnum int_basicBlock::getTargetEdgeType(int_basicBlock * target) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred);
    for( ; eit != ib_outs.end(); ++eit)
        if((*eit)->trg() == target->ib_)
            return (*eit)->type();
    return NOEDGE;
}

EdgeTypeEnum int_basicBlock::getSourceEdgeType(int_basicBlock *source) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred);
    for( ; eit != ib_ins.end(); ++eit)
        if((*eit)->src() == source->ib_)
            return (*eit)->type();
    return NOEDGE;
}

int_basicBlock *int_basicBlock::getFallthrough() const {
    SingleContext epred(func()->ifunc(),true,true);
    NoSinkPredicate epred2(&epred);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred2);
    for( ; eit != ib_outs.end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == FALLTHROUGH ||
           e->type() == CALL_FT ||
           e->type() == COND_NOT_TAKEN)
        {
            return func()->findBlockByAddr
                ( ((image_basicBlock*)e->trg())->firstInsnOffset() + 
                  func()->getAddress()-func()->ifunc()->getOffset() );
        }
    }
    return NULL;
}

bool int_basicBlock::needsRelocation() const {
   if(ib_->isShared() || ib_->needsRelocation()) {
        // If we've _already_ relocated, then we're no longer shared
        // because we have our own copy.

        if (instances_.size() > 1) {
            return false;
        }

        // We have only the one instance, so we're still shared.
        return true;
    }
    //else if(isEntryBlock() && func()->containsSharedBlocks())
    //    return true;
    else
        return false;
}

bool int_basicBlock::isEntryBlock() const { 
    return ib_->isEntryBlock(func_->ifunc());
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
    for (set< int_basicBlock * , int_basicBlock::compare >::const_iterator 
             cb = blockList.begin();
         cb != blockList.end(); 
         cb++) 
    {
        bblInstance* orig = (*cb)->origInstance();
        fprintf(stderr, "  Block start 0x%lx, end 0x%lx\n", orig->firstInsnAddr(),
                orig->endAddr());
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

void int_function::addBBLInstance(bblInstance *instance) {
    assert(instance);
    blocksByAddr_.insert(instance);
}

void int_function::deleteBBLInstance(bblInstance *instance) {
    assert(instance);
    blocksByAddr_.remove(instance->firstInsnAddr());
}

image_func *int_function::ifunc() {
    return ifunc_;
}

int int_basicBlock_count = 0;

int_basicBlock::int_basicBlock(image_basicBlock *ib, Address baseAddr, int_function *func, int id) :
    func_(func),
    ib_(ib),
    id_(id)
{
#if defined(ROUGH_MEMORY_PROFILE)
    int_basicBlock_count++;
    if ((int_basicBlock_count % 100) == 0)
        fprintf(stderr, "int_basicBlock_count: %d (%d)\n",
                int_basicBlock_count, int_basicBlock_count*sizeof(int_basicBlock));
#endif

    bblInstance *inst = new bblInstance(ib->firstInsnOffset() + baseAddr,
                                        ib->lastInsnOffset() + baseAddr,
                                        ib->endOffset() + baseAddr,
                                        this, 
                                        0);
    instances_.push_back(inst);
    assert(func_);
    func_->addBBLInstance(inst);
}

int_basicBlock::int_basicBlock(const int_basicBlock *parent, int_function *func,int id) :
    func_(func),
    ib_(parent->ib_),
    id_(id)
{
    for (unsigned i = 0; i < parent->instances_.size(); i++) {
        bblInstance *bbl = new bblInstance(parent->instances_[i], this);
        instances_.push_back(bbl);
        func_->addBBLInstance(bbl);
    }
}

int_basicBlock::~int_basicBlock() {
    // don't kill func_;
    // don't kill ib_;
    for (unsigned i = 0; i < instances_.size(); i++) {
        delete instances_[i];
    }
#if defined (cap_use_pdvector)
    instances_.zap();
#else
    instances_.clear();
#endif
}

bblInstance *int_basicBlock::origInstance() const {
    assert(instances_.size());
    return instances_[0];
}

bblInstance *int_basicBlock::instVer(unsigned id) const {
    if (id >= instances_.size())
    {
        fprintf(stderr, "ERROR: requesting bblInstance %u, only %d known "
                "for block at 0x%lx %s[%d]\n", id, (int)instances_.size(), 
                instances_[0]->firstInsnAddr(), FILE__,__LINE__);
        return instances_[instances_.size()-1];
    }
    return instances_[id];
}

void int_basicBlock::removeVersion(unsigned id, bool deleteInstance) {
    if (id >= instances_.size()) {
        fprintf(stderr, "ERROR: deleting bblInstance %u, only %ld known\n",
                id, (long) instances_.size());
        return;
    }
    if (id < (instances_.size() - 1)) {
        fprintf(stderr, "ERROR: deleting bblInstance %u, not last\n",
                id);
        assert(0);
        return;
    }
    if (deleteInstance) {
        bblInstance *inst = instances_[id];
        delete inst;
    }
    instances_.pop_back();
}


const pdvector<bblInstance *> &int_basicBlock::instances() const {
    return instances_;
}

int bblInstance_count = 0;

bblInstance::bblInstance(Address start, Address last, Address end, int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(start),
    lastInsnAddr_(last),
    blockEndAddr_(end),
    block_(parent),
    version_(version)
{
#if defined(ROUGH_MEMORY_PROFILE)
    bblInstance_count++;
    if ((bblInstance_count % 100) == 0)
        fprintf(stderr, "bblInstance_count: %d (%d)\n",
                bblInstance_count, bblInstance_count*sizeof(bblInstance));
#endif


    // And add to the mapped_object code range
    block_->func()->obj()->codeRangesByAddr_.insert(this);
};

bblInstance::bblInstance(int_basicBlock *parent, int version) : 
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(0),
    lastInsnAddr_(0),
    blockEndAddr_(0),
    block_(parent),
    version_(version)
{
    // And add to the mapped_object code range
    //block_->func()->obj()->codeRangesByAddr_.insert(this);
};

bblInstance::bblInstance(const bblInstance *parent, int_basicBlock *block) :
#if defined(cap_relocation)
    reloc_info(NULL),
#endif
    firstInsnAddr_(parent->firstInsnAddr_),
    lastInsnAddr_(parent->lastInsnAddr_),
    blockEndAddr_(parent->blockEndAddr_),
    block_(block),
    version_(parent->version_) {
#if defined(cap_relocation)
   if (parent->reloc_info) {
      reloc_info = new reloc_info_t(parent->reloc_info, block);
   }
#endif

    // If the bblInstance is the original version, add to the mapped_object
    // code range; if it is the product of relocation, add it to the
    // process.
    if(version_ == 0)
        block_->func()->obj()->codeRangesByAddr_.insert(this);
    else
        block_->func()->obj()->proc()->addOrigRange(this);
}

bblInstance::~bblInstance() {
#if defined(cap_relocation)
   if (reloc_info)
      delete reloc_info;
#endif
}

int_basicBlock *bblInstance::block() const {
    if ( ! func()->obj()->isExploratoryModeOn() ) {
        assert(block_);
    }
    return block_;
}

void int_basicBlock::setHighLevelBlock(void *newb)
{
   highlevel_block = newb;
}

void *int_basicBlock::getHighLevelBlock() const {
   return highlevel_block;
}

bool int_basicBlock::containsCall()
{
    Block::edgelist & out_edges = llb()->targets();
    Block::edgelist::iterator eit = out_edges.begin();
    for( ; eit != out_edges.end(); ++eit) {
        if ( CALL == (*eit)->type() ) {
            return true;
        }
    }
    return false;
}

int_function *bblInstance::func() const {
    assert(block_);
    return block_->func();
}

AddressSpace *bblInstance::proc() const {
    assert(block_);
    return block_->func()->proc();
}


static Address relocLookup
(const pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> relocs, Address addr)
{
    for (unsigned i = 0; i < relocs.size(); i++) {
        if (relocs[i]->origAddr == addr)
	        return relocs[i]->relocAddr;
        if (relocs[i]->relocAddr == addr)
            return relocs[i]->origAddr;
    }
    return 0;
}

// addr can correspond to the new block or to the "this" block, unless
// neither of the versions is an origInstance, in which case addr should 
// correspond to the "this" block
Address bblInstance::equivAddr(int newVersion, Address addr) const {

    Address translAddr = 0;

    if (newVersion == version()) {
        translAddr = addr;
        return translAddr;
    }

#if defined(cap_relocation)

    // account for possible prior deletion of the int_basicBlock in 
    // exploratory mode
    if ( NULL == block_ ) {
        if ( ! func()->obj()->isExploratoryModeOn() ) {
            assert(0);
        }
        assert(0 == newVersion && "translating to non-zero version after int_basicBlock deletion");

        unsigned int iidx=0;
        while (iidx < get_relocs().size() && 
               addr != get_relocs()[iidx]->origAddr) 
           iidx++;
        if (iidx < get_relocs().size()) {
            translAddr = get_relocs()[iidx]->origAddr;
        } else {
            mal_printf("%s[%d] WARNING: returning 0 in equivAddr, called on "
                    "bblInstance at %lx whose block has been deleted %lx\n", 
                    FILE__,__LINE__,firstInsnAddr_);
            return 0;
        }
    }

    assert (newVersion < (int)block_->instances().size());

    // do the translation
    if (0 == version()) {
        translAddr = relocLookup(block_->instVer(newVersion)->get_relocs(), 
                                 addr);
    }
    else if (0 == newVersion) {
        translAddr = relocLookup(get_relocs(), addr);
    } else { // neither version is non-zero, first translate to origInstance, 
             // then to the new version instance
        translAddr = block()->origInstance()->equivAddr(newVersion, 
                                                        equivAddr(0,addr));
    }

#endif

    if (!translAddr) {
        fprintf(stderr,"ERROR: returning 0 in equivAddr, called on bblInstance"
                " at %lx for new version %d in function at %lx %s[%d]\n", 
                firstInsnAddr_, newVersion, 
                block()->func()->getAddress(),FILE__,__LINE__);
        return 0;
    }
    return translAddr;
}

void *bblInstance::getPtrToInstruction(Address addr) const {
    if (addr < firstInsnAddr_) return NULL;
    if (addr >= blockEndAddr_) return NULL;

#if defined(cap_relocation)
    if (version_ > 0) {
      // We might be relocated...
      if (getGeneratedBlock() != NULL) {
        addr -= firstInsnAddr();
        return getGeneratedBlock().get_ptr(addr);
      }
    }
#endif
    
    return func()->obj()->getPtrToInstruction(addr);

}

void *bblInstance::get_local_ptr() const {
#if defined(cap_relocation)
    if (!reloc_info) return NULL; 
    return reloc_info->generatedBlock_.start_ptr();
#else
    return NULL;
#endif
}

int bblInstance::version() const 
{
   return version_;
}

#if defined(cap_relocation)

const void *bblInstance::getPtrToOrigInstruction(Address addr) const {
  if (version_ > 0) {
    for (unsigned i = 0; i < get_relocs().size(); i++) {
      if (get_relocs()[i]->relocAddr == addr) {
         return (const void *) get_relocs()[i]->origPtr;
      }
    }
    assert(0);
    return NULL;
  }

  return getPtrToInstruction(addr);
}

unsigned bblInstance::getRelocInsnSize(Address addr) const {
  if (version_ > 0) {
    for (unsigned i = 0; i < get_relocs().size()-1; i++) {
      if (get_relocs()[i]->relocAddr == addr)
	return get_relocs()[i+1]->relocAddr - get_relocs()[i]->relocAddr;
    }
    if (get_relocs()[get_relocs().size()-1]->relocAddr == addr) {
      return blockEndAddr_ - get_relocs()[get_relocs().size()-1]->relocAddr;
    }
    assert(0);
    return 0;
  }
  // ... uhh...
  // This needs to get handled by the caller

  return 0;
}

void bblInstance::getOrigInstructionInfo(Address addr, const void *&ptr, 
                                         Address &origAddr, 
                                         unsigned &origSize) const 
{
   if (version_ > 0) {
      fprintf(stderr, "getPtrToOrigInstruction 0x%lx, version %d\n",
              addr, version_);
      for (unsigned i = 0; i < get_relocs().size(); i++) {
         if (get_relocs()[i]->relocAddr == addr) {
            fprintf(stderr, "... returning 0x%lx off entry %d\n",
                    get_relocs()[i]->origAddr,i);
            ptr = get_relocs()[i]->origPtr;
            origAddr = get_relocs()[i]->origAddr;
            if (i == (get_relocs().size()-1)) {
               origSize = blockEndAddr_ - get_relocs()[i]->relocAddr;
            }
            else
               origSize = get_relocs()[i+1]->relocAddr - get_relocs()[i]->relocAddr;
            return;
         }
      }
      assert(0);
      return;
   }
   
   // Must be handled by caller
   ptr = NULL;
   origAddr = 0;
   origSize = 0;
   return;
}

unsigned &bblInstance::maxSize() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->maxSize_;
}

unsigned &bblInstance::minSize() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->minSize_;
}

bblInstance *&bblInstance::origInstance() {
   if (!reloc_info) {
      reloc_info = new reloc_info_t();
      reloc_info->origInstance_ = block_->origInstance();
   }
   return reloc_info->origInstance_;
}

pdvector<funcMod *> &bblInstance::appliedMods() {
   if (!reloc_info)
      reloc_info = new reloc_info_t();
   return reloc_info->appliedMods_;
}

codeGen &bblInstance::generatedBlock() {
  if (!reloc_info)
      reloc_info = new reloc_info_t();
  return reloc_info->generatedBlock_;
}

functionReplacement *&bblInstance::jumpToBlock() {
  if (!reloc_info)
      reloc_info = new reloc_info_t();
  return reloc_info->jumpToBlock_;
}

pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &bblInstance::get_relocs() const {
  assert(reloc_info);
  return reloc_info->relocs_;
}

pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &bblInstance::relocs() {
  if (!reloc_info)
    reloc_info = new reloc_info_t();
  return get_relocs();
}

unsigned bblInstance::getMaxSize() const {
   if (!reloc_info)
      return 0;
   return reloc_info->maxSize_;
}

bblInstance *bblInstance::getOrigInstance() const {
   if (!reloc_info)
      return NULL;
   return reloc_info->origInstance_;
}

pdvector<funcMod *> &bblInstance::getAppliedMods() const {
   assert(reloc_info);
   return reloc_info->appliedMods_;
}

codeGen &bblInstance::getGeneratedBlock() const {
   assert(reloc_info);
   return reloc_info->generatedBlock_;
}

functionReplacement *bblInstance::getJumpToBlock() const {
   if (!reloc_info)
      return NULL;
   return reloc_info->jumpToBlock_;
}


bblInstance::reloc_info_t::reloc_info_t() : 
   maxSize_(0), 
   minSize_(0), 
   origInstance_(NULL), 
   jumpToBlock_(NULL),
   funcRelocBase_(0)
{};

bblInstance::reloc_info_t::reloc_info_t(reloc_info_t *parent, 
                                        int_basicBlock *block)  :
   maxSize_(0),
   minSize_(0),
   funcRelocBase_(0)
{
   if (parent->origInstance_)
      origInstance_ = block->instVer(parent->origInstance_->version());
   else
      origInstance_ = NULL;

   if (parent->jumpToBlock_)
       jumpToBlock_ = new functionReplacement(*(parent->jumpToBlock_));
   else
       jumpToBlock_ = NULL;

   for (unsigned i = 0; i < parent->relocs_.size(); i++) {
     relocs_.push_back( parent->relocs_[i] );
   }

}

bblInstance::reloc_info_t::~reloc_info_t() {
  // XXX this wasn't safe, as copies of bblInstances
  //     reference the same relocInsns.
  //     relocs_ now holds shared_ptrs
  //for (unsigned i = 0; i < relocs_.size(); i++) {
  //  delete relocs_[i];
  //}

#if defined (cap_use_pdvector)
  relocs_.zap();
#else
  relocs_.clear();
#endif

   // appliedMods is deleted by the function....
   // jumpToBlock is deleted by the process....
};

#endif

int_basicBlock *functionReplacement::source() { 
   return sourceBlock_; 
}

int_basicBlock *functionReplacement::target() { 
   return targetBlock_; 
}

unsigned functionReplacement::sourceVersion() { 
   return sourceVersion_; 
}

unsigned functionReplacement::targetVersion() { 
   return targetVersion_; 
}


// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool int_function::getSharingFuncs(int_basicBlock *b,
                                   pdvector< int_function *> & funcs)
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

        // Let's see if we've already got it...
        bool found = false;
        for (unsigned j = 0; j < funcs.size(); j++) {
            if (funcs[j] == hl_func) {
                found = true;
                break;
            }
        }
        if (!found) {
            ret = true;
            funcs.push_back(hl_func);
        }
    }

    return ret;
}

// Find overlapping functions via checking all basic blocks. We might be
// able to check only exit points; but we definitely need to check _all_
// exits so for now we're checking everything.

bool int_function::getOverlappingFuncs(pdvector<int_function *> &funcs) {
    bool ret = false;

    funcs.clear();

    // Create the block list.
    blocks();

    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); 
         bIter != blockList.end(); 
         bIter++) {
        if (getSharingFuncs(*bIter,funcs))
            ret = true;
    }

    return ret;
}

Address int_function::get_address() const 
{
#if !defined(cap_relocation)
   return getAddress();
#else
   if (!entryPoints_.size())
      return getAddress();
   
   instPoint *entryPoint = entryPoints_[0];
   int_basicBlock *block = entryPoint->block();
   bblInstance *inst = block->instVer(installedVersion_);
   return inst->firstInsnAddr();
#endif 
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

bblInstance * bblInstance::getTargetBBL() {
    // Check to see if we need to fix up the target....
    pdvector<int_basicBlock *> targets;
    block_->getTargets(targets);
    
    // We have edge types on the internal data, so we drop down and get that. 
    // We want to find the "branch taken" edge and override the destination
    // address for that guy.
    Block::edgelist & out_edges = block_->llb()->targets();
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());
   
    Block::edgelist::iterator eit = out_edges.begin();
    for( ; eit != out_edges.end(); ++eit) {
        EdgeTypeEnum edgeType = (*eit)->type();
        if ((edgeType == COND_TAKEN) ||
            (edgeType == DIRECT)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = (image_basicBlock*)(*eit)->trg();
            int_basicBlock *hlTarget = NULL;
            for (unsigned t_iter = 0; t_iter < targets.size(); t_iter++) {
                // Should be the same index, but this is a small set...
                if (targets[t_iter]->llb() == llTarget) {
                    hlTarget = targets[t_iter];
                    break;
                }
            }
            if (hlTarget == NULL) {
                fprintf(stderr, "targets:%d out_edges:%d src:0x%lx->0x%lx trg:0x%lx->0x%lx\n", targets.size(), out_edges.size(), (*eit)->src()->start(), (*eit)->src()->end(), (*eit)->trg()->start(), (*eit)->trg()->end());
            }

            assert(hlTarget != NULL);
            return hlTarget->instVer(version_);
        }
    }
    return NULL;
}

bblInstance * bblInstance::getFallthroughBBL() {

    if (func()->obj()->isExploratoryModeOn()) {
        // if this bblInstance has been invalidated, see if block splitting has
        // happened, in which case, get the latter of the two blocks and 
        // return its fallthrough block 
        if ( block_->instances().size() <= (unsigned) version_ ||
             this != block_->instVer(version_) ) 
        {
            bblInstance *origInst = func()->findBlockInstanceByAddr
                (get_relocs().back()->origAddr);
            return origInst->getFallthroughBBL();
        }
    }

    // Check to see if we need to fix up the target....
    pdvector<int_basicBlock *> targets;
    block_->getTargets(targets);
    
    // We have edge types on the internal data, so we drop down and get that. 
    // We want to find the "branch taken" edge and override the destination
    // address for that guy.
    Block::edgelist & out_edges = block_->llb()->targets();
    
    // May be greater; we add "extra" edges for things like function calls, etc.
    assert (out_edges.size() >= targets.size());

    NoSinkPredicate nsp;
    
    Block::edgelist::iterator eit = out_edges.begin(&nsp);
    for( ; eit != out_edges.end(); ++eit) {
        EdgeTypeEnum edgeType = (*eit)->type();
        if ((edgeType == COND_NOT_TAKEN) ||
            (edgeType == FALLTHROUGH) ||
            (edgeType == CALL_FT)) {
            // Got the right edge... now find the matching high-level
            // basic block
            image_basicBlock *llTarget = (image_basicBlock*)(*eit)->trg();
            int_basicBlock *hlTarget = NULL;
            for (unsigned t_iter = 0; t_iter < targets.size(); t_iter++) {
                // Should be the same index, but this is a small set...
                if (targets[t_iter]->llb() == llTarget) {
                    hlTarget = targets[t_iter];
                    break;
                }
            }
            assert(hlTarget != NULL);
            
            return hlTarget->instVer(version_);
        }
    }
    return NULL;
}


bool int_function::performInstrumentation(bool stopOnFailure,
                                          pdvector<instPoint *> &failedInstPoints) {

    // We have the following possible side-effects:
    // 
    // 1) Generating an instPoint (e.g., creating the multiTramp and its code)
    //    may determine the function is too small to fit the instrumentation,
    //    requiring relocation.
    // 2) Instrumenting a shared block may also trigger relocation as a 
    //    mechanism to unwind the sharing. 
    //
    // 3) Relocation will add additional instPoint instances.
    //

    // Thus, we have the following order of events:
    //
    // 1) Generate all instPoints that actually have instrumentation. 
    //    This will identify whether the function requires relocation.
    // 2) If relocation is necessary:
    // 2a) Generate relocation; this will create the function copy and update 
    //     function-local data structures.
    // 2b) Install relocation; this will update process-level data structures and
    //     copy the relocated function into the address space.
    // 2c) Generate instPoints again to handle any new instPointInstances that
    //     have showed up. This should _not_ result in required relocation.
    // 3) Install instPoints
    // 4) Link (relocated copy of the function) and instPoints.

    // Assumptions: 
    // 1) calling generate/install/link on "empty" instPoints has no effect.
    // 2) Generate/install/link operations are idempotent.

    // Let's avoid a lot of work and collect up all instPoints that have
    // something interesting going on; that is, that have instrumentation
    // added since the last time something came up. 

#if defined(arch_x86_64)
  if(proc()->getAddressWidth() == 8)
  {
    ia32_set_mode_64(true);
  }
  else
  {
    ia32_set_mode_64(false);
  }
#endif  

  if (isBeingInstrumented_) return false;
  isBeingInstrumented_ = true;

    std::set<instPoint *> newInstrumentation;
    std::set<instPoint *> anyInstrumentation;

    getNewInstrumentation(newInstrumentation);
    getAnyInstrumentation(anyInstrumentation);

    // Quickie correctness assert: newInstrumentation \subseteq anyInstrumentation
    assert(newInstrumentation.size() <= anyInstrumentation.size()); 

    bool relocationRequired = false;

    // Step 1: Generate all new instrumentation
    generateInstrumentation(newInstrumentation, failedInstPoints, relocationRequired); 
    
    if (failedInstPoints.size() && stopOnFailure) {
      isBeingInstrumented_ = false;
      return false;
    }

#if defined(cap_relocation)
    // Step 2: is relocation necessary?
    if (relocationRequired) {
        // Yar.
        // This will calculate the sizes required for our basic blocks.
        expandForInstrumentation();
        
        // And keep a list of other functions that need relocation due to
        // sharing.
        pdvector<int_function *> need_reloc;

        // Generate the relocated copy of the function.
        relocationGenerate(enlargeMods(), 0, need_reloc);
        
        // Install the relocated copy of the function.
        relocationInstall();

        // Aaaand link it. 
        pdvector<codeRange *> overwritten_objs;
        relocationLink(overwritten_objs);

        // We've added a new version of the function; therefore, we need
        // to update _everything_ that's been instrumented. 
        // We do this in two ways. First, we call generate on all
        // instPoints to get them in the right place.
        // Second, we replace newInstrumentation with anyInstrumentation,
        // then call install/link as normal.

        // Clear the failedInstPoints vector first; we'll re-generate
        // it in any case.
        failedInstPoints.clear();
        relocationRequired = false;

        // Update instPoint instances to include the new function
        for (std::set<instPoint*>::iterator iter = anyInstrumentation.begin();
             iter != anyInstrumentation.end(); 
             iter++) 
        {
            (*iter)->updateInstancesBatch();
        }
        // We _explicitly_ don't call the corresponding updateInstancesFinalize,
        // as the only purpose of that function is to regenerate instrumentation;
        // we do that explicitly below.

        generateInstrumentation(anyInstrumentation,
                                failedInstPoints,
                                relocationRequired);
        // I'm commenting this out; I originally thought it would be the case,
        // but on further thought the original instPoint will _still_ be asking
        // for relocation. 
        //assert(relocationRequired == false);

        newInstrumentation = anyInstrumentation;

	// If there are any other functions that we need to relocate
	// due to this relocation, handle it now. We don't care if they
	// fail to install instrumentation though.
	pdvector<instPoint *> dontcare;
	for (unsigned i = 0; i < need_reloc.size(); i++) {
	  need_reloc[i]->performInstrumentation(false, dontcare);
	}

        mal_printf("%s[%d] relocating function at %lx\n", 
                   __FILE__,__LINE__,getAddress());
    }
#endif

    // Okay, back to what we were doing...
    
    installInstrumentation(newInstrumentation,
                           failedInstPoints);
    linkInstrumentation(newInstrumentation,
                        failedInstPoints);

    if (obj()->isSharedLib()) {
        //printf("===> Instrumenting function in shared library: %s [%s]\n",
                //prettyName().c_str(), obj()->fileName().c_str());
        obj()->setDirty();
    }

    isBeingInstrumented_ = false;
    return (failedInstPoints.size() == 0);
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

void int_function::generateInstrumentation(std::set<instPoint *> &input,
                                           pdvector<instPoint *> &failed,
                                           bool &relocationRequired) {
    for (std::set<instPoint*>::iterator iter = input.begin();
         iter != input.end(); 
         iter++) 
    {
        switch ((*iter)->generateInst()) {
        case instPoint::tryRelocation:
            relocationRequired = true;
            break;
        case instPoint::generateSucceeded:
            break;
        case instPoint::generateFailed:
            failed.push_back(*iter);
            break;
        default:
            assert(0);
            break;
        }
    }
}

void int_function::installInstrumentation(std::set<instPoint *> &input,
                                          pdvector<instPoint *> &failed) {
    for (std::set<instPoint*>::iterator iter = input.begin();
         iter != input.end(); 
         iter++) 
    {
        switch ((*iter)->installInst()) {
        case instPoint::wasntGenerated:
            break;
        case instPoint::installSucceeded:
            break;
        case instPoint::installFailed:
            failed.push_back(*iter);
            break;
        default:
            assert(0);
            break;
        }
    }
}


void int_function::linkInstrumentation(std::set<instPoint *> &input,
                                       pdvector<instPoint *> &failed) {
    for (std::set<instPoint*>::iterator iter = input.begin();
         iter != input.end(); 
         iter++) 
    {
        switch ((*iter)->linkInst()) {
        case instPoint::wasntInstalled:
            break;
        case instPoint::linkSucceeded:
            break;
        case instPoint::linkFailed:
            failed.push_back(*iter);
            break;
        default:
            assert(0);
            break;
        }
    }
}


Offset int_function::addrToOffset(const Address addr) const { 
    return addr - getAddress() + ifunc_->getOffset(); 
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

#if defined(cap_instruction_api) 
void bblInstance::getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> >&instances) const {
  instances.clear();
  block()->llb()->getInsnInstances(instances);
  for (unsigned i = 0; i < instances.size(); ++i) {
    instances[i].second += firstInsnAddr_ - block()->llb()->start();
  }
}
#endif
#if 0
int_basicBlock *int_function::findBlockByImage(image_basicBlock *block) {
  unsigned img_id = block->id();
  unsigned int_id = blockIDmap[img_id];
  return blockList[int_id];
}
#endif


/* removes all function blocks in the specified range
 */
bool int_function::removeFunctionSubRange(
                   Address startAddr, 
                   Address endAddr, 
                   std::vector<Address> &deadBlockAddrs,
                   int_basicBlock *&entryBlock)
{
    std::vector<int_basicBlock *> deadBlocks;
    std::vector<ParseAPI::Block *> papiDeadBlocks;

    findBlocksByRange(deadBlocks,startAddr,endAddr);

    // warning if blocks are instrumented
    vector<int_basicBlock *>::iterator biter = deadBlocks.begin();
    for (; biter != deadBlocks.end(); biter++) {
        assert( (*biter)->func() == this );
        codeRange* range = proc()->findModByAddr
            ((*biter)->origInstance()->firstInsnAddr());
        if (range) {
            fprintf(stderr,"WARNING: mod range %lx %lx for purged block "
                    "%lx %lx %s[%d]\n", range->get_address(), 
                    range->get_address()+range->get_size(),
                    (*biter)->origInstance()->firstInsnAddr(),
                    (*biter)->origInstance()->endAddr(),FILE__,__LINE__);
            proc()->removeModifiedRange(range);
            return false;
        }
        deadBlockAddrs.push_back((*biter)->origInstance()->firstInsnAddr());
        papiDeadBlocks.push_back((*biter)->llb());
    }
    
    // set new entry point 
    setNewEntryPoint( entryBlock );

    // remove dead image_basicBlocks and int_basicBlocks
    ifunc()->deleteBlocks( papiDeadBlocks, entryBlock->llb() );
    for (biter = deadBlocks.begin(); biter != deadBlocks.end(); biter++) {
        deleteBlock(*biter);
    }

    return true;
}


