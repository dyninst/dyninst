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
 
// $Id: image-func.C,v 1.14 2005/11/03 05:21:05 jaw Exp $

#include "function.h"
#include "instPoint.h"
#include "InstrucIter.h"
#include "symtab.h"
#include "showerror.h"

pdstring image_func::emptyString("");

int image_func_count = 0;

// Verify that this is code
// Find the call points
// Find the return address
// TODO -- use an instruction object to remove
// Sets err to false on error, true on success
//
// Note - this must define funcEntry and funcReturn
// 
image_func::image_func(const pdstring &symbol,
		       Address offset, 
		       const unsigned symTabSize,
		       pdmodule *m,
		       image *i) :
  startOffset_(offset),
  symTabSize_(symTabSize),
  mod_(m),
  image_(i),
  parsed_(false),
  noStackFrame(false),
  makesNoCalls_(false),
  savesFP_(false),
  call_points_have_been_checked(false),
  isTrap(false),
  isInstrumentable_(true),
#if defined(arch_ia64)
  framePointerCalculator(NULL),
  usedFPregs(NULL),
#endif
  canBeRelocated_(true),
  needsRelocation_(false),
  originalCode(NULL),
  o7_live(false)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_func_count++;
    if ((image_func_count % 10) == 0)
        fprintf(stderr, "image_func_count: %d (%d)\n",
                image_func_count, image_func_count*sizeof(image_func));
#endif
#if defined(cap_stripped_binaries)
    endOffset_ = startOffset_;
#else
    endOffset_ = startOffset_ + symTabSize_;
#endif
    symTabName_.push_back(symbol);
}


image_func::~image_func() { 
  /* TODO */ 
}

void image_func::changeModule(pdmodule *mod) {
  // Called from buildFunctionLists, so we aren't entered in any 
  // module-level data structures. If this changes, UPDATE THE
  // FUNCTION.
  mod_ = mod;
}

bool image_func::isInstrumentableByFunctionName()
{
#if defined(i386_unknown_solaris2_5)
    /* On Solaris, this function is called when a signal handler
       returns.  If it requires trap-based instrumentation, it can foul
       the handler return mechanism.  So, better exclude it.  */
    if (prettyName() == "_setcontext" || prettyName() == "setcontext")
        return false;
#endif /* i386_unknown_solaris2_5 */
    
    if( prettyName() == "__libc_free" )
        return false;
    
    // XXXXX kludge: these functions are called by DYNINSTgetCPUtime, 
    // they can't be instrumented or we would have an infinite loop
    if (prettyName() == "gethrvtime" || prettyName() == "_divdi3"
        || prettyName() == "GetProcessTimes")
        return false;
    return true;
}

Address image_func::getEndOffset() {
    if (!parsed_) image_->analyzeIfNeeded();
    return endOffset_;
}


const pdvector<image_instPoint *> &image_func::funcEntries() {
  if (!parsed_) image_->analyzeIfNeeded();
  return funcEntries_;
}

const pdvector<image_instPoint*> &image_func::funcExits() {
  if (!parsed_) image_->analyzeIfNeeded();

  return funcReturns;
}

const pdvector<image_instPoint*> &image_func::funcCalls() {
  if (!parsed_) image_->analyzeIfNeeded();

  return calls;
}

int image_basicBlock_count = 0;

image_basicBlock::image_basicBlock(image_func *func, Address firstOffset) :
    firstInsnOffset_(firstOffset),
    lastInsnOffset_(0),
    blockEndOffset_(0),
    isEntryBlock_(false),
    isExitBlock_(false),
    blockNumber_(-1),
    func_(func) {
#if defined(ROUGH_MEMORY_PROFILE)
    image_basicBlock_count++;
    if ((image_basicBlock_count % 10) == 0)
        fprintf(stderr, "image_basicBlock_count: %d (%d)\n",
                image_basicBlock_count, image_basicBlock_count*sizeof(image_basicBlock));
#endif
}


