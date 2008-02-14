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

// $Id: liveness.C,v 1.6 2008/02/14 19:58:59 bernat Exp $

#if defined(cap_liveness)

#include "debug.h"
#include "image-func.h"
#include "function.h"
#include "instPoint.h"
#include "registerSpace.h"
#include "debug.h"
#include "InstrucIter.h"
#include "symtab.h"

#if defined(arch_x86) || defined(arch_x86_64)
// Special-casing for IA-32...
#include "inst-x86.h"
#endif

// Code for register liveness detection

// Takes information from instPoint and resets
// regSpace liveness information accordingly
// Right now, all the registers are assumed to be live by default
void registerSpace::specializeSpace(const bitArray &liveRegs) {
    // Liveness info is stored as a single bitarray for all registers.

#if defined(arch_x86) || defined(arch_x86_64) 
    // We use "virtual" registers on the IA-32 platform (or AMD-64 in 
    // 32-bit mode), and thus the registerSlot objects have _no_ relation
    // to the liveRegs input set. We handle this as a special case, and
    // look only for the flags representation (which is used to set
    // the IA32_FLAG_VIRTUAL_REGISTER "register"
    assert(liveRegs.size() == getBitArray().size());
    if (addr_width == 4) {
        registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::dead;
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            if (liveRegs[i]) {
                registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::live;
                break;
            }
        }
        // All we care about for now.
        return;
    }
#endif
    assert(liveRegs.size() == getBitArray().size());
    for (regDictIter i = registers_.begin(); i != registers_.end(); i++) {
        if (liveRegs[i.currval()->number])
            i.currval()->liveState = registerSlot::live;
        else
            i.currval()->liveState = registerSlot::dead;
    }
#if defined(arch_x86_64)
    // ???
    registers_[REGNUM_RAX]->liveState = registerSlot::live;
#endif

}

const bitArray &image_basicBlock::getLivenessIn() {
    // Calculate if it hasn't been done already
    if (in.size() == 0)
        summarizeBlockLivenessInfo();
    return in;
}

const bitArray image_basicBlock::getLivenessOut() const {
    bitArray out(in.size());

    // OUT(X) = UNION(IN(Y)) for all successors Y of X
    pdvector<image_edge *> target_edges;
    getTargets(target_edges);
    
    for(unsigned i = 0; i < target_edges.size(); i++) {
        if (target_edges[i]->getType() == ET_CALL) continue;
        // Is this correct?
        if (target_edges[i]->getType() == ET_CATCH) continue;
        
        // TODO: multiple entry functions and you?
        
        if (target_edges[i]->getTarget()) {
            out |= target_edges[i]->getTarget()->getLivenessIn();
        }
    }
    return out;
}

