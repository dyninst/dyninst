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

// $Id: miniTramp.C,v 1.41 2008/02/20 22:34:20 legendre Exp $
// Code to install and remove instrumentation from a running process.

#include "miniTramp.h"
#include "baseTramp.h"
#include "instP.h"
#include "instPoint.h"
#include "process.h"
#include "ast.h"
#include "addressSpace.h"
#include "dyninstAPI/h/BPatch.h"

// for AIX
#include "function.h"

int miniTramp::_id = 1;

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 * New logic: this routine gaps the minitramp out of the execution
 * sequence, but leaves deletion for later. There is a routine in 
 * the process object which maintains a list of elements to be deleted,
 * and the associated data to ensure that deletion is safe. I've
 * added a callback function to the instInstance class which is 
 * called when deletion actually takes place. This allows recordkeeping
 * for any data which may rely on the minitramp (i.e. Paradyn variables)
 *
 */


// returns true if deleted, false if not deleted (because non-existant
// or already deleted
bool miniTramp::uninstrument() {

  // First check: have we started to delete this guy already?
  // This happens when we try to delete an instInstance and GC it
  // We then pause the process, but if the process is exited Paradyn
  // tries to disable all instrumentation... thus causing this one
  // to be deleted again. Sigh. 
  
  // Better fix: figure out why we're double-deleting instrCodeNodes.

    if (proc()->proc() &&
        !proc()->proc()->isAttached()) {
        return true;
    }

    if (deleteInProgress) {
        return false;
    }

    
    stats_instru.startTimer(INST_REMOVE_TIMER);
    stats_instru.incrementCounter(INST_REMOVE_COUNTER);

    deleteInProgress = true;
    
    // We do next so that it's definitely fixed before we call
    // correctBTJumps below.
    if (next) {
        next->prev = prev;
    }
    else {
        // Like above, except last
        baseT->lastMini = prev;
    }
    
    if (prev) {
        prev->next = next; 
    }
    else {
        // We're first, so clean up the base tramp
        baseT->firstMini = next;
        // Correcting of jumps will be handled by removeCode calls
    }
    
    
    // DON'T delete the miniTramp. When it is deleted, the callback
    // is made... which should only happen when the memory is freed.
    // Place it on the list to be deleted.
    topDownDelete_ = true;
    for (int i = instances.size() ; i > 0 ; i--)
        instances[i-1]->removeCode(NULL);

    // When all instances are successfully deleted, the miniTramp
    // will be deleted as well.
    
    if(BPatch::bpatch->baseTrampDeletion())
        {
            baseT->deleteIfEmpty();
      }
    
    stats_instru.stopTimer(INST_REMOVE_TIMER);
    return true;
}

void miniTramp::deleteMTI(miniTrampInstance *mti) {
    for (unsigned i = 0; i < instances.size(); i++)
        if (instances[i] == mti) {
            instances[i] = instances.back();
            instances.pop_back();               
        }
    if (deleteInProgress && !topDownDelete_ && !instances.size())
        delete this;
}

// Defined in multiTramp.C, dinky "get the debugger to stop here" function.
extern void debugBreakpoint();

bool miniTramp::generateMT(registerSpace *rs) 
{
    //inst_printf("AST pointer is %p\n", ast_);

    // This can be called multiple times
    if (miniTrampCode_ != NULL) return true;

    miniTrampCode_.allocate(MAX_MINITRAMP_SIZE);
    miniTrampCode_.setAddrSpace(proc());
    miniTrampCode_.setRegisterSpace(rs);
    miniTrampCode_.setPoint(instP());

    /* VG(11/06/01): Added location, needed by effective address AST node */
    returnOffset = ast_->generateTramp(miniTrampCode_, 
                                       cost, false);

    size_ = miniTrampCode_.used();
    miniTrampCode_.finalize();

    debugBreakpoint();
    
    return true;
}

bool miniTramp::correctMTJumps() {
    for (unsigned i = 0; i < instances.size(); i++) {
        instances[i]->linkCode();
    }
    return true;
}

miniTrampInstance *miniTramp::getMTInstanceByBTI(baseTrampInstance *bti,
                                                 bool create_if_not_found) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->baseTI == bti)
            return instances[i];
    }

    if(create_if_not_found) {
       // Didn't find it... add it if the miniTramp->baseTramp mapping
       // is correct
       assert(baseT == bti->baseT);

       miniTrampInstance *mtInst = new miniTrampInstance(this, bti);

       instances.push_back(mtInst);
       return mtInst;
    }
    return NULL;
}

miniTrampInstance::~miniTrampInstance() {
    mini->deleteMTI(this);
}


