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

// $Id: instPoint.C,v 1.11 2005/12/01 00:56:24 jaw Exp $
// instPoint code


#include <assert.h>
//#include <sys/signal.h>
//#include <sys/param.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/arch.h"


#ifndef BPATCH_LIBRARY
#include "paradynd/src/init.h"
#endif

unsigned int instPointBase::id_ctr = 1;

dictionary_hash <pdstring, unsigned> primitiveCosts(pdstring::hash);

#if defined(rs6000_ibm_aix4_1)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

miniTramp *instPoint::addInst(AstNode *&ast,
                              callWhen when,
                              callOrder order,
                              bool trampRecursive,
                              bool noCost) {
    // This only adds a new minitramp; code generation and any actual
    // _work_ is put off until later.

    baseTramp *baseT = getBaseTramp(when);
    if (!baseT) return NULL;

    // Will complain if we have multiple miniTramps that don't agree
    baseT->setRecursive(trampRecursive);
    
    miniTramp *miniT = new miniTramp(when,
                                     ast,
                                     baseT,
                                     this,
                                     noCost);
    
    assert (miniT);
    
    // Sets prev and next members of the mt
    if (!baseT->addMiniTramp(miniT, order)) {
        inst_printf("Basetramp failed to add miniTramp, ret false\n");
        delete miniT;
        return NULL;
    }
    
    return miniT;
}

// Get the appropriate base tramp structure. Cannot rely on
// multiTramps existing.

baseTramp *instPoint::getBaseTramp(callWhen when) {
    if (when == callPreInsn) {
        if (preBaseTramp_)
            return preBaseTramp_;
    }
    else if (when == callPostInsn) {
        if (postBaseTramp_)
            return postBaseTramp_;
    }
    else if (when == callBranchTargetInsn) {
        if (targetBaseTramp_)
            return targetBaseTramp_;
    }

    // We do the if tree twice to keep the function structure clean
    /////////////////////////////////////////////////
    // See if there's already a baseTramp out there...
    // We see if there is a neighboring instPoint, and then
    // check it's pre/post tramp directly. If we called getTramp,
    // we'd hit recursion....

    // Only do this for pre/post; target by definition cannot share.
    // Also, if we're the first/last insn and looking for pre/post,
    // don't bother...

    // This is by multiTramp footprint, not actually a basic
    // block. Even if we have a neighbor, if they're in a different
    // multiTramp, we _really_ don't want to be sharing
    // baseTramps. It's a limitation of the current system,
    // actually... but for now I'll be pessimistic.

    Address mtStartAddr;
    unsigned mtSize;
    if (!multiTramp::getMultiTrampFootprint(addr(),
                                            proc(),
                                            mtStartAddr,
                                            mtSize)) {
        return NULL;
    }

    // Time to iterate
    InstrucIter insnIter(mtStartAddr,
                         mtSize,
                         proc());
    // On x86 this crawls the block so that we can figure out
    // previous instructions. Other platforms are easier.
    insnIter.setCurrentAddress(addr());

    Address neighborAddr = 0;
    if (when == callPreInsn) {
        neighborAddr = insnIter.peekPrev();
    }
    else if (when == callPostInsn) {
        neighborAddr = insnIter.peekNext();
    }
    else {
        assert(when == callBranchTargetInsn);
        neighborAddr = 0;
    }
    // Check for illegal values - outside the MT
    if ((neighborAddr < mtStartAddr) ||
        (neighborAddr >= (mtStartAddr + mtSize)))
        neighborAddr = 0;
    
    if (neighborAddr) {
        instPoint *neighbor = proc()->findInstPByAddr(neighborAddr);
        if (neighbor) {
            if (when == callPreInsn) {
                // Our pre is their post
                preBaseTramp_ = neighbor->postBaseTramp_;
                if (preBaseTramp_) {
                    // Safety checking and invariant maintenance
                    assert(!preBaseTramp_->preInstP);
                    preBaseTramp_->preInstP = this;
                }
            }
            else {
                assert(when == callPostInsn);
                postBaseTramp_ = neighbor->preBaseTramp_;
                if (postBaseTramp_) {
                    assert(!postBaseTramp_->postInstP);
                    postBaseTramp_->postInstP = this;
                }
            }
        }
    }
    
    // We now may have a base tramp; check again, make if not, and return.

    if (when == callPreInsn) {
        if (preBaseTramp_) return preBaseTramp_;
        preBaseTramp_ = new baseTramp();
        preBaseTramp_->preInstP = this;
        return preBaseTramp_;
    }
    else if (when == callPostInsn) {
        if (postBaseTramp_) return postBaseTramp_;
        postBaseTramp_ = new baseTramp();
        postBaseTramp_->postInstP = this;
        return postBaseTramp_;
    }
    else {
        assert(!targetBaseTramp_);
        assert(when == callBranchTargetInsn);
        targetBaseTramp_ = new baseTramp();
        targetBaseTramp_->postInstP = this;
        return targetBaseTramp_;
    }
    
    assert(0);
    return NULL;
}