void image_basicBlock::summarizeBlockLivenessInfo() 
{
    if(in.size())
    {
      return;
    }

    stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

    unsigned width = getFirstFunc()->img()->getObject()->getAddressWidth();

    in = registerSpace::getBitArray();
    def = in;
    use = in;

    liveness_printf("%s[%d]: Getting liveness summary for block starting at 0x%lx\n", 
                    FILE__, __LINE__, firstInsnOffset());

    bitArray read = in;
    bitArray written = in;

    InstrucIter ii(this);
    while(ii.hasMore()) {
        std::set<Register> tmpRead;
        std::set<Register> tmpWritten;
        ii.getAllRegistersUsedAndDefined(tmpRead, tmpWritten);

        // We need to get numbers into our bit array representation
        // However, we want a 0..n numbering rather than the native
        // register numbers. For now, we map via the registerSpace. 

        for (std::set<Register>::const_iterator i = tmpRead.begin(); i != tmpRead.end(); i++) {
            read[*i] = true;
        }
        for (std::set<Register>::const_iterator i = tmpWritten.begin(); 
             i != tmpWritten.end(); i++) {
            written[*i] = true;
        }

        // TODO "If trusting the ABI..."
        // Otherwise we should go interprocedural
        if (ii.isACallInstruction()) {
            read |= (registerSpace::getRegisterSpace(width)->getCallReadRegisters());
            written |= (registerSpace::getRegisterSpace(width)->getCallWrittenRegisters());
        }
        if (ii.isAReturnInstruction()) {
            read |= (registerSpace::getRegisterSpace(width)->getReturnReadRegisters());
            // Nothing written implicitly by a return
        }
        if (ii.isSyscall()) {
            read |= (registerSpace::getRegisterSpace(width)->getSyscallReadRegisters());
            written |= (registerSpace::getRegisterSpace(width)->getSyscallWrittenRegisters());
        }

        // We have a special case for used registers. If a register
        // was defined by an earlier instruction _in this block_,
        // and used now, we _don't_ add the use to the summary. 
        // This is because the summary represents conditions at
        // the "top" of the block. 

        // If something is read, then it has been used.
        use |= (read & ~def);
        // And if written, then was defined
        def |= written;

        liveness_printf("%s[%d] After instruction at address 0x%lx:\n", FILE__, __LINE__, *ii);
        liveness_cerr << read << endl << written << endl << use << endl << def << endl;
        
        read.reset();
        written.reset();
        ++ii;
    }

    liveness_printf("%s[%d] Liveness summary for block:\n", FILE__, __LINE__);
    liveness_cerr << in << endl << def << endl << use << endl;
    liveness_printf("%s[%d] --------------------\n---------------------\n", FILE__, __LINE__);

    stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
    return;
}

/* This is used to do fixed point iteration until 
   the in and out don't change anymore */
bool image_basicBlock::updateBlockLivenessInfo() 
{
  bool change = false;

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

  // old_IN = IN(X)
  bitArray oldIn = in;
  // tmp is an accumulator
  bitArray out = getLivenessOut();
  
  // Liveness is a reverse dataflow algorithm
 
  // OUT(X) = UNION(IN(Y)) for all successors Y of X

  // IN(X) = USE(X) + (OUT(X) - DEF(X))
  in = use | (out - def);
  
  // if (old_IN != IN(X)) then change = true
  if (in != oldIn)
      change = true;
      
  liveness_printf("%s[%d] Step: block 0x%llx, hasChanged %d\n", FILE__, __LINE__, firstInsnOffset(), change);
  liveness_cerr << in << endl;

  stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

  return change;
}

// Calculate basic block summaries of liveness information
// TODO: move this to an image_func level. 

void image_func::calcBlockLevelLiveness() {
    if (livenessCalculated_) return;

    // Make sure we have parsed...
    blocks();

    // Step 1: gather the block summaries
    for (unsigned i = 0; i < blockList.size(); i++) {
        blockList[i]->summarizeBlockLivenessInfo();
    }
    
    // We now have block-level summaries of gen/kill info
    // within the block. Propagate this via standard fixpoint
    // calculation
    bool changed = true;
    while (changed) {
        changed = false;
        for (unsigned i = 0; i < blockList.size(); i++) {
            if (blockList[i]->updateBlockLivenessInfo()) {
                changed = true;
            }
        }
    }

    livenessCalculated_ = true;
}

