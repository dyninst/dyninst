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

// $Id: baseTramp.C,v 1.3 2005/08/03 23:00:51 bernat Exp $

#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/process.h"

#if defined(os_aix)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

// Normal constructor
baseTrampInstance::baseTrampInstance(baseTramp *tramp,
                                     multiTramp *multi) :    
    generatedCodeObject(),
    trampAddr_(0), // Unallocated
    baseT(tramp),
    multiT(multi),
    genVersion(0),
    hadMini(false)
{
}

// Fork constructor
baseTrampInstance::baseTrampInstance(const baseTrampInstance *parBTI,
                                     baseTramp *cBT,
                                     multiTramp *cMT,
                                     process *child) :
    generatedCodeObject(parBTI, child),
    trampAddr_(parBTI->trampAddr_),
    trampPostOffset(parBTI->trampPostOffset),
    baseT(cBT),
    multiT(cMT),
    genVersion(parBTI->genVersion),
    hadMini(parBTI->hadMini)
{
    // Register with parent
    cBT->instances.push_back(this);

    // And copy miniTrampInstances
    for (unsigned i = 0; i < parBTI->mtis.size(); i++) {
        miniTramp *cMini = NULL;
        getInheritedMiniTramp(parBTI->mtis[i]->mini,
                              cMini,
                              child);
        assert(cMini);
        miniTrampInstance *newMTI = new miniTrampInstance(parBTI->mtis[i],
                                                          this,
                                                          cMini,
                                                          child);
        mtis.push_back(newMTI);
    }
}

baseTrampInstance::~baseTrampInstance() {
    // What about mtis?
    for (unsigned i = 0; i < mtis.size(); i++)
        delete mtis[i];
}

void baseTramp::unregisterInstance(baseTrampInstance *inst) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i] == inst) {
            instances[i] = instances.back();
            instances.pop_back();
            return;
        }
    }
}

#define PRE_TRAMP_SIZE 4096
#define POST_TRAMP_SIZE 4096

baseTramp::baseTramp() :
    preSize(0),
    postSize(0),
    saveStartOffset(0),
    saveEndOffset(0),
    guardLoadOffset(0),
    guardBranchSize(0),
    costUpdateOffset(0),
    costSize(0),
    instStartOffset(0),
    instSize(0),
    restoreStartOffset(0),
    restoreEndOffset(0),
    trampEnd(0),
    guardBranchIndex(0),
    costValueOffset(0),
    guardTargetIndex(0),
    cost(0),
    costAddr(0),
    clobberedGPR(NULL),
    clobberedFPR(NULL),
    totalClobbered(0),
    trampGuardFlagAddr(0),
    trampGuardFlagValue(0),
#if 0
    saveRegion(NULL),
    restoreRegion(NULL),
    codeRegion(NULL),
#endif
    preInstP(NULL),
    postInstP(NULL),
    rpcMgr_(NULL),
    isMerged(false),
    firstMini(NULL),
    lastMini(NULL),
    preTrampCode_(),
    postTrampCode_(),
    valid(false),
    guardState_(unset_BTR),
    instVersion_()
{
}

baseTramp::baseTramp(const baseTramp *pt, process *proc) :
    preSize(pt->preSize),
    postSize(pt->postSize),
    saveStartOffset(pt->saveStartOffset),
    saveEndOffset(pt->saveEndOffset),
    guardLoadOffset(pt->guardLoadOffset),
    guardBranchSize(pt->guardBranchSize),
    costUpdateOffset(pt->costUpdateOffset),
    costSize(pt->costSize),
    instStartOffset(pt->instStartOffset),
    instSize(pt->instSize),
    restoreStartOffset(pt->restoreStartOffset),
    restoreEndOffset(pt->restoreEndOffset),
    trampEnd(pt->trampEnd),
    guardBranchIndex(pt->guardBranchIndex),
    costValueOffset(pt->costValueOffset),
    guardTargetIndex(pt->guardTargetIndex),
    cost(pt->cost),
    costAddr(pt->costAddr),
    clobberedGPR(NULL),
    clobberedFPR(NULL),
    totalClobbered(pt->totalClobbered),
    trampGuardFlagAddr(pt->trampGuardFlagAddr),
    trampGuardFlagValue(pt->trampGuardFlagValue),
#if 0
    saveRegion(NULL),
    restoreRegion(NULL),
    codeRegion(NULL),
