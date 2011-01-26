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

// Code to install and remove instrumentation from a running process.

#include "multiTramp.h"
#include "baseTramp.h"
#include "miniTramp.h"
#include "instPoint.h"
#include "function.h"
#include "addressSpace.h"
#include "debug.h"
#include "pcProcess.h"
#if defined(cap_instruction_api)
#include "instructionAPI/h/InstructionDecoder.h"
#else
#include "parseAPI/src/InstrucIter.h"
#endif // defined(cap_instruction_api)
#include "BPatch.h"
// Need codebuf_t, Address
#include "codegen.h"
#include <sstream>

using namespace Dyninst;

unsigned int multiTramp::id_ctr = 1;

multiTramp *multiTramp::getMulti(int id, AddressSpace *p) {
    return p->findMultiTrampById(id);
}

baseTrampInstance *multiTramp::getBaseTrampInstance(instPointInstance *point,
                                                    callWhen when) const {
    // How do we instrument this point at the desired address

    // And a safety note;
    assert(point->multi() == this);

    generatedCodeObject *insn = insns_[point->addr()];
    assert(insn);

    switch (when) {
    case callPreInsn: {
        //inst_printf("Matching preBTI\n");
        baseTrampInstance *preBTI = dynamic_cast<baseTrampInstance *>(insn->previous_);
        if (preBTI) return preBTI;
        break;
    }
    case callPostInsn: {
        //inst_printf("Matching postBTI\n");
        baseTrampInstance *postBTI = dynamic_cast<baseTrampInstance *>(insn->fallthrough_);
        if (postBTI) return postBTI;
        break;
    }
    case callBranchTargetInsn: {
        //inst_printf("Matching targetBTI\n");
        baseTrampInstance *BTI = dynamic_cast<baseTrampInstance *>(insn->target_);
        if (BTI) return BTI;
        break;                
    }
    default:
        assert(0);
        break;
    }
    // Didn't find a match.
    return NULL;
}

// Reverse mapping: I have an address, and I want a BTI
baseTrampInstance *multiTramp::getBaseTrampInstanceByAddr(Address addr) const {
    // Note: this is a slight modification of instToUninst, but has a
    // different target.

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        baseTrampInstance *i = dynamic_cast<baseTrampInstance *>(obj);
        if (i && i->isInInstance(addr))
            return i;
    }

    return NULL;
}

// Begin the long and arduous task of deleting a multiTramp.

void multiTramp::removeCode(generatedCodeObject *subObject) {
    // One of our baseTramps just went away; let's see what's going on.
    // Or we're going away from a top level.
    baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(subObject);
    relocatedInstruction *reloc = dynamic_cast<relocatedInstruction *>(subObject);
    trampEnd *te = dynamic_cast<trampEnd *>(subObject);
    replacedInstruction *ri = dynamic_cast<replacedInstruction *>(subObject);

    // Have _no_ idea how to handle one of these guys going away.
    assert(!reloc);
    assert(!te);
    assert(trampEnd_);

    // Un-replace an instruction? Not right now...
    // TODO
    assert(!ri);

    bool doWeDelete = false;

    if (subObject == NULL) doWeDelete = true;
    
    if (bti) {
        // We in-line baseTramps, which means if a baseTrampInstance goes away,
        // we need to regenerate it in toto. 

        // Go through the generatedCFG, get rid of this instance, and see if there's
        // still anyone left.

        bool stillInst = false;
        bool foundBTI = false;

        generatedCFG_t::iterator cfgIter(generatedCFG_);
        generatedCodeObject *obj = NULL;

        while ((obj = cfgIter++)) {
            baseTrampInstance *tmp = dynamic_cast<baseTrampInstance *>(obj);
            if (!tmp) {
                continue;
            }
            // We loop through everything on purpose to determine whether
            // there are other instrumented points.
            if (tmp == bti) {
                if (tmp->previous_ && 
                    (tmp->previous_->fallthrough_ == tmp)) {
                    tmp->previous_->setFallthrough(tmp->fallthrough_);
                }
                else if (tmp->previous_ && 
                    (tmp->previous_->target_ == tmp)) {
                    // Edge instrumentation...
                    tmp->previous_->setTarget(tmp->fallthrough_);
                }
                else {
                    // Assert we're the first thing in line.
                    assert(tmp == generatedCFG_.start());
                    generatedCFG_.setStart(tmp->fallthrough_);
                }
                if (tmp->fallthrough_) {
                    tmp->fallthrough_->setPrevious(tmp->previous_);
                }
                foundBTI = true;
                tmp->removeCode(this);
            }
            else {
                if (!tmp->isEmpty())
                    stillInst = true;
            }
            if (stillInst && foundBTI)
                break;
        }
                
        if (stillInst) {
            // There's a baseTrampInstance left. Since we in-lined, this means
            // we need to regenerate the code. For safety, we make a new multiTramp
            // in place of this one.
            multiTramp::replaceMultiTramp(this, doWeDelete);
        }
        else {
            doWeDelete = true;
        }
        
        int found_index = -1;
        for (unsigned int i = 0; i <deletedObjs.size(); ++i) {
           if (subObject == deletedObjs[i]) {
              found_index = i;
              break;
           }
        }
        if (-1 == found_index) 
           deletedObjs.push_back(bti);
    }
    
    if ( doWeDelete && isActive_ && ! partlyGone_ ) {
        mal_printf("Unlinking but not deleting multiTramp %lx [%lx %lx] on "
                "grounds that it is actively executing %s[%d]\n",instAddr_,
                trampAddr_, trampAddr_+trampSize_, FILE__,__LINE__);
    }

    // unlink the tramp if we haven't already
    if ( doWeDelete && ! partlyGone_ ) {
        disable();
        partlyGone_ = true;
    }

    if (doWeDelete && !isActive_) {

        if (proc()->findMultiTrampById(id()) == this) {
            // Won't leak as the process knows to delete us
            proc()->removeMultiTramp(this);
        }
        
        // Move everything else to the deleted list
        generatedCFG_t::iterator cfgIter(generatedCFG_);
        generatedCodeObject *obj = NULL;

        while ((obj = cfgIter++)) {
           int found_index = -1;
           for (unsigned int i = 0; i <deletedObjs.size(); ++i) {
              if (obj == deletedObjs[i]) {
                 found_index = i;
                 break;
              }
           }
           if (-1 == found_index)  {
              deletedObjs.push_back(obj);
              obj->removeCode(this);
           }
        }
        generatedCFG_.setStart(NULL);

        proc()->deleteGeneratedCode(this);
    }   
    assert(trampEnd_);
    return;
}

   bool multiTramp::safeToFree(codeRange *range) {
      if (dynamic_cast<multiTramp *>(range) == this)
         return false;

      generatedCFG_t::iterator cfgIter(generatedCFG_);
      generatedCodeObject *obj = NULL;

    while ((obj = cfgIter++)) {
        if (!obj->safeToFree(range))
            return false;
    }
    return true;
}


void multiTramp::freeCode() {
    proc()->inferiorFree(trampAddr_);
    delete this;
}

multiTramp::~multiTramp() {

    mal_printf("Deleting multi instAddr 0x%lx trampAddr 0x%lx\n",
               instAddr_,trampAddr_);

    for (unsigned i = 0; i < deletedObjs.size(); i++)
       if(deletedObjs[i] != NULL) {
          delete deletedObjs[i];          
       }
    deletedObjs.clear();

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;

    while ((obj = cfgIter++)) {
        // Before we delete the object :)
        if(obj != NULL)
           delete obj;
    }

    if (savedCodeBuf_)
        free(savedCodeBuf_);

    // And this is why we want a process pointer ourselves. Trusting the
    // function to still be around is... iffy... at best.
    proc()->removeMultiTramp(this);
    proc()->inferiorFree(trampAddr_);

    // Everything else is statically allocated
}

// Assumes there is no matching multiTramp at this address
int multiTramp::findOrCreateMultiTramp(Address pointAddr, 
                                       bblInstance *bbl) {
    AddressSpace *proc = bbl->func()->proc();
    multiTramp *newMulti = proc->findMultiTrampByAddr(pointAddr);
    if (newMulti && newMulti->func() == bbl->func()) { 
        
        // Check whether we're in trap shape
        // Sticks to false if it ever is false
        return newMulti->id();
    }

    Address startAddr = 0;
    unsigned size = 0;
    bool basicBlockTramp = false;

    // On most platforms we instrument an entire basic block at a
    // time. IA64 does bundles. This is controlled by the static
    // getMultiTrampFootprint function; so if you want to do something
    // odd then go poke there. Individual instrumentation _should_
    // work but is not tested.

    if (!multiTramp::getMultiTrampFootprint(pointAddr,
                                            proc,
                                            startAddr,
                                            size,
					    basicBlockTramp)) {
        // Assert fail?
        inst_printf("Could not get multiTramp footprint at 0x%lx, ret false\n", pointAddr);
        return 0;
    }

    // Cannot make a point here if someone else got there first
	// We're allowing this for now; we'll go through the motions, and finally
	// "fake" the actual link phase. Otherwise things get weirdly out of
	// step.

    newMulti = new multiTramp(startAddr,
                              size,
                              bbl->func());
    
    // Iterate over the covered instructions and pull each one
    relocatedInstruction *prev = NULL;

    // There are two ways of handling this... if we're in original code,
    // run an instructIter over the thing. If not, things get tricky...
    // We're in a relocated function, but want to use the original instruction
    // representations (with new targets and sizes. Oy.)

    bool done = false;
#if defined(cap_relocation)
    if (bbl->version() > 0) {
        done = true;
      // Relocated!
      
      // We assert that we're going over the entire block. This is okay, as the only
      // platform where we wouldn't is IA-64, which doesn't do function relocation.

      // uhh....
      pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &relocInsns = bbl->get_relocs();
      assert(relocInsns[0]->relocAddr == startAddr);
      for (unsigned i = 0; i < relocInsns.size(); i++) {
	relocatedInstruction *reloc = new relocatedInstruction(relocInsns[i]->origInsn,
							       // Original address...
							       relocInsns[i]->origAddr,
							       // Current address...
							       relocInsns[i]->relocAddr,
							       // Target, if set
							       relocInsns[i]->relocTarget,
							       newMulti);
        newMulti->insns_[relocInsns[i]->relocAddr] = reloc;

        if (prev) {
	  prev->setFallthrough(reloc);
        }
        else {
	  // Other initialization?
	  newMulti->setFirstInsn(reloc);
        }
        reloc->setPrevious(prev);
        prev = reloc;
      }
    }
#endif
    if (!done) {

#if defined(cap_instruction_api)
      size_t offset = 0;
      using namespace Dyninst::InstructionAPI;
      const unsigned char* relocatedInsnBuffer =
      reinterpret_cast<const unsigned char*>(proc->getPtrToInstruction(startAddr));
      
      InstructionDecoder decoder(relocatedInsnBuffer, size, proc->getArch());
      while(offset < size)
      {
	Address insnAddr = startAddr + offset;

	relocatedInstruction *reloc = new relocatedInstruction(relocatedInsnBuffer + offset, insnAddr, insnAddr, 0, newMulti);
	newMulti->insns_[insnAddr] = reloc;
	if (prev) {
	  prev->setFallthrough(reloc);
	}
	else {
	  // Other initialization?
	  newMulti->setFirstInsn(reloc);
	}
	reloc->setPrevious(prev);
	prev = reloc;	
	offset += decoder.decode()->size();
	#if defined(arch_sparc)
	#error "Instruction API not implemented for SPARC yet"
	// delay slot handling goes here
	#endif
      }
      
#else
      for (InstrucIter insnIter(startAddr, size, proc);
	   insnIter.hasMore(); 
	   insnIter++) {
        instruction *insn = insnIter.getInsnPtr();
        Address insnAddr = *insnIter;
	
        relocatedInstruction *reloc = new relocatedInstruction(insn, 
							       insnAddr,
							       insnAddr, 
							       0,
							       newMulti);
        newMulti->insns_[insnAddr] = reloc;
        
        if (prev) {
	  prev->setFallthrough(reloc);
        }
        else {
	  // Other initialization?
	  newMulti->setFirstInsn(reloc);
        }
        reloc->setPrevious(prev);
        prev = reloc;
        
        // SPARC!
#if defined(arch_sparc)
        // Just to avoid cluttering this up
        // If we have a delay slot, grab the next insn and aggregate,
        // if it exists. These functions advance the iterator; effectively
        // we're gluing delay slot and jump together.
        
        if (insn->isDCTI()) {
	  instruction *ds, *agg;
	  insnIter.getAndSkipDSandAgg(ds, agg);

	  if (ds) {
	    reloc->ds_insn = ds;
	  }
	  if (agg) {
	    reloc->agg_insn = agg;
	  }
        }
#endif // defined(arch_sparc)
      } 
#endif // defined(cap_instruction_api)  
    }

    assert(prev);

    // Add a trampEnd object for fallthroughs
    trampEnd *end = NULL;
    int_basicBlock *fallthroughBlock = NULL;

    if (basicBlockTramp) {
      fallthroughBlock = bbl->block()->getFallthrough();
    }

    if (fallthroughBlock) {
        bblInstance *fallthroughInstance = fallthroughBlock->instVer(bbl->version());
        // We really need one of these...
        assert(fallthroughInstance);
        end = new trampEnd(newMulti, fallthroughInstance->firstInsnAddr());
    }
    else {
        // No fallthrough... 
        // Could be a real case of no fallthrough, or we could be instrumenting
        // an instruction at a time due to an indirect jump. Or we could be
        // on IA-64.

        // In an alternate case of indirect jump - function return. In this case,
        // we'll add a branch that is never taken (but appears to go to the next 
        // function, confusing).
        
        end = new trampEnd(newMulti, startAddr + size);
    }
    
    assert(end);
    prev->setFallthrough(end);
    end->setPrevious(prev);
    
    newMulti->trampEnd_ = end;

    // Put this off until we generate
    //newMulti->updateInstInstances();

    proc->addMultiTramp(newMulti);

    return newMulti->id();
}