// This function does two things.
// First, it does a backwards iteration over instructions in its
// block to calculate its liveness.
// At the same time, we cache liveness (which was calculated) in
// any instPoints we cover. Since an iP only exists if the user
// asked for it, we take its existence to indicate that they'll
// also be instrumenting. 
void instPoint::calcLiveness() {
    // Assume that the presence of information means we
    // did this already.
    if (postLiveRegisters_.size()) {
        return;
    }
    // First, ensure that the block liveness is done.
    func()->ifunc()->calcBlockLevelLiveness();

    // We know: 
    //    liveness in at the block level:
    const bitArray &block_in = block()->llb()->getLivenessIn();
    //    liveness _out_ at the block level:
    const bitArray &block_out = block()->llb()->getLivenessOut();

    postLiveRegisters_ = block_out;

    assert(postLiveRegisters_.size());

    // We now want to do liveness analysis for straight-line code. 
        
    stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

    // We iterate backwards over instructions in the block. 

    InstrucIter ii(const_cast<image_basicBlock *>(block()->llb()));
    // set to the last instruction in the block; setCurrentAddress handles the x86
    // ii's inability to be a random-access iterator
    ii.setCurrentAddress(block()->llb()->lastInsnOffset());

    bitArray read(block_out.size());
    bitArray written(block_out.size());
    
    liveness_printf("%s[%d] instPoint calcLiveness: %d, 0x%lx, 0x%lx\n", 
                    FILE__, __LINE__, ii.hasPrev(), *ii, addr());

    while(ii.hasPrev() && (*ii > addr())) {

        // Cache it in the instPoint we just covered (if such exists)
        instPoint *possiblePoint = func()->findInstPByAddr(*ii);
        if (possiblePoint) {
            if (possiblePoint->postLiveRegisters_.size() == 0) {
                possiblePoint->postLiveRegisters_ = postLiveRegisters_;
            }
        }

        std::set<Register> tmpRead;
        std::set<Register> tmpWritten;
        ii.getAllRegistersUsedAndDefined(tmpRead, tmpWritten);

        for (std::set<Register>::const_iterator i = tmpRead.begin(); 
             i != tmpRead.end(); i++) {
            read[*i] = true;
        }
        for (std::set<Register>::const_iterator i = tmpWritten.begin(); 
             i != tmpWritten.end(); i++) {
            written[*i] = true;
        }

        // TODO "If trusting the ABI..."
        // Otherwise we should go interprocedural
        if (ii.isACallInstruction()) {
            read |= (registerSpace::getRegisterSpace(proc())->getCallReadRegisters());
            written |= (registerSpace::getRegisterSpace(proc())->getCallWrittenRegisters());
        }
        if (ii.isAReturnInstruction()) {
            read |= (registerSpace::getRegisterSpace(proc())->getReturnReadRegisters());
            // Nothing written implicitly by a return
        }

        if (ii.isSyscall()) {
            read |= (registerSpace::getRegisterSpace(proc())->getSyscallReadRegisters());
            written |= (registerSpace::getRegisterSpace(proc())->getSyscallWrittenRegisters());
        }

        liveness_printf("%s[%d] Calculating liveness for iP 0x%lx, insn at 0x%lx\n",
                        FILE__, __LINE__, addr(), *ii);
        liveness_cerr << "Pre: " << postLiveRegisters_ << endl;

        postLiveRegisters_ &= (~written);
        postLiveRegisters_ |= read;
        liveness_cerr << "Post: " << postLiveRegisters_ << endl;

        written.reset();
        read.reset();
        --ii;
    }

    stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

    assert(postLiveRegisters_.size());

    return;
}

const int *instPoint::liveRegisterArray() {
    calcLiveness();

    if (postLiveRegisters_.size() == 0) return NULL;

    registerSpace *rs = registerSpace::getRegisterSpace(proc());

    // Only do GPRs. 
    unsigned size = rs->numGPRs();

    int *liveRegs = new int[size];

    for (unsigned i = 0; i < size; i++) {
        int regNum = rs->GPRs()[i]->number;
        assert(regNum < size);
        if (postLiveRegisters_[i])
            liveRegs[regNum] = 1;
        else
            liveRegs[regNum] = 0;
    }
    return liveRegs;
}


