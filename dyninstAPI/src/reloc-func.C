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
 
// $Id: reloc-func.C,v 1.4 2005/09/28 17:03:17 bernat Exp $

// We'll also try to limit this to relocation-capable platforms
// in the Makefile. Just in case, though....

#include "common/h/Types.h"
#include "function.h"
#include "reloc-func.h"
#include "process.h"
#include "showerror.h"
#include "codeRange.h"
#include "instPoint.h"
#include "multiTramp.h"
#include "InstrucIter.h"

class int_basicBlock;
class instruction;

#if defined(cap_relocation)


// And happy fun time. Relocate a function: make a physical copy somewhere else
// in the address space that will execute as the original function did. 
// This creates a somewhat inefficient new version; relocation is done
// for correctness, not efficiency.

// Input: a list of things to change when we relocate the function.

// TODO: which version do we relocate? ;)


bool int_function::relocationGenerate(pdvector<funcMod *> &mods, 
                                      int sourceVersion /* = 0 */) {
    unsigned i;

    assert(sourceVersion <= version_);
    if (generatedVersion_ > version_) {
        // Odd case... we generated, but never installed or
        // linked. Nuke the "dangling" version.
        relocationInvalidate();
    }

    generatedVersion_++;

    reloc_printf("Relocating function %s, version %d, 0x%lx to 0x%lx\n",
                 symTabName().c_str(), sourceVersion,
                 getAddress(), getSize_NP());
    // Make sure the blocklist is created.
    blocks(); 

    // Make the basic block instances; they're placeholders for now.
    pdvector<bblInstance *> newInstances;
    for (i = 0; i < blockList.size(); i++) {
        reloc_printf("Block %d, creating instance...", i);
        bblInstance *newInstance = new bblInstance(blockList[i], generatedVersion_);
        assert(newInstance);
        newInstances.push_back(newInstance);
        blockList[i]->instances_.push_back(newInstance);
        reloc_printf("and added to basic block\n");
    }
    assert(newInstances.size() == blockList.size());

    // Whip through them and let people play with the sizes.
    // We can also keep a tally of how much space we'll need while
    // we're at it...
    unsigned size_required = 0;
    for (i = 0; i < newInstances.size(); i++) {
        reloc_printf("Calling relocationSetup on block %d...\n",
                     i);
        reloc_printf("Calling newInst:relocationSetup(%d)\n",
                     sourceVersion);
        newInstances[i]->relocationSetup(blockList[i]->instVer(sourceVersion),
                                         mods);
        size_required += newInstances[i]->sizeRequired();
        reloc_printf("After block %d, %d bytes required\n",
                     i, size_required);
    }

    // AIX: we try to target the data heap, since it's near instrumentation; we can
    // do big branches at the start of a function, but not within. So amusingly, function
    // relocation probably won't _enlarge_ the function, just pick it up and move it nearer
    // instrumentation. Bring the mountain to Mohammed, I guess.
#if defined(os_aix)
    Address baseInMutatee = proc()->inferiorMalloc(size_required, dataHeap);
#else
    // We're expandin'
    Address baseInMutatee = proc()->inferiorMalloc(size_required);
#endif

    if (!baseInMutatee) return false;
    reloc_printf("... new version at 0x%lx in mutatee\n", baseInMutatee);

    Address currAddr = baseInMutatee;
    // Inefficiency, part 1: we pin each block at a particular address
    // so that we can one-pass generate and get jumps done correctly.
    for (i = 0; i < newInstances.size(); i++) {
        reloc_printf("Pinning block %d to 0x%lx\n", i, currAddr);
        newInstances[i]->setStartAddr(currAddr);
        currAddr += newInstances[i]->sizeRequired();
    }

    // Okay, so we have a set of "new" basicBlocks. Now go through and
    // generate code for each; we can do branches appropriately, since
    // we know where the targets will be.
    // This builds the codeGen member of the bblInstance
    bool success = true;
    for (i = 0; i < newInstances.size(); i++) {
        reloc_printf("... relocating block %d\n", blockList[i]->id());
        success &= newInstances[i]->generate();
        if (!success) break;
    }

    if (!success) {
        relocationInvalidate();
        return false;
    }

    // We use basicBlocks as labels.
    for (i = 0; i < blockList.size(); i++) {
        if (!blockList[i]->isEntryBlock()) continue;
        functionReplacement *funcRep = new functionReplacement(blockList[i], 
                                                               blockList[i],
                                                               sourceVersion,
                                                               generatedVersion_);
        if (funcRep->generateFuncRep())
            blockList[i]->instVer(generatedVersion_)->jumpToBlock = funcRep;
        else
            success = false;
    }
        
    return success;
}