// Get the footprint for the multiTramp at this address. The MT may
// not exist yet; our instrumentation code needs this.
bool multiTramp::getMultiTrampFootprint(Address instAddr,
                                        AddressSpace *proc,
                                        Address &startAddr,
                                        unsigned &size,
					bool &basicBlock)
{
    // We use basic blocks
    // Otherwise, make one.
    codeRange *range = proc->findOrigByAddr(instAddr);
    if (!range) {
      inst_printf("%s[%d]: no code range for given address 0x%lx!\n", FILE__, __LINE__, instAddr);
      return false;
    }
    bblInstance *bbl = range->is_basicBlockInstance();
    if (!bbl) {
        inst_printf("%s[%d]: No basic block instance for addr 0x%lx in createMultiTramp, ret NULL\n",
		    FILE__, __LINE__, instAddr);
        return false;
    }

    // check whether this block is legal to relocate (instrumentation requires
    // relocation of the basic block)
    if(!bbl->block()->llb()->canBeRelocated())
    {
      inst_printf("%s[%d]: Basic block at 0x%lx cannot be instrumented, ret NULL\n",
		  FILE__, __LINE__, instAddr);
        return false;
    }

    // If the function contains unresolved indirect branches, we have to
    // assume that the branch could go anywhere (e.g., it could split
    // this block). So we don't get to use the entire block size, just
    // the instruction size.
    if(bbl->func()->ifunc()->instLevel() == HAS_BR_INDIR)
    {
        inst_printf("Target function contains unresolved indirect branches\n"
                    "   Setting multiTramp size to instruction size\n");

        
        startAddr = instAddr;
#if defined(cap_instruction_api)
	using namespace Dyninst::InstructionAPI;
	InstructionDecoder decoder((unsigned char*)(proc->getPtrToInstruction(instAddr)),
				   InstructionDecoder::maxInstructionLength,
				   proc->getArch());
        Instruction::Ptr instInsn = decoder.decode();
	size = instInsn->size();
#else
        InstrucIter ah(instAddr,proc);
        size = ah.getInstruction().size();
#endif // defined(cap_instruction_api)
        basicBlock = false;
        return true;
    }
    
    // start is the start of the basic block, size is the size
    startAddr = bbl->firstInsnAddr();
    size = (unsigned) bbl->getSize();
    basicBlock = true;

    return true;
}

void multiTramp::updateInstInstances() {
    // Go over all the instructions in our little CFG-let and insert baseTrampInstances
    // et. al.

    assert(func());

    generatedCFG_t::iterator cfgIter(generatedCFG_);

    // We need to regenerate if anything changes; that defined as a baseTrampInstance
    // appearing or disappearing.


    generatedCodeObject *obj = NULL;
    generatedCodeObject *prev = NULL;

    while ((obj = cfgIter++)) {
        // If we're a relocInsn, see if there's a new pre or post
        // tramp. If so, stick it in line.
        // If we've been replaced (replacedInstruction), then swap the
        // relocInsn for the replacedInstruction

        // If we're the last thing in a chain (of which there may be many),
        // add a trampEnd if there isn't already one.
        if (!obj->fallthrough_) {
            trampEnd *end = dynamic_cast<trampEnd *>(obj);

	    // Should have been added in initial generation.
	    assert(end);
        }
        
        relocatedCode *c = dynamic_cast<relocatedCode *>(obj);
        if (!c) {
            prev = obj;
            continue;
        }
            
        const relocatedInstruction *insn = c->relocInsn();
        assert(insn);

        Address insnAddr = insn->fromAddr_;
        instPoint *instP = func()->findInstPByAddr(insnAddr);
        if (!instP) {
            // There's no instPoint for here, so there's (by definition)
            // no instrumentation/replacement
            prev = obj;
            continue;
        }            

        baseTramp *preBT = instP->preBaseTramp();
        
        if (preBT) {
            baseTrampInstance *preBTI = preBT->findOrCreateInstance(this);
            assert(preBTI);
            // Now insert this guy in line. If it's already there, cool; 
            // otherwise set the regen flag.
            if (prev) {
                if (prev != preBTI) {
                    prev->setFallthrough(preBTI);
                    preBTI->setFallthrough(obj);
                    preBTI->setPrevious(prev);
                    obj->setPrevious(preBTI);
                    changedSinceLastGeneration_ = true;
                }
                else {
                    assert(prev->fallthrough_ == obj);
                }
            }
            else {
                // Previous is NULL: we're the first insn. Bump the CFG
                // code and set next
                generatedCFG_.setStart(preBTI);
                preBTI->setFallthrough(obj);
                obj->setPrevious(preBTI);
                changedSinceLastGeneration_ = true;
            }
        }

        baseTramp *postBT = instP->postBaseTramp();

        if (postBT) {
            baseTrampInstance *postBTI = postBT->findOrCreateInstance(this);
            assert(postBTI);

            // Append to us.
            if (obj->fallthrough_ != postBTI) {
                postBTI->setFallthrough(obj->fallthrough_);
                obj->fallthrough_->setPrevious(postBTI);
                obj->setFallthrough(postBTI);
                postBTI->setPrevious(obj);

                changedSinceLastGeneration_ = true;
            }
        }

        baseTramp *targetBT = instP->targetBaseTramp();

        if (targetBT) {
            baseTrampInstance *targetBTI = targetBT->findOrCreateInstance(this);
            assert(targetBTI);
            // Set target
            if (obj->target_) {
                assert(obj->target_ == targetBTI);
            }
            else {
                targetBTI->setPrevious(obj);
                obj->setTarget(targetBTI);
                // And a tramp end marker goes here as well.
                Address origTarget = insn->originalTarget();
                targetBTI->setFallthrough(new trampEnd(this, origTarget));
                targetBTI->fallthrough_->setPrevious(targetBTI);
                changedSinceLastGeneration_ = true;
            }
        }

        // See if we've been replaced....
        AstNodePtr replacedAST = instP->replacedCode_;
        replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);
        if (ri) assert(replacedAST != AstNodePtr()); // We don't un-replace yet...
        
        if ((replacedAST != AstNodePtr()) && (ri == NULL)) {
            // We've been asked to replace the current instruction...
            replacedInstruction *newRI = new replacedInstruction(insn, 
                                                                 replacedAST,
                                                                 instP,
                                                                 this);
            assert(newRI);

            if (obj->previous_ == NULL) {
               // We were the first thing in the CFG, so 
               // replace that with newRI
               generatedCFG_.setStart(newRI);
            }

            // And now swap into line...
            newRI->setPrevious(obj->previous_);
            newRI->setFallthrough(obj->fallthrough_);
            newRI->setTarget(obj->target_);

            // We can't be the target - only base tramps can be a target...
            if (newRI->previous_) 
                newRI->previous_->setFallthrough(newRI);
            if (newRI->fallthrough_)
                newRI->fallthrough_->setPrevious(newRI);
            if (newRI->target_)
                newRI->target_->setPrevious(newRI);

            cfgIter.find(generatedCFG_, 
                         newRI);
            // We stepped back one...
            cfgIter++;
            prev = newRI; 
            continue;
        }        
        prev = obj;
    }

    updateInsnDict();

}

// Whaddya know, on IA-64 multiTramps can overlap basic blocks (which can
// start inside of bundles). Argh.

