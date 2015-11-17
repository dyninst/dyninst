/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include "instructionAPI/h/BinaryFunction.h"
#include "instructionAPI/h/Dereference.h"
#include "instructionAPI/h/Expression.h"
#include "instructionAPI/h/Immediate.h"
#include "instructionAPI/h/Register.h"

#include <queue>
#include <vector>
#include <boost/bind.hpp>

#include "ABI.h"
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

AnnotationClass <StackAnalysis::Intervals> Stack_Anno(std::string("Stack_Anno"));


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
    fixpoint();

    stackanalysis_printf("\tCreating SP interval tree\n");
    summarize();

    func->addAnnotation(intervals_, Stack_Anno);

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
  Function::blocklist bs(func->blocks());
  Function::blocklist::iterator bit = bs.begin();
  for( ; bit != bs.end(); ++bit) {
    Block *block = *bit;
    // Accumulators. They have the following behavior:
    // 
    // New region: add to the end of the regions list
    // Offset to stack pointer: accumulate to delta.
    // Setting the stack pointer: zero delta, set set_value.
    // 

    SummaryFunc &bFunc = blockEffects[block];

    stackanalysis_printf("\t Block starting at 0x%lx: %s\n", 
			 block->start(),
			 bFunc.format().c_str());
    InsnVec instances;
    getInsnInstances(block, instances);

    for (unsigned j = 0; j < instances.size(); j++) {
      InstructionAPI::Instruction::Ptr insn = instances[j].first;
      Offset &off = instances[j].second;

      // Fills in insnEffects[off]
      TransferFuncs &xferFuncs = insnEffects[block][off];

      computeInsnEffects(block, insn, off,
                         xferFuncs);
      bFunc.add(xferFuncs);

      stackanalysis_printf("\t\t\t At 0x%lx:  %s\n",
			               off, bFunc.format().c_str());
    }
    stackanalysis_printf("\t Block summary for 0x%lx: %s\n", block->start(), bFunc.format().c_str());
  }
}

struct intra_nosink : public ParseAPI::EdgePredicate
{
  virtual bool operator()(Edge* e)
  {
    static Intraproc i;
    static NoSinkPredicate n;
    
    return i(e) && n(e);
  }
  
};

void add_target(std::queue<Block*>& worklist, Edge* e)
{
	worklist.push(e->trg());
}

void StackAnalysis::fixpoint() {
  std::queue<Block *> worklist;
  
  //Intraproc epred; // ignore calls, returns in edge iteration
  //NoSinkPredicate nosink(); // ignore sink node (unresolvable)
  intra_nosink epred2;
  
  

  worklist.push(func->entry());

  Block *entry = func->entry();
  
  while (!worklist.empty()) {
    Block *block = worklist.front();
    stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n", block->start());
    
    worklist.pop();
    
    // Step 1: calculate the meet over the heights of all incoming
    // intraprocedural blocks.
    
    RegisterState input;
    
    if (block == entry) {
       createEntryInput(input);
       stackanalysis_printf("\t Primed initial block\n");
    }
    else {
        stackanalysis_printf("\t Calculating meet with block [%x-%x]\n", block->start(), block->lastInsnAddr());
       meetInputs(block, blockInputs[block], input);
    }
    
    stackanalysis_printf("\t New in meet: %s\n", format(input).c_str());

    // Step 2: see if the input has changed
    
    if (input == blockInputs[block]) {
       // No new work here
       stackanalysis_printf("\t ... equal to current, skipping block\n");
       continue;
    }
    
    stackanalysis_printf("\t ... inequal to current %s, analyzing block\n", format(blockInputs[block]).c_str());
    
    blockInputs[block] = input;
    
    // Step 3: calculate our new outs
    
    blockEffects[block].apply(input,
                              blockOutputs[block]);
    
    stackanalysis_printf("\t ... output from block: %s\n", format(blockOutputs[block]).c_str());
    
    // Step 4: push all children on the worklist.
    
    const Block::edgelist & outEdges = block->targets();
    std::for_each(boost::make_filter_iterator(epred2, outEdges.begin(), outEdges.end()),
		  boost::make_filter_iterator(epred2, outEdges.end(), outEdges.end()),
		  boost::bind(add_target, boost::ref(worklist), _1));    
  }
}



void StackAnalysis::summarize() {
    // Now that we know the actual inputs to each block,
    // we create intervals by replaying the effects of each
    // instruction.

    intervals_ = new Intervals();

    Function::blocklist bs = func->blocks();
    Function::blocklist::iterator bit = bs.begin();
    for( ; bit != bs.end(); ++bit) {
        Block *block = *bit;
        RegisterState input = blockInputs[block];

        for (std::map<Offset, TransferFuncs>::iterator iter = insnEffects[block].begin();
            iter != insnEffects[block].end(); ++iter) {
            Offset off = iter->first;
            TransferFuncs &xferFuncs = iter->second;

            // TODO: try to collapse these in some intelligent fashion
            (*intervals_)[block][off] = input;

            for (TransferFuncs::iterator iter2 = xferFuncs.begin();
                iter2 != xferFuncs.end(); ++iter2) {
                input[iter2->target] = iter2->apply(input);
            }
        }
        (*intervals_)[block][block->end()] = input;

        assert(input == blockOutputs[block]);
    }
}