bool int_function::relocationInstall() {
    // Okay, we now have a new copy of the function. Go through 
    // the version to be replaced, and replace each basic block
    // with a "jump to new basic block" combo.
    // If we overlap a bbl (which we probably will), oops.
    unsigned i;

    if (installedVersion_ == generatedVersion_)
        return true; // Nothing to do here...

    bool success = true;
    for (i = 0; i < blockList.size(); i++) {
        success &= blockList[i]->instVer(generatedVersion_)->install();
        if (!success) break;
        
        // Add all the basicBlocks to the process data range...
        proc()->addCodeRange(blockList[i]->instVer(generatedVersion_));
        addBBLInstance(blockList[i]->instVer(generatedVersion_));
    }
    if (!success) {
        fprintf(stderr, "Warning: installation of relocated function failed\n");
        return false;
    }

    installedVersion_ = generatedVersion_;
    version_ = installedVersion_;

    // Fix up all of our instPoints....
    // This will cause multiTramps, etc. to be built in the new
    // version of the function.  
    for (i = 0; i < entryPoints_.size(); i++)
        entryPoints_[i]->updateInstances();
    for (i = 0; i < exitPoints_.size(); i++)
        exitPoints_[i]->updateInstances();
    for (i = 0; i < callPoints_.size(); i++)
        callPoints_[i]->updateInstances();
    for (i = 0; i < arbitraryPoints_.size(); i++)
        arbitraryPoints_[i]->updateInstances();

    return success;
}

bool int_function::relocationCheck(pdvector<Address> &checkPCs) {
    unsigned i;

    assert(generatedVersion_ == installedVersion_);
    if (installedVersion_ == installedVersion_)
        return true;
    for (i = 0; i < blockList.size(); i++) {
        if (!blockList[i]->instVer(installedVersion_)->check(checkPCs))
            return false;
    }
    return true;
}
        

bool int_function::relocationLink(pdvector<codeRange *> &overwritten_objs) {
    unsigned i;

    if (linkedVersion_ == installedVersion_) {
        assert(linkedVersion_ == version_);
        return true; // We're already done...
    }

    // If the assert fails, then we linked but did not
    // update the global function version. That's _BAD_.

    bool success = true;
    for (i = 0; i < blockList.size(); i++) {
        success &= blockList[i]->instVer(installedVersion_)->link(overwritten_objs);
        if (!success)
            break;
    }
    if (!success) {
        // Uh oh...
        fprintf(stderr, "ERROR: linking relocated function failed!\n");
        assert(0);
    }

    linkedVersion_ = installedVersion_;
    assert(linkedVersion_ == version_);

    return true;
}

bool int_function::relocationInvalidate() {
    unsigned i;
    // The increase pattern goes like so:
    // generatedVersion_++;
    // installedVersion_++;
    // version_++; -- so that instpoints will be updated
    // linkedVersion_++;
    if (linkedVersion_ == version_) return true;
    
    while (installedVersion_ > linkedVersion_) {
        reloc_printf("******* Removing installed version %d\n",
                     installedVersion_);
        for (i = 0; i < blockList.size(); i++) {
            bblInstance *instance = blockList[i]->instVer(installedVersion_);
            assert(instance);
            proc()->deleteCodeRange(instance->firstInsnAddr());
            deleteBBLInstance(instance);
            // Nuke any attached multiTramps...
            multiTramp *multi = proc()->findMultiTramp(instance->firstInsnAddr());
            if (multi)
                delete multi;
        }
        installedVersion_--;
    }
    
    while (generatedVersion_ > installedVersion_) {
        reloc_printf("******* Removing generated version %d\n",
                     generatedVersion_);
        proc()->inferiorFree(blockList[0]->instVer(generatedVersion_)->firstInsnAddr());
        for (i = 0; i < blockList.size(); i++) {
            blockList[i]->removeVersion(generatedVersion_);
        }
        generatedVersion_--;
    }
    version_ = linkedVersion_;

    for (i = 0; i < entryPoints_.size(); i++)
        entryPoints_[i]->updateInstances();
    for (i = 0; i < exitPoints_.size(); i++)
        exitPoints_[i]->updateInstances();
    for (i = 0; i < callPoints_.size(); i++)
        callPoints_[i]->updateInstances();
    for (i = 0; i < arbitraryPoints_.size(); i++)
        arbitraryPoints_[i]->updateInstances();

    return true;
}

