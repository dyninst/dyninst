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

#include "stackanalysis.h"

#include <boost/bind.hpp>
#include <queue>
#include <vector>

#include "instructionAPI/h/BinaryFunction.h"
#include "instructionAPI/h/Dereference.h"
#include "instructionAPI/h/Expression.h"
#include "instructionAPI/h/Immediate.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/Register.h"
#include "instructionAPI/h/Result.h"
#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeObject.h"

#include "ABI.h"
#include "Annotatable.h"
#include "debug_dataflow.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;

const StackAnalysis::Height StackAnalysis::Height::bottom(
   StackAnalysis::Height::notUnique, StackAnalysis::Height::BOTTOM);
const StackAnalysis::Height StackAnalysis::Height::top(
   StackAnalysis::Height::uninitialized, StackAnalysis::Height::TOP);

AnnotationClass<StackAnalysis::Intervals>
   Stack_Anno_Intervals(std::string("Stack_Anno_Intervals"));
AnnotationClass<StackAnalysis::BlockEffects>
   Stack_Anno_Block_Effects(std::string("Stack_Anno_Block_Effects"));
AnnotationClass<StackAnalysis::InstructionEffects>
   Stack_Anno_Insn_Effects(std::string("Stack_Anno_Insn_Effects"));

template class std::list<Dyninst::StackAnalysis::TransferFunc*>;
template class std::map<Dyninst::Absloc, Dyninst::StackAnalysis::Height>;
template class std::vector<Dyninst::InstructionAPI::Instruction::Ptr>;
//
// Concepts:
//
// There are three terms we throw around:
// 
// Stack height: the size of the stack; (cur_stack_ptr - func_start_stack_ptr)
// Stack delta: the difference in the size of the stack over the execution of
//   a region of code (basic block here). (block_end_stack_ptr -
//   block_start_stack_ptr)
// Stack clean: the amount the callee function shifts the stack. This is an x86
//   idiom. On x86 the caller pushes arguments onto the stack (and thus shifts
//   the stack). Normally the caller also cleans the stack (that is, removes
//   arguments). However, some callee functions perform this cleaning
//   themselves. We need to account for this in our analysis, which otherwise
//   assumes that callee functions don't alter the stack.


bool StackAnalysis::analyze() {
   df_init_debug();

   genInsnEffects();

   stackanalysis_printf("\tPerforming fixpoint analysis\n");
   fixpoint();
   stackanalysis_printf("\tCreating SP interval tree\n");
   summarize();

   func->addAnnotation(intervals_, Stack_Anno_Intervals);

   if (df_debug_stackanalysis) {
      debug();
   }

   stackanalysis_printf("Finished stack analysis for function %s\n",
      func->name().c_str());

   return true;
}


bool StackAnalysis::genInsnEffects() {
   // Check if we've already done this work
   if (blockEffects != NULL && insnEffects != NULL) return true;
   func->getAnnotation(blockEffects, Stack_Anno_Block_Effects);
   func->getAnnotation(insnEffects, Stack_Anno_Insn_Effects);
   if (blockEffects != NULL && insnEffects != NULL) return true;

   blockEffects = new BlockEffects();
   insnEffects = new InstructionEffects();

   stackanalysis_printf("Beginning insn effect generation for function %s\n",
      func->name().c_str());

   // Initial analysis to get register stack heights
   stackanalysis_printf("\tSummarizing intermediate block effects\n");
   summarizeBlocks();
   stackanalysis_printf("\tPerforming intermediate fixpoint analysis\n");
   fixpoint();
   stackanalysis_printf("\tCreating intermediate SP interval tree\n");
   summarize();

   // Clear mappings
   blockEffects->clear();
   insnEffects->clear();
   blockInputs.clear();
   blockOutputs.clear();

   // Generate final block effects with stack slot tracking
   stackanalysis_printf("\tGenerating final block effects\n");
   summarizeBlocks();

   // Annotate insnEffects and blockEffects to avoid rework
   func->addAnnotation(blockEffects, Stack_Anno_Block_Effects);
   func->addAnnotation(insnEffects, Stack_Anno_Insn_Effects);

   stackanalysis_printf("Finished insn effect generation for function %s\n",
      func->name().c_str());

   return true;
}

typedef std::vector<std::pair<Instruction::Ptr, Offset> > InsnVec;
static void getInsnInstances(Block *block, InsnVec &insns) {
   Offset off = block->start();
   const unsigned char *ptr = (const unsigned char *)
      block->region()->getPtrToInstruction(off);
   if (ptr == NULL) return;
   InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
   while (off < block->end()) {
      insns.push_back(std::make_pair(d.decode(), off));
      off += insns.back().first->size();
   }
}


// We want to create a transfer function for the block as a whole. This will
// allow us to perform our fixpoint calculation over blocks (thus, O(B^2))
// rather than instructions (thus, O(I^2)).
//
// Handling the stack height is straightforward. We also accumulate region
// changes in terms of a stack of Region objects.
void StackAnalysis::summarizeBlocks() {
   Function::blocklist bs(func->blocks());
   for (auto bit = bs.begin(); bit != bs.end(); ++bit) {
      Block *block = *bit;
      // Accumulators. They have the following behavior:
      //
      // New region: add to the end of the regions list
      // Offset to stack pointer: accumulate to delta.
      // Setting the stack pointer: zero delta, set set_value.

      SummaryFunc &bFunc = (*blockEffects)[block];

      stackanalysis_printf("\t Block starting at 0x%lx: %s\n", block->start(),
         bFunc.format().c_str());
      InsnVec instances;
      getInsnInstances(block, instances);

      for (unsigned j = 0; j < instances.size(); j++) {
         InstructionAPI::Instruction::Ptr insn = instances[j].first;
         Offset &off = instances[j].second;

         // Fills in insnEffects[off]
         TransferFuncs &xferFuncs = (*insnEffects)[block][off];

         computeInsnEffects(block, insn, off, xferFuncs);
         bFunc.add(xferFuncs);

         stackanalysis_printf("\t\t\t At 0x%lx:  %s\n", off,
            bFunc.format().c_str());
      }
   stackanalysis_printf("\t Block summary for 0x%lx: %s\n", block->start(),
      bFunc.format().c_str());
   }
}

struct intra_nosink : public ParseAPI::EdgePredicate {
   virtual bool operator()(Edge* e) {
      static Intraproc i;
      static NoSinkPredicate n;
      return i(e) && n(e);
   }
};

void add_target(std::queue<Block*>& worklist, Edge* e) {
   worklist.push(e->trg());
}

void StackAnalysis::fixpoint() {
   intra_nosink epred2;

   std::queue<Block *> worklist;
   worklist.push(func->entry());

   bool firstBlock = true;
   while (!worklist.empty()) {
      Block *block = worklist.front();
      worklist.pop();
      stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n",
         block->start());

      // Step 1: calculate the meet over the heights of all incoming
      // intraprocedural blocks.
      AbslocState input;
      if (firstBlock) {
         createEntryInput(input);
         stackanalysis_printf("\t Primed initial block\n");
      } else {
         stackanalysis_printf("\t Calculating meet with block [%x-%x]\n",
            block->start(), block->lastInsnAddr());
         meetInputs(block, blockInputs[block], input);
      }
      stackanalysis_printf("\t New in meet: %s\n", format(input).c_str());

      // Step 2: see if the input has changed
      if (input == blockInputs[block]) {
         // No new work here
         stackanalysis_printf("\t ... equal to current, skipping block\n");
         continue;
      }
      stackanalysis_printf("\t ... inequal to current %s, analyzing block\n",
         format(blockInputs[block]).c_str());

      blockInputs[block] = input;

      // Step 3: calculate our new outs
      (*blockEffects)[block].apply(input, blockOutputs[block]);
      stackanalysis_printf("\t ... output from block: %s\n",
         format(blockOutputs[block]).c_str());

      // Step 4: push all children on the worklist.
      const Block::edgelist & outEdges = block->targets();
      std::for_each(
         boost::make_filter_iterator(epred2, outEdges.begin(), outEdges.end()),
         boost::make_filter_iterator(epred2, outEdges.end(), outEdges.end()),
         boost::bind(add_target, boost::ref(worklist), _1)
      );

      firstBlock = false;
   }
}


bool StackAnalysis::getFunctionSummary(TransferSet &summary) {
   df_init_debug();

   if (!genInsnEffects()) return false;
   assert(!blockEffects->empty());

   const Function::const_blocklist &retBlocks = func->returnBlocks();
   if (retBlocks.empty()) return false;  // No return edges means no summary

   if (blockSummaryOutputs.empty()) {
      summaryFixpoint();
   }

   // Join possible values at all return edges
   TransferSet tempSummary;
   for (auto iter = retBlocks.begin(); iter != retBlocks.end(); iter++) {
      Block *currBlock = *iter;
      meetSummary(blockSummaryOutputs[currBlock], tempSummary);
   }

   // Remove identity functions for simplicity
   summary.clear();
   for (auto iter = tempSummary.begin(); iter != tempSummary.end(); iter++) {
      const Absloc &loc = iter->first;
      const TransferFunc &tf = iter->second;
      if (!tf.isIdentity()) {
         summary[loc] = tf;
      }
   }

   return true;
}


void StackAnalysis::summaryFixpoint() {
   intra_nosink epred2;

   std::queue<Block *> worklist;
   worklist.push(func->entry());

   bool firstBlock = true;
   while (!worklist.empty()) {
      Block *block = worklist.front();
      worklist.pop();
      stackanalysis_printf("\tSummary fixpoint analysis: visiting block at "
         "0x%lx\n", block->start());

      // Step 1: calculate the meet over the heights of all incoming
      // intraprocedural blocks.
      TransferSet input;
      if (firstBlock) {
         createSummaryEntryInput(input);
         stackanalysis_printf("\t Primed initial block\n");
      } else {
         stackanalysis_printf("\t Calculating meet with block [%x-%x]\n",
            block->start(), block->lastInsnAddr());
         meetSummaryInputs(block, blockSummaryInputs[block], input);
      }
      stackanalysis_printf("\t New in meet: %s\n", format(input).c_str());

      // Step 2: see if the input has changed
      if (input == blockSummaryInputs[block] && !firstBlock) {
         // No new work here
         stackanalysis_printf("\t ... equal to current, skipping block\n");
         continue;
      }
      stackanalysis_printf("\t ... inequal to current %s, analyzing block\n",
         format(blockSummaryInputs[block]).c_str());

      blockSummaryInputs[block] = input;

      // Step 3: calculate our new outs
      (*blockEffects)[block].accumulate(input, blockSummaryOutputs[block]);
      stackanalysis_printf("\t ... output from block: %s\n",
         format(blockSummaryOutputs[block]).c_str());

      // Step 4: push all children on the worklist.
      const Block::edgelist & outEdges = block->targets();
      std::for_each(
         boost::make_filter_iterator(epred2, outEdges.begin(), outEdges.end()),
         boost::make_filter_iterator(epred2, outEdges.end(), outEdges.end()),
         boost::bind(add_target, boost::ref(worklist), _1)
      );

      firstBlock = false;
   }
}