void StackAnalysis::computeInsnEffects(ParseAPI::Block *block,
                                       Instruction::Ptr insn,
                                       const Offset off,
                                       TransferFuncs &xferFuncs) {
    stackanalysis_printf("\t\tInsn at 0x%lx\n", off);
    entryID what = insn->getOperation().getID();

    // Reminder: what we're interested in:
    // a) Use of the stack pointer to define another register
    //      Assumption: this is done by an e_mov and is a direct copy
    //                  else bottom
    // b) Use of another register to define the stack pointer
    //      Assumption: see ^^
    // c) Modification of the stack pointer
    // 
    // The reason for these assumptions is to keep the analysis reasonable; also,
    // other forms just don't occur. 
    //
    // In summary, the stack pointer must be read or written for us to be interested.
    // 
    // However, we need to detect when a register is destroyed. So if something is written,
    // we assume it is destroyed.
    // 
    // For now, we assume an all-to-all mapping for speed. 

    // Cases we handle
    if (isCall(insn)) {
       if (handleNormalCall(insn, block, off, xferFuncs))
          return;
       else if (handleThunkCall(insn, xferFuncs))
          return;
       else
          return handleDefault(insn, xferFuncs);
    }

    int sign = 1;
    switch (what) {
       case e_push:
          sign = -1;
          //FALLTHROUGH
       case e_pop:
          handlePushPop(insn, sign, xferFuncs);
          break;
       case e_ret_near:
       case e_ret_far:
          handleReturn(insn, xferFuncs);
          break;
       case e_lea:
	 handleLEA(insn, xferFuncs);
	 break;
       case e_sub:
          sign = -1;
          //FALLTHROUGH
       case e_add:
          handleAddSub(insn, sign, xferFuncs);
          break;
       case e_leave:
          handleLeave(xferFuncs);
          break;
       case e_pushfd:
          sign = -1;
          //FALLTHROUGH
       case e_popfd:
          handlePushPopFlags(sign, xferFuncs);
          break;
       case e_pushad:
           sign = -1;
           handlePushPopRegs(sign, xferFuncs);
           break;
       case e_popad:
           // This nukes all registers
           handleDefault(insn, xferFuncs);
           break;
       case power_op_addi:
       case power_op_addic:
          handlePowerAddSub(insn, sign, xferFuncs);
          break;
       case power_op_stwu:
          handlePowerStoreUpdate(insn, xferFuncs);
          break;
       case e_mov:
          handleMov(insn, xferFuncs);
          break;
       case e_movzx:
          handleZeroExtend(insn, xferFuncs);
          break;
       case e_movsx:
       case e_movsxd:
       case e_cbw:
       case e_cwde:
          handleSignExtend(insn, xferFuncs);
          break;
       case e_xor:
          handleXor(insn, xferFuncs);
          break;
       default:
          handleDefault(insn, xferFuncs);
    }
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

    Function::const_blocklist returnBlocks = func->returnBlocks();
    auto rets = returnBlocks.begin();
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
         if (ops.size() == 1) {
             val = 0;
         } else {
             Result imm = ops[1].getValue()->eval();
             assert(imm.defined);
             val = (int) imm.val.s16val;
         }
         returnCleanVals.insert(Height(val));
    }

	Height clean = Height::meet(returnCleanVals);
	if (clean == Height::top) {
		// Non-returning or tail-call exits?
		clean = Height::bottom;
	}

    funcCleanAmounts[func] = clean;

    return clean;
}

StackAnalysis::StackAnalysis() :
   func(NULL), intervals_(NULL), word_size(0) {};
   

StackAnalysis::StackAnalysis(Function *f) : func(f),
					    intervals_(NULL) {
   word_size = func->isrc()->getAddressWidth();
   theStackPtr = Expression::Ptr(new RegisterAST(MachRegister::getStackPointer(func->isrc()->getArch())));
   thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(func->isrc()->getArch())));
};

void StackAnalysis::debug() {
}

std::string StackAnalysis::TransferFunc::format() const {
   std::stringstream ret;

   ret << "[";
   if (target.isValid())
      ret << target.name();
   else
      ret << "<INVALID>";
   ret << ":=";
   if (isBottom()) ret << "<BOTTOM>";
   else if (isTop()) ret << "<TOP>";
   else {
      bool foundType = false;
      if (isAlias()) {
         ret << from.name();
         foundType = true;
      }
      if (isAbs()) {
         ret << abs << dec;
         foundType = true;
      }
      if (isSIB()) {
          for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
              if (iter != fromRegs.begin()) {
                  ret << "+";
              }
              ret << "(";
              ret << (*iter).first.name() << "*" << (*iter).second.first;
              if ((*iter).second.second) {
                  ret << ", will round to TOP or BOTTOM";
              }
              ret << ")";
          }
          foundType = true;
      }
      if (isDelta()) {
          if (!foundType) {
            ret << target.name() << "+" << delta;
          } else {
            ret << "+" << delta;
          }
      }
   }
   if (isTopBottom()) {
    ret << ", will round to TOP or BOTTOM";
   }
   ret << "]";
   return ret.str();
}

std::string StackAnalysis::SummaryFunc::format() const {
   stringstream ret;
   for (TransferSet::const_iterator iter = accumFuncs.begin();
        iter != accumFuncs.end(); ++iter) {
      ret << iter->second.format() << endl;
   }
   return ret.str();
}
void StackAnalysis::findDefinedHeights(ParseAPI::Block* b, Address addr, std::vector<std::pair<MachRegister, Height> >& heights)
{
  if (func == NULL) return;
  
  if (!intervals_) {
    // Check annotation
    func->getAnnotation(intervals_, Stack_Anno);
  }
  if (!intervals_) {
    // Analyze?
    if (!analyze()) return;
  }
  assert(intervals_);
  for(RegisterState::iterator i = (*intervals_)[b][addr].begin();
      i != (*intervals_)[b][addr].end();
      ++i)
  {
    if(i->second.isTop())
    {
      continue;
    }
    stackanalysis_printf("\t\tAdding %s:%s to defined heights at 0x%lx\n",
			 i->first.name().c_str(),
			 i->second.format().c_str(),
			 addr);
    
    heights.push_back(*i);
  }
}

StackAnalysis::Height StackAnalysis::find(Block *b, Address addr, MachRegister reg) {

  Height ret; // Defaults to "top"

  if (func == NULL) return ret;
  
  if (!intervals_) {
    // Check annotation
    func->getAnnotation(intervals_, Stack_Anno);
  }
  if (!intervals_) {
    // Analyze?
    if (!analyze()) return Height();
  }
  assert(intervals_);

  //(*intervals_)[b].find(addr, state);
  //  ret = (*intervals_)[b][addr][reg];
  Intervals::iterator iter = intervals_->find(b);
  if (iter == intervals_->end()) {
	  // How do we return "you stupid idiot"?

	  return Height::bottom;
  }

  StateIntervals &sintervals = iter->second;
  if (sintervals.empty()) {
	  return Height::bottom;
  }
  //Find the last instruction that is <= addr
  StateIntervals::iterator i = sintervals.lower_bound(addr);
  if ((i == sintervals.end() && !sintervals.empty()) || 
      (i->first != addr && i != sintervals.begin())) {
      i--;
  }
  if (i == sintervals.end()) return Height::bottom;

  ret = i->second[reg];

  if (ret.isTop()) {
     return Height::bottom;
  }
  return ret;
}

StackAnalysis::Height StackAnalysis::findSP(Block *b, Address addr) {
   return find(b, addr, sp());
}

StackAnalysis::Height StackAnalysis::findFP(Block *b, Address addr) {
   return find(b, addr, fp());
}

std::ostream &operator<<(std::ostream &os, const Dyninst::StackAnalysis::Height &h) {
  os << "STACK_SLOT[" << h.format() << "]";
  return os;
}

