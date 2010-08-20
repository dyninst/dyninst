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

// $Id: baseTramp.C,v 1.68 2008/09/03 06:08:44 jaw Exp $

#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/h/BPatch.h"
#include "debug.h"
#include "process.h"

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
    saveStartOffset(0),
    saveEndOffset(0),
    restoreStartOffset(0),
    restoreEndOffset(0),
    baseT(tramp),
    multiT(multi),
    genVersion(0),
    alreadyDeleted_(false),
    hasOptInfo_(false),
    spilledRegisters_(false),
    hasLocalSpace_(false),
    hasStackFrame_(false),
    flags_saved_(false),
    saved_fprs_(false),
    saved_orig_addr_(false),
    hasFuncJump_(cfj_unset),
    trampStackHeight_(0)
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
    saveStartOffset(parBTI->saveStartOffset), 
    saveEndOffset(parBTI->saveEndOffset),
    restoreStartOffset(parBTI->restoreStartOffset), 
    restoreEndOffset(parBTI->restoreEndOffset),
    baseT(cBT),
    multiT(cMT),
    genVersion(parBTI->genVersion),
    alreadyDeleted_(parBTI->alreadyDeleted_),
    hasOptInfo_(parBTI->hasOptInfo_),
    spilledRegisters_(parBTI->spilledRegisters_),
    hasLocalSpace_(parBTI->hasLocalSpace_),
    hasStackFrame_(parBTI->hasStackFrame_),
    flags_saved_(parBTI->flags_saved_),
    saved_fprs_(parBTI->saved_fprs_),
    saved_orig_addr_(parBTI->saved_orig_addr_),
    hasFuncJump_(parBTI->hasFuncJump_),
    trampStackHeight_(parBTI->trampStackHeight_)
{
    // Register with parent
    cBT->instances.push_back(this);
    // And copy miniTrampInstances
    for (unsigned i = 0; i < parBTI->mtis.size(); i++) {
        miniTramp *cMini = NULL;

        cMini = parBTI->mtis[i]->mini->getInheritedMiniTramp(child);

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
            VECTOR_ERASE(instances,i, i);
            return;
        }
    }
}

bool baseTrampInstance::shouldRegenBaseTramp(registerSpace *rs)
{
#if !defined(cap_tramp_liveness)
   return false;
#endif

   int saved_unneeded = 0;
   unsigned actually_saved = 0;
   int needed_saved = 0;
   
   regalloc_printf("BT: checking for unneeded saved registers (in %p)\n", this);

   if (spilledRegisters() && !hasLocalSpace())
      return true;

   pdvector<registerSlot *> &regs = rs->trampRegs();
   for (unsigned i = 0; i < regs.size(); i++) {
      registerSlot *reg = regs[i];
      regalloc_printf("[%s:%u] - checking reg (index %d, number %d, encoding %d)\n", __FILE__, 
		      __LINE__, i, reg->number, reg->encoding());

      if (reg->spilledState != registerSlot::unspilled) {
	regalloc_printf("[%s:%u] - reg %d saved\n", __FILE__, 
			__LINE__, reg->number);
         actually_saved++;
      }
      if (definedRegs[reg->encoding()]) {
	regalloc_printf("[%s:%u] - reg %d used\n", __FILE__, 
			__LINE__, reg->number);
	needed_saved++;
      }

      if ((reg->spilledState != registerSlot::unspilled) &&
          (!definedRegs[reg->encoding()]) &&
          (!reg->offLimits))
      {
         saved_unneeded++;
         regalloc_printf("[%s:%u] - baseTramp saved unneeded register %d, "
                         "suggesting regen\n", __FILE__, __LINE__, reg->number);
      }
      if (!reg->liveState == registerSlot::spilled &&
          definedRegs[reg->encoding()])
      {
         regalloc_printf("[%s:%u] - Decided not to save a defined register %d. "
                         "App liveness?\n",  __FILE__, __LINE__, reg->number);         
      }
   }
   regalloc_printf("[%s:%u] - Should regen found %d unneeded saves\n",
                   __FILE__, __LINE__, saved_unneeded);
#if defined(arch_x86_64) || defined(arch_x86)
   if (baseT->proc()->getAddressWidth() == 4)
   {
      //No regen if we did a pusha and saved more regs than the 
      // X86_REGS_SAVE_LIMIT recommended limit.
      if (actually_saved == regs.size() &&
          needed_saved > X86_REGS_SAVE_LIMIT) {
         return false;
      }
   }
#endif
   return (saved_unneeded != 0);
}

#define PRE_TRAMP_SIZE 4096
#define POST_TRAMP_SIZE 4096

