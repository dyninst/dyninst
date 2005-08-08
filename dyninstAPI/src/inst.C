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

// $Id: inst.C,v 1.138 2005/08/08 22:39:24 bernat Exp $
// Code to install and remove instrumentation from a running process.

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

    inst_printf("instPoint %p, adding inst %d, %d\n",
                this, when, order);


    baseTramp *baseT = getBaseTramp(when);
    
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

/*
 * return the time required to execute the passed primitive.
 *
 */
unsigned getPrimitiveCost(const pdstring &name)
{

    static bool init=false;

    if (!init) { init = 1; initPrimitiveCost(); }

    if (!primitiveCosts.defines(name)) {
      return 1;
    } else
      return (primitiveCosts[name]);
}


// find any tags to associate semantic meaning to function
unsigned findTags(const pdstring ) {
  return 0;
#ifdef notdef
  if (tagDict.defines(funcName))
    return (tagDict[funcName]);
  else
    return 0;
#endif
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
        assert(0);
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
        postBaseTramp_->postInstP = this;
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

bool instPoint::usesTrap() const {
    // We report trap usage if all multiTs use a trap. This could also
    // be if one uses, or be removed entirely.

    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->multi() &&
            (!instances[i]->multi()->usesTrap()))
            return false;
    }
    return true;
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
        return NULL;
    }
    int_function *func = range->is_function();
    if (!func) {
        inst_printf("Address not in function, ret null\n");
        return NULL;
    }

    inst_printf("Point is offset %d (0x%x) from start of function %s\n",
                addr - func->getAddress(), 
                addr - func->getAddress(), 
                func->symTabName().c_str());
    
    int_basicBlock *block = func->findBlockByAddr(addr);
    if (!block) {
        inst_printf("Can't find block for addr, ret null\n");
        return NULL;
    }
    
    InstrucIter newIter(block);
    while ((*newIter) < addr) newIter++;
    if (*newIter != addr) {
        inst_printf("Unaligned try for instruction iterator, ret null\n");
        return NULL; // Not aligned
    }
#if defined(arch_sparc)
    // Can't instrument delay slots
    if (newIter.hasPrev()) {
        if (newIter.getPrevInstruction().isDCTI()) {
            return NULL;
        }
    }
#endif
    
    newIP = new instPoint(proc,
                          newIter.getInstruction(),
                          addr,
                          func);
    
    if (!commonIPCreation(newIP,
                          block)) {
        delete newIP;
        inst_printf("Failed common IP creation, ret null\n");
        return NULL;
    }
    
    func->addArbitraryPoint(newIP);
    
    return newIP;
}
 
bool instPoint::commonIPCreation(instPoint *newIP,
                                 int_basicBlock * /*block*/) {

    newIP->updateInstances();

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
    unsigned funcVer = func()->version();
    
    if (funcVer == funcVersion) return true;

    // For each copy of the function:
    //   make an instPointInstance
    //   check for an existing multiTramp
    // By convention, instances[0] is the first version of the function
    
    funcIterator funcIter(func());
    int_function *funcInstance = NULL;
    unsigned c = 0;
    while ( (funcInstance = *funcIter) != NULL) {
        inst_printf("Creating instance for function version %d...",
                    c++);
        // Can anything in here fail? If so, need error handling

        // Yay inefficient
        bool found = false;
        for (unsigned i = 0; i < instances.size(); i++) {
            if (instances[i]->func() == funcInstance) {
                found = true;
                break;
            }
        }

        if (!found) {
            Address newAddr = funcInstance->equivAddr(func(), addr());
            instPointInstance *ipInst = new instPointInstance(newAddr,
                                                              funcInstance,
                                                              this);
            instances.push_back(ipInst);
            // Register with the process before asking for a multitramp
            proc()->registerInstPointAddr(newAddr, this);
        }


        // Next!
        funcIter++;
    }
    funcVersion = funcVer;
    
    return true;
}

// Blah blah blah...
miniTramp *instPoint::instrument(AstNode *ast,
                                 callWhen when,
                                 callOrder order,
                                 bool trampRecursive,
                                 bool noCost,
                                 bool allowTrap) {
    
    inst_printf("instrument(%p, %d, %d, %d, %d, %d)\n",
                ast, when, order, trampRecursive, noCost,
                allowTrap);

    miniTramp *mini = addInst(ast, when, order, trampRecursive, noCost);
    if (!mini) {
        cerr << "instPoint::instrument: failed addInst, ret NULL" << endl;
        return NULL;
    }
    if (!generateInst(allowTrap)) {
        cerr << "instPoint::instrument: failed generateInst, ret NULL" << endl;
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

bool instPoint::generateInst(bool allowTrap) {
    updateInstances();
    bool success = true;
    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->generateInst(allowTrap))
            success = false;
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
            if ((pc >= mt->instAddr()) ||
                (pc < (mt->instAddr() + mt->instSize())))
                return false;
        }
    }
    return true;
}

