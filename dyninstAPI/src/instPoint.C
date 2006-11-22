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

// $Id: instPoint.C,v 1.30 2006/11/22 04:03:21 bernat Exp $
// instPoint code


#include <assert.h>
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
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emitter.h"
#if defined(arch_x86_64)
// For 32/64-bit mode knowledge
#include "dyninstAPI/src/emit-x86.h"
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

bool instPoint::replaceCode(AstNode *&ast) {
    // We inject a "replacedInstruction" into all known multitramps,
    // then trigger generation and installation.
    
    // Actually, the multiTramp is set up to pull information out
    // of the instPoints (rather than "push"), so we set up our 
    // data structures and let 'er rip.

    replacedCode_ = ast;
    
    if (!generateInst())
        return false;

    if (!installInst())
        return false;

    if (!linkInst())
        return false;

    return true;
}

// Get the appropriate base tramp structure. Cannot rely on
// multiTramps existing.

baseTramp *instPoint::getBaseTramp(callWhen when) {
  switch(when) {
  case callPreInsn:
    if (!preBaseTramp_) {
      preBaseTramp_ = new baseTramp(this);
    }
    return preBaseTramp_;
    break;
  case callPostInsn:
    if (!postBaseTramp_) {
      postBaseTramp_ = new baseTramp(this);
    }
    return postBaseTramp_;
    break;
  case callBranchTargetInsn:
    if (!targetBaseTramp_) {
      targetBaseTramp_ = new baseTramp(this);
    }
    return targetBaseTramp_;
    break;
  default:
    assert(0);
    break;
  }
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

  int_function *func = proc->findFuncByAddr(addr);
  // TODO: multiple functions... 
  if (!func) return NULL;

  inst_printf("Creating arbitrary point at 0x%x\n", addr);
  instPoint *newIP = func->findInstPByAddr(addr);
  if (newIP) return newIP;

  // Check to see if we're creating the new instPoint on an
  // instruction boundary. First, get the instance...

    bblInstance *bbl = func->findBlockInstanceByAddr(addr);
    if (!bbl) {
        inst_printf("Address not in known code, ret null\n");
        fprintf(stderr, "%s[%d]: Address not in known code, ret null\n", FILE__, __LINE__);
        return NULL;
    }
    int_basicBlock *block = bbl->block();
    assert(block);

    // Some blocks cannot be relocated; since instrumentation requires
    // relocation of the block, don't even bother.
    if(!block->llb()->canBeRelocated())
    {
        inst_printf("Address is in unrelocatable block, ret null\n");
        return NULL;
    }    

    // For now: we constrain the address to be in the original instance
    // of the basic block.
    if (block->origInstance() != bbl) {
        fprintf(stderr, "%s[%d]: Address not in original basic block instance\n", FILE__, __LINE__);
        return NULL;
    }

    InstrucIter newIter(bbl);
    while ((*newIter) < addr) newIter++;
    if (*newIter != addr) {
        inst_printf("Unaligned try for instruction iterator, ret null\n");
        fprintf(stderr, "%s[%d]: Unaligned try for instruction iterator, ret null\n", FILE__, __LINE__);
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
    ip->func()->registerInstPointAddr(ip->addr(), ip);

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
        reloc_printf("DEBUG: func version %d, our version %d, block instances %d, our instances %d\n",
                func()->version(), funcVersion, 
                block()->instances().size(),
                instances.size());
        const pdvector<bblInstance *> &bbls = block()->instances();
        assert(bbls.size() <= instances.size());
        for (unsigned i = instances.size(); i > bbls.size(); i--) {
            instPointInstance *inst = instances[i-1];
            instances.pop_back();
            // Delete...
            func()->unregisterInstPointAddr(inst->addr(), this);
        }

        // Safety check....
        for (unsigned i = 0; i < instances.size(); i++) {
            reloc_printf("%s[%d]: checking IPI block %p against block %p, entry %d\n",
                         FILE__, __LINE__, instances[i]->block(), bbls[i], i);
            assert(instances[i]->block() == bbls[i]);
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
                func()->registerInstPointAddr(newAddr, this);
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

// Return value... we used to return false if any generation failed. However,
// if we relocate a function then generating into the entry point of the original
// will fail, and that's 'bad'. So we now return true if anyone succeeded.

bool instPoint::generateInst() {
    updateInstances();

    bool success = false;
    for (unsigned i = 0; i < instances.size(); i++) {
        // I was using |=, but it looked like it was getting short-cutted?
        bool ret = instances[i]->generateInst();
        if (ret) success = true;
    }
    return success;
}

// See above return value comment...

bool instPoint::installInst() {
    bool success = false;
    for (unsigned i = 0; i < instances.size(); i++) {
        bool ret = instances[i]->installInst();
        if (ret) success = true;
    }
    return success;
}

// Return false if the PC is within the jump range of any of our
// multiTramps
bool instPoint::checkInst(pdvector<Address> &checkPCs) 
{
    
    for (unsigned sI = 0; sI < checkPCs.size(); sI++) {
        Address pc = checkPCs[sI];
        for (unsigned iI = 0; iI < instances.size(); iI++) {
            multiTramp *mt = instances[iI]->multi();
            // No multi -> not installed.
            if (!mt) continue;
            if ((pc > mt->instAddr()) &&
                (pc < (mt->instAddr() + mt->instSize()))) {
                // We have a conflict. Now, we may still be able to make this 
                // work; if we're not conflicting on the actual branch, we
                // may have trap-filled the remainder which allows us to
                // catch and transfer.
                if (pc < (mt->instAddr() + mt->branchSize())) {
                    // We're in the jump area, conflict.
                    fprintf(stderr, "MT conflict (MT from 0x%p to 0x%p, 0x%p to 0x%p dangerous), PC 0x%p\n",
                            (void *)mt->instAddr(),
                            (void *)(mt->instAddr() + mt->instSize()), 
                            (void *)mt->instAddr(),
                            (void *)(mt->instAddr() + mt->branchSize()),
                            (void *)pc);
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

// See comment in generateInst w.r.t. return value

bool instPoint::linkInst() {
    bool success = false;

    for (unsigned i = 0; i < instances.size(); i++) {
        bool ret = instances[i]->linkInst();
        if (ret) success = true;
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
    replacedCode_(NULL),
    proc_(proc),
    img_p_(NULL),
    block_(block),
    addr_(addr),
    actualGPRLiveSet_(NULL),
    actualFPRLiveSet_(NULL),
    actualSPRLiveSet_(NULL)
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
    replacedCode_(NULL),
    proc_(proc),
    img_p_(img_p),
    block_(block),
    addr_(addr),
    actualGPRLiveSet_(NULL),
    actualFPRLiveSet_(NULL),
    actualSPRLiveSet_(NULL)
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
    replacedCode_(NULL),
    proc_(child->proc()),
    img_p_(parP->img_p_),
    block_(child),
    addr_(parP->addr()),
    actualGPRLiveSet_(NULL),
    actualFPRLiveSet_(NULL),
    actualSPRLiveSet_(NULL)
 {
     assert(parP->replacedCode_ == NULL);
}
                  

instPoint *instPoint::createParsePoint(int_function *func,
                                       image_instPoint *img_p) {    
    // Now we need the addr and block so we can toss this to
    // commonIPCreation.

    inst_printf("Creating parse point for function %s, type %d\n",
                func->symTabName().c_str(),
                img_p->getPointType());


    Address offsetInFunc = img_p->offset() - img_p->func()->getOffset();
    Address absAddr = offsetInFunc + func->getAddress();

    instPoint *newIP = func->findInstPByAddr(absAddr);
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
    assert(proc);
    int_function *func = child->func();

    // Add to the process
    if (parP->instances.size() == 0) {
        // We created but never actually instrumented. However, 
        // we still need to let the function know we exist, mimicing the
        // behavior in commonIPcreation
        func->registerInstPointAddr(newIP->addr(), newIP);
    }
    else {
        for (unsigned i = 0; i < parP->instances.size(); i++) {
            instPointInstance *pI = parP->instances[i];
            instPointInstance *nI = new instPointInstance(pI->addr_,
                                                          child->instVer(i), 
                                                          newIP);
            // could also call child->func()->findBlockInstance...
            
            nI->multiID_ = pI->multiID_;
            newIP->instances.push_back(nI);
            func->registerInstPointAddr(pI->addr_, newIP);
        }
    }

    // And make baseTramp-age. If we share one, the first guy
    // waits and the second guy makes, so we can be absolutely
    // sure that we've made instPoints before we go trying to 
    // make baseTramps.
    
    baseTramp *parPre = parP->preBaseTramp_;
    if (parPre) {
        assert(parPre->instP() == parP);
        
	newIP->preBaseTramp_ = new baseTramp(parPre, proc);
	newIP->preBaseTramp_->instP_ = newIP;
    }


    baseTramp *parPost = parP->postBaseTramp_;
    if (parPost) {
        assert(parPost->instP() == parP);
        
	newIP->postBaseTramp_ = new baseTramp(parPost, proc);
	newIP->postBaseTramp_->instP_ = newIP;
    }

    baseTramp *parTarget = parP->targetBaseTramp_;
    if (parTarget) {
        assert(parTarget->instP() == parP);

        // Unlike others, can't share, so make now.
        newIP->targetBaseTramp_ = new baseTramp(parTarget, proc);
        newIP->targetBaseTramp_->instP_ = newIP;
    }

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

        // May not exist if instrumentation was overridden by (say)
        // a relocated function.
        if (!target->multi()) {
            continue;
        }
        
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
                                                       miniTramp *mt) 
{
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
  
    // Relocation is necessary if the containing block is shared. 
    // See if we're big enough to put the branch jump in. If not, trap.
    // Can also try to relocate; we'll still trap _here_, but another
    // ipInstance will be created that can jump.

    force_reloc.clear();

    if (block_->block()->needsRelocation() ||
        errCode == multiTramp::mtTryRelocation) 
    {
        // We can try to simply shift the entire function nearer
        // instrumentation. TODO: a funcMod that says "hey, move me to
        // this address.
        reloc_printf("Trying relocation, %d, %d %s\n", 
                     block_->getSize(),
                     multi()->sizeDesired(),
                     block_->block()->needsRelocation() ? 
                     "(block req reloc)" : "");
        
        if (block_->getSize() < multi()->sizeDesired()) {
            // expandForInstrumentation will append to the expand list, so can 
            // be called multiple times without blowing up
            if (func()->expandForInstrumentation()) {
                // and relocationGenerate invalidates old, "in-flight" versions.
                func()->relocationGenerate(func()->enlargeMods(), 0, 
                                           force_reloc);
            }
        }
        else {
            // Use func's enlargeMods so that we don't regress the 
            // relocated state.
            func()->relocationGenerate(func()->enlargeMods(), 
                                       0, force_reloc);
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

    // the original relocation may force others; install them too
    for(unsigned i=0; i < force_reloc.size(); i++)
    {
        force_reloc[i]->relocationInstall();
    }
#endif

    if (!multi()) {
        // Alternative: keep a set of sequence #s for generated/
        // installed/linked. We tried to generate and failed (prolly
        // due to stepping on a relocated function), so fail here
        // but don't assert.
        return false;
    }

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

    for(unsigned i=0; i < force_reloc.size(); i++)
    {
        force_reloc[i]->relocationLink(overwrittenObjs);
    }
#endif


    // Funny thing is, we might very well try to link a multiTramp
    // multiple times...
    // Ah, well.
    // See comment in installInst
    if (!multi()) return false;
    
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

Address instPoint::callTarget() const {
    if (img_p_->callTarget() == 0) return 0;

    // We can have an absolute addr... lovely.
    if (img_p_->targetIsAbsolute())
        return img_p_->callTarget();

    // Return the shifted kind...
    return img_p_->callTarget() + func()->obj()->codeBase();
}

int *instPoint::optimisticGPRLiveSet_ = NULL;
int *instPoint::optimisticFPRLiveSet_ = NULL;
int *instPoint::optimisticSPRLiveSet_ = NULL;
int *instPoint::pessimisticGPRLiveSet_ = NULL;
int *instPoint::pessimisticFPRLiveSet_ = NULL;
int *instPoint::pessimisticSPRLiveSet_ = NULL;

#if defined(arch_x86_64)
int *instPoint::optimisticGPRLiveSet64_ = NULL;
int *instPoint::optimisticFPRLiveSet64_ = NULL;
int *instPoint::optimisticSPRLiveSet64_ = NULL;
int *instPoint::pessimisticGPRLiveSet64_ = NULL;
int *instPoint::pessimisticFPRLiveSet64_ = NULL;
int *instPoint::pessimisticSPRLiveSet64_ = NULL;
#endif

bool instPoint::hasSpecializedGPRegisters() const {
    return (actualGPRLiveSet_ != NULL);
}
bool instPoint::hasSpecializedFPRegisters() const {
    return (actualFPRLiveSet_ != NULL);
}
bool instPoint::hasSpecializedSPRegisters() const {
    return (actualSPRLiveSet_ != NULL);
}

int *instPoint::liveGPRegisters() const {
    if (actualGPRLiveSet_) return actualGPRLiveSet_;

    // If we're entry/exit/call site, return the optimistic version
    if (getPointType() == functionEntry)
        return optimisticGPRLiveSet();
    if (getPointType() == functionExit)
        return optimisticGPRLiveSet();
    if (getPointType() == callSite)
        return optimisticGPRLiveSet();

    return pessimisticGPRLiveSet();
}

int *instPoint::liveFPRegisters() const {
    if (actualFPRLiveSet_) return actualFPRLiveSet_;

    // If we're entry/exit/call site, return the optimistic version
    if (getPointType() == functionEntry)
        return optimisticFPRLiveSet();
    if (getPointType() == functionExit)
        return optimisticFPRLiveSet();
    if (getPointType() == callSite)
        return optimisticFPRLiveSet();

    return pessimisticFPRLiveSet();
}

int *instPoint::liveSPRegisters() const {
    if (actualSPRLiveSet_) return actualSPRLiveSet_;

    // If we're entry/exit/call site, return the optimistic version
    if (getPointType() == functionEntry)
        return optimisticSPRLiveSet();
    if (getPointType() == functionExit)
        return optimisticSPRLiveSet();
    if (getPointType() == callSite)
        return optimisticSPRLiveSet();

    return pessimisticSPRLiveSet();
}

int *instPoint::optimisticGPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return optimisticGPRLiveSet64_;
    }
#endif
    return optimisticGPRLiveSet_;
}

int *instPoint::optimisticFPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return optimisticFPRLiveSet64_;
    }
#endif
    return optimisticFPRLiveSet_;
}

int *instPoint::optimisticSPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return optimisticSPRLiveSet64_;
    }
#endif
    return optimisticSPRLiveSet_;
}


int *instPoint::pessimisticGPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return pessimisticGPRLiveSet64_;
    }
#endif
    return pessimisticGPRLiveSet_;
}
int *instPoint::pessimisticFPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return pessimisticFPRLiveSet64_;
    }
#endif
    return pessimisticFPRLiveSet_;
}

int *instPoint::pessimisticSPRLiveSet() {
#if defined(arch_x86_64)
    if (code_emitter == &emitter64) {
        return pessimisticSPRLiveSet64_;
    }
#endif
    return pessimisticSPRLiveSet_;
}