#endif
    preInstP(NULL),
    postInstP(NULL),
    firstMini(NULL),
    lastMini(NULL),
    preTrampCode_(pt->preTrampCode_),
    postTrampCode_(pt->postTrampCode_),
    valid(pt->valid),
    guardState_(pt->guardState_),
    instVersion_(pt->instVersion_)
{

#if 0
    if (pt->saveRegion)
        assert(0); // Need to copy these
#endif
    if (pt->clobberedGPR)
        assert(0); // Don't know how to copy these
    if (pt->clobberedFPR)
        assert(0);

    // And copy minis
    miniTramp *parMini = NULL;
    miniTramp *childPrev = NULL;
    miniTramp *childMini = NULL;
    
    parMini = pt->firstMini;
    while (parMini) {
        childMini = new miniTramp(parMini, this, proc);
        if (childPrev) {
            childPrev->next = childMini;
        }
        else {
            firstMini = childMini;
        }
        childMini->prev = childPrev;
        childPrev = childMini;
        parMini = parMini->next;
    }
    lastMini = childMini;
}

baseTramp::~baseTramp() {
    // Delete clobberedGPR and clobberedFPR?
    // Dunno
}

unsigned baseTrampInstance::get_size_cr() const { 
    // Odd... especially if there are minitramps in the middle. 
    // For now return a+b; eventually we'll need a codeRange
    // that can handle noncontiguous ranges.
    assert(baseT);
    Address endAddr = trampPostAddr() + baseT->postSize;
    return endAddr - get_address_cr();
}


// recursive into miniTramps
// If we generated once, we skip most of this as useless.
// However, we still go into the miniTramps.

bool baseTrampInstance::generateCode(codeGen &gen,
                                     Address baseInMutatee) {
    inst_printf("baseTrampInstance %p ::generateCode(%p, 0x%x, %d)\n",
                this, gen.start_ptr(), baseInMutatee, gen.used());

    updateMTInstances();

    if (!generated_) {

        if (mtis.size() == 0) {
            hadMini = false;
            return true;
        }

        baseT->generateBT();

        // if in-line...
        // For now BTs are in-lined; they could be made out-of-line
        
        assert(baseT);
        if (!baseT->valid) return false;
        
        trampAddr_ = gen.currAddr(baseInMutatee);
    }
    else {
        // We better be where we were...
        assert(trampAddr_ == gen.currAddr(baseInMutatee));
    }

    codeBufIndex_t preIndex = gen.getIndex();
    unsigned preStart = gen.used();

    // preTramp
    if (!generated_) {
        // Only copy if we haven't been here before. 
        gen.copy(baseT->preTrampCode_);
    }
    else {
        gen.moveIndex(baseT->preSize);
    }
    inst_printf("offset now %d\n", gen.used());
    //unsigned preEnd = offset;
    
    // miniTramps
    // If we go backwards, life gets much easier because each miniTramp
    // needs to know where to jump to (if we're doing out-of-line). 
    // Problem is, we can't if we're doing in-line. 
    
    // So we'll get to it later.

    for (unsigned miter = 0; miter < mtis.size(); miter++) {
        mtis[miter]->generateCode(gen, baseInMutatee);
        // Will increment offset if it writes to baseInMutator;
        // if in-lined each mini will increment offset.
        inst_printf("mti %d, offset %d\n", miter, gen.used());
    }
    
    codeBufIndex_t postIndex = gen.getIndex();
    unsigned postStart = gen.used();
    
    if (!generated_) {
        gen.copy(baseT->postTrampCode_);
    }
    else {
        gen.moveIndex(baseT->postSize);
    }

    inst_printf("post, offset %d\n", gen.used());
    
    if (!generated_) {
        trampPostOffset = postStart - preStart;
    }
    else {
        assert(trampPostOffset == (postStart - preStart));
    }

    // trampAddr_ was set above. 

    // Now, we have a lot of relative jumps that need to be fixed.
    // We've ignored the skipJumps; if there are no miniTramps we
    // just skip the BT altogether. However, there is the guard jump
    // and miniTramp jumps

    // Fix the guard branch...

    if (!generated_) {
        if (baseT->guardBranchIndex) {
            codeBufIndex_t savedIndex = gen.getIndex();
            gen.setIndex(preIndex + baseT->guardBranchIndex);
            int disp = codeGen::getDisplacement(preIndex + baseT->guardBranchIndex,
                                                postIndex + baseT->guardTargetIndex);
            finalizeGuardBranch(gen, disp);
            gen.setIndex(savedIndex);
        }
        else {
            assert(baseT->guardBranchIndex == 0);
            assert(baseT->guardLoadOffset == 0);
            assert(baseT->guardTargetIndex == 0);
        }
    }
    
    // We just generated all the miniTramps,
    // so bump versions on the generated instPointInstances
    generated_ = true;
    hadMini = true;
    return true;
}

