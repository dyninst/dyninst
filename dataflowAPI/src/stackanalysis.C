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

#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Result.h"
#include "instructionAPI/h/Instruction.h"

#include <queue>
#include <vector>

#include "stackanalysis.h"

#include "Annotatable.h"

#include "debug_dataflow.h"

#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeObject.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;

const StackAnalysis::Height StackAnalysis::Height::bottom(StackAnalysis::Height::notUnique);
const StackAnalysis::Height StackAnalysis::Height::top(StackAnalysis::Height::uninitialized);

const StackAnalysis::InsnTransferFunc StackAnalysis::InsnTransferFunc::bottom(StackAnalysis::InsnTransferFunc::notUnique, false);
const StackAnalysis::InsnTransferFunc StackAnalysis::InsnTransferFunc::top(StackAnalysis::InsnTransferFunc::uninitialized, false);

const StackAnalysis::BlockTransferFunc StackAnalysis::BlockTransferFunc::bottom(StackAnalysis::BlockTransferFunc::notUnique, false, false);
const StackAnalysis::BlockTransferFunc StackAnalysis::BlockTransferFunc::top(StackAnalysis::BlockTransferFunc::uninitialized, false, false);

const StackAnalysis::Range StackAnalysis::defaultRange(std::make_pair(0, 0), 0, 0);
const StackAnalysis::Range::range_t StackAnalysis::Range::infinite(std::make_pair(MINLONG, MAXLONG));

AnnotationClass <StackAnalysis::HeightTree> SP_Anno(std::string("SP_Anno"));
AnnotationClass <StackAnalysis::HeightTree> FP_Anno(std::string("FP_Anno"));


//
//
// Concepts:
//
// There are three terms we throw around:
// 
// Stack height: the size of the stack; (cur_stack_ptr - func_start_stack_ptr)
// Stack delta: the difference in the size of the stack over the execution of
//   a region of code (basic block here). (block_end_stack_ptr - block_start_stack_ptr)
// Stack clean: the amount the callee function shifts the stack. This is an x86 idiom.
//   On x86 the caller pushes arguments onto the stack (and thus shifts the stack).
//   Normally the caller also cleans the stack (that is, removes arguments). However, 
//   some callee functions perform this cleaning themselves. We need to account for this
//   in our analysis, which otherwise assumes that callee functions don't alter the stack.
// 

bool StackAnalysis::analyze()
{
  df_init_debug();
  stackanalysis_printf("Beginning stack analysis for function %s\n",
		       func->name().c_str());

    stackanalysis_printf("\tSummarizing block effects\n");
    summarizeBlocks();
    
    stackanalysis_printf("\tPerforming fixpoint analysis\n");
    sp_fixpoint();

    stackanalysis_printf("\tCreating SP interval tree\n");
    sp_createIntervals();

    stackanalysis_printf("\tCreating FP interval tree\n");
    fp_fixpoint();

    fp_createIntervals();
    
    func->addAnnotation(sp_intervals_, SP_Anno);
    func->addAnnotation(fp_intervals_, FP_Anno);
    if (df_debug_stackanalysis) {
        debug();
    }

    stackanalysis_printf("Finished stack analysis for function %s\n",
			 func->name().c_str());

    return true;
}

// We want to create a transfer function for the block as a whole. 
// This will allow us to perform our fixpoint calculation over
// blocks (thus, O(B^2)) rather than instructions (thus, O(I^2)). 
//
// Handling the stack height is straightforward. We also accumulate
// region changes in terms of a stack of Region objects.

typedef std::vector<std::pair<Instruction::Ptr, Offset> > InsnVec;

static void getInsnInstances(Block *block,
		      InsnVec &insns) {
  Offset off = block->start();
  const unsigned char *ptr = (const unsigned char *)block->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
  while (off < block->end()) {
    insns.push_back(std::make_pair(d.decode(), off));
    off += insns.back().first->size();
  }
}