baseTramp::baseTramp(instPoint *iP, callWhen when) :
#if defined( cap_unwind )
    baseTrampRegion( NULL ),
#endif /* defined( cap_unwind ) */
    instP_(iP),
    rpcMgr_(NULL),
    firstMini(NULL),
    lastMini(NULL),
    valid(false),
    optimized_out_guards(false),
    guardState_(guarded_BTR),
    suppress_threads_(false),
    savedFlagSize(0), 
    createFrame_(true),
    instVersion_(),
    when_(when),
    wasNonEmpty_(false)
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

baseTramp::baseTramp(const baseTramp *pt, process *child) :
    instP_(NULL),
    firstMini(NULL),
    lastMini(NULL),
    valid(pt->valid),
    optimized_out_guards(false),
    guardState_(pt->guardState_),
    suppress_threads_(pt->suppress_threads_),
    createFrame_(pt->createFrame_),
    instVersion_(pt->instVersion_),
    when_(pt->when_)
{
#if defined( cap_unwind )
    baseTrampRegion = duplicateRegionList( pt->baseTrampRegion );
#endif /* defined( cap_unwind ) */

    // And copy minis
    miniTramp *parMini = NULL;
    miniTramp *childPrev = NULL;
    miniTramp *childMini = NULL;
    
    parMini = pt->firstMini;
    while (parMini) {
        childMini = new miniTramp(parMini, this, child);
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
        rpcMgr_ = child->getRpcMgr();
    }
}

baseTramp::~baseTramp() {
}

unsigned baseTrampInstance::get_size() const { 
    // Odd... especially if there are minitramps in the middle. 
    // For now return a+b; eventually we'll need a codeRange
    // that can handle noncontiguous ranges.
    assert(baseT);
    Address endAddr = trampPostAddr() + restoreEndOffset;
    return endAddr - get_address();
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

    gen.setPCRelUseCount(0);
    gen.setBTI(this);
    if (baseT->instP()) {
       //iRPCs already have this set
       gen.setPoint(baseT->instP());
       gen.setRegisterSpace(registerSpace::actualRegSpace(baseT->instP(), baseT->when_));
    }
    int count = 0;

    for (;;) {
       regalloc_printf("[%s:%u] - Beginning baseTramp generate iteration # %d\n",
                       __FILE__, __LINE__, ++count);
       codeBufIndex_t start = gen.getIndex();
       
       unsigned int num_patches = gen.allPatches().size();

       gen.beginTrackRegDefs();
       gen.rs()->initRealRegSpace();
       bool result = generateCodeInlined(gen, baseInMutatee, unwindRegion);
       if (!result)
          return false;
       gen.endTrackRegDefs();

       definedRegs = gen.getRegsDefined();
       setHasOptInfo(true);
       if (!spilledRegisters())
          setSpilledRegisters(gen.rs()->spilledAnything());

       if (!shouldRegenBaseTramp(gen.rs())) {
          break;
       }
	  
       gen.setPCRelUseCount(gen.rs()->pc_rel_use_count);
       
       markChanged(true);

       gen.setIndex(start);
       while (gen.allPatches().size() > num_patches) {
          gen.allPatches().pop_back();
       }
    }

    gen.setBTI(NULL);
    return true;
}


#include "BPatch.h"
#include "BPatch_collections.h"