multiTramp::multiTramp(Address addr,
                       unsigned size,
                       int_function *func) :
    generatedCodeObject(),
    id_(id_ctr++),
    instAddr_(addr),
    trampAddr_(0),
    trampSize_(0),
    instSize_(size),
    branchSize_(0),
    usedTrap_(false),
    func_(func),
    proc_(func->proc()),
    insns_(addrHash4),
    previousInsnAddrs_(NULL),
    generatedMultiT_(),
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(false),
    trampEnd_(NULL),
    isActive_(false),
    partlyGone_(false),
    stompMulti_(NULL)
{
    // .. filled in by createMultiTramp
    assert(proc());
    proc()->addMultiTramp(this);
    mal_printf("new multi id=%d instAddr=%lx func %lx[%lx]\n",
               id_,instAddr_,func_->ifunc()->getOffset(),
               func_->get_address());
    // find the base address at which the function relocation is created, the
    // first block, in address order, is the first to be laid out in the 
    // relocated function, it shouldn't be possible for the function to have 
    // been updated since then, so there can't be new blocks that are not part 
    // of the relocation, an assertion makes sure that this is the case 
    int_basicBlock *firstBlock = func_->findBlockByAddr(func_->getAddress());
    funcBaseInMutatee_ = 
        firstBlock->instVer( firstBlock->numInstances()-1 )->firstInsnAddr();
}


// "Copy" constructor; for in-place replacements.
multiTramp::multiTramp(multiTramp *oM) :
    generatedCodeObject(),
    id_(oM->id_),
    instAddr_(oM->instAddr_),
    trampAddr_(0),
    trampSize_(0),
    instSize_(oM->instSize_),
    branchSize_(0),
    usedTrap_(false),
    func_(oM->func_),
    proc_(oM->proc_),
    insns_(addrHash4),
    previousInsnAddrs_(NULL),
    generatedMultiT_(), // Not copied
    jumpBuf_(), // Not copied
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(true),
    trampEnd_(NULL),
    isActive_(false),
    partlyGone_(false),
    funcBaseInMutatee_(oM->funcBaseInMutatee_),
    stompMulti_(NULL)
{
    // This is superficial and insufficient to recreate the multiTramp; please
    // call replaceCode with the old tramp as an argument.
}

// Fork constructor.
multiTramp::multiTramp(const multiTramp *parMulti, AddressSpace *child) :
    generatedCodeObject(parMulti, child),
    id_(parMulti->id_),
    instAddr_(parMulti->instAddr_),
    trampAddr_(parMulti->trampAddr_),
    trampSize_(parMulti->trampSize_),
    instSize_(parMulti->instSize_),
    branchSize_(parMulti->branchSize_),
    usedTrap_(parMulti->usedTrap_),
    func_(NULL),
    proc_(child),
    insns_(addrHash4),
    previousInsnAddrs_(NULL),
    generatedMultiT_(parMulti->generatedMultiT_),
    jumpBuf_(parMulti->jumpBuf_),
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(parMulti->changedSinceLastGeneration_),
    trampEnd_(),
    isActive_(false),
    partlyGone_(false),
    funcBaseInMutatee_(parMulti->funcBaseInMutatee_),
    stompMulti_(NULL)
{
    // TODO:
    // Copy logical state: insns_ and generatedCFG;
    // Copy savedCode. 
    
    if (parMulti->savedCodeBuf_) {
        savedCodeBuf_ = malloc(instSize_);
        memcpy(savedCodeBuf_, parMulti->savedCodeBuf_, instSize_);
    }

    func_ = child->findFuncByAddr(instAddr_);
    assert(func_);

    // Get the CFG.
    
    generatedCFG_ = generatedCFG_t(parMulti->generatedCFG_, this, child);

    // Now that the CFG is right, we get insns
    updateInsnDict();

    child->addOrigRange(this);

    // And we should not have been replacing a previous one and had a fork
    // hit...
    assert(parMulti->previousInsnAddrs_ == NULL);
}

////////////

// To avoid mass include inclusion
int_function *multiTramp::func() const { return func_; }

AddressSpace *multiTramp::proc() const { return proc_; }

int fooDebugFlag = 0;

// Some debuggers don't stop at lines very well.
void debugBreakpoint() {
    fooDebugFlag = 1;
}

// Generate the code for a multiTramp
// baseInMutator: a pointer to the local copy of a basic block
// baseInMutatee: basic block address
// offset: offset into the two above.
// Since we out-line multiTramps (effectively), we write
// a jump into the inputs.
bool multiTramp::generateCode(codeGen & /*jumpBuf...*/,
                              Address /*baseInMutatee*/,
                              UNW_INFO_TYPE * * /* ignored */) 
{
    if (!hasChanged() && generated_) {
        // We don't actually use the input code generator yet;
        // if we go entirely inlined we will.
        //assert(gen.currAddr(baseInMutatee) == instAddr_);
        //gen.moveIndex(instSize_);
        assert(generatedMultiT_.used() > 0);
        return true;
    }

    unsigned size_required = 0;

    generatedCFG_t::iterator cfgIter;
    generatedCodeObject *obj = NULL;

    inst_printf("Generating multiTramp from addr 0x%lx\n",
                instAddr_);

    // We might be getting called but nothing changed...
    if (!generated_) {
	/* 	this is part of the code to ensure that when we add the call to dlopen
		at the entry of main on AIX during save the world, any multi that was
		already there gets regenerated
	*/
        assert(!trampAddr_);
        assert(generatedMultiT_ == NULL);
        assert(jumpBuf_ == NULL);
        
        // A multiTramp is the code sequence for a set of instructions in
        // a basic block and all baseTramps, etc. that are being used for
        // those instructions. We use a recursive code generation
        // technique: for each item in the insns_ vector, generate the
        // preTramp (if it exists), relocate the instruction, and generate
        // the postTramp or targetTramp (if they exist). As an
        // optimization, we don't double-generate base tramps; if
        // consecutive instructions are instrumented, only one baseTramp
        // will be generated between them.
        
        // We use a vector of instructions to support our iterative
        // generation technique:
        // 1) Generate representation of all instructions and tramps
        // 2) Given offsets for generated code, fix any branches that need it.
        // 3) Determine the size of the generated code
        // 4) Allocate memory in the mutatee
        // 5) Given the final address (determined in step 4), check to see if the
        //   final code size changes; if retry and repeat.
        
        // We use a vector of a vector of instructions. Each instruction
        // vector represents a single logical stream of instructions,
        // whether a base tramp or a relocated instruction. All of these
        // combined represent the multitramp. This multi-level system
        // allows us to modify instructions easily if necessary.
        // That part is TODO, btw.

        // Code generation is easy. Iterate through the generatedCFG
        // object. Fallthrough happens naturally; if there's a target,
        // pin it in place (which guarantees that it will be generated at that
        // address) and set the jump.

        // First, we determine how much space we need.

        cfgIter.initialize(generatedCFG_);
        while ((obj = cfgIter++)) {
            // If we're the target for someone, pin at this
            // addr. This means some wasted space; however, it's
            // easier and allows us to generate in one pass.
            
#if !defined(cap_noaddr_gen)
            if (obj->previous_ &&
                obj->previous_->target_ == obj) {
                // We're a target for somebody.
                obj->pinnedOffset = size_required;
            }
#endif
            
            // Then update the size
            size_required += obj->maxSizeRequired();
            //inst_printf("... %d bytes, total %d\n",
            //obj->maxSizeRequired(), size_required);
        }
        // We never re-use multiTramps
        assert(!trampAddr_);

        inferiorHeapType heapToUse = anyHeap;
#if defined(os_aix) && defined(bug_aix_broken_fork)
        // We need the base tramp to be in allocated heap space, not scavenged
        // text space, because scavenged text space is _not_ copied on fork.
        // Argh.
        if (func()->prettyName() == "__fork") {
            heapToUse = (inferiorHeapType) (textHeap | dataHeap); // not uncopiedHeap
        }
#endif

        trampAddr_ = proc()->inferiorMalloc(size_required,
                                            heapToUse,
                                            instAddr_);
        inst_printf("%s[%d]: inferiorMalloc returned %x\n", FILE__, __LINE__, trampAddr_);
        if (!trampAddr_) {
            fprintf(stderr, "Failed to inferiorMalloc, ret false\n");
            return false;
        }
        generatedMultiT_.allocate(size_required);
        generatedMultiT_.setAddrSpace(proc());
        generatedMultiT_.setAddr(trampAddr_);
        generatedMultiT_.setFunction(func());

        // We don't want to generate the jump buffer until after
        // we've done the multiTramp; we may need to know
        // where instructions got moved to if we trap-fill.
    }
    else {
        assert(!changedSinceLastGeneration_);
        assert(generatedMultiT_ != NULL);
        assert(jumpBuf_ != NULL);
        assert(trampAddr_);
        
        // We go through the motions again to give everyone
        // a chance to say "I need to do something"
        size_required = trampSize_;
    }
    
    if (!generated_) {
        jumpBuf_.allocate(instSize_);
        jumpBuf_.setAddrSpace(proc());

        // We set this = true before we call generateBranchToTramp
        generated_ = true;
        if (!generateBranchToTramp(jumpBuf_)) {
            inst_printf("%s[%d]: MT %p needs reloc, can't install\n", 
                        __FILE__, __LINE__, this);
            return false;
        }
    }
    
    
    generatedMultiT_.setIndex(0);

    UNW_INFO_TYPE **unwind_region = NULL; 
#if defined( cap_unwind )
	/* Initialize the unwind information structure. */
	if( unwindInformation != NULL ) { free( unwindInformation ); }
    unwindInformation = (unw_dyn_info_t *)calloc( 1, sizeof( unw_dyn_info_t ) );
    assert( unwindInformation != NULL );
    
    unwindInformation->format = UNW_INFO_FORMAT_DYNAMIC;
    unwindInformation->prev = NULL;
    unwindInformation->next = NULL;
    
    unwindInformation->u.pi.name_ptr = (unw_word_t) "dynamic instrumentation";
    unwindInformation->u.pi.handler = (Address) NULL;
    
    /* Generate the initial region, and then link it in.  This way,
       we can pass around a region pointer reference. */
    unw_dyn_region_info_t * initialRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
    assert( initialRegion != NULL );
    
    /* Special format for point ALIAS: a zero-length region. */
    initialRegion->insn_count = 0;
    initialRegion->op_count = 2;
    initialRegion->next = NULL;
    
    dyn_unw_printf( "%s[%d]: aliasing multitramp to 0x%lx\n", __FILE__, __LINE__, instAddr_ );
    _U_dyn_op_alias( & initialRegion->op[0], _U_QP_TRUE, -1, instAddr_ );
    _U_dyn_op_stop( & initialRegion->op[1] );
    
    unwindInformation->u.pi.regions = initialRegion;
    
    /* For the iterator, below. */
    unwind_region = &initialRegion;
#endif /* defined( cap_unwind ) */
            
    //Call ::generate(...) for each object
    generateCodeWorker(size_required, unwind_region);

    trampSize_ = generatedMultiT_.used();

    // Free up some of that memory...
    proc()->inferiorRealloc(trampAddr_, trampSize_);
    generatedMultiT_.finalize();

    // Now that we know where we're heading, see if we can put in 
    // a jump
    assert(instAddr_);
    assert(instSize_);

    changedSinceLastGeneration_ = false;
    
    //debugBreakpoint();
   
    return true;
}