bool instPoint::match(Address a) const { 
    if (a == addr()) return true;

    for (unsigned i = 0; i < instances.size(); i++)
        if (instances[i]->addr() == a)
            return true;
    return false;
}

instPoint *instPoint::createArbitraryInstPoint(Address addr, process *proc) {
  // See if we get lucky

    inst_printf("Creating arbitrary point at 0x%x\n", addr);
    instPoint *newIP = proc->findInstPByAddr(addr);
    if (newIP) return newIP;

    // Check to see if we're creating the new instPoint on an
    // instruction boundary. First, find the containing function.
    codeRange *range = proc->findCodeRangeByAddress(addr);
    if (!range) {
        inst_printf("Failed to find address, ret null\n");
        fprintf(stderr, "%s[%d]: Failed to find address, ret null\n", FILE__, __LINE__);
        return NULL;
    }
    bblInstance *bbl = range->is_basicBlockInstance();
    if (!bbl) {
        inst_printf("Address not in known code, ret null\n");
        fprintf(stderr, "Address not in known code, ret null\n", FILE__, __LINE__);
        return NULL;
    }
    int_basicBlock *block = bbl->block();
    assert(block);
    // For now: we constrain the address to be in the original instance
    // of the basic block.
    if (block->origInstance() != bbl) {
        fprintf(stderr, "Address not in original basic block instance\n", FILE__, __LINE__);
        return NULL;
    }
    int_function *func = bbl->func();
    assert(func); // If we're in a basic block, we have to be able to follow it back.

    InstrucIter newIter(bbl);
    while ((*newIter) < addr) newIter++;
    if (*newIter != addr) {
        inst_printf("Unaligned try for instruction iterator, ret null\n");
        fprintf(stderr, "Unaligned try for instruction iterator, ret null\n", FILE__, __LINE__);
        return NULL; // Not aligned
    }
#if defined(arch_sparc)
    // Can't instrument delay slots
    if (newIter.hasPrev()) {
        if (newIter.getPrevInstruction().isDCTI()) {
            inst_printf("%s[%d]:  can't instrument delay slot\n", FILE__, __LINE__);
            fprintf(stderr, "%s[%d]:  can't instrument delay slot\n", FILE__, __LINE__);
            return NULL;
        }
    }
#endif
    
    newIP = new instPoint(proc,
                          newIter.getInstruction(),
                          addr,
                          block);
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        inst_printf("Failed common IP creation, ret null\n");
        return NULL;
    }
    
    func->addArbitraryPoint(newIP);

    return newIP;
}
 
