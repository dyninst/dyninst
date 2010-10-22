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
#include "mapped_object.h"

#if defined(os_aix)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

int_function *baseTramp::func() const { 
   return instP()->func();
}

// Normal constructor
baseTrampInstance::baseTrampInstance(baseTramp *tramp) :
    trampAddr_(0), // Unallocated
    trampSize_(0),
    trampPostOffset(0),
    saveStartOffset(0),
    saveEndOffset(0),
    restoreStartOffset(0),
    restoreEndOffset(0),
    baseT(tramp),
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
                                     process *child) :
    trampAddr_(parBTI->trampAddr_),
    trampPostOffset(parBTI->trampPostOffset),
    saveStartOffset(parBTI->saveStartOffset), 
    saveEndOffset(parBTI->saveEndOffset),
    restoreStartOffset(parBTI->restoreStartOffset), 
    restoreEndOffset(parBTI->restoreEndOffset),
    baseT(cBT),
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
   assert(0);
#if 0
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
#endif
}

baseTrampInstance::~baseTrampInstance() {
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
    instP_(iP),
    rpcMgr_(NULL),
    valid(false),
    optimized_out_guards(false),
    guardState_(guarded_BTR),
    suppress_threads_(false),
    savedFlagSize(0), 
    createFrame_(true),
    when_(when),
    wasNonEmpty_(false)
{
}

baseTramp::baseTramp(const baseTramp *pt, process *child) :
    instP_(NULL),
    valid(pt->valid),
    optimized_out_guards(false),
    guardState_(pt->guardState_),
    suppress_threads_(pt->suppress_threads_),
    createFrame_(pt->createFrame_),
    when_(pt->when_)
{
    for (const_iterator iter = pt->begin(); iter != pt->end(); ++iter) {
       miniTramp *childMini = new miniTramp(*iter, this, child);
       miniTramps_.push_back(childMini);
    }

    rpcMgr_ = NULL;
    if (pt->rpcMgr_) {
        rpcMgr_ = child->getRpcMgr();
    }
}

baseTramp::~baseTramp() {
}

int_function *baseTrampInstance::func() const {
   return baseT->func();
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
                                     Address baseInMutatee) {
    inst_printf("baseTrampInstance %p ::generateCode(%p, 0x%x, %d)\n",
                this, gen.start_ptr(), baseInMutatee, gen.used());
    
    if (baseT->empty()) {
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
       bool result = generateCodeInlined(gen, baseInMutatee);
       if (!result)
          return false;
       gen.endTrackRegDefs();

       definedRegs = gen.getRegsDefined();
       setHasOptInfo(true);
       if (!spilledRegisters()) {
          setSpilledRegisters(gen.rs()->spilledAnything());
       }

       if (!shouldRegenBaseTramp(gen.rs())) {
          break;
       }
	  
       gen.setPCRelUseCount(gen.rs()->pc_rel_use_count);
       
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
                                            Address) {
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
    for (baseTramp::iterator iter = baseT->begin(); iter != baseT->end(); ++iter) {
       miniTramps.push_back((*iter)->ast_->getAST());
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

    return retval;
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
   if (order == orderFirstAtPoint) {
      miniTramps_.push_front(newMT);
   }
   else {
      miniTramps_.push_back(newMT);
   }

   wasNonEmpty_ = true;

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


// Where should the minitramps jump back to?
Address baseTrampInstance::miniTrampReturnAddr() {
    // Might want to make an offset, but for now we go straight back.
    return trampPostAddr();
}

bool baseTramp::isConservative() {
    if (instP() && (instP()->getPointType() == otherPoint))
    {
        return true;
    }
  if (rpcMgr_)
  {
      return true;
  }
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

cfjRet_t baseTrampInstance::checkForFuncJumps()
{
   if (hasFuncJump_ != cfj_unset)
      return hasFuncJump_;

   hasFuncJump_ = cfj_none;
   for (baseTramp::iterator iter = baseT->begin();
        iter != baseT->end(); ++iter) {
      miniTramp *cur = *iter;
      cfjRet_t tmp = cur->ast_->containsFuncJump();
      if ((int) tmp > (int) hasFuncJump_)
         hasFuncJump_ = tmp;
   }
   
   return hasFuncJump_;
}

bool baseTramp::doOptimizations() 
{
   bool hasFuncCall = false;
   bool usesReg = false;

   if (BPatch::bpatch->getInstrStackFrames()) {
      usesReg = true;
   }
   
   for (iterator iter = begin(); iter != end(); ++iter) {
      miniTramp *cur_mini = *iter;
      assert(cur_mini->ast_);
      if (!hasFuncCall && cur_mini->ast_->containsFuncCall()) {
         hasFuncCall = true;
      }
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


AddressSpace *baseTrampInstance::proc() const {
    assert(baseT);
    return baseT->proc();
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

int_function *baseTrampInstance::func() const
{
    return baseT->instP()->func();
}
