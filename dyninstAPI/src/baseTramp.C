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

// $Id: baseTramp.C,v 1.52 2006/12/14 20:12:03 bernat Exp $

#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/dyn_thread.h"

#if defined(os_aix)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

// Normal constructor
baseTrampInstance::baseTrampInstance(baseTramp *tramp,
                                     multiTramp *multi) :    
    generatedCodeObject(),
    trampAddr_(0), // Unallocated
    trampSize_(0),
    trampPostOffset(0),
    baseT(tramp),
    multiT(multi),
    genVersion(0),
    alreadyDeleted_(false)
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
    alreadyDeleted_(parBTI->alreadyDeleted_)
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

    if (!alreadyDeleted_ && baseT) {
        baseT->unregisterInstance(this);
    }
}

void baseTramp::unregisterInstance(baseTrampInstance *inst) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i] == inst) {
            instances.erase(i, i);
            return;
        }
    }
}

#define PRE_TRAMP_SIZE 4096
#define POST_TRAMP_SIZE 4096

baseTramp::baseTramp(instPoint *iP) :
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
#if defined( arch_ia64 )
    trampGuardFlagAddr( 0 ),
    trampGuardFlagValue( 0 ),
    addressRegister( 0 ),
    valueRegister( 0 ),
#endif /* defined( arch_ia64 ) */
#if defined( cap_unwind )
    baseTrampRegion( NULL ),
#endif /* defined( cap_unwind ) */
    instP_(iP),
    rpcMgr_(NULL),
    isMerged(false),
    firstMini(NULL),
    lastMini(NULL),
    preTrampCode_(),
    postTrampCode_(),
	regSpace_(NULL),
    valid(false),
    optimized_out_guards(false),
    guardState_(unset_BTR),
    suppress_threads_(false),
    instVersion_()
{
}

#if defined( cap_unwind )
/* Duplicates the list headed by tail and appends it to head, returning the new end of the list.
   If head is NULL, returns NULL.  If tail is NULL, returns head. */
unw_dyn_region_info_t * appendRegionList( unw_dyn_region_info_t * head,  unw_dyn_region_info_t * tail ) {
	if( head == NULL ) { return NULL; }
	if( tail == NULL ) { return head; }
    
    unw_dyn_region_info_t * currentRegion = head;
    unw_dyn_region_info_t * nextRegion = tail;
    unw_dyn_region_info_t * newRegion = NULL;
    
	do {
		/* Allocate. */
		unsigned int nextRegionSize = _U_dyn_region_info_size( nextRegion->op_count );
		newRegion = (unw_dyn_region_info_t *)malloc( nextRegionSize );
		assert( newRegion != NULL );
		
		/* Copy. */
		memcpy( newRegion, nextRegion, nextRegionSize );
		
		/* Link. */
		currentRegion->next = newRegion;
		
		/* Iterate. */
		currentRegion = newRegion;
		nextRegion = currentRegion->next;
		} while( nextRegion != NULL );

    return currentRegion;        
	} /* end duplicateRegionList() */
	
/* Returns the head of the duplicate list. */
unw_dyn_region_info_t * duplicateRegionList( unw_dyn_region_info_t * head ) {
	if( head == NULL ) { return NULL; }	

	/* Duplicate the head and append the remainder to it. */
	unsigned int headSize = _U_dyn_region_info_size( head->op_count );
	unw_dyn_region_info_t * duplicateHead = (unw_dyn_region_info_t *)malloc( headSize );
	assert( duplicateHead != NULL );
	memcpy( duplicateHead, head, headSize );
	
	appendRegionList( duplicateHead, head->next ); 
	return duplicateHead;
	} /* end duplicateRegionList() */
#endif /* defined( cap_unwind ) */

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
#if defined( arch_ia64 )
    trampGuardFlagAddr( pt->trampGuardFlagAddr ),
    trampGuardFlagValue( pt->trampGuardFlagValue ),
	addressRegister( pt->addressRegister ),
	valueRegister( pt->valueRegister ),