void StackAnalysis::summarize() {
   // Now that we know the actual inputs to each block, we create intervals by
   // replaying the effects of each instruction.
   if (intervals_ != NULL) delete intervals_;
   intervals_ = new Intervals();

   Function::blocklist bs = func->blocks();
   for (auto bit = bs.begin(); bit != bs.end(); ++bit) {
      Block *block = *bit;
      AbslocState input = blockInputs[block];

      std::map<Offset, TransferFuncs>::iterator iter;
      for (iter = (*insnEffects)[block].begin();
         iter != (*insnEffects)[block].end(); ++iter) {
         Offset off = iter->first;
         TransferFuncs &xferFuncs = iter->second;

         // TODO: try to collapse these in some intelligent fashion
         (*intervals_)[block][off] = input;

         for (TransferFuncs::iterator iter2 = xferFuncs.begin();
            iter2 != xferFuncs.end(); ++iter2) {
            input[iter2->target] = iter2->apply(input);
            if (input[iter2->target].isTop()) {
               input.erase(iter2->target);
            }
         }
      }
      (*intervals_)[block][block->end()] = input;
      assert(input == blockOutputs[block]);
   }
}

void StackAnalysis::computeInsnEffects(ParseAPI::Block *block,
   Instruction::Ptr insn, const Offset off, TransferFuncs &xferFuncs) {
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
   // In summary, the stack pointer must be read or written for us to be
   // interested.
   //
   // However, we need to detect when a register is destroyed. So if something
   // is written, we assume it is destroyed.
   //
   // For now, we assume an all-to-all mapping for speed.

   // Cases we handle
   if (isCall(insn)) {
      if (handleNormalCall(insn, block, off, xferFuncs)) return;
      else if (handleThunkCall(insn, xferFuncs)) return;
      else return handleDefault(insn, xferFuncs);
   }

   int sign = 1;
   switch (what) {
      case e_push:
         sign = -1;
         //FALLTHROUGH
      case e_pop:
         handlePushPop(insn, block, off, sign, xferFuncs);
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
      case e_addsd:
         handleAddSub(insn, block, off, sign, xferFuncs);
         break;
      case e_leave:
         handleLeave(block, off, xferFuncs);
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
      case e_movsd_sse:
         handleMov(insn, block, off, xferFuncs);
         break;
      case e_movzx:
         handleZeroExtend(insn, block, off, xferFuncs);
         break;
      case e_movsx:
      case e_movsxd:
         handleSignExtend(insn, block, off, xferFuncs);
         break;
      case e_cbw:
      case e_cwde:
         handleSpecialSignExtend(insn, xferFuncs);
         break;
      case e_xor:
         handleXor(insn, block, off, xferFuncs);
         break;
      case e_div:
      case e_idiv:
         handleDiv(insn, xferFuncs);
         break;
      case e_mul:
      case e_imul:
         handleMul(insn, xferFuncs);
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

   InstructionDecoder decoder((const unsigned char*) NULL, 0,
      func->isrc()->getArch());
   unsigned char *cur;

   std::set<Height> returnCleanVals;

   Function::const_blocklist returnBlocks = func->returnBlocks();
   for (auto rets = returnBlocks.begin(); rets != returnBlocks.end(); ++rets) {
      Block *ret = *rets;
      cur = (unsigned char *) ret->region()->getPtrToInstruction(
         ret->lastInsnAddr());
      Instruction::Ptr insn = decoder.decode(cur);

      entryID what = insn->getOperation().getID();
      if (what != e_ret_near) continue;

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

StackAnalysis::StackAnalysis() : func(NULL), blockEffects(NULL),
   insnEffects(NULL), intervals_(NULL), word_size(0) {}
   
StackAnalysis::StackAnalysis(Function *f) : func(f), blockEffects(NULL),
   insnEffects(NULL), intervals_(NULL) {
   word_size = func->isrc()->getAddressWidth();
   theStackPtr = Expression::Ptr(new RegisterAST(MachRegister::getStackPointer(
      func->isrc()->getArch())));
   thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(
      func->isrc()->getArch())));
}

void StackAnalysis::debug() {
}

std::string StackAnalysis::TransferFunc::format() const {
   std::stringstream ret;

   ret << "[";
   if (target.isValid())
      ret << target.format();
   else
      ret << "<INVALID>";
   ret << ":=";
   if (isBottom()) ret << "<BOTTOM>";
   else if (isRetop()) ret << "<re-TOP>";
   else if (isTop()) ret << "<TOP>";
   else {
      bool foundType = false;
      if (isCopy()) {
         ret << from.format();
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
            ret << (*iter).first.format() << "*" << (*iter).second.first;
            if ((*iter).second.second) {
               ret << ", will round to TOP or BOTTOM";
            }
            ret << ")";
         }
         foundType = true;
      }
      if (isDelta()) {
         assert(foundType);
         ret << "+" << delta;
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


void StackAnalysis::findDefinedHeights(ParseAPI::Block* b, Address addr,
   std::vector<std::pair<Absloc, Height> >& heights) {
   if (func == NULL) return;

   if (!intervals_) {
      // Check annotation
      func->getAnnotation(intervals_, Stack_Anno_Intervals);
   }
   if (!intervals_) {
      // Analyze?
      if (!analyze()) return;
   }
   assert(intervals_);
   for (AbslocState::iterator i = (*intervals_)[b][addr].begin();
      i != (*intervals_)[b][addr].end(); ++i) {
      if (i->second.isTop()) continue;

      stackanalysis_printf("\t\tAdding %s:%s to defined heights at 0x%lx\n",
         i->first.format().c_str(), i->second.format().c_str(), addr);

      heights.push_back(*i);
   }
}

StackAnalysis::Height StackAnalysis::find(Block *b, Address addr, Absloc loc) {
   Height ret; // Defaults to "top"

   if (func == NULL) return ret;

   if (!intervals_) {
      // Check annotation
      func->getAnnotation(intervals_, Stack_Anno_Intervals);
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
   // Find the last instruction that is <= addr
   StateIntervals::iterator i = sintervals.lower_bound(addr);
   if ((i == sintervals.end() && !sintervals.empty()) ||
      (i->first != addr && i != sintervals.begin())) {
      i--;
   }
   if (i == sintervals.end()) return Height::bottom;

   ret = i->second[loc];

   if (ret.isTop()) {
      return Height::bottom;
   }
   return ret;
}

StackAnalysis::Height StackAnalysis::findSP(Block *b, Address addr) {
   return find(b, addr, Absloc(sp()));
}

StackAnalysis::Height StackAnalysis::findFP(Block *b, Address addr) {
   return find(b, addr, Absloc(fp()));
}

std::ostream &operator<<(std::ostream &os,
   const Dyninst::StackAnalysis::Height &h) {
   os << "STACK_SLOT[" << h.format() << "]";
   return os;
}

///////////////////
// Insn effect fragments
///////////////////

// Visitor class to evaluate stack heights and PC-relative addresses
class StateEvalVisitor : public Visitor {
public:
   // addr is the starting address of instruction insn.
   // insn is the instruction containing the expression to evaluate.
   StateEvalVisitor(Address addr, Instruction::Ptr insn,
      StackAnalysis::AbslocState *s) : defined(true), state(s) {
      rip = addr + insn->size();
   }

   StateEvalVisitor() : defined(false), state(NULL), rip(0) {}

   bool isDefined() {
      return defined && results.size() == 1;
   }

   std::pair<Address, bool> getResult() {
      return isDefined() ? results.back() : make_pair((Address) 0, false);
   }

   virtual void visit(BinaryFunction *bf) {
      if (!defined) return;

      Address arg1 = results.back().first;
      bool isHeight1 = results.back().second;
      results.pop_back();
      Address arg2 = results.back().first;
      bool isHeight2 = results.back().second;
      results.pop_back();

      if (bf->isAdd()) {
         if (isHeight1 && isHeight2) {
            defined = false;
         } else {
            results.push_back(make_pair(arg1 + arg2, isHeight1 || isHeight2));
         }
      } else if (bf->isMultiply()) {
         if (isHeight1 || isHeight2) {
            defined = false;
         } else {
            results.push_back(make_pair(arg1 * arg2, false));
         }
      } else {
         defined = false;
      }
   }

   virtual void visit(Immediate *imm) {
      if (!defined) return;

      results.push_back(make_pair(imm->eval().convert<Address>(), false));
   }

   virtual void visit(RegisterAST *rast) {
      if (!defined) return;

      MachRegister reg = rast->getID();
      if (reg == x86::eip || reg == x86_64::eip || reg == x86_64::rip) {
         results.push_back(make_pair(rip, false));
      } else if (state != NULL) {
         auto regState = state->find(Absloc(reg));
         if (regState == state->end() || regState->second.isTop() ||
            regState->second.isBottom()) {
            defined = false;
         } else {
            results.push_back(make_pair(regState->second.height(), true));
         }
      } else {
         defined = false;
      }
   }

   virtual void visit(Dereference *) {
      defined = false;
   }

private:
   bool defined;
   StackAnalysis::AbslocState *state;
   Address rip;

   // Stack for calculations
   // bool is true if the value in Address is a stack height
   std::deque<std::pair<Address, bool>> results;

};

void StackAnalysis::handleXor(Instruction::Ptr insn, Block *block,
   const Offset off, TransferFuncs &xferFuncs) {
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
      // xor reg1, reg1
      const MachRegister &reg = (*writtenSet.begin())->getID();
      Absloc loc(reg);
      xferFuncs.push_back(TransferFunc::absFunc(loc, 0));
      retopBaseSubReg(reg, xferFuncs);
      return;
   }


   if (insn->writesMemory()) {
      // xor mem1, reg2/imm2
      assert(writtenSet.size() == 0);

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> &addrExpr = children0;
      assert(addrExpr.size() == 1);

      // Try to determine the written memory address
      Absloc writtenLoc;
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         std::pair<Address, bool> resultPair = visitor.getResult();
         if (resultPair.second) {
            // We have a stack slot
            writtenLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            writtenLoc = Absloc(resultPair.first);
         }
      } else {
         // Couldn't determine written location, so assume it's not on the stack
         // and ignore it.
         return;
      }

      if (readSet.size() > 0) {
         // xor mem1, reg2
         assert(readSet.size() == 1);
         Absloc from((*readSet.begin())->getID());
         std::map<Absloc, std::pair<long, bool>> fromRegs;
         fromRegs[writtenLoc] = std::make_pair(1, true);
         fromRegs[from] = std::make_pair(1, true);
         xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
      } else {
         // xor mem1, imm2
         // Xor with immediate.  Set topBottom on target loc
         Expression::Ptr immExpr = operands[1].getValue();
         assert(typeid(*immExpr) == typeid(Immediate));
         xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
            true));
      }
      return;
   }


   assert(writtenSet.size() == 1);
   MachRegister written = (*writtenSet.begin())->getID();
   Absloc writtenLoc(written);

   if (insn->readsMemory()) {
      // xor reg1, mem2
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> &addrExpr = children1;
      assert(addrExpr.size() == 1);

      // Try to determine the read memory address
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         std::pair<Address, bool> resultPair = visitor.getResult();
         Absloc readLoc;
         if (resultPair.second) {
            // We have a stack slot
            readLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            readLoc = Absloc(resultPair.first);
         }
         std::map<Absloc, std::pair<long, bool>> fromRegs;
         fromRegs[writtenLoc] = std::make_pair(1, true);
         fromRegs[readLoc] = std::make_pair(1, true);
         xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // Couldn't determine the read location.  Assume it's not on the stack.
         xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
            true));
         copyBaseSubReg(written, xferFuncs);
      }
      return;
   }


   // xor reg1, reg2/imm2
   MachRegister read;
   if (!readSet.empty()) {
      assert(readSet.size() == 1);
      read = (*readSet.begin())->getID();
   }
   Absloc readLoc(read);

   if (read.isValid()) {
      // xor reg1, reg2
      std::map<Absloc, std::pair<long, bool>> fromRegs;
      fromRegs[writtenLoc] = std::make_pair(1, true);
      fromRegs[readLoc] = std::make_pair(1, true);
      xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
      copyBaseSubReg(written, xferFuncs);
   } else {
      // xor reg1, imm1
      InstructionAPI::Expression::Ptr readExpr = operands[1].getValue();
      assert(typeid(*readExpr) == typeid(InstructionAPI::Immediate));
      xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc, true));
      copyBaseSubReg(written, xferFuncs);
   }
}