bool instPoint::commonIPCreation(instPoint *ip) {
    // Delay until generation....
    //newIP->updateInstances();

    // But tell people we exist.
    ip->proc()->registerInstPointAddr(ip->addr(), ip);

#if 0
    // Now handled in getBaseTramp_, left here
    // for commenting and clarity
    if (newIter.isACondBranchInstruction() || 
        newIter.isAJumpInstruction()) {
        inst_printf("Jump instruction requires target BT at addr 0x%x\n", *newIter);
        // In this case, also set the target baseTramp
        // Should also assert we're the last person in the basicBlock, actually.
        assert(!newIP->targetBaseTramp_);
        newIP->targetBaseTramp_ = new baseTramp();
    }
    
    
    // Set backchain pointers
    if (newIP->preBaseTramp_)
        newIP->preBaseTramp_->preInstP = newIP;
    if (newIP->postBaseTramp_)
        newIP->postBaseTramp_->postInstP = newIP;
    if (newIP->targetBaseTramp_)
        newIP->targetBaseTramp_->postInstP = newIP;
    
    // Okay, so we have an aligned address. Perform the following steps:
    // Make a new instPoint
    
    // Register us by address with the process -- good for finding other
    // instPoints
    proc()->registerInstPointAddr(newIP->addr(), newIP);
#endif

    //// Do we want this? proc->registerNewInstPoint(original, newIP, ...);
    return true;
}

// We do on-the-fly generation of Instances... as a result, we may have
// versions of functions (frex, relocation) that do not have associated
// Instances. This function updates the Instance list and makes sure
// that everything works right.

bool instPoint::updateInstances() {
    unsigned i;

    if (func()->version() == funcVersion)
        return true;
    else if (func()->version() < funcVersion) {
        const pdvector<bblInstance *> &bbls = block()->instances();
        assert(bbls.size() < instances.size());
        for (unsigned i = instances.size(); i > bbls.size(); i--) {
            instPointInstance *inst = instances[i-1];
            instances.pop_back();
            // Delete...
            proc()->unregisterInstPointAddr(inst->addr(), this);
        }
        funcVersion = func()->version();
        return true;
    }
    else {
        // For each instance of the basic block we're attached to,
        // make an instPointInstance with the appropriate addr
        // Might be smaller as well...
        
        const pdvector<bblInstance *> &bbls = block()->instances();
        assert(instances.size() < bbls.size());
        for (i = instances.size(); i < bbls.size(); i++) {
            bblInstance *bblI = bbls[i];
            
            Address newAddr = bblI->equivAddr(block()->origInstance(), addr());

            // However, check if we can do a multiTramp at this point (as we may have
            // overwritten with a jump). If we can't, then skip the instance.
            unsigned multiID_ = multiTramp::findOrCreateMultiTramp(newAddr, proc());
            if (multiID_) {
                instPointInstance *ipInst = new instPointInstance(newAddr,
                                                                  bblI,
                                                                  this);
                
                instances.push_back(ipInst);
                // Register with the process before asking for a multitramp
                inst_printf("Registering IP %p at 0x%lx (%d)\n",
                            this, newAddr, i);
                proc()->registerInstPointAddr(newAddr, this);
            }
        }
        
        // We need all instances to stay in step; so if the first (default)
        // instance is generated/installed/linked, then make sure any new
        // instances are the same.
        bool generated = false;
        bool installed = false;
        bool linked = false;
        
        if (instances[0]->multi()) {
            generated = instances[0]->multi()->generated();
            installed = instances[0]->multi()->installed();
            linked = instances[0]->multi()->linked();
        }
        
        // Check whether there's something at my address...
        for (i = 0; i < instances.size(); i++) {

            if (!instances[i]->multi()) {
                instances[i]->multiID_ = multiTramp::findOrCreateMultiTramp(instances[i]->addr(),
                                                                            proc());
                if (instances[i]->multi()) {
                    if (generated) {
                        instances[i]->multi()->generateMultiTramp();
                    }
                    if (installed) {
                        instances[i]->multi()->installMultiTramp();
                    }
                    if (linked) {
                        instances[i]->multi()->linkMultiTramp();
                    }
                }
                /*
                  // Not actually an error anymore, leaving in for debug purposes
                  else {
                  fprintf(stderr, "ERROR: instance %p, addr 0x%lx, IP %p, no multitramp!\n",
                  instances[i], instances[i]->addr(), this);
                  }
                */
            }
        }
        funcVersion = func()->version();
    }
    return true;
}