void StackAnalysis::summarizeBlocks() {
    // STACK HEIGHT:
    // Foreach block B:
    //   Let E = block effect
    //   Foreach (insn i, offset o) in B:
    //     Let T = transfer function representing i;
    //     E = T(E);
    //     blockToInsnFuncs[B][o] = T;
    //   blockEffects[B] = E;

    // STACK PRESENCE:
    // Foreach block B:
    //   Let d = uninitialized;
    //   Foreach (insn i, offset o) in B:
    //     d = change in stack presence at d;
    //     blockToInsnDeltas[B][o] = d;
    //   blockHeightDeltas[B] = d;

  Function::blocklist & bs = func->blocks();
  Function::blocklist::iterator bit = bs.begin();
  for( ; bit != bs.end(); ++bit) {
    Block *block = *bit;
  
    // Accumulators. They have the following behavior:
    // 
    // New region: add to the end of the regions list
    // Offset to stack pointer: accumulate to delta.
    // Setting the stack pointer: zero delta, set set_value.
    // 
    
    BlockTransferFunc &bFunc = sp_blockEffects[block];
    
    bFunc = BlockTransferFunc::top;
    
    stackanalysis_printf("\t Block starting at 0x%lx: %s\n", 
			 block->start(),
			 bFunc.format().c_str());
    
    InsnVec instances;
    getInsnInstances(block, instances);
    
    for (unsigned j = 0; j < instances.size(); j++) {
      InstructionAPI::Instruction::Ptr insn = instances[j].first;
      Offset &off = instances[j].second;
      Offset next;
      if (j < (instances.size() - 1)) {
	next = instances[j+1].second;
      }
      else {
	next = block->end();
      }
      
      InsnTransferFunc iFunc;
      fp_State fpCopied = fp_noChange;
      
      computeInsnEffects(block, insn, off, 
			 iFunc, fpCopied);
      
      if (iFunc != InsnTransferFunc::top) {
	iFunc.apply(bFunc);
	sp_blockToInsnFuncs[block][next] = iFunc;
      }
      if (fpCopied != fp_noChange) {
	fp_changePoints[block].push_back(std::make_pair<fp_State, Offset>(fpCopied, off + insn->size()));
      }
      
      stackanalysis_printf("\t\t\t At 0x%lx:  %s\n",
			   off,
			   bFunc.format().c_str());
    }
    stackanalysis_printf("\t Block summary for 0x%lx: %s\n", block->start(), bFunc.format().c_str());
  }
}


void StackAnalysis::sp_fixpoint() {
  std::queue<Block *> worklist;
  
  Intraproc epred; // ignore calls, returns in edge iteration
  NoSinkPredicate epred2(&epred); // ignore sink node (unresolvable)

  worklist.push(func->entry());

  Block *entry = func->entry();
  
  while (!worklist.empty()) {
    Block *block = worklist.front();
    stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n", block->start());
    
    worklist.pop();
    
    // Step 1: calculate the meet over the heights of all incoming
    // intraprocedural blocks.
    
    std::set<BlockTransferFunc> inEffects;
    
    if (block == entry) {
      // FIXME for POWER/non-IA32
      // IA32 - the in height includes the return address and therefore
      // is <wordsize>
      // POWER - the in height is 0
      
#if defined(arch_power)
      inEffects.insert(BlockTransferFunc(0, true, true));
#elif (defined(arch_x86) || defined(arch_x86_64))
      int word_size = func->isrc()->getAddressWidth();
      inEffects.insert(BlockTransferFunc(-1*word_size,
					 true, true));
#else
      assert(0 && "Unimplemented architecture");
#endif
      
      stackanalysis_printf("\t Primed initial block\n");
    }
    else {
      
      Block::edgelist & inEdges = block->sources();
      Block::edgelist::iterator eit = inEdges.begin(&epred2);
      for( ; eit != inEdges.end(); ++eit) {
	Edge *edge = (Edge*)*eit;
	//if (edge->type() == CALL || edge->type() == RET) continue;
	inEffects.insert(sp_outBlockEffects[edge->src()]);
	stackanalysis_printf("\t\t Inserting 0x%lx: %s\n", 
			     edge->src()->start(),
			     sp_outBlockEffects[edge->src()].format().c_str());
      }
    }
    
    BlockTransferFunc newInEffect = meet(inEffects);
    
    stackanalysis_printf("\t New in meet:  %s\n",
			 newInEffect.format().c_str());
    
    // Step 2: see if the input has changed
    
    if (newInEffect == sp_inBlockEffects[block]) {
      // No new work here
      stackanalysis_printf("\t ... equal to current, skipping block\n");
      continue;
    }
    
    stackanalysis_printf("\t ... inequal to current %s, analyzing block\n",
			 sp_inBlockEffects[block].format().c_str());
    
    sp_inBlockEffects[block] = newInEffect;
    
    // Step 3: calculate our new outs
    
    sp_blockEffects[block].apply(newInEffect, 
				 sp_outBlockEffects[block]);
    
    stackanalysis_printf("\t ... output from block: %s\n",
			 sp_outBlockEffects[block].format().c_str());
    
    // Step 4: push all children on the worklist.
    
    Block::edgelist & outEdges = block->targets();
    Block::edgelist::iterator eit = outEdges.begin(&epred2);
    for( ; eit != outEdges.end(); ++eit) {
      //if ((*eit)->type() == CALL) continue;
      worklist.push((Block*)(*eit)->trg());
    }
  }
}