bool baseTrampInstance::installCode() {
    // All BTIs are in-lined, so this has no direct work to do;
    // call into minitramps and then ensure that all jumps
    // are correct.

    if (mtis.size() == 0) return true;

    // If in-line...

    for (unsigned i = 0; i < mtis.size(); i++) {
        mtis[i]->installCode();
    }

    installed_ = true;
    return true;
}

void baseTrampInstance::generateBranchToMT(codeGen &gen) {
    // Finally, from the BT to miniTramps
    if (baseT->firstMini) {
        Address firstTarget = baseT->firstMini->getMTInstanceByBTI(this)->trampBase;
        // If MT is out-of-line...
        instruction::generateBranch(gen,
				    trampPreAddr() + baseT->instStartOffset,
				    firstTarget);
        // Safety: make sure we didn't stomp on anything important.
        assert(gen.used() <= baseT->instSize);
    }
    else {
        // We must have nuked the miniTramps...
        assert(baseT->lastMini == NULL);
        // And clear out ze branch-y
        gen.fill(baseT->instSize, codeGen::cgNOP);
    }
}

process *baseTramp::proc() const { 
    if (preInstP)
        return preInstP->proc();
    if (postInstP)
        return postInstP->proc();
    if (rpcMgr_)
        return rpcMgr_->proc();
    return NULL;
}

Address baseTramp::origInstAddr() {
    // Where we would be if we weren't relocated, if we were in the
    // original spot, etc. etc. etc.  Base tramp instances aren't
    // necessarily sorted by anything meaningful, so check the long
    // way.

    instPoint *setInstP = preInstP;
    if (!setInstP) setInstP = postInstP;

    assert(setInstP); // Base tramp with no instPoint? That's an error.

    return setInstP->addr();
}

bool baseTramp::inBasetramp( Address addr ) {
  for (unsigned i = 0; i < instances.size(); i++) {
    if (instances[i]->isInInstance(addr))
      return true;
  }
  return false;
}


bool baseTramp::inSavedRegion (Address addr) {
  for (unsigned i = 0; i < instances.size(); i++) {
    if (instances[i]->isInInstru(addr))
      return true;
  }
  return false;
}

#if 0
void
baseTramp::updateTrampCost(int trampCost) {
#ifndef alpha_dec_osf4_0 /* XXX We don't calculate cost yet on Alpha */
    cost = cost + trampCost;
    if (cost < 0) cost = 0;

    char costInsn[40];
    Address csize = 0;
    Address caddr = proc()->getObservedCostAddr();    
    if (caddr == 0) {
      bpwarn("Observed cost address 0, skipping cost calculation");
      return;
    }
    emitVupdate(updateCostOp, cost, 0, caddr, costInsn, csize, false);
    proc()->writeDataSpace((caddr_t)costAddr, csize, costInsn);
#endif
}
#endif