#endif /* defined( arch_ia64 ) */    
    instP_(NULL),
    firstMini(NULL),
    lastMini(NULL),
    preTrampCode_(pt->preTrampCode_),
    postTrampCode_(pt->postTrampCode_),
	regSpace_(NULL),
    valid(pt->valid),
    optimized_out_guards(false),
    guardState_(pt->guardState_),
    suppress_threads_(pt->suppress_threads_),
    instVersion_(pt->instVersion_)
{
    if (pt->clobberedGPR)
        assert(0); // Don't know how to copy these
    if (pt->clobberedFPR)
        assert(0);
        
	if (pt->regSpace_)
		regSpace_ = new registerSpace(*(pt->regSpace_));
	else
		regSpace_ = NULL;

#if defined( cap_unwind )
    baseTrampRegion = duplicateRegionList( pt->baseTrampRegion );
#endif /* defined( cap_unwind ) */

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

    rpcMgr_ = NULL;
    if (pt->rpcMgr_) {
        rpcMgr_ = proc->getRpcMgr();
    }
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
                                     Address baseInMutatee,
                                     UNW_INFO_TYPE ** unwindRegion) 
{
    inst_printf("baseTrampInstance %p ::generateCode(%p, 0x%x, %d)\n",
                this, gen.start_ptr(), baseInMutatee, gen.used());
    
    updateMTInstances();
    
    if (isEmpty()) {
        hasChanged_ = false;
        generated_ = true;
        return true;
    }


    gen.setPoint(baseT->instP());
	gen.setRegisterSpace(registerSpace::actualRegSpace(baseT->instP()));
    
    if (BPatch::bpatch->isMergeTramp()) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: generating code in-the-line\n", __FILE__, __LINE__ );
        return generateCodeInlined(gen, baseInMutatee, unwindRegion);
    }
    else {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: generating code out-of-line\n", __FILE__, __LINE__ );
	    return generateCodeOutlined(gen, baseInMutatee, unwindRegion);
    }

}