void StackAnalysis::sp_createIntervals() {
  // We now have a summary of the state of the stack at
  // the entry to each block. We need to push that information
  // into the block. Assume that frame affecting instructions
  // are rare, where i_m, i_n affect the stack (m < n). Then
  // each interval runs from [i_m, i_n) or [i_m, i_{n-1}]. 
  
  sp_intervals_ = new HeightTree();
  
  Function::blocklist & bs = func->blocks();
  Function::blocklist::iterator bit = bs.begin();
  for( ; bit != bs.end(); ++bit) {
    Block *block = *bit;
    
    stackanalysis_printf("\t Interval creation (H): visiting block at 0x%lx\n", block->start());
    
    Offset curLB = block->start();
    Offset curUB = 0;
    BlockTransferFunc curHeight = sp_inBlockEffects[block];
    
    stackanalysis_printf("\t\t Block starting state: 0x%lx, %s\n", 
			 curLB, curHeight.format().c_str());
    
    // We only cache points where the frame height changes. 
    // Therefore, we can simply iterate through them. 
    for (InsnFuncs::iterator iter = sp_blockToInsnFuncs[block].begin();
	 iter != sp_blockToInsnFuncs[block].end(); iter++) {
      
      // We've changed the state of the stack pointer. End this interval
      // and start a new one. 
      
      curUB = iter->first;
      
      
      sp_intervals_->insert(curLB, curUB, 
			    Height(curHeight.delta(),
				   getRegion(curHeight.ranges())));
      
      curLB = curUB;
      // Adjust height
      iter->second.apply(curHeight);
      
      stackanalysis_printf("\t\t Block continuing state: 0x%lx, %s\n", 
			   curLB, curHeight.format().c_str());
    }
    
    if (curLB != block->end()) {
      // Cap off the extent for the current block
      curUB = block->end();
      if (curHeight != sp_outBlockEffects[block]) {
	fprintf(stderr, "ERROR: accumulated state not equal to fixpoint state!\n");
	fprintf(stderr, "\t %s\n", curHeight.format().c_str());
	fprintf(stderr, "\t %s\n", sp_outBlockEffects[block].format().c_str());
      }
      
      assert(curHeight == sp_outBlockEffects[block]);
      
      sp_intervals_->insert(curLB, curUB, 
			    Height(curHeight.delta(),
				   getRegion(curHeight.ranges())));
    }
  }
}

// Now determine where the FP is set and what value
// it is set to. This should take a single pass through
// to perform. 