void image_basicBlock::addSource(image_basicBlock *source) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == source)
            return;
    sources_.push_back(source);
}

void image_basicBlock::addTarget(image_basicBlock *target) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == target)
            return;
    targets_.push_back(target);
}

void image_basicBlock::removeSource(image_basicBlock *source) {
    for (unsigned i = 0; i < sources_.size(); i++)
        if (sources_[i] == source) {
            sources_[i] = sources_.back();
            sources_.resize(sources_.size()-1);
            return;
        }
}

void image_basicBlock::removeTarget(image_basicBlock *target) {
    for (unsigned i = 0; i < targets_.size(); i++)
        if (targets_[i] == target) {
            targets_[i] = targets_.back();
            targets_.resize(targets_.size()-1);
            return;
        }
}

void image_basicBlock::getSources(pdvector<image_basicBlock *> &ins) const {
    for (unsigned i = 0; i < sources_.size(); i++)
        ins.push_back(sources_[i]);
}
// Need to be able to get a copy
void image_basicBlock::getTargets(pdvector<image_basicBlock *> &outs) const {
    for (unsigned i = 0; i < targets_.size(); i++)
        outs.push_back(targets_[i]);
}

int image_instPoint_count = 0;

image_instPoint::image_instPoint(Address offset,
                                 instruction insn,
                                 image_func *func,
                                 instPointType_t type) :
    instPointBase(insn, type),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(0),
    isDynamicCall_(0)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_instPoint_count++;
    if ((image_instPoint_count % 10) == 0)
        fprintf(stderr, "image_instPoint_count: %d (%d)\n",
                image_instPoint_count,
                image_instPoint_count*sizeof(image_instPoint));
#endif
}

image_instPoint::image_instPoint(Address offset,
                                 instruction insn,
                                 image_func *func,
                                 Address callTarget,
                                 bool isDynamicCall) :
    instPointBase(insn, callSite),
    offset_(offset),
    func_(func),
    callee_(NULL),
    callTarget_(callTarget),
    isDynamicCall_(isDynamicCall)
{
#if defined(ROUGH_MEMORY_PROFILE)
    image_instPoint_count++;
    if ((image_instPoint_count % 10) == 0)
        fprintf(stderr, "image_instPoint_count: %d (%d)\n",
                image_instPoint_count, image_instPoint_count * sizeof(image_instPoint));
#endif
    if (isDynamicCall_)
        assert(callTarget_ == 0);
    else
        assert(!isDynamicCall_);
}

bool image_func::addBasicBlock(Address newAddr,
                               image_basicBlock *oldBlock,
                               BPatch_Set<Address> &leaders,
                               dictionary_hash<Address, image_basicBlock *> &leadersToBlock,
                               pdvector<Address> &jmpTargets) {
    // Doublecheck 
    if (!image_->isCode(newAddr))
        return false;

    // Add to jump targets = local parse points
    jmpTargets.push_back(newAddr);

    // Make a new basic block for this target
    if (!leaders.contains(newAddr)) {
        leadersToBlock[newAddr] = new image_basicBlock(this, newAddr);
        leaders += newAddr;
        blockList.push_back(leadersToBlock[newAddr]);
    }
    // In any case, add source<->target mapping
    assert(leadersToBlock[newAddr]);
    leadersToBlock[newAddr]->addSource(oldBlock);
    oldBlock->addTarget(leadersToBlock[newAddr]);

    return true;
}

