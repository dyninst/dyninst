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

// $Id: multiTramp.C,v 1.15 2005/09/15 22:08:59 bernat Exp $
// Code to install and remove instrumentation from a running process.

#include "multiTramp.h"
#include "baseTramp.h"
#include "miniTramp.h"
#include "instPoint.h"
#include "process.h"
#include "InstrucIter.h"

unsigned int multiTramp::id_ctr = 1;

multiTramp *multiTramp::getMulti(int id, process *p) {
    if(p->multiTrampDict.defines(id))
        return p->multiTrampDict[id];
    else
        return NULL;
}

baseTrampInstance *multiTramp::getBaseTrampInstance(instPointInstance *point,
                                                    callWhen when) const {
    // How do we instrument this point at the desired address

    // And a safety note;
    assert(point->multi() == this);

    relocatedInstruction *insn = insns_[point->addr()];
    assert(insn);

    switch (when) {
    case callPreInsn: {
        inst_printf("Matching preBTI\n");
        baseTrampInstance *preBTI = dynamic_cast<baseTrampInstance *>(insn->previous_);
        if (preBTI) return preBTI;
        break;
    }
    case callPostInsn: {
        inst_printf("Matching postBTI\n");
        baseTrampInstance *postBTI = dynamic_cast<baseTrampInstance *>(insn->fallthrough_);
        if (postBTI) return postBTI;
        break;
    }
    case callBranchTargetInsn: {
        inst_printf("Matching targetBTI\n");
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

    // Have _no_ idea how to handle one of these guys going away.
    assert(!reloc);
    assert(!te);

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
            if (tmp == bti) {
                if (tmp->previous_) {
                    tmp->previous_->setFallthrough(tmp->fallthrough_);
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
                bti->removeCode(this);
            }
            else {
                if (!bti->isEmpty())
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
    }
    
    if (doWeDelete) {
        // We're empty. Time to leave.
        if (savedCodeBuf_) {
            // Okay, they're all empty. Overwrite the jump to this guy with the saved buffer.
            bool res = proc()->writeTextSpace((void *)instAddr_,
                                             instSize_,
                                             (void *)savedCodeBuf_);
            // This better work, or we're going to be jumping into all sorts
            // of hurt.
            assert(res);
            
            free(savedCodeBuf_);
            savedCodeBuf_ = 0;
        }
        if (proc()->multiTrampDict[id()] == this) {
            // Won't leak as the process knows to delete us
            proc()->multiTrampDict[id()] = NULL;
        }
        
        proc()->deleteGeneratedCode(this);
        proc()->removeMultiTramp(this);

        for (unsigned i = 0; i < deletedObjs.size(); i++) 
            deletedObjs[i]->removeCode(this);
    }   
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
    for (unsigned i = 0; i < deletedObjs.size(); i++) 
        delete deletedObjs[i];

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;

    while ((obj = cfgIter++)) {
        // Before we delete the object :)
        delete obj;
    }

    if (savedCodeBuf_)
        free(savedCodeBuf_);

    // And this is why we want a process pointer ourselves. Trusting the
    // function to still be around is... iffy... at best.
    proc()->removeMultiTramp(this);
    proc()->deleteCodeRange(get_address_cr());
    proc()->inferiorFree(trampAddr_);

    // Everything else is statically allocated
}

// Assumes there is no matching multiTramp at this address
int multiTramp::findOrCreateMultiTramp(Address pointAddr, 
                                       process *proc) {
    multiTramp *newMulti = proc->findMultiTramp(pointAddr);
    if (newMulti) {
        // Check whether we're in trap shape
        // Sticks to false if it ever is false
        return newMulti->id();
    }

    // Multitramps need to know their functions, so look it up.
    codeRange *range = proc->findCodeRangeByAddress(pointAddr);
    if (!range) {
        return 0;
    }
    int_function *func = range->is_function();
    if (!func) {
        fprintf(stderr, "No function in createMultiTramp, ret NULL\n");
        return 0;
    }

    Address startAddr;
    unsigned size;

    // On most platforms we instrument an entire basic block at a
    // time. IA64 does bundles. This is controlled by the static
    // getMultiTrampFootprint function; so if you want to do something
    // odd then go poke there. Individual instrumentation _should_
    // work but is not tested.

    if (!multiTramp::getMultiTrampFootprint(pointAddr,
                                            proc,
                                            startAddr,
                                            size)) {
        // Assert fail?
        return 0;
    }

    // Cannot make a point here if someone else got there first
    if (proc->findModifiedPointByAddr(startAddr))
        return 0;

    newMulti = new multiTramp(startAddr,
                              size,
                              func);
    
    // Iterate over the covered instructions and pull each one
    InstrucIter insnIter(startAddr,
                         size,
                         proc);
    relocatedInstruction *prev = NULL;
    
    while (insnIter.hasMore()) {
        instruction *insn = insnIter.getInsnPtr();
        Address insnAddr = *insnIter;

        relocatedInstruction *reloc = new relocatedInstruction(insn, insnAddr, newMulti);
        inst_printf("Relocating instruction at 0x%x: %p\n", insnAddr, reloc);
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
#endif
        insnIter++;
    }   

    
    // Put this off until we generate
    //newMulti->updateInstInstances();

    proc->addMultiTramp(newMulti);

    return newMulti->id();
}

// Get the footprint for the multiTramp at this address. The MT may
// not exist yet; our instrumentation code needs this.
bool multiTramp::getMultiTrampFootprint(Address instAddr,
                                        process *proc,
                                        Address &startAddr,
                                        unsigned &size) {
#if defined(arch_ia64)
    // IA64 bundle-izes
    startAddr = instAddr - (instAddr % 16);
    size = 16; // bundlesize
    return true;
#else
    // We use basic blocks
    // Otherwise, make one.
    codeRange *range = proc->findCodeRangeByAddress(instAddr);
    if (!range) {
        return false;
    }
    bblInstance *bbl = range->is_basicBlockInstance();
    if (!bbl) {
        inst_printf("No basic block instance in createMultiTramp, ret NULL\n");
        return false;
    }
    
    // start is the start of the basic block, size is the size
    startAddr = bbl->firstInsnAddr();
    size = (unsigned) bbl->getSize();

    return true;
#endif
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
        // If we're the last thing in a chain (of which there may be many),
        // add a trampEnd if there isn't already one.
        if (!obj->fallthrough_) {
            trampEnd *end = dynamic_cast<trampEnd *>(obj);
            if (!end) {
                // This is the end of a chain but we don't have an end
                // marker. Add one.

                // The question is, where do we jump? We want to go to the
                // next instruction from the relocated insn. First, find the
                // relocated instruction (as obj might be a baseTramp)
                relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
                if (!insn) {
                    assert(dynamic_cast<baseTrampInstance *>(obj));
                    assert(obj->previous_);
                    insn = dynamic_cast<relocatedInstruction *>(obj->previous_);
                }
                assert(insn);
                
                // Let's get the next insn... we can do this with an InstrucIter
                InstrucIter iter(insn->origAddr, func());

                obj->setFallthrough(new trampEnd(this, iter.peekNext()));
                obj->fallthrough_->setPrevious(obj);
                changedSinceLastGeneration_ = true;
            }
        }
        
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        if (!insn) { 
            prev = obj;
            continue;
        }

        Address insnAddr = insn->origAddr;
        instPoint *instP = proc()->findInstPByAddr(insnAddr);
        if (!instP) {
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
        
        prev = obj;
    }

#if 0
    cfgIter.initialize(generatedCFG_);
    while ((obj = cfgIter++)) {
        fprintf(stderr, "obj: %p (prev %p, next %p)...", obj,
                obj->previous_, obj->fallthrough_);
        relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
        baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
        trampEnd *end = dynamic_cast<trampEnd *>(obj);
        if (insn) fprintf(stderr, "insn\n");
        if (bti) fprintf(stderr, "bti\n");
        if (end) fprintf(stderr, "end\n");
    }
#endif
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
    generatedMultiT_(),
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(false)
{
    // .. filled in by createMultiTramp
    assert(proc());
    proc()->multiTrampDict[id_] = this;
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
    generatedMultiT_(), // Not copied
    jumpBuf_(), // Not copied
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(true) 
{
    // This is superficial and insufficient to recreate the multiTramp; please
    // call replaceCode with the old tramp as an argument.
}

// Fork constructor.
multiTramp::multiTramp(const multiTramp *parMulti, process *child) :
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
    generatedMultiT_(parMulti->generatedMultiT_),
    jumpBuf_(parMulti->jumpBuf_),
    savedCodeBuf_(NULL),
#if defined( cap_unwind )
    unwindInformation(NULL),	
#endif /* defined( cap_unwind ) */    
    changedSinceLastGeneration_(parMulti->changedSinceLastGeneration_) 
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
}

////////////

#if 0
// Add a new instInstance to our list of instrumented spots. Should be
// tolerant of re-added instPIs.
void multiTramp::addInstInstance(instPointInstance *instInstance) {
    instPoint *instP = instInstance->point;
    // Just in case it wasn't already set...
    
    if (!instInstance->multi())
        instInstance->updateMulti(id());
    else {
        assert(instInstance->multi() == this);
    }
    
    Address addr = instInstance->addr();
    for (unsigned i = 0; i < insns_.size(); i++) {
        if (insns_[i]->origAddr() == addr) {
            bookkeeping *relocInfo = insns_[i];
            if (relocInfo->inst) {
                // Already done
                assert(relocInfo->inst == instInstance);
            }
            else {
                relocInfo->inst = instInstance;
                changedSinceLastGeneration_ = true;
            }

            // We found the instInstance. Now see if
            // it's changed recently. That includes:
            // New: has changed.
            // Has added or removed a baseTrampInstance;
            // the baseTrampInstance reports an update

            // Set up baseTrampInstances, making sure that no overlaps occur
            baseTramp *preBT = instP->preBaseTramp();
            baseTramp *postBT = instP->postBaseTramp();
            baseTramp *targetBT = instP->targetBaseTramp();
            
            // Don't double-create...
            // Note: this can be done more efficiently. For now, correctness.
            for (unsigned foo = 0; foo < allTramps.size(); foo++) {
                if (preBT &&
                    allTramps[foo]->baseT == preBT) {

                    assert(allTramps[foo]->multiT == this);
                    assert(!relocInfo->preBTI ||
                           relocInfo->preBTI == allTramps[foo]);

                    if (!relocInfo->preBTI) {
                        relocInfo->preBTI = allTramps[foo];
                        changedSinceLastGeneration_ = true;
                    }
                }
                if (postBT &&
                    allTramps[foo]->baseT == postBT) {
                    assert(allTramps[foo]->multiT == this);
                    assert(!relocInfo->postBTI ||
                           relocInfo->postBTI == allTramps[foo]);
                    if (!relocInfo->postBTI) {
                        relocInfo->postBTI = allTramps[foo];
                        changedSinceLastGeneration_ = true;
                    }
                }
                if (targetBT &&
                    allTramps[foo]->baseT == targetBT) {
                    assert(allTramps[foo]->multiT == this);
                    assert(!relocInfo->targetBTI ||
                           relocInfo->targetBTI == allTramps[foo]);
                    if (!relocInfo->targetBTI) {
                        relocInfo->targetBTI = allTramps[foo];
                        changedSinceLastGeneration_ = true;
                    }
                }
            }
            // Constructor adds this to the BT instance list
            if (preBT && !relocInfo->preBTI) {
                relocInfo->preBTI = preBT->findOrCreateInstance(this);
                allTramps.push_back(relocInfo->preBTI);
                changedSinceLastGeneration_ = true;
            }
            if (postBT && !relocInfo->postBTI) {
                relocInfo->postBTI = postBT->findOrCreateInstance(this);
                allTramps.push_back(relocInfo->postBTI);
                changedSinceLastGeneration_ = true;
            }
            if (targetBT && !relocInfo->targetBTI) {
                relocInfo->targetBTI = targetBT->findOrCreateInstance(this);
                allTramps.push_back(relocInfo->targetBTI);
                changedSinceLastGeneration_ = true;
            }
            
            return;
        }// If we found the right insn
    }
    // Uhh... we added an instPointInstance to the wrong multiTramp!
    assert(0);
    return;
}

#endif

// To avoid mass include inclusion
int_function *multiTramp::func() const { return func_; }

process *multiTramp::proc() const { return proc_; }

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
                              UNW_INFO_TYPE * * /* ignored */) {
    unsigned size_required = 0;

    generatedCFG_t::iterator cfgIter;
    generatedCodeObject *obj = NULL;

    // We might be getting called but nothing changed...
    if (!generated_) {
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
        obj = NULL;
        
        while ((obj = cfgIter++)) {
            // If we're the target for someone, pin at this
            // addr. This means some wasted space; however, it's
            // easier and allows us to generate in one pass.
            
            if (obj->previous_ &&
                obj->previous_->target_ == obj) {
                // We're a target for somebody.
                obj->pinnedOffset = size_required;
            }
            
            // Then update the size
            size_required += obj->maxSizeRequired();
        }
        
        // We never re-use multiTramps
        assert(!trampAddr_);
        
        inferiorHeapType heapToUse = anyHeap;
#if defined(bug_aix_proc_broken_fork)
        // We need the base tramp to be in allocated heap space, not scavenged
        // text space, because scavenged text space is _not_ copied on fork.
        // Argh.
        if (func()->prettyName() == pdstring("__fork")) {
            heapToUse = (inferiorHeapType) (textHeap | dataHeap); // not uncopiedHeap
        }
#endif

        trampAddr_ = proc()->inferiorMalloc(size_required,
                                            heapToUse,
                                            instAddr_);
        if (!trampAddr_) {
            fprintf(stderr, "Failed to inferiorMalloc, ret false\n");
            return false;
        }
        generatedMultiT_.allocate(size_required);

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
    
    generatedMultiT_.setIndex(0);
    
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
    unw_dyn_region_info_t * unwindRegion = initialRegion;
#endif /* defined( cap_unwind ) */
        
    inst_printf("multiTramp generation: local %p, remote 0x%x, size %d\n",
                generatedMultiT_.start_ptr(), trampAddr_, size_required);

    cfgIter.initialize(generatedCFG_);
    obj = NULL;
    while ((obj = cfgIter++)) {
        
        if (obj->pinnedOffset) {
            // We need to advance the pointer, backfilling
            // with noops (or illegals, actually)
            // This won't do anything if we're in the right place
            generatedMultiT_.fill(obj->pinnedOffset - generatedMultiT_.used(),
                                  codeGen::cgNOP);
            assert(generatedMultiT_.used() == obj->pinnedOffset);
        }
        
        // Target override if necessary
        if (obj->target_) {
            relocatedInstruction *relocInsn = dynamic_cast<relocatedInstruction *>(obj);
            assert(relocInsn);
            relocInsn->overrideTarget(trampAddr_ + obj->target_->pinnedOffset);
        }

#if ! defined( cap_unwind )
        if( !obj->generateCode( generatedMultiT_, trampAddr_, NULL ) ) {
            return false;
        }
#else
        if( ! obj->generateCode( generatedMultiT_, trampAddr_, & unwindRegion ) ) {
            return false;
        }
#endif /* ! defined( cap_unwind ) */

        inst_printf("After node: mutatee 0x%x, offset %d, size req %d\n",
                    generatedMultiT_.currAddr(trampAddr_),
                    generatedMultiT_.used(), size_required);

        // Safety...
        assert(generatedMultiT_.used() <= size_required);
	/*
          if (counter == 0) {
	  instruction trap = instruction::createBreakPoint();
	  trap.generate(generatedMultiT_, mtOffset);
          }
          counter++;
	*/	
    }

    trampSize_ = generatedMultiT_.used();

    // Now that we know where we're heading, see if we can put in 
    // a jump
    assert(instAddr_);
    assert(instSize_);

    changedSinceLastGeneration_ = false;
    
    if (!generated_) {
        jumpBuf_.allocate(instSize_);
        // We set this = true before we call generateBranchToTramp
        generated_ = true;
        if (!generateBranchToTramp(jumpBuf_)) {
            // This failing is serious. For little things like "we need to
            // use a trap", return true but set branchSize > instSize...
            // TODO: clean me up :)
            invalidateCode();
            return false;
        }
    }
    
    //debugBreakpoint();
    
    return true;
}

bool multiTramp::installCode() {
    // We need to add a jump back and fix any conditional jump
    // instrumentation
    assert(generatedMultiT_.used() == trampSize_); // Nobody messed with things
    assert(generated_);

    // Try to branch to the tramp, but if we can't use a trap
    if (branchSize_ > instSize_) {
        // Crud. Go with traps.
        jumpBuf_.setIndex(0);
        generateTrapToTramp(jumpBuf_);
    }
    fillJumpBuf(jumpBuf_);

    if (!installed_) {
        bool success = proc()->writeTextSpace((void *)trampAddr_,
                                              trampSize_,
                                              generatedMultiT_.start_ptr());
        if( success ) {
            proc()->addCodeRange(this);
#if defined( cap_unwind )
            if( unwindInformation != NULL ) {
                unwindInformation->start_ip = trampAddr_;
                unwindInformation->end_ip = trampAddr_ + trampSize_;
                unwindInformation->gp = proc()->getTOCoffsetInfo( instAddr_ );
            
                dyn_unw_printf( "%s[%d]: registering multitramp unwind information for 0x%lx, at 0x%lx-0x%lx, GP 0x%lx\n",
                                __FILE__, __LINE__, instAddr_, unwindInformation->start_ip, unwindInformation->end_ip,
                                unwindInformation->gp );
	            if( ! proc()->insertAndRegisterDynamicUnwindInformation( unwindInformation ) ) {
    	    	    return false;
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
    // Relocation should be done before this is called... not sure when though.

    // We may already be linked in, and getting called because of a regeneration.
    // 
    // First, copy out where we're going
    assert(installed_);

    assert(jumpBuf_.used() == instSize_);

    if (!linked_) {
        codeRange *prevRange = proc()->findModifiedPointByAddr(instAddr_);
        if (prevRange != NULL) {
            // Someone's already here....
            if (prevRange->is_function_replacement()) {
                // Don't install here, just dummy-return true
                return true;
            }
            else if (prevRange->is_replaced_call()) {
                // TODO
                fprintf(stderr, "ERROR: instrumentation stomping on replaced call!\n");
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

    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        obj->linkCode();
    }
    return true;
}

// And a wrapper for the above
multiTramp::mtErrorCode_t multiTramp::linkMultiTramp() {
    assert(!hasChanged()); // since we generated code...

    if (linkCode())
        return mtSuccess;
    else {
        fprintf(stderr, "Linking multiTramp failed!\n");
        return mtError;
    }
}

multiTramp::mtErrorCode_t multiTramp::installMultiTramp() {
    assert(!hasChanged()); // Since we generated code...

    // See if there is enough room to fit the jump in... then
    // decide whether to go ahead or not.

    if (installCode())
        return mtSuccess;
    else {
        fprintf(stderr, "multiTramp::install failed!\n");
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
    if (!proc()->writeTextSpace((void *)instAddr_,
                                instSize_,
                                savedCodeBuf_))
        return false;
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

        if (insn && 
            (addr >= insn->relocAddr()) &&
            (addr < insn->relocAddr() + insn->get_size_cr()))
            return insn->origAddr;
        if (bti && bti->isInInstance(addr)) {
            // If we're pre for an insn, return that;
            // else return the insn we're post/target for.
            baseTramp *baseT = bti->baseT;
            instPoint *point = NULL;
            if (baseT->preInstP)
                point = baseT->preInstP;
            else if (baseT->postInstP)
                point = baseT->postInstP;
            else
                assert(0);
            for (unsigned i = 0; i < point->instances.size(); i++) {
                // We check by ID instead of pointer because we may
                // have been replaced by later instrumentation, but 
                // are still being executed.
                if (point->instances[i]->multiID() == id_)
                    return point->addr();
            }
            // No match: bad data structures.
            assert(0);
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
        if ((obj->get_address_cr() <= addr) &&
            (addr < (obj->get_address_cr() + obj->get_size_cr()))) {
            relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
            baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
            trampEnd *end = dynamic_cast<trampEnd *>(obj);
            if (end) {
                // We don't want the tramp end; so see if there is a previous
                // base tramp or instruction
                insn = dynamic_cast<relocatedInstruction *>(obj->previous_);
                bti = dynamic_cast<baseTrampInstance *>(obj->previous_);
            }
            assert(insn || bti);

            if (insn)
                return proc()->findInstPByAddr(addr);
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

    assert(generated_);

    relocatedInstruction *insn = NULL;
    if (insns_.find(addr))
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

multiTramp::mtErrorCode_t multiTramp::generateMultiTramp() {
    updateInstInstances();
    if (hasChanged()) {
        if (linked_) {
            // We're in the process' address space; if we need to change the
            // multiTramp, we replace it. replaceMultiTramp takes care of
            // all that. 
            bool deleteReplaced = false;
            bool res = multiTramp::replaceMultiTramp(this, deleteReplaced);
            if (deleteReplaced)
                removeCode(NULL);
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
    if (generateCode(jumpBuf_, instAddr_, NULL))
        return mtSuccess;
    else {
        fprintf(stderr, "mt::generateCode failed!!!\n");
        return mtError;
    }
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
    
    if (generatedMultiT_ != NULL)
        generatedMultiT_.invalidate();

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
    
    multiTramp *newMulti = dynamic_cast<multiTramp *>(oldMulti->replaceCode(NULL));
    assert(newMulti);

    // newMulti either has copies of old data structures (all
    // generatedCodeObjects) or the originals, depending on which
    // is appropriate. Status flags are not copied; the new
    // tramp is neither generated nor linked.
    
    // Sub the new tramp into the old tramp:
    assert(newMulti->proc() == oldMulti->proc());
    process *proc = oldMulti->proc();
    assert(proc->multiTrampDict[oldMulti->id()] == oldMulti);
    proc->multiTrampDict[oldMulti->id()] = newMulti;

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
        res = newMulti->linkCode();
        if (!res) return false;
    }

    // Process update
    codeRange *range = newMulti->proc()->findModifiedPointByAddr(newMulti->instAddr());
    // Range may be null, since we can replace before we link the first time
    if (range && range->is_multitramp())
        newMulti->proc()->addMultiTramp(newMulti);
    // Otherwise is function relocation... so don't overwrite.
    else
        fprintf(stderr, "Function replacement already there, not adding at 0x%lx\n",
                newMulti->instAddr());

    // Caller decides whether to remove the original version
    deleteReplaced = true;
    return true;
}

generatedCodeObject *generatedCFG_t::copy_int(generatedCodeObject *obj, 
                                              generatedCodeObject *par,
                                              multiTramp *newMulti,
                                              pdvector<generatedCodeObject *> &unused) {
    generatedCodeObject *newObj = obj->replaceCode(newMulti);

    if (newObj != obj)
        unused.push_back(obj);

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
                                              process *child) {
    // Since you can't do a virtual constructor...
    // Could add a fork() method to everyone, but I like the consistency of the
    // constructor(..., process *) model.
    const baseTrampInstance *bti = dynamic_cast<const baseTrampInstance *>(parObj);
    const trampEnd *te = dynamic_cast<const trampEnd *>(parObj);
    const relocatedInstruction *ri = dynamic_cast<const relocatedInstruction *>(parObj);

    generatedCodeObject *childObj = NULL;

    if (bti) {
        assert(!te);
        assert(!ri);
        // We need to get the child baseTramp. We duck through an instPoint to
        // do so. We could also register all baseTramps with the process object,
        // but fork is infrequent and so can be expensive.
        baseTramp *parBT = bti->baseT;
        instPoint *cIP = NULL;
        baseTramp *childBT = NULL;
        if (parBT->postInstP) {
            cIP = child->findInstPByAddr(parBT->postInstP->addr());
            assert(cIP);
            // We split here... post and target show up as "pre" to the BT
            if (parBT->postInstP->postBaseTramp() == parBT)
                childBT = cIP->postBaseTramp();
            else if (parBT->postInstP->targetBaseTramp() == parBT)
                childBT = cIP->targetBaseTramp();
            else 
                assert(0);
        } else if (parBT->preInstP) {
            cIP = child->findInstPByAddr(parBT->preInstP->addr());
            assert(cIP);
            childBT = cIP->preBaseTramp();
        }
        else {
            assert(0);
        }
        assert(childBT);
        childObj = new baseTrampInstance(bti,
                                         childBT,
                                         childMulti,
                                         child);
    }
    else if (te) {
        assert(!bti);
        assert(!ri);
        childObj = new trampEnd(te, childMulti, child);
    } 
    else if (ri) {
        assert(!bti);
        assert(!te);
        childObj = new relocatedInstruction(ri, childMulti, child);
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
                               process *child) {
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
            if (insns_.find(insn->origAddr))
                assert(insns_[insn->origAddr] == insn);
            insns_[insn->origAddr] = insn;
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
    deletedObjs.clear();
    newMulti->generatedCFG_.replaceCode(generatedCFG_, newMulti, deletedObjs);

    // Nuke ours, just in case
    generatedCFG_.destroy();
    
    // Update addrs
    newMulti->updateInsnDict();

    // Buffers: generatedMultiT_, savedCodeBuf_, and jumpBuf_.
    // generatedMultiT_ and jumpBuf_ are multiTramp specific.
    assert(newMulti->generatedMultiT_ == NULL);
    assert(newMulti->jumpBuf_ == NULL);
    // and savedCodeBuf_ we want to take
    newMulti->savedCodeBuf_ = savedCodeBuf_;
    savedCodeBuf_ = NULL;

    // And final checking
    assert(newMulti->func_);
    assert(newMulti->proc_);

    assert(newMulti->instAddr_);
    assert(newMulti->instSize_);
    assert(newMulti->trampAddr_ == 0);
    assert(newMulti->trampSize_ == 0);

    return newMulti;
}
    

bool multiTramp::hasChanged() {
    if (changedSinceLastGeneration_) {
        return true;
    }
    generatedCFG_t::iterator cfgIter(generatedCFG_);
    generatedCodeObject *obj = NULL;
    
    while ((obj = cfgIter++)) {
        if (obj->hasChanged()) {
#if 0
            fprintf(stderr, "Obj %p changed\n",
                    obj);
            relocatedInstruction *insn = dynamic_cast<relocatedInstruction *>(obj);
            baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(obj);
            trampEnd *end = dynamic_cast<trampEnd *>(obj);
            if (insn) fprintf(stderr, "insn\n");
            if (bti) fprintf(stderr, "bti\n");
            if (end) fprintf(stderr, "end\n");
#endif            
            return true;
        }
        else {
#if 0
            fprintf(stderr, "Obj %p not changed\n",
                    obj);
#endif
        }
    }
    
    return false;
}

relocatedInstruction::relocatedInstruction(const relocatedInstruction *parRI,
                                           multiTramp *cMT,
                                           process *child) :
    generatedCodeObject(parRI, child),
    origAddr(parRI->origAddr),
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
                   process *child) :
    generatedCodeObject(parEnd, child),
    multi_(cMT),
    target_(parEnd->target_)
{}
    

// Can end up copying this; return a new one.
generatedCodeObject *trampEnd::replaceCode(generatedCodeObject *obj) {
    multiTramp *newMulti = dynamic_cast<multiTramp *>(obj);
    assert(newMulti);
    
    return new trampEnd(multi_, target_);
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
            baseTrampInstance *bti = dynamic_cast<baseTrampInstance *>(cur_);
            relocatedInstruction *reloc = dynamic_cast<relocatedInstruction *>(cur_);
            trampEnd *te = dynamic_cast<trampEnd *>(cur_);
            if (bti)
                fprintf(stderr, "current is a base tramp\n");
            else if (reloc)
                fprintf(stderr, "current is relocated insn from 0x%lx\n",
                        reloc->origAddr);
            else if (te)
                fprintf(stderr, "current is tramp end\n");
            bti = dynamic_cast<baseTrampInstance *>(cur_->fallthrough_);
            reloc = dynamic_cast<relocatedInstruction *>(cur_->fallthrough_);
            te = dynamic_cast<trampEnd *>(cur_->fallthrough_);
            if (bti)
                fprintf(stderr, "next is a base tramp\n");
            else if (reloc)
                fprintf(stderr, "next is relocated insn from 0x%llx\n",
                        reloc->origAddr);
            else if (te)
                fprintf(stderr, "next is tramp end\n");

            bti = dynamic_cast<baseTrampInstance *>(cur_->fallthrough_->previous_);
            reloc = dynamic_cast<relocatedInstruction *>(cur_->fallthrough_->previous_);
            te = dynamic_cast<trampEnd *>(cur_->fallthrough_->previous_);
            if (bti)
                fprintf(stderr, "next->previous is a base tramp\n");
            else if (reloc)
                fprintf(stderr, "next->previous is relocated insn from 0x%llx\n", 
                        reloc->origAddr);
            else if (te)
                fprintf(stderr, "next->previous is tramp end\n");
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
            fprintf(stderr, "ERROR: current address 0x%x != previous address 0x%x\n",
                    gen.currAddr(baseInMutatee), addrInMutatee_);
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
                                 codeRange *range) {
    // range is optional, and might be NULL. It's provided to avoid
    // having to do two billion codeRange lookups
    if (range == NULL)
        range = proc()->findCodeRangeByAddress(pc);
    // Oopsie...
    if (!range) assert(0);

    multiTramp *rangeMulti = range->is_multitramp();
    miniTrampInstance *rangeMTI = range->is_minitramp();

    assert((rangeMulti != NULL) || (rangeMTI != NULL));

    if (rangeMTI) {
        assert(rangeMTI->baseTI->multiT == this);
        
        // Check to see if we're at an equivalent miniTramp. If not, 
        // fall through
        if (rangeMTI->mini->instP == newMT->instP) {
            // This is easier
            return miniTramp::catchupRequired(rangeMTI->mini, newMT);
        }
        else {
            rangeMulti = rangeMTI->baseTI->multiT;
            // And we need to fake the PC. Good thing
            // it's passed by value...
            // Pick the post address; doesn't really matter
            pc = rangeMTI->baseTI->trampPostAddr();
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
            (pc < insn->relocAddr() + insn->get_size_cr())) {
            assert(pcObj == NULL);
            pcObj = insn;
        }
        else if (bti) {
            // Can be either the instPoint or the original instruction,
            // so check both
            if (bti->isInInstance(pc)) {
                assert(pcObj == NULL);
                pcObj = bti;
            }
            if (bti->baseT == newMT->baseT)
                newBTI = bti;
        }
        else if (end) {
            if ((end->addrInMutatee_ <= pc) &&
                (pc < (end->addrInMutatee_ + end->size_))) {
                assert(pcObj == NULL);
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
        if (pc < (newBTI->trampPreAddr() + newBTI->baseT->preSize))
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
                                        UNW_INFO_TYPE ** unwindInformation ) {
    if (alreadyGenerated(gen, baseInMutatee))
        return true;
    generateSetup(gen, baseInMutatee);

    // addrInMutatee_ == base for this insn
    if (!insn->generate(gen,
                        multiT->proc(),
                        origAddr,
                        addrInMutatee_,
                        0, // fallthrough is not overridden
                        targetOverride_)) {
        fprintf(stderr, "WARNING: returned false from relocate insn (orig at 0x%lx, now 0x%lx)\n",
                origAddr, addrInMutatee_);
        return false;
    }

#if defined(arch_sparc) 
    // We pin delay instructions.
    if (insn->isDCTI()) {
        if (ds_insn) {
            inst_printf("... copying delay slot\n");
            ds_insn->generate(gen);
        }
        if (agg_insn) {
            inst_printf("... copying aggregate\n");
            agg_insn->generate(gen);
        }
    }
#endif

    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;
    
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
    while (gen.used() < instSize()) {
        Address origAddr = gen.currAddr(instAddr_);
        Address addrInMulti = uninstToInstAddr(origAddr);
        if (addrInMulti) {
            // addrInMulti may be 0 if our trap instruction does not
            // map onto a real instruction
#if defined(arch_x86) || defined(arch_x86_64)
            // x86: traps read at PC + 1
            proc()->trampTrapMapping[origAddr+1] = addrInMulti;
#else
            proc()->trampTrapMapping[origAddr] = addrInMulti;
#endif
        }
        instruction::generateTrap(gen);
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
    if (instSize_ < instruction::jumpSize(instAddr_, trampAddr_)) {
        branchSize_ = instruction::jumpSize(instAddr_, trampAddr_);
        return true;
    }
#if defined(arch_sparc)
    int dist = (trampAddr_ - instAddr_);
    if (!instruction::offsetWithinRangeOfBranchInsn(dist) &&
        func()->is_o7_live()) {
        // We can't use our multi-insn jump, so we must trap
        return false;
    }
#endif

    instruction::generateBranch(gen, instAddr_, trampAddr_);

    branchSize_ = gen.used() - origUsed;

    return true;
}

bool multiTramp::generateTrapToTramp(codeGen &gen) {
    // We're doing a trap. Now, we know that trap addrs are reported
    // as "finished" address... so use that one (not instAddr_)
#if defined(arch_x86) || defined(arch_x86_64)
    proc()->trampTrapMapping[gen.currAddr(instAddr_)+1] = trampAddr_;
#else
    proc()->trampTrapMapping[gen.currAddr(instAddr_)] = trampAddr_;
#endif
    unsigned start = gen.used();
    instruction::generateTrap(gen);
    branchSize_ = gen.used() - start;

    usedTrap_ = true;

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
    // todo: in-line :)
    return instruction::maxJumpSize();
}