///////////////////
// Insn effect fragments
///////////////////
void StackAnalysis::handleXor(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2);

   // Handle the case where a register is being zeroed out.
   // We recognize such cases as follows:
   //   1. Exactly one register is both read and written
   //   2. The Expression tree for each operand consists of a single leaf node
   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writtenSet;
   operands[0].getWriteSet(writtenSet);
   operands[1].getReadSet(readSet);

   std::vector<Expression::Ptr> children0;
   std::vector<Expression::Ptr> children1;
   operands[0].getValue()->getChildren(children0);
   operands[1].getValue()->getChildren(children1);

   if (readSet.size() == 1 && writtenSet.size() == 1 &&
      (*readSet.begin())->getID() == (*writtenSet.begin())->getID() &&
      children0.size() == 0 && children1.size() == 0) {
      stackanalysis_printf("\t\t\t Register zeroing detected\n");
      xferFuncs.push_back(TransferFunc::absFunc((*writtenSet.begin())->getID(), 0));
   } else {
      handleDefault(insn, xferFuncs);
   }
}


void StackAnalysis::handlePushPop(Instruction::Ptr insn, int sign, TransferFuncs &xferFuncs) {
   long delta = 0;
   Operand arg = insn->getOperand(0);
   // Why was this here? bernat, 12JAN11
   if (arg.getValue()->eval().defined) {
      delta = sign * word_size;
      stackanalysis_printf("\t\t\t Stack height changed by evaluated push/pop: %lx\n", delta);
   }
   else {
	   delta = sign * arg.getValue()->size();
	   //cerr << "Odd case: set delta to " << hex << delta << dec << " for instruction " << insn->format() << endl;
      stackanalysis_printf("\t\t\t Stack height changed by unevalled push/pop: %lx\n", delta);
   }
   //   delta = sign *arg.getValue()->size();
   xferFuncs.push_back(TransferFunc::deltaFunc(sp(), delta));

   // Let's get whatever was popped (if it was)
   if (insn->getOperation().getID() == e_pop &&
       !insn->writesMemory()) {
      MachRegister reg = sp();

      std::set<RegisterAST::Ptr> written;
      insn->getWriteSet(written);
      for (std::set<RegisterAST::Ptr>::iterator iter = written.begin(); 
           iter != written.end(); ++iter) {
         if ((*iter)->getID() != sp()) reg = (*iter)->getID();
      }
      xferFuncs.push_back(TransferFunc::bottomFunc(reg));
   }
}

void StackAnalysis::handleReturn(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   long delta = 0;
   std::vector<Operand> operands;
   insn->getOperands(operands);
   if (operands.size() == 0) {
      delta = word_size;
   }
   else if (operands.size() == 1) {
      // Ret immediate
      Result imm = operands[0].getValue()->eval();
      if (imm.defined) {
         delta = word_size + imm.convert<int>();
      }
      else {
         stackanalysis_printf("\t\t\t Stack height changed by return: bottom\n");
         xferFuncs.push_back(TransferFunc::bottomFunc(sp()));   
      }
   }
   stackanalysis_printf("\t\t\t Stack height changed by return: %lx\n", delta);
   xferFuncs.push_back(TransferFunc::deltaFunc(sp(), delta));   
   return;
}

void StackAnalysis::handleAddSub(Instruction::Ptr insn, int sign, TransferFuncs &xferFuncs) {
   // add reg, mem is ignored
   // add mem, reg bottoms reg
   if (insn->writesMemory()) return;
   if (insn->readsMemory()) {
      handleDefault(insn, xferFuncs);
      return;
   }

   std::set<RegisterAST::Ptr> readSet;
   insn->getOperand(0).getReadSet(readSet);
   if (readSet.size() != 1) {
       fprintf(stderr, "readSet != 1\n");
       handleDefault(insn, xferFuncs);
       return;
   }

   // Add/subtract are op0 += (or -=) op1
   Operand arg = insn->getOperand(1);
   Result res = arg.getValue()->eval();
   if(res.defined) {
     long delta = 0;
     // Size is in bytes... 
     switch(res.size()) {
     case 1:
       delta = sign * res.convert<int8_t>();
       break;
     case 2:
       delta = sign * res.convert<int16_t>();
       break;
     case 4:
       delta = sign * res.convert<int32_t>();
       break;
     case 8:
       delta = sign * res.convert<int64_t>(); 
       break;
     default:
       assert(0);
     }
     stackanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %lx\n", delta);
     xferFuncs.push_back(TransferFunc::deltaFunc((*(readSet.begin()))->getID(), delta));
   }
   else {
     handleDefault(insn, xferFuncs);
   }

   return;
}