bool int_function::expandForInstrumentation() {
    unsigned i;
    // Take the most recent version of the function, check the instPoints
    // registered. If one needs more room, create an expansion record.
    // When we're done, relocate the function (most recent version only).

    // Oh, only do that if there's instrumentation added at the point?
    reloc_printf("Function expandForInstrumentation, version %d\n",
                 version_);
    // Right now I'm basing everything off version 0; that is, if we
    // relocate multiple times, we will have discarded versions instead
    // of a long chain. 

    if (!canBeRelocated_) {
        //fprintf(stderr, "Skipping relocation of function %s: has non-reloc constructs\n",
        //symTabName().c_str());
        return false;
    }

    for (i = 0; i < blockList.size(); i++) {
        bblInstance *bblI = blockList[i]->origInstance();
        assert(bblI->block() == blockList[i]);
        // Simplification: check if there's a multiTramp at the block.
        // If there isn't, then we don't care.
        multiTramp *multi = proc()->findMultiTramp(bblI->firstInsnAddr());
        if (!multi) continue;
        if (bblI->getSize() < multi->sizeDesired()) {
            reloc_printf("Enlarging basic block %d\n",
                         i);
            enlargeBlock *mod = new enlargeBlock(bblI->block(), multi->maxSizeRequired());
            enlargeMods_.push_back(mod);
        }
    }
    return true;
}

#if 0
bool foo() {
    // Left here for reference: we install and link as part of the 
    // instrumentation process, which allows us to tag along on the
    // stackwalk-based check. 

    if (mods.size() == 0)
        return true;

    reloc_printf("Calling relocateAndReplace, version to use %d\n",
                 0);
    
    // We work off version 0 for simplicity...
    if (!relocationGenerate(mods, 0)) {
        fprintf(stderr, "Failed relocationGenerate, ret false\n");
        return false;
    }

    if (!relocationInstall()) {
        fprintf(stderr, "Failed relocationInstall, ret false\n");
        return false;
    }
    // CHECK THE STACK!!!
    pdvector<codeRange *> overwritten_objs;
    if (!relocationLink(0, overwritten_objs)) {
        fprintf(stderr, "Failed relocationLink, ret false\n");
        return false;
    }
    
    // We still need to do something about the overwritten_objs
    // list...

    return true;
}
#endif

// Return the absolute maximum size required to relocate this
// block somewhere else. If this looks very familiar, well, 
// _it is_. We should unify the instruction and relocatedInstruction
// classes.
// This is a bit inefficient, since we rapidly use and delete
// relocatedInstructions... ah, well :)
unsigned bblInstance::sizeRequired() {
    assert(maxSize_);
    assert(insns_.size());
    return maxSize_;
}


// Make a copy of the basic block (from the original provided),
// applying the modifications stated in the vector of funcMod
// objects.

bool bblInstance::relocationSetup(bblInstance *orig, pdvector<funcMod *> &mods) {
    unsigned i;

    origInstance_ = orig;
    assert(origInstance_);
    // First, build the insns vector
    insns_.clear();
    // Keep a running count of how big things are...
    maxSize_ = 0;
    InstrucIter insnIter(orig);
    while (insnIter.hasMore()) {
        instruction *insnPtr = insnIter.getInsnPtr();
        assert(insnPtr);
        insns_.push_back(insnPtr);
        maxSize_ += insnPtr->spaceToRelocate();
        insnIter++;
    }

    // Apply any hanging-around relocations from our previous instance
    for (i = 0; i < orig->appliedMods_.size(); i++) {
        if (orig->appliedMods_[i]->modifyBBL(block_, insns_, maxSize_)) {
            appliedMods_.push_back(orig->appliedMods_[i]);
        }
    }

    // So now we have a rough size and a list of insns. See if any of
    // those mods want to play.
    for (i = 0; i < mods.size(); i++) {
        if (mods[i]->modifyBBL(block_, insns_, maxSize_)) {
            // Store for possible further relocations.
            appliedMods_.push_back(mods[i]);
        }
    }
    return true;
}