#if defined(cap_noaddr_gen)
bool multiTramp::generateCodeWorker(unsigned size_required, 
                                    UNW_INFO_TYPE **unwind_region)
{
   generatedCFG_t::iterator cfgIter;
   generatedCodeObject *obj = NULL;
   
   inst_printf("address-less generation: local %p, remote 0x%x, size %d\n",
               generatedMultiT_.start_ptr(), trampAddr_, size_required);
   
   cfgIter.initialize(generatedCFG_);
   obj = NULL;
   while ((obj = cfgIter++)) {
      // Target override if necessary
      if (obj->target_) {
         relocatedInstruction *relocInsn = dynamic_cast<relocatedInstruction *>(obj);
         assert(relocInsn);
         relocInsn->overrideTarget(obj->target_);
      }
      generatedMultiT_.setObj(obj);
      if( ! obj->generateCode( generatedMultiT_, trampAddr_, unwind_region ) ) {
         return false;
      }
      assert(obj->generated());
      // Safety...
      assert(generatedMultiT_.used() <= size_required);
   }

   generatedMultiT_.applyPCRels(trampAddr_);
   return true;
}
#else
bool multiTramp::generateCodeWorker(unsigned size_required, 
                                    UNW_INFO_TYPE **unwind_region)
{
   generatedCFG_t::iterator cfgIter;
   generatedCodeObject *obj = NULL;
   
   inst_printf("multiTramp generation: local %p, remote 0x%x, size %d\n",
               generatedMultiT_.start_ptr(), trampAddr_, size_required);
   
   cfgIter.initialize(generatedCFG_);
   obj = NULL;
   while ((obj = cfgIter++)) {
      if (obj->pinnedOffset) {
         // We need to advance the pointer, backfilling
         // with noops (or illegals, actually)
         // This won't do anything if we're in the right place
         assert(generatedMultiT_.used() <= obj->pinnedOffset);
         
         inst_printf("... NOOP-filling to %d, currently %d\n",
                     obj->pinnedOffset, generatedMultiT_.used());
         generatedMultiT_.fill(obj->pinnedOffset - generatedMultiT_.used(),
                               codeGen::cgNOP);
         assert(generatedMultiT_.used() == obj->pinnedOffset);
      }
        
      // Target override if necessary
      toAddressPatch override(0);
      if (obj->target_) {
         relocatedInstruction *relocInsn = dynamic_cast<relocatedInstruction *>(obj);
         assert(relocInsn);
         override.set_address(obj->target_->pinnedOffset + trampAddr_);
         relocInsn->overrideTarget(&override);
      }
      
      generatedMultiT_.setObj(obj);
      if( ! obj->generateCode( generatedMultiT_, trampAddr_, unwind_region ) ) {
         return false;
      }
      assert(obj->generated());
      // Safety...
      assert(generatedMultiT_.used() <= size_required);
   }
   return true;
}
#endif

bool multiTramp::installCode() {
   inst_printf("%s[%d]: installing multiTramp 0x%lx to 0x%lx\n",
               FILE__, __LINE__, instAddr_, instAddr_ + instSize_);
    // We need to add a jump back and fix any conditional jump
    // instrumentation
    assert(generatedMultiT_.used() == trampSize_); // Nobody messed with things
    assert(generated_);

    // Try to branch to the tramp, but if we can't use a trap
    if (branchSize_ > instSize_)  {
        // Crud. Go with traps.
        jumpBuf_.setIndex(0);
        if (!generateTrapToTramp(jumpBuf_)) {
           inst_printf("%s[%d]: \t failed to generate trap to tramp, ret false\n",
                       FILE__, __LINE__);
           return false;
        }
    }
    fillJumpBuf(jumpBuf_);

    if (!installed_) {
        inst_printf("Copying multitramp (inst 0x%lx to 0x%lx) from 0x%p to 0x%lx, %d bytes\n",
                    instAddr_, instAddr_+instSize_, generatedMultiT_.start_ptr(), trampAddr_, trampSize_);

        bool success = proc()->writeTextSpace((void *)trampAddr_,
                                              trampSize_,
                                              generatedMultiT_.start_ptr());
        if( success ) {
            proc()->addOrigRange(this);
#if defined( cap_unwind )
            static bool warned_buggy_libunwind = false;
            if( unwindInformation != NULL ) {
                unwindInformation->start_ip = trampAddr_;
                unwindInformation->end_ip = trampAddr_ + trampSize_;
                unwindInformation->gp = proc()->proc()->getTOCoffsetInfo( instAddr_ );
            
                dyn_unw_printf( "%s[%d]: registering multitramp unwind information for 0x%lx, at 0x%lx-0x%lx, GP 0x%lx\n",
                                __FILE__, __LINE__, instAddr_, unwindInformation->start_ip, unwindInformation->end_ip,
                                unwindInformation->gp );
	            if( ! proc()->proc()->insertAndRegisterDynamicUnwindInformation( unwindInformation ) ) 
                    {
                        if(!warned_buggy_libunwind)
                        {
                            std::cerr << "Insertion of unwind information via libunwind failed; stack walks outside of instrumented areas may not behave as expected" << std::endl;
                            warned_buggy_libunwind = true;
                        }
                    }
            }
#endif /* defined( cap_unwind ) */
        }
        else { return false; }
    }


    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        obj->installCode();
    }
    
    installed_ = true;
    return true;
}

// The multitramp has been installed; now actually put the jump
// in. Also has the fun job of relocating the PC to the new tramp (if
// possible).

bool multiTramp::linkCode() {

    inst_printf("%s[%d]: Entering multiTramp::linkCode (%p)\n",
                FILE__, __LINE__, this);

    // Relocation should be done before this is called... not sure when though.

    // We may already be linked in, and getting called because of a regeneration.
    // 
    // First, copy out where we're going
    assert(installed_);

    assert(jumpBuf_.used() == instSize_);
    inst_printf("Linking multiTramp 0x%lx to 0x%lx, to 0x%lx to 0x%lx\n",
                instAddr_, instAddr_ + instSize_,
                trampAddr_, trampAddr_ + trampSize_);
    if (!linked_) {
        codeRange *prevRange = proc()->findModByAddr(instAddr_);
        if (prevRange != NULL) {
            
            inst_printf("%s[%d]: this address already modified: %p (cur) %p (prev)\n",
                        FILE__, __LINE__, this, prevRange);

            // Someone's already here....
            if (prevRange->is_function_replacement()) {
                // Don't install here, just dummy-return true
                inst_printf("Linking at function replacement, skip, ret true\n");
                return true;
            }
            else if (prevRange->is_replaced_call()) {
                // TODO
                fprintf(stderr, "ERROR: instrumentation stomping on replaced call!\n");
                return true;
            }
            else if (prevRange->is_multitramp()) {
                inst_printf("%s[%d]: previous range was multiTramp, overwriting\n",
                            FILE__, __LINE__);
                // This is okay...
            }
            else {
                fprintf(stderr, "ERROR: instrumentation stomping on something unknown!\n");
                return true;
            }
        }
        if (!savedCodeBuf_) {
            // We may have an old version's savedCode.
            
            savedCodeBuf_ = malloc(instSize_);
            if (!proc()->readTextSpace((void *)instAddr_,
                                       instSize_,
                                       savedCodeBuf_)) {
            }
        }
        if (!proc()->writeTextSpace((void *)instAddr_,
                                    instSize_,
                                    jumpBuf_.start_ptr())) {
           fprintf(stderr, "ERROR: failed to write %d to 0x%lx\n",
                   instSize_, instAddr_);
           return false;
        }
        linked_ = true;
    }

    // Time to stomp old multiTramps with traps...
    if (previousInsnAddrs_ && BPatch::bpatch->isMergeTramp()) {
        codeGen gen(16); // Overkill, it's either 1, 4, or 16.
        insnCodeGen::generateTrap(gen);

        for (unsigned i = 0; i < previousInsnAddrs_->size(); i++) {
            pdpair<Address, Address> addrs = (*previousInsnAddrs_)[i];
            Address uninst = addrs.first;
            Address oldMultiAddr = addrs.second;

            if (oldMultiAddr == 0) continue;
            if (!insns_.find(uninst)) {
                assert(0);
            }
            Address newMultiAddr = insns_[uninst]->relocAddr();
            
            if (!proc()->writeTextSpace((void *)oldMultiAddr,
                                        gen.used(),
                                        gen.start_ptr())) {
                fprintf(stderr, "ERROR: failed to write %d to %p\n",
                        gen.used(), gen.start_ptr());
                return false;
            }
            proc()->trapMapping.addTrapMapping(oldMultiAddr, newMultiAddr, false);
        }

        delete previousInsnAddrs_;
        previousInsnAddrs_ = NULL;
    }


    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        obj->linkCode();
        assert(obj->linked());
    }

    inst_printf("%s[%d]: returning true from multiTramp::linkCode (%p)\n",
                FILE__, __LINE__, this);
    return true;
}

// And a wrapper for the above
multiTramp::mtErrorCode_t multiTramp::linkMultiTramp() {
    inst_printf("%s[%d]: multiTramp::linkMultiTramp(%p)\n",
                FILE__, __LINE__, this);

    // We can call generate, return an error, then call install anyway.
    if (!installed_) {
        inst_printf("%s[%d]: multiTramp::linkMultiTramp(%p): not installed, ret mtError\n",
                    FILE__, __LINE__, this);
        return mtError;
    }

    assert(!hasChanged()); // since we generated code...

    if (linkCode()) {
        inst_printf("%s[%d]: multiTramp::linkMultiTramp(%p): linked, ret mtSuccess\n",
                    FILE__, __LINE__, this);
        return mtSuccess;
    }
    else {
        inst_printf("%s[%d]: multiTramp::linkMultiTramp(%p): failed linko, ret mtError\n",
                    FILE__, __LINE__, this);
        return mtError;
    }
}

multiTramp::mtErrorCode_t multiTramp::installMultiTramp() {

    // We can call generate, return an error, then call install anyway.
    if (!generated_) return mtError;

    assert(!hasChanged()); // Since we generated code...

    // See if there is enough room to fit the jump in... then
    // decide whether to go ahead or not.

    if (installCode()) {
      return mtSuccess;
    }
    else {
        return mtError;
    }
}

bool multiTramp::enable() {
    if (!linked_)
        return true;
    // Copy in the jumpBuf. Like linkCode
    assert(jumpBuf_.used() == instSize_);
    
    if (!proc()->writeTextSpace((void *)instAddr_,
                                jumpBuf_.used(),
                                jumpBuf_.start_ptr())) 
        return false;
    return true;
}