void StackAnalysis::handleDiv(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   stackanalysis_printf("\t\t\thandleDiv: %s\n", insn->format().c_str());
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 3);

   Expression::Ptr quotient = operands[1].getValue();
   Expression::Ptr remainder = operands[0].getValue();
   Expression::Ptr divisor = operands[2].getValue();
   assert(typeid(*quotient) == typeid(RegisterAST));
   assert(typeid(*remainder) == typeid(RegisterAST));
   assert(typeid(*divisor) != typeid(Immediate));

   MachRegister quotientReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(quotient))->getID();
   MachRegister remainderReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(remainder))->getID();

   xferFuncs.push_back(TransferFunc::retopFunc(Absloc(quotientReg)));
   xferFuncs.push_back(TransferFunc::retopFunc(Absloc(remainderReg)));
   retopBaseSubReg(quotientReg, xferFuncs);
   retopBaseSubReg(remainderReg, xferFuncs);
}


void StackAnalysis::handleMul(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   // MULs have a few forms:
   //   1. mul reg1, reg2/mem2
   //     -- reg1 = reg1 * reg2/mem2
   //   2. mul reg1, reg2/mem2, imm3
   //     -- reg1 = reg2/mem2 * imm3
   //   3. mul reg1, reg2, reg3/mem3
   //     -- reg1:reg2 = reg2 * reg3/mem3
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2 || operands.size() == 3);

   Expression::Ptr target = operands[0].getValue();
   assert(typeid(*target) == typeid(RegisterAST));
   MachRegister targetReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(target))->getID();

   if (operands.size() == 2) {
      // Form 1
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(targetReg)));
      retopBaseSubReg(targetReg, xferFuncs);
   } else {
      Expression::Ptr multiplicand = operands[1].getValue();
      Expression::Ptr multiplier = operands[2].getValue();

      if (typeid(*multiplier) == typeid(Immediate)) {
         // Form 2
         assert(typeid(*multiplicand) == typeid(RegisterAST) ||
            typeid(*multiplicand) == typeid(Dereference));
         long multiplierVal = multiplier->eval().convert<long>();
         if (multiplierVal == 0) {
            xferFuncs.push_back(TransferFunc::absFunc(Absloc(targetReg), 0));
            retopBaseSubReg(targetReg, xferFuncs);
         } else if (multiplierVal == 1) {
            if (typeid(*multiplicand) == typeid(RegisterAST)) {
               // mul reg1, reg2, 1
               MachRegister multiplicandReg = boost::dynamic_pointer_cast<
                  RegisterAST>(multiplicand)->getID();
               Absloc targetLoc(targetReg);
               Absloc multiplicandLoc(multiplicandReg);
               xferFuncs.push_back(TransferFunc::copyFunc(multiplicandLoc,
                  targetLoc));
               copyBaseSubReg(targetReg, xferFuncs);
            } else {
               // mul reg1, mem2, 1
               Absloc targetLoc(targetReg);
               xferFuncs.push_back(TransferFunc::bottomFunc(targetLoc));
               bottomBaseSubReg(targetReg, xferFuncs);
            }
         } else {
            xferFuncs.push_back(TransferFunc::retopFunc(Absloc(targetReg)));
            retopBaseSubReg(targetReg, xferFuncs);
         }
      } else {
         // Form 3
         assert(typeid(*multiplicand) == typeid(RegisterAST));
         assert(typeid(*multiplier) == typeid(RegisterAST) ||
            typeid(*multiplier) == typeid(Dereference));
         MachRegister multiplicandReg = boost::dynamic_pointer_cast<
            RegisterAST>(multiplicand)->getID();
         xferFuncs.push_back(TransferFunc::retopFunc(Absloc(targetReg)));
         xferFuncs.push_back(TransferFunc::retopFunc(Absloc(multiplicandReg)));
         retopBaseSubReg(targetReg, xferFuncs);
         retopBaseSubReg(multiplicandReg, xferFuncs);
      }
   }
}


void StackAnalysis::handlePushPop(Instruction::Ptr insn, Block *block,
   const Offset off, int sign, TransferFuncs &xferFuncs) {

   long delta = 0;
   Operand arg = insn->getOperand(0);
   // Why was this here? bernat, 12JAN11
   if (arg.getValue()->eval().defined) {
      delta = sign * word_size;
      stackanalysis_printf(
         "\t\t\t Stack height changed by evaluated push/pop: %lx\n", delta);
   } else {
      delta = sign * arg.getValue()->size();
      //cerr << "Odd case: set delta to " << hex << delta << dec <<
      //   " for instruction " << insn->format() << endl;
      stackanalysis_printf(
         "\t\t\t Stack height changed by unevalled push/pop: %lx\n", delta);
   }
   //   delta = sign *arg.getValue()->size();
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), delta));
   copyBaseSubReg(sp(), xferFuncs);

   if (insn->getOperation().getID() == e_push && insn->writesMemory()) {
      // This is a push.  Let's record the value pushed on the stack if
      // possible.
      if (intervals_ != NULL) {
         Absloc sploc(sp());
         Height spHeight = (*intervals_)[block][off][sploc];
         if (!spHeight.isTop() && !spHeight.isBottom()) {
            // Get written stack slot
            long writtenSlotHeight = spHeight.height() - word_size;
            Absloc writtenLoc(writtenSlotHeight, 0, NULL);

            // Get copied register
            Expression::Ptr readExpr = insn->getOperand(0).getValue();
            assert(typeid(*readExpr) == typeid(RegisterAST));
            MachRegister readReg = boost::dynamic_pointer_cast<RegisterAST>(
               readExpr)->getID();
            Absloc readLoc(readReg);

            xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
         }
      }
   } else if (insn->getOperation().getID() == e_pop && !insn->writesMemory()) {
      // This is a pop.  Let's record the value popped if possible.

      // Get target register
      Expression::Ptr targExpr = insn->getOperand(0).getValue();
      assert(typeid(*targExpr) == typeid(RegisterAST));
      MachRegister targReg = boost::dynamic_pointer_cast<RegisterAST>(
         targExpr)->getID();
      Absloc targLoc(targReg);

      if (intervals_ != NULL) {
         Absloc sploc(sp());
         Height spHeight = (*intervals_)[block][off][sploc];
         if (spHeight.isTop()) {
            // Load from a topped location. Since StackMod fails when storing
            // to an undetermined topped location, it is safe to assume the
            // value loaded here is not a stack height.
            xferFuncs.push_back(TransferFunc::retopFunc(targLoc));
            retopBaseSubReg(targReg, xferFuncs);
         } else if (spHeight.isBottom()) {
            xferFuncs.push_back(TransferFunc::bottomFunc(targLoc));
            bottomBaseSubReg(targReg, xferFuncs);
         } else {
            // Get copied stack slot
            long readSlotHeight = spHeight.height();
            Absloc readLoc(readSlotHeight, 0, NULL);

            xferFuncs.push_back(TransferFunc::copyFunc(readLoc, targLoc));
            copyBaseSubReg(targReg, xferFuncs);
         }
      } else {
         xferFuncs.push_back(TransferFunc::bottomFunc(targLoc));
         bottomBaseSubReg(targReg, xferFuncs);
      }
   } else {
      assert(false);
   }
}

void StackAnalysis::handleReturn(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   long delta = 0;
   std::vector<Operand> operands;
   insn->getOperands(operands);
   if (operands.size() < 2) {
      delta = word_size;
   } else {
      fprintf(stderr, "Unhandled RET instruction: %s\n",
         insn->format().c_str());
      assert(false);
   }
/*   else if (operands.size() == 1) {
      // Ret immediate
      Result imm = operands[0].getValue()->eval();
      if (imm.defined) {
         delta = word_size + imm.convert<int>();
      }
      else {
         stackanalysis_printf("\t\t\t Stack height changed by return: bottom\n");
         xferFuncs.push_back(TransferFunc::bottomFunc(Absloc(sp())));
      }
   }
*/
   stackanalysis_printf("\t\t\t Stack height changed by return: %lx\n", delta);
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), delta));
   copyBaseSubReg(sp(), xferFuncs);
}


