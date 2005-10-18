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

// $Id: miniTramp.C,v 1.13 2005/10/18 17:35:36 bernat Exp $
// Code to install and remove instrumentation from a running process.

#include "miniTramp.h"
#include "baseTramp.h"
#include "instP.h"
#include "instPoint.h"
#include "process.h"

// for AIX
#include "function.h"

int miniTramp::_id = 1;

#if defined(os_aix)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

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

  if (deleteInProgress)
    return false;

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
  for (unsigned i = 0; i < instances.size(); i++)
      instances[i]->removeCode(NULL);
  // When all instances are successfully deleted, the miniTramp
  // will be deleted as well.
  
  if(BPatch::bpatch->baseTrampDeletion())
      {
          baseT->deleteIfEmpty();
      }

  return true;
}

void miniTramp::deleteMTI(miniTrampInstance *mti) {
    for (unsigned i = 0; i < instances.size(); i++)
        if (instances[i] == mti) {
            instances[i] = instances.back();
            instances.pop_back();               
        }
    if (deleteInProgress &&
        (instances.size() == 0))
        delete this;
}

// Defined in multiTramp.C, dinky "get the debugger to stop here" function.
extern void debugBreakpoint();

bool miniTramp::generateMT() {
    //inst_printf("AST pointer is %p\n", ast_);

    // This can be called multiple times
    if (miniTrampCode_ != NULL) return true;

    miniTrampCode_.allocate(MAX_MINITRAMP_SIZE);
    
    registerSpace * regS = NULL;
#if defined( arch_ia64 )
	/* FIXME: We should be able to cache this in the baseTramp or its location. */
	extern unsigned int deadRegisterListLength;
	extern Register deadRegisterList[];
	
	regS = new registerSpace( deadRegisterListLength, deadRegisterList, 0, NULL );

	regS->resetSpace();
	regS->resetClobbers();
	
	defineBaseTrampRegisterSpaceFor( instP, regS, deadRegisterList );
#else
	Register blank[] = {1};
	regS = new registerSpace(0, blank, 0, blank, false);
#endif /* defined( arch_ia64 ) */
    
    /* VG(11/06/01): Added location, needed by effective address AST node */
    returnOffset = ast_->generateTramp(proc(), instP,
                                       miniTrampCode_, &cost, 
                                       false, // noCost -- we can always ignore it later
                                       regS);

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

miniTrampInstance *miniTramp::getMTInstanceByBTI(baseTrampInstance *bti) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->baseTI == bti)
            return instances[i];
    }
    // Didn't find it... add it if the miniTramp->baseTramp mapping
    // is correct
    assert(baseT == bti->baseT);
    
    miniTrampInstance *mtInst = new miniTrampInstance(this, bti);

    instances.push_back(mtInst);
    return mtInst;
}

miniTrampInstance::~miniTrampInstance() {
    mini->deleteMTI(this);

    proc()->deleteCodeRange(get_address_cr());
    proc()->inferiorFree(trampBase);
}


unsigned miniTrampInstance::maxSizeRequired() {
    // If in-line...

    if (mini->baseT->firstMini == mini) {
        //inst_printf("Size request for first mini\n");
        return instruction::maxJumpSize();
    }
    return 0;
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
                                     UNW_INFO_TYPE ** unwindInformation ) {
    /*
      inst_printf("miniTrampInstance::generateCode(%p, 0x%x, %d)\n",
                gen.start_ptr(), baseInMutatee, gen.used());
    */
    assert(mini);
    // if (in-line...)
    // Are we first?
    if (mini->baseT->firstMini == mini) {
        if (!generated_) { 
            gen.fill(instruction::maxJumpSize(),
                     codeGen::cgNOP);
            mini->baseT->instSize = instruction::maxJumpSize();
        }
        else {
            gen.moveIndex(instruction::maxJumpSize());
        }
#if defined( cap_unwind )
	/* maxJumpSize() returns bytes */
#if defined( arch_ia64 )
    (*unwindInformation)->insn_count += (instruction::maxJumpSize() / 16) * 3;
#else
#error How do I know how many instructions are in the jump region?
#endif /* defined( arch_ia64 ) */
#endif /* defined( cap_unwind ) */       
    }
    
    // And make-y the code
    //inst_printf("Generating MT code\n");
    if (!mini->generateMT())
        return false;
    generated_ = true;
    //inst_printf("Done with MT code generation\n");
    return true;
}