// It'd be nice to do the calcLiveness here, but it's defined as const...
bitArray instPoint::liveRegisters(callWhen when) {
    // postLiveRegisters_ is our _output_ liveness. If the 
    // instrumentation is pre-point, we need to update it with
    // the effects of this instruction.

    // Override: if we have an unparsed jump table in the _function_,
    // return "everything could be live".
    if (func()->ifunc()->instLevel() == HAS_BR_INDIR ||
        func()->ifunc()->instLevel() == UNINSTRUMENTABLE) {
        bitArray allOn(registerSpace::getBitArray().size());
        allOn.set();
        return allOn;
    }
        
        
    bool debug = false;

    calcLiveness();

    if ((when == callPostInsn) ||
        (when == callBranchTargetInsn)) {
        return postLiveRegisters_;
    }
    assert(when == callPreInsn);

    // We need to do one more step.
    // Get the current instruction iterator.
    InstrucIter ii(block());
    ii.setCurrentAddress(addr());

    bitArray read(postLiveRegisters_.size());
    bitArray written(postLiveRegisters_.size());
    bitArray ret(postLiveRegisters_);

    std::set<Register> tmpRead;
    std::set<Register> tmpWritten;
    ii.getAllRegistersUsedAndDefined(tmpRead, tmpWritten);

    for (std::set<Register>::const_iterator i = tmpRead.begin(); 
         i != tmpRead.end(); i++) {
        read[*i] = true;
    }
    for (std::set<Register>::const_iterator i = tmpWritten.begin(); 
         i != tmpWritten.end(); i++) {
        written[*i] = true;
    }


    // TODO "If trusting the ABI..."
    // Otherwise we should go interprocedural
    if (ii.isACallInstruction()) {
        read |= (registerSpace::getRegisterSpace(proc())->getCallReadRegisters());
        written |= (registerSpace::getRegisterSpace(proc())->getCallWrittenRegisters());
    }
    if (ii.isAReturnInstruction()) {
        read |= (registerSpace::getRegisterSpace(proc())->getReturnReadRegisters());
        // Nothing written implicitly by a return
    }

    ret &= (~written);
    ret |= read;

    if (debug) {
        fprintf(stderr, "Liveness out for instruction at 0x%lx\n",
                addr());
        cerr << "        " << "?RNDITCPAZSOF11111100DSBSBDCA" << endl;
        cerr << "        " << "?FTFFFFFFFFFP54321098IIPPXXXX" << endl;
        cerr << "Read    " << read << endl;
        cerr << "Written " << written << endl;
        cerr << "Live    " << ret << endl;
    }

    return ret;
}

#endif

//// OLD VERSIONS FOR REFERENCE

#if 0
/* Iterates over instructions in the basic block to 
   create the initial gen kill sets for that block */