void StackAnalysis::fp_fixpoint() {

  std::queue<Block *> worklist;

  Intraproc epred; // ignore calls, returns
  NoSinkPredicate epred2(&epred);

  Block *entry = func->entry();
  worklist.push(entry);

  while (!worklist.empty()) {
    Block *block = worklist.front();
    stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n", block->start());
      
    worklist.pop();
      
    // Step 1: calculate the meet over the heights of all incoming
    // intraprocedural blocks.
      
    std::set<Height> inHeights;
      
    if (block == entry) {
      // The in height is 0
      // The set height is... 0
      // And there is no input region.
      inHeights.insert(Height::bottom);
      stackanalysis_printf("\t Primed initial block\n");
    }
    else {
        Block::edgelist & inEdges = block->sources();
        Block::edgelist::iterator eit = inEdges.begin(&epred2);
        for( ; eit != inEdges.end(); ++eit) {
	        Edge *edge = (Edge*)*eit;
	        //if (edge->type() == CALL) continue;
	        inHeights.insert(fp_outBlockHeights[edge->src()]);
	        stackanalysis_printf("\t\t Inserting 0x%lx: %s\n", 
			     edge->src()->start(),
			     fp_outBlockHeights[edge->src()].format().c_str());
        }
    }
    
    Height newInHeight = meet(inHeights);
    
    stackanalysis_printf("\t New in meet:  %s\n",
			 newInHeight.format().c_str());
    
    // Step 2: see if the input has changed
      
    if (newInHeight == fp_inBlockHeights[block]) {
      // No new work here
      stackanalysis_printf("\t ... equal to current, skipping block\n");
      continue;
    }
      
    stackanalysis_printf("\t ... inequal to current %s, analyzing block\n",
			 fp_inBlockHeights[block].format().c_str());
      
    fp_inBlockHeights[block] = newInHeight;
      
    // Step 3: calculate our new outs
      
    if (!fp_changePoints[block].empty()) {
      switch (fp_changePoints[block].back().first) {
      case fp_noChange:
	fp_outBlockHeights[block] = newInHeight;
	break;
      case fp_created: {
	// We set the FP to the last thing in the block
	Height sp = findSP(fp_changePoints[block].back().second);
	fp_outBlockHeights[block] = sp;
	break;
      }
      case fp_destroyed:
	fp_outBlockHeights[block] = Height::bottom;
	break;
      }
    }
    else {
      // Copy through
      fp_outBlockHeights[block] = newInHeight;
    }

    stackanalysis_printf("\t ... output from block: %s\n",
			 fp_outBlockHeights[block].format().c_str());
      
    // Step 4: push all children on the worklist.
     
    Block::edgelist & outEdges = block->targets();
    Block::edgelist::iterator eit = outEdges.begin(&epred2);
    for( ; eit != outEdges.end(); ++eit) { 
      //if ((*eit)->type() == CALL) continue;
      worklist.push((Block*)(*eit)->trg());
    }
  }
}

void StackAnalysis::fp_createIntervals() {
  // We now have a summary of the state of the stack at
  // the entry to each block. We need to push that information
  // into the block. Assume that frame affecting instructions
  // are rare, where i_m, i_n affect the stack (m < n). Then
  // each interval runs from [i_m, i_n) or [i_m, i_{n-1}]. 

  fp_intervals_ = new HeightTree();

  Function::blocklist & bs = func->blocks();
  Function::blocklist::iterator bit = bs.begin();
  for( ; bit != bs.end(); ++bit) {
    Block *block = *bit;

    stackanalysis_printf("\t Interval creation (H): visiting block at 0x%lx\n", 
			 block->start());
    
    Offset curLB = block->start();
    Offset curUB = 0;
    Height curHeight = fp_inBlockHeights[block];
    
    stackanalysis_printf("\t\t Block starting state: 0x%lx, %s\n", 
			 curLB, curHeight.format().c_str());
    
    // We only cache points where the frame height changes. 
    // Therefore, we can simply iterate through them. 
    for (FPChangePoints::iterator iter2 = fp_changePoints[block].begin();
	 iter2 != fp_changePoints[block].end(); ++iter2) {

      // We've changed the state of the stack pointer. End this interval
      // and start a new one. 
           
      curUB = iter2->second;

      fp_intervals_->insert(curLB, curUB, 
			    curHeight);

      stackanalysis_printf("\t\t\t Inserting interval: 0x%lx, 0x%lx, %s\n",
			   curLB, curUB, curHeight.format().c_str());

      switch(iter2->first) {
      case fp_noChange:
	// ????
	continue;
      case fp_created: 
	curHeight = findSP(curUB);
	break;
      case fp_destroyed:
	curHeight = Height::bottom;
	break;
      }
      
      curLB = curUB;
    }
    
    if (curLB != block->end()) {
      // Cap off the extent for the current block
      curUB = block->end();
      if (curHeight != fp_outBlockHeights[block]) {
	fprintf(stderr, "ERROR: accumulated state not equal to fixpoint state!\n");
	fprintf(stderr, "\t %s\n", curHeight.format().c_str());
	fprintf(stderr, "\t %s\n", fp_outBlockHeights[block].format().c_str());
      }


      stackanalysis_printf("\t\t\t Inserting interval: 0x%lx, 0x%lx, %s\n",
			   curLB, curUB, curHeight.format().c_str());
      fp_intervals_->insert(curLB, curUB, 
			    curHeight);
    }
  }
}