void StackAnalysis::handleAddSub(Instruction::Ptr insn, Block *block,
   const Offset off, int sign, TransferFuncs &xferFuncs) {
   // Possible forms for add/sub:
   //   1. add reg1, reg2
   //     -- reg1 = reg1 + reg2
   //   2. add reg1, mem2
   //     -- reg1 = reg1 + mem2
   //   3. add mem1, reg2
   //     -- mem1 = mem1 + reg2
   //   4. add reg1, imm2
   //     -- reg1 = reg1 + imm2
   //   5. add mem1, imm2
   //     -- mem1 = mem1 + imm2
   //
   //   #1 is handled by setting reg1 to a sibFunc of reg1 and reg2.
   //   #2 depends on whether or not the location of mem2 can be determined
   //      statically.
   //      a. If it can, reg1 is set to a sibFunc of reg1 and mem2.
   //      b. Otherwise, reg1 is set to topBottom.
   //   #3 depends on whether or not the location of mem1 can be determined
   //      statically.
   //      a. If it can, mem1 is set to a sibFunc of mem1 and reg2.
   //      b. Otherwise, nothing happens.
   //   #4 is handled with a delta.
   //   #5 depends on whether or not the location of mem1 can be determined
   //      statically.
   //      a. If it can, mem1 is handled with a delta.
   //      b. Otherwise, nothing happens.

   stackanalysis_printf("\t\t\t handleAddSub, insn = %s\n",
      insn->format().c_str());
   Architecture arch = insn->getArch();  // Needed for debug messages
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2);

   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writeSet;
   operands[1].getReadSet(readSet);
   operands[0].getWriteSet(writeSet);

   if (insn->writesMemory()) {
      // Cases 3 and 5
      assert(writeSet.size() == 0);
      stackanalysis_printf("\t\t\tMemory add/sub to: %s\n",
         operands[0].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[0].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the written memory address
      Absloc writtenLoc;
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         std::pair<Address, bool> resultPair = visitor.getResult();
         if (resultPair.second) {
            // We have a stack slot
            writtenLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            writtenLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            writtenLoc.format().c_str());
      } else {
         // Cases 3b and 5b
         stackanalysis_printf("\t\t\tCan't determine location\n");
         return;
      }

      if (readSet.size() > 0) {
         // Case 3a
         assert(readSet.size() == 1);
         std::map<Absloc, std::pair<long, bool>> terms;
         Absloc src((*readSet.begin())->getID());
         Absloc &dest = writtenLoc;
         terms[src] = make_pair(sign, false);
         terms[dest] = make_pair(1, false);
         xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, dest));
      } else {
         // Case 5a
         Expression::Ptr immExpr = operands[1].getValue();
         assert(typeid(*immExpr) == typeid(Immediate));
         long immVal = immExpr->eval().convert<long>();
         xferFuncs.push_back(TransferFunc::deltaFunc(writtenLoc,
            sign * immVal));
      }
      return;
   }

   // Cases 1, 2, and 4
   assert(writeSet.size() == 1);
   MachRegister written = (*writeSet.begin())->getID();
   Absloc writtenLoc(written);

   if (insn->readsMemory()) {
      // Case 2
      stackanalysis_printf("\t\t\tAdd/sub from: %s\n",
         operands[1].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the read memory address
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         // Case 2a
         std::pair<Address, bool> resultPair = visitor.getResult();
         Absloc readLoc;
         if (resultPair.second) {
            // We have a stack slot
            readLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            readLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            readLoc.format().c_str());
         std::map<Absloc, std::pair<long, bool>> terms;
         terms[readLoc] = make_pair(sign, false);
         terms[writtenLoc] = make_pair(1, false);
         xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // Case 2b
         stackanalysis_printf("\t\t\tCan't determine location\n");
         xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
            true));
         copyBaseSubReg(written, xferFuncs);
      }
      return;
   }

   Result res = operands[1].getValue()->eval();
   if (res.defined) {
      // Case 4
      long delta = sign * extractDelta(res);
      stackanalysis_printf("\t\t\t Register changed by add/sub: %lx\n", delta);
      xferFuncs.push_back(TransferFunc::deltaFunc(writtenLoc, delta));
      copyBaseSubReg(written, xferFuncs);
   } else {
      // Case 1
      std::map<Absloc, std::pair<long, bool>> terms;
      Absloc src((*readSet.begin())->getID());
      Absloc &dest = writtenLoc;
      terms[src] = make_pair(sign, false);
      terms[dest] = make_pair(1, false);
      xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, dest));
      copyBaseSubReg(written, xferFuncs);
   }
}

void StackAnalysis::handleLEA(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   // LEA has a few patterns:
   //   op0: target register
   //-------------------------------
   //   op1: reg + reg * imm + imm
   //            or
   //   op1: reg + reg * imm
   //            or
   //   op1: imm + reg * imm
   //            or
   //   op1: reg + imm
   //            or
   //   op1: reg
   //

   stackanalysis_printf("\t\t\t handleLEA, insn = %s\n",
      insn->format().c_str());

   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writtenSet;
   insn->getOperand(0).getWriteSet(writtenSet);
   insn->getOperand(1).getReadSet(readSet);
   assert(writtenSet.size() == 1);
   assert(readSet.size() == 1 || readSet.size() == 2);
   MachRegister written = (*writtenSet.begin())->getID();
   Absloc writeloc(written);

   InstructionAPI::Operand srcOperand = insn->getOperand(1);
   InstructionAPI::Expression::Ptr srcExpr = srcOperand.getValue();
   std::vector<InstructionAPI::Expression::Ptr> children;
   srcExpr->getChildren(children);

   stackanalysis_printf("\t\t\t\t srcOperand = %s\n",
      srcExpr->format().c_str());

   if (readSet.size() == 1) {
      InstructionAPI::Expression::Ptr regExpr, scaleExpr, deltaExpr;
      bool foundScale = false;
      bool foundDelta = false;

      if (children.size() == 2) {
         if (typeid(*children[0]) == typeid(Immediate)) {
            // op1: imm + reg * imm
            deltaExpr = children[0];
            Expression::Ptr scaleIndexExpr = children[1];
            assert(typeid(*scaleIndexExpr) == typeid(BinaryFunction));
            children.clear();
            scaleIndexExpr->getChildren(children);

            regExpr = children[0];
            scaleExpr = children[1];
            assert(typeid(*regExpr) == typeid(RegisterAST));
            assert(typeid(*scaleExpr) == typeid(Immediate));
            foundScale = true;
            foundDelta = true;
         } else if (typeid(*children[0]) == typeid(RegisterAST)) {
            // op1: reg + imm
            regExpr = children[0];
            deltaExpr = children[1];
            stackanalysis_printf("\t\t\t\t reg: %s\n",
               regExpr->format().c_str());
            stackanalysis_printf("\t\t\t\t delta: %s\n",
               deltaExpr->format().c_str());
            assert(typeid(*regExpr) == typeid(RegisterAST));
            assert(typeid(*deltaExpr) == typeid(Immediate));
            foundDelta = true;
         } else {
            assert(false);
         }
      } else if (children.size() == 0) {
         // op1: reg
         regExpr = srcExpr;
         assert(typeid(*regExpr) == typeid(RegisterAST));
      } else {
         assert(false);
      }

      MachRegister reg = (boost::dynamic_pointer_cast<InstructionAPI::
         RegisterAST>(regExpr))->getID();

      long scale = 1;
      if (foundScale) {
         scale = scaleExpr->eval().convert<long>();
      }

      long delta = 0;
      if (foundDelta) {
         delta = extractDelta(deltaExpr->eval());
      }

      if (foundScale) {
         std::map<Absloc,std::pair<long, bool> > fromRegs;
         fromRegs.insert(make_pair(Absloc(reg), make_pair(scale, false)));
         xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, delta, writeloc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         Absloc readloc(reg);
         TransferFunc lea = TransferFunc::copyFunc(readloc, writeloc);
         lea.delta = delta;
         xferFuncs.push_back(lea);
         copyBaseSubReg(written, xferFuncs);
      }
   } else if (readSet.size() == 2) {
      Expression::Ptr baseExpr, indexExpr, scaleExpr, deltaExpr;
      bool foundDelta = false;

      assert(children.size() == 2);
      if (typeid(*children[1]) == typeid(Immediate)) {
         // op1: reg + reg * imm + imm
         // Extract the delta and continue on to get base, index, and scale
         deltaExpr = children[1];
         stackanalysis_printf("\t\t\t\t delta: %s\n",
            deltaExpr->format().c_str());
         Expression::Ptr sibExpr = children[0];
         assert(typeid(*sibExpr) == typeid(BinaryFunction));
         children.clear();
         sibExpr->getChildren(children);
         assert(children.size() == 2);
         foundDelta = true;
      }

      // op1: reg + reg * imm
      baseExpr = children[0];
      Expression::Ptr scaleIndexExpr = children[1];
      stackanalysis_printf("\t\t\t\t base: %s\n", baseExpr->format().c_str());
      assert(typeid(*scaleIndexExpr) == typeid(BinaryFunction));

      // Extract the index and scale
      children.clear();
      scaleIndexExpr->getChildren(children);
      assert(children.size() == 2);
      indexExpr = children[0];
      scaleExpr = children[1];
      stackanalysis_printf("\t\t\t\t index: %s\n",
         indexExpr->format().c_str());
      stackanalysis_printf("\t\t\t\t scale: %s\n",
         scaleExpr->format().c_str());

      assert(typeid(*baseExpr) == typeid(RegisterAST));
      assert(typeid(*indexExpr) == typeid(RegisterAST));
      assert(typeid(*scaleExpr) == typeid(Immediate));

      MachRegister base = (boost::dynamic_pointer_cast<InstructionAPI::
         RegisterAST>(baseExpr))->getID();
      MachRegister index = (boost::dynamic_pointer_cast<InstructionAPI::
         RegisterAST>(indexExpr))->getID();
      long scale = scaleExpr->eval().convert<long>();

      long delta = 0;
      if (foundDelta) {
         Result deltaRes = deltaExpr->eval();
         delta = extractDelta(deltaRes);
      }

      assert(base.isValid() && index.isValid() && scale != -1);

      // Consolidate when possible
      if (base == index) {
         base = MachRegister();
         scale++;
      }

      std::map<Absloc,std::pair<long,bool> > fromRegs;
      if (base.isValid()) {
         fromRegs.insert(make_pair(Absloc(base), make_pair(1, false)));
      }
      fromRegs.insert(make_pair(Absloc(index), make_pair(scale, false)));
      xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, delta, writeloc));
      copyBaseSubReg(written, xferFuncs);
   } else {
      assert(false);
   }
}

void StackAnalysis::handleLeave(Block *block, const Offset off,
   TransferFuncs &xferFuncs) {
   // This is... mov esp, ebp; pop ebp.
   // Handle it as such.

   // mov esp, ebp;
   xferFuncs.push_back(TransferFunc::copyFunc(Absloc(fp()), Absloc(sp())));
   copyBaseSubReg(fp(), xferFuncs);

   // pop ebp: adjust stack pointer
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), word_size));
   copyBaseSubReg(sp(), xferFuncs);

   // pop ebp: copy value from stack to ebp
   Absloc targLoc(fp());
   if (intervals_ != NULL) {
      Absloc sploc(sp());
      Height spHeight = (*intervals_)[block][off][sploc];
      if (spHeight.isTop()) {
         // Load from a topped location. Since StackMod fails when storing
         // to an undetermined topped location, it is safe to assume the
         // value loaded here is not a stack height.
         xferFuncs.push_back(TransferFunc::retopFunc(targLoc));
         retopBaseSubReg(fp(), xferFuncs);
      } else if (spHeight.isBottom()) {
         xferFuncs.push_back(TransferFunc::bottomFunc(targLoc));
         bottomBaseSubReg(fp(), xferFuncs);
      } else {
         // Get copied stack slot
         long readSlotHeight = spHeight.height();
         Absloc readLoc(readSlotHeight, 0, NULL);

         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, targLoc));
         copyBaseSubReg(fp(), xferFuncs);
      }
   } else {
      xferFuncs.push_back(TransferFunc::bottomFunc(targLoc));
      bottomBaseSubReg(fp(), xferFuncs);
   }
}

void StackAnalysis::handlePushPopFlags(int sign, TransferFuncs &xferFuncs) {
   // Fixed-size push/pop
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), sign * word_size));
   copyBaseSubReg(sp(), xferFuncs);
}

void StackAnalysis::handlePushPopRegs(int sign, TransferFuncs &xferFuncs) {
   // Fixed-size push/pop
   // 8 registers
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()),
      sign * 8 * word_size));
   copyBaseSubReg(sp(), xferFuncs);
}