bool instPoint::linkInst() {
    bool success = true;

    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->linkInst())
            success = false;
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

instPoint::instPoint(process *proc,
                     instruction insn,
                     Address addr,
                     int_function *func) : 
    instPointBase(insn, otherPoint),
    funcVersion(0),
    callee_(NULL),
    isDynamicCall_(false),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    proc_(proc),
    img_p_(NULL),
    func_(func),
    addr_(addr)
{
}

// Process specialization of a parse-time instPoint
instPoint::instPoint(process *proc,
                     image_instPoint *img_p,
                     Address addr,
                     int_function *func) :
    instPointBase(img_p->insn(),
                  img_p->getPointType(),
                  img_p->id()),
    funcVersion(0),
    callee_(NULL),
    isDynamicCall_(img_p->isDynamicCall()),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    proc_(proc),
    img_p_(img_p),
    func_(func),
    addr_(addr)
{

}

// Copying over from fork
instPoint::instPoint(instPoint *parP,
                     int_function *child) :
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
    func_(child),
    addr_(parP->addr()) {
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
    
    if (absAddr > (func->getAddress() + func->getSize())) {
        assert(0); // Something broke
    }

    int_basicBlock *block = func->findBlockByAddr(absAddr);
    assert(block);


    newIP = new instPoint(func->proc(),
                          img_p,
                          absAddr,
                          func);
    
    inst_printf("Entering common IP creation...\n");
    if (!commonIPCreation(newIP,
                          block)) {
        delete newIP;
        return NULL;
    }

    inst_printf("Returning new instPoint\n");
    return newIP;
}