void bblInstance::setStartAddr(Address addr) {
    if (addr) {
        // No implicit overriding - set it to 0 first.
        assert(firstInsnAddr_ == 0);
        firstInsnAddr_ = addr;
    }
    else {
        firstInsnAddr_ = 0;
    }
}

bool bblInstance::generate() {
    assert(firstInsnAddr_);
    assert(insns_.size());
    assert(maxSize_);
    assert(block_);
    assert(origInstance_);
    unsigned i;

    generatedBlock_.allocate(maxSize_);
    fprintf(stderr, "Block ptr at %p\n", generatedBlock_.start_ptr()); 
    Address origAddr = origInstance_->firstInsnAddr();
    for (i = 0; i < insns_.size(); i++) {
        Address currAddr = generatedBlock_.currAddr(firstInsnAddr_);
        Address fallthroughOverride = 0;
        Address targetOverride = 0;
        if (i == (insns_.size()-1)) {
            // Check to see if we need to fix up the target....
            pdvector<int_basicBlock *> targets;
            block_->getTargets(targets);
            if (targets.size() > 2) {
                // Multiple jump... we can't handle this yet
                fprintf(stderr, "ERROR: attempt to relocate function %s with indirect jump!\n",
                        block_->func()->symTabName().c_str());
                       
                return false;
            }
            // So we have zero, one, or two targets; one of them
            // may be a jump target as opposed to a fallthrough.
            // Find it and set targetOverride to the start of that
            // basic block. We find the target by a guy who is not
            // contiguous with us... would be much easier to label
            // edges
            for (unsigned j = 0; j < targets.size(); j++) {
                bblInstance *targetInst = targets[j]->instVer(origInstance_->version_);
                assert(targetInst);
                if (targetInst->firstInsnAddr() != origInstance_->endAddr()) {
                    // This is a jump target; get the start addr for the
                    // new block.
                    assert(targetOverride == 0);
                    targetOverride = targets[j]->instVer(version_)->firstInsnAddr();
                }
            }
        }
        reloc_printf("... generating insn %d, orig addr 0x%lx, new addr 0x%lx, fallthrough 0x%lx, target 0x%lx\n",
                     i, origAddr, currAddr, fallthroughOverride, targetOverride);
        insns_[i]->generate(generatedBlock_,
                            proc(),
                            origAddr,
                            currAddr,
                            fallthroughOverride,
                            targetOverride); // targetOverride

        // And set the remaining bbl variables correctly
        // This may be overwritten multiple times, but will end
        // correct.
        lastInsnAddr_ = currAddr;

        changedAddrs_[origAddr] = currAddr;
        origAddr += insns_[i]->size();
    }
    generatedBlock_.fillRemaining(codeGen::cgNOP);

    blockEndAddr_ = firstInsnAddr_ + maxSize_;
    
    // Post conditions
    assert(firstInsnAddr_);
    assert(lastInsnAddr_);
    assert(blockEndAddr_);
    
    return true;
}

bool bblInstance::install() {
    assert(firstInsnAddr_);
    assert(generatedBlock_ != NULL);
    assert(maxSize_);
    if (maxSize_ != generatedBlock_.used()) {
        fprintf(stderr, "ERROR: max size of block is %d, but %d used!\n",
                maxSize_, generatedBlock_.used());
    }
    assert(generatedBlock_.used() == maxSize_);
    
    bool success = proc()->writeTextSpace((void *)firstInsnAddr_,
                                          generatedBlock_.used(),
                                          generatedBlock_.start_ptr());
    if (success) {
        return true;
    }
    else 
        return false;
}

bool bblInstance::check(pdvector<Address> &checkPCs) {
    if (!jumpToBlock) return true;
    return jumpToBlock->checkFuncRep(checkPCs);
}

bool bblInstance::link(pdvector<codeRange *> &overwrittenObjs) {
    if (!jumpToBlock) return true;
    return jumpToBlock->linkFuncRep(overwrittenObjs);
}


#endif // cap_relocation

functionReplacement::functionReplacement(int_basicBlock *sourceBlock,
                                         int_basicBlock *targetBlock,
                                         unsigned sourceVersion /* =0 */,
                                         unsigned targetVersion /* =0 */) :
    sourceBlock_(sourceBlock),
    targetBlock_(targetBlock),
    sourceVersion_(sourceVersion),
    targetVersion_(targetVersion),
    overwritesMultipleBlocks_(false) 
{}
    