void StackAnalysis::handlePowerAddSub(Instruction::Ptr insn, int sign,
   TransferFuncs &xferFuncs) {

   // Add/subtract are op0 = op1 +/- op2; we'd better read the stack pointer as
   // well as writing it
   if (!insn->isRead(theStackPtr) || !insn->isWritten(theStackPtr)) {
      return handleDefault(insn, xferFuncs);
   }
   
   Operand arg = insn->getOperand(2);
   Result res = arg.getValue()->eval();
   Absloc sploc(sp());
   if (res.defined) {
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc,
         sign * res.convert<long>()));
      copyBaseSubReg(sp(), xferFuncs);
      stackanalysis_printf(
         "\t\t\t Stack height changed by evalled add/sub: %lx\n",
         sign * res.convert<long>());
   } else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
      bottomBaseSubReg(sp(), xferFuncs);
      stackanalysis_printf(
         "\t\t\t Stack height changed by unevalled add/sub: bottom\n");
   }
}

void StackAnalysis::handlePowerStoreUpdate(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {

   if (!insn->isWritten(theStackPtr)) {
      return handleDefault(insn, xferFuncs);
   }
   
   std::set<Expression::Ptr> memWriteAddrs;
   insn->getMemoryWriteOperands(memWriteAddrs);
   Expression::Ptr stackWrite = *(memWriteAddrs.begin());
   stackanalysis_printf("\t\t\t ...checking operand %s\n",
      stackWrite->format().c_str());
   stackanalysis_printf("\t\t\t ...binding %s to 0\n",
      theStackPtr->format().c_str());
   stackWrite->bind(theStackPtr.get(), Result(u32, 0));
   Result res = stackWrite->eval();
   Absloc sploc(sp());
   if (res.defined) {
      long delta = res.convert<long>();
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc, delta));
      copyBaseSubReg(sp(), xferFuncs);
      stackanalysis_printf(
         "\t\t\t Stack height changed by evalled stwu: %lx\n", delta);
   } else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
      bottomBaseSubReg(sp(), xferFuncs);
      stackanalysis_printf(
         "\t\t\t Stack height changed by unevalled stwu: bottom\n");
   }
}


void StackAnalysis::handleMov(Instruction::Ptr insn, Block *block,
   const Offset off, TransferFuncs &xferFuncs) {
   // Some cases:
   //   1. mov reg, reg
   //   2. mov imm, reg
   //   3. mov mem, reg
   //   4. mov reg, mem
   //   5. mov imm, mem
   // 
   // #1 Causes register copying.
   // #2 Causes an absolute value in the register.
   // #3 Depends on whether the memory address we're loading from can be
   //    determined statically.
   //    a. If it can, we copy the register to the memory location we're
   //       loading from.
   //    b. Otherwise, we set the register to TOP.  Note that this is safe
   //       since StackMod fails whenever a stack height is written out to an
   //       undetermined (topped) location.
   // #4 Depends on whether the address we're storing to can be determined
   //    statically.
   //    a. If it can, we copy the memory address to the register.
   //    b. Otherwise, we ignore the store.
   // #5 Depends on whether the address we're storing to can be determined
   //    statically.
   //    a. If it can, we give the address an absolute value.
   //    b. Otherwise, we ignore the store.

   Architecture arch = insn->getArch();  // Needed for debug messages

   // Extract operands
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);


   if (insn->writesMemory()) {
      assert(writtenRegs.size() == 0);
      stackanalysis_printf("\t\t\tMemory write to: %s\n",
         operands[0].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[0].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the written memory address
      Absloc writtenLoc;
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         std::pair<Address, bool> resultPair = visitor.getResult();
         if (resultPair.second) {
            // We have a stack slot
            writtenLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            writtenLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            writtenLoc.format().c_str());
      } else {
         // Cases 4b and 5b
         stackanalysis_printf("\t\t\tCan't determine location\n");
         return;
      }

      if (readRegs.size() > 0) {
         // Case 4a
         assert(readRegs.size() == 1);
         Absloc from((*readRegs.begin())->getID());
         xferFuncs.push_back(TransferFunc::copyFunc(from, writtenLoc));
      } else {
         // Case 5a
         Expression::Ptr immExpr = operands[1].getValue();
         assert(typeid(*immExpr) == typeid(Immediate));
         long immVal = immExpr->eval().convert<long>();
         xferFuncs.push_back(TransferFunc::absFunc(writtenLoc, immVal));
      }
      return;
   }


   // Only Cases 1, 2, and 3 can reach this point.
   // As a result, we know there's exactly one written register.
   assert(writtenRegs.size() == 1);
   MachRegister written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   if (insn->readsMemory()) {
      stackanalysis_printf("\t\t\tMemory read from: %s\n",
         operands[1].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the read memory address
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         // Case 3a
         std::pair<Address, bool> resultPair = visitor.getResult();
         Absloc readLoc;
         if (resultPair.second) {
            // We have a stack slot
            readLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            readLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            readLoc.format().c_str());
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // Case 3b
         stackanalysis_printf("\t\t\tCan't determine location\n");
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }


   // Only Cases 1 and 2 can reach this point.
   // As a result, we know there's either 0 or 1 read registers
   MachRegister read;
   if (!readRegs.empty()) {
      assert(readRegs.size() == 1);
      read = (*readRegs.begin())->getID();
   }
   Absloc readLoc(read);


   if (read.isValid()) {
      // Case 1
      stackanalysis_printf("\t\t\tCopy detected: %s -> %s\n",
         read.name().c_str(), written.name().c_str());
      xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
      copyBaseSubReg(written, xferFuncs);
   } else {
      // Case 2
      InstructionAPI::Expression::Ptr readExpr = operands[1].getValue();
      assert(typeid(*readExpr) == typeid(InstructionAPI::Immediate));
      long readValue = readExpr->eval().convert<long>();
      stackanalysis_printf("\t\t\tImmediate to register move: %s set to %ld\n",
         written.name().c_str(), readValue);
      xferFuncs.push_back(TransferFunc::absFunc(writtenLoc, readValue));
      retopBaseSubReg(written, xferFuncs);
   }
}

void StackAnalysis::handleZeroExtend(Instruction::Ptr insn, Block *block,
   const Offset off, TransferFuncs &xferFuncs) {
   // In x86/x86_64, zero extends can't write to memory
   assert(!insn->writesMemory());

   Architecture arch = insn->getArch();  // Needed for debug messages

   // Extract operands
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);

   assert(writtenRegs.size() == 1);
   const MachRegister &written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   // Handle memory loads
   if (insn->readsMemory()) {
      stackanalysis_printf("\t\t\tMemory read from: %s\n",
         operands[1].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the read memory address
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         // We can track the memory location we're loading from
         std::pair<Address, bool> resultPair = visitor.getResult();
         Absloc readLoc;
         if (resultPair.second) {
            // We have a stack slot
            readLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            readLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            readLoc.format().c_str());
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // We can't track this memory location
         stackanalysis_printf("\t\t\tCan't determine location\n");
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }

   assert(readRegs.size() == 1);
   Absloc readLoc((*readRegs.begin())->getID());

   stackanalysis_printf("\t\t\tCopy detected: %s -> %s\n",
      readLoc.format().c_str(), writtenLoc.format().c_str());
   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
   copyBaseSubReg(written, xferFuncs);
}

void StackAnalysis::handleSignExtend(Instruction::Ptr insn, Block *block,
   const Offset off, TransferFuncs &xferFuncs) {
   // This instruction sign extends the read register into the written
   // register. Copying insn't really correct here...sign extension is going
   // to change the value...

   // In x86/x86_64, sign extends can't write to memory
   assert(!insn->writesMemory());

   Architecture arch = insn->getArch();  // Needed for debug messages

   // Extract operands
   std::vector<Operand> operands;
   insn->getOperands(operands);
   assert(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);

   assert(writtenRegs.size() == 1);
   const MachRegister &written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   // Handle memory loads
   if (insn->readsMemory()) {
      stackanalysis_printf("\t\t\tMemory read from: %s\n",
         operands[1].format(arch).c_str());

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      assert(addrExpr.size() == 1);

      // Try to determine the read memory address
      StateEvalVisitor visitor;
      if (intervals_ == NULL) {
         visitor = StateEvalVisitor(off, insn, NULL);
      } else {
         visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
      }
      addrExpr[0]->apply(&visitor);
      if (visitor.isDefined()) {
         // We can track the memory location we're loading from
         std::pair<Address, bool> resultPair = visitor.getResult();
         Absloc readLoc;
         if (resultPair.second) {
            // We have a stack slot
            readLoc = Absloc(resultPair.first, 0, NULL);
         } else {
            // We have a static address
            readLoc = Absloc(resultPair.first);
         }
         stackanalysis_printf("\t\t\tEvaluates to: %s\n",
            readLoc.format().c_str());
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc,
            true));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // We can't track this memory location
         stackanalysis_printf("\t\t\tCan't determine location\n");
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }

   assert(readRegs.size() == 1);
   Absloc readLoc((*readRegs.begin())->getID());

   stackanalysis_printf(
      "\t\t\t Sign extend insn detected: %s -> %s (must be top or bottom)\n",
      readLoc.format().c_str(), writtenLoc.format().c_str());
   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc, true));
   copyBaseSubReg(written, xferFuncs);
}

void StackAnalysis::handleSpecialSignExtend(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   // This instruction sign extends the read register into the written
   // register. Copying insn't really correct here...sign extension is going
   // to change the value...

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   insn->getWriteSet(writtenRegs);
   assert(writtenRegs.size() == 1);
   MachRegister writtenReg = (*writtenRegs.begin())->getID();
   MachRegister readReg;
   if (writtenReg == x86_64::rax) readReg = x86_64::eax;
   else if (writtenReg == x86_64::eax) readReg = x86_64::ax;
   else if (writtenReg == x86_64::ax) readReg = x86_64::al;
   else if (writtenReg == x86::eax) readReg = x86::ax;
   else if (writtenReg == x86::ax) readReg = x86::al;
   else assert(false);

   Absloc writtenLoc(writtenReg);
   Absloc readLoc(readReg);

   stackanalysis_printf(
      "\t\t\t Special sign extend detected: %s -> %s (must be top/bottom)\n",
      readLoc.format().c_str(), writtenLoc.format().c_str());
   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc, true));
   copyBaseSubReg(writtenReg, xferFuncs);
}