// Blah blah blah...
miniTramp *instPoint::instrument(AstNode *ast,
                                 callWhen when,
                                 callOrder order,
                                 bool trampRecursive,
                                 bool noCost) {
    
    miniTramp *mini = addInst(ast, when, order, trampRecursive, noCost);
    if (!mini) {
        cerr << "instPoint::instrument: failed addInst, ret NULL" << endl;
        return NULL;
    }

    if (!generateInst()) {
        cerr << "instPoint::instrument: failed generateInst, ret NULL" << endl;
        return NULL;
    }

    if (!installInst()) {
        cerr << "instPoint::instrument: failed installInst, ret NULL" << endl;
        return NULL;
    }

    if (!linkInst()) {
        cerr << "instPoint::instrument: failed linkInst, ret NULL" << endl;
        return NULL;
    }

    return mini;
}

// Generate the correct code for all baseTramps touched by the
// miniTramps at this point. May cause regeneration of the multiTramps
// as well. Complicated.
// The code generated by this is explicitly _not_ injected into the process;
// that is, while it might be copied into the address space, it is not linked
// by any jumps. This allows for us to atomically add multiple different
// instPoints at the same time.

bool instPoint::generateInst() {
    updateInstances();

    bool success = true;
    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->generateInst()) {
            fprintf(stderr, "Failed generation for instance %d!\n",
                    i);
            success = false;
        }
    }
    return success;
}

bool instPoint::installInst() {
    bool success = true;
    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->installInst()) {
            success = false;
        }
    }
    return success;
}

// Return false if the PC is within the jump range of any of our
// multiTramps
bool instPoint::checkInst(pdvector<Address> &checkPCs) {
    
    for (unsigned sI = 0; sI < checkPCs.size(); sI++) {
        Address pc = checkPCs[sI];
        for (unsigned iI = 0; iI < instances.size(); iI++) {
            multiTramp *mt = instances[iI]->multi();
            if ((pc > mt->instAddr()) &&
                (pc < (mt->instAddr() + mt->instSize()))) {
                // We have a conflict. Now, we may still be able to make this 
                // work; if we're not conflicting on the actual branch, we
                // may have trap-filled the remainder which allows us to
                // catch and transfer.
                if (pc < (mt->instAddr() + mt->branchSize())) {
                    // We're in the jump area, conflict.
                    fprintf(stderr, "MT conflict (MT from 0x%x to 0x%x, 0x%x to 0x%x dangerous), PC 0x%x\n",
                            mt->instAddr(),
                            mt->instAddr() + mt->instSize(), 
                            mt->instAddr(),
                            mt->instAddr() + mt->branchSize(),
                            pc);
                    return false;
                }
            }
        }
    }
#if defined(cap_relocation)
    // Yay check relocation
    if (!func()->relocationCheck(checkPCs))
        return false;
#endif

    return true;
}

bool instPoint::linkInst() {
    bool success = true;

    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->linkInst()) {
            fprintf(stderr, "instance %d failed link\n",
                    i);
            success = false;
        }
    }
    return success;
}

instPointInstance *instPoint::getInstInstance(Address addr) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->addr() == addr)
            return instances[i];
    }
    return NULL;
}

int instPoint_count = 0;

instPoint::instPoint(process *proc,
                     instruction insn,
                     Address addr,
                     int_basicBlock *block) :
    instPointBase(insn, otherPoint),
    funcVersion(-1),
    callee_(NULL),
    isDynamicCall_(false),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    proc_(proc),
    img_p_(NULL),
    block_(block),
    addr_(addr),
    liveRegisters(NULL),
    liveFPRegisters(NULL),
    liveSPRegisters(NULL)

{
#if defined(ROUGH_MEMORY_PROFILE)
    instPoint_count++;
    if ((instPoint_count % 10) == 0)
        fprintf(stderr, "instPoint_count: %d (%d)\n",
                instPoint_count, instPoint_count*sizeof(instPoint));
#endif
}