bool baseTrampInstance::generateCodeInlined(codeGen &gen,
                                            Address baseInMutatee,
                                            UNW_INFO_TYPE **
                                            ) {
#if 0
    if (!hasChanged() && generated_ &&
	(gen.currAddr(baseInMutatee) == trampAddr_)) {
      gen.moveIndex(trampSize_);
      return true;
    }
#endif

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
	
    gen.setRegisterSpace(registerSpace::actualRegSpace(baseT->instP(), baseT->when_));

    pdvector<AstNodePtr> miniTramps;
    for (unsigned miter = 0; miter < mtis.size(); miter++) {
      miniTramps.push_back(mtis[miter]->mini->ast_->getAST());
      // And nuke the hasChanged flag in there - we don't generate
      // code that way
      mtis[miter]->markChanged(false);
      mtis[miter]->markGenerated(true);
    }
    AstNodePtr minis = AstNode::sequenceNode(miniTramps);

    // Let's build the tramp guard addr (if we want it)
    AstNodePtr threadIndex;
    AstNodePtr trampGuardAddr;
    if (baseT->guarded() &&
        minis->containsFuncCall() &&
        (proc()->trampGuardAST() != AstNodePtr())) {
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
                                               (void *)(long)gen.thread()->get_index());
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
                                                                        AstNode::operatorNode(timesOp,
                                                                                              threadIndex,
                                                                                              AstNode::operandNode(AstNode::Constant, 
                                                                                                                   (void *)sizeof(unsigned))),
                                                                        proc()->trampGuardAST()));

            /* At the moment, we can't directly specify the fact
               that we're loading 4 bytes instead of a normal
               (address-width) word. */
            trampGuardAddr->setType( BPatch::getBPatch()->builtInTypes->findBuiltInType( "unsigned int" ) );
        }
        else {
            trampGuardAddr = AstNode::operandNode(AstNode::DataIndir,
                                                  proc()->trampGuardAST());
            trampGuardAddr->setType( BPatch::getBPatch()->builtInTypes->findBuiltInType( "unsigned int" ) );
        }
    }


    AstNodePtr baseTrampSequence;
    pdvector<AstNodePtr > baseTrampElements;

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

    AstNodePtr baseTramp;

    // If trampAddr is non-NULL, then we wrap this with an IF. If not, 
    // we just run the minitramps.
    if (trampGuardAddr == NULL) {
        baseTramp = baseTrampSequence;
        baseTrampSequence.reset();
    }
    else {
        // Oh, boy. 
        // Short form of the above
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
    saveStartOffset = gen.used();
    baseT->generateSaves(gen, gen.rs(), this);
    saveEndOffset = gen.used();
    bool retval = true;

    if (!baseTramp->generateCode(gen, false)) {
      fprintf(stderr, "Gripe: base tramp creation failed\n");
      retval = false;
    }

    trampPostOffset = gen.used();
    restoreStartOffset = 0;
    baseT->generateRestores(gen, gen.rs(), this);
    restoreEndOffset = gen.used() - trampPostOffset;
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
    Address endSaves = saveEndOffset + gen.startAddr();
    Address beginRestores = trampPostOffset + gen.startAddr();
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

    BinaryEdit *binedit = dynamic_cast<BinaryEdit *>(proc());
    if (binedit && BPatch::bpatch->getInstrStackFrames()) {
      createBTSymbol();
    }

    installed_ = true;
    return true;
}


AddressSpace *baseTramp::proc() const { 
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

// Add a miniTramp to the appropriate place: at the start or end of
// its instPoint minis.
bool baseTramp::addMiniTramp(miniTramp *newMT, callOrder order) {
  wasNonEmpty_ = true;
  
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
            pc < (trampPostAddr() + restoreEndOffset));
}

bool baseTrampInstance::isInInstru(Address pc) {
    assert(baseT);
    return (pc >= (trampPreAddr() + saveStartOffset) &&
            pc < (trampPostAddr() + restoreEndOffset));
}

instPoint *baseTrampInstance::findInstPointByAddr(Address /*addr*/) {
    assert(baseT);
    return baseT->instP_;
}

bool baseTrampInstance::isEmpty() const {
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

    for (unsigned i = 0; i < instances.size(); i++) {
      instances[i]->baseT = NULL;
    }

    instances.clear();
    delete this;
}

// Where should the minitramps jump back to?
Address baseTrampInstance::miniTrampReturnAddr() {
    // Might want to make an offset, but for now we go straight back.
    return trampPostAddr();
}

bool baseTramp::isConservative() {
    if (instP() && (instP()->getPointType() == otherPoint))
    return true;
  if (rpcMgr_)
    return true;
  return false;
}