void StackAnalysis::handleDefault(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {
   std::set<RegisterAST::Ptr> written;
   insn->getWriteSet(written);
   for (auto iter = written.begin(); iter != written.end(); ++iter) {
      const MachRegister &reg = (*iter)->getID();
      if ((signed int) reg.regClass() == x86::FLAG ||
         (signed int) reg.regClass() == x86_64::FLAG) {
         continue;
      }
      Absloc loc(reg);
      xferFuncs.push_back(TransferFunc::copyFunc(loc, loc, true));
      copyBaseSubReg(reg, xferFuncs);
      stackanalysis_printf(
         "\t\t\t Unhandled insn %s detected: %s set to topBottom\n",
         insn->format().c_str(), (*iter)->getID().name().c_str());
   }
}

bool StackAnalysis::isCall(Instruction::Ptr insn) {
   return insn->getCategory() == c_CallInsn;
}

bool StackAnalysis::handleNormalCall(Instruction::Ptr insn, Block *block,
   Offset off, TransferFuncs &xferFuncs) {

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
      }
      if ((*iter).first.regClass() == gpr) {
         if (callWritten.test((*iter).second)) {
            Absloc loc((*iter).first);
            if (returnRegs.test((*iter).second)) {
               // Bottom
               xferFuncs.push_back(TransferFunc::bottomFunc(loc));
               bottomBaseSubReg(iter->first, xferFuncs);
            } else {
               // Top
               xferFuncs.push_back(TransferFunc::retopFunc(loc));
               retopBaseSubReg(iter->first, xferFuncs);
            }
         }
      }
   }

   const Block::edgelist &outs = block->targets();
   for (auto eit = outs.begin(); eit != outs.end(); ++eit) {
      Edge *cur_edge = (Edge*)*eit;

      Absloc sploc(sp());

      if (cur_edge->type() == DIRECT) {
         // For some reason we're treating this
         // call as a branch. So it shifts the stack
         // like a push (heh) and then we're done.
         stackanalysis_printf(
            "\t\t\t Stack height changed by simulate-jump call\n");
         xferFuncs.push_back(TransferFunc::deltaFunc(sploc, -1 * word_size));
         copyBaseSubReg(sp(), xferFuncs);
         return true;
      }
      
      if (cur_edge->type() != CALL) continue;
      
      Block *target_bbl = cur_edge->trg();
      Function *target_func = target_bbl->obj()->findFuncByEntry(
         target_bbl->region(), target_bbl->start());
      
      if (!target_func) continue;
      
      Height h = getStackCleanAmount(target_func);
      if (h == Height::bottom) {
         stackanalysis_printf(
            "\t\t\t Stack height changed by self-cleaning function: bottom\n");
         xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
         bottomBaseSubReg(sp(), xferFuncs);
      } else {
         stackanalysis_printf(
            "\t\t\t Stack height changed by self-cleaning function: %ld\n",
            h.height());
         xferFuncs.push_back(TransferFunc::deltaFunc(sploc, h.height()));
         copyBaseSubReg(sp(), xferFuncs);
      }
      return true;

   }
   stackanalysis_printf("\t\t\t Stack height assumed unchanged by call\n");
   return true;
}
                                       

bool StackAnalysis::handleThunkCall(Instruction::Ptr insn,
   TransferFuncs &xferFuncs) {

   // We know that we're not a normal call, so it depends on whether the CFT is
   // "next instruction" or not.
   if (insn->getCategory() != c_CallInsn || !insn->getControlFlowTarget()) {
      return false;
   }

   // Eval of getCFT(0) == size?
   Expression::Ptr cft = insn->getControlFlowTarget();
   cft->bind(thePC.get(), Result(u32, 0));
   Result res = cft->eval();
   if (!res.defined) return false;
   if (res.convert<unsigned>() == insn->size()) {
      Absloc sploc(sp());
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc, -1 * word_size));
      copyBaseSubReg(sp(), xferFuncs);
      return true;
   }
   // Else we're calling a mov, ret thunk that has no effect on the stack
   // pointer
   return true;
}


void StackAnalysis::createEntryInput(AbslocState &input) {
   // FIXME for POWER/non-IA32
   // IA32 - the in height includes the return address and therefore
   // is <wordsize>
   // POWER - the in height is 0
#if defined(arch_power)
   input[Absloc(sp())] = Height(0);
#elif (defined(arch_x86) || defined(arch_x86_64))
   input[Absloc(sp())] = Height(-1 * word_size);
   if (sp() == x86_64::rsp) input[Absloc(x86_64::esp)] = Height(-word_size);
#else
   assert(0 && "Unimplemented architecture");
#endif
}

void StackAnalysis::createSummaryEntryInput(TransferSet &input) {
   // Get a set of all input locations
   std::set<Absloc> inputLocs;
   for (auto beIter = blockEffects->begin(); beIter != blockEffects->end();
      beIter++) {
      const SummaryFunc &sf = beIter->second;
      const TransferSet &af = sf.accumFuncs;
      for (auto afIter = af.begin(); afIter != af.end(); afIter++) {
         const TransferFunc &tf = afIter->second;
         if (tf.target.isValid()) {
            inputLocs.insert(tf.target);
         }
         if (tf.from.isValid()) {
            inputLocs.insert(tf.from);
         }
         if (!tf.fromRegs.empty()) {
            for (auto frIter = tf.fromRegs.begin(); frIter != tf.fromRegs.end();
               frIter++) {
               Absloc deploc = frIter->first;
               inputLocs.insert(deploc);
            }
         }
      }
   }

   // Create identity functions for each input location
   for (auto locIter = inputLocs.begin(); locIter != inputLocs.end();
      locIter++) {
      const Absloc &loc = *locIter;
      input[loc] = TransferFunc::identityFunc(loc);
   }
}


StackAnalysis::AbslocState StackAnalysis::getSrcOutputLocs(Edge* e) {
   Block* b = e->src();
   stackanalysis_printf("%lx ", b->lastInsnAddr());
   return blockOutputs[b];
}

StackAnalysis::TransferSet StackAnalysis::getSummarySrcOutputLocs(Edge* e) {
   Block* b = e->src();
   stackanalysis_printf("%lx ", b->lastInsnAddr());
   return blockSummaryOutputs[b];
}

void StackAnalysis::meetInputs(Block *block, AbslocState& blockInput,
   AbslocState &input) {

   input.clear();

   //Intraproc epred; // ignore calls, returns in edge iteration
   //NoSinkPredicate epred2(&epred); // ignore sink node (unresolvable)
   intra_nosink epred2;

   stackanalysis_printf("\t ... In edges: ");
   const Block::edgelist & inEdges = block->sources();
   std::for_each(
      boost::make_filter_iterator(epred2, inEdges.begin(), inEdges.end()),
      boost::make_filter_iterator(epred2, inEdges.end(), inEdges.end()),
      boost::bind(&StackAnalysis::meet, this,
         boost::bind(&StackAnalysis::getSrcOutputLocs, this, _1),
         boost::ref(input)));
   stackanalysis_printf("\n");

   meet(blockInput, input);
}


void StackAnalysis::meetSummaryInputs(Block *block, TransferSet &blockInput,
   TransferSet &input) {

   input.clear();

   //Intraproc epred; // ignore calls, returns in edge iteration
   //NoSinkPredicate epred2(&epred); // ignore sink node (unresolvable)
   intra_nosink epred2;

   stackanalysis_printf("\t ... In edges: ");
   const Block::edgelist & inEdges = block->sources();
   std::for_each(
      boost::make_filter_iterator(epred2, inEdges.begin(), inEdges.end()),
      boost::make_filter_iterator(epred2, inEdges.end(), inEdges.end()),
      boost::bind(&StackAnalysis::meetSummary, this,
         boost::bind(&StackAnalysis::getSummarySrcOutputLocs, this, _1),
         boost::ref(input)));
   stackanalysis_printf("\n");

   meetSummary(blockInput, input);
}

void StackAnalysis::meet(const AbslocState &input, AbslocState &accum) {
   for (AbslocState::const_iterator iter = input.begin();
      iter != input.end(); ++iter) {
      accum[iter->first] = Height::meet(iter->second, accum[iter->first]);
      if (accum[iter->first].isTop()) {
         accum.erase(iter->first);
      }
   }
}


void StackAnalysis::meetSummary(const TransferSet &input, TransferSet &accum) {
   for (auto iter = input.begin(); iter != input.end(); ++iter) {
      const Absloc &loc = iter->first;
      const TransferFunc &inputFunc = iter->second;
      accum[loc] = TransferFunc::meet(inputFunc, accum[loc]);
      if (accum[loc].isTop() && !accum[loc].isRetop()) {
         accum.erase(loc);
      }
   }
}


StackAnalysis::Height &StackAnalysis::Height::operator+=(const Height &other) {
   if (isBottom()) return *this;
   if (other.isBottom()) {
      *this = bottom;
      return *this;
   }
   if (isTop() && other.isTop()) {
      // TOP + TOP = TOP
      *this = top;
      return *this;
   }
   if (isTop() || other.isTop()) {
      // TOP + height = BOTTOM
      *this = bottom;
      return *this;
   }

   height_ += other.height_;
   return *this;
}

StackAnalysis::Height &StackAnalysis::Height::operator+=(
   const signed long &rhs) {
   if (isBottom()) return *this;
   if (isTop()) return *this;

   height_ += rhs;
   return *this;
}

const StackAnalysis::Height StackAnalysis::Height::operator+(const Height &rhs)
   const {
   if (isBottom()) return bottom;
   if (rhs.isBottom()) return rhs;
   if (isTop() && rhs.isTop()) return top;
   if (isTop() || rhs.isTop()) return bottom;
   return Height(height_ + rhs.height_);
}

const StackAnalysis::Height StackAnalysis::Height::operator+(
   const signed long &rhs) const {
   if (isBottom()) return bottom;
   if (isTop()) return top;
   return Height(height_ + rhs);
}

const StackAnalysis::Height StackAnalysis::Height::operator-(const Height &rhs)
   const {
   if (isBottom()) return bottom;
   if (rhs.isBottom()) return rhs;
   if (isTop() && rhs.isTop()) return top;
   if (isTop() || rhs.isTop()) return bottom;
   return Height(height_ - rhs.height_);
}