void StackAnalysis::handleLEA(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   // LEA has a pattern of:
   // op0: target register
   // op1: add(source, <const>)
   // 
   // Since we don't know what the value in source is, we can't do this a priori. Instead,
   // TRANSFER FUNCTION!
   
   stackanalysis_printf("\t\t\t handleLEA, insn = %s\n", insn->format().c_str());

   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writtenSet;
   insn->getOperand(0).getWriteSet(writtenSet); assert(writtenSet.size() == 1);
   insn->getOperand(1).getReadSet(readSet); //assert(readSet.size() == 1);
   MachRegister written = (*writtenSet.begin())->getID();

   TransferFunc lea;

   // conservative...
   if (readSet.size() != 1) {
       if (readSet.size() != 2) {
           // This shouldn't happen
           stackanalysis_printf("\t\t\tUnexpected LEA case. Setting %s = BOTTOM\n", written.name().c_str());
           xferFuncs.push_back(TransferFunc::bottomFunc(written));
           return;
       }

       MachRegister base, index;
       long scale = -1;

       InstructionAPI::Operand srcOperand = insn->getOperand(1);
       InstructionAPI::Expression::Ptr srcExpr = srcOperand.getValue();

       std::vector<InstructionAPI::Expression::Ptr> children;
       srcExpr->getChildren(children);

       InstructionAPI::Expression::Ptr baseExpr, scaleIndexExpr, deltaExpr;
       bool foundDelta = false;
       while (1) {
           if (typeid(*(children[0])) == typeid(InstructionAPI::RegisterAST)) {
               baseExpr = children[0];
               scaleIndexExpr = children[1];
               if (typeid(*scaleIndexExpr) != typeid(InstructionAPI::BinaryFunction)) {
                   stackanalysis_printf("\t\t\tUnhandled LEA - scale or index missing. Setting %s = BOTTOM\n", written.name().c_str());
                   xferFuncs.push_back(TransferFunc::bottomFunc(written));
                   return;
               }
               break;
           }

           // If there's a delta, then the SIB is inside a dereference,
           // and we need to get the child of the deref
           deltaExpr = children[1];
           if (typeid(*deltaExpr) != typeid(InstructionAPI::Immediate)) {
               stackanalysis_printf("\t\t\tUnhandled LEA - non-immediate delta. Setting %s = BOTTOM\n", written.name().c_str());
               xferFuncs.push_back(TransferFunc::bottomFunc(written));
               return;
           }
           foundDelta = true;
           InstructionAPI::Expression::Ptr tmp = children[0];
           children.clear();
           tmp->getChildren(children);
       }

       // Get the base register
       base = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(baseExpr))->getID();

       // Extract the index and scale
       children.clear();
       scaleIndexExpr->getChildren(children);
       InstructionAPI::Expression::Ptr scaleExpr, indexExpr;
       indexExpr = children[0];
       if (typeid(*indexExpr) != typeid(InstructionAPI::RegisterAST)) {
           stackanalysis_printf("Unhandled LEA - non-register index. Setting %s = BOTTOM\n", written.name().c_str());
           xferFuncs.push_back(TransferFunc::bottomFunc(written));
           return;
       }
       scaleExpr = children[1];
       if (typeid(*scaleExpr) != typeid(InstructionAPI::Immediate)) {
           stackanalysis_printf("Unhandled LEA - non-immediate scale. Setting %s = BOTTOM\n", written.name().c_str());
           xferFuncs.push_back(TransferFunc::bottomFunc(written));
           return;
       }

       index = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(indexExpr))->getID();
       scale = (boost::dynamic_pointer_cast<InstructionAPI::Immediate>(scaleExpr))->eval().convert<long>();

       long delta = 0;
       if (foundDelta) {
           Result deltaRes = (boost::dynamic_pointer_cast<InstructionAPI::Immediate>(deltaExpr))->eval();
           switch(deltaRes.size()) {
               case 1:
                   delta = (long)deltaRes.convert<int8_t>();
                   break;
               case 2:
                   delta = (long)deltaRes.convert<int16_t>();
                   break;
               case 4:
                   delta = (long)deltaRes.convert<int32_t>();
                   break;
               case 8:
                   delta = (long)deltaRes.convert<int64_t>();
                   break;
               default:
                   assert(0);
           }
       }

       if (!base.isValid() || !index.isValid() || (scale==-1)) {
           stackanalysis_printf("Misunderstood LEA. Setting %s = BOTTOM\n", written.name().c_str());
           xferFuncs.push_back(TransferFunc::bottomFunc(written));
           return;
       }

       // Consolidate when possible
       if (base == index) {
           base = MachRegister();
           scale++;
       }

       std::map<MachRegister,std::pair<long,bool> > fromRegs;
       if (base.isValid()) {
           fromRegs.insert(make_pair(base, make_pair(1, false)));
       }
       fromRegs.insert(make_pair(index, make_pair(scale, false)));
       lea = TransferFunc::sibFunc(fromRegs, delta, written);
       xferFuncs.push_back(lea);
       return;
   } else {
       lea = TransferFunc::aliasFunc((*(readSet.begin()))->getID(), written);
   }

   // Non-SIB LEA also performs computation, so we need to determine and set the delta parameter.
   MachRegister reg;
   long scale = 1;
   long delta = 0;
   InstructionAPI::Operand srcOperand = insn->getOperand(1);
   InstructionAPI::Expression::Ptr srcExpr = srcOperand.getValue();

   stackanalysis_printf("\t\t\t\t srcOperand = %s\n", srcExpr->format().c_str());

   std::vector<InstructionAPI::Expression::Ptr> children;
   srcExpr->getChildren(children);
   stackanalysis_printf("\t\t\t\t srcOperand # children = %d\n", children.size());

   InstructionAPI::Expression::Ptr regExpr, scaleExpr, deltaExpr;
   bool foundScale = false;
   bool foundDelta = false;

   if (children.size()) {
       for (auto iter = children.begin(); iter != children.end(); ++iter) {
           InstructionAPI::Expression::Ptr child = *iter;
           if (typeid(*child) == typeid(InstructionAPI::Immediate)) {
               deltaExpr = child;
               foundDelta = true;
           } else if (typeid(*child) == typeid(InstructionAPI::RegisterAST)) {
               regExpr = child;
           } else if (typeid(*child) == typeid(InstructionAPI::BinaryFunction)) {
               std::vector<InstructionAPI::Expression::Ptr> subchildren;
               child->getChildren(subchildren);
               regExpr = subchildren[0];
               if (typeid(*regExpr) != typeid(InstructionAPI::RegisterAST)) {
                   stackanalysis_printf("Unhandled LEA - non-SIB non-register index. Setting %s = BOTTOM\n", written.name().c_str());
                   xferFuncs.push_back(TransferFunc::bottomFunc(written));
                   return;
               }
               scaleExpr = subchildren[1];
               if (typeid(*scaleExpr) != typeid(InstructionAPI::Immediate)) {
                   stackanalysis_printf("Unhandled LEA - non-SIB non-immediate scale. Setting %s = BOTTOM\n", written.name().c_str());
                   xferFuncs.push_back(TransferFunc::bottomFunc(written));
                   return;
               }
               foundScale = true;
           }
       }
   } else {
       regExpr = srcExpr;
   }
   reg = (boost::dynamic_pointer_cast<InstructionAPI::RegisterAST>(regExpr))->getID();

   if (foundScale) {
       scale = (boost::dynamic_pointer_cast<InstructionAPI::Immediate>(scaleExpr))->eval().convert<long>();
   }

   if (foundDelta) {
       Result deltaRes = (boost::dynamic_pointer_cast<InstructionAPI::Immediate>(deltaExpr))->eval();
       switch(deltaRes.size()) {
           case 1:
               delta = (long)deltaRes.convert<int8_t>();
               break;
           case 2:
               delta = (long)deltaRes.convert<int16_t>();
               break;
           case 4:
               delta = (long)deltaRes.convert<int32_t>();
               break;
           case 8:
               delta = (long)deltaRes.convert<int64_t>();
               break;
           default:
               assert(0);
       }
   }

   if (!foundScale) {
       lea.delta = delta;
   } else {
       std::map<MachRegister,std::pair<long, bool> > fromRegs;
       fromRegs.insert(make_pair(reg, make_pair(scale, false)));
       lea = TransferFunc::sibFunc(fromRegs, delta, written);
   }

   xferFuncs.push_back(lea);
   return;
}

void StackAnalysis::handleLeave(TransferFuncs &xferFuncs) {
   // This is... mov esp, ebp; pop ebp.
   // Handle it as such.

   // mov esp, ebp;
    xferFuncs.push_back(TransferFunc::aliasFunc(fp(), sp()));
    
   // pop ebp
    xferFuncs.push_back(TransferFunc::deltaFunc(sp(), word_size)); 
   xferFuncs.push_back(TransferFunc::bottomFunc(fp()));
}