// Add a miniTramp to the appropriate place: at the start or end of
// its instPoint minis.
bool baseTramp::addMiniTramp(miniTramp *newMT, callOrder order) {
    if (firstMini == NULL) {
        // Life is easy...
        assert(lastMini == NULL);
        firstMini = lastMini = newMT;
    }
    else {
        if (order == orderFirstAtPoint) {
            // Iterate forwards, until we either see our instPoint or
            // one with a higher address (which we want to be before)
            miniTramp *iter = firstMini;
            while (iter != NULL) {
                if (iter->instP == newMT->instP)
                    break;
                else if (iter->instP->addr() >
                         newMT->instP->addr())
                    break;
                iter = iter->next;
            }
            if (iter != NULL) {
                miniTramp *previousPrev = iter->prev;

                newMT->next = iter;
                iter->prev = newMT;

                if (previousPrev)
                    previousPrev->next = newMT;
                newMT->prev = previousPrev;
                
                if (iter == firstMini)
                    firstMini = newMT;
            }
            else {
                // We ran off the end
                lastMini->next = newMT;
                newMT->prev = lastMini;
                lastMini = newMT;
            }
        }
        else {
            // Just like above, but go backwards.

            miniTramp *iter = lastMini;
            while (iter != NULL) {
                if (iter->instP == newMT->instP)
                    break;
                else if (iter->instP->addr() <
                         newMT->instP->addr())
                    break;
                iter = iter->prev;
            }
            if (iter != NULL) {
                miniTramp *previousNext = iter->next;

                newMT->prev = iter;
                iter->next = newMT;

                if (previousNext)
                    previousNext->prev = newMT;
                newMT->next = previousNext;
                
                if (iter == lastMini)
                    lastMini = newMT;
            }
            else {
                // We ran off the end
                firstMini->prev = newMT;
                newMT->next = firstMini;
                firstMini = newMT;
            }
        }
    }
    assert(firstMini != NULL);
    assert(lastMini != NULL);

    instVersion_++;

    return true;
}

bool baseTrampInstance::isInInstance(Address pc) {
    assert(baseT);
    return (pc >= trampPreAddr() &&
            pc < (trampPostAddr() + baseT->postSize));
}

bool baseTrampInstance::isInInstru(Address pc) {
    assert(baseT);
    return (pc >= (trampPreAddr() + baseT->saveStartOffset) &&
            pc < (trampPostAddr() + baseT->restoreEndOffset));
}

instPoint *baseTrampInstance::findInstPointByAddr(Address addr) {
    assert(baseT);
    // Weird, this.
    // If we're in pre-instru, return the prior instP (if it exists)
    // If we're in post, return the next one
    // If only one exists, well...
    if (baseT->preInstP != NULL &&
        baseT->postInstP == NULL) return baseT->preInstP;
    else if (baseT->preInstP == NULL &&
             baseT->postInstP != NULL) return baseT->postInstP;
    else if (baseT->preInstP != NULL &&
             baseT->postInstP != NULL) {
        assert(addr > trampPreAddr());
        if (addr <= (trampPreAddr() + baseT->preSize))
            return baseT->preInstP;
        assert(addr > trampPostAddr());
        if (addr <= (trampPostAddr() + baseT->postSize))
            return baseT->postInstP;
        assert(0);
    }
    else {
        return NULL;
    }
}

// If all miniTramps are removed, nuke the baseTramp
// Chains to multiTramp->deleteIfEmpty
// But for now puts in the "skip" jump if all miniTramps are removed
void baseTramp::deleteIfEmpty() {
    if (firstMini != NULL) {
        assert(lastMini != NULL);
        return;
    }

    assert(lastMini == NULL);

    // This deletion handles _only_ the logical side of things;
    // don't touch the Instances (they stick around until we
    // can garbage collect the code).

    // Clean up this guy
    if (preInstP) {
        assert(preInstP->preBaseTramp() == this);
        preInstP->preBaseTramp_ = NULL;
    }
    if (postInstP) {
        assert(postInstP->postBaseTramp() == this);
        postInstP->postBaseTramp_ = NULL;
    }
    
    delete this;
}

// Global fixing of all BT jumps
bool baseTramp::correctBTJumps() {
    for (unsigned i = 0; i < instances.size(); i++) {
        codeGen gen;
        instances[i]->generateBranchToMT(gen);
        proc()->writeDataSpace((void *)(instances[i]->trampPreAddr() + instStartOffset),
                               gen.used(),
                               gen.start_ptr());
    }
    return true;
}

// Where should the minitramps jump back to?
Address baseTrampInstance::miniTrampReturnAddr() {
    // Might want to make an offset, but for now we go straight back.
    return trampPostAddr();
}

bool baseTramp::isConservative() {
    if (preInstP && preInstP->getPointType() == otherPoint)
        return true;
    if (postInstP && postInstP->getPointType() == otherPoint)
        return true;
    if (rpcMgr_)
        return true;
    return false;
}

bool baseTramp::isCallsite() {
    if (preInstP && preInstP->getPointType() == callSite)
        return true;
    if (postInstP && postInstP->getPointType() == callSite)
        return true;
    return false;
}

bool baseTramp::isEntryExit() {
    if (preInstP && ((preInstP->getPointType() == functionEntry) ||
                     (preInstP->getPointType() == functionExit)))
        return true;
    if (postInstP && ((postInstP->getPointType() == functionEntry) ||
                      (postInstP->getPointType() == functionExit)))
        return true;
    return false;
}