void image_basicBlock::debugPrint() {
    // 64-bit
    parsing_printf("Block %d: starts 0x%lx (%d), last 0x%lx (%d), end 0x%lx (%d)\n",
                   blockNumber_,
                   firstInsnOffset_,
                   firstInsnOffset_ - func_->getOffset(),
                   lastInsnOffset_,
                   lastInsnOffset_ - func_->getOffset(),
                   blockEndOffset_,
                   blockEndOffset_ - func_->getOffset());
    parsing_printf("  Flags: entry %d, exit %d\n",
                   isEntryBlock_, isExitBlock_);
    parsing_printf("  Sources:\n");
    for (unsigned s = 0; s < sources_.size(); s++) {
        parsing_printf("    %d: block %d\n",
                       s, sources_[s]->blockNumber_);
    }
    parsing_printf("  Targets:\n");
    for (unsigned t = 0; t < targets_.size(); t++) {
        parsing_printf("    %d: block %d\n",
                       t, targets_[t]->blockNumber_);
    }
}

// Make sure no blocks overlap, sort stuff by address... you know,
// basic stuff.

bool image_func::cleanBlockList() {
    unsigned i;

    // For all entry, exit, call points...
    //points_[u].point->checkInstructions();
    // Should also make multipoint decisions

#if !defined(arch_x86) && !defined(arch_power) && !defined(arch_x86_64)
    // We need to make sure all the blocks are inside the function
    pdvector<image_basicBlock *>cleanedList;
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        if ((blockList[foo]->firstInsnOffset() < getOffset()) ||
            (blockList[foo]->firstInsnOffset() >= getEndOffset())) {
            inst_printf("Block %d at 0x%lx is outside of function (0x%lx to 0x%lx)\n",
                        foo,
                        blockList[foo]->firstInsnOffset(),
                        getOffset(),
                        getEndOffset());
                        
            delete blockList[foo];
        }
        else
            cleanedList.push_back(blockList[foo]);
    }
    blockList.clear();
    for (unsigned bar = 0; bar < cleanedList.size(); bar++)
        blockList.push_back(cleanedList[bar]);