Address functionReplacement::get_address_cr() const {
    assert(sourceBlock_);
    return sourceBlock_->instVer(sourceVersion_)->firstInsnAddr();
}

unsigned functionReplacement::get_size_cr() const {
    if (jumpToRelocated != NULL)
        return jumpToRelocated.used();
    else
        return 0;
}

bool functionReplacement::generateFuncRep() {
    assert(sourceBlock_);
    assert(targetBlock_);
    assert(jumpToRelocated == NULL);

#if !defined(cap_relocation)
    assert(sourceVersion_ == 0);
    assert(targetVersion_ == 0);
#endif

    // TODO: if check modules and do ToC if not the same one.

    bblInstance *sourceInst = sourceBlock_->instVer(sourceVersion_);
    assert(sourceInst);
    bblInstance *targetInst = targetBlock_->instVer(targetVersion_);
    assert(targetInst);

    Address sourceAddr = sourceInst->firstInsnAddr();
    Address targetAddr = targetInst->firstInsnAddr();

    jumpToRelocated.allocate(instruction::maxInterFunctionJumpSize());
    fprintf(stderr, "******* generating interFunctionJump...\n");
    instruction::generateInterFunctionBranch(jumpToRelocated,
                                             sourceInst->firstInsnAddr(),
                                             targetInst->firstInsnAddr());

    if (jumpToRelocated.used() > sourceInst->getSize()) {
        // Okay, things are going to get ugly. There are two things we
        // can't do:
        // 1) Overwrite another entry point
        // 2) Overwrite a different function
        // So start seeing where this jump is going to run into...
        unsigned overflow = jumpToRelocated.used() - sourceInst->getSize();
        Address currAddr = sourceInst->endAddr();
        bool safe = true;
        while (overflow > 0) {
            bblInstance *curInst = sourceBlock_->func()->findBlockInstanceByAddr(currAddr);
            if (curInst) {
                // Okay, we've got another block in the function. Check to see
                // if it's an entry point.
                if (curInst->block()->isEntryBlock()) {
                    safe = false;
                    break;
                }
                // Otherwise keep going
                // Inefficient...
                currAddr = curInst->endAddr();
                if (curInst->getSize() > overflow)
                    overflow = 0;
                else
                    overflow -= curInst->getSize();
            }
            else {
                // Ummm... see if anyone else claimed this space.
                int_function *func = sourceBlock_->proc()->findFuncByAddr(currAddr);
                if (func) {
                    // Consistency check...
                    assert(func != sourceBlock_->func());
                    safe = false;
                    break;
                }
                else {
                    // Ummm... empty space, but what about the next byte? 
                    // We need a better overlap check.
                    // For now, guess that we have enough room.
                    overflow = 0;
                    fprintf(stderr, "Warning: function replacement going into unclaimed space, possibly dangerous...\n");
                }
            }
        }
        if (!safe) {
            fprintf(stderr, "Warning: unsafe function replacement\n");
        }
        overwritesMultipleBlocks_ = true;
    }
    return true;
}

bool functionReplacement::installFuncRep() {
  // Nothing to do here unless we go to a springboard model.
return true;
}

// TODO: jumps that overwrite multiple basic blocks...
bool functionReplacement::checkFuncRep(pdvector<Address> &checkPCs) {
    unsigned i;

    Address start = get_address_cr();
    Address end = get_address_cr() + get_size_cr();
    for (i = 0; i < checkPCs.size(); i++) {
        if ((checkPCs[i] > start) &&
            (checkPCs[i] < end))
            return false;
    }
    return true;
}

bool functionReplacement::linkFuncRep(pdvector<codeRange *> &overwrittenObjs) {
    fprintf(stderr, "Linking function replacement......\n");
    if (sourceBlock_->proc()->writeTextSpace((void *)get_address_cr(),
                                             jumpToRelocated.used(),
                                             jumpToRelocated.start_ptr())) {
        sourceBlock_->proc()->addFunctionReplacement(this,
                                       overwrittenObjs);
        return true;
    }
    else
        return false;
}

bool enlargeBlock::modifyBBL(int_basicBlock *block,
                             pdvector<instruction *> &,
                             unsigned &size)
{
    if (block == targetBlock_) {
        if (size < targetSize_) {
            size = targetSize_;
        }
        return true;
    }
    return false;
}