bool multiTramp::disable() {
    // This could also be "whoops, not ready, skip"
    if (!linked_) {
        return true;
    }

    assert(savedCodeBuf_);

    bblInstance *bbi = func()->proc()->findOrigByAddr(instAddr_)->
        is_basicBlockInstance();
    if (bbi && 0 == bbi->version()) {
        void *objPtr = func()->obj()->getPtrToInstruction(instAddr_);
        if (memcmp(objPtr,savedCodeBuf_,instSize_)) {
            mal_printf("WARNING: different bytes in the mappedFile and "
                "multiTramp's saved code buf [%lx %lx], either the code was "
                "overwritten or there's a bug %s[%d]\n",instAddr_,
                instAddr_ + instSize_, FILE__, __LINE__);
        }
        if (!proc()->writeTextSpace((void *)instAddr_,
                                    instSize_,
                                    objPtr))
            return false;
    } 
    else {
    if (!proc()->writeTextSpace((void *)instAddr_,
                                instSize_,
                                savedCodeBuf_))
        return false;
    }
    return true;
}


Address multiTramp::instToUninstAddr(Address addr) {
    if ((addr < trampAddr_) ||
        addr > (trampAddr_ + trampSize_))
        // Possibly an assert fail? 
        return addr;

    // Okay, there's two possibilities: we're at an instruction, or
    // we're in a base tramp. If we're at an instruction, figure out
    // its offset in the original function, add to "uninstAddr_", and
    // we're done. If we're in a base tramp, then set it as the _next_
    // instruction.

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);
        replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);

        if (insn && 
            (addr >= insn->relocAddr()) &&
            (addr < insn->relocAddr() + insn->get_size()))
        {
            // in exploratory mode, the function relocation may
            // have been invalidated due to new code discovery;
            // we may have to return the origAddr instead
            if (proc()->findBasicBlockByAddr(insn->fromAddr_)) {
                return insn->fromAddr_;
            } else {
                return insn->origAddr_;
            }
        }
        if (ri && 
            (addr >= ri->get_address()) &&
            (addr < ri->get_address() + ri->get_size()))
            return ri->uninstrumentedAddr();

        if (bti && bti->isInInstance(addr)) {
            // If we're pre for an insn, return that;
            // else return the insn we're post/target for.
            baseTramp *baseT = bti->baseT;
            instPoint *point = baseT->instP();
	    assert(point);

            for (unsigned i = 0; i < point->instances.size(); i++) {
                // We check by ID instead of pointer because we may
                // have been replaced by later instrumentation, but 
                // are still being executed.
                if (point->instances[i]->multiID() == id_)
                    return point->addr();
            }
            // No match: bad data structures.
            fprintf(stderr, "ERROR: data structures corrupted!\n");
            fprintf(stderr, "Looking for 0x%lx\n", addr);
            fprintf(stderr, "multiTramp %p reports being 0x%lx to 0x%lx, from 0x%lx to 0x%lx in orig func\n",
                    this, trampAddr_, trampAddr_ + trampSize_, instAddr_, instAddr_ + instSize_);
            if ( !func()->obj()->isExploratoryModeOn() ) {
                assert(0);
            } else {
                // this can happen when a multitramp is marked as active 
                // and the isntrumentation has been removed
                // assert(0);
                return point->addr();
            }
        }
        if (end) {
            // Ah hell. 
            // Umm... see if we're in the size_ area
            if ((end->addrInMutatee_ <= addr) &&
                (addr < (end->addrInMutatee_ + end->size_))) {
                return end->target();
            }
        }
    }

    // Ran out of iterator... 
    for (unsigned i = 0; i < deletedObjs.size(); i++) {
        obj = deletedObjs[i];
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);
        replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);

        if (ri && 
            (addr >= ri->get_address()) &&
            (addr < ri->get_address() + ri->get_size()))
            return ri->uninstrumentedAddr();

        if (insn && 
            (addr >= insn->relocAddr()) &&
            (addr < insn->relocAddr() + insn->get_size()))
            return insn->fromAddr_;
        if (bti && bti->isInInstance(addr)) {
            // If we're pre for an insn, return that;
            // else return the insn we're post/target for.
            baseTramp *baseT = bti->baseT;
            instPoint *point = baseT->instP();
	    assert(point);

            return point->addr();
            // We used to check by instances... thing is, we can remove instances
            // and still be running inside that instrumentation.

        }
        if (end) {
            // Ah hell. 
            // Umm... see if we're in the size_ area
            if ((end->addrInMutatee_ <= addr) &&
                (addr < (end->addrInMutatee_ + end->size_))) {
                return end->target();
            }
        }
    }

    // Assert: we should never reach here

    // DEBUGGING TIME! Break it down...
    
    generatedCFG_t::iterator debugIter(generatedCFG_);

    fprintf(stderr, "ERROR: Multitramp address mapping for addr 0x%lx not found!\n",
            addr);

    fprintf(stderr, "First pointer in internal CFG: %p\n",
            generatedCFG_.start());

    while ((obj = debugIter++)) {
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);
        replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);

        if (ri) {
            fprintf(stderr, "Replaced instruction from 0x%lx to 0x%lx\n",
                    ri->get_address(),
                    ri->get_address() + ri->get_size());
        }
        if (insn) {
            fprintf(stderr, "Relocated instruction from 0x%lx to 0x%lx\n",
                    insn->relocAddr(), insn->relocAddr() + insn->get_size());
        }
        if (bti)
            fprintf(stderr, "Base tramp instance from 0x%lx to 0x%lx\n",
                    bti->trampPreAddr(),
                    bti->trampPostAddr() + bti->restoreEndOffset);
        if (end) {
            fprintf(stderr, "Tramp end from 0x%lx to 0x%lx\n",
                    end->addrInMutatee_,
                    end->addrInMutatee_ + end->size_);
        }
    }

    // Ran out of iterator... 
    fprintf(stderr, "Checking %lu deleted objects\n", (unsigned long) deletedObjs.size());
    for (unsigned i = 0; i < deletedObjs.size(); i++) {
        obj = deletedObjs[i];
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);
        replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);

        if (ri) {
            fprintf(stderr, "<DELETED> Replaced instruction from 0x%lx to 0x%lx\n",
                    ri->get_address(),
                    ri->get_address() + ri->get_size());
        }

        if (insn) {
            fprintf(stderr, "<DELETED> Relocated instruction from 0x%lx to 0x%lx\n",
                    insn->relocAddr(), insn->relocAddr() + insn->get_size());
        }
        if (bti)
            fprintf(stderr, "<DELETED> Base tramp instance from 0x%lx to 0x%lx\n",
                    bti->trampPreAddr(),
                    bti->trampPostAddr() + bti->restoreEndOffset);
        if (end) {
            fprintf(stderr, "<DELETED> Tramp end from 0x%lx to 0x%lx\n",
                    end->addrInMutatee_,
                    end->addrInMutatee_ + end->size_);
        }
    }

    assert(0);
    return 0;
}

instPoint *multiTramp::findInstPointByAddr(Address addr) {
    // Like the instToUninst above; but this time find the
    // closest instPoint that we're in. If we're in a baseTramp
    // or relocatedInstruction it's easy. trampEnd goes to predecessor
    // if there is one. Returns NULL if we're in instrumentation
    // but not at an instPoint (i.e., if we're inside a basic block
    // but not at the instPoint).

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        if ((obj->get_address() <= addr) &&
            (addr < (obj->get_address() + obj->get_size()))) {
            const relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
            baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
            trampEnd *end = dynamic_cast<trampEnd *>(obj);
            replacedInstruction *ri = dynamic_cast<replacedInstruction *>(obj);
            
            if (ri) {
                // We can pull what we need out of oldInsn...
                insn = ri->oldInsn_;
            }
            if (end) {
                // We don't want the tramp end; so see if there is a previous
                // base tramp or instruction
                insn = dynamic_cast<relocatedInstruction *>(obj->previous_);
                bti = dynamic_cast<baseTrampInstance *>(obj->previous_);
            }
            assert(insn || bti);

            if (insn)
                return func()->findInstPByAddr(addr);
            else if (bti) {
                return bti->findInstPointByAddr(addr);
            }
        }
    }
    return NULL;
}    

Address multiTramp::uninstToInstAddr(Address addr) {
    // The reverse of the above; given an original address,
    // find the matching addr in the multiTramp. However,
    // there's a bit of a catch -- if there's a preTramp,
    // relocate to there.
    
    if (addr < instAddr()) return 0;
    if (addr >= (instAddr() + instSize())) return 0;

    assert(generated_);

    relocatedCode *insn = NULL;
    while (!insns_.find(addr)) {
        addr--;
        if (addr < instAddr()) return trampAddr_;
    }
    
    insn = insns_[addr];
    
    if (!insn) {
        // This is expected
        return 0;
    }
    // Check for preTramp
    baseTrampInstance *pre = dynamic_cast<baseTrampInstance *>(insn->previous_);
    if (pre && pre->trampPreAddr()) {
        return pre->trampPreAddr();
    }

    assert(insn->relocAddr());
    return insn->relocAddr();
}

////////////////////////////////////////////////////////////////////
// Key methods
////////////////////////////////////////////////////////////////////

// This is the key multiTramp mechanism. It is reponsible for creating
// a new (*) multiTramp structure and generating all necessary code
// for the multiTramp, all relocated instructions, baseTramps, miniTramps,
// and on down the line. There are several calling possibilities:
// 
// 1) This is the first time the multiTramp has been generated; we have
//   a skeleton structure that needs to be filled out.
// 2) The multiTramp was generated but not installed; we can throw away the
//   work we've done and start over.
// 3) The multiTramp was generated and installed, and we're doing it again.
//   In this case, we actually make a new structure and keep the old one
//   to track deletion. 
//
// In all cases, check whether the current multiTramp is current given the
// instrumentation state. If not, regenerate the multiTramp; this may be
// done in-place or via replacement.

multiTramp::mtErrorCode_t multiTramp::generateMultiTramp() 
{
    updateInstInstances();
    if (hasChanged()) {
        if (linked_) {
            // We're in the process' address space; if we need to change the
            // multiTramp, we replace it. replaceMultiTramp takes care of
            // all that. 
            bool deleteReplaced = false;
            bool res = multiTramp::replaceMultiTramp(this, deleteReplaced);
            if (res) 
                return mtSuccess;
            else
                return mtError;
        }
        if (installed_ || generated_) {
            // Odd case... we generated, but never installed. Huh.
            invalidateCode();
        }
    }
    // TODO: generatedCodeObjects return booleans. We need a broader
    // error return type, probably #defines
    if (!generateCode(jumpBuf_, instAddr_, NULL))
        return mtError;

    // Relocation...
    if (branchSize_ > instSize_)
        return mtTryRelocation;

    return mtSuccess;
}

void generatedCodeObject::invalidateCode() {
    assert(!linked_);
    pinnedOffset = 0;
    size_ = 0;
    addrInMutatee_ = 0;
    generated_ = false;
    installed_ = false;
}

void multiTramp::invalidateCode() {
    generatedCodeObject::invalidateCode();

    // Recursive...
    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        obj->invalidateCode();
    }
    
    if (generatedMultiT_ != NULL) {
        generatedMultiT_.invalidate();
    }
    if (jumpBuf_ != NULL) {
        jumpBuf_.invalidate();
    }

    if (savedCodeBuf_)
        free(savedCodeBuf_);
    savedCodeBuf_ = NULL;

    if (trampAddr_) {
        // Allocated but never used; we can immediately delete
        proc()->inferiorFree(trampAddr_);
    }

    trampAddr_ = 0;
    trampSize_ = 0;

    generated_ = false;
    installed_ = false;
}