bool miniTrampInstance::installCode() {
    // If in-line...

    // Write, I say _write_ into the addr space
    if (installed_) {
        assert(trampBase);
        return true;
    }
    
    // See if there's anyone nearby.
    Address nearAddr = 0;

    miniTramp *nearby = (mini->next);
    if (nearby) {
        miniTrampInstance *nearMTI = nearby->getMTInstanceByBTI(baseTI);
        assert(nearMTI);
        nearAddr = nearMTI->trampBase;
    }
    if (!nearAddr) {
        // Either no next, or hasn't been allocated... try again with prev
        nearby = mini->prev;
        if (nearby) {
            miniTrampInstance *nearMTI = nearby->getMTInstanceByBTI(baseTI);
            assert(nearMTI);
            nearAddr = nearMTI->trampBase;
        }
    }
    // NearAddr might be 0 if there's nobody nearby or if the neighbor hasn't been
    // allocated yet.

#if defined(arch_alpha)
    // Short branch range...
    if (!nearAddr)
        nearAddr = baseTI->trampPreAddr();
#endif


    inferiorHeapType htype;
#if defined(os_aix)
    // We use the data heap on AIX because it is unlimited, unlike
    // the textHeap. The text heap is allocated from spare parts of 
    // pages, and as such can run out. Since minitramps can be arbitrarily
    // far from the base tramp (link register branching), but only a 
    // single jump from each other, we cluster them in the dataHeap.
    htype = dataHeap;
#else
    htype = (mini->proc()->splitHeaps) ? ((inferiorHeapType) (textHeap | uncopiedHeap)) : (anyHeap);
#endif
    
#if defined(os_aix)
    if (proc()->requestTextMiniTramp ||
        ((nearAddr < 0x20000000)  && (nearAddr > 0x0))) {
        htype = anyHeap;
        nearAddr = 0x10000000;
    }
    else {
#if defined(bug_aix_proc_broken_fork)
        if (mini->instP->func()->prettyName() == pdstring("__fork")) {
            nearAddr = 0;
            htype = dataHeap;
        }
#endif
    }
#endif
    bool err = false;
    trampBase = mini->proc()->inferiorMalloc(mini->size_, htype, nearAddr, &err);

    if (err) {
        cerr << "inst.C: failed allocation" << endl;
        return false;
    }
    if (!proc()->writeTextSpace((void *)trampBase,
                                mini->miniTrampCode_.used(),
                                (void *)mini->miniTrampCode_.start_ptr()))
        return false;

#if defined( cap_unwind )
	/* TODO: Minitramps don't change the unwind state of the program (except
	   via calls), so they all ALIAS to the corresponding jump in the
	   base tramp.  We could notionally cache the regions, but since
	   we have to change the unw_dyn_info_t anyway, this is just simpler. */
	
	unw_dyn_info_t * miniTrampDynamicInfo = (unw_dyn_info_t *)calloc( 1, sizeof( unw_dyn_info_t ) );
	assert( miniTrampDynamicInfo != NULL );

    miniTrampDynamicInfo->start_ip = trampBase;
    miniTrampDynamicInfo->end_ip = trampBase + mini->miniTrampCode_.used();
    miniTrampDynamicInfo->gp = mini->proc()->getTOCoffsetInfo( baseTI->multiT->instAddr()  );
    miniTrampDynamicInfo->format = UNW_INFO_FORMAT_DYNAMIC;

    miniTrampDynamicInfo->u.pi.name_ptr = (unw_word_t) "dynamic instrumentation: mini tramp";
	
	miniTrampDynamicInfo->next = NULL;
	miniTrampDynamicInfo->prev = NULL;
	
	unw_dyn_region_info_t * aliasRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 2 ) );
	assert( aliasRegion != NULL );

	aliasRegion->insn_count = 0;
	aliasRegion->op_count = 2;
	
	Address jumpToMiniTrampAddress = baseTI->trampAddr_ + baseTI->baseT->instStartOffset;
	dyn_unw_printf( "%s[%d]: ALIASING minitramp (0x%lx - 0x%lx) to 0x%lx\n", __FILE__, __LINE__, miniTrampDynamicInfo->start_ip, miniTrampDynamicInfo->end_ip, jumpToMiniTrampAddress );
	_U_dyn_op_alias( & aliasRegion->op[0], _U_QP_TRUE, -1, jumpToMiniTrampAddress );
	_U_dyn_op_stop( & aliasRegion->op[1] );
	
	unw_dyn_region_info_t * trampRegion = (unw_dyn_region_info_t *)malloc( _U_dyn_region_info_size( 1 ) );
	assert( trampRegion != NULL );
	
	// mini->miniTrampCode_.used() returns bytes.
#if defined( arch_ia64 )
	trampRegion->insn_count = (mini->miniTrampCode_.used() / 16) * 3;
#else
#error How do I know how many instructions are in the jump region?
#endif /* defined( arch_ia64 ) */
	trampRegion->op_count = 1;

	aliasRegion->next = trampRegion;
	trampRegion->next = NULL;
	
	_U_dyn_op_stop( & trampRegion->op[0] );

    miniTrampDynamicInfo->u.pi.regions = aliasRegion;
    bool status = mini->proc()->insertAndRegisterDynamicUnwindInformation( miniTrampDynamicInfo );
	if( ! status ) { return false; }

	free( trampRegion );
    free( aliasRegion );
    free( miniTrampDynamicInfo );
	