StackAnalysis::TransferFunc StackAnalysis::TransferFunc::identityFunc(
   Absloc r) {
   return copyFunc(r, r);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::deltaFunc(Absloc r,
   long d) {
   return TransferFunc(uninitialized, d, r, r);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::absFunc(Absloc r,
   long a, bool i) {
   return TransferFunc(a, 0, Absloc(), r, i);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::copyFunc(Absloc f,
   Absloc t, bool i) {
   return TransferFunc (uninitialized, 0, f, t, i);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::bottomFunc(Absloc r) {
   return TransferFunc(notUnique, notUnique, Absloc(), r, false, false, BOTTOM);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::retopFunc(Absloc r) {
   return TransferFunc(uninitialized, 0, Absloc(), r, false, true, TOP);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::sibFunc(
   std::map<Absloc, std::pair<long,bool> > f, long d, Absloc t) {
   return TransferFunc(f, d, t);
}

StackAnalysis::TransferFunc StackAnalysis::TransferFunc::meet(
   const TransferFunc &lhs, const TransferFunc &rhs) {
   // Handle default-constructed TransferFuncs
   if (lhs.isTop() && !lhs.isRetop()) return rhs;
   if (rhs.isTop() && !rhs.isRetop()) return lhs;

   assert(lhs.target == rhs.target);
   if (lhs == rhs) return lhs;

   TransferFunc ret;
   if (lhs.isAbs()) {
      if (rhs.isAbs()) {
         // Since we know lhs != rhs, the abs values must be different
         ret = retopFunc(lhs.target);
      } else if (rhs.isCopy() || rhs.isBottom() || rhs.isRetop() ||
         rhs.isSIB()) {
         ret = rhs;
      } else {
         assert(false);
      }
   } else if (lhs.isCopy()) {
      if (rhs.isAbs()) {
         ret = lhs;
      } else if (rhs.isCopy()) {
         if (lhs.from == rhs.from) {
            // Same base register. Since we know lhs != rhs, either the deltas
            // are different or the topBottom values are different. Either way,
            // we need to return a function that is topBottom.
            ret = lhs;
            ret.topBottom = true;
            ret.delta = 0;
         } else {
            // Different base registers.  Use a SIB function to capture both
            // possible bases.  Note that current SIB function handling doesn't
            // actually add the terms together.
            // FIXME if SIB function handling changes.
            std::map<Absloc, std::pair<long, bool>> fromRegs;
            fromRegs[lhs.from] = std::make_pair(1, true);
            fromRegs[rhs.from] = std::make_pair(1, true);
            ret = sibFunc(fromRegs, 0, lhs.target);
         }
      } else if (rhs.isBottom()) {
         ret = rhs;
      } else if (rhs.isRetop()) {
         ret = lhs;
      } else if (rhs.isSIB()) {
         // Add the source register of the LHS copy to the RHS SIB function.
         // Note that current SIB function handling doesn't actually add the
         // terms together.
         // FIXME if SIB function handling changes.
         ret = rhs;
         ret.fromRegs[lhs.from] = std::make_pair(1, true);
      } else {
         assert(false);
      }
   } else if (lhs.isBottom()) {
      ret = lhs;
   } else if (lhs.isRetop()) {
      if (rhs.isAbs()) {
         ret = lhs;
      } else if (rhs.isCopy() || rhs.isBottom() || rhs.isRetop() ||
         rhs.isSIB()) {
         ret = rhs;
      } else {
         assert(false);
      }
   } else if (lhs.isSIB()) {
      if (rhs.isAbs()) {
         ret = lhs;
      } else if (rhs.isCopy()) {
         // Add the source register of the RHS copy to the LHS SIB function.
         // Note that current SIB function handling doesn't actually add the
         // terms together.
         // FIXME if SIB function handling changes.
         ret = lhs;
         ret.fromRegs[rhs.from] = std::make_pair(1, true);
      } else if (rhs.isBottom()) {
         ret = rhs;
      } else if (rhs.isRetop()) {
         ret = lhs;
      } else if (rhs.isSIB()) {
         if (lhs.fromRegs == rhs.fromRegs) {
            // Same SIB.  Since we know lhs != rhs, either the deltas are
            // different or the topBottom values are different.  Either way, we
            // need to return a function that is topBottom.
            ret = lhs;
            ret.topBottom = true;
            ret.delta = 0;
         } else {
            // Different SIBs.  Combine the terms of the LHS and RHS SIB
            // functions.  Note that current SIB function handling doesn't
            // actually add the terms together.
            // FIXME if SIB function handling changes.
            ret = lhs;
            for (auto iter = rhs.fromRegs.begin(); iter != rhs.fromRegs.end();
               iter++) {
               const Absloc &loc = iter->first;
               const std::pair<long, bool> &val = iter->second;
               auto findLoc = ret.fromRegs.find(loc);
               if (findLoc == ret.fromRegs.end() || (val.first == 1 &&
                  ret.fromRegs[loc].first != 1)) {
                  ret.fromRegs[loc] = val;
               }
            }
         }
      } else {
         assert(false);
      }
   } else {
      assert(false);
   }
   return ret;
}


bool StackAnalysis::TransferFunc::isIdentity() const {
   return isCopy() && from == target && delta == 0 && !topBottom;
}

bool StackAnalysis::TransferFunc::isBottom() const {
   return type_ == BOTTOM;
}

bool StackAnalysis::TransferFunc::isTop() const {
   return type_ == TOP;
}

bool StackAnalysis::TransferFunc::isRetop() const {
   return isTop() && retop;
}

bool StackAnalysis::TransferFunc::isCopy() const {
   return from.isValid();
}

bool StackAnalysis::TransferFunc::isAbs() const {
   return !isTop() && !isBottom() && !isCopy() && !isSIB();
}

bool StackAnalysis::TransferFunc::isDelta() const {
   return delta != 0;
}

bool StackAnalysis::TransferFunc::isSIB() const {
    return (fromRegs.size() > 0);
}


// Destructive update of the input map. Assumes inputs are absolute,
// uninitialized, or bottom; no deltas.
StackAnalysis::Height StackAnalysis::TransferFunc::apply(
   const AbslocState &inputs) const {
   assert(target.isValid());
   // Bottom stomps everything
   if (isBottom()) {
      return Height::bottom;
   }

   AbslocState::const_iterator iter = inputs.find(target);
   Height input;
   if (iter != inputs.end()) {
      input = iter->second;
   } else {
      input = Height::top;
   }

   bool isTopBottomOrig = isTopBottom();

   if (isSIB()) {
      input = Height::top; // SIB overwrites, so start at TOP
      for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
         Absloc curLoc = (*iter).first;
         long curScale = (*iter).second.first;
         bool curTopBottom = (*iter).second.second;
         auto findLoc = inputs.find(curLoc);
         Height locInput;
         if (findLoc == inputs.end()) {
            locInput = Height::top;
         } else {
            locInput = findLoc->second;
         }

         if (locInput == Height::top) {
            // This term doesn't affect our end result, so it can be safely
            // ignored.
         } else if (locInput == Height::bottom) {
            if (curScale == 1) {
               // Must bottom everything only if the scale is 1.  Otherwise,
               // any stack height will be obfuscated.
               input = Height::bottom;
               break;
            }
         } else {
            if (curScale == 1) {
               // If the scale isn't 1, then any stack height is obfuscated,
               // and we can safely ignore the term.
               if (curTopBottom) {
                  input = Height::bottom;
                  break;
               } else {
                  input += locInput; // Matt: Always results in bottom?
               }
            }
         }
      }
   }

   if (isAbs()) {
      // We cannot be a copy, as the absolute removes that.
      assert(!isCopy());
      // Apply the absolute
      // NOTE: an absolute is not a stack height, set input to top
      //input = abs;
      input = Height::top;
   }
   if (isCopy()) {
      // Cannot be absolute
      assert(!isAbs());
      // Copy the input value from whatever we're a copy of.
      AbslocState::const_iterator iter2 = inputs.find(from);
      if (iter2 != inputs.end()) input = iter2->second;
      else input = Height::top;
   }
   if (isDelta()) {
      input += delta;
   }
   if (isRetop()) {
      input = Height::top;
   }
   if (isTopBottomOrig) {
      if (!input.isTop()) {
         input = Height::bottom;
      }
   }
   return input;
}

// Returns accumulated transfer function without modifying inputs
StackAnalysis::TransferFunc StackAnalysis::TransferFunc::summaryAccumulate(
   const TransferSet &inputs) const {
   TransferFunc input;
   auto findTarget = inputs.find(target);
   if (findTarget != inputs.end()) {
      input = findTarget->second;
   }
   if (input.target.isValid()) assert(input.target == target);
   input.target = target; // Default constructed TransferFuncs won't have this
   assert(target.isValid());

   // Bottom stomps everything
   if (isBottom()) {
      input = bottomFunc(target);
      return input;
   }
   bool isTopBottomOrig = isTopBottom();
   if (!input.isTopBottom() && !input.isRetop()) {
      input.topBottom = isTopBottomOrig;
   }
   // Absolutes override everything else
   if (isAbs()) {
      input = absFunc(target, abs, false);
      return input;
   }
   if (isSIB()) {
      // First check for special cases
      bool allAbs = true;
      bool allToppable = true;
      bool anyBottomed = false;
      for (auto iter = fromRegs.begin(); iter != fromRegs.end(); iter++) {
         Absloc fromLocOrig = iter->first;
         long scaleOrig = iter->second.first;
         auto inputEntry = inputs.find(fromLocOrig);
         if (inputEntry == inputs.end()) {
            allAbs = false;
            if (scaleOrig == 1) {
               allToppable = false;
            }
         } else {
            if (inputEntry->second.isBottom() && scaleOrig == 1) {
               anyBottomed = true;
            }
            if (!inputEntry->second.isRetop() && !inputEntry->second.isAbs() &&
               scaleOrig == 1) {
               allToppable = false;
            }
            if (!inputEntry->second.isAbs()) {
               allAbs = false;
            }
         }
      }

      // Now handle special cases
      if (anyBottomed) {
         input = bottomFunc(target);
         return input;
      }
      if (allAbs) {
         long newDelta = delta;
         for (auto iter = fromRegs.begin(); iter != fromRegs.end(); iter++) {
            Absloc fromLocOrig = iter->first;
            long scaleOrig = iter->second.first;
            TransferFunc inputAbsFunc = inputs.find(fromLocOrig)->second;
            newDelta += inputAbsFunc.abs * scaleOrig;
         }
         input = absFunc(target, newDelta);
         return input;
      }
      if (allToppable) {
         input = retopFunc(target);
         return input;
      }

      // Handle default case
      std::map<Absloc, std::pair<long,bool> > newFromRegs;
      bool anyToppedTerms = false;
      for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
         Absloc fromLocOrig = (*iter).first;
         long scaleOrig = (*iter).second.first;

         // Because any term with a scale != 1 is TOP, such terms do not affect
         // the final TOP/BOTTOM/Height value of the LEA and can be ignored.
         // FIXME if we change apply() to include constant propagation
         if (scaleOrig != 1) {
            anyToppedTerms = true;
            continue;
         }

         auto findLoc = inputs.find(fromLocOrig);
         if (findLoc == inputs.end()) {
            // Easy case
            // Only add the term if we're not already tracking that register.
            // FIXME if we change apply() to include constant propagation
            auto found = newFromRegs.find(fromLocOrig);
            if (found == newFromRegs.end()) {
               newFromRegs.insert(make_pair(fromLocOrig,
                  make_pair(scaleOrig, false)));
            }
         } else {
            TransferFunc fromRegFunc = findLoc->second;
            assert(!fromRegFunc.isBottom());  // Should be special case
            if (fromRegFunc.isSIB()) {
               // Replace registers and update scales
               for (auto regIter = fromRegFunc.fromRegs.begin();
                  regIter != fromRegFunc.fromRegs.end(); ++regIter) {
                  Absloc replaceLoc = regIter->first;
                  long replaceScale = regIter->second.first;
                  bool replaceTopBottom = regIter->second.second;
                  long newScale = replaceScale * scaleOrig;

                  // Add to our map only the registers that we aren't already
                  // considering.
                  // FIXME if we change apply() to include constant propagation
                  auto found = newFromRegs.find(replaceLoc);
                  if (found == newFromRegs.end()) {
                     newFromRegs.insert(make_pair(replaceLoc,
                        make_pair(newScale, replaceTopBottom)));
                  }
               }
            }
            if (fromRegFunc.isCopy()) {
               // Replace fromRegOrig with fromRegFunc.from only if we aren't
               // already considering fromRegFunc.from in our map.
               // FIXME if we change apply() to include constant propagation
               auto found = newFromRegs.find(fromRegFunc.from);
               if (found == newFromRegs.end()) {
                  newFromRegs.insert(make_pair(fromRegFunc.from,
                     make_pair(scaleOrig, fromRegFunc.isTopBottom())));
               }
            }
            if (fromRegFunc.isDelta()) {
               if (!fromRegFunc.isCopy() && !fromRegFunc.isSIB() &&
                  !fromRegFunc.isAbs()) {
                  // Add the register back in...
                  // FIXME if we change apply() to include constant propagation
                  auto found = newFromRegs.find(fromLocOrig);
                  if (found == newFromRegs.end()) {
                     newFromRegs.insert(make_pair(fromLocOrig,
                        make_pair(scaleOrig, fromRegFunc.isTopBottom())));
                  }
               }
            }
            if (fromRegFunc.isRetop()) {
               // This is a register that was re-topped due to an instruction
               // in this block.  Thus this term is TOP and doesn't affect the
               // value of the LEA, so we can ignore it.
               // FIXME if we change apply() to include constant propagation
               anyToppedTerms = true;
               continue;
            }
            if (fromRegFunc.isTop()) {
               // This is the default constructed target when the target didn't
               // already exist.  Keep track of this register unless we already
               // are.
               // FIXME if we change apply() to include constant propagation
               auto found = newFromRegs.find(fromLocOrig);
               if (found == newFromRegs.end()) {
                  newFromRegs.insert(make_pair(fromLocOrig,
                     make_pair(scaleOrig, false)));
               }
            }
         }
      }

      if (anyToppedTerms) {
         // If any term is later discovered to be a stack height, we need to
         // bottom the target register since this SIB contains a topped term
         // (we know the topped term isn't a stack height, but we don't know
         // precisely what it is).  We indicate this by setting the
         // topBottom flag on non-topped registers.
         for (auto iter = newFromRegs.begin(); iter != newFromRegs.end();
            iter++) {
            iter->second.second = true;
         }
      }

      input = sibFunc(newFromRegs, 0, target);
      input.topBottom = isTopBottomOrig;
      return input;
   }

   // Copies can be tricky
   // apply copy logic only if registers are different
   if (isCopy()) {
      // We need to record that we want to take the inflow height
      // of a different register. 
      // Don't do an inputs[from] as that creates
      auto iter = inputs.find(from);
      if (iter == inputs.end()) {
         // Copying to something we haven't seen yet; easy
         input = *this;
         if (isTopBottomOrig) {
            input.topBottom = true;
            input.delta = 0;
         }
         return input;
      }

      const TransferFunc &orig = iter->second;

      if (orig.isAbs()) {
         // We reset the height, so we don't care about the inflow height
         assert(!orig.isCopy());
         input = orig;
         input.target = target;
         input.topBottom = false;
         assert(input.target.isValid());

         if (isDelta()) {
            input.delta += delta;
         }
         return input;
      }
      if (orig.isCopy()) {
         assert(!orig.isAbs());
         input = orig;
         input.target = target;
         assert(input.target.isValid());

         if (isDelta()) {
            input.delta += delta;
         }
         if (isTopBottomOrig || orig.isTopBottom()) {
            input.topBottom = true;
            input.delta = 0;
         }
         return input;
      }
      if (orig.isSIB()) {
         input = orig;
         input.target = target;

         if (isDelta()) {
            input.delta += delta;
         }
         if (isTopBottomOrig || orig.isTopBottom()) {
            input.topBottom = true;
            input.delta = 0;
         }
         return input;
      }

      // without bottom we mess up in the default case.
      if (orig.isBottom()) {
         input = bottomFunc(target);
         return input;
      }

      if (orig.isRetop()) {
         input = retopFunc(target);
         return input;
      }

      // Default case: record the copy, zero out everything else, copy over
      // the delta if it's defined.
      input.from = orig.target;
      input.abs = uninitialized;
      assert(!orig.isDelta());
      input.delta = 0;
      input.topBottom = isTopBottomOrig || orig.isTopBottom();
      input.fromRegs.clear();
   }

   assert(!isDelta());

   if (isRetop()) {
      input = *this;
   }

   return input;
}


// Accumulation to the input map. This is intended to create a summary, so we
// create something that can take further input.
void StackAnalysis::TransferFunc::accumulate(TransferSet &inputs) {
   inputs[target] = summaryAccumulate(inputs);
}


void StackAnalysis::SummaryFunc::apply(const AbslocState &in,
   AbslocState &out) const {

   // Copy all the elements we don't have xfer funcs for.
   out = in;

   // Apply in parallel since all summary funcs are from the start of the block
   for (TransferSet::const_iterator iter = accumFuncs.begin();
      iter != accumFuncs.end(); ++iter) {
      assert(iter->first.isValid());
      out[iter->first] = iter->second.apply(in);
      if (out[iter->first].isTop()) {
         out.erase(iter->first);
      }
   }
}


void StackAnalysis::SummaryFunc::accumulate(const TransferSet &in,
   TransferSet &out) const {

   // Copy all the elements we don't have xfer funcs for.
   out = in;

   // Apply in parallel since all summary funcs are from the start of the block
   for (TransferSet::const_iterator iter = accumFuncs.begin();
      iter != accumFuncs.end(); ++iter) {
      assert(iter->first.isValid());
      out[iter->first] = iter->second.summaryAccumulate(in);
      if (out[iter->first].isTop() && !out[iter->first].isRetop()) {
         out.erase(iter->first);
      }
   }
}

void StackAnalysis::SummaryFunc::add(TransferFuncs &xferFuncs) {
   // We need to update our register->xferFunc map
   // with the effects of each of the transferFuncs.
   for (auto iter = xferFuncs.begin(); iter != xferFuncs.end(); ++iter) {
      TransferFunc &func = *iter;
      func.accumulate(accumFuncs);
   }
   validate();
}

void StackAnalysis::SummaryFunc::validate() const {
   for (TransferSet::const_iterator iter = accumFuncs.begin(); 
      iter != accumFuncs.end(); ++iter) {
      const TransferFunc &func = iter->second;
      assert(func.target.isValid());
      if (func.isCopy()) assert(!func.isAbs());
      if (func.isAbs()) assert(!func.isCopy());
      if (func.isBottom()) assert(!func.isCopy());
   }
}

MachRegister StackAnalysis::sp() { 
   return MachRegister::getStackPointer(func->isrc()->getArch());
}

MachRegister StackAnalysis::fp() { 
   return MachRegister::getFramePointer(func->isrc()->getArch());
}

std::string StackAnalysis::format(const AbslocState &input) const {
   std::stringstream ret;
   for (AbslocState::const_iterator iter = input.begin();
      iter != input.end(); ++iter) {
      ret << iter->first.format() << " := " << iter->second.format() << ", ";
   }
   return ret.str();
}

std::string StackAnalysis::format(const TransferSet &input) const {
   std::stringstream ret;
   for (auto iter = input.begin(); iter != input.end(); ++iter) {
      ret << iter->second.format() << ", ";
   }
   return ret.str();
}


// Converts a delta in a Result to a long
long StackAnalysis::extractDelta(Result deltaRes) {
   long delta;
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
   return delta;
}


bool StackAnalysis::getSubReg(const MachRegister &reg, MachRegister &subreg) {
   if (reg == x86_64::rax) subreg = x86_64::eax;
   else if (reg == x86_64::rbx) subreg = x86_64::ebx;
   else if (reg == x86_64::rcx) subreg = x86_64::ecx;
   else if (reg == x86_64::rdx) subreg = x86_64::edx;
   else if (reg == x86_64::rsp) subreg = x86_64::esp;
   else if (reg == x86_64::rbp) subreg = x86_64::ebp;
   else if (reg == x86_64::rsi) subreg = x86_64::esi;
   else if (reg == x86_64::rdi) subreg = x86_64::edi;
   else if (reg == x86_64::r8) subreg = x86_64::r8d;
   else if (reg == x86_64::r9) subreg = x86_64::r9d;
   else if (reg == x86_64::r10) subreg = x86_64::r10d;
   else if (reg == x86_64::r11) subreg = x86_64::r11d;
   else if (reg == x86_64::r12) subreg = x86_64::r12d;
   else if (reg == x86_64::r13) subreg = x86_64::r13d;
   else if (reg == x86_64::r14) subreg = x86_64::r14d;
   else if (reg == x86_64::r15) subreg = x86_64::r15d;
   else return false;
   return true;
}

// If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
// retops the subregister.  If reg is a 4 byte register (eax, ebx, ecx, etc.)
// with an 8 byte base register, retops the base register.  The appropriate
// retop transfer function is added to xferFuncs.
void StackAnalysis::retopBaseSubReg(const MachRegister &reg,
   TransferFuncs &xferFuncs) {
   if (reg.size() == 4 && reg.getBaseRegister().size() == 8) {
      const MachRegister &baseReg = reg.getBaseRegister();
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(baseReg)));
   } else if (reg.size() == 8 && reg.getBaseRegister().size() == 8) {
      MachRegister subreg;
      if (getSubReg(reg, subreg)) {
         xferFuncs.push_back(TransferFunc::retopFunc(Absloc(subreg)));
      }
   }
}

// If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
// copies the subregister into the base register.  If reg is a 4 byte register
// (eax, ebx, ecx, etc.) with an 8 byte base register, copies the base register
// into the subregister. The appropriate copy transfer function is added to
// xferFuncs.
void StackAnalysis::copyBaseSubReg(const MachRegister &reg,
   TransferFuncs &xferFuncs) {
   if (reg.size() == 4 && reg.getBaseRegister().size() == 8) {
      const MachRegister &baseReg = reg.getBaseRegister();
      xferFuncs.push_back(TransferFunc::copyFunc(Absloc(reg), Absloc(baseReg),
         true));
   } else if (reg.size() == 8 && reg.getBaseRegister().size() == 8) {
      MachRegister subreg;
      if (getSubReg(reg, subreg)) {
         xferFuncs.push_back(TransferFunc::copyFunc(Absloc(reg), Absloc(subreg),
            false));
      }
   }
}

// If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
// bottoms the subregister.  If reg is a 4 byte register (eax, ebx, ecx, etc.)
// with an 8 byte base register, bottoms the base register.  The appropriate
// bottom transfer function is added to xferFuncs.
void StackAnalysis::bottomBaseSubReg(const MachRegister &reg,
   TransferFuncs &xferFuncs) {
   if (reg.size() == 4 && reg.getBaseRegister().size() == 8) {
      const MachRegister &baseReg = reg.getBaseRegister();
      xferFuncs.push_back(TransferFunc::bottomFunc(Absloc(baseReg)));
   } else if (reg.size() == 8 && reg.getBaseRegister().size() == 8) {
      MachRegister subreg;
      if (getSubReg(reg, subreg)) {
         xferFuncs.push_back(TransferFunc::bottomFunc(Absloc(subreg)));
      }
   }
}