unsigned miniTrampInstance::maxSizeRequired() {
    // Estimate...
    // Test1 has this enormous miniTramp that basically
    // screws it up for everyone else :)
    return mini->ast_->getTreeSize()*512;
}


// This must write the "top-level" code for the minitramp;
// that is, for inlined minitramps the entire body, and
// for out-of-line minitramps a set of code that will
// reach the (allocated and installed) minitramp.

// Right now we only do out-of-line, so the first miniTramp
// reserves space for a jump (filled with noops for now),
// and the rest just return.

/* Note that out-of-line minitramps imply that they only
   add their /inline/ regions (jumps) to the unwindInformation
   chain, and register their /out-of-line/ regions on their own. */
bool miniTrampInstance::generateCode(codeGen &gen,
                                     Address baseInMutatee,
                                     UNW_INFO_TYPE ** /* unwindInformation */ )
{
    inst_printf("miniTrampInstance(%p)::generateCode(%p, 0x%x, %d)\n",
                this, gen.start_ptr(), baseInMutatee, gen.used());
    assert(mini);
    
    if (!mini->generateMT(gen.rs()))
        return false;

    // Copy code into the buffer
    gen.copy(mini->miniTrampCode_);
    // TODO unwind information
    
    generated_ = true;
    hasChanged_ = false;
    //inst_printf("Done with MT code generation\n");
    return true;
}

bool miniTrampInstance::installCode() {
    installed_ = true;
    return true;
}

bool miniTrampInstance::safeToFree(codeRange *range) {
    // TODO: in-line...

    if (dynamic_cast<miniTrampInstance *>(range) == this) {
        return false;
    }
    else {
        // Out-of-line miniTramps are independent; if we're not
        // inside one, we can safely nuke.
        return true;
    }
}

void miniTrampInstance::removeCode(generatedCodeObject *subObject) {

    baseTrampInstance *delBTI = dynamic_cast<baseTrampInstance *>(subObject);
    if (baseTI && baseTI->multiT->getIsActive()) {
        mal_printf("removing miniTrampInstance at %lx for point %lx in ACTIVE multiTramp"
                "(instaddr=%lx [%lx %lx]) %s[%d]\n", baseTI->multiT->instAddr(),
                baseTI->baseT->instP()->addr(), 
                baseTI->multiT->instAddr(), baseTI->multiT->get_address(),
                baseTI->multiT->get_address() + baseTI->multiT->get_size(), 
                FILE__,__LINE__);
    }

    assert((subObject == NULL) || delBTI);

    // removeCode can be called in one of two directions: from a child
    // (with NULL as the argument) or a parent. We differ in 
    // behavior depending on the type.

    if (subObject == NULL) {
	// Make sure our previous guy jumps to the next guy
        if (mini->prev) {
            miniTrampInstance *prevI = mini->prev->getMTInstanceByBTI(baseTI, false);
            if(prevI != NULL)
                prevI->linkCode();
        }

        baseTI->removeCode(this);
        
	delete this;

    }
    else {
        assert(delBTI);
        // Base tramp went away; but me may have been reattached to
        // a different instance. If so, we're cool. If not, clean 
        // up and go away.
        if (delBTI == baseTI) {
            delete this;
        }
    }
}

void miniTrampInstance::freeCode() {
    // TODO: in-line

    // baseTrampInstance is deleted by the multiTramp...
    // baseTI->deleteMTI(this);

    mini->deleteMTI(this);
    proc()->inferiorFree(trampBase);
    delete this;
}

AddressSpace *miniTrampInstance::proc() const {
    return mini->proc();
}

bool miniTrampInstance::linkCode() {
    linked_ = true;
    return true;
}

void miniTrampInstance::invalidateCode() {
    assert(!linked_);

    if (trampBase)
        proc()->inferiorFree(trampBase);
    trampBase = 0;
    
    generated_ = false;
    installed_ = false;
}

unsigned miniTrampInstance::cost() {
    if (!mini->noCost_)
        return mini->cost;
    return 0;
}

generatedCodeObject *miniTrampInstance::replaceCode(generatedCodeObject *newParent) {
    baseTrampInstance *newBTI = dynamic_cast<baseTrampInstance *>(newParent);
    assert(newBTI);
    assert(this);

    baseTI->deleteMTI(this);

    if (!generated_) {
        baseTI = newBTI;
        return this;
    }
    // We replace ourselves...
    miniTrampInstance *newMTI = new miniTrampInstance(this, newBTI);
    assert(newMTI);
    return dynamic_cast<generatedCodeObject *>(newMTI);
}

bool miniTrampInstance::hasChanged() {
    return hasChanged_;
}
 