bool baseTramp::isCallsite() {
    if (instP() && (instP()->getPointType() == callSite))
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


    // TEMPORARY HACK!!! WILL NEED TO GET SOMETHING BETTER THAT
    // FIGURES OUT THE SIZE FOR THE BASE TRAMP WITHOUT MAKING IT
    // SO THE MINI-TRAMP IS GENERATED AFTER THE BASE TRAMP,
    // FOR NOW, WE'LL JUST ERR ON THE SAFE SIDE FOR THE BUFFER
    size += 100; // For the base tramp


    for (unsigned i = 0; i < mtis.size(); i++)
      size += mtis[i]->maxSizeRequired();
    
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

cfjRet_t baseTrampInstance::checkForFuncJumps()
{
   if (hasFuncJump_ != cfj_unset)
      return hasFuncJump_;

   hasFuncJump_ = cfj_none;
   miniTramp *cur = baseT->firstMini;
   while (cur) {
      assert(cur->ast_);
      cfjRet_t tmp = cur->ast_->containsFuncJump();
      if ((int) tmp > (int) hasFuncJump_)
         hasFuncJump_ = tmp;
      cur = cur->next;
   }

   return hasFuncJump_;
}

bool baseTramp::doOptimizations() 
{
   miniTramp *cur_mini = firstMini;
   bool hasFuncCall = false;
   bool usesReg = false;

   if (BPatch::bpatch->getInstrStackFrames()) {
      usesReg = true;
   }

   while (cur_mini) {
      assert(cur_mini->ast_);
      if (!hasFuncCall && cur_mini->ast_->containsFuncCall()) {
         hasFuncCall = true;
      }
      cur_mini = cur_mini->next;
   }
   
   setCreateFrame(usesReg);
   
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
   /* Tramp guards now work for static binaries
   BinaryEdit *binEdit = dynamic_cast<BinaryEdit *>(proc());
   if (binEdit && binEdit->getMappedObject()->parse_img()->getObject()->isStaticBinary()) {
   	guardState_ = recursive_BTR;
	return;
   }
   */

   if (force) {
      guardState_ = trampRecursive ? recursive_BTR : guarded_BTR;
      return;
   }
    if (trampRecursive) {
        if (guardState_ == unset_BTR)
            guardState_ = recursive_BTR;
        else {
            if (guardState_ == guarded_BTR) {
                guardState_ = recursive_BTR; //ccw 24 jan 2006 
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
                  //                  cerr << "WARNING: collision between pre-existing recursive miniTramp and new miniTramp, now guarded!" << endl;
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

void baseTrampInstance::removeCode(generatedCodeObject *subObject) {
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
            hasChanged_ = true;
	    //multiT->markChanged(true);
            bool doWeDelete = false;
            multiTramp::replaceMultiTramp(multiT, doWeDelete);
            if (doWeDelete && (!multiT || !multiT->getIsActive())) {
                proc()->deleteGeneratedCode(multiT);
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

AddressSpace *baseTrampInstance::proc() const {
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

    for (unsigned i = 0; i < mtis.size(); i++) {
        mtis[i]->linkCode();
    }

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
   valid = false;

   suppress_threads_ = !new_val;
}



void *baseTrampInstance::getPtrToInstruction(Address /*addr*/ ) const {
    assert(0); // FIXME if we do out-of-line baseTramps
    return NULL;
}

Address baseTrampInstance::trampPreAddr() const 
{ 
   return trampAddr_;
}

Address baseTrampInstance::trampPostAddr() const 
{ 
   return trampPreAddr() + trampPostOffset; 
}

Address baseTrampInstance::get_address() const
{
   if (trampAddr_)
      return trampAddr_;

   if (isEmpty() && generated_ && fallthrough_) {
      return fallthrough_->get_address();
   }
   
   return 0;
}

bool baseTramp::createFrame()
{
   return createFrame_;
}

void baseTramp::setCreateFrame(bool frame)
{
   createFrame_ = frame;
}

int baseTrampInstance::numDefinedRegs()
{
   int count = 0;
   if (!hasOptInfo())
      return -1;
   registerSpace *rs = registerSpace::getRegisterSpace(proc()->getAddressWidth());
   pdvector<registerSlot *> &regs = rs->trampRegs();
   for (unsigned i=0; i<regs.size(); i++) {
      registerSlot *reg = regs[i];
      if (definedRegs[reg->encoding()]) {
         count++;
      }
   }
   return count;
}

SymtabAPI::Symbol *baseTrampInstance::createBTSymbol()
{
  using SymtabAPI::Symtab;
  using SymtabAPI::Symbol;
  using SymtabAPI::Region;

  //Make a symbol on this baseTramp to help ID it.
  BinaryEdit *binedit = dynamic_cast<BinaryEdit *>(proc());
  assert(binedit);

  Symtab *symobj = binedit->getMappedObject()->parse_img()->getObject();
  std::stringstream name_stream;
  name_stream << "dyninstBT_" << std::hex << get_address() << "_" << std::dec << get_size();
  if (hasStackFrame_) {
    name_stream << "_" << std::hex << trampStackHeight_;
  }
  std::string name = name_stream.str();
  
  Region *textsec = NULL;
  symobj->findRegion(textsec, ".text");

  Symbol *newsym = new Symbol(name,
			      Symbol::ST_FUNCTION,
			      Symbol::SL_LOCAL,
			      Symbol::SV_INTERNAL,
			      get_address(),
			      symobj->getDefaultModule(),
			      textsec,
			      get_size(),
			      false,
			      false);
  assert(newsym);
  symobj->addSymbol(newsym);
  return newsym;
}

int baseTrampInstance::funcJumpSlotSize()
{
   switch (checkForFuncJumps()) {
      case cfj_unset: assert(0);
      case cfj_none: return 0;
      case cfj_jump: return 1;
      case cfj_call: return 2;
   }
   assert(0);
   return 0;
}