void StackAnalysis::computeInsnEffects(Block *block,
                                       const Instruction::Ptr &insn,
                                       Offset off,
                                       InsnTransferFunc &iFunc,
				       fp_State &fpState) 
{
    stackanalysis_printf("\t\tInsn at 0x%lx\n", off);
    static Expression::Ptr theStackPtr(new RegisterAST(MachRegister::getStackPointer(func->isrc()->getArch())));
    static Expression::Ptr theFramePtr(new RegisterAST(MachRegister::getFramePointer(func->isrc()->getArch())));
    
    //TODO: Integrate entire test into analysis lattice
    entryID what = insn->getOperation().getID();

    if (insn->isWritten(theFramePtr)) {
      stackanalysis_printf("\t\t\t FP written\n");
      if (what == e_mov &&
          (insn->isRead(theStackPtr))) {
	fpState = fp_created;
	stackanalysis_printf("\t\t\t Frame created\n");
      }
      else {
	fpState = fp_destroyed;
      }
    }

    int word_size = func->isrc()->getAddressWidth();
    if (insn->getControlFlowTarget() && insn->getCategory() == c_CallInsn) {
      if (off != block->lastInsnAddr()) {
	// Call in the middle of the block? Must be a get PC operation
	iFunc.delta() = -1*word_size;
	stackanalysis_printf("\t\t\t getPC call: %s\n", iFunc.format().c_str());
	return;
      }
        
        Block::edgelist & outs = block->targets();  
        Block::edgelist::iterator eit = outs.begin();
        for( ; eit != outs.end(); ++eit) {
            Edge *cur_edge = (Edge*)*eit;

	        if (cur_edge->type() == DIRECT) {
	            // For some reason we're treating this
	            // call as a branch. So it shifts the stack
	            // like a push (heh) and then we're done.
	            iFunc.delta() = -1*word_size;
	            stackanalysis_printf("\t\t\t Stack height changed by simulate-jump call\n");
	            return;
	        }

            if (cur_edge->type() != CALL) 
                continue;
            
            Block *target_bbl = cur_edge->trg();
	    Function *target_func = target_bbl->obj()->findFuncByEntry(target_bbl->region(), target_bbl->start());

            if (!target_func)
                continue;
            Height h = getStackCleanAmount(target_func);
            if (h == Height::bottom) {
                iFunc == InsnTransferFunc::bottom;
            }
            else {
                iFunc.delta() = h.height();
            }

            stackanalysis_printf("\t\t\t Stack height changed by self-cleaning function: %s\n", iFunc.format().c_str());
            return;
        }
        stackanalysis_printf("\t\t\t Stack height assumed unchanged by call\n");
        return;
    }
    
    if(!insn->isWritten(theStackPtr)) {
      return;
    }

    // Here's one for you:
    // lea    0xfffffff4(%ebp),%esp
    // Yeah, that's "do math off the base pointer and assign the stack pointer". 
    // Oy. 
    // Doesn't work right, so leaving it out for now. 
#if 0
    if (insn->isRead(theFramePtr) || insn->isRead(framePtr64)) {
      
      bool done;
      std::vector<Operand> operands;
      insn->getOperands(operands);
      cerr << "************" << endl;
      for (unsigned i = 0; i < operands.size(); ++i) {
	cerr << operands[i].format() << endl;
	// We need to find the thing that be done read
	if (operands[i].isRead(operands[i].getValue())) {
	  if (done) {
	    // Multiple conflicting uses. WTF?
	    iFunc.range() = Range(Range::infinite, 0, off);
	    return;
	  }
	  done = true;
	  static Expression::Ptr theFramePtr(new RegisterAST(MachRegister::getFramePointer(Arch_x86)));
	  static Expression::Ptr theFramePtr64(new RegisterAST(MachRegister::getFramePointer(Arch_x86_64)));
	  operands[i].getValue()->bind(theFramePtr.get(), Result(s32, -2*word_size));
	  operands[i].getValue()->bind(theFramePtr64.get(), Result(s32, -2*word_size));

	  Result res = operands[i].getValue()->eval();
	  if (res.defined) {
	    iFunc.abs() = true;
	    iFunc.delta() = res.convert<long>();
            stackanalysis_printf("\t\t\t Stack height changed by ref off FP %s: %s\n", insn->format().c_str(), iFunc.format().c_str());
	  }
	  else {
	    iFunc.range() = Range(Range::infinite, 0, off);
            stackanalysis_printf("\t\t\t Stack height changed by unevalled ref off FP %s: %s\n", insn->format().c_str(), iFunc.format().c_str());
	  }
	}
      }
    }

#endif
    int sign = 1;
    switch(what) {
    case e_push:
        sign = -1;
    case e_pop: {
        Operand arg = insn->getOperand(0);
        if (arg.getValue()->eval().defined) {
            iFunc.delta() = sign * word_size;
            stackanalysis_printf("\t\t\t Stack height changed by evaluated push/pop: %s\n", iFunc.format().c_str());
            return;
        }
        iFunc.delta() = sign * arg.getValue()->size();
        stackanalysis_printf("\t\t\t Stack height changed by unevalled push/pop: %s\n", iFunc.format().c_str());
        return;
    }
    case e_ret_near:
    case e_ret_far: {
      std::vector<Operand> operands;
      insn->getOperands(operands);
      if (operands.size() == 0) {
	iFunc.delta() = word_size;
      }
      else if (operands.size() == 1) {
	// Ret immediate
	Result imm = operands[0].getValue()->eval();
	if (imm.defined) {
	  iFunc.delta() = word_size + imm.convert<int>();
	}
	else {
	  iFunc.range() = Range(Range::infinite, 0, off);
	}
      }
      stackanalysis_printf("\t\t\t Stack height changed by return: %s\n", iFunc.format().c_str());
      
      return;
    }
    case e_sub:
        sign = -1;
    case e_add:
      {
        // Add/subtract are op0 += (or -=) op1
        Operand arg = insn->getOperand(1);
        Result delta = arg.getValue()->eval();
        if(delta.defined) {
	  iFunc.delta() = sign * delta.convert<long>();
	  stackanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %s\n", iFunc.format().c_str());
	  return;
        }
        iFunc.range() = Range(Range::infinite, 0, off);
        stackanalysis_printf("\t\t\t Stack height changed by unevalled add/sub: %s\n", iFunc.format().c_str());
        return;
    }
        // We treat return as zero-modification right now
    case e_leave:
        iFunc.abs() = true;
        iFunc.delta() = -1*word_size;
        stackanalysis_printf("\t\t\t Stack height reset by leave: %s\n", iFunc.format().c_str());
        return;
    case e_pushfd:
        sign = -1;
    case e_popfd:
        iFunc.delta() = sign * word_size;
        stackanalysis_printf("\t\t\t Stack height changed by flag push/pop: %s\n", iFunc.format().c_str());
        return;
    case power_op_addi:
    case power_op_addic:
        {
        // Add/subtract are op0 = op1 +/- op2; we'd better read the stack pointer as well as writing it
        Operand arg = insn->getOperand(2);
        Result delta = arg.getValue()->eval();
        if(delta.defined && insn->isRead(theStackPtr)) {
	    iFunc.delta() = sign * delta.convert<long>();
	    stackanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %s\n", iFunc.format().c_str());
	    return;
        }
        iFunc.range() = Range(Range::infinite, 0, off);
        stackanalysis_printf("\t\t\t Stack height changed by unevalled add/sub: %s\n", iFunc.format().c_str());
        return;
    }
    case power_op_stwu: {
        std::set<Expression::Ptr> memWriteAddrs;
        insn->getMemoryWriteOperands(memWriteAddrs);
	Expression::Ptr stackWrite = *(memWriteAddrs.begin());
        stackanalysis_printf("\t\t\t ...checking operand %s\n", stackWrite->format().c_str());
        stackanalysis_printf("\t\t\t ...binding %s to 0\n", theStackPtr->format().c_str());
        stackWrite->bind(theStackPtr.get(), Result(u32, 0));
        Result delta = stackWrite->eval();
        if(delta.defined) {
            iFunc.delta() = delta.convert<long>();
            stackanalysis_printf("\t\t\t Stack height changed by evalled stwu: %s\n", iFunc.format().c_str());
            return;
        }
    }
    case e_mov: {
      // Special case: mov ebp, esp is a stack teardown that we need to handle
      // correctly. Things to think about... moving the FP analysis before the SP
      // analysis? Or add an "assignment" func? For now, special-casing...
      if (insn->isRead(theFramePtr)) {
	// Awesome
	iFunc.abs() = true;
	iFunc.delta() = -2*word_size;
	stackanalysis_printf("\t\t\t Stack height changed by mov ebp, esp: %s\n", iFunc.format().c_str());
	return;
      }
      // Otherwise fall through to default
    }
    default:
        iFunc.range() = Range(Range::infinite, 0, off);
        stackanalysis_printf("\t\t\t Stack height changed by unhandled insn \"%s\": %s\n", 
			     insn->format().c_str(), iFunc.format().c_str());
        return;
    }
    assert(0);
    return;
}