bool baseTrampInstance::shouldGenerate() {
    miniTramp *mini = baseT->firstMini;
    // No miniTramps, no work, no bother.
    return (mini != NULL);
}

unsigned baseTrampInstance::maxSizeRequired() {
    updateMTInstances();

    if (mtis.size() == 0)
        return 0;

    if (!baseT->valid)
        baseT->generateBT();

    assert(baseT->valid);

    unsigned size = 0;

    for (unsigned i = 0; i < mtis.size(); i++)
        size += mtis[i]->maxSizeRequired();

    size += baseT->preSize + baseT->postSize;

    inst_printf("Pre-size %d, post-size %d, total size %d\n",
                baseT->preSize,
                baseT->postSize,
                size);
    return size;
}

// Should have some mechanism to understand if we need
// to clear this out and start over... this is a lot
// of wasted work.
void baseTrampInstance::updateMTInstances() {
    mtis.clear();

    miniTramp *mini = baseT->firstMini;
    while (mini) {
        miniTrampInstance *mti = mini->getMTInstanceByBTI(this);
        mtis.push_back(mti);
        mini = mini->next;
    }
}

baseTrampInstance *baseTramp::findOrCreateInstance(multiTramp *multi) {
    // First, check to see if we have one...
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->multiT == multi)
            return instances[i];
    }
    baseTrampInstance *newInst = new baseTrampInstance(this, multi);
    instances.push_back(newInst);
    return newInst;
}

void baseTramp::setRecursive(bool trampRecursive) {
    if (trampRecursive) {
        if (guardState_ == unset_BTR)
            guardState_ = recursive_BTR;
        else {
            if (guardState_ == guarded_BTR) {
                cerr << "WARNING: collision between pre-existing guarded miniTramp and new miniTramp, keeping guarded!" << endl;
            }
        }
    }
    else {
        if (guardState_ == unset_BTR) {
            guardState_ = guarded_BTR;
        }
        else {
            if (guardState_ == recursive_BTR) {
                cerr << "WARNING: collision between pre-existing recursive miniTramp and new miniTramp, now guarded!" << endl;
                guardState_ = guarded_BTR;
            }
        }
    }
}

bool baseTramp::getRecursive() const {
    switch (guardState_) {
    case unset_BTR:
        assert(0);
        return false;
        break;
    case recursive_BTR:
        return true;
        break;
    case guarded_BTR:
        return false;
        break;
    }
    assert(0);
    return false;
}