void StackAnalysis::handlePushPopFlags(int sign, TransferFuncs &xferFuncs) {
   // Fixed-size push/pop
   xferFuncs.push_back(TransferFunc::deltaFunc(sp(), sign * word_size));
}

void StackAnalysis::handlePushPopRegs(int sign, TransferFuncs &xferFuncs) {
   // Fixed-size push/pop
	// 8 registers
   xferFuncs.push_back(TransferFunc::deltaFunc(sp(), sign * 8 * word_size));
}

void StackAnalysis::handlePowerAddSub(Instruction::Ptr insn, int sign, TransferFuncs &xferFuncs) {
   // Add/subtract are op0 = op1 +/- op2; we'd better read the stack pointer as well as writing it
   if (!insn->isRead(theStackPtr) ||
       !insn->isWritten(theStackPtr)) {
      return handleDefault(insn, xferFuncs);
   }
   
   Operand arg = insn->getOperand(2);
   Result res = arg.getValue()->eval();
   if(res.defined) {
      xferFuncs.push_back(TransferFunc::deltaFunc(sp(), sign * res.convert<long>()));
      stackanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %lx\n", sign * res.convert<long>());
   }
   else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sp()));
      stackanalysis_printf("\t\t\t Stack height changed by unevalled add/sub: bottom\n");
   }   
   return;
}

void StackAnalysis::handlePowerStoreUpdate(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   if(!insn->isWritten(theStackPtr)) {
      return handleDefault(insn, xferFuncs);
   }
   
   std::set<Expression::Ptr> memWriteAddrs;
   insn->getMemoryWriteOperands(memWriteAddrs);
   Expression::Ptr stackWrite = *(memWriteAddrs.begin());
   stackanalysis_printf("\t\t\t ...checking operand %s\n", stackWrite->format().c_str());
   stackanalysis_printf("\t\t\t ...binding %s to 0\n", theStackPtr->format().c_str());
   stackWrite->bind(theStackPtr.get(), Result(u32, 0));
   Result res = stackWrite->eval();
   if(res.defined) {
      long delta = res.convert<long>();
      xferFuncs.push_back(TransferFunc::deltaFunc(sp(), delta));
      stackanalysis_printf("\t\t\t Stack height changed by evalled stwu: %lx\n", delta);
   }
   else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sp()));
      stackanalysis_printf("\t\t\t Stack height changed by unevalled stwu: bottom\n");
   }
}

void StackAnalysis::handleMov(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   // A couple of cases:
   //   1. mov reg, reg
   //   2. mov mem, reg
   //   3. mov reg, mem
   //   4. mov imm, reg
   // 
   // The first and fourth are fine, the second bottoms the reg, and the third we ignore.

   // Handle case 3
   if (insn->writesMemory()) return;

   MachRegister read;
   MachRegister written;
   std::set<RegisterAST::Ptr> regs;
   RegisterAST::Ptr reg;

   insn->getWriteSet(regs);

   // There can only be 0 or 1 target regs. Since we short-circuit the case
   // where the target is memory, there must be exactly 1 target reg.
   assert(regs.size() == 1);
   reg = *(regs.begin());
   written = reg->getID();
   regs.clear();

   // Handle case 2
   if (insn->readsMemory()) {
      xferFuncs.push_back(TransferFunc::bottomFunc(written));
      return;
   }

   insn->getReadSet(regs);
   if (regs.size() > 1) {
       //assert(regs.size() < 2);
       // I had been asserting that there were two registers. Then a rep-prefixed mov instruction came
       // along and ruined my day...
       // This is a garbage instruction that is prefixed. Treat as bottom. 
       xferFuncs.push_back(TransferFunc::bottomFunc(written));
       return;
   }


   if (!regs.empty()) {
	   reg = *(regs.begin());
	   read = reg->getID();
   }
   regs.clear();

   // Handle case 1
   if (read.isValid()) {
	   stackanalysis_printf("\t\t\t Alias detected: %s -> %s\n", read.name().c_str(), written.name().c_str());
	   xferFuncs.push_back(TransferFunc::aliasFunc(read, written));
   }
   else {
       InstructionAPI::Operand readOperand = insn->getOperand(1);
       InstructionAPI::Expression::Ptr readExpr = readOperand.getValue();
       stackanalysis_printf("\t\t\t\t readOperand = %s\n", readExpr->format().c_str());

       // Handle case 4
       if (typeid(*readExpr) == typeid(InstructionAPI::Immediate)) {
           long readValue = (boost::dynamic_pointer_cast<InstructionAPI::Immediate>(readExpr))->eval().convert<long>();
           stackanalysis_printf("\t\t\t Non-register-register move: %s set to %ld\n", written.name().c_str(), readValue);
           xferFuncs.push_back(TransferFunc::absFunc(written, readValue));
       } else {
           // This case is not expected to occur
           stackanalysis_printf("\t\t\t Unhandled non-register-register move: %s set to BOTTOM\n", written.name().c_str());
           xferFuncs.push_back(TransferFunc::bottomFunc(written));
       }
       return;
   }
}

void StackAnalysis::handleZeroExtend(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
    // This instruction zero extends the read register into the written register

    // Don't care about memory stores
    if (insn->writesMemory()) return;

    MachRegister read;
    MachRegister written;
    std::set<RegisterAST::Ptr> regs;
    RegisterAST::Ptr reg;

    insn->getWriteSet(regs);

    // There can only be 0 or 1 target regs. Since we short-circuit the case
    // where the target is memory, there must be exactly 1 target reg.
    assert(regs.size() == 1);
    reg = *(regs.begin());
    written = reg->getID();
    regs.clear();

    // Handle memory loads
    if (insn->readsMemory()) {
        xferFuncs.push_back(TransferFunc::bottomFunc(written));
        return;
    }

    insn->getReadSet(regs);
    if (regs.size() != 1) {
        stackanalysis_printf("\t\t\t Unhandled zero extend, setting %s = BOTTOM\n", written.name().c_str());
        xferFuncs.push_back(TransferFunc::bottomFunc(written));
        return;
    }
    reg = *(regs.begin());
    read = reg->getID();

    stackanalysis_printf("\t\t\t Alias detected: %s -> %s\n", read.name().c_str(), written.name().c_str());
    xferFuncs.push_back(TransferFunc::aliasFunc(read, written));
}