StackAnalysis::Height StackAnalysis::getStackCleanAmount(Function *func) {
    // Cache previous work...
    if (funcCleanAmounts.find(func) != funcCleanAmounts.end()) {
        return funcCleanAmounts[func];
    }

    if (!func->cleansOwnStack()) {
        funcCleanAmounts[func] = 0;
        return funcCleanAmounts[func];
    }

    InstructionDecoder decoder((const unsigned char*)NULL, 0, func->isrc()->getArch());
    unsigned char *cur;

    std::set<Height> returnCleanVals;

    Function::blocklist &returnBlocks = func->returnBlocks();
    Function::blocklist::iterator rets = returnBlocks.begin();
    for (; rets != returnBlocks.end(); ++rets) {
      Block *ret = *rets;
      cur = (unsigned char *) ret->region()->getPtrToInstruction(ret->lastInsnAddr());
      Instruction::Ptr insn = decoder.decode(cur);
        
      entryID what = insn->getOperation().getID();
      if (what != e_ret_near)
	continue;
      
      int val;
      std::vector<Operand> ops;
      insn->getOperands(ops);
      if (ops.size() == 0) {
	val = 0;
      }
      else {      
	Result imm = ops[0].getValue()->eval();
	assert(imm.defined);
	val = (int) imm.val.s16val;
      }
      returnCleanVals.insert(Height(val));
    }
    
    funcCleanAmounts[func] = meet(returnCleanVals);
    return funcCleanAmounts[func];
}