bool int_basicBlock::initRegisterGenKill() 
{  
  in = new bitArray;
  in->bitarray_init(maxGPR,in);  

  out = new bitArray;
  out->bitarray_init(maxGPR,out);  

  gen = new bitArray;
  gen->bitarray_init(maxGPR,gen);  
  
  kill = new bitArray;
  kill->bitarray_init(maxGPR,kill);  

  inFP = new bitArray;
  inFP->bitarray_init(maxFPR,inFP);  

  outFP = new bitArray;
  outFP->bitarray_init(maxFPR,outFP);  

  genFP = new bitArray;
  genFP->bitarray_init(maxFPR,genFP);  
  
  killFP = new bitArray;
  killFP->bitarray_init(maxFPR,killFP);  

  
  InstrucIter ii(this);
  
  
  while(ii.hasMore()) {
    /* GPR Gens */
    if (ii.isA_RT_ReadInstruction()){
      if (!kill->bitarray_check(ii.getRTValue(),kill))
	gen->bitarray_set(ii.getRTValue(),gen);
    }
    if (ii.isA_RA_ReadInstruction()){
      if (!kill->bitarray_check(ii.getRAValue(),kill))
	gen->bitarray_set(ii.getRAValue(),gen);
    }
    if (ii.isA_RB_ReadInstruction()){
      if (!kill->bitarray_check(ii.getRBValue(),kill))
	gen->bitarray_set(ii.getRBValue(),gen);
    }
    if (ii.isA_MRT_ReadInstruction()) {
       /* Assume worst case scenario */
       for (int i = ii.getRTValue(); i < 32; i++) {
          if (!kill->bitarray_check(i,kill))
             gen->bitarray_set(i,gen);
       }
    }
    
    /* FPR Gens */
    if (ii.isA_FRT_ReadInstruction()){
      if (!killFP->bitarray_check(ii.getFRTValue(),killFP))
	genFP->bitarray_set(ii.getFRTValue(),genFP);
    }
    if (ii.isA_FRA_ReadInstruction()){
      if (!killFP->bitarray_check(ii.getFRAValue(),killFP))
	genFP->bitarray_set(ii.getFRAValue(),genFP);
    }
    if (ii.isA_FRB_ReadInstruction()){
      if (!killFP->bitarray_check(ii.getFRBValue(),killFP))
	genFP->bitarray_set(ii.getFRBValue(),genFP);
    }
    if (ii.isA_FRC_ReadInstruction()){
      if (!killFP->bitarray_check(ii.getFRCValue(),killFP))
	genFP->bitarray_set(ii.getFRCValue(),genFP);
    }

    /* GPR Kills */
    if (ii.isA_RT_WriteInstruction()){
      kill->bitarray_set(ii.getRTValue(),kill);
    }    
    if (ii.isA_RA_WriteInstruction()){
      kill->bitarray_set(ii.getRAValue(),kill);
    }
    if (ii.isA_MRT_WriteInstruction()) {
       /* Assume worst case scenario */
       for (int i = ii.getRTValue(); i < 32; i++) {
          kill->bitarray_set(i,kill);
       }
    }

    /* FPR Kills */
    if (ii.isA_FRT_WriteInstruction()){
      killFP->bitarray_set(ii.getFRTValue(),killFP);
    }    
    if (ii.isA_FRA_WriteInstruction()){
      killFP->bitarray_set(ii.getFRAValue(),killFP);
    }

    if (ii.isAReturnInstruction()){
      /* Need to gen the possible regurn arguments */
      gen->bitarray_set(3,gen);
      gen->bitarray_set(4,gen);
      genFP->bitarray_set(1,genFP);
      genFP->bitarray_set(2,genFP);
    }
    if (ii.isAJumpInstruction()) {
       Address branchAddress = ii.getBranchTargetAddress();
       // Tail call optimization may mask a return

       codeRange *range = func_->ifunc();

       if (range) {
          if (!(range->get_address() <= branchAddress &&
                branchAddress < (range->get_address() + range->get_size()))) {
             gen->bitarray_set(3,gen);
             gen->bitarray_set(4,gen);
             genFP->bitarray_set(1,genFP);
             genFP->bitarray_set(2,genFP);
             
          }
       } else {
          gen->bitarray_set(3,gen);
          gen->bitarray_set(4,gen);
          genFP->bitarray_set(1,genFP);
          genFP->bitarray_set(2,genFP);
       }
    }

    if (ii.isADynamicCallInstruction())
      {
	 for (int a = 3; a <= 10; a++)
	   gen->bitarray_set(a,gen);
	 for (int a = 1; a <= 13; a++)
	   genFP->bitarray_set(a,genFP);
      }

    /* If it is a call instruction we look at which registers are used
       at the beginning of the called function, If we can't do that, we just
       gen registers 3-10 (the parameter holding volative 
       registers for power) & (1-13 for FPR)*/
    if (ii.isACallInstruction())
      {
	Address callAddress = ii.getBranchTargetAddress();
	//printf("Call Address is 0x%x \n",callAddress);
	
	//process *proc = flowGraph->getBProcess()->lowlevel_process();
	
	// process * procc = proc();
	int_function * funcc;
          
	codeRange * range = proc()->findOrigByAddr(callAddress);
	
	if (range)
	  {
	    funcc = range->is_function();
	    if (funcc)
	      {
		InstrucIter ah(funcc);
		while (ah.hasMore())
		  {
		    // GPR
		    if (ah.isA_RT_ReadInstruction()){
		      gen->bitarray_set(ah.getRTValue(),gen);
		    }
		    if (ah.isA_RA_ReadInstruction()){
		      gen->bitarray_set(ah.getRAValue(),gen);
		    }
		    if (ah.isA_RB_ReadInstruction()){
		      gen->bitarray_set(ah.getRBValue(),gen);
		    }
		    
		    // FPR
		    if (ah.isA_FRT_ReadInstruction()){
		      genFP->bitarray_set(ah.getFRTValue(),genFP);
		    }
		    if (ah.isA_FRA_ReadInstruction()){
		      genFP->bitarray_set(ah.getFRAValue(),genFP);
		    }
		    if (ah.isA_FRB_ReadInstruction()){
		      genFP->bitarray_set(ah.getFRBValue(),genFP);
		    }
		    if (ah.isA_FRC_ReadInstruction()){
		      genFP->bitarray_set(ah.getFRCValue(),genFP);
		    }
		    if (ah.isACallInstruction() || ah.isAIndirectJumpInstruction()) {
		      // Non-leaf.  Called function may
		      // use registers never used by callee,
		      // but used by this function.
		      
		      // Tail call optimization can also use the CTR
		      // register that would normally be used by an
		      // indirect jump.  Err on the safe side
		      for (int a = 3; a <= 10; a++)
			gen->bitarray_set(a,gen);
		      for (int a = 1; a <= 13; a++)
			genFP->bitarray_set(a,genFP);
		      break;
		    }
		    if (ii.isAJumpInstruction()) {
		      Address branchAddress = ii.getBranchTargetAddress();
		      // This function might not use any input registers
		      // but the tail-call optimization could
		      
		      codeRange *range = func_->ifunc();
		      
		      if (range) {
			if (!(range->get_address() <= branchAddress &&
			      branchAddress < (range->get_address() + range->get_size()))) {
			  for (int a = 3; a <= 10; a++)
			    gen->bitarray_set(a,gen);
			  for (int a = 1; a <= 13; a++)
			    genFP->bitarray_set(a,genFP);
			  break;
			}
		      } else {
			for (int a = 3; a <= 10; a++)
			  gen->bitarray_set(a,gen);
			for (int a = 1; a <= 13; a++)
			  genFP->bitarray_set(a,genFP);
			break;
		      }
		    }
		    
		    ah++;
		  } /* ah.hasMore */
	      }
	    else
	      {
		for (int a = 3; a <= 10; a++)
		  gen->bitarray_set(a,gen);
		for (int a = 1; a <= 13; a++)
		  genFP->bitarray_set(a,genFP);
	      } /* funcc */
	  }
	else /* if range */
	  {
	    for (int a = 3; a <= 10; a++)
	      gen->bitarray_set(a,gen);
	    for (int a = 1; a <= 13; a++)
	      genFP->bitarray_set(a,genFP);
	  }
      } /* if call */
    ++ii;
  } /*while ii.hasMore */
  return true;
}
#endif

#if 0
int int_basicBlock::liveSPRegistersIntoSet(instPoint *iP,
                                           unsigned long address)
{
   if (iP->hasSpecializedSPRegisters()) return 0;

   stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);

    int *liveSPReg = new int[1]; // only care about MQ for Power for now
    liveSPReg[0] = 0;
    InstrucIter ii(this);
    
    while (ii.hasMore() &&
           *ii <= address) {
        if (ii.isA_MX_Instruction()) {
            liveSPReg[0] = 1;
            break;
        }
        ++ii;
    }

    iP->actualSPRLiveSet_ = liveSPReg;
    stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);

    return 1;
}

#endif