// Generates an instruction buffer that holds the base tramp
bool baseTramp::generateBT() {
    if (valid)
        return true; // May be called multiple times

    inst_printf("Generating a baseTramp, guarded %d\n", guardState_);
    // Make a base tramp. That is, a save/restore pair that includes the base
    // tramp guard. 
    // Also populate member vrbles:
    // preSize, postSize;
    // saveStartOffset
    // saveEndOffset
    // guardLoadOffset
    // guardBranchOffset
    // ..  and guardBranchSize
    // costUpdateOffset
    // instStartOffset
    // ... instSize is set by the miniTramp

    // restoreStartOffset
    // guardTargetOffset
    // restoreEndOffset
    // trampEnd
    
    /*
     * This is what a base tramp looks like:
     offset     insn                 cost

     ////////////////// X86 ///////////////////

     PRE:
     0:         pushfd               5
     1:         pushad               9
     2:         push ebp             1
     3:         mov esp, ebp         1
     6:         subl esp, 0x80       1
     ... multithread code (must go before tramp guard)
     ... tramp guard

     12:        start of minitramp   ??

     POST:
     0:         add <costaddr> <cost> 3
     10:        leave                 3
     11:        popad                 14
     12:        popfd                 5
     //////////////////////////////////////////

     ////////////////POWER/////////////////////
     0:         stu   (make stack frame)
     ....
     
     */

    // We should catch if we're regenerating
    assert(!preTrampCode_());
    assert(!postTrampCode_());

    preTrampCode_.allocate(PRE_TRAMP_SIZE);
    postTrampCode_.allocate(POST_TRAMP_SIZE);

    extern registerSpace *regSpace;

#if defined(arch_power)
    // For tracking saves/restores.
    // Could be used on other platforms as well, but isn't yet.
    extern registerSpace *conservativeRegSpace;

    if (isConservative())
        theRegSpace = conservativeRegSpace;
    else
        theRegSpace = regSpace;
#endif

    saveStartOffset = preTrampCode_.used();
    inst_printf("Starting saves: offset %d\n", saveStartOffset);
    generateSaves(preTrampCode_, regSpace);

    // Done with save
    saveEndOffset = preTrampCode_.used();
    inst_printf("Starting MT: offset %d\n", saveEndOffset);
    // Multithread
    generateMTCode(preTrampCode_);
    // Guard code
    guardLoadOffset = preTrampCode_.used();
    inst_printf("Starting guard: offset %d\n", guardLoadOffset);
    if (guarded() &&
        generateGuardPreCode(preTrampCode_,
                             guardBranchIndex,
                             regSpace)) {
        // Cool.
    }
    else {
        // No guard...
        guardBranchIndex = 0;
        guardLoadOffset = 0;
    }
    
    costUpdateOffset = preTrampCode_.used();

    inst_printf("Starting cost: offset %d\n", costUpdateOffset);

    // We may not want cost code... for now if we're an iRPC tramp
    // In the future, this may change
    // In general, instrumentation wants cost code. The minitramps may be 
    // null, but we put in the stub anyway.
    if (rpcMgr_ == NULL &&
        generateCostCode(preTrampCode_, costValueOffset, regSpace)) {
        costSize = preTrampCode_.used() - costUpdateOffset;
        // If this is zero all sorts of bad happens
        assert(costValueOffset);
    }
    else {
        costSize = 0;
        costValueOffset = 0;
    }

    instStartOffset = preTrampCode_.used();
    preSize = preTrampCode_.used();
    inst_printf("Starting inst: offset %d\n", instStartOffset);
    inst_printf("preSize is: %d\n", preSize);    

    // Post...
    // Guard redux
    if (guarded())
        generateGuardPostCode(postTrampCode_, 
                              guardTargetIndex,
                              regSpace);
    else
        guardTargetIndex = 0;

    // Restore registers
    restoreStartOffset = postTrampCode_.used();
    generateRestores(postTrampCode_, regSpace);
    restoreEndOffset = postTrampCode_.used();
    inst_printf("Ending restores: %d\n", restoreEndOffset);

    trampEnd = postTrampCode_.used();
    postSize = postTrampCode_.used();

    valid = true;
    preTrampCode_.finalize();
    postTrampCode_.finalize();

    inst_printf("pre size: %d, post size: %d\n",
                preSize, postSize);
    inst_printf("saveStart: %d, saveEnd: %d, guardLoad: %d, guardBranch %d, cost %d, inst %d\n",
                saveStartOffset,
                saveEndOffset,
                guardLoadOffset,
                guardBranchIndex,
                costUpdateOffset,
                instStartOffset);

    inst_printf("restoreStart %d, guardTarget %d, restoreEnd %d, trampEnd %d\n",
                restoreStartOffset,
                guardTargetIndex,
                restoreEndOffset,
                trampEnd);

    return true;
}
    
void baseTrampInstance::removeCode(generatedCodeObject *subObject) {
    // TODO: out-of-line baseTramps?

    miniTrampInstance *delMTI = dynamic_cast<miniTrampInstance *>(subObject);
    multiTramp *delMulti = dynamic_cast<multiTramp *>(subObject);
    assert(delMTI || delMulti);

    if (delMTI) {
        // We lost a miniTramp...
        
        // Move the MTI from the current list to the deleted list.
        for (unsigned i = 0; i < mtis.size(); i++) {
            if (mtis[i] == subObject) {
                deletedMTIs.push_back(mtis[i]);
                mtis[i] = mtis.back();
                mtis.pop_back();
                break;
            }
        }
        
        // See if this is the last MTI; if so, delete ourselves.
        if (mtis.size() == 0) {
            // We didn't allocate, so we don't call deleteGenerated...
            //proc()->deleteGeneratedCode(this);
            multiT->removeCode(this);
            // The above call to removeCode may cause all sorts of havoc,
            // including regenerating the multiTramp. On the other hand,
            // if there are miniTramps left, we effectively stop here.
        }
        else {
            // When we in-line, this will need to change. For now,
            // we can always fix jumps by hand
            codeGen gen;
            generateBranchToMT(gen);
            proc()->writeDataSpace((void *)(trampPreAddr() + baseT->instStartOffset),
                                   gen.used(),
                                   gen.start_ptr());
        }
    }
    else {
        assert(delMulti); 
        if (multiT != delMulti) {
            // Huh. Okay, someone we don't care about any more went away. Nifty.
            return;
        }
        // The multiTramp was just removed. Tell our miniTramps about it;
        // if they haven't been attached to a different baseTrampInstance,
        // this will remove them.
        for (unsigned j = 0; j < mtis.size(); j++) {
            mtis[j]->removeCode(this);
        }
        baseT->unregisterInstance(this);
    }
           
}