StackAnalysis::StackAnalysis() :
    func(NULL), sp_intervals_(NULL), fp_intervals_(NULL), 
    rt(Region::Ptr(new Region())) {};
    

StackAnalysis::StackAnalysis(Function *f) : func(f),
					    sp_intervals_(NULL),
					    fp_intervals_(NULL),
                                            rt(Region::Ptr(new Region())) {}
void StackAnalysis::debug() {
#if 0
    if (!sp_intervals_) return;
    std::vector<std::pair<std::pair<Offset, Offset>, Height> > elements;
    sp_intervals_->elements(elements);

    for (unsigned i = 0; i < elements.size(); i++) {
        fprintf(stderr, "0x%lx - 0x%lx: %s\n",
                elements[i].first.first,
                elements[i].first.second,
                elements[i].second.format().c_str());
    }
#endif
}

std::string StackAnalysis::InsnTransferFunc::format() const {
    char buf[256];

    if (*this == bottom) {
        sprintf(buf, "<BOTTOM>");
        return buf;
    }
    if (*this == top) {
        sprintf(buf, "<TOP>");
        return buf;
    }

    if (range_ == defaultRange) {
        if (!abs_) {
	  sprintf(buf, "<%ld>", delta_);
        }
        else {
	  sprintf(buf, "Abs: %ld", delta_);
        }
    }
    else {
        sprintf(buf, "%s", range_.format().c_str());
    }
    return buf;
}

std::string StackAnalysis::BlockTransferFunc::format() const {
    if (*this == bottom) {
        return "<BOTTOM>";
    }
    if (*this == top) {
        return "<TOP>";
    }

    std::stringstream retVal;

    if (!abs_) {
        retVal << "<" << delta_ << ">";
    }
    else {
        retVal << "Abs: " << delta_;
    }

    for (unsigned i = 0; i < ranges_.size(); i++) {
        retVal << ranges_[i].format();
    }
    return retVal.str();
}

void StackAnalysis::InsnTransferFunc::apply(const BlockTransferFunc &in,
                                            BlockTransferFunc &out) const {
    out = in;

    apply(out);
}


void StackAnalysis::InsnTransferFunc::apply(BlockTransferFunc &out) const {
    if (out == BlockTransferFunc::bottom) return;

    if (*this == bottom) {
        out = BlockTransferFunc::bottom;
        return;
    }

    if (abs_) {
        out.delta() = 0;
        out.abs() = true;
        out.ranges().clear();
        out.reset() = true;
    }

    if (delta_ != uninitialized) {
        if (out.delta() == uninitialized) {
            out.delta() = 0;
        }
        out.delta() += delta_;
    }

    if (range_ != defaultRange) {
        out.ranges().push_back(Range(range_, 
                                     ((out.delta() == uninitialized ) ? 0 : out.delta())));
        out.reset() = true;
        out.delta() = 0;
    }
}