// Process specialization of a parse-time instPoint
instPoint::instPoint(process *proc,
                     image_instPoint *img_p,
                     Address addr,
                     int_basicBlock *block) :
    instPointBase(img_p->insn(),
                  img_p->getPointType(),
                  img_p->id()),
    funcVersion(-1),
    callee_(NULL),
    isDynamicCall_(img_p->isDynamicCall()),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    proc_(proc),
    img_p_(img_p),
    block_(block),
    addr_(addr),
    liveRegisters(NULL),
    liveFPRegisters(NULL),
    liveSPRegisters(NULL)
{
#if defined(ROUGH_MEMORY_PROFILE)
    instPoint_count++;
    if ((instPoint_count % 10) == 0)
        fprintf(stderr, "instPoint_count: %d (%d)\n",
                instPoint_count, instPoint_count*sizeof(instPoint));
#endif
}

// Copying over from fork
instPoint::instPoint(instPoint *parP,
                     int_basicBlock *child) :
    instPointBase(parP->insn(),
                  parP->getPointType(),
                  parP->id()),
    funcVersion(parP->funcVersion),
    callee_(NULL), // Will get set later
    isDynamicCall_(parP->isDynamicCall_),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    proc_(child->proc()),
    img_p_(parP->img_p_),
    block_(child),
    addr_(parP->addr()),
    liveRegisters(NULL),
    liveFPRegisters(NULL),
    liveSPRegisters(NULL)
 {
}
                  

instPoint *instPoint::createParsePoint(int_function *func,
                                       image_instPoint *img_p) {    
    process *proc = func->proc();
    // Now we need the addr and block so we can toss this to
    // commonIPCreation.

    inst_printf("Creating parse point for function %s, type %d\n",
                func->symTabName().c_str(),
                img_p->getPointType());


    Address offsetInFunc = img_p->offset() - img_p->func()->getOffset();
    Address absAddr = offsetInFunc + func->getAddress();

    instPoint *newIP = proc->findInstPByAddr(absAddr);
    if (newIP) return newIP;

    inst_printf("Parsed offset: 0x%x, in func 0x%x, absolute addr 0x%x\n",
                img_p->offset(),
                offsetInFunc,
                absAddr);
    
    int_basicBlock *block = func->findBlockByAddr(absAddr);
    if (!block) return false; // Not in the function...
    assert(block);

    newIP = new instPoint(func->proc(),
                          img_p,
                          absAddr,
                          block);
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        return NULL;
    }

    return newIP;
}

instPoint *instPoint::createForkedPoint(instPoint *parP, int_basicBlock *child) {
    // Make a copy of the parent instPoint. We don't have multiTramps yet,
    // which is okay; just get the ID right.
    instPoint *newIP = new instPoint(parP, child);
    process *proc = child->proc();
    
    // Add to the process
    for (unsigned i = 0; i < parP->instances.size(); i++) {
        instPointInstance *pI = parP->instances[i];
        instPointInstance *nI = new instPointInstance(pI->addr_,
                                                      child->instVer(i), 
                                                      newIP);
        // could also call child->func()->findBlockInstance...

        nI->multiID_ = pI->multiID_;
        newIP->instances.push_back(nI);
        proc->registerInstPointAddr(pI->addr_, newIP);
    }

    // And make baseTramp-age. If we share one, the first guy
    // waits and the second guy makes, so we can be absolutely
    // sure that we've made instPoints before we go trying to 
    // make baseTramps.
    
    baseTramp *parPre = parP->preBaseTramp_;
    if (parPre) {
        assert(parPre->preInstP == parP);
        
        if (parPre->postInstP) {
            // We've already made this guy... so pick him up. Somehow.
            // Did I mention argh?
            Address nextAddr = parPre->postInstP->addr();
            assert(nextAddr > newIP->addr());
            instPoint *neighbor = proc->findInstPByAddr(nextAddr);
            if (neighbor) {
                // We're second. So make and fix up their side.
                newIP->preBaseTramp_ = new baseTramp(parPre, proc);
                newIP->preBaseTramp_->preInstP = newIP;
                neighbor->postBaseTramp_ = newIP->preBaseTramp_;
                neighbor->postBaseTramp_->postInstP = neighbor;
            }
            else {
                // We're first, so wait.
                // ...
            }
        }
        else {
            // Nobody else, so make a copy
            newIP->preBaseTramp_ = new baseTramp(parPre, proc);
            newIP->preBaseTramp_->preInstP = newIP;
        }
    }


    baseTramp *parPost = parP->postBaseTramp_;
    if (parPost) {
        assert(parPost->postInstP == parP);
        
        if (parPost->preInstP) {
            // We've already made this guy... so pick him up. Somehow.
            // Did I mention argh?
            Address prevAddr = parPre->preInstP->addr();
            assert(prevAddr < newIP->addr());
            instPoint *neighbor = proc->findInstPByAddr(prevAddr);
            if (neighbor) {
                // We're second. Make and fix
                newIP->postBaseTramp_ = new baseTramp(parPost, proc);
                newIP->postBaseTramp_->postInstP = newIP;
                neighbor->preBaseTramp_ = newIP->postBaseTramp_;
                neighbor->preBaseTramp_->postInstP = neighbor;
            }
            else {
                // First...
            }
        }
        else {
            // No neighbor
            newIP->postBaseTramp_ = new baseTramp(parPost, proc);
            newIP->postBaseTramp_->postInstP = newIP;
        }
    }

    baseTramp *parTarget = parP->targetBaseTramp_;
    if (parTarget) {
        assert(parTarget->postInstP == parP);

        // Unlike others, can't share, so make now.
        newIP->targetBaseTramp_ = new baseTramp(parTarget, proc);
        newIP->targetBaseTramp_->postInstP = newIP;
    }

    if (newIP->preBaseTramp_)
        newIP->preBaseTramp_->preInstP = newIP;

    if (newIP->postBaseTramp_)
        newIP->postBaseTramp_->postInstP = newIP;

    if (newIP->targetBaseTramp_)
        newIP->targetBaseTramp_->postInstP = newIP;


    // Still don't have the baseTramp structures... ARGH.
    return newIP;
}    
    
    
instPoint::~instPoint() {
    // TODO
}