bool baseTrampInstance::generateCodeOutlined(codeGen &gen,
                                             Address baseInMutatee,
                                             UNW_INFO_TYPE **unwindRegion) {    
    if (!generated_) {
        baseT->generateBT(gen);
        
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
    //inst_printf("offset now %d\n", gen.used());
    //unsigned preEnd = offset;
    
#if defined( cap_unwind )
    // This should become preTrampRegion, if the basetramp stops using only one region.
    /* appendRegionList() returns the last valid region it duplicated.
       This leaves unwindRegion pointing to the end of the list. */
    * unwindRegion = appendRegionList( * unwindRegion, baseT->baseTrampRegion );
#endif /* defined( cap_unwind ) */
    
    // miniTramps
    // If we go backwards, life gets much easier because each miniTramp
    // needs to know where to jump to (if we're doing out-of-line). 
    // Problem is, we can't if we're doing in-line. 
    
    // So we'll get to it later.
    
	// We're using a pre-generated code template and copying it in -
	// which means the register information is _all_ screwed up.
	// For now, we cheese out by assuming everything is saved (well, 
	// it is) and swapping register spaces. The baseTramp keeps one,
	// since it needs to know where registers are saved.
	registerSpace *curRegSpace = gen.rs();
	assert(baseT->regSpace_);
	baseT->regSpace_->cleanSpace();
	gen.setRegisterSpace(baseT->regSpace_);


    for (unsigned miter = 0; miter < mtis.size(); miter++) {
        mtis[miter]->generateCode(gen, baseInMutatee, unwindRegion);
        // Will increment offset if it writes to baseInMutator;
        // if in-lined each mini will increment offset.
        inst_printf("mti %d, offset %d\n", miter, gen.used());
    }
    gen.setRegisterSpace(curRegSpace);

    
    codeBufIndex_t postIndex = gen.getIndex();
    unsigned postStart = gen.used();
    
      
    if (!generated_) {
        gen.copy(baseT->postTrampCode_);
    }
    else {
        gen.moveIndex(baseT->postSize);
    }

    //inst_printf("post, offset %d\n", gen.used());
    
    if (!generated_) {
        trampPostOffset = postStart - preStart;
    }
    else {
        assert(trampPostOffset == (postStart - preStart));
    }

#if defined( cap_unwind )
	// Since the basetramp (currently) uses only one region, don't add it into the list again.
	/* appendRegionList() returns the last valid region it duplicated.
	   This leaves unwindRegion pointing to the end of the list. */
	// * unwindRegion = appendRegionList( * unwindRegion, baseT->postUnwindRegion );
#endif /* defined( cap_unwind ) */

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
    hasChanged_ = false;

    return true;
}

#include "BPatch.h"
#include "BPatch_collections.h"
bool baseTrampInstance::generateCodeInlined(codeGen &gen,
                                            Address baseInMutatee,
                                            UNW_INFO_TYPE **unwindRegion) {

    if (!hasChanged() && generated_) {
        assert(gen.currAddr(baseInMutatee) == trampAddr_);
        gen.moveIndex(trampSize_);
        return true;
    }

    // Experiment: use AST-based code generation....

    // We're generating something like so:
    //
    // <Save state>
    // <If>
    //    <compare>
    //      <load>
    //        <add>
    //          <tramp guard addr>
    //          <multiply>
    //            <thread index>
    //            <sizeof (int)>
    //      <0>
    //    <sequence>
    //      <store>
    //        <... tramp guard addr>
    //        <1>
    //      <mini tramp sequence>
    //      <store>
    //        <... tramp guard addr>
    //        <0>
    // <Cost section>
    // <Load state>


    // Break it down...
    // <Save state>
    //   -- TODO: an AST for saves that knows how many registers
    //      we're using...

    // Now we start building up the ASTs to generate. Let's get the
    // pieces.

    // Specialize for the instPoint...
	
    gen.setRegisterSpace(registerSpace::actualRegSpace(baseT->instP()));

    pdvector<AstNode *> miniTramps;
    for (unsigned miter = 0; miter < mtis.size(); miter++) {
        miniTramps.push_back(mtis[miter]->mini->ast_->getAST());
        // And nuke the hasChanged flag in there - we don't generate
        // code that way
        mtis[miter]->hasChanged_ = false;
    }
    AstNode *minis = AstNode::sequenceNode(miniTramps);

    // Let's build the tramp guard addr (if we want it)
    AstNode *threadIndex = NULL;
    AstNode *trampGuardAddr = NULL;
    if (baseT->guarded() &&
        minis->containsFuncCall() &&
        (proc()->trampGuardAST() != NULL)) {
        // If we don't have a function call, then we don't
        // need the guard....

        // Now, the guard flag. 
        // If we're multithreaded, we need to index off
        // the base address.
        if (!baseT->threaded()) {
            // ...
        }
        else if (gen.thread()) {
            // Constant override...
            threadIndex = AstNode::operandNode(AstNode::Constant,
                                               (void *)gen.thread()->get_index());
        }
        else {
            // TODO: we can get clever with this, and have the generation of
            // the thread index depend on whether gen has a thread defined...
            // For that, we'd need an AST thread index node. Just something
            // to think about. Maybe a child of funcNode?
            threadIndex = AstNode::threadIndexNode();
        }
        
        if (threadIndex) {
            trampGuardAddr = AstNode::operandNode(AstNode::DataIndir,
                                                  AstNode::operatorNode(plusOp,
                                                                        AstNode::operatorNode(getAddrOp,
                                                                                              proc()->trampGuardAST()),
                                                                        AstNode::operatorNode(timesOp,
                                                                                              threadIndex,
                                                                                              AstNode::operandNode(AstNode::Constant, 
                                                                                                                   (void *)sizeof(unsigned)))));
            /* At the moment, we can't directly specify the fact
               that we're loading 4 bytes instead of a normal
               (address-width) word. */
            trampGuardAddr->setType( BPatch::getBPatch()->builtInTypes->findBuiltInType( "unsigned int" ) );
        }
        else {
            trampGuardAddr = proc()->trampGuardAST();
        }
    }


    AstNode *baseTrampSequence = NULL;
    pdvector<AstNode *> baseTrampElements;

    if (trampGuardAddr) {
        // First, set it to 0
        baseTrampElements.push_back(AstNode::operatorNode(storeOp, 
                                                          trampGuardAddr,
                                                          AstNode::operandNode(AstNode::Constant,
                                                                               (void *)0)));
    }
    
    // Run the minitramps
    baseTrampElements.push_back(minis);
    
    // Cost code...
    //
    
    if (trampGuardAddr) {
        // And set the tramp guard flag to 1
        baseTrampElements.push_back(AstNode::operatorNode(storeOp,
                                                          trampGuardAddr,
                                                          AstNode::operandNode(AstNode::Constant,
                                                                               (void *)1)));
    }

    baseTrampSequence = AstNode::sequenceNode(baseTrampElements);

    AstNode *baseTramp = NULL;

    // If trampAddr is non-NULL, then we wrap this with an IF. If not, 
    // we just run the minitramps.
    if (trampGuardAddr == NULL) {
        baseTramp = baseTrampSequence;
        baseTrampSequence = NULL;
    }
    else {
        // Oh, boy. 
#if 0
        baseTramp = AstNode::operatorNode(ifOp,
                                          AstNode::operatorNode(eqOp,
                                                                trampGuardAddr,
                                                                AstNode::operandNode(AstNode::Constant, (void *)1)),
                                          baseTrampSequence);
#endif
        baseTramp = AstNode::operatorNode(ifOp,
                                          trampGuardAddr,
                                          baseTrampSequence);
    }


#if defined(cap_unwind)
      if (baseT->baseTrampRegion != NULL) {
          free(baseT->baseTrampRegion);
          baseT->baseTrampRegion = NULL;
      }
#endif


    trampAddr_ = gen.currAddr();

    // Sets up state in the codeGen object (and gen.rs())
    // that is later used when saving and restoring. This
    // MUST HAPPEN BEFORE THE SAVES, and state should not
    // be reset until AFTER THE RESTORES.
    baseTramp->initRegisters(gen);

    baseT->generateSaves(gen, gen.rs());
    Address endSaves = gen.currAddr();
    bool retval = true;
    if (!baseTramp->generateCode(gen, false)) {
        fprintf(stderr, "Gripe: base tramp creation failed\n");
        retval = false;
    }
    Address beginRestores = gen.currAddr();
    baseT->generateRestores(gen, gen.rs());
    trampSize_ = gen.currAddr() - trampAddr_;

    // And now to clean up after us
    //if (minis) delete minis;
    //if (trampGuardAddr) delete trampGuardAddr;
    //if (baseTrampSequence) delete baseTrampSequence;
    //if (baseTramp) delete baseTramp;

#if defined( cap_unwind )
	/* At the moment, AST nodes can /not/ modify unwind state.  (Whoo-hoo!)
	   Therefore, to account for them, we just need to know how much code
	   they generated... */
    baseT->baseTrampRegion->insn_count += ((beginRestores - endSaves)/16) * 3;
    
	/* appendRegionList() returns the last valid region it duplicated.
	   This leaves unwindRegion pointing to the end of the list. */
    * unwindRegion = appendRegionList( * unwindRegion, baseT->baseTrampRegion );
#endif /* defined( cap_unwind ) */

    generated_ = true;
    hasChanged_ = false;

    return retval;
}

bool baseTrampInstance::installCode() {
    // All BTIs are in-lined, so this has no direct work to do;
    // call into minitramps and then ensure that all jumps
    // are correct.

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
#if defined(os_aix) 
        // AIX uses funky branches
        instruction::generateInterFunctionBranch(gen,
                                                 trampPreAddr() + baseT->instStartOffset,
                                                 firstTarget);
#else
        instruction::generateBranch(gen,
				    trampPreAddr() + baseT->instStartOffset,
				    firstTarget);
#endif
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
  if (instP_)
    return instP_->proc();
  if (rpcMgr_)
    return rpcMgr_->proc();
  return NULL;
}

Address baseTramp::origInstAddr() {
  // Where we would be if we weren't relocated, if we were in the
  // original spot, etc. etc. etc.  Base tramp instances aren't
  // necessarily sorted by anything meaningful, so check the long
  // way.
  
  // TODO: a post tramp _should_ return the next addr, but hey...

  if (!instP_) {
    assert(rpcMgr_ != NULL);
    return 0;
  }
  
  return instP()->addr();
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
	// orderFirstAtPoint, post-instrumentation
	assert(firstMini);
	firstMini->prev = newMT;
	newMT->next = firstMini;
	firstMini = newMT;
      }
      else {
	// orderLastAtPoint, post-instrumentation
	assert(lastMini);
	lastMini->next = newMT;
	newMT->prev = lastMini;
	lastMini = newMT;
      }
    }

    // Push to baseTrampInstances
    for (unsigned i = 0; i < instances.size(); i++) {
        instances[i]->updateMTInstances();
    }

    assert(firstMini != NULL);
    assert(lastMini != NULL);

    instVersion_++;

    return true;
}