#endif

    // TODO in-line
    proc()->addCodeRange(this);
    
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
    // TODO: in-line

    baseTrampInstance *delBTI = dynamic_cast<baseTrampInstance *>(subObject);

    assert((subObject == NULL) || delBTI);

    // removeCode can be called in one of two directions: from a child
    // (with themselves as the object) or a parent. We differ in 
    // behavior depending on the type.

    if (subObject == NULL) {
        baseTI->removeCode(this);
        proc()->deleteGeneratedCode(this);
        // Make sure our previous guy jumps to the next guy
        if (mini->prev) {
            miniTrampInstance *prevI = mini->prev->getMTInstanceByBTI(baseTI);
            assert(prevI);
            prevI->linkCode();
        }
    }
    else {
        assert(delBTI);
        // Base tramp went away; but me may have been reattached to
        // a different instance. If so, we're cool. If not, clean 
        // up and go away.
        if (delBTI == baseTI) {
            proc()->deleteGeneratedCode(this);
            // Don't call baseTI->removeCode...
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

process *miniTrampInstance::proc() {
    return mini->proc();
}

bool miniTrampInstance::linkCode() {
    assert(baseTI);
    assert(baseTI->trampPreAddr());

    // if in-line...
    if (mini->next) {
        assert(baseTI);
        miniTrampInstance *nextI = mini->next->getMTInstanceByBTI(baseTI);
        assert(nextI);
        /*
        inst_printf("Writing branch from 0x%x (0x%x,0x%x) to 0x%x, miniT -> miniT\n",
                    trampBase + mini->returnOffset,
                    trampBase,
                    mini->returnOffset,
                    nextI->trampBase);
        */
        generateAndWriteBranch(mini->proc(), 
                               trampBase + mini->returnOffset,
                               nextI->trampBase,
                               instruction::maxJumpSize());
    }
    else {
        // Last one; go to the base tramp
        /*
        inst_printf("Writing branch from 0x%x to 0x%x, miniT -> baseT\n",
                    trampBase + mini->returnOffset,
                    baseTI->miniTrampReturnAddr());
        */
#if defined(os_aix)
        // TODO: miniTramp methods to wrap this.
        resetBR(mini->proc(),
                trampBase + mini->returnOffset);
#else
        generateAndWriteBranch(mini->proc(),
                               (trampBase + mini->returnOffset),
                               baseTI->miniTrampReturnAddr(),
                               instruction::maxJumpSize());
#endif
    }

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
    // We out-of-line, and so this just shifts allegiance.
    baseTrampInstance *newBTI = dynamic_cast<baseTrampInstance *>(newParent);
    assert(newBTI);

    baseTI->deleteMTI(this);
    baseTI = newBTI;

    // Not linked yet...
    linked_ = false;
    return this;
}
 
unsigned miniTrampInstance::get_size_cr() const { 
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
   
                      

miniTramp::miniTramp(callWhen when_,
                     AstNode *ast,
                     baseTramp *base,
                     instPoint *inst,
                     bool noCost) :
    miniTrampCode_(),
    ID(_id++), 
    returnOffset(0), 
    size_(0),
    baseT(base),
    instP(inst), 
    proc_(inst->proc()),
    when(when_),
    cost(0), 
    noCost_(noCost),
    prev(NULL), next(NULL),
    callback(NULL), callbackData(NULL),
    deleteInProgress(false) {
    ast_ = assignAst(ast);
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
    prev(NULL),
    next(NULL),
    callback(NULL),
    callbackData(NULL),
    deleteInProgress(parMini->deleteInProgress)
{
    assert(parMini->ast_);
    ast_ = assignAst(parMini->ast_);

    // Fix up instP
    instP = proc->findInstPByAddr(parMini->instP->addr());
    assert(instP);
    // Uhh... what about callbacks?
    // Can either set them to null or have them returning 
    // the same void * as their parent...

}    

miniTramp::~miniTramp() {
    if (callback)
        (*callback)(callbackData, this);  
    removeAst(ast_);
}

// Given a miniTramp parentMT, find the equivalent in the child
// process (matching by the ID member). Fill in childMT.
  
bool getInheritedMiniTramp(const miniTramp *parentMT, 
                             miniTramp * &childMT, 
                             process *childProc) {
    pdvector<instPoint *> instPs;
    if (parentMT->baseT->preInstP)
        instPs.push_back(parentMT->baseT->preInstP);
    if (parentMT->baseT->postInstP)
        instPs.push_back(parentMT->baseT->postInstP);
    
    for (unsigned i = 0; i < instPs.size(); i++) {
        
        instPoint *childP = childProc->findInstPByAddr(instPs[i]->addr());
        baseTramp *childB = childP->getBaseTramp(parentMT->when);
        miniTramp *mt = childB->firstMini;
        
        while (mt) {
            if (mt->ID == parentMT->ID) {
                childMT = mt;
                return true;
            }
            mt = mt->next;
        }
    }
    return false;
}

Address miniTrampInstance::uninstrumentedAddr() const {
    // We're "in" the baseTramp, so it knows what's going on
    return baseTI->uninstrumentedAddr();
}

// Returns true if the "current" is after the "new". Currently does not
// handle in-lining, where you might need catchup even if you're 
// "before".
bool miniTramp::catchupRequired(miniTramp *curMT, miniTramp *newMT) {
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