bool instPoint::instrSideEffect(Frame &frame)
{
    bool modified = false;
    
    assert(instances.size());
    // We explicitly do not update instances here; we're
    // between installing and linking, so don't change the 
    // instInstance list.
    //updateInstances();
    
    for (unsigned i = 0; i < instances.size(); i++) {
        instPointInstance *target = instances[i];
        // Question: generalize into "if the PC is in instrumentation,
        // move to the equivalent address?" 
        // Sure....
        
        // See previous call-specific version below; however, this
        // _should_ work.

        Address newPC = target->multi()->uninstToInstAddr(frame.getPC());
        if (newPC) {
            if (frame.setPC(newPC))
                modified = true;
        }
        // That's if we want to move into instrumentation. Mental note:
        // check to see if we're handling return points correctly; we should
        // be. If calls ever end basic blocks, we'll have to fix this.
    }
    return modified;
}

instPoint::catchup_result_t instPoint::catchupRequired(Address pc,
                                                       miniTramp *mt) {
    // If the PC isn't in a multiTramp that corresponds to one of
    // our instances, return noMatch_c

    // If the PC is in an older version of the current multiTramp,
    // return missed
    
    // If we're in a miniTramp chain, then hand it off to the 
    // multitramp.

    // Otherwise, hand it off to the multiTramp....
    codeRange *range = proc()->findCodeRangeByAddress(pc);
    
    multiTramp *rangeMT = range->is_multitramp();
    miniTrampInstance *rangeMTI = range->is_minitramp();

    if ((rangeMT == NULL) &&
        (rangeMTI == NULL)) {
        // We _cannot_ be in the jump footprint. So we missed.
        return noMatch_c;
    }

    if (rangeMTI) {
        // Back out to the multiTramp for now
        rangeMT = rangeMTI->baseTI->multiT;
    }

    assert(rangeMT != NULL);

    unsigned curID = rangeMT->id();

    bool found = false;

    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->multiID() == curID) {
            found = true;
            // If not the same one, we replaced. Return missed.
            if (instances[i]->multi() != rangeMT) {
                return missed_c;
            }
            else {
                // It is the same one; toss into low-level logic
                if (rangeMT->catchupRequired(pc, mt, range))
                    return missed_c;
                else
                    return notMissed_c;
            }
            break;
        }
    }

    assert(!found);

    return noMatch_c;
}