bool baseTrampInstance::isInInstance(Address pc) {
    assert(baseT);
    if (trampPreAddr() == 0) {
        // Not allocated yet...
        return false;
    }

    if (trampSize_) { // Inline
        return ((pc >= trampAddr_) &&
                (pc < (trampAddr_ + trampSize_)));
    }

    return (pc >= trampPreAddr() &&
            pc < (trampPostAddr() + baseT->postSize));
}

bool baseTrampInstance::isInInstru(Address pc) {
    assert(baseT);
    return (pc >= (trampPreAddr() + baseT->saveStartOffset) &&
            pc < (trampPostAddr() + baseT->restoreEndOffset));
}

instPoint *baseTrampInstance::findInstPointByAddr(Address /*addr*/) {
    assert(baseT);
    return baseT->instP_;
}

bool baseTrampInstance::isEmpty() {
    return (mtis.size() == 0);
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
    if (instP()) {
        if (instP()->preBaseTramp() == this)
            instP()->preBaseTramp_ = NULL;
        
        if (instP()->postBaseTramp() == this)
            instP()->postBaseTramp_ = NULL;
      
      if (instP()->targetBaseTramp() == this)
         instP()->targetBaseTramp_ = NULL;
    }

    instP_ = NULL;

    instances.clear();
    delete this;
}