instPoint *instPoint::createForkedPoint(instPoint *parP, int_function *child) {
    // Make a copy of the parent instPoint. We don't have multiTramps yet,
    // which is okay; just get the ID right.
    instPoint *newIP = new instPoint(parP, child);
    process *proc = child->proc();
    
    // Add to the process
    for (unsigned i = 0; i < parP->instances.size(); i++) {
        instPointInstance *pI = parP->instances[i];
        instPointInstance *nI = new instPointInstance(pI->addr_,
                                                      child, // FIXME
                                                      newIP);
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


trampEnd::trampEnd(multiTramp *multi, Address target) :
    multi_(multi), target_(target) 
{}

Address relocatedInstruction::originalTarget() const {
  return insn.getTarget(origAddr);
}

void relocatedInstruction::overrideTarget(Address newTarget) {
    targetOverride_ = newTarget;
}

// allowTrap shouldn't really go here; problem is, 
// multiple instPoints might use the same multiTramp,
// and I'm not sure how to handle it.
bool instPointInstance::generateInst(bool allowTrap) {

    // Do all the hard work; this will create a complete
    // instrumentation structure and copy it into the address
    // space, but not link it up. We do that later. It may also
    // change the multiTramp we have a pointer to, though that's
    // handled through the multi() wrapper.

    if (!multiID_) {
        multiID_ = multiTramp::findOrCreateMultiTramp(addr(), proc(), allowTrap);
    }
    
    if (!multi()->generateMultiTramp()) {
        return false;
    }
    // Can and will set our multiTramp ID if there isn't one already;
    // if there is, will reuse that slot in the multiTramp dictionary.
    // This allows us to regenerate multiTramps without having to 
    // worry about changing pointers.
    
    // We now "install", that is copy the generated code into the 
    // addr space. This doesn't link.
    
    if (!multi()->installMultiTramp()) {
        return false;
    }

    return true;
}

bool instPointInstance::linkInst() {
    // Funny thing is, we might very well try to link a multiTramp
    // multiple times...
    // Ah, well.
    assert(multi());

    return multi()->linkMultiTramp();
}

multiTramp *instPointInstance::multi() const {
    if (multiID_ == 0)
        return NULL;
    return multiTramp::getMulti(multiID_, proc());
}

void instPointInstance::updateMulti(int id) {
    if (multiID_)
        assert(id == multiID_);
    else
        multiID_ = id;
}

process *instPointInstance::proc() const {
    return point->proc();
}

bool instPoint::instrSideEffect(Frame &frame)
{
  assert(instances.size());
  updateInstances();

  for (unsigned i = 0; i < instances.size(); i++) {
      instPointInstance *target = instances[i];
      
      int_function *instFunc = target->func();
      if (!instFunc) return false;
      
      codeRange *range = frame.getRange();
      if (range->is_function() != instFunc) {
          // Not in this function at all, so skip
          continue;
      }
      
      // Question: generalize into "if the PC is in instrumentation,
      // move to the equivalent address?" 
      // Sure....
      
      // See previous call-specific version below; however, this
      // _should_ work.
      Address newPC = target->multi()->uninstToInstAddr(frame.getPC());
      if (newPC) 
          frame.setPC(newPC);

      // That's if we want to move into instrumentation. Mental note:
      // check to see if we're handling return points correctly; we should
      // be. If calls ever end basic blocks, we'll have to fix this.

  }
  return true;
}

#if !defined(arch_sparc)
// Sparc has its own version... how annoying. It's in inst-sparc.C
bool trampEnd::generateCode(codeGen &gen,
                            Address baseInMutatee) {
    if (alreadyGenerated(gen, baseInMutatee))
        return true;
    
    generateSetup(gen, baseInMutatee);

    if (target_) {
        instruction::generateBranch(gen,
                                    gen.currAddr(baseInMutatee),
                                    target_);
    }
    // And a sigill insn
    instruction::generateIllegal(gen);
    
    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;
    
    return true;
}
#endif


//////////////////////////
// Move to arch.C
//////////////////////////

#if defined(arch_x86) || defined(arch_x86_64)
#define CODE_GEN_OFFSET_SIZE 1
#elif defined(arch_ia64)
#define CODE_GEN_OFFSET_SIZE (sizeof(codeBuf_t))
#else
#define CODE_GEN_OFFSET_SIZE (instruction::size())
#endif


codeGen::codeGen() :
    buffer_(NULL),
    offset_(0),
    size_(0),
    allocated_(false) {}

// Note: this is in "units", typically the 
// instruction size
codeGen::codeGen(unsigned size) :
    offset_(0),
    size_(size),
    allocated_(true) {
    buffer_ = (codeBuf_t *)malloc(size);
    if (!buffer_)
        fprintf(stderr, "Malloc failed for buffer of size %d\n", size_);
    assert(buffer_);
}

// And this is actual size in bytes.
codeGen::codeGen(codeBuf_t *buffer, int size) :
    offset_(0),
    size_(size),
    allocated_(true) {
    buffer_ = buffer;
}

codeGen::~codeGen() {
    if (allocated_ && buffer_) free(buffer_);
}

// Deep copy
codeGen::codeGen(const codeGen &g) :
    offset_(g.offset_),
    size_(g.size_),
    allocated_(g.allocated_) {
    if (size_ != 0) {
        assert(allocated_); 
        buffer_ = (codeBuf_t *) malloc(size_);
        memcpy(buffer_, g.buffer_, size_);
    }
    else
        buffer_ = NULL;
}

bool codeGen::operator==(void *p) {
    return (p == (void *)buffer_);
}

bool codeGen::operator!=(void *p) {
    return (p != (void *)buffer_);
}

bool codeGen::operator()() {
    if (buffer_ && used())
        return true;
    else
        return false;
}

codeGen &codeGen::operator=(const codeGen &g) {
    // Same as copy constructor, really
    invalidate();
    offset_ = g.offset_;
    size_ = g.size_;
    allocated_ = g.allocated_;

    if (size_ != 0) {
        assert(allocated_); 
        buffer_ = (codeBuf_t *) malloc(size_);
        memcpy(buffer_, g.buffer_, size_);
    }
    else
        buffer_ = NULL;
    return *this;
}

void codeGen::allocate(unsigned size) {
    // No implicit reallocation
    assert(buffer_ == NULL);
    size_ = size;
    offset_ = 0;
    buffer_ = (codeBuf_t *)malloc(size);
    allocated_ = true;
    assert(buffer_);
}

// Very similar to destructor
void codeGen::invalidate() {
    if (allocated_ && buffer_)
        free(buffer_);
    buffer_ = NULL;
    size_ = 0;
    offset_ = 0;
    allocated_ = false;
}

void codeGen::finalize() {
    assert(buffer_);
    assert(size_);
    if (size_ == offset_) return;
    if (offset_ == 0) {
        invalidate();
        return;
    }
    codeBuf_t *newbuf = (codeBuf_t *)malloc(used());
    memcpy((void *)newbuf, (void *)buffer_, used());
    size_ = offset_;
    free(buffer_);
    buffer_ = newbuf;
}

void codeGen::copy(const void *b, const unsigned size) {
    assert(buffer_);
    memcpy(cur_ptr(), b, size);
    // "Upgrade" to next index side
    int disp = size;
    if (disp % CODE_GEN_OFFSET_SIZE) {
        disp += (CODE_GEN_OFFSET_SIZE - (disp % CODE_GEN_OFFSET_SIZE));
    }
    moveIndex(disp);
}

void codeGen::copy(codeGen &gen) {
    memcpy((void *)cur_ptr(), (void *)gen.start_ptr(), gen.used());
    offset_ += gen.offset_;
    assert(used() <= size_);
}

// codeBufIndex_t stores in platform-specific units.
unsigned codeGen::used() const {
    return offset_ * CODE_GEN_OFFSET_SIZE;
}

void *codeGen::start_ptr() const {
    return (void *)buffer_;
}

void *codeGen::cur_ptr() const {
    assert(buffer_);
    if (sizeof(codeBuf_t) != CODE_GEN_OFFSET_SIZE)
        fprintf(stderr, "ERROR: sizeof codeBuf %d, OFFSET %d\n",
                sizeof(codeBuf_t), CODE_GEN_OFFSET_SIZE);
    assert(sizeof(codeBuf_t) == CODE_GEN_OFFSET_SIZE);
    codeBuf_t *ret = buffer_;
    ret += offset_;
    return (void *)ret;
}

void codeGen::update(codeBuf_t *ptr) {
    assert(buffer_);
    assert(sizeof(unsigned char) == 1);
    unsigned diff = ((unsigned char *)ptr) - ((unsigned char *)buffer_);
    
    // Align...
    if (diff % CODE_GEN_OFFSET_SIZE) {
        diff += CODE_GEN_OFFSET_SIZE - (diff % CODE_GEN_OFFSET_SIZE);
        assert ((diff % CODE_GEN_OFFSET_SIZE) == 0);
    }
    // and integer division rules
    offset_ = diff / CODE_GEN_OFFSET_SIZE;
    if (used() > size_) {
        fprintf(stderr, "ERROR: code generation, used %d bytes, total size %d (%p)!\n",
                used(), size_, this);
    }
    assert(used() <= size_);
}

void codeGen::setIndex(codeBufIndex_t index) {
    offset_ = index;
    if (used() > size_) {
        fprintf(stderr, "ERROR: overran codeGen (%d > %d)\n",
                used(), size_);
    }
    assert(used() <= size_);
}

codeBufIndex_t codeGen::getIndex() const {
    return offset_;
}

void codeGen::moveIndex(int disp) {
    int cur = getIndex() * CODE_GEN_OFFSET_SIZE;
    cur += disp;
    if (cur % CODE_GEN_OFFSET_SIZE) {
        fprintf(stderr, "Error in codeGen: current index %d/%d, moving by %d, mod %d\n",
                getIndex(), cur, disp, cur % CODE_GEN_OFFSET_SIZE);
    }
    assert((cur % CODE_GEN_OFFSET_SIZE) == 0);
    setIndex(cur / CODE_GEN_OFFSET_SIZE);
}

int codeGen::getDisplacement(codeBufIndex_t from, codeBufIndex_t to) {
    return ((to - from) * CODE_GEN_OFFSET_SIZE);
}

Address codeGen::currAddr(const Address base) const {
    return (offset_ * CODE_GEN_OFFSET_SIZE) + base;
}

void codeGen::fill(int fillSize, int fillType) {
    if (fillType == cgNOP) {
        instruction::generateNOOP(*this, fillSize);
    }
    else {
        assert(0 && "unimplemented");
    }
}

void codeGen::fillRemaining(int fillType) {
    if (fillType == cgNOP) {
        instruction::generateNOOP(*this,
                                  size_ - used());
    }
    else {
        assert(0 && "unimplemented");
    }
}

instMapping::instMapping(const instMapping *parIM,
                         process *child) :
    func(parIM->func),
    inst(parIM->inst),
    where(parIM->where),
    when(parIM->when),
    order(parIM->order),
    useTrampGuard(parIM->useTrampGuard),
    mt_only(parIM->mt_only),
    allow_trap(parIM->allow_trap)
{
    for (unsigned i = 0; i < parIM->args.size(); i++) {
        args.push_back(assignAst(parIM->args[i]));
    }
    for (unsigned j = 0; j < parIM->miniTramps.size(); j++) {
        miniTramp *cMT = NULL;
        getInheritedMiniTramp(parIM->miniTramps[j],
                              cMT,
                              child);
        assert(cMT);
        miniTramps.push_back(cMT);
    }
}

Address relocatedInstruction::relocAddr() const {
    return addrInMutatee_;
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