unsigned miniTrampInstance::get_size() const { 
     return mini->size_;
}

miniTrampInstance::miniTrampInstance(const miniTrampInstance *parMTI,
                                     baseTrampInstance *cBTI,
                                     miniTramp *cMT,
                                     process *child) :
    generatedCodeObject(parMTI, child),
    baseTI(cBTI),
    mini(cMT),
    trampBase(parMTI->trampBase),
    deleted(parMTI->deleted) 
{
    mini->instances.push_back(this);
}

miniTrampInstance::miniTrampInstance(const miniTrampInstance *origMTI,
                                     baseTrampInstance *parBTI) :
    generatedCodeObject(origMTI, origMTI->proc()),
    baseTI(parBTI),
    mini(origMTI->mini),
    trampBase(0),
    deleted(false)
{
    mini->instances.push_back(this);
}

   
miniTramp::miniTramp(callWhen when_,
                     AstNodePtr ast,
                     baseTramp *base,
                     bool noCost) :
    miniTrampCode_(),
    ID(_id++), 
    returnOffset(0), 
    size_(0),
    baseT(base),
    proc_(NULL),
    when(when_),
    cost(0), 
    noCost_(noCost),
	topDownDelete_(false),
    prev(NULL), next(NULL),
    callback(NULL), callbackData(NULL),
    deleteInProgress(false) {
    ast_ = dyn_detail::boost::dynamic_pointer_cast<AstMiniTrampNode>(AstNode::miniTrampNode(ast));

    assert(baseT);
    proc_ = baseT->proc();
}

miniTramp::miniTramp(const miniTramp *parMini,
                     baseTramp *childT,
                     process *proc) :
    miniTrampCode_(parMini->miniTrampCode_),
    ID(parMini->ID),
    returnOffset(parMini->returnOffset),
    size_(parMini->size_),
    baseT(childT),
    proc_(proc),
    when(parMini->when),
    cost(parMini->cost),
    noCost_(parMini->noCost_),
    topDownDelete_(parMini->topDownDelete_),
    prev(NULL),
    next(NULL),
    callback(NULL),
    callbackData(NULL),
    deleteInProgress(parMini->deleteInProgress)
{
    ast_ = parMini->ast_;

    // Uhh... what about callbacks?
    // Can either set them to null or have them returning 
    // the same void * as their parent...

}    

miniTramp::~miniTramp() {
    if (callback)
        (*callback)(callbackData, this);  
    // FIXME
    //removeAst(ast_);
}

// Given a miniTramp parentMT, find the equivalent in the child
// process (matching by the ID member). Fill in childMT.
  
miniTramp *miniTramp::getInheritedMiniTramp(process *childProc) {
    int_function *childF = childProc->findFuncByInternalFunc(func()->ifunc());
    assert(childF);
    instPoint *childP = childF->findInstPByAddr(instP()->addr());
    
    baseTramp *childB = childP->getBaseTramp(when);
    miniTramp *mt = childB->firstMini;
    
    while (mt) {
        if (mt->ID == ID) {
            return mt;
        }
        mt = mt->next;
    }
    
    return NULL;
}

Address miniTrampInstance::uninstrumentedAddr() const {
    // We're "in" the baseTramp, so it knows what's going on
    return baseTI->uninstrumentedAddr();
}

// Returns true if the "current" is after the "new". Currently does not
// handle in-lining, where you might need catchup even if you're 
// "before".
bool miniTramp::catchupRequired(miniTramp *curMT, miniTramp *newMT, bool) 
{
    // We start at current and iterate. If we see new, stop. If
    // we hit the end, stop.
    miniTramp *iterMT = newMT;

    while (iterMT) {
        if (iterMT == curMT) {
            // Oops, we just hit our current.
            return true;
        }
        iterMT = iterMT->next;
    }

    // We didn't hit it. Optimistically assume that newMT is after
    // curMT, and thus we don't need to worry. 
    
    // TODO INLINE
    return false;
}

void *miniTrampInstance::getPtrToInstruction(Address addr) const {
    if (!installed_) return NULL;
    if (addr < trampBase) return NULL;
    if (addr >= (trampBase + mini->returnOffset)) return NULL;

    addr -= trampBase;
    assert(mini->miniTrampCode_ != NULL);
    return mini->miniTrampCode_.get_ptr(addr);
}

instPoint *miniTramp::instP() const {
    return baseT->instP();
}

int_function *miniTramp::func() const {
    return baseT->instP()->func();
}

bool miniTramp::instrumentedViaTrap() const {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->baseTI->multiT->usesTrap())
            return false;
    }
    return true;
}