// Return value: were we replaced with a new one.
bool multiTramp::replaceMultiTramp(multiTramp *oldMulti, 
                                   bool &deleteReplaced) {
    // We in-line multiTramps; so deleteReplaced is always true.
    // It's a suggestion to the caller of what to do with the
    // old tramp, and can be ignored.
    deleteReplaced = false; // Set to false initially until there's
    // a new multiTramp

    // Something has changed enough for us to care about it;
    // we have a multiTramp that is no longer up to date. So
    // fix. Two options: 1) we're actually okay due to changes
    // in out-of-line code; 2) we need to replace the current
    // multiTramp with a new one and generate it.

    // Note: we need to leave the new multiTramp in the same condition
    // as the old one; if generated, generate; if installed, install;
    // If linked, link. The latter will get automatically handled
    // through calls to generateCode, installCode, and linkCode; 
    // all we have to do is make sure the data structures are set
    // up right.

    if (oldMulti->proc()->findMultiTrampById(oldMulti->id())!= oldMulti)
    {
       //The old multitramp has already been unlinked and removed.
       //Don't bother doing the replacement.
       return true;
    }
    
    
    multiTramp *newMulti = dynamic_cast<multiTramp *>(oldMulti->replaceCode(NULL));
    assert(newMulti);

    // newMulti either has copies of old data structures (all
    // generatedCodeObjects) or the originals, depending on which
    // is appropriate. Status flags are not copied; the new
    // tramp is neither generated nor linked.
    
    // Sub the new tramp into the old tramp:
    assert(newMulti->proc() == oldMulti->proc());
    AddressSpace *proc = oldMulti->proc();
    assert(proc->findMultiTrampById(oldMulti->id()) == oldMulti);
    assert(oldMulti->id() == newMulti->id());
    proc->addMultiTramp(newMulti);

    // Generate/install/link; this could be redundant depending on
    // the next few calls, or we could be getting called from a
    // removed baseTramp.

    bool res = false;

    if (oldMulti->generated()) {
        // We don't want to just call generateMultiTramp; that's possibly
        // what called us.
        assert(newMulti->jumpBuf_ == NULL);
        res = newMulti->generateCode(newMulti->jumpBuf_,
                                     newMulti->instAddr_,
                                     NULL);
        if (!res) return false;
    }
    if (oldMulti->installed()) {
        res = newMulti->installCode();
        if (!res) return false;
    }
    if (oldMulti->linked()) {

        // record info about the multi that we're stomping, if there is one
        if ( newMulti->previousInsnAddrs_ && 
             newMulti->previousInsnAddrs_->size() ) {
            oldMulti->stompMulti_ = newMulti;
            if (oldMulti->isActive_) {
                newMulti->setIsActive(true);
                newMulti->proc()->proc()->addActiveMulti(newMulti);
            }
        }

        res = newMulti->linkCode();
        if (!res) return false;
    }

    // Caller decides whether to remove the original version
    deleteReplaced = true;
    return true;
}

generatedCodeObject *generatedCFG_t::copy_int(generatedCodeObject *obj, 
                                              generatedCodeObject *par,
                                              multiTramp *newMulti,
                                              pdvector<generatedCodeObject *> &unused) {
    generatedCodeObject *newObj = obj->replaceCode(newMulti);

    if (newObj != obj) {
        int found_index = -1;
        for (unsigned int i = 0; i <unused.size(); ++i) {
           if (obj == unused[i]) {
              found_index = i;
              break;
           }
        }
        if (-1 == found_index) 
           unused.push_back(obj);
    }

    newObj->setPrevious(par);
    if (obj->fallthrough_)
        newObj->setFallthrough(copy_int(obj->fallthrough_,
                                        newObj,
                                        newMulti,
                                        unused));
    if (obj->target_)
        newObj->setTarget(copy_int(obj->target_, 
                                   newObj,
                                   newMulti,
                                   unused));
    return newObj;
}

generatedCodeObject *generatedCFG_t::fork_int(const generatedCodeObject *parObj, 
                                              generatedCodeObject *childPrev,
                                              multiTramp *childMulti,
                                              AddressSpace *child) {
    // Since you can't do a virtual constructor...
    // Could add a fork() method to everyone, but I like the consistency of the
    // constructor(..., AddressSpace *) model.
    const baseTrampInstance *bti = dynamic_cast<const baseTrampInstance *>(parObj);
    const trampEnd *te = dynamic_cast<const trampEnd *>(parObj);
    const relocatedInstruction *ri = dynamic_cast<const relocatedInstruction *>(parObj);
    const replacedInstruction *repI = dynamic_cast<const replacedInstruction *>(parObj);

    generatedCodeObject *childObj = NULL;

    if (bti) {
        assert(!te);
        assert(!ri);
        assert(!repI);
        // We need to get the child baseTramp. We duck through an instPoint to
        // do so. We could also register all baseTramps with the process object,
        // but fork is infrequent and so can be expensive.
        baseTramp *parBT = bti->baseT;
        instPoint *cIP = NULL;
        baseTramp *childBT = NULL;

	int_function *cFunc = childMulti->func();
	assert(cFunc);

	cIP = cFunc->findInstPByAddr(parBT->instP()->addr());
	assert(cIP);
	
	// We split here... post and target show up as "pre" to the BT
	if (parBT->instP()->preBaseTramp() == parBT) 
	  childBT = cIP->preBaseTramp();
	else if (parBT->instP()->postBaseTramp() == parBT)
	  childBT = cIP->postBaseTramp();
	else if (parBT->instP()->targetBaseTramp() == parBT)
	  childBT = cIP->targetBaseTramp();
	else 
	  assert(0);
	
        assert(childBT);
        childObj = new baseTrampInstance(bti,
                                         childBT,
                                         childMulti,
                                         child);
    }
    else if (te) {
        assert(!bti);
        assert(!ri);
        assert(!repI);
        childObj = new trampEnd(te, childMulti, child);
        childMulti->setTrampEnd(*(trampEnd*) childObj);
    } 
    else if (ri) {
        assert(!bti);
        assert(!te);
        assert(!repI);
        childObj = new relocatedInstruction(ri, childMulti, child);
    }
    else if (repI) {
       assert(!bti);
       assert(!te);
       assert(!ri);
       childObj = new replacedInstruction(repI, childMulti, child);
    }
    else {
       assert(0);
    }

    assert(childObj);
    
    assert(childObj->previous_ == NULL);
    assert(childObj->target_ == NULL);
    assert(childObj->fallthrough_ == NULL);

                
    childObj->setPrevious(childPrev);
    if (parObj->fallthrough_)
        childObj->setFallthrough(fork_int(parObj->fallthrough_,
                                          childObj,
                                          childMulti,
                                          child));
    if (parObj->target_)
        childObj->setTarget(fork_int(parObj->target_, 
                                     childObj,
                                     childMulti,
                                     child));
    return childObj;
}

generatedCFG_t::generatedCFG_t(const generatedCFG_t &parCFG,
                               multiTramp *cMT,
                               AddressSpace *child) {
    start_ = fork_int(parCFG.start_,
                      NULL,
                      cMT,
                      child);
}
    

void generatedCFG_t::replaceCode(generatedCFG_t &oldCFG, multiTramp *newMulti,
                                 pdvector<generatedCodeObject *> &unused) {
    start_ = copy_int(oldCFG.start_, 
                      NULL,
                      newMulti,
                      unused);
}

void generatedCFG_t::destroy() {
    start_ = NULL;
}

void multiTramp::updateInsnDict() {
    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    while ((obj = cfgIter++)) {
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        if (insn){
            if (insns_.find(insn->fromAddr_))
                assert(insns_[insn->fromAddr_] == insn);
            insns_[insn->fromAddr_] = insn;
        }
        replacedInstruction *replacement = dynamic_cast<replacedInstruction *>(obj);
        if (replacement) {
            insns_[replacement->oldInsn_->fromAddr_] = replacement;
        }            
    }
}

// Make an appropriately deep copy, suitable for generating a new
// multiTramp to replace the old. Also make suitable changes in the
// old one setting up replacement (assuming buffers, that sort of thing).
generatedCodeObject *multiTramp::replaceCode(generatedCodeObject *newParent) {
    // Don't handle a parent of a multiTramp yet...
    assert(newParent == NULL);

    // We deep-copy since we represent different areas in memory. First:
    // make a new tramp
    multiTramp *newMulti = new multiTramp(this);

    // We might hand over some of our objects to the new multiTramp;
    // and so the generatedCFG will not be legal after this. Make a
    // deletion vector to maintain the ones we have to get rid of.
    
    // This will hold everything; if the multiTramp pointer doesn't
    // change, then we can nuke it.

    // Copy the generatedCFG CFG.    
    // Anything not reused is stuck in deletedObjs
    newMulti->generatedCFG_.replaceCode(generatedCFG_, newMulti, deletedObjs);

    // Nuke ours, just in case
    generatedCFG_.destroy();
    
    // Update addrs
    newMulti->updateInsnDict();

    newMulti->constructPreviousInsnList(this);

    // Buffers: generatedMultiT_, savedCodeBuf_, and jumpBuf_.
    // generatedMultiT_ and jumpBuf_ are multiTramp specific.
    assert(newMulti->generatedMultiT_ == NULL);
    assert(newMulti->jumpBuf_ == NULL);
    // and savedCodeBuf_ we want to take
    newMulti->savedCodeBuf_ = savedCodeBuf_;
    savedCodeBuf_ = NULL;

    partlyGone_ = true; // multi is being unlinked and replaced

    // And final checking
    assert(newMulti->func_);
    assert(newMulti->proc_);

    assert(newMulti->instAddr_);
    assert(newMulti->instSize_);
    assert(newMulti->trampAddr_ == 0);
    assert(newMulti->trampSize_ == 0);

    return newMulti;
}

void multiTramp::constructPreviousInsnList(multiTramp *oldMulti) {
    // Only if we're using merged tramps...
    if (!BPatch::bpatch->isMergeTramp())
        return;


    if (previousInsnAddrs_ == NULL) {
        previousInsnAddrs_ = new pdvector<pdpair<Address, Address> >();
    }

    dictionary_hash_iter<Address, relocatedCode *> insnIter(oldMulti->insns_);
    Address uninstAddr;
    for (; insnIter; insnIter++) {
        uninstAddr = insnIter.currkey();
        
        const relocatedInstruction *oldRelocInsn = insnIter.currval()->relocInsn();
        
        assert(oldRelocInsn);

        // Okay... we want (reloc addr in old multi), (reloc addr in new multi)
        Address oldAddr = oldRelocInsn->relocAddr();

        if (!insns_.find(uninstAddr)) {
            // Existed in the old, not in the new?
            assert(0);
        }

        previousInsnAddrs_->push_back(pdpair<Address, Address> (uninstAddr, oldAddr));
    }
}
        