void StackAnalysis::handleSignExtend(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
    // This instruction sign extends the read register into the written register
    // Aliasing insn't really correct here...sign extension is going to change the value...

    // Don't care about memory stores
    if (insn->writesMemory()) return;

    MachRegister read;
    MachRegister written;
    std::set<RegisterAST::Ptr> regs;
    RegisterAST::Ptr reg;

    insn->getWriteSet(regs);

    // There can only be 0 or 1 target regs. Since we short-circuit the case
    // where the target is memory, there must be exactly 1 target reg.
    assert(regs.size() == 1);
    reg = *(regs.begin());
    written = reg->getID();
    regs.clear();

    // Handle memory loads
    if (insn->readsMemory()) {
        xferFuncs.push_back(TransferFunc::bottomFunc(written));
        return;
    }

    insn->getReadSet(regs);
    if (regs.size() != 1) {
        stackanalysis_printf("\t\t\t Unhandled sign extend, setting %s = BOTTOM\n", written.name().c_str());
        xferFuncs.push_back(TransferFunc::bottomFunc(written));
        return;
    }
    reg = *(regs.begin());
    read = reg->getID();

    stackanalysis_printf("\t\t\t Sign extend insn detected: %s -> %s (must be top or bottom)\n", read.name().c_str(), written.name().c_str());
    xferFuncs.push_back(TransferFunc::aliasFunc(read, written, true));
    return;
}

void StackAnalysis::handleDefault(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   std::set<RegisterAST::Ptr> written;
   insn->getWriteSet(written);
   for (std::set<RegisterAST::Ptr>::iterator iter = written.begin(); 
        iter != written.end(); ++iter) {

      xferFuncs.push_back(TransferFunc::aliasFunc((*iter)->getID(), (*iter)->getID(), true));
      stackanalysis_printf("\t\t\t Unhandled insn %s detected: %s set to topBottom\n", insn->format().c_str(),
                           (*iter)->getID().name().c_str());
   }
   return;
}

bool StackAnalysis::isCall(Instruction::Ptr insn) {
   return insn->getCategory() == c_CallInsn;
}

bool StackAnalysis::handleNormalCall(Instruction::Ptr insn, Block *block, Offset off, TransferFuncs &xferFuncs) {
   if (!insn->getControlFlowTarget()) return false;

   // Must be a thunk based on parsing.
   if (off != block->lastInsnAddr()) return false;
   
   // Top caller-save registers
   // Bottom return registers
   ABI* abi = ABI::getABI(word_size);
   const bitArray callWritten = abi->getCallWrittenRegisters();
   const bitArray returnRegs = abi->getReturnRegisters();
   for (auto iter = abi->getIndexMap()->begin();
           iter != abi->getIndexMap()->end();
           ++iter) {
       // We only care about GPRs right now
       unsigned int gpr;
       Architecture arch = insn->getArch();
       switch(arch) {
           case Arch_x86:
               gpr = x86::GPR;
               break;
           case Arch_x86_64:
               gpr = x86_64::GPR;
               break;
           case Arch_ppc32:
               gpr = ppc32::GPR;
               break;
           case Arch_ppc64:
               gpr = ppc64::GPR;
               break;
           default:
               handleDefault(insn, xferFuncs);
               return true;
       };
       if ((*iter).first.regClass() == gpr) {
           if (callWritten.test((*iter).second)) {
               if (returnRegs.test((*iter).second)) {
                   // Bottom
                   xferFuncs.push_back(TransferFunc::bottomFunc((*iter).first));
               } else {
                   // Top
                   xferFuncs.push_back(TransferFunc::topFunc((*iter).first));
               }
           }
       }
   }

   const Block::edgelist & outs = block->targets();  
   Block::edgelist::const_iterator eit = outs.begin();
   for( ; eit != outs.end(); ++eit) {
      Edge *cur_edge = (Edge*)*eit;
      
      if (cur_edge->type() == DIRECT) {
         // For some reason we're treating this
         // call as a branch. So it shifts the stack
         // like a push (heh) and then we're done.
         stackanalysis_printf("\t\t\t Stack height changed by simulate-jump call\n");
         xferFuncs.push_back(TransferFunc::deltaFunc(sp(), -1 * word_size));
         return true;
      }
      
      if (cur_edge->type() != CALL) 
         continue;
      
      Block *target_bbl = cur_edge->trg();
      Function *target_func = target_bbl->obj()->findFuncByEntry(target_bbl->region(), target_bbl->start());
      
      if (!target_func)
         continue;
      
      Height h = getStackCleanAmount(target_func);
      if (h == Height::bottom) {
         stackanalysis_printf("\t\t\t Stack height changed by self-cleaning function: bottom\n");
         xferFuncs.push_back(TransferFunc::bottomFunc(sp()));
      }
      else {
         stackanalysis_printf("\t\t\t Stack height changed by self-cleaning function: %ld\n", h.height());
         xferFuncs.push_back(TransferFunc::deltaFunc(sp(), h.height()));
      }
      return true;

   }
   stackanalysis_printf("\t\t\t Stack height assumed unchanged by call\n");
   return true;
}
                                       

bool StackAnalysis::handleThunkCall(Instruction::Ptr insn, TransferFuncs &xferFuncs) {
   // We know that we're not a normal call, so it depends on whether
   // the CFT is "next instruction" or not. 
   if (insn->getCategory() != c_CallInsn ||
       !insn->getControlFlowTarget()) return false;
   
   // Eval of getCFT(0) == size?
   Expression::Ptr cft = insn->getControlFlowTarget();
   cft->bind(thePC.get(), Result(u32, 0));
   Result res = cft->eval();
   if (!res.defined) return false;
   if (res.convert<unsigned>() == insn->size()) {
      xferFuncs.push_back(TransferFunc::deltaFunc(sp(), -1 * word_size));
      return true;
   }
   // Else we're calling a mov, ret thunk that has no effect on the stack pointer
   return true;
}


void StackAnalysis::createEntryInput(RegisterState &input) {
   // FIXME for POWER/non-IA32
   // IA32 - the in height includes the return address and therefore
   // is <wordsize>
   // POWER - the in height is 0

#if defined(arch_power)
   input[sp()] = Height(0);
#elif (defined(arch_x86) || defined(arch_x86_64))
   input[sp()] = Height(-1 * word_size);
#else
   assert(0 && "Unimplemented architecture");
#endif
}

StackAnalysis::RegisterState StackAnalysis::getSrcOutputRegs(Edge* e)
{
	Block* b = e->src();
	stackanalysis_printf("%lx ", b->lastInsnAddr());
	return blockOutputs[b];
}

void StackAnalysis::meetInputs(Block *block, RegisterState& blockInput, RegisterState &input) {
   input.clear();

   //Intraproc epred; // ignore calls, returns in edge iteration
   //NoSinkPredicate epred2(&epred); // ignore sink node (unresolvable)
   intra_nosink epred2;
   
   stackanalysis_printf("\t ... In edges: ");
   const Block::edgelist & inEdges = block->sources();
   std::for_each(boost::make_filter_iterator(epred2, inEdges.begin(), inEdges.end()),
		 boost::make_filter_iterator(epred2, inEdges.end(), inEdges.end()),
		 boost::bind(&StackAnalysis::meet,
					 this,
					 boost::bind(&StackAnalysis::getSrcOutputRegs, this, _1),
					 boost::ref(input)));
   stackanalysis_printf("\n");

   meet(blockInput, input);

}