// Where should the minitramps jump back to?
Address baseTrampInstance::miniTrampReturnAddr() {
    // Might want to make an offset, but for now we go straight back.
    return trampPostAddr();
}

bool baseTramp::isConservative() {
  if (instP() && instP()->getPointType() == otherPoint)
    return true;
  if (rpcMgr_)
    return true;
  return false;
}

bool baseTramp::isCallsite() {
    if (instP() && instP()->getPointType() == callSite)
        return true;

    return false;
}

bool baseTramp::isEntryExit() {
  if (instP() && ((instP()->getPointType() == functionEntry) ||
		(instP()->getPointType() == functionExit)))
    return true;
  
  return false;
}

bool baseTrampInstance::shouldGenerate() {
    miniTramp *mini = baseT->firstMini;
    // No miniTramps, no work, no bother.
    return (mini != NULL);
}

unsigned baseTrampInstance::maxSizeRequired() {
    assert(this);

    updateMTInstances();

    if (isEmpty())
        return 0;

    unsigned size = 0;


    if (BPatch::bpatch->isMergeTramp()) {
	// TEMPORARY HACK!!! WILL NEED TO GET SOMETHING BETTER THAT
	// FIGURES OUT THE SIZE FOR THE BASE TRAMP WITHOUT MAKING IT
	// SO THE MINI-TRAMP IS GENERATED AFTER THE BASE TRAMP,
	// FOR NOW, WE'LL JUST ERR ON THE SAFE SIDE FOR THE BUFFER
		size += 100; // For the base tramp
    }
    else {
		if (!baseT->valid)
            baseT->generateBT(codeGen::baseTemplate);
        
		assert(baseT->valid);
		size += baseT->preSize + baseT->postSize;
    }


    for (unsigned i = 0; i < mtis.size(); i++)
      size += mtis[i]->maxSizeRequired();
    
#if defined(arch_ia64)
	size *= 8; // Enormous on this platform...
#endif
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
    unsigned oldNum = mtis.size();
    mtis.clear();

    miniTramp *mini = baseT->firstMini;
    while (mini) {
        miniTrampInstance *mti = mini->getMTInstanceByBTI(this);
        mtis.push_back(mti);
        if (mti->hasChanged()) {
            hasChanged_ = true;
        }
        mini = mini->next;
    }

    // If we now have an MTI, we've changed (as previously
    // we wouldn't bother generating code)
    if ((oldNum == 0) &&
        (mtis.size() != 0)) {
        hasChanged_ = true;
    }
    inst_printf("BTI %p update: %d originally, %d now\n",
                this, oldNum, mtis.size());
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

bool baseTramp::doOptimizations() 
{
   miniTramp *cur_mini = firstMini;
   bool hasFuncCall = false;

   if (!BPatch::bpatch->isMergeTramp())
      return false;

   while (cur_mini) {
      AstMiniTrampNode *ast = cur_mini->ast_;
      if (ast->containsFuncCall()) {
         hasFuncCall = true;
         break;
      }
      cur_mini = cur_mini->next;
   }
   
   if (!hasFuncCall) {
      guardState_ = unset_BTR;
      setRecursive(true, true);
      optimized_out_guards = true;
      setThreaded(false);
      return true;
   }
   return false;
}

void baseTramp::setRecursive(bool trampRecursive, bool force) {
   if (force) {
      guardState_ = trampRecursive ? recursive_BTR : guarded_BTR;
      return;
   }
    if (trampRecursive) {
        if (guardState_ == unset_BTR)
            guardState_ = recursive_BTR;
        else {
            if (guardState_ == guarded_BTR) {
#if defined(os_aix) 
		/* 	this is part of the code to ensure that when we add the call to dlopen
			at the entry of main on AIX during save the world, an UNGUARDED tramp is produced
		*/
		if( !proc()->requestTextMiniTramp ){
#endif
                    //cerr << "WARNING: collision between pre-existing guarded miniTramp and new miniTramp, keeping guarded!" << endl;
#if defined(os_aix) 
		}else{
			//override the tramp guard state if we are doing savetheworld on AIX

			guardState_ = recursive_BTR; //ccw 24 jan 2006 
		}
#endif
            }
        }
    }
    else {
        if (guardState_ == unset_BTR) {
            guardState_ = guarded_BTR;
        }
        else {
            if (guardState_ == recursive_BTR) {
               if (!optimized_out_guards)
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
bool baseTramp::generateBT(codeGen &baseGen) {

  if (valid && !(BPatch::bpatch->isMergeTramp())
#if defined(os_aix) 
	/* 	this is part of the code to ensure that when we add the call to dlopen
		at the entry of main on AIX during save the world, any multi that was
		already there gets regenerated
	*/
    && !proc()->requestTextMiniTramp 
#endif
     ){
      return true; // May be called multiple times
    }
  else
    {
      preTrampCode_.invalidate();
      postTrampCode_.invalidate();

#if defined(cap_unwind)
      if (baseTrampRegion != NULL) {
          free(baseTrampRegion);
          baseTrampRegion = NULL;
      }
#endif
    }
    //inst_printf("Generating a baseTramp, guarded %d\n", guardState_);
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
    
	// Set up the registerSpace for the baseGen structure
	// If we're calling generateBT, we're out-of-lining... so 
	// we can't take advantage of instr-unused registers
	baseGen.setRegisterSpace(registerSpace::actualRegSpace(instP()));
	

  assert(preTrampCode_ == NULL);
  assert(postTrampCode_ == NULL);
  preTrampCode_.applyTemplate(baseGen);
  preTrampCode_.allocate(PRE_TRAMP_SIZE);
  postTrampCode_.applyTemplate(baseGen);
  postTrampCode_.allocate(POST_TRAMP_SIZE);

  preTrampCode_.setProcess(proc());
  postTrampCode_.setProcess(proc());

    preTrampCode_.setRegisterSpace(baseGen.rs());
    
    
    saveStartOffset = preTrampCode_.used();
    inst_printf("Starting saves: offset %d\n", saveStartOffset);

    generateSaves(preTrampCode_, preTrampCode_.rs());

    // Done with save
    saveEndOffset = preTrampCode_.used();
    inst_printf("Starting MT: offset %d\n", saveEndOffset);
    

#if defined(os_aix) 
	/* 	if requestTextMiniTramp is set then we are instrumenting the
		entry point of main for save the world on AIX.
		
		We only want to disable the MT code for the BaseTramp that
		calls dlopen, which will be the FIRST BaseTramp in the MultiTramp.
	
		Hence, the increment of requestTextMiniTramp.  This gets called
		TWO times (where we want to skip the MT code) before we see the second 
		BaseTramp in the MultiTram, where we DO want MT code.
	*/
    if (proc()->requestTextMiniTramp==0 || proc()->requestTextMiniTramp>2 ){
#endif
  // Multithread
    generateMTCode(preTrampCode_, preTrampCode_.rs());
#if defined(os_aix) 
   }else{
	proc()->setRequestTextMiniTramp(proc()->requestTextMiniTramp+1);
   }
#endif


    // Guard code
    guardLoadOffset = preTrampCode_.used();
    inst_printf("Starting guard: offset %d\n", guardLoadOffset);
    if (guarded() &&
        generateGuardPreCode(preTrampCode_,
                             guardBranchIndex,
                             preTrampCode_.rs())) {
        // Cool.
    }
    else {
        // No guard...
        guardBranchIndex = 0;
        guardLoadOffset = 0;
    }
    
    

    costUpdateOffset = preTrampCode_.used();

#if defined(os_aix) 
    if (!proc()->requestTextMiniTramp ){
#endif

    inst_printf("Starting cost: offset %d\n", costUpdateOffset);

    // We may not want cost code... for now if we're an iRPC tramp
    // In the future, this may change
    // In general, instrumentation wants cost code. The minitramps may be 
    // null, but we put in the stub anyway.
    if (rpcMgr_ == NULL &&
        generateCostCode(preTrampCode_, costValueOffset, preTrampCode_.rs())) {
        costSize = preTrampCode_.used() - costUpdateOffset;
        // If this is zero all sorts of bad happens
        assert(costValueOffset);
    }
    else {
        costSize = 0;
        costValueOffset = 0;
    }
#if defined(os_aix) 
    }else{  //ccw 8 oct 2005
       costSize=0;
       costValueOffset=0;
    }
#endif


    instStartOffset = preTrampCode_.used();
    preSize = preTrampCode_.used();
    inst_printf("Starting inst: offset %d\n", instStartOffset);
    inst_printf("preSize is: %d\n", preSize);    

// IA-64...
	if (preTrampCode_.rs() != baseGen.rs()) {
		// Grab any updates that were made
		baseGen.setRegisterSpace(preTrampCode_.rs());
	}

    postTrampCode_.setRegisterSpace(baseGen.rs());


    // Post...
    // Guard redux
    if (guarded())
        generateGuardPostCode(postTrampCode_, 
                              guardTargetIndex,
                              postTrampCode_.rs());
    else
        guardTargetIndex = 0;

    // Restore registers
    restoreStartOffset = postTrampCode_.used();
    generateRestores(postTrampCode_, postTrampCode_.rs());

//#if defined(arch_power)
//    baseGen.rs()->setAllLive();
//#endif 

    restoreEndOffset = postTrampCode_.used();
    //inst_printf("Ending restores: %d\n", restoreEndOffset);

    trampEnd = postTrampCode_.used();
    postSize = postTrampCode_.used();

    valid = true;
    preTrampCode_.finalize();
    postTrampCode_.finalize();

	if (regSpace_)
		delete regSpace_;
    regSpace_ = new registerSpace(*(baseGen.rs()));

    /*
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
    */
#if defined( cap_unwind )
	/* FIXME: the guard and multitramp code don't yet update the baseTrampRegion
	   insn_count themselves, so do that here. */
#if defined( arch_ia64 )
	baseTrampRegion->insn_count += ( (instStartOffset - saveEndOffset)/ 16 ) * 3;
	baseTrampRegion->insn_count += ( (restoreStartOffset - 0)/ 16 ) * 3;
#else
#error How do I know how many instructions are in the jump region?
#endif /* defined( arch_ia64 ) */
#endif /* defined( cap_unwind ) */

    return true;
}
    
void baseTrampInstance::removeCode(generatedCodeObject *subObject) {
    miniTrampInstance *delMTI = dynamic_cast<miniTrampInstance *>(subObject);
    multiTramp *delMulti = dynamic_cast<multiTramp *>(subObject);
    assert(delMTI || delMulti);

    if (delMTI) {
        // We lost a miniTramp...
        
        // Move the MTI from the current list to the deleted list.
        for (unsigned i = 0; i < mtis.size(); i++) {
            if (mtis[i] == subObject) {
                if (BPatch::bpatch->isMergeTramp()) 
                    deletedMTIs.push_back(mtis[i]);
                mtis[i] = mtis.back();
                mtis.pop_back();
                break;
            }
        }
        
        // See if this is the last MTI; if so, delete ourselves.
        if (isEmpty()) {
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
            if (BPatch::bpatch->isMergeTramp()) {
                hasChanged_ = true;
            }
            else {
                // we occasionally see a case where this->trampAddr_ has 
                // already been reset to zero, assume no need to update
                if(trampPreAddr() != 0) {
                    codeGen gen(instruction::maxJumpSize());
                    generateBranchToMT(gen);
                    proc()->writeDataSpace((void *)(trampPreAddr() + baseT->instStartOffset),
                                           gen.used(),
                                           gen.start_ptr());
                }
            }
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
            if (!BPatch::bpatch->isMergeTramp())
                deletedMTIs.push_back(mtis[j]);
        }
        mtis.clear();

        baseT->unregisterInstance(this);
        alreadyDeleted_ = true;
    }
           
}

bool baseTrampInstance::safeToFree(codeRange *range) {

    if (dynamic_cast<baseTrampInstance *>(range) == this)
        return false;

    // Better not be freeing ourselves if we have sub-mtis left...
    assert(isEmpty());

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
    generatedCodeObject::invalidateCode();
    trampAddr_ = 0;
    trampSize_ = 0;
    trampPostOffset = 0;
    for (unsigned i = 0; i < mtis.size(); i++)
        mtis[i]->invalidateCode();
}

bool baseTrampInstance::linkCode() {
    if (isEmpty()) {

        linked_ = true;
        return true;
    }

    unsigned cost = 0;
    for (unsigned i = 0; i < mtis.size(); i++) {
        mtis[i]->linkCode();
        cost += mtis[i]->cost();
    }

    if (!BPatch::bpatch->isMergeTramp()) {
      Address leave = trampPreAddr() + baseT->instStartOffset;
      
      Address arrive = baseT->firstMini->getMTInstanceByBTI(this)->trampBase;
        
        inst_printf("writing branch from 0x%x to 0x%x, baseT (%p)->miniT (%p)\n",
                    leave, arrive,
                    this,
                    baseT->firstMini->getMTInstanceByBTI(this));
        generateAndWriteBranch(baseT->proc(), 
                               leave, 
                               arrive, 
                               instruction::maxJumpSize());
    }

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
    
    if (!generated_) {
        multiT = newMulti;
        return this;
    }
    
    baseTrampInstance *newBTI = baseT->findOrCreateInstance(newMulti);
    assert(newBTI);
    for (unsigned i = 0; i < mtis.size(); i++) {
        
        generatedCodeObject *gen = mtis[i]->replaceCode(newBTI);
        miniTrampInstance *newMTI = dynamic_cast<miniTrampInstance *>(gen);
        assert(newMTI);
    }
    newBTI->updateMTInstances();
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

Address baseTrampInstance::uninstrumentedAddr() const {
    if (fallthrough_)
        return fallthrough_->uninstrumentedAddr();
    else if (previous_)
        return previous_->uninstrumentedAddr();
    else 
        assert(0);
    return 0;
}

bool baseTramp::threaded() const {
   if (!proc()->multithread_ready() || suppress_threads_)
      return false;
   return true;
}

void baseTramp::setThreaded(bool new_val)
{
   if (suppress_threads_ != new_val)
      return;
   if (valid)
   {
      valid = false;
      preTrampCode_.invalidate();
      postTrampCode_.invalidate();
   }
   suppress_threads_ = !new_val;
}

void *baseTrampInstance::getPtrToInstruction(Address /*addr*/ ) const {
    assert(0); // FIXME if we do out-of-line baseTramps
    return NULL;
}