bool multiTramp::hasChanged() {
    if (changedSinceLastGeneration_) {
        return true;
    }
    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        if (obj->hasChanged()) {
            return true;
        }
    }
    return false;
}

relocatedInstruction::relocatedInstruction(const relocatedInstruction *parRI,
                                           multiTramp *cMT,
                                           AddressSpace *child) :
    relocatedCode(parRI, child),
    origAddr_(parRI->origAddr_),
    fromAddr_(parRI->fromAddr_),
    targetAddr_(parRI->targetAddr_),
    multiT(cMT),
    targetOverride_(parRI->targetOverride_)
{
    insn = parRI->insn->copy();
#if defined(arch_sparc)
    if (parRI->ds_insn)
        ds_insn = parRI->ds_insn;
    else
        ds_insn = NULL;
    if (parRI->agg_insn)
        agg_insn = parRI->agg_insn;
    else
        agg_insn = NULL;
#endif

}


generatedCodeObject *relocatedInstruction::replaceCode(generatedCodeObject *newParent) {
    // Since we are generated in-line, we return a copy
    // instead of ourselves.
    multiTramp *newMulti = dynamic_cast<multiTramp *>(newParent);
    assert(newMulti);

    relocatedInstruction *newInsn = new relocatedInstruction(this,
                                                             newMulti);
    return newInsn;
}

relocatedInstruction::~relocatedInstruction() {
    // We need to check if someone else grabbed these pointers... or reference
    // count. For now, _do not delete_ since someone else might have
    // grabbed the ptr
}

trampEnd::trampEnd(const trampEnd *parEnd,
                   multiTramp *cMT,
                   AddressSpace *child) :
    generatedCodeObject(parEnd, child),
    multi_(cMT),
    target_(parEnd->target_)
{}
    

// Can end up copying this; return a new one.
generatedCodeObject *trampEnd::replaceCode(generatedCodeObject *obj) {
    multiTramp *newMulti = dynamic_cast<multiTramp *>(obj);
    assert(newMulti);
    trampEnd *newTE = new trampEnd(newMulti, target_);
    newMulti->setTrampEnd(*newTE);
    return newTE;
}

std::string generatedCodeObject::getTypeString() const
{
  return "unknown";
}
std::string baseTrampInstance::getTypeString() const
{
  return "a base tramp";
}
std::string relocatedInstruction::getTypeString() const
{
  std::stringstream s;
  s << "relocated instruction from " << std::hex << fromAddr_;
  return s.str();
}
std::string trampEnd::getTypeString() const
{
  return "tramp end";
}


// And the code iterator

generatedCodeObject *generatedCFG_t::iterator::operator++(int) {
    // If we've hit the bottom of our current stack, pop back and take
    // the target route. Otherwise fall through
    if (!cur_) return NULL; // Ended, just spin.
    
    if (cur_->target_) stack_.push_back(cur_->target_);
    
    if (cur_->fallthrough_) {
        if (cur_->fallthrough_->previous_ != cur_) {
            fprintf(stderr, "ERROR: broken list: %p->%p->%p != %p\n",
                    cur_, cur_->fallthrough_,
                    cur_->fallthrough_->previous_,
                    cur_);
	    fprintf(stderr, "current is a %s\n", cur_->getTypeString().c_str());
	    fprintf(stderr, "previous is a %s\n", cur_->previous_->getTypeString().c_str());
            if (cur_->previous_) 
                fprintf(stderr, "Previous pointers: fallthrough %p, target %p\n",
                        cur_->previous_->fallthrough_,
                        cur_->previous_->target_);
	    fprintf(stderr, "next is a %s\n", cur_->fallthrough_->getTypeString().c_str());
	    fprintf(stderr, "next->previous is a %s\n", cur_->fallthrough_->previous_->getTypeString().c_str());
        }
        assert(cur_->fallthrough_->previous_ == cur_);
        cur_ = cur_->fallthrough_;
        return cur_->previous_;
    }
    else {
        generatedCodeObject *tmp;
        // See if there's anywhere else to go.
        tmp = cur_;
        if (stack_.size()) {
            cur_ = stack_.back();
            stack_.pop_back();
            return tmp;
        }
        else {
            cur_ = NULL;
            return tmp;
        }
    }
    return cur_;
}

generatedCodeObject *generatedCFG_t::iterator::operator*() {
    return cur_;
}

void generatedCFG_t::iterator::find(generatedCFG_t &cfg,
                                        generatedCodeObject *pointer) {
    // This sucks.... and is slow. Reason is we need to keep things synced.
    stack_.clear();
    cur_ = cfg.start_;

    generatedCodeObject *tmp = NULL;

    while (cur_ != pointer && cur_ != NULL)
        tmp = (*this)++;
    
    assert(cur_);
}

void generatedCFG_t::iterator::initialize(generatedCFG_t &cfg) {
    stack_.clear();
    cur_ = cfg.start_;
}


// We don't want duplicates in the CFG, so if obj == this ignore
generatedCodeObject *generatedCodeObject::setPrevious(generatedCodeObject *obj) {
    if (obj == this)
        return this;
    else {
        previous_ = obj;
        return obj;
    }
}

// We don't want duplicates in the CFG, so if obj == this ignore
generatedCodeObject *generatedCodeObject::setTarget(generatedCodeObject *obj) {
    if (obj == this)
        return this;
    else {
        target_ = obj;
        return obj;
    }
}

// We don't want duplicates in the CFG, so if obj == this ignore
generatedCodeObject *generatedCodeObject::setFallthrough(generatedCodeObject *obj) {
    if (obj == this)
        return this;
    else {
        fallthrough_ = obj;
        return obj;
    }
}

bool generatedCodeObject::alreadyGenerated(codeGen &gen,
                                           Address baseInMutatee) {
    if (generated_) {
        if (gen.currAddr(baseInMutatee) != addrInMutatee_) {
            fprintf(stderr, "ERROR: current address 0x%p != previous address 0x%p\n",
                    (void *)gen.currAddr(baseInMutatee), (void *)addrInMutatee_);
        }
        assert(gen.currAddr(baseInMutatee) == addrInMutatee_);
        assert(size_);
        gen.moveIndex(size_);
        return true;
    }
    return false;
}

bool generatedCodeObject::generateSetup(codeGen &gen,
                                        Address baseInMutatee) {
    addrInMutatee_ = gen.currAddr(baseInMutatee);
    return true;
}

bool generatedCodeObject::objIsChild(generatedCodeObject *obj) {
    assert(this != NULL);
    // Recursive descent. First, base case:
    if (fallthrough_ &&
        (obj == fallthrough_))
        return true;
    if (target_ &&
        (obj == target_))
        return true;
    if (fallthrough_ &&
        fallthrough_->objIsChild(obj))
        return true;
    if (target_ &&
        target_->objIsChild(obj))
        return true;
    return false;
}

bool multiTramp::catchupRequired(Address pc, miniTramp *newMT,
                                 bool active,
                                 codeRange *range) 
{
    // range is optional, and might be NULL. It's provided to avoid
    // having to do two billion codeRange lookups
    if (range == NULL)
        range = proc()->findOrigByAddr(pc);
    // Oopsie...
    if (!range) assert(0);

    if (BPatch::bpatch->isMergeTramp()) {
        // This is a shortcut. If we're doing mergeTramps, then we
        // know the miniTramp wasn't generated into this multitramp -
        // it couldn't have been. So we automatically need catchup.
        
        if (active) return true;
        // Otherwise: if we're at the starting address and not active,
        // then we're going to _return_ to this multiTramp - return false.
        assert(!active);
        if (pc == getAddress() ||
            pc == instAddr()) {
            // We're returning to this point...
            return false;
        }
        else
            return true;
    }

    multiTramp *rangeMulti = range->is_multitramp();
    miniTrampInstance *rangeMTI = range->is_minitramp();

    assert((rangeMulti != NULL) || (rangeMTI != NULL));

    if (rangeMTI) {

        catchup_printf("%s[%d]: in mini tramp...\n", FILE__, __LINE__);
        
        assert(rangeMTI->baseTI->multiT == this);
        
        // Check to see if we're at an equivalent miniTramp. If not, 
        // fall through
        if (rangeMTI->mini->instP() == newMT->instP()) {
            // This is easier
            catchup_printf("%s[%d]: mini tramp is for same instPoint, handing down\n",
                           FILE__, __LINE__);
            return miniTramp::catchupRequired(rangeMTI->mini, newMT, active);
        }
        else {
            rangeMulti = rangeMTI->baseTI->multiT;
            // And we need to fake the PC. Good thing
            // it's passed by value...
            // Pick the post address; doesn't really matter
            pc = rangeMTI->baseTI->trampPostAddr();
            catchup_printf("%s[%d]: mini tramp is for different instPoint, iterating with fake PC 0x%lx\n",
                           FILE__, __LINE__, pc);
        }
    }

    
    assert(rangeMulti == this);


    // We're not in miniTramps (or have faked a PC). However, we can't safely do
    // a linear comparison because the multiTramp is a CFG -- if we've done 
    // branch instrumentation, then we need to check that.
    // We also need to worry about being in the baseTrampInstance... the easiest thing
    // to do is run through the CFG iterator. We could also codeRange this once
    // generation has hit. 
    
    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;

    // This is probably overkill, but I want to get it right. Optimize later.
    generatedCodeObject *pcObj = NULL;
    // This is the baseTrampInstance for the new instrumentation.
    baseTrampInstance *newBTI = NULL;
    
    while ((obj = cfgIter++)) {
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);

        // Check to see if we're at the instPoint
        if (insn && 
            (pc >= insn->relocAddr()) &&
            (pc < insn->relocAddr() + insn->get_size())) {
            pcObj = insn;
        }
        else if (bti) {
            // Can be either the instPoint or the original instruction,
            // so check both
            if (bti->isInInstance(pc)) {
                pcObj = bti;
            }
            if (bti->baseT == newMT->baseT)
                newBTI = bti;
        }
        else if (end) {
            if ((end->addrInMutatee_ <= pc) &&
                (pc < (end->addrInMutatee_ + end->size_))) {
                pcObj = end;
            }
        }
        if ((newBTI != NULL) &&
            (pcObj != NULL))
            break;
    }

    assert(newBTI != NULL);
    assert(pcObj != NULL);

    if (newBTI == pcObj) {
        // Argh....
        // If we're in pre-insn, then return false. Post-insn, return true
        assert(pc >= newBTI->trampPreAddr());
        if (pc < (newBTI->trampPreAddr() + newBTI->saveEndOffset))
            return false;
        if (pc >= newBTI->trampPostAddr())
            return true;
        assert(0);
    }
    else {
        // So we return true if we've already passed the point. 
        // So start with the BTI, and return true if the pcObj is
        // the child
        return newBTI->objIsChild(pcObj);
    }

    assert(0);
    return false;
}