bool baseTrampInstance::safeToFree(codeRange *range) {

    if (dynamic_cast<baseTrampInstance *>(range) == this)
        return false;

    // Better not be freeing ourselves if we have sub-mtis left...
    assert(mtis.size() == 0);

    // If the PC is in any of the miniTramps, we're not
    // good.
    
    // I think we can just check all of the MTIs using their
    // safeToFree; if we can't free a miniTramp, we can't free
    // the baseTramp
    for (unsigned i = 0; i < deletedMTIs.size(); i++)
        if (!deletedMTIs[i]->safeToFree(range))
            return false;

    return true;
}

process *baseTrampInstance::proc() const {
    assert(baseT);
    return baseT->proc();
}


void baseTrampInstance::invalidateCode() {
    assert(!linked_);
    for (unsigned i = 0; i < mtis.size(); i++)
        mtis[i]->invalidateCode();
}

bool baseTrampInstance::hasChanged() {
    // TODO: in-line...
    // In general, baseTramps are okay with adding
    // or removing instrumentation, since miniTramps
    // are out-of-line. However, if there are no miniTramps,
    // we're considered to have changed.

    return ((baseT->firstMini != NULL) != hadMini);
}

bool baseTrampInstance::linkCode() {
    // TODO inline

    if (mtis.size() == 0) return true;

    unsigned cost = 0;
    for (unsigned i = 0; i < mtis.size(); i++) {
        mtis[i]->linkCode();
        cost += mtis[i]->cost();
    }

    Address leave = trampPreAddr() + baseT->instStartOffset;
    
    Address arrive = baseT->firstMini->getMTInstanceByBTI(this)->trampBase;

    inst_printf("writing branch from 0x%x to 0x%x, baseT->miniT\n",
                leave, arrive);

#if defined(os_aix)
    resetBRL(baseT->proc(), leave, arrive);
#else
    generateAndWriteBranch(baseT->proc(), 
                           leave, 
                           arrive, 
                           miniTramp::interJumpSize());
#endif

    // Cost calculation
    if (cost) {
        // If 0, all mts were set to noCost, so don't increment us.
        cost += baseT->getBTCost();
    }
    updateTrampCost(cost);

    linked_ = true;
    return true;
}

generatedCodeObject *baseTrampInstance::replaceCode(generatedCodeObject *newParent) {
    // Since we are generated in-line, we return a copy
    // instead of ourselves.

    // Though if we haven't been generated yet, don't bother
    inst_printf("replaceCode for baseTramp %p, new par %p, previous %p\n", this,
            newParent, multiT);

    multiTramp *newMulti = dynamic_cast<multiTramp *>(newParent);
    assert(newMulti);

    if (trampAddr_ == 0) {
        multiT = newMulti;
        return this;
    }

    baseTrampInstance *newBTI = baseT->findOrCreateInstance(newMulti);
    assert(newBTI);
    for (unsigned i = 0; i < mtis.size(); i++) {
        mtis[i]->replaceCode(newBTI);
    }
    return newBTI;
}

void baseTrampInstance::deleteMTI(miniTrampInstance *mti) {
    for (unsigned i = 0; i < mtis.size(); i++) {
        if (mtis[i] == mti) {
            mtis[i] = mtis.back();
            mtis.pop_back();
            return;
        }
    }
    assert(0);
    return;
}

instPoint *baseTramp::point() const {
    if (preInstP) return preInstP;
    else if (postInstP) return postInstP;
    else assert(0);
    return NULL;
}

Address baseTrampInstance::uninstrumentedAddr() const {
    if (fallthrough_)
        return fallthrough_->uninstrumentedAddr();
    else if (previous_)
        return previous_->uninstrumentedAddr();
    else 
        assert(0);
    return 0;
}