#endif

   //sorted_ips_vector expects funcReturns and calls to be sorted
    VECTOR_SORT( funcReturns, image_instPoint::compare);
    VECTOR_SORT( calls, image_instPoint::compare);

    //check if basic blocks need to be split   
    VECTOR_SORT( blockList, image_basicBlock::compare );
    //parsing_printf("INITIAL BLOCK LIST\n");
    //maybe image_flowGraph.C would be a better home for this bit of code?
    for( unsigned int iii = 0; iii < blockList.size(); iii++ )
    {
        blockList[iii]->blockNumber_ = iii;
        blockList[iii]->debugPrint();        
    }
  
    for( unsigned int r = 0; r + 1 < blockList.size(); r++ )
    {
        image_basicBlock* b1 = blockList[ r ];
        image_basicBlock* b2 = blockList[ r + 1 ];
        
        if( b2->firstInsnOffset() < b1->endOffset() )
        {
            //parsing_printf("Blocks %d and %d overlap...\n",
            //b1->blockNumber_,
            //b2->blockNumber_);
            pdvector< image_basicBlock* > out;
            b1->getTargets( out );
            
            for( unsigned j = 0; j < out.size(); j++ )
            {
                out[j]->removeSource( b1 );
                out[j]->addSource( b2 );
            }        
          
            //set end address of higher block
            b2->lastInsnOffset_ =  b1->lastInsnOffset();
            b2->blockEndOffset_ =  b1->endOffset();
            b2->targets_ = b1->targets_;	    
            b2->addSource( b1 );
            
            b1->targets_.clear();
            b1->targets_.push_back(b2);
            
            //find the end of the split block	       
            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();

            if( b1->isExitBlock_ )
            {
                b1->isExitBlock_ = false;
                b2->isExitBlock_ = true;
            }
        }
    }
    for( unsigned q = 0; q + 1 < blockList.size(); q++ )
    {
        image_basicBlock* b1 = blockList[ q ];
        image_basicBlock* b2 = blockList[ q + 1 ];
        
        if( b1->endOffset() == 0 )
        {
            ///parsing_printf("Block %d has zero size; expanding to block %d\n",
            //b1->blockNumber_,
            //b2->blockNumber_);

            //find the end of this block.

            // Make the iterator happy; we can set the end offset to
            // the start of b2. It will be that or smaller.
            b1->blockEndOffset_ = b2->firstInsnOffset();

            InstrucIter ah( b1 );
            while( *ah + ah.getInstruction().size() < b2->firstInsnOffset() )
                ah++;
            
            b1->lastInsnOffset_ = *ah;
            b1->blockEndOffset_ = *ah + ah.getInstruction().size();
            b1->addTarget( b2 );	  
            b2->addSource( b1 );	            
        }        
    }    
    for (i = 0; i < blockList.size(); i++) {
        // Check sources and targets for legality
        image_basicBlock *b1 = blockList[i];
        for (unsigned s = 0; s < b1->sources_.size(); s++) {
            if (((unsigned)b1->sources_[s]->id() >= blockList.size()) ||
                (b1->sources_[s]->id() < 0)) {
                fprintf(stderr, "WARNING: block %d in function %s has illegal source block %d\n",
                        b1->id(), symTabName().c_str(), b1->sources_[s]->id());
                b1->removeSource(b1->sources_[s]);
            }
        }
#if defined(cap_relocation)
        // Don't do multiple-exit-edge blocks; 1 or 2 is cool, > is bad
        if (b1->targets_.size() > 2) {
            canBeRelocated_ = false;
        }
#endif
        for (unsigned t = 0; t < b1->targets_.size(); t++) {
            if (((unsigned)b1->targets_[t]->id() >= blockList.size()) ||
                (b1->targets_[t]->id() < 0)) {
                fprintf(stderr, "WARNING: block %d in function %s has illegal target block %d\n",
                        b1->id(), symTabName().c_str(), b1->targets_[t]->id());
                b1->removeTarget(b1->targets_[t]);
            }
        }

    }

    parsing_printf("CLEANED BLOCK LIST\n");
    for (unsigned foo = 0; foo < blockList.size(); foo++) {
        // Safety check; we need the blocks to be sorted by addr
        blockList[foo]->debugPrint();

        // Safety checks.

        // I've disabled this one but left it in so that we don't add
        // it in the future.. Since we're doing offsets, zero is
        // _fine_.
        //assert(blockList[foo]->firstInsnOffset() != 0);
        assert(blockList[foo]->lastInsnOffset() != 0);
        assert(blockList[foo]->endOffset() != 0);

        /* Serious safety checks. These can tag things that are
           legal. Enable if you're trying to track down a parsing
           problem. */
#if !defined(arch_x86) && !defined(arch_x86_64)
        if (foo > 0) {
            assert(blockList[foo]->firstInsnOffset() >= blockList[foo-1]->endOffset());
        }
        assert(blockList[foo]->endOffset() >= blockList[foo]->firstInsnOffset());
#endif

        blockList[foo]->finalize();
    }
    funcEntries_.reserve_exact(funcEntries_.size());
    funcReturns.reserve_exact(funcReturns.size());
    calls.reserve_exact(calls.size());
    return true;
}

// Bind static call points

void image_func::checkCallPoints() {
    // Check if there are any dangling calls
    for (unsigned c = 0; c < calls.size(); c++) {
        image_instPoint *p = calls[c];
        assert(p);
        
        if (p->getCallee() != NULL)
            continue;
        
        Address destOffset = p->callTarget();
        if (!destOffset) {
            // Couldn't determine contact; skip
            continue;
        }
        
        image_func *pdf = image_->findFuncByOffset(destOffset);
        
        if (pdf) {
            p->setCallee(pdf);
        }
    }
}


#if defined(cap_stripped_binaries)