void StackAnalysis::meet(const RegisterState &input, RegisterState &accum) {
   for (RegisterState::const_iterator iter = input.begin();
        iter != input.end(); ++iter) {
      accum[iter->first] = Height::meet(iter->second, accum[iter->first]);
   }
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::deltaFunc(MachRegister r, long d) {
   return TransferFunc(uninitialized, d, MachRegister(), r);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::absFunc(MachRegister r, long a, bool i) {
   return TransferFunc(a, 0, MachRegister(), r, i);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::aliasFunc(MachRegister f, MachRegister t, bool i) {
   return TransferFunc (uninitialized, 0, f, t, i);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::bottomFunc(MachRegister r) {
   return TransferFunc(notUnique, notUnique, MachRegister(), r);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::topFunc(MachRegister r) {
   return absFunc(r, uninitialized, false);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::sibFunc(std::map<MachRegister, std::pair<long,bool> > f, long d, MachRegister t) {
    return TransferFunc(f, d, t);
}

bool StackAnalysis::TransferFunc::isBottom() const {
   return (delta == notUnique || abs == notUnique) && !from.isValid() && !isSIB();
}

bool StackAnalysis::TransferFunc::isTop() const {
   return (!isDelta() && !isAbs() && !isAlias() && !isSIB());
}

bool StackAnalysis::TransferFunc::isAlias() const {
   return from.isValid();
}

bool StackAnalysis::TransferFunc::isAbs() const {
   return (abs != uninitialized && abs != notUnique);
}

bool StackAnalysis::TransferFunc::isDelta() const {
   return (delta != 0);
}

bool StackAnalysis::TransferFunc::isSIB() const {
    return (fromRegs.size() > 0);
}


// Destructive update of the input map. Assumes inputs are absolute, uninitalized, or 
// bottom; no deltas.
StackAnalysis::Height StackAnalysis::TransferFunc::apply(const RegisterState &inputs ) const {
	assert(target.isValid());
	// Bottom stomps everything
	if (isBottom()) {
		return Height::bottom;
	}

	RegisterState::const_iterator iter = inputs.find(target);
	Height input;
	if (iter != inputs.end()) {
		input = iter->second;
	}
	else {
		input = Height::top;
	}

    bool isTopBottomOrig = isTopBottom();

    if (isSIB()) {
        input = Height::top; // SIB overwrites, so start at TOP
        for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
            MachRegister curReg = (*iter).first;
            long curScale = (*iter).second.first;
            bool curTopBottom = (*iter).second.second;
            auto findReg = inputs.find(curReg);
            Height regInput;
            if (findReg == inputs.end()) {
                regInput = Height::top;
            } else {
                regInput = findReg->second;
            }

            if (regInput == Height::top) {
                // Ignore--has no effect on ouput
            } else if (regInput == Height::bottom) {
                // Must bottom everything
                input = Height::bottom;
                break;
            } else {
                if (curScale != 1) {
                    // Must bottom
                    input = Height::bottom;
                    break;
                } else {
                    if (input != Height::top) {
                        // Must bottom--cannot add stack heights
                        input = Height::bottom;
                        break;
                    } else {
                        if (curTopBottom) {
                            input = Height::bottom;
                        } else {
                            // Finally! We can add!
                            input += regInput;
                        }
                    }
                }
            }
        }
    }

   if (isAbs()) {
      // We cannot be an alias, as the absolute removes that. 
      assert(!isAlias());
      // Apply the absolute
      // NOTE: an absolute is not a stack height, set input to top
      //input = abs;
      input = Height::top;
   }
   if (isAlias()) {
      // Cannot be absolute
      assert(!isAbs());
      // Copy the input value from whatever we're an alias of.
	  RegisterState::const_iterator iter2 = inputs.find(from);
	  if (iter2 != inputs.end()) input = iter2->second;
	  else input = Height::top;
   }
   if (isDelta()) {
      input += delta;
   }
   if (isTopBottomOrig) {
       if (!input.isTop()) {
           input = Height::bottom;
       }
   }
   return input;
}

// Accumulation to the input map. This is intended to create a summary, so we create
// something that can take further input.
void StackAnalysis::TransferFunc::accumulate(std::map<MachRegister, TransferFunc> &inputs ) {
   TransferFunc &input = inputs[target];
   if (input.target.isValid()) assert(input.target == target);
   input.target = target; // Default constructed TransferFuncs won't have this
   assert(target.isValid());

   // Bottom stomps everything
   if (isBottom()) {
      input = bottomFunc(target);
      return;
   }
   bool isTopBottomOrig = isTopBottom();
   if (!input.isTopBottom()) {
       input.topBottom = isTopBottomOrig;
   }
   // Absolutes override everything else
   if (isAbs()) {
      input = absFunc(target, abs, isTopBottomOrig);
      return;
   }
   if (isSIB()) {
        std::map<MachRegister, std::pair<long,bool> > newFromRegs;
        long newDelta = delta;

        for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
            MachRegister fromRegOrig = (*iter).first;
            long scaleOrig = (*iter).second.first;

            auto findReg = inputs.find(fromRegOrig);
            if (findReg == inputs.end()) {
                // Easy case
                auto found = newFromRegs.find(fromRegOrig);
                if (found == newFromRegs.end()) {
                    newFromRegs.insert(make_pair(fromRegOrig, make_pair(scaleOrig, false)));
                } else {
                    newFromRegs[fromRegOrig].first += scaleOrig;
                }
            } else {
                TransferFunc fromRegFunc = findReg->second;

                if (fromRegFunc.isAbs()) {
                    newDelta += fromRegFunc.abs*scaleOrig;

                    // If we're processing the last source register,
                    // and we haven't added anything to the new register set, this must be a group of abs
                    auto tmp = iter;
                    tmp++;
                    if (tmp == fromRegs.end()) {
                        if (newFromRegs.size() == 0) {
                            input = absFunc(target, newDelta);
                            return;
                        }
                    }
                }
                if (fromRegFunc.isSIB()) {
                    // Replace registers and update scales
                    for (auto regIter = fromRegFunc.fromRegs.begin(); regIter != fromRegFunc.fromRegs.end(); ++regIter) {
                        MachRegister replaceReg = regIter->first;
                        long replaceScale = regIter->second.first;
                        long replaceTopBottom = regIter->second.second;
                        long newScale = replaceScale*scaleOrig;

                        // If this register already exists in the map, we need to add, rather than overwrite!
                        auto found = newFromRegs.find(replaceReg);
                        if (found == newFromRegs.end()) {
                            newFromRegs.insert(make_pair(replaceReg, make_pair(newScale, replaceTopBottom)));
                        } else {
                            newFromRegs[replaceReg].first += newScale;
                            newFromRegs[replaceReg].second += replaceTopBottom;
                        }
                    }
                }
                if (fromRegFunc.isBottom()) {
                    // Any bottom'd input bottoms the output
                    input = bottomFunc(target);
                    return;
                }
                if (fromRegFunc.isAlias()) {
                    // Replace fromRegOrig with fromRegFunc.from
                    auto found = newFromRegs.find(fromRegFunc.from);
                    if (found == newFromRegs.end()) {
                        newFromRegs.insert(make_pair(fromRegFunc.from, make_pair(scaleOrig,fromRegFunc.isTopBottom())));
                    } else {
                        newFromRegs[fromRegFunc.from].first += scaleOrig;
                    }
                }
                if (fromRegFunc.isDelta()) {
                    newDelta += fromRegFunc.delta*scaleOrig;
                    if (!fromRegFunc.isAlias() && !fromRegFunc.isSIB() && !fromRegFunc.isAbs()) {
                        // Add the register back in...
                        auto found = newFromRegs.find(fromRegOrig);
                        if (found == newFromRegs.end()) {
                            newFromRegs.insert(make_pair(fromRegOrig, make_pair(scaleOrig,false)));
                        } else {
                            newFromRegs[fromRegOrig].first += scaleOrig;
                        }
                    }
                }
                // This is the default constructed target when the target didn't already exist--need to explicitly update...
                if (fromRegFunc.isTop()) {
                    auto found = newFromRegs.find(fromRegOrig);
                    if (found == newFromRegs.end()) {
                        newFromRegs.insert(make_pair(fromRegOrig, make_pair(scaleOrig,false)));
                    } else {
                        newFromRegs[fromRegOrig].first += scaleOrig;
                    }
                }
            }
        }
        input = sibFunc(newFromRegs, newDelta, target);
        input.topBottom = isTopBottomOrig;
        return;
   }

   // Aliases can be tricky
   // apply alias logic only if registers are different
   if (isAlias() && target != from) {
      // We need to record that we want to take the inflow height
      // of a different register. 
	   // Don't do an inputs[from] as that creates
	   std::map<MachRegister, TransferFunc>::iterator iter = inputs.find(from);
	   if (iter == inputs.end()) {
		   // Aliasing to something we haven't seen yet; easy
		   input = *this;
           input.topBottom = isTopBottomOrig;
		   return;
	   }

	   TransferFunc &alias = iter->second;
      
      if (alias.isAbs()) {
         // Easy; we reset the height, so we don't care about the inflow height.
         // This might be a delta from that absolute, but all that we care about is that
         // we ignore any inflow. 
         assert(!alias.isAlias());
		 input = absFunc(input.target, alias.abs);
         input.topBottom = isTopBottomOrig || alias.isTopBottom(); // If we got an abs from an isTopBottom, this also needs to be set
		 assert(input.target.isValid());

         // if the input was also a delta, apply this
         if (isDelta()) {
            input.delta += delta;
         }
         return;
      }
      if (alias.isAlias()) {
         // Transitivity! We cut that short.
         // Again, it might be a delta since the alias, which we'll copy over. It cannot
         // be an absolute because that will remove the alias (and vice versa).
         assert(!alias.isAbs());
         input = alias;
         input.target = target;
         input.topBottom = isTopBottomOrig || alias.isTopBottom();
         assert(input.target.isValid());
                   
                 // if the input was also a delta, apply this also 
                 if (isDelta()) {
                    input.delta += delta;
                 }
      
         return;
      }
      if (alias.isSIB()) {
        input = alias;
        input.target = target;
        input.topBottom = isTopBottomOrig;

        if (isDelta()) {
            input.delta += delta;
        }

        return;
      }

      // without bottom we mess up in the default case.
      if (alias.isBottom()) {
          input = bottomFunc(target);
          return;
      }

      // add top?

	  // Default case: record the alias, zero out everything else, copy over the delta
	  // if it's defined.
	  //input.target is defined
	  input.from = alias.target;
	  input.abs = uninitialized;
	  if (alias.isDelta()) {
         input.delta = alias.delta;
	  }
	  else {
		  input.delta = 0;
	  }
      input.topBottom = isTopBottomOrig || alias.isTopBottom(); // pass source tb
      input.fromRegs.clear();

          // if the input was also a delta, apply this also 
          if (isDelta()) {
            input.delta += delta;
          }

	  return;
   }

   if (isDelta()) {
      // A delta can apply cleanly to anything, since Height += handles top/bottom
      input.delta += delta;
      return;
   }
   // Reachable because isDelta returns false for delta = 0; this is OK
   return;
}

void StackAnalysis::SummaryFunc::apply(const RegisterState &in, RegisterState &out) const {
	// Copy all the elements we don't have xfer funcs for. 
	out = in;

	// We apply in parallel, since all summary funcs are from the start of the block.
	for (TransferSet::const_iterator iter = accumFuncs.begin();
		iter != accumFuncs.end(); ++iter) {
		assert(iter->first.isValid());
		out[iter->first] = iter->second.apply(in);
	}
}


void StackAnalysis::SummaryFunc::add(TransferFuncs &xferFuncs) {
   // We need to update our register->xferFunc map
   // with the effects of each of the transferFuncs. 
   
   for (TransferFuncs::iterator iter = xferFuncs.begin(); 
        iter != xferFuncs.end(); ++iter) {
      TransferFunc &func = *iter;

      func.accumulate(accumFuncs);
      validate();
   }

}

void StackAnalysis::SummaryFunc::validate() const {
   for (TransferSet::const_iterator iter = accumFuncs.begin(); 
	   iter != accumFuncs.end(); ++iter)
   {
	   const TransferFunc &func = iter->second;
	   assert(func.target.isValid());
	   if (func.isAlias()) assert(!func.isAbs());
	   if (func.isAbs()) assert(!func.isAlias());
	   if (func.isBottom()) assert(!func.isAlias());
   }
}

MachRegister StackAnalysis::sp() { 
 return MachRegister::getStackPointer(func->isrc()->getArch());
}

MachRegister StackAnalysis::fp() { 
 return MachRegister::getFramePointer(func->isrc()->getArch());
}

std::string StackAnalysis::format(const RegisterState &input) const {
	std::stringstream ret;
	for (RegisterState::const_iterator iter = input.begin(); iter != input.end(); ++iter) {
		ret << iter->first.name() << " := " << iter->second.format() << ", ";
	}
	return ret.str();
}
