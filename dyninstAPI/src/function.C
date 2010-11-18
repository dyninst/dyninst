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
	if (img_entries.empty()) {
	  cerr << "Warning: function " << prettyName() << " has no parsed entry points" << endl;
	}
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
            if (!findBlockByOffsetInFunc(offsetInFunc)) {
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
            // TEMPORARY FIX: we're seeing points identified by low-level
            // code that aren't actually in the function.            
            Address offsetInFunc = img_exits[i]->offset()-ifunc_->getOffset();
            // add points that we've already seen
            if ( instPsByAddr_.find( offsetInFunc + getAddress() ) ) {
                exitPoints_.push_back( instPsByAddr_[offsetInFunc + getAddress()] );
                continue;
            }
            if (!findBlockByOffsetInFunc(offsetInFunc)) {
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
        //KEVINTODO: exploratory mode can be made more efficient by making 
        // callPoints a set and not clearing it in that mode
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
            if (!findBlockByOffsetInFunc(offsetInFunc)) {
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


bool int_function::removePoint(instPoint *point) 
{
    bool foundPoint = false;
    if (instPsByAddr_.find(point->addr()))
        instPsByAddr_.undef(point->addr());
    switch(point->getPointType()) {
    case functionEntry:
        for (unsigned i = 0; !foundPoint && i < entryPoints_.size(); i++) {
            if (entryPoints_[i] == point) {
                entryPoints_[i] = entryPoints_[entryPoints_.size()-1];
                entryPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case functionExit:
        for (unsigned i = 0; !foundPoint && i < exitPoints_.size(); i++) {
            if (exitPoints_[i]->addr() == point->addr()) {
                exitPoints_[i] = exitPoints_[exitPoints_.size()-1];
                exitPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case callSite:
        for (unsigned i = 0; !foundPoint && i < callPoints_.size(); i++) {
            if (callPoints_[i] == point) {
                callPoints_[i] = callPoints_[callPoints_.size()-1];
                callPoints_.pop_back();
                foundPoint = true;
            }
        }
        break;
    case otherPoint:
        for (unsigned i = 0; !foundPoint && i < arbitraryPoints_.size(); i++) {
            if (arbitraryPoints_[i] == point) {
                arbitraryPoints_[i] = arbitraryPoints_[arbitraryPoints_.size()-1];
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
int_basicBlock * int_function::setNewEntryPoint()
{
    int_basicBlock *newEntry = NULL;

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
        newEntry = *blockList.begin();
    }
    ifunc()->setEntryBlock(newEntry->llb());
    this->addr_ = newEntry->origInstance()->firstInsnAddr();

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
    return newEntry;
}

/* 0. The target and source must be in the same mapped region, make sure memory
 *    for the target is up to date
 * 1. Parse from target address, add new edge at image layer
 * 2. Register all newly created functions as a result of new edge parsing
 * 3. Add image blocks as int_basicBlocks
 * 4. fix up mapping of split blocks with points
 * 5. Add image points, as instPoints 
*/
bool int_function::parseNewEdges(const std::vector<edgeStub> &stubs )
{
    using namespace SymtabAPI;
    using namespace ParseAPI;

    vector<ParseAPI::Block*> sources;
    vector<Address> targets;
    vector<EdgeTypeEnum> edgeTypes;
    for (unsigned sidx = 0; sidx < stubs.size(); sidx++) {
        sources.push_back(stubs[sidx].src->block()->llb());
        targets.push_back(stubs[sidx].trg);
        edgeTypes.push_back(stubs[sidx].type);
    }

/* 0. Make sure memory for the target is up to date */

    // Do various checks and set edge types, if necessary
    Address loadAddr = getAddress() - ifunc()->getOffset();
    for (unsigned idx=0; idx < stubs.size(); idx++) {

        Block *cursrc = stubs[idx].src->block()->llb();

        // update target region if needed
        if (BPatch_defensiveMode == obj()->hybridMode()) {
            obj()->updateCodeBytesIfNeeded(stubs[idx].trg);
        }

        // translate targets to memory offsets rather than absolute addrs
        targets[idx] -= loadAddr;

        // figure out edge types if they have not been set yet
        if (ParseAPI::NOEDGE == stubs[idx].type) {
            Block::edgelist & edges = cursrc->targets();
            Block::edgelist::iterator eit = edges.begin();
            bool isIndirJmp = false;
            bool isCondl = false;
            for (; eit != edges.end(); eit++) {
                if ((*eit)->trg()->start() == stubs[idx].trg) {
                    edgeTypes[idx] = (*eit)->type();
                    break;
                } 
                if (ParseAPI::INDIRECT == (*eit)->type()) {
                    isIndirJmp = true;
                } else if (ParseAPI::COND_NOT_TAKEN == (*eit)->type()
                            || ParseAPI::COND_TAKEN == (*eit)->type()) {
                    isCondl = true;
                }
            }
            if (ParseAPI::NOEDGE == edgeTypes[idx]) {
                bool isCall = false;
                funcCalls();
                instPoint *pt = findInstPByAddr(
                    cursrc->lastInsnAddr()+loadAddr);
                if (pt && callSite == pt->getPointType()) {
                    isCall = true;
                }
                if (cursrc->end() == targets[idx]) {
                    if (isCall) {
                        edgeTypes[idx] = CALL_FT;
                    } else if (isCondl) {
                        edgeTypes[idx] = ParseAPI::COND_NOT_TAKEN;
                    } else {
                        edgeTypes[idx] = ParseAPI::FALLTHROUGH;
                    }
                } else if (isCall) {
                    edgeTypes[idx] = ParseAPI::CALL;
                } else if (isIndirJmp) {
                    edgeTypes[idx] = ParseAPI::INDIRECT;
                } else if (isCondl) {
                    edgeTypes[idx] = ParseAPI::COND_TAKEN;
                } else {
                    edgeTypes[idx] = ParseAPI::DIRECT;
                }
            }
        }
    }
 
/* 1. Parse from target address, add new edge at image layer  */
    assert( !ifunc()->img()->hasSplitBlocks() && 
            !ifunc()->img()->hasNewBlocks());
    ifunc()->img()->codeObject()->parseNewEdges(sources, targets, edgeTypes);

/* 2. Register all newly created image_funcs as a result of new edge parsing */
    obj()->registerNewFunctions();

    for(unsigned sidx=0; sidx < sources.size(); sidx++) {
        vector<ParseAPI::Function*> funcs;
        sources[sidx]->getFuncs(funcs);
        for (unsigned fidx=0; fidx < funcs.size(); fidx++) 
        {
            int_function *func = proc()->findFuncByInternalFunc(
                static_cast<image_func*>(funcs[fidx]));

/* 3. Add img-level blocks and points to int-level datastructures */
            func->addMissingBlocks();
            func->addMissingPoints();

            // invalidate liveness calculations
            func->ifunc()->invalidateLiveness();
        }
    }

/* 5. fix mapping of split blocks that have points */
    if (ifunc()->img()->hasSplitBlocks()) {
        obj()->splitIntLayer();
        ifunc()->img()->clearSplitBlocks();
    }

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
void int_function::fixHandlerReturnAddr(Address faultAddr)
{
    if ( !proc()->proc() || ! handlerFaultAddrAddr_ ) {
        assert(0);
        return;
    }

	// Do a straightfoward forward map of faultAddr
	// First, get the original address
	int_function *func; baseTrampInstance *ignored;
	Address origAddr;
	if (!proc()->getRelocInfo(faultAddr, origAddr, func, ignored)) {
		func = dynamic_cast<process *>(proc())->findActiveFuncByAddr(faultAddr);
		origAddr = faultAddr;
		}
	std::list<Address> relocAddrs;
	proc()->getRelocAddrs(origAddr, this, relocAddrs, true);
	Address newPC = (!relocAddrs.empty() ? relocAddrs.back() : origAddr);

	if (newPC != faultAddr) {
            if(!proc()->writeDataSpace((void*)handlerFaultAddrAddr_, 
                                           sizeof(Address), 
                                           (void*)&newPC))
            {
                assert(0);
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
	if (imgBlock->isShared()) {
		cerr << "BAD CASE : block is shared" << endl;
		std::vector<ParseAPI::Function *> funcs;
		imgBlock->getFuncs(funcs);
		for (unsigned i = 0; i < funcs.size(); ++i) {
			cerr << "\t" << i << ": func @ " << hex << funcs[i]->entry()->start() << dec << endl;
			const ParseAPI::Function::blocklist &blocks = funcs[i]->blocks();
			for (ParseAPI::Function::blocklist::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
				cerr << "\t\t Block: " << hex << (*iter)->start() << " -> " << (*iter)->end() << endl;
				const ParseAPI::Block::edgelist &edges = (*iter)->targets();
				for (ParseAPI::Block::edgelist::iterator e_iter = edges.begin(); e_iter != edges.end(); ++e_iter) {
					cerr << "\t\t\t Edge to: " << hex << (*e_iter)->trg()->start() << dec << endl;
					}
				}
			}
		}

	assert( ! imgBlock->isShared() ); //KEVINTODO: unimplemented case
    Address baseAddr = obj()->codeBase();

    // remove parse points
    pdvector<image_instPoint*> imgPoints;
    ifunc()->img()->getInstPoints( origbbi->firstInsnAddr()-baseAddr, 
                                   origbbi->endAddr()-baseAddr, 
                                   imgPoints );
    for (unsigned pidx=0; pidx < imgPoints.size(); pidx++) {
        image_instPoint *imgPt = imgPoints[pidx];
        instPoint *point = findInstPByAddr( imgPt->offset() + baseAddr );
        if (!point) {
            addMissingBlocks();
            point = findInstPByAddr( imgPt->offset() + baseAddr );
        }
        removePoint( point );
    }

    // remove arbitrary points
    for (unsigned pidx=0; pidx < arbitraryPoints_.size(); pidx++) {
        if (origbbi->firstInsnAddr() <= arbitraryPoints_[pidx]->addr() &&
            origbbi->endAddr() > arbitraryPoints_[pidx]->addr()) 
        {
            removePoint(arbitraryPoints_[pidx]); // removes point from the vector
            pidx--;
        }
    }


    // Remove block from int-level datastructures 
    pdvector<bblInstance*> bbis = block->instances();
    obj()->removeRange(bbis[0]);
    for (unsigned bIdx = 1; bIdx < bbis.size(); bIdx++) 
    {   // the original instance is not in the process range
        proc()->removeOrigRange(bbis[bIdx]);
    }
    for (unsigned bIdx=0; bIdx < block->instances_.size(); bIdx++) {
        blocksByAddr_.remove(bbis[bIdx]->firstInsnAddr());
    }
    blockList.erase(block);
    
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
    delete(this);
}

void int_function::addMissingBlock(image_basicBlock & missingB)
{
    Address baseAddr = getAddress() - ifunc()->getOffset();
    bblInstance *bbi = findBlockInstanceByAddr( 
        missingB.firstInsnOffset() + baseAddr );

    if ( bbi && &missingB != bbi->block()->llb() ) 
    {
        image_basicBlock *imgB = bbi->block()->llb();
        // Check to see if missingB and imgB overlap
        // If that's the case, missingB's end must lie within imgB's range
        // or vice versa
        Address higherStart = (missingB.start() > imgB->start()) ? missingB.start() : imgB->start();
        Address lowerEnd = (missingB.end() < imgB->end()) ? missingB.end() : imgB->end();
        if (lowerEnd > higherStart)
        {
            // blocks have misaligned parses, add block (could checked needsRelocation_ flag)
            bbi = NULL;
        }
        else {
            // the block was split during parsing, adjust the end and lastInsn 
            // fields of both bblInstances 
            Address blockBaseAddr = bbi->firstInsnAddr() - 
                imgB->firstInsnOffset();
            assert(baseAddr == blockBaseAddr);
            mal_printf("adjusting boundaries of split block %lx (split at %lx)\n",
                       imgB->start(), missingB.start());
            bbi->setEndAddr( imgB->endOffset() + blockBaseAddr );
            bbi->setLastInsnAddr( imgB->lastInsnOffset() + blockBaseAddr );
            // instance 2
            bblInstance *otherInst = findBlockInstanceByAddr
                            (missingB.firstInsnOffset() + blockBaseAddr);
            if (otherInst && otherInst != bbi) {
                bbi = otherInst;
                imgB = bbi->block()->llb();
                blockBaseAddr = bbi->firstInsnAddr() - 
                    imgB->firstInsnOffset();
                assert(baseAddr == blockBaseAddr);
                bbi->setEndAddr( imgB->endOffset() + blockBaseAddr );
                bbi->setLastInsnAddr(imgB->lastInsnOffset() + blockBaseAddr);
            }

            // now try and find the block again
            bblInstance *newbbi = findBlockInstanceByAddr( 
                missingB.firstInsnOffset() + blockBaseAddr );
            if (bbi == newbbi) {
                // there's real overlapping going on
                mal_printf("WARNING: overlapping blocks, major obfuscation or "
                        "bad parse [%lx %lx] [%lx %lx] %s[%d]\n",
                        bbi->firstInsnAddr(), 
                        bbi->endAddr(), 
                        baseAddr + missingB.firstInsnOffset(), 
                        baseAddr + missingB.endOffset(), 
                        FILE__,__LINE__);
            }
            bbi = newbbi;
        }
    }

    if ( ! bbi ) {
        // create new int_basicBlock and add it to our datastructures
        int_basicBlock *intBlock = new int_basicBlock
            ( &missingB, baseAddr, this, nextBlockID );
        bblInstance *bbi = intBlock->origInstance();
        assert(bbi);
        blocksByAddr_.insert(bbi);
        nextBlockID++;
        blockList.insert(intBlock);
        blockIDmap[missingB.id()] = blockIDmap.size();
    } 

    // see if the new block falls through into a function that
    // was already parsed
    Block::edgelist & edges = missingB.targets();
    SingleContext epred(ifunc(),true,true);
    Function *parsedInto=NULL;
    vector<Function*> funcs;
    missingB.getFuncs(funcs);
    for (Block::edgelist::iterator eit = edges.begin(&epred);
         !parsedInto && eit != edges.end();
         eit++)
    {
        vector<Function*> tfuncs;
        (*eit)->trg()->getFuncs(tfuncs);
        if (tfuncs.size() > funcs.size()) {
            // we fell through into another function and need to add all of
            // its blocks to our function
            vector<Function*>::iterator ait = funcs.begin();
            vector<Function*>::iterator bit = tfuncs.begin();
            for (;ait != funcs.end() && bit != tfuncs.end(); ait++,bit++) {
                if ((*ait) != (*bit)) {
                    parsedInto = *bit;
                    break;
                }
            }
            if (!parsedInto) {
                assert(bit != tfuncs.end());
                parsedInto = *bit;
            }
        }
    }
    if (parsedInto) {
        malware_cerr << "PARSED INTO SHARED FUNC" << endl;
        Function::blocklist & blocks = parsedInto->blocks();
        for (Function::blocklist::iterator bit = blocks.begin();
            bit != blocks.end(); 
            bit++)
        {
            addMissingBlock(*static_cast<image_basicBlock*>(*bit));
        }
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
    
    // iterate through whichever of blockList and img->newBlocks_ is smaller
    if (blockList.size() < ifunc_->img()->getNewBlocks().size()) {
        Function::blocklist & imgBlocks = ifunc_->blocks();
        Function::blocklist::iterator sit = imgBlocks.begin();
        for( ; sit != imgBlocks.end(); ++sit) {
            addMissingBlock( *dynamic_cast<image_basicBlock*>(*sit) );
        }
    }
    else {
        const vector<image_basicBlock*> & nblocks = 
            ifunc()->img()->getNewBlocks();
        vector<image_basicBlock*>::const_iterator nit = nblocks.begin();
        for( ; nit != nblocks.end(); ++nit) {
            mal_printf("nblock [%lx %lx)", (*nit)->start(), (*nit)->end());
            if ( ifunc()->contains( *nit ) ) {
                addMissingBlock( **nit );
                mal_printf(" was missing\n");
            } else {
                mal_printf(" not missing\n");
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

void int_function::getReachableBlocks(const set<bblInstance*> &exceptBlocks,
                                      const list<bblInstance*> &seedBlocks,
                                      set<bblInstance*> &reachBlocks)//output
{
    list<image_basicBlock*> imgSeeds;
    for (list<bblInstance*>::const_iterator sit = seedBlocks.begin();
         sit != seedBlocks.end(); 
         sit++) 
    {
        imgSeeds.push_back((*sit)->block()->llb());
    }
    set<image_basicBlock*> imgExcept;
    for (set<bblInstance*>::const_iterator eit = exceptBlocks.begin();
         eit != exceptBlocks.end(); 
         eit++) 
    {
        imgExcept.insert((*eit)->block()->llb());
    }

    // image-level function does the work
    set<image_basicBlock*> imgReach;
    ifunc()->getReachableBlocks(imgExcept,imgSeeds,imgReach);

    Address base = getAddress() - ifunc()->addr();
    for (set<image_basicBlock*>::iterator rit = imgReach.begin();
         rit != imgReach.end(); 
         rit++) 
    {
        reachBlocks.insert( findBlockInstanceByAddr(base + (*rit)->start()) );
    }
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

bblInstance *int_function::findBlockInstanceByEntry(Address addr) {
   std::map<Address, bblInstance *>::iterator iter = blocksByEntry_.find(addr);
   if (iter == blocksByEntry_.end()) return NULL;
   return iter->second;
}

int_basicBlock *int_function::findBlockByAddr(Address addr) {
    bblInstance *inst = findBlockInstanceByAddr(addr);
    if (inst)
        return inst->block();
    else {
      cerr << "Error: unable to find block with address " << hex << addr << endl;
      debugPrint();
      assert(0);
      return NULL;
    }
}


const std::set<int_basicBlock*,int_basicBlock::compare> &int_function::blocks()
{
    int i = 0;

    if (blockList.empty()) {
        // defensiveMode triggers premature block list creation when it
        // checks that the targets of control transfers have not been
        // tampered with.  
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

int_basicBlock *int_function::entryBlock() {
  blocks();

  funcEntries();
  assert(entryPoints_.size() == 1);
  return entryPoints_[0]->block();

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
    if (firstInsnAddr_ == 0x9334c7) {
            cerr << "DEBUG BREAKPOINT!" << endl;
        }

    // And add to the mapped_object code range
    block_->func()->obj()->codeRangesByAddr_.insert(this);
    block_->func()->blocksByEntry_[firstInsnAddr()] = this;
};

bblInstance::bblInstance(int_basicBlock *parent, int version) : 
    firstInsnAddr_(0),
    lastInsnAddr_(0),
    blockEndAddr_(0),
    block_(parent),
    version_(version)
{
    // And add to the mapped_object code range
    //block_->func()->obj()->codeRangesByAddr_.insert(this);

    block_->func()->blocksByEntry_[firstInsnAddr()] = this;
};

bblInstance::bblInstance(const bblInstance *parent, int_basicBlock *block) :
    firstInsnAddr_(parent->firstInsnAddr_),
    lastInsnAddr_(parent->lastInsnAddr_),
    blockEndAddr_(parent->blockEndAddr_),
    block_(block),
    version_(parent->version_) {

    // If the bblInstance is the original version, add to the mapped_object
    // code range; if it is the product of relocation, add it to the
    // process.
    if(version_ == 0)
        block_->func()->obj()->codeRangesByAddr_.insert(this);
    else
        block_->func()->obj()->proc()->addOrigRange(this);
}

bblInstance::~bblInstance() {
    mal_printf("deleting bblInstance at %lx\n", firstInsnAddr());
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



// addr can correspond to the new block or to the "this" block, unless
// neither of the versions is an origInstance, in which case addr should 
// correspond to the "this" block
Address bblInstance::equivAddr(int newVersion, Address addr) const {
   return addr;
#if 0

    Address translAddr = 0;

    if (newVersion == version()) {
        translAddr = addr;
        return translAddr;
    }

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


    if (!translAddr) {
        fprintf(stderr,"ERROR: returning 0 in equivAddr, called on bblInstance"
                " at %lx for new version %d in function at %lx %s[%d]\n", 
                firstInsnAddr_, newVersion, 
                block()->func()->getAddress(),FILE__,__LINE__);
        return 0;
    }
    return translAddr;
#endif
}

void *bblInstance::getPtrToInstruction(Address addr) const {
   if (addr < firstInsnAddr_) {
      assert(0);
      return NULL;
   }
   if (addr >= blockEndAddr_) {
      assert(0);
      return NULL;
   }

   return func()->obj()->getPtrToInstruction(addr);
}

void *bblInstance::get_local_ptr() const {
    return NULL;
}

int bblInstance::version() const 
{
   return version_;
}


// Dig down to the low-level block of b, find the low-level functions
// that share it, and map up to int-level functions and add them
// to the funcs list.
bool int_function::getSharingFuncs(int_basicBlock *b,
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

// Find overlapping functions via checking all basic blocks. We might be
// able to check only exit points; but we definitely need to check _all_
// exits so for now we're checking everything.

bool int_function::getOverlappingFuncs(std::set<int_function *> &funcs) {
    bool ret = false;

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
                fprintf(stderr, "targets:%d out_edges:%d src:0x%lx->0x%lx trg:0x%lx->0x%lx\n", (int)targets.size(), (int)out_edges.size(), (*eit)->src()->start(), (*eit)->src()->end(), (*eit)->trg()->start(), (*eit)->trg()->end());
            }

            assert(hlTarget != NULL);
            return hlTarget->instVer(version_);
        }
    }
    return NULL;
}

bblInstance * bblInstance::getFallthroughBBL() {
#if 0
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
#endif

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

#if defined(cap_instruction_api) 
void bblInstance::getInsnInstances(std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> >&instances) const {
  instances.clear();
  block()->llb()->getInsnInstances(instances);
  for (unsigned i = 0; i < instances.size(); ++i) {
    instances[i].second += firstInsnAddr_ - block()->llb()->start();
  }
}

void bblInstance::disassemble() const {
   std::vector<std::pair<InstructionAPI::Instruction::Ptr, Address> > instances;
   getInsnInstances(instances);
   for (unsigned i = 0; i < instances.size(); ++i) {
      cerr << "\t" << hex << instances[i].second << ": " << instances[i].first->format() << dec << endl;
   }
}

#endif


int_basicBlock *int_function::findBlockByImage(image_basicBlock *block) {
  return findBlockByOffset(block->start());
}