void StackAnalysis::BlockTransferFunc::apply(const BlockTransferFunc &in,
                                             BlockTransferFunc &out) const {
    out = in;
    apply(out);
}

void StackAnalysis::BlockTransferFunc::apply(BlockTransferFunc &out) const {
    if (out == bottom) return;
    if (*this == bottom) {
        out = bottom;
        return;
    }

    // abs: we encountered a leave or somesuch
    // that rebased the stack pointer
    if (abs_) {
        // Any increment will happen below.
        out.delta() = 0;
        out.ranges().clear();
        out.abs() = true;
    }

    // This just says reset delta to 0;
    // we've got a new relative origin.
    if (reset_) {
        out.delta() = 0;
        out.reset() = true;
    }

    if (delta_ != uninitialized) {
        if (out.delta() == uninitialized) {
            out.delta() = 0;
        }
        out.delta() += delta_;
    }
    
    for (unsigned i = 0; i < ranges_.size(); i++) {
        out.ranges().push_back(ranges_[i]);
    }
}

std::string StackAnalysis::Range::format() const {
    std::stringstream retVal;

    if (off_ == 0) {
        return "[NONE]";
    }
    else {
        retVal << "[" << std::hex << off_ 
               << std::dec
               << ", " << delta_
               << ", [";
        if (range_.first == MINLONG)
            retVal << "-inf";
        else
            retVal << range_.first;

        retVal << ",";

        if (range_.second == MAXLONG)
            retVal << "+inf";
        else
            retVal << range_.second;
        
        retVal << "]]";
        return retVal.str();
    }
}

StackAnalysis::Region::Ptr StackAnalysis::RangeTree::find(Ranges &str) {
    if (str.empty()) return root->region;

    Node *cur = root;
    for (unsigned i = 0; i < str.size(); i++) {
        std::map<Range, Node *>::iterator iter = cur->children.find(str[i]);
        if (iter == cur->children.end()) {
            stackanalysis_printf("\t Creating new node for range %s\n", 
				 str[i].format().c_str());
            // Need to create a new node...
            Node *newNode = new Node(Region::Ptr(new Region(getNewRegionID(),
                                                            str[i],
                                                            cur->region)));
            cur->children[str[i]] = newNode;
            cur = newNode;
        }
        else {
	  stackanalysis_printf("\t Found existing node for range %s\n",
			       str[i].format().c_str());
            cur = iter->second;
        }
    }
    stackanalysis_printf("\t Returning region %s\n", cur->region->format().c_str());
    return cur->region;
}

StackAnalysis::Region::Ptr StackAnalysis::getRegion(Ranges &ranges) {
    return rt.find(ranges);
}

std::string StackAnalysis::Region::format() const {
    std::stringstream retVal;
    
    retVal << "(" << name_ << "," << range_.format() << ") ";
    if (prev_)
        retVal << prev_->format();

    return retVal.str();
}

StackAnalysis::Height StackAnalysis::findSP(Address addr) {
  Height ret; // Defaults to "top"

  if (func == NULL) return ret;

  if (!sp_intervals_) {
    // Check annotation
    func->getAnnotation(sp_intervals_, SP_Anno);
  }
  if (!sp_intervals_) {
    // Analyze?
    if (!analyze()) return Height();
  }
  assert(sp_intervals_);

  sp_intervals_->find(addr, ret);
  if(ret.isTop()) {
    if(!analyze()) return Height();
  }
  sp_intervals_->find(addr, ret);
  assert(!ret.isTop());
  return ret;
}

StackAnalysis::Height StackAnalysis::findFP(Address addr) {
  Height ret; // Defaults to "top"

  if (func == NULL) return ret;

  if (!fp_intervals_) {
    // Check annotation
    func->getAnnotation(fp_intervals_, FP_Anno);
  }
  if (!fp_intervals_) {
    // Analyze?
    if (!analyze()) return ret;
  }
  assert(fp_intervals_);

  fp_intervals_->find(addr, ret);
  return ret;
}

std::ostream &operator<<(std::ostream &os, const Dyninst::StackAnalysis::Height &h) {
  os << "STACK_SLOT[" << h.format() << "]";
  return os;
}