int_basicBlock *instPoint::block() const { 
    assert(block_);
    return block_;
}

int_function *instPoint::func() const {
    return block()->func();
}





// allowTrap shouldn't really go here; problem is, 
// multiple instPoints might use the same multiTramp,
// and I'm not sure how to handle it.
bool instPointInstance::generateInst() {
    // Do all the hard work; this will create a complete
    // instrumentation structure and copy it into the address
    // space, but not link it up. We do that later. It may also
    // change the multiTramp we have a pointer to, though that's
    // handled through the multi() wrapper.    
    if (!multi()) {
        multiID_ = multiTramp::findOrCreateMultiTramp(addr(),
                                                      proc());
    }
    if (!multi()) {
        fprintf(stderr, "No multiTramp for instInstance %p, ret false\n", this);
        return false;
    }
    multiTramp::mtErrorCode_t errCode = multi()->generateMultiTramp();
    if (errCode == multiTramp::mtError) {
        return false;
    }
    // Can and will set our multiTramp ID if there isn't one already;
    // if there is, will reuse that slot in the multiTramp dictionary.
    // This allows us to regenerate multiTramps without having to 
    // worry about changing pointers.

#if defined(cap_relocation)

    // Moved from ::generate; we call ::generate multiple times, then ::install.
    // This allows us to relocate once per function.... 
    
    // See if we're big enough to put the branch jump in. If not, trap.
    // Can also try to relocate; we'll still trap _here_, but another
    // ipInstance will be created that can jump.
    if (errCode == multiTramp::mtTryRelocation) {
        // We can try to simply shift the entire function nearer
        // instrumentation. TODO: a funcMod that says "hey, move me to
        // this address.
        inst_printf("Trying relocation, %d, %d\n", 
                    block_->getSize(),
                    multi()->sizeDesired());
        if (((int) block_->getSize()) < multi()->sizeDesired()) {
            // expandForInstrumentation will append to the expand list, so can 
            // be called multiple times without blowing up
            if (func()->expandForInstrumentation()) {
                // and relocationGenerate invalidates old, "in-flight" versions.
                func()->relocationGenerate(func()->enlargeMods(), 0);
            }
        }
        else {
            pdvector<funcMod *> funcMods; // Empty since we're not trying to make changes.
            func()->relocationGenerate(funcMods, 0);
        }
    }
#endif
    
    return true;
    //return (errCode == multiTramp::mtSuccess);
}

bool instPointInstance::installInst() {

#if defined(cap_relocation)
    // This is harmless to call if there isn't a relocation in-flight
    func()->relocationInstall();
#endif

    assert(multi());
    // We now "install", that is copy the generated code into the 
    // addr space. This doesn't link.
    
    if (multi()->installMultiTramp() != multiTramp::mtSuccess) {
        fprintf(stderr, "Instance failed to install multiTramp, ret false\n");
        return false;
    }
    return true;
}

bool instPointInstance::linkInst() {
#if defined(cap_relocation)
    // This is ignored (for now), is handled in updateInstInstances...
    pdvector<codeRange *> overwrittenObjs;
    // This is harmless to call if there isn't a relocation in-flight
    func()->relocationLink(overwrittenObjs);
#endif

    // Funny thing is, we might very well try to link a multiTramp
    // multiple times...
    // Ah, well.
    assert(multi());
    
    if (multi()->linkMultiTramp() != multiTramp::mtSuccess) {
        fprintf(stderr, "ipInst: linkMulti returned false for 0x%lx\n",
                addr());
        return false;
    }
    return true;
}

multiTramp *instPointInstance::multi() const {
    if (multiID_ == 0)
        return NULL;
    return multiTramp::getMulti(multiID_, proc());
}

void instPointInstance::updateMulti(unsigned id) {
    if (multiID_)
        assert(id == multiID_);
    else
        multiID_ = id;
}

process *instPointInstance::proc() const {
    return point->proc();
}

int_function *instPointInstance::func() const {
    return point->func();
}