//correct parsing errors that overestimate the function's size by
// 1. updating all the vectors of instPoints
// 2. updating the vector of basicBlocks
// 3. updating the function size
// 4. update the address of the last basic block if necessary
void image_func::updateFunctionEnd(Address newEnd)
{

    //update the size
    endOffset_ = newEnd;
    //remove out of bounds call Points
    //assumes that calls was sorted by address in findInstPoints
    for( int i = (int)calls.size() - 1; i >= 0; i-- )
        {
            if( calls[ i ]->offset() >= newEnd )
                {
                    delete calls[ i ];
                    calls.pop_back();
                }
            else 
                break;
        }
    //remove out of bounds return points
    //assumes that funcReturns was sorted by address in findInstPoints
    for( int j = (int)funcReturns.size() - 1; j >= 0; j-- )
        {
            if( funcReturns[ j ]->offset() >= newEnd )
                {
                    delete funcReturns[ j ];
                    funcReturns.pop_back();
                }
            else
                break;
        }

    //remove out of bounds basicBlocks
    //assumes blockList was sorted by start address in findInstPoints
    for( int k = (int) blockList.size() - 1; k >= 0; k-- )
        {
            image_basicBlock* curr = blockList[ k ];
            if( curr->firstInsnOffset() >= newEnd )
                {
                    //remove all references to this block from the flowgraph
                    //my source blocks should no longer have me as a target
                    pdvector< image_basicBlock* > ins;
                    curr->getSources( ins );
                    for( unsigned o = 0; o < ins.size(); o++ )
                        {
                            ins[ o ]->removeTarget(curr);
                            ins[ o ]->isExitBlock_ = true;
                        } 
                    
                    //my target blocks should no longer have me as a source 
                    pdvector< image_basicBlock* > outs;
                    curr->getTargets( outs );
                    for( unsigned p = 0; p < outs.size(); p++ )
                        {
                            outs[ p ]->removeSource(curr);
                        }                     
                    delete curr;
                    blockList.pop_back();          
                }
            else
                break;
        } 
    
    //we might need to correct the end address of the last basic block
    int n = blockList.size() - 1;
    if( n >= 0 && blockList[n]->endOffset() >= newEnd )
        {
            parsing_printf("Updating block end: new end of function 0x%x\n",
                           newEnd);
            blockList[n]->debugPrint();
            
            // TODO: Ask Laune; this doesn't look like it's doing what we want.
            image_basicBlock* blk = blockList[n];
            
            InstrucIter ah(blk);	
            while( ah.peekNext() < newEnd )
                ah++;
            
            blk->lastInsnOffset_ = *ah ;
            blk->blockEndOffset_ = ah.peekNext();
            
            if (!blk->isExitBlock_) {
                blk->isExitBlock_ = true;
                // Make a new exit point
                image_instPoint *p = new image_instPoint(*ah,
                                                         ah.getInstruction(),
                                                         this,
                                                         functionExit);
                funcReturns.push_back(p);
            }
            
            parsing_printf("After fixup:\n");
            blk->debugPrint();
        }

    funcReturns.reserve_exact(funcReturns.size());
    calls.reserve_exact(calls.size());
    
    // And rerun memory trimming
    for (unsigned foo = 0; foo < blockList.size(); foo++)
        blockList[foo]->finalize();
    
}    

#endif


void *image_basicBlock::getPtrToInstruction(Address addr) const {
    if (addr < firstInsnOffset_) return NULL;
    if (addr >= blockEndOffset_) return NULL;
    return func()->img()->getPtrToInstruction(addr);
}

void *image_func::getPtrToInstruction(Address addr) const {
    if (addr < startOffset_) return NULL;
    if (!parsed_) image_->analyzeIfNeeded();
    if (addr >= endOffset_) return NULL;
    for (unsigned i = 0; i < blockList.size(); i++) {
        void *ptr = blockList[i]->getPtrToInstruction(addr);
        if (ptr) return ptr;
    }
    return NULL;
}

void image_basicBlock::finalize() {
    targets_.reserve_exact(targets_.size());
    sources_.reserve_exact(sources_.size());
}