bool relocatedInstruction::generateCode(codeGen &gen,
                                        Address baseInMutatee,
                                        UNW_INFO_TYPE ** /* unwindInformation*/ ) 
{
  if( ! alreadyGenerated(gen, baseInMutatee) ) {
    generateSetup(gen, baseInMutatee);

    // addrInMutatee_ == base for this insn
    toAddressPatch orig_target(0);
    patchTarget *target;
    if (!targetOverride_) {
       orig_target.set_address(originalTarget());
       target = &orig_target;
    }
    else {
       target = targetOverride_;
    }

    if (!insnCodeGen::generate(gen,
                        *insn,
                        multiT->proc(),
                        origAddr_,
                        addrInMutatee_,
                        NULL, // fallthrough is not overridden
                        target))
    {
       // We use the override if present, otherwise the original target (which may be
       // overridden in function relocation....)
       fprintf(stderr, "WARNING: returned false from relocate insn "
               "(orig at 0x%lx, from 0x%lx, now 0x%lx)\n", 
               origAddr_, fromAddr_, addrInMutatee_);
       return false;
    }

#if defined(arch_sparc) 
    // We pin delay instructions.
    if (insn->isDCTI()) {
      if (ds_insn) {
        inst_printf("... copying delay slot\n");
        insnCodeGen::generate(gen,*ds_insn);
      }
      if (agg_insn) {
        inst_printf("... copying aggregate\n");
        insnCodeGen::generate(gen,*agg_insn);
      }
    }
#endif

    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;
    hasChanged_ = false;
  } /* end code generation */
    
#if defined( cap_unwind )
  /* FIXME: a relocated instruction could easily change the unwind state.
     IA64-specific: can we ALIAS into the middle of bundles?
     Generally, can a relocated instruction tell how far into a basic block (bundle) it is? */
  dyn_unw_printf( "%s[%d]: aliasing relocated instruction to 0x%lx\n", __FILE__, __LINE__, multiT->instAddr() );
  unw_dyn_region_info_t * aliasRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
  assert( aliasRegion != NULL );
  aliasRegion->insn_count = 0;
  aliasRegion->op_count = 2;
	
  _U_dyn_op_alias( & aliasRegion->op[0], _U_QP_TRUE, -1, multiT->instAddr() );
  _U_dyn_op_stop( & aliasRegion->op[1] );
     
  unw_dyn_region_info_t * relocatedRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 1 ) );
  assert( relocatedRegion != NULL );
    
  relocatedRegion->op_count = 1;
  _U_dyn_op_stop( & relocatedRegion->op[0] );
		
  /* size_ is in bytes. */
#if defined( arch_ia64 )
  relocatedRegion->insn_count = (size_ / 16) * 3;
#else 	
#error How do I know how many instructions are in the jump region?
#endif /* defined( arch_ia64 ) */

  /* The care and feeding of pointers. */
  unw_dyn_region_info_t * prevRegion = * unwindInformation;
  prevRegion->next = aliasRegion;
  aliasRegion->next = relocatedRegion;
  relocatedRegion->next = NULL;
  * unwindInformation = relocatedRegion;
#endif /* defined( cap_unwind ) */
  return true;
}

bool multiTramp::fillJumpBuf(codeGen &gen) {

    // We play a cute trick with the rest of the overwritten space: fill it with
    // traps. If one is hit, we can transfer the PC into the multiTramp without
    // further problems. Cute, eh?
    if (proc()->canUseTraps()) {
        while (gen.used() < instSize()) {
            Address origAddr = gen.currAddr(instAddr_);
            Address addrInMulti = uninstToInstAddr(origAddr);
            if (addrInMulti) {
               // addrInMulti may be 0 if our trap instruction does not
               // map onto a real instruction
               proc()->trapMapping.addTrapMapping(origAddr, addrInMulti, false);
            }
            insnCodeGen::generateTrap(gen);
        }
    }
    else {
        // Don't want to use traps, but we still need to fill
        // this up. So instead we use noops. 
        insnCodeGen::generateNOOP(gen, instSize() - gen.used());
    }
    return true;
}


/* Generate a jump to a base tramp. Return the size of the instruction
   generated at the instrumentation point. */

// insn: buffer to generate code

// Again, IA64 has its own version. TODO: can it use the same mechanism we use here?
bool multiTramp::generateBranchToTramp(codeGen &gen)
{
    assert(instAddr_);
    assert(trampAddr_);
    unsigned origUsed = gen.used();

    // TODO: we can use shorter branches, ya know.
    unsigned long jumpSizeNeeded = instruction::jumpSize(instAddr_, trampAddr_,
                                                         proc()->getAddressWidth());

    if (instSize_ <  jumpSizeNeeded) { // jumpSizeNeeded > 0...
        branchSize_ = jumpSizeNeeded;
        // Return value: do we continue making the tramp?
        // Yes... we'll just use a trap later
        return true;
    }
#if defined(arch_sparc)
    int dist = (trampAddr_ - instAddr_);
    if (!instruction::offsetWithinRangeOfBranchInsn(dist) &&
        func()->is_o7_live()) {
        branchSize_ = (unsigned) -1;
        return true;
    }
#endif
    insnCodeGen::generateBranch(gen, instAddr_, trampAddr_);

    branchSize_ = gen.used() - origUsed;

    return true;
}

bool multiTramp::generateTrapToTramp(codeGen &gen) {
    if (!proc()->canUseTraps())  {
       inst_printf("%s[%d]: process cannot use traps, attempting to use trap, ret false\n", FILE__, __LINE__);
        return false;
    }

    // We're doing a trap. Now, we know that trap addrs are reported
    // as "finished" address... so use that one (not instAddr_)
    proc()->trapMapping.addTrapMapping(gen.currAddr(instAddr_), trampAddr_, true);
    unsigned start = gen.used();
    insnCodeGen::generateTrap(gen);
    branchSize_ = gen.used() - start;

    usedTrap_ = true;

    inst_printf("TRAPPING TO TRAMP AT 0x%lx (%d bytes)\n", instAddr_, instSize_);

    return true;
}

void *multiTramp::getPtrToInstruction(Address addr) const {
    if (!installed_) return NULL;
    if (addr < trampAddr_) return NULL;
    if (addr >= (trampAddr_ + trampSize_)) return NULL;

    addr -= trampAddr_;
    assert(generatedMultiT_ != NULL);
    return generatedMultiT_.get_ptr(addr);
}

unsigned multiTramp::maxSizeRequired() {
    // A jump to the multiTramp, 
    return instruction::maxJumpSize(proc()->getAddressWidth());
}

relocatedInstruction::relocatedInstruction(instruction *i,
                      Address o, Address f, Address t,
                      multiTramp *m) :
   relocatedCode(),
   insn(i),
#if defined(arch_sparc)
   ds_insn(NULL),
   agg_insn(NULL),
#endif
   origAddr_(o), 
   fromAddr_(f), 
   targetAddr_(t),
   multiT(m), 
   targetOverride_(NULL) 
{
}

relocatedInstruction::relocatedInstruction(relocatedInstruction *prev,
                                           multiTramp *m) :
   relocatedCode(),
   insn(prev->insn),
#if defined(arch_sparc)
   ds_insn(prev->ds_insn),
   agg_insn(prev->agg_insn),
#endif
   origAddr_(prev->origAddr_), fromAddr_(prev->fromAddr_),
   targetAddr_(prev->targetAddr_),
   multiT(m),
   targetOverride_(prev->targetOverride_) 
{
}
#if defined(cap_instruction_api)
relocatedInstruction::relocatedInstruction(const unsigned char* insnPtr, Address o, Address f, Address t,
                      multiTramp *m) :
   relocatedCode(),
#if defined(arch_sparc)
   ds_insn(NULL),
   agg_insn(NULL),
#endif
   origAddr_(o), 
   fromAddr_(f), 
   targetAddr_(t),
   multiT(m), 
   targetOverride_(NULL) 
{
  const codeBuf_t *buf = reinterpret_cast<const codeBuf_t*>(insnPtr);
  insn = new instruction;
  insn->setInstruction(const_cast<codeBuf_t*>(buf), (Address)(o));
}
#endif

replacedInstruction::replacedInstruction(const relocatedInstruction *i,
                                         AstNodePtr ast,
                                         instPoint *p, 
                                         multiTramp *m) :
   relocatedCode(),
   oldInsn_(i),
   ast_(ast),
   point_(p),
   multiT_(m) 
{
}

replacedInstruction::replacedInstruction(replacedInstruction *prev,
                                         multiTramp *m) :
   relocatedCode(),
   oldInsn_(prev->oldInsn_),
   ast_(prev->ast_),
   point_(prev->point_),
   multiT_(m) 
{
}

generatedCodeObject *generatedCodeObject::nextObj()
{
   if (fallthrough_)
      return fallthrough_;
   return target_;
}

void trampEnd::changeTarget(Address newTarg) 
{ 
    target_ = newTarg; 
}

void multiTramp::setTrampEnd(trampEnd &newTramp)
{
    trampEnd_ = &newTramp;
}


void multiTramp::updateTrampEnd(instPoint *point)
{
    int_basicBlock * fallThroughB = point->block()->getFallthrough();
    if ( ! fallThroughB ) {
        return; // don't need to make any changes
    }
    assert( trampEnd_ ); // should exist if there's a fallthroughBlock

    Address endTarget = trampEnd_->target(); // current target

    // figure out the right target
    bblInstance * fallThroughBBI = fallThroughB->instVer( 
        fallThroughB->numInstances() -1 );
    if ( endTarget == fallThroughBBI->firstInsnAddr() ) {
        return; // no changes necessary
    }
    mal_printf("%s[%d] trampEnd at %lx in multi [%lx %lx]: target is %lx will "
            "point it to block originally at %lx"
            ", but now at %lx\n", FILE__,__LINE__,trampEnd_->get_address(), 
            addrInMutatee_, 
            addrInMutatee_ + get_size(), endTarget, 
            fallThroughB->origInstance()->firstInsnAddr(), 
            fallThroughBBI->firstInsnAddr() );

    // trigger new code generation just for the trampEnd
    trampEnd_->changeTarget( fallThroughBBI->firstInsnAddr() );
    codeGen endGen(getAddress() + get_size() - trampEnd_->get_address());
    trampEnd_->generateCode(endGen, trampEnd_->get_address(), NULL);
    assert ( endGen.used() <= trampEnd_->get_size() );

    // copy the newly generated code to the mutatee
    proc()->writeTextSpace((void*)trampEnd_->get_address(), 
                            trampEnd_->get_size(), 
                            endGen.start_ptr());
}

void multiTramp::setIsActive(bool value) 
{ 
    if (value != isActive_) {
        mal_printf("changing isactive to %d for multiTramp %lx [%lx %lx]\n",
                value, instAddr_, trampAddr_, trampAddr_+trampSize_);
    }
    isActive_ = value; 
}
