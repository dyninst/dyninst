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

#include <boost/bind/bind.hpp>
#include <queue>
#include <stack>
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
#include "common/h/compiler_diagnostics.h"

#include "ABI.h"
#include "Annotatable.h"
#include "debug_dataflow.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace Dyninst::ParseAPI;

const StackAnalysis::Height StackAnalysis::Height::bottom(
   StackAnalysis::Height::notUnique, StackAnalysis::Height::BOTTOM);
const StackAnalysis::Height StackAnalysis::Height::top(
   StackAnalysis::Height::uninitialized, StackAnalysis::Height::TOP);

namespace
{
AnnotationClass<StackAnalysis::Intervals>
        Stack_Anno_Intervals(std::string("Stack_Anno_Intervals"), NULL);
AnnotationClass<StackAnalysis::BlockEffects>
        Stack_Anno_Block_Effects(std::string("Stack_Anno_Block_Effects"), NULL);
AnnotationClass<StackAnalysis::InstructionEffects>
        Stack_Anno_Insn_Effects(std::string("Stack_Anno_Insn_Effects"), NULL);
AnnotationClass<StackAnalysis::CallEffects>
        Stack_Anno_Call_Effects(std::string("Stack_Anno_Call_Effects"), NULL);
}

template class std::list<Dyninst::StackAnalysis::TransferFunc*>;
template class std::map<Dyninst::Absloc, Dyninst::StackAnalysis::Height>;
template class std::vector<Dyninst::InstructionAPI::Instruction::Ptr>;

struct stackanalysis_exception : public std::runtime_error {
    stackanalysis_exception(std::string what) : std::runtime_error(std::string("Stackanalysis failure: ") + what) {}
};

#define STACKANALYSIS_ASSERT(X) \
if(!(X)) { \
    throw(stackanalysis_exception(#X));\
}

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
   genInsnEffects();

   stackanalysis_printf("\tPerforming fixpoint analysis\n");
   fixpoint(true);
   stackanalysis_printf("\tCreating SP interval tree\n");
   summarize();

   func->addAnnotation(intervals_, Stack_Anno_Intervals);

   if (df_debug_stackanalysis_on()) {
      debug();
   }

   stackanalysis_printf("Finished stack analysis for function %s\n",
      func->name().c_str());

   return true;
}


bool StackAnalysis::genInsnEffects() {
   // Check if we've already done this work
   if (blockEffects != NULL && insnEffects != NULL && callEffects != NULL) {
      return true;
   }
   func->getAnnotation(blockEffects, Stack_Anno_Block_Effects);
   func->getAnnotation(insnEffects, Stack_Anno_Insn_Effects);
   func->getAnnotation(callEffects, Stack_Anno_Call_Effects);
   if (blockEffects != NULL && insnEffects != NULL && callEffects != NULL) {
      return true;
   }

   blockEffects = new BlockEffects();
   insnEffects = new InstructionEffects();
   callEffects = new CallEffects();

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
   callEffects->clear();
   blockInputs.clear();
   blockOutputs.clear();

   // Generate final block effects with stack slot tracking
   stackanalysis_printf("\tGenerating final block effects\n");
   summarizeBlocks(true);

   // Annotate insnEffects and blockEffects to avoid rework
   func->addAnnotation(blockEffects, Stack_Anno_Block_Effects);
   func->addAnnotation(insnEffects, Stack_Anno_Insn_Effects);
   func->addAnnotation(callEffects, Stack_Anno_Call_Effects);

   stackanalysis_printf("Finished insn effect generation for function %s\n",
      func->name().c_str());

   return true;
}

typedef std::vector<std::pair<Instruction, Offset> > InsnVec;
static void getInsnInstances(Block *block, InsnVec &insns) {
   Offset off = block->start();
   const unsigned char *ptr = (const unsigned char *)
      block->region()->getPtrToInstruction(off);
   if (ptr == NULL) return;
   InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
   while (off < block->end()) {
      insns.push_back(std::make_pair(d.decode(), off));
      off += insns.back().first.size();
   }
}

struct intra_nosink_nocatch : public ParseAPI::EdgePredicate {
   virtual bool operator()(Edge* e) {
      static Intraproc i;
      static NoSinkPredicate n;
      return i(e) && n(e) && e->type() != CATCH;
   }
};

void add_target(std::queue<Block*>& worklist, Edge* e) {
   worklist.push(e->trg());
}

void add_target_list_exclude(std::queue<Block *> &worklist,
   std::set<Block *> &excludeSet,  Edge *e) {
   Block *b = e->trg();
   if (excludeSet.find(b) == excludeSet.end()) {
      excludeSet.insert(b);
      worklist.push(b);
   }
}

void add_target_exclude(std::stack<Block *> &workstack,
   std::set<Block *> &excludeSet,  Edge *e) {
   Block *b = e->trg();
   if (excludeSet.find(b) == excludeSet.end()) {
      excludeSet.insert(b);
      workstack.push(b);
   }
}

// We want to create a transfer function for the block as a whole. This will
// allow us to perform our fixpoint calculation over blocks (thus, O(B^2))
// rather than instructions (thus, O(I^2)).
void StackAnalysis::summarizeBlocks(bool verbose) {
   intra_nosink_nocatch epred;
   std::set<Block *> doneSet;
   std::stack<Block *> workstack;
   doneSet.insert(func->entry());
   workstack.push(func->entry());

   while (!workstack.empty()) {
      Block *block = workstack.top();
      workstack.pop();

      SummaryFunc &bFunc = (*blockEffects)[block];

      if (verbose) {
         stackanalysis_printf("\t Block starting at 0x%lx: %s\n",
            block->start(), bFunc.format().c_str());
      }

      InsnVec instances;
      getInsnInstances(block, instances);
      for (unsigned j = 0; j < instances.size(); j++) {
         const InstructionAPI::Instruction& insn = instances[j].first;
         const Offset &off = instances[j].second;

         // Fills in insnEffects[off]
         TransferFuncs &xferFuncs = (*insnEffects)[block][off];

         TransferSet funcSummary;
         computeInsnEffects(block, insn, off, xferFuncs, funcSummary);
         bFunc.add(xferFuncs);
         if (!funcSummary.empty()) {
            (*callEffects)[block][off] = funcSummary;
            bFunc.addSummary(funcSummary);
         }

         if (verbose) {
            stackanalysis_printf("\t\t\t At 0x%lx:  %s\n", off,
               bFunc.format().c_str());
         }
      }

      if (verbose) {
         stackanalysis_printf("\t Block summary for 0x%lx: %s\n",
            block->start(), bFunc.format().c_str());
      }

      // Add blocks reachable from this one to the work stack
      boost::lock_guard<Block> g(*block);
      const Block::edgelist &targs = block->targets();
      std::for_each(
         boost::make_filter_iterator(epred, targs.begin(), targs.end()),
         boost::make_filter_iterator(epred, targs.end(), targs.end()),
         boost::bind(add_target_exclude, boost::ref(workstack),
            boost::ref(doneSet), boost::placeholders::_1)
      );
   }
}

void StackAnalysis::fixpoint(bool verbose) {
   intra_nosink_nocatch epred2;
   std::set<Block *> touchedSet;
   std::set<Block *> workSet;
   std::queue<Block *> worklist;
   workSet.insert(func->entry());
   worklist.push(func->entry());

   bool firstBlock = true;
   while (!worklist.empty()) {
      Block *block = worklist.front();
      worklist.pop();
      workSet.erase(block);

      if (verbose) {
         stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n",
            block->start());
      }

      // Step 1: calculate the meet over the heights of all incoming
      // intraprocedural blocks.
      AbslocState input;
      if (firstBlock) {
         createEntryInput(input);
         if (verbose) {
            stackanalysis_printf("\t Primed initial block\n");
         }
      } else {
         if (verbose) {
            stackanalysis_printf("\t Calculating meet with block [%lx-%lx]\n",
               block->start(), block->lastInsnAddr());
         }
         meetInputs(block, blockInputs[block], input);
      }

      if (verbose) {
         stackanalysis_printf("\t New in meet: %s\n", format(input).c_str());
      }

      // Step 2: see if the input has changed. Analyze each block at least once
      if (input == blockInputs[block] &&
         touchedSet.find(block) != touchedSet.end()) {
         // No new work here
         if (verbose) {
            stackanalysis_printf("\t ... equal to current, skipping block\n");
         }
         continue;
      }

      if (verbose) {
         stackanalysis_printf("\t ... inequal to current %s, analyzing block\n",
            format(blockInputs[block]).c_str());
      }

      blockInputs[block] = input;

      // Step 3: calculate our new outs
      (*blockEffects)[block].apply(block, input, blockOutputs[block]);
      if (verbose) {
         stackanalysis_printf("\t ... output from block: %s\n",
            format(blockOutputs[block]).c_str());
      }

      // Step 4: push all children on the worklist.
      boost::lock_guard<Block> g(*block);
      const Block::edgelist &outEdges = block->targets();
      std::for_each(
         boost::make_filter_iterator(epred2, outEdges.begin(), outEdges.end()),
         boost::make_filter_iterator(epred2, outEdges.end(), outEdges.end()),
         boost::bind(add_target_list_exclude, boost::ref(worklist),
            boost::ref(workSet), boost::placeholders::_1)
      );

      firstBlock = false;
      touchedSet.insert(block);
   }
}


namespace {
void getRetAndTailCallBlocks(Function *func, std::set<Block *> &retBlocks) {
   retBlocks.clear();
   intra_nosink_nocatch epred;
   std::set<Block *> doneSet;
   std::stack<Block *> workstack;
   doneSet.insert(func->entry());
   workstack.push(func->entry());

   while (!workstack.empty()) {
      Block *currBlock = workstack.top();
      workstack.pop();

      boost::lock_guard<Block> g(*currBlock);
      const Block::edgelist &targs = currBlock->targets();
      for (auto iter = targs.begin(); iter != targs.end(); iter++) {
         Edge *currEdge = *iter;
         if (currEdge->type() == RET ||
            (currEdge->interproc() && currEdge->type() == DIRECT)) {
            retBlocks.insert(currEdge->src());
         }
      }

      std::for_each(
         boost::make_filter_iterator(epred, targs.begin(), targs.end()),
         boost::make_filter_iterator(epred, targs.end(), targs.end()),
         boost::bind(add_target_exclude, boost::ref(workstack),
            boost::ref(doneSet), boost::placeholders::_1)
      );
   }
}
}  // namespace


// Looks for return edges in the function, following tail calls if necessary.
// Returns true if any return edges are found.
bool StackAnalysis::canGetFunctionSummary() {
   std::set<Block *> retBlocks;
   getRetAndTailCallBlocks(func, retBlocks);
   return !retBlocks.empty();
}


bool StackAnalysis::getFunctionSummary(TransferSet &summary) {
    try {
        genInsnEffects();

        if (!canGetFunctionSummary()) {
            stackanalysis_printf("Cannot generate function summary for %s\n",
                                 func->name().c_str());
            return false;
        }

        STACKANALYSIS_ASSERT(!blockEffects->empty());

        stackanalysis_printf("Generating function summary for %s\n",
                             func->name().c_str());

        if (blockSummaryOutputs.empty()) {
            summaryFixpoint();
        }

        // Join possible values at all return edges
        TransferSet tempSummary;
        std::set<Block *> retBlocks;
        getRetAndTailCallBlocks(func, retBlocks);
        STACKANALYSIS_ASSERT(!retBlocks.empty());
        for (auto iter = retBlocks.begin(); iter != retBlocks.end(); iter++) {
            Block *currBlock = *iter;
            meetSummary(blockSummaryOutputs[currBlock], tempSummary);
        }

        // Remove identity functions for simplicity.  Also remove stack slots, except
        // for stack slots in the caller's frame.  Remove copies from base regs to
        // subregs when the base regs are identities since the link between base reg
        // and subreg will be made in the caller.
        summary.clear();
        for (auto iter = tempSummary.begin(); iter != tempSummary.end(); iter++) {
            const Absloc &loc = iter->first;
            const TransferFunc &tf = iter->second;
            if (!tf.isIdentity() && (tf.target.type() != Absloc::Stack ||
                                     tf.target.off() >= 0)) {
                if (!tf.isBaseRegCopy() && !tf.isBaseRegSIB()) {
                    summary[loc] = tf;
                } else {
                    const MachRegister &baseReg = tf.target.reg().getBaseRegister();
                    const Absloc baseLoc(baseReg);
                    if (tempSummary.find(baseLoc) != tempSummary.end() &&
                        !tempSummary[baseLoc].isIdentity()) {
                        summary[loc] = tf;
                    }
                }
            }
        }

        stackanalysis_printf("Finished function summary for %s:\n%s\n",
                             func->name().c_str(), format(summary).c_str());

        return true;

    }
    catch(...)
    {
        return false;
    }
}


void StackAnalysis::summaryFixpoint() {
   intra_nosink_nocatch epred2;

   std::queue<Block *> worklist;
   worklist.push(func->entry());

   bool firstBlock = true;
   while (!worklist.empty()) {
      Block *block = worklist.front();
      worklist.pop();

      // Step 1: calculate the meet over the heights of all incoming
      // intraprocedural blocks.
      TransferSet input;
      if (firstBlock) {
         createSummaryEntryInput(input);
      } else {
         meetSummaryInputs(block, blockSummaryInputs[block], input);
      }

      // Step 2: see if the input has changed
      if (input == blockSummaryInputs[block] && !firstBlock) {
         // No new work here
         continue;
      }

      blockSummaryInputs[block] = input;

      // Step 3: calculate our new outs
      (*blockEffects)[block].accumulate(input, blockSummaryOutputs[block]);

      // Step 4: push all children on the worklist.
      boost::lock_guard<Block> g(*block);
      const Block::edgelist &outEdges = block->targets();

      std::for_each(
         boost::make_filter_iterator(epred2, outEdges.begin(), outEdges.end()),
         boost::make_filter_iterator(epred2, outEdges.end(), outEdges.end()),
         boost::bind(add_target, boost::ref(worklist), boost::placeholders::_1)
      );

      firstBlock = false;
   }
}


void StackAnalysis::summarize() {
   // Now that we know the actual inputs to each block, we create intervals by
   // replaying the effects of each instruction.
   if (intervals_ != NULL) delete intervals_;
   intervals_ = new Intervals();

   // Map to record definition addresses as they are resolved.
   std::map<Block *, std::map<Absloc, Address> > defAddrs;

   for (auto bit = blockInputs.begin(); bit != blockInputs.end(); ++bit) {
      Block *block = bit->first;
      AbslocState input = bit->second;

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
            DefHeightSet &s = input[iter2->target];
            const Definition &def = s.begin()->def;
            const Height &h = s.begin()->height;
            if (def.type == Definition::DEF && def.block == NULL) {
               // New definition
               STACKANALYSIS_ASSERT(iter2->target == def.origLoc);
               s.makeNewSet(block, off, iter2->target, h);
               defAddrs[block][iter2->target] = off;
            }
            if (h.isTop()) {
               input.erase(iter2->target);
            }
         }

         if (callEffects->find(block) != callEffects->end() &&
            (*callEffects)[block].find(off) != (*callEffects)[block].end()) {
            // We have a function summary to apply
            const TransferSet &summary = (*callEffects)[block][off];
            AbslocState newInput = input;
            for (auto summaryIter = summary.begin();
               summaryIter != summary.end(); summaryIter++) {
               const Absloc &target = summaryIter->first;
               const TransferFunc &tf = summaryIter->second;
               newInput[target] = tf.apply(input);
               DefHeightSet &s = newInput[target];
               const Definition &def = s.begin()->def;
               const Height &h = s.begin()->height;
               if (def.type == Definition::DEF && def.block == NULL) {
                  // New definition
                  STACKANALYSIS_ASSERT(target == def.origLoc);
                  s.makeNewSet(block, off, target, h);
                  defAddrs[block][target] = off;
               }
               if (h.isTop()) {
                  newInput.erase(target);
               }
            }
            input = newInput;
         }
         //stackanalysis_printf("\tSummary %lx: %s\n", off,
         //   format(input).c_str());
      }

      (*intervals_)[block][block->end()] = input;
      //stackanalysis_printf("blockOutputs: %s\n",
      //   format(blockOutputs[block]).c_str());
      STACKANALYSIS_ASSERT(input == blockOutputs[block]);
   }

   // Resolve addresses in all propagated definitions using our map.
   for (auto bIter = intervals_->begin(); bIter != intervals_->end(); bIter++) {
      Block *block = bIter->first;
      for (auto aIter = (*intervals_)[block].begin();
         aIter != (*intervals_)[block].end(); aIter++) {
         //Address addr = aIter->first;
         AbslocState &as = aIter->second;
         for (auto tIter = as.begin(); tIter != as.end(); tIter++) {
            const Absloc &target = tIter->first;
            DefHeightSet &dhSet = tIter->second;
            DefHeightSet dhSetNew;
            for (auto dIter = dhSet.begin(); dIter != dhSet.end(); dIter++) {
               const Definition &def = dIter->def;
               const Height &h = dIter->height;
               if (def.addr == 0 &&
                  defAddrs.find(def.block) != defAddrs.end() &&
                  defAddrs[def.block].find(def.origLoc) !=
                     defAddrs[def.block].end()) {
                  // Update this definition using our map
                  Definition defNew(def.block, defAddrs[def.block][def.origLoc],
                     def.origLoc);
                  dhSetNew.insert(DefHeight(defNew, h));
               } else {
                  dhSetNew.insert(DefHeight(def, h));
               }
            }
            as[target] = dhSetNew;
         }
         //stackanalysis_printf("Final defs %lx: %s\n\n", addr,
         //   format((*intervals_)[block][addr]).c_str());
      }
   }
}

void StackAnalysis::computeInsnEffects(ParseAPI::Block *block,
                                       Instruction insn, const Offset off, TransferFuncs &xferFuncs,
                                       TransferSet &funcSummary) {
   entryID what = insn.getOperation().getID();

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
      if (handleNormalCall(insn, block, off, xferFuncs, funcSummary)) return;
      else if (handleThunkCall(insn, block, off, xferFuncs)) return;
      else return handleDefault(insn, block, off, xferFuncs);
   }

   if (isJump(insn)) {
      handleJump(insn, block, off, xferFuncs, funcSummary);
      return;
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
      case e_pushf:
         sign = -1;
         //FALLTHROUGH
      case e_popfd:
         handlePushPopFlags(sign, xferFuncs);
         break;
      case e_pushal:
         sign = -1;
         handlePushPopRegs(sign, xferFuncs);
         break;
      case e_popaw:
         // This nukes all registers
         handleDefault(insn, block, off, xferFuncs);
         break;
      case power_op_addi:
      case power_op_addic:
         handlePowerAddSub(insn, block, off, sign, xferFuncs);
         break;
      case power_op_stwu:
         handlePowerStoreUpdate(insn, block, off, xferFuncs);
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
      case e_syscall:
         handleSyscall(insn, block, off, xferFuncs);
         break;
      default:
         handleDefault(insn, block, off, xferFuncs);
   }
}

StackAnalysis::Height StackAnalysis::getStackCleanAmount(Function *func_) {
   // Cache previous work...
   if (funcCleanAmounts.find(func_) != funcCleanAmounts.end()) {
      return funcCleanAmounts[func_];
   }

   if (!func_->cleansOwnStack()) {
      funcCleanAmounts[func_] = 0;
      return funcCleanAmounts[func_];
   }

   InstructionDecoder decoder((const unsigned char*) NULL, 0,
      func_->isrc()->getArch());
   unsigned char *cur;

   std::set<Height> returnCleanVals;

   Function::const_blocklist returnBlocks = func_->returnBlocks();
   for (auto rets = returnBlocks.begin(); rets != returnBlocks.end(); ++rets) {
      Block *ret = *rets;
      cur = (unsigned char *) ret->region()->getPtrToInstruction(
         ret->lastInsnAddr());
      Instruction insn = decoder.decode(cur);

      entryID what = insn.getOperation().getID();
      if (what != e_ret_near) continue;

      int val;
      std::vector<Operand> ops;
      insn.getOperands(ops);
      if (ops.size() == 1) {
         val = 0;
      } else {
         Result imm = ops[1].getValue()->eval();
         STACKANALYSIS_ASSERT(imm.defined);
         val = (int) imm.val.s16val;
      }
      returnCleanVals.insert(Height(val));
   }

   Height clean = Height::meet(returnCleanVals);
   if (clean == Height::top) {
      // Non-returning or tail-call exits?
      clean = Height::bottom;
   }
   funcCleanAmounts[func_] = clean;

   return clean;
}

StackAnalysis::StackAnalysis() : func(NULL), blockEffects(NULL),
   insnEffects(NULL), callEffects(NULL), intervals_(NULL), word_size(0) {}
   
StackAnalysis::StackAnalysis(Function *f) : func(f), blockEffects(NULL),
   insnEffects(NULL), callEffects(NULL), intervals_(NULL) {
   word_size = func->isrc()->getAddressWidth();
   theStackPtr = Expression::Ptr(new RegisterAST(MachRegister::getStackPointer(
      func->isrc()->getArch())));
   thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(
      func->isrc()->getArch())));
}

StackAnalysis::StackAnalysis(Function *f, const std::map<Address, Address> &crm,
   const std::map<Address, TransferSet> &fs,
   const std::set<Address> &toppable) :
   func(f), callResolutionMap(crm), functionSummaries(fs),
   toppableFunctions(toppable), blockEffects(NULL), insnEffects(NULL),
   callEffects(NULL), intervals_(NULL) {
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
         STACKANALYSIS_ASSERT(foundType);
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

std::string StackAnalysis::Definition::format() const {
   std::stringstream ret;
   if (type == TOP) {
      ret << "TOP";
   } else if (type == BOTTOM) {
      ret << "BOTTOM";
   } else if (addr != 0) {
      STACKANALYSIS_ASSERT(block != NULL);
      ret << "0x" << std::hex << block->start() << "-0x" << std::hex <<
         block->last() << "(";
      ret << "0x" << std::hex << addr;
      ret << ", " << origLoc.format();
      ret << ")";
   } else if (block != NULL) {
      ret << "0x" << std::hex << block->start() << "-0x" << std::hex <<
         block->last() << "(" << origLoc.format() << ")";
   } else {
      ret << "BAD";
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
   STACKANALYSIS_ASSERT(intervals_);
   for (AbslocState::iterator i = (*intervals_)[b][addr].begin();
      i != (*intervals_)[b][addr].end(); ++i) {
      if (i->second.isTopSet()) continue;

      heights.push_back(std::make_pair(i->first, i->second.getHeightSet()));
   }
}


void StackAnalysis::findDefHeightPairs(Block *b, Address addr,
   std::vector<std::pair<Absloc, DefHeightSet> > &defHeights) {
   if (func == NULL) return;

   if (!intervals_) {
      // Check annotation
      func->getAnnotation(intervals_, Stack_Anno_Intervals);
   }
   if (!intervals_) {
      // Analyze?
      if (!analyze()) return;
   }
   STACKANALYSIS_ASSERT(intervals_);
   for (AbslocState::iterator i = (*intervals_)[b][addr].begin();
      i != (*intervals_)[b][addr].end(); ++i) {
      if (i->second.isTopSet()) continue;

      defHeights.push_back(std::make_pair(i->first, i->second));
   }
}


StackAnalysis::DefHeightSet StackAnalysis::findDefHeight(Block *b, Address addr, Absloc loc) {
   DefHeightSet ret;
   ret.makeTopSet();

   if (func == NULL) return ret;

   if (!intervals_) {
      // Check annotation
      func->getAnnotation(intervals_, Stack_Anno_Intervals);
   }
   if (!intervals_) {
      // Analyze?
      if (!analyze()) return ret;
   }
   STACKANALYSIS_ASSERT(intervals_);

   //(*intervals_)[b].find(addr, state);
   //  ret = (*intervals_)[b][addr][reg];
   Intervals::iterator iter = intervals_->find(b);
   if (iter == intervals_->end()) {
      // How do we return "you stupid idiot"?
      ret.makeBottomSet();
      return ret;
   }

   StateIntervals &sintervals = iter->second;
   if (sintervals.empty()) {
      ret.makeBottomSet();
      return ret;
   }
   // Find the last instruction that is <= addr
   StateIntervals::iterator i = sintervals.lower_bound(addr);
   if ((i == sintervals.end() && !sintervals.empty()) ||
      (i->first != addr && i != sintervals.begin())) {
      i--;
   }
   if (i == sintervals.end()) {
      ret.makeBottomSet();
      return ret;
   }

   ret = i->second[loc];
   return ret;
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
   STACKANALYSIS_ASSERT(intervals_);

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

   ret = i->second[loc].getHeightSet();
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
   StateEvalVisitor(Address addr, Instruction insn,
      StackAnalysis::AbslocState *s) : defined(true), state(s) {
      rip = addr + insn.size();
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
         if (regState == state->end() ||
            regState->second.size() != 1 ||
            regState->second.begin()->height.isTop() ||
            regState->second.begin()->height.isBottom()) {
            defined = false;
         } else {
            results.push_back(make_pair(
               regState->second.begin()->height.height(), true));
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
   std::deque<std::pair<Address, bool> > results;

};

void StackAnalysis::handleXor(Instruction insn, Block *block,
                              const Offset off, TransferFuncs &xferFuncs) {
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2);

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


   if (insn.writesMemory()) {
      // xor mem1, reg2/imm2
      STACKANALYSIS_ASSERT(writtenSet.size() == 0);

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> &addrExpr = children0;
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         STACKANALYSIS_ASSERT(readSet.size() == 1);
         Absloc from((*readSet.begin())->getID());
         std::map<Absloc, std::pair<long, bool> > fromRegs;
         fromRegs[writtenLoc] = std::make_pair(1, true);
         fromRegs[from] = std::make_pair(1, true);
         xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
      } else {
         // xor mem1, imm2
         // Xor with immediate.  Set topBottom on target loc
         Expression::Ptr immExpr = operands[1].getValue();
         STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(immExpr.get()));
         xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
            true));
      }
      return;
   }


   STACKANALYSIS_ASSERT(writtenSet.size() == 1);
   MachRegister written = (*writtenSet.begin())->getID();
   Absloc writtenLoc(written);

   if (insn.readsMemory()) {
      // xor reg1, mem2
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> &addrExpr = children1;
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         std::map<Absloc, std::pair<long, bool> > fromRegs;
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
      STACKANALYSIS_ASSERT(readSet.size() == 1);
      read = (*readSet.begin())->getID();
   }
   Absloc readLoc(read);

   if (read.isValid()) {
      // xor reg1, reg2
      std::map<Absloc, std::pair<long, bool> > fromRegs;
      fromRegs[writtenLoc] = std::make_pair(1, true);
      fromRegs[readLoc] = std::make_pair(1, true);
      xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
      copyBaseSubReg(written, xferFuncs);
   } else {
      // xor reg1, imm1
      InstructionAPI::Expression::Ptr readExpr = operands[1].getValue();
      STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(readExpr.get()));
      xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc, true));
      copyBaseSubReg(written, xferFuncs);
   }
}


void StackAnalysis::handleDiv(Instruction insn,
                              TransferFuncs &xferFuncs) {
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 3);

   Expression::Ptr quotient = operands[1].getValue();
   Expression::Ptr remainder = operands[0].getValue();
   Expression::Ptr divisor = operands[2].getValue();
   STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(quotient.get()));
   STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(remainder.get()));
   STACKANALYSIS_ASSERT(!dynamic_cast<Immediate*>(divisor.get()));

   MachRegister quotientReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(quotient))->getID();
   MachRegister remainderReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(remainder))->getID();

   xferFuncs.push_back(TransferFunc::retopFunc(Absloc(quotientReg)));
   xferFuncs.push_back(TransferFunc::retopFunc(Absloc(remainderReg)));
   retopBaseSubReg(quotientReg, xferFuncs);
   retopBaseSubReg(remainderReg, xferFuncs);
}


void StackAnalysis::handleMul(Instruction insn,
                              TransferFuncs &xferFuncs) {
   // MULs have a few forms:
   //   1. mul reg1, reg2/mem2
   //     -- reg1 = reg1 * reg2/mem2
   //   2. mul reg1, reg2/mem2, imm3
   //     -- reg1 = reg2/mem2 * imm3
   //   3. mul reg1, reg2, reg3/mem3
   //     -- reg1:reg2 = reg2 * reg3/mem3
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2 || operands.size() == 3);

   Expression::Ptr target = operands[0].getValue();
   STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(target.get()));
   MachRegister targetReg = (boost::dynamic_pointer_cast<InstructionAPI::
      RegisterAST>(target))->getID();

   if (operands.size() == 2) {
      // Form 1
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(targetReg)));
      retopBaseSubReg(targetReg, xferFuncs);
   } else {
      Expression::Ptr multiplicand = operands[1].getValue();
      Expression::Ptr multiplier = operands[2].getValue();

      if (dynamic_cast<Immediate*>(multiplier.get())) {
         // Form 2
         STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(multiplicand.get()) ||
            dynamic_cast<Dereference*>(multiplicand.get()));
         long multiplierVal = multiplier->eval().convert<long>();
         if (multiplierVal == 0) {
            xferFuncs.push_back(TransferFunc::absFunc(Absloc(targetReg), 0));
            retopBaseSubReg(targetReg, xferFuncs);
         } else if (multiplierVal == 1) {
            if (dynamic_cast<RegisterAST*>(multiplicand.get())) {
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
         STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(multiplicand.get()));
         STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(multiplier.get()) ||
            dynamic_cast<Dereference*>(multiplier.get()));
         MachRegister multiplicandReg = boost::dynamic_pointer_cast<
            RegisterAST>(multiplicand)->getID();
         xferFuncs.push_back(TransferFunc::retopFunc(Absloc(targetReg)));
         xferFuncs.push_back(TransferFunc::retopFunc(Absloc(multiplicandReg)));
         retopBaseSubReg(targetReg, xferFuncs);
         retopBaseSubReg(multiplicandReg, xferFuncs);
      }
   }
}


void StackAnalysis::handlePushPop(Instruction insn, Block *block,
                                  const Offset off, int sign, TransferFuncs &xferFuncs) {
   long delta = 0;
   Operand arg = insn.getOperand(0);
   // Why was this here? bernat, 12JAN11
   if (arg.getValue()->eval().defined) {
      delta = sign * word_size;
   } else {
      delta = sign * arg.getValue()->size();
   }
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), delta));
   copyBaseSubReg(sp(), xferFuncs);

   if (insn.getOperation().getID() == e_push && insn.writesMemory()) {
      // This is a push.  Let's record the value pushed on the stack if
      // possible.
      if (intervals_ != NULL) {
         Absloc sploc(sp());
         const DefHeightSet &spSet = (*intervals_)[block][off][sploc];
         const Height &spHeight = spSet.getHeightSet();
         if (!spHeight.isTop() && !spHeight.isBottom()) {
            // Get written stack slot
            long writtenSlotHeight = spHeight.height() - word_size;
            Absloc writtenLoc(writtenSlotHeight, 0, NULL);

            Expression::Ptr readExpr = insn.getOperand(0).getValue();
            if (dynamic_cast<RegisterAST*>(readExpr.get())) {
               // Get copied register
               MachRegister readReg = boost::dynamic_pointer_cast<RegisterAST>(
                  readExpr)->getID();
               Absloc readLoc(readReg);
               xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
            } else if (dynamic_cast<Immediate*>(readExpr.get())) {
               // Get pushed immediate
               long immVal = readExpr->eval().convert<long>();
               xferFuncs.push_back(TransferFunc::absFunc(writtenLoc, immVal));
            } else if (dynamic_cast<Dereference *>(readExpr.get())) {
               // Extract the read address expression
               std::vector<Expression::Ptr> addrExpr;
               readExpr->getChildren(addrExpr);
               STACKANALYSIS_ASSERT(addrExpr.size() == 1);

               // Try to determine the read memory address
               StateEvalVisitor visitor;
               if (intervals_ == NULL) {
                  visitor = StateEvalVisitor(off, insn, NULL);
               } else {
                  visitor = StateEvalVisitor(off, insn,
                     &(*intervals_)[block][off]);
               }
               addrExpr[0]->apply(&visitor);
               if (visitor.isDefined()) {
                  Absloc readLoc;
                  std::pair<Address, bool> resultPair = visitor.getResult();
                  if (resultPair.second) {
                     // We have a stack slot
                     readLoc = Absloc(resultPair.first, 0, NULL);
                  } else {
                     // We have a static address
                     readLoc = Absloc(resultPair.first);
                  }
                  xferFuncs.push_back(TransferFunc::copyFunc(readLoc,
                     writtenLoc));
               } else {
                  // Unknown read address.  Assume top.
                  xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
               }
            } else {
               STACKANALYSIS_ASSERT(false);
            }
         }
      }
   } else if (insn.getOperation().getID() == e_pop && !insn.writesMemory()) {
      // This is a pop.  Let's record the value popped if possible.

      // Get target register
      Expression::Ptr targExpr = insn.getOperand(0).getValue();
      STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(targExpr.get()));
      MachRegister targReg = boost::dynamic_pointer_cast<RegisterAST>(
         targExpr)->getID();
      Absloc targLoc(targReg);

      if (intervals_ != NULL) {
         Absloc sploc(sp());
         const DefHeightSet &spSet = (*intervals_)[block][off][sploc];
         const Height &spHeight = spSet.getHeightSet();
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
      STACKANALYSIS_ASSERT(false);
   }
}

void StackAnalysis::handleReturn(Instruction insn,
                                 TransferFuncs &xferFuncs) {
   long delta = 0;
   std::vector<Operand> operands;
   insn.getOperands(operands);
   if (operands.size() < 2) {
      delta = word_size;
   } else {
      STACKANALYSIS_ASSERT(operands.size() == 2);
      Result imm = operands[1].getValue()->eval();
      STACKANALYSIS_ASSERT(imm.defined);
      delta = word_size + imm.convert<long>();
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
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), delta));
   copyBaseSubReg(sp(), xferFuncs);
}


void StackAnalysis::handleAddSub(Instruction insn, Block *block,
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

   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2);

   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writeSet;
   operands[1].getReadSet(readSet);
   operands[0].getWriteSet(writeSet);

   if (insn.writesMemory()) {
      // Cases 3 and 5
      STACKANALYSIS_ASSERT(writeSet.size() == 0);

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[0].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         // Cases 3b and 5b
         return;
      }

      if (readSet.size() > 0) {
         // Case 3a
         STACKANALYSIS_ASSERT(readSet.size() == 1);
         const MachRegister &srcReg = (*readSet.begin())->getID();
         if ((signed int) srcReg.regClass() == x86::XMM ||
            (signed int) srcReg.regClass() == x86_64::XMM) {
            // Assume XMM registers only contain FP values, not pointers
            xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
               true));
         } else {
            std::map<Absloc, std::pair<long, bool> > terms;
            Absloc src(srcReg);
            Absloc &dest = writtenLoc;
            terms[src] = make_pair(sign, false);
            terms[dest] = make_pair(1, false);
            xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, dest));
         }
      } else {
         // Case 5a
         Expression::Ptr immExpr = operands[1].getValue();
         STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(immExpr.get()));
         long immVal = immExpr->eval().convert<long>();
         xferFuncs.push_back(TransferFunc::deltaFunc(writtenLoc,
            sign * immVal));
      }
      return;
   }

   // Cases 1, 2, and 4
   STACKANALYSIS_ASSERT(writeSet.size() == 1);
   const MachRegister &written = (*writeSet.begin())->getID();
   Absloc writtenLoc(written);

   if ((signed int) written.regClass() == x86::XMM ||
      (signed int) written.regClass() == x86_64::XMM) {
      // Assume XMM registers only contain FP values, not pointers
      xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
      return;
   }

   if (insn.readsMemory()) {
      // Case 2
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         std::map<Absloc, std::pair<long, bool> > terms;
         terms[readLoc] = make_pair(sign, false);
         terms[writtenLoc] = make_pair(1, false);
         xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // Case 2b
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
      xferFuncs.push_back(TransferFunc::deltaFunc(writtenLoc, delta));
      copyBaseSubReg(written, xferFuncs);
   } else {
      // Case 1
      const MachRegister &srcReg = (*readSet.begin())->getID();
      if ((signed int) srcReg.regClass() == x86::XMM ||
         (signed int) srcReg.regClass() == x86_64::XMM) {
         // Assume XMM registers only contain FP values, not pointers
         xferFuncs.push_back(TransferFunc::copyFunc(writtenLoc, writtenLoc,
            true));
      } else {
         std::map<Absloc, std::pair<long, bool> > terms;
         Absloc src(srcReg);
         Absloc &dest = writtenLoc;
         terms[src] = make_pair(sign, false);
         terms[dest] = make_pair(1, false);
         xferFuncs.push_back(TransferFunc::sibFunc(terms, 0, dest));
      }
      copyBaseSubReg(written, xferFuncs);
   }
}

void StackAnalysis::handleLEA(Instruction insn,
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
   //            or
   //   op1: imm
   //
   std::set<RegisterAST::Ptr> readSet;
   std::set<RegisterAST::Ptr> writtenSet;
   insn.getOperand(0).getWriteSet(writtenSet);
   insn.getOperand(1).getReadSet(readSet);
   STACKANALYSIS_ASSERT(writtenSet.size() == 1);
   STACKANALYSIS_ASSERT(readSet.size() == 0 || readSet.size() == 1 || readSet.size() == 2);
   MachRegister written = (*writtenSet.begin())->getID();
   Absloc writeloc(written);

   InstructionAPI::Operand srcOperand = insn.getOperand(1);
   InstructionAPI::Expression::Ptr srcExpr = srcOperand.getValue();
   std::vector<InstructionAPI::Expression::Ptr> children;
   srcExpr->getChildren(children);

   if (readSet.size() == 0) {
      // op1: imm
      STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(srcExpr.get()));
      long immVal = srcExpr->eval().convert<long>();
      xferFuncs.push_back(TransferFunc::absFunc(writeloc, immVal));
      retopBaseSubReg(written, xferFuncs);
   } else if (readSet.size() == 1) {
      InstructionAPI::Expression::Ptr regExpr, scaleExpr, deltaExpr;
      bool foundScale = false;
      bool foundDelta = false;

      if (children.size() == 2) {
         if (dynamic_cast<Immediate*>(children[0].get())) {
            // op1: imm + reg * imm
            deltaExpr = children[0];
            Expression::Ptr scaleIndexExpr = children[1];
            STACKANALYSIS_ASSERT(dynamic_cast<BinaryFunction*>(scaleIndexExpr.get()));
            children.clear();
            scaleIndexExpr->getChildren(children);

            regExpr = children[0];
            scaleExpr = children[1];
            STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(regExpr.get()));
            STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(scaleExpr.get()));
            foundScale = true;
            foundDelta = true;
         } else if (dynamic_cast<RegisterAST*>(children[0].get())) {
            // op1: reg + imm
            regExpr = children[0];
            deltaExpr = children[1];
            STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(regExpr.get()));
            STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(deltaExpr.get()));
            foundDelta = true;
         } else {
            STACKANALYSIS_ASSERT(false);
         }
      } else if (children.size() == 0) {
         // op1: reg
         regExpr = srcExpr;
         STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(regExpr.get()));
      } else {
         STACKANALYSIS_ASSERT(false);
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

      STACKANALYSIS_ASSERT(children.size() == 2);
      if (dynamic_cast<Immediate*>(children[1].get())) {
         // op1: reg + reg * imm + imm
         // Extract the delta and continue on to get base, index, and scale
         deltaExpr = children[1];
         Expression::Ptr sibExpr = children[0];
         STACKANALYSIS_ASSERT(dynamic_cast<BinaryFunction*>(sibExpr.get()));
         children.clear();
         sibExpr->getChildren(children);
         STACKANALYSIS_ASSERT(children.size() == 2);
         foundDelta = true;
      }

      // op1: reg + reg * imm
      baseExpr = children[0];
      Expression::Ptr scaleIndexExpr = children[1];
      STACKANALYSIS_ASSERT(dynamic_cast<BinaryFunction*>(scaleIndexExpr.get()));

      // Extract the index and scale
      children.clear();
      scaleIndexExpr->getChildren(children);
      STACKANALYSIS_ASSERT(children.size() == 2);
      indexExpr = children[0];
      scaleExpr = children[1];

      STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(baseExpr.get()));
      STACKANALYSIS_ASSERT(dynamic_cast<RegisterAST*>(indexExpr.get()));
      STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(scaleExpr.get()));

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

      STACKANALYSIS_ASSERT(base.isValid() && index.isValid() && scale != -1);

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
      STACKANALYSIS_ASSERT(false);
   }
}

void StackAnalysis::handleLeave(Block *block, const Offset off,
   TransferFuncs &xferFuncs) {
   // This is... mov esp, ebp; pop ebp.
   // Handle it as such.

   // mov esp, ebp;
   xferFuncs.push_back(TransferFunc::copyFunc(Absloc(fp()), Absloc(sp())));
   copyBaseSubReg(fp(), xferFuncs);

   // pop ebp: copy value from stack to ebp
   Absloc targLoc(fp());
   if (intervals_ != NULL) {
      // Note that the stack pointer after the copy recorded above is now the
      // same as the frame pointer at the start of this instruction.  Thus, we
      // use the height of the frame pointer at the start of this instruction to
      // track the memory location read by the pop.
      Absloc sploc(fp());
      const DefHeightSet &spSet = (*intervals_)[block][off][sploc];
      const Height &spHeight = spSet.getHeightSet();
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

   // pop ebp: adjust stack pointer
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), word_size));
   copyBaseSubReg(sp(), xferFuncs);
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

void StackAnalysis::handlePowerAddSub(Instruction insn, Block *block,
                                      const Offset off, int sign, TransferFuncs &xferFuncs) {
   // Add/subtract are op0 = op1 +/- op2; we'd better read the stack pointer as
   // well as writing it
   if (!insn.isRead(theStackPtr) || !insn.isWritten(theStackPtr)) {
      return handleDefault(insn, block, off, xferFuncs);
   }
   
   Operand arg = insn.getOperand(2);
   Result res = arg.getValue()->eval();
   Absloc sploc(sp());
   if (res.defined) {
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc,
         sign * res.convert<long>()));
      copyBaseSubReg(sp(), xferFuncs);
   } else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
      bottomBaseSubReg(sp(), xferFuncs);
   }
}

void StackAnalysis::handlePowerStoreUpdate(Instruction insn, Block *block,
                                           const Offset off, TransferFuncs &xferFuncs) {
   if (!insn.isWritten(theStackPtr)) {
      return handleDefault(insn, block, off, xferFuncs);
   }
   
   std::set<Expression::Ptr> memWriteAddrs;
   insn.getMemoryWriteOperands(memWriteAddrs);
   Expression::Ptr stackWrite = *(memWriteAddrs.begin());
   stackWrite->bind(theStackPtr.get(), Result(u32, 0));
   Result res = stackWrite->eval();
   Absloc sploc(sp());
   if (res.defined) {
      long delta = res.convert<long>();
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc, delta));
      copyBaseSubReg(sp(), xferFuncs);
   } else {
      xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
      bottomBaseSubReg(sp(), xferFuncs);
   }
}


void StackAnalysis::handleMov(Instruction insn, Block *block,
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

   // Extract operands
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);


   if (insn.writesMemory()) {
      STACKANALYSIS_ASSERT(writtenRegs.size() == 0);

      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[0].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         // Cases 4b and 5b
         return;
      }

      if (readRegs.size() > 0) {
         // Case 4a
         STACKANALYSIS_ASSERT(readRegs.size() == 1);
         const MachRegister &reg = (*readRegs.begin())->getID();
         if ((signed int) reg.regClass() == x86::XMM ||
            (signed int) reg.regClass() == x86_64::XMM) {
            // Assume XMM registers only contain FP values, not pointers
            xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         } else {
            Absloc from(reg);
            xferFuncs.push_back(TransferFunc::copyFunc(from, writtenLoc));
         }
      } else {
         // Case 5a
         Expression::Ptr immExpr = operands[1].getValue();
         STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(immExpr.get()));
         long immVal = immExpr->eval().convert<long>();
         xferFuncs.push_back(TransferFunc::absFunc(writtenLoc, immVal));
      }
      return;
   }


   // Only Cases 1, 2, and 3 can reach this point.
   // As a result, we know there's exactly one written register.
   STACKANALYSIS_ASSERT(writtenRegs.size() == 1);
   const MachRegister &written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   if ((signed int) written.regClass() == x86::XMM ||
      (signed int) written.regClass() == x86_64::XMM) {
      // Assume XMM registers only contain FP values, not pointers
      xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
      return;
   }

   if (insn.readsMemory()) {
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // Case 3b
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }


   // Only Cases 1 and 2 can reach this point.
   // As a result, we know there's either 0 or 1 read registers
   MachRegister read;
   if (!readRegs.empty()) {
      STACKANALYSIS_ASSERT(readRegs.size() == 1);
      read = (*readRegs.begin())->getID();
   }
   Absloc readLoc(read);


   if (read.isValid()) {
      // Case 1
      if ((signed int) read.regClass() == x86::XMM ||
         (signed int) read.regClass() == x86_64::XMM) {
         // Assume XMM registers only contain FP values, not pointers
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
      } else {
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
      }
      copyBaseSubReg(written, xferFuncs);
   } else {
      // Case 2
      InstructionAPI::Expression::Ptr readExpr = operands[1].getValue();
      STACKANALYSIS_ASSERT(dynamic_cast<Immediate*>(readExpr.get()));
      long readValue = readExpr->eval().convert<long>();
      xferFuncs.push_back(TransferFunc::absFunc(writtenLoc, readValue));
      retopBaseSubReg(written, xferFuncs);
   }
}

void StackAnalysis::handleZeroExtend(Instruction insn, Block *block,
                                     const Offset off, TransferFuncs &xferFuncs) {
   // In x86/x86_64, zero extends can't write to memory
   STACKANALYSIS_ASSERT(!insn.writesMemory());

   // Extract operands
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);

   STACKANALYSIS_ASSERT(writtenRegs.size() == 1);
   const MachRegister &written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   // Handle memory loads
   if (insn.readsMemory()) {
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // We can't track this memory location
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }

   STACKANALYSIS_ASSERT(readRegs.size() == 1);
   Absloc readLoc((*readRegs.begin())->getID());

   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc));
   copyBaseSubReg(written, xferFuncs);
}

void StackAnalysis::handleSignExtend(Instruction insn, Block *block,
                                     const Offset off, TransferFuncs &xferFuncs) {
   // This instruction sign extends the read register into the written
   // register. Copying insn't really correct here...sign extension is going
   // to change the value...

   // In x86/x86_64, sign extends can't write to memory
   STACKANALYSIS_ASSERT(!insn.writesMemory());

   // Extract operands
   std::vector<Operand> operands;
   insn.getOperands(operands);
   STACKANALYSIS_ASSERT(operands.size() == 2);

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   operands[0].getWriteSet(writtenRegs);
   operands[1].getReadSet(readRegs);

   STACKANALYSIS_ASSERT(writtenRegs.size() == 1);
   const MachRegister &written = (*writtenRegs.begin())->getID();
   Absloc writtenLoc(written);

   // Handle memory loads
   if (insn.readsMemory()) {
      // Extract the expression inside the dereference
      std::vector<Expression::Ptr> addrExpr;
      operands[1].getValue()->getChildren(addrExpr);
      STACKANALYSIS_ASSERT(addrExpr.size() == 1);

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
         xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc,
            true));
         copyBaseSubReg(written, xferFuncs);
      } else {
         // We can't track this memory location
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         retopBaseSubReg(written, xferFuncs);
      }
      return;
   }

   STACKANALYSIS_ASSERT(readRegs.size() == 1);
   Absloc readLoc((*readRegs.begin())->getID());

   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc, true));
   copyBaseSubReg(written, xferFuncs);
}

void StackAnalysis::handleSpecialSignExtend(Instruction insn,
                                            TransferFuncs &xferFuncs) {
   // This instruction sign extends the read register into the written
   // register. Copying insn't really correct here...sign extension is going
   // to change the value...

   // Extract written/read register sets
   std::set<RegisterAST::Ptr> writtenRegs;
   insn.getWriteSet(writtenRegs);
   STACKANALYSIS_ASSERT(writtenRegs.size() == 1);
   MachRegister writtenReg = (*writtenRegs.begin())->getID();
   MachRegister readReg;
   if (writtenReg == x86_64::rax) readReg = x86_64::eax;
   else if (writtenReg == x86_64::eax) readReg = x86_64::ax;
   else if (writtenReg == x86_64::ax) readReg = x86_64::al;
   else if (writtenReg == x86::eax) readReg = x86::ax;
   else if (writtenReg == x86::ax) readReg = x86::al;
   else STACKANALYSIS_ASSERT(false);

   Absloc writtenLoc(writtenReg);
   Absloc readLoc(readReg);

   xferFuncs.push_back(TransferFunc::copyFunc(readLoc, writtenLoc, true));
   copyBaseSubReg(writtenReg, xferFuncs);
}

void StackAnalysis::handleSyscall(Instruction insn, Block *block,
                                  const Offset off, TransferFuncs &xferFuncs) {
   Architecture arch = insn.getArch();
   if (arch == Arch_x86) {
      // x86 returns an error code in EAX
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(x86::eax)));
   } else if (arch == Arch_x86_64) {
      // x86_64 returns an error code in RAX and destroys RCX and R11
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(x86_64::rax)));
      retopBaseSubReg(x86_64::rax, xferFuncs);
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(x86_64::rcx)));
      retopBaseSubReg(x86_64::rcx, xferFuncs);
      xferFuncs.push_back(TransferFunc::retopFunc(Absloc(x86_64::r11)));
      retopBaseSubReg(x86_64::r11, xferFuncs);
   } else {
      handleDefault(insn, block, off, xferFuncs);
   }
}

// Handle instructions for which we have no special handling implemented.  Be
// conservative for safety.
void StackAnalysis::handleDefault(Instruction insn, Block *block,
                                  const Offset off, TransferFuncs &xferFuncs) {
   // Form sets of read/written Abslocs
   std::set<RegisterAST::Ptr> writtenRegs;
   std::set<RegisterAST::Ptr> readRegs;
   insn.getWriteSet(writtenRegs);
   insn.getReadSet(readRegs);
   std::set<Absloc> writtenLocs;
   std::set<Absloc> readLocs;
   for (auto iter = writtenRegs.begin(); iter != writtenRegs.end(); iter++) {
      const MachRegister &reg = (*iter)->getID();
      if ((signed int) reg.regClass() == x86::GPR ||
         (signed int) reg.regClass() ==  x86_64::GPR) {
         writtenLocs.insert(Absloc(reg));
      }
   }
   for (auto iter = readRegs.begin(); iter != readRegs.end(); iter++) {
      const MachRegister &reg = (*iter)->getID();
      if ((signed int) reg.regClass() == x86::GPR ||
         (signed int) reg.regClass() ==  x86_64::GPR) {
         readLocs.insert(Absloc(reg));
      }
   }
   if (insn.readsMemory()) {
      // Add any determinable read locations to readLocs
      std::set<Expression::Ptr> memExprs;
      insn.getMemoryReadOperands(memExprs);
      for (auto iter = memExprs.begin(); iter != memExprs.end(); iter++) {
         const Expression::Ptr &memExpr = *iter;
         StateEvalVisitor visitor;
         if (intervals_ == NULL) {
            visitor = StateEvalVisitor(off, insn, NULL);
         } else {
            visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
         }
         memExpr->apply(&visitor);
         if (visitor.isDefined()) {
            // Read location is determinable
            std::pair<Address, bool> resultPair = visitor.getResult();
            Absloc readLoc;
            if (resultPair.second) {
               // Read from stack slot
               readLoc = Absloc(resultPair.first, 0, NULL);
            } else {
               // Read from static address
               readLoc = Absloc(resultPair.first);
            }
            readLocs.insert(readLoc);
         }
      }
   }
   if (insn.writesMemory()) {
      // Add any determinable written locations to writtenLocs
      std::set<Expression::Ptr> memExprs;
      insn.getMemoryWriteOperands(memExprs);
      for (auto iter = memExprs.begin(); iter != memExprs.end(); iter++) {
         const Expression::Ptr &memExpr = *iter;
         StateEvalVisitor visitor;
         if (intervals_ == NULL) {
            visitor = StateEvalVisitor(off, insn, NULL);
         } else {
            visitor = StateEvalVisitor(off, insn, &(*intervals_)[block][off]);
         }
         memExpr->apply(&visitor);
         if (visitor.isDefined()) {
            // Written location is determinable
            std::pair<Address, bool> resultPair = visitor.getResult();
            Absloc writtenLoc;
            if (resultPair.second) {
               // Write to stack slot
               writtenLoc = Absloc(resultPair.first, 0, NULL);
            } else {
               // Write to static address
               writtenLoc = Absloc(resultPair.first);
            }
            writtenLocs.insert(writtenLoc);
         }
      }
   }

   // Now that we have a complete set of read/written Abslocs, we assign the
   // written Abslocs to be a conservative combination of the read Abslocs.
   for (auto wIter = writtenLocs.begin(); wIter != writtenLocs.end(); wIter++) {
      const Absloc &writtenLoc = *wIter;
      if (readLocs.empty()) {
         // We can get here in two situations: (1) no locations are read, or (2)
         // only non-GPRs and undeterminable memory locations are read.  In
         // either case, we assume the written value is not a stack height.
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         if (writtenLoc.type() == Absloc::Register) {
            retopBaseSubReg(writtenLoc.reg(), xferFuncs);
         }
         continue;
      }
      if (writtenLoc.type() == Absloc::Register &&
         writtenLoc.reg().size() < 4) {
         // Assume registers smaller than 4 bytes are not used to hold pointers
         xferFuncs.push_back(TransferFunc::retopFunc(writtenLoc));
         continue;
      }
      std::map<Absloc, std::pair<long, bool> > fromRegs;
      for (auto rIter = readLocs.begin(); rIter != readLocs.end(); rIter++) {
         const Absloc &readLoc = *rIter;
         fromRegs[readLoc] = std::make_pair(1, true);
      }
      xferFuncs.push_back(TransferFunc::sibFunc(fromRegs, 0, writtenLoc));
      if (writtenLoc.type() == Absloc::Register) {
         copyBaseSubReg(writtenLoc.reg(), xferFuncs);
      }
   }
}

bool StackAnalysis::isCall(Instruction insn) {
   return insn.getCategory() == c_CallInsn;
}

bool StackAnalysis::isJump(Instruction insn) {
   return insn.getCategory() == c_BranchInsn;
}

bool StackAnalysis::handleNormalCall(Instruction insn, Block *block,
                                     Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary) {

   // Identify syscalls of the form: call *%gs:0x10
   Expression::Ptr callAddrExpr = insn.getOperand(0).getValue();
   if (dynamic_cast<Dereference *>(callAddrExpr.get())) {
      std::vector<Expression::Ptr> children;
      callAddrExpr->getChildren(children);
      if (children.size() == 1 &&
         dynamic_cast<Immediate *>(children[0].get()) &&
         children[0]->eval().convert<long>() == 16) {
         // We have a syscall
         handleSyscall(insn, block, off, xferFuncs);
         return true;
      }
   }

   if (!insn.getControlFlowTarget()) return false;

   // Must be a thunk based on parsing.
   if (off != block->lastInsnAddr()) return false;

   Address calledAddr = 0;
   boost::lock_guard<Block> g(*block);
   const Block::edgelist &outs = block->targets();
   for (auto eit = outs.begin(); eit != outs.end(); eit++) {
      Edge *edge = *eit;
      if (edge->type() != CALL) continue;

      if (callResolutionMap.find(off) != callResolutionMap.end()) {
         // This call has already been resolved with PLT info available, so we
         // should use that resolution information.
         calledAddr = callResolutionMap[off];
      } else {
         // This call has not been resolved yet.
         Block *calledBlock = edge->trg();
         calledAddr = calledBlock->start();
      }
      if (intervals_ != NULL &&
         functionSummaries.find(calledAddr) != functionSummaries.end()) {
         stackanalysis_printf("\t\t\tFound function summary for %lx\n",
            calledAddr);
         xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), -word_size));
         copyBaseSubReg(sp(), xferFuncs);

         // Update stack slots in the summary to line up with this stack frame,
         // and then add the modified transfer functions to xferFuncs.
         Absloc sploc(sp());
         const DefHeightSet &spSet = (*intervals_)[block][off][sploc];
         const Height &spHeight = spSet.getHeightSet();
         const TransferSet &fs = functionSummaries[calledAddr];
         for (auto fsIter = fs.begin(); fsIter != fs.end(); fsIter++) {
            Absloc summaryLoc = fsIter->first;
            TransferFunc tf = fsIter->second;
            if (tf.from.type() == Absloc::Stack) {
               if (spHeight.isBottom() || tf.from.off() < 0) {
                  // Copying from unknown stack slot.  Bottom the target.
                  tf = TransferFunc::bottomFunc(tf.target);
               } else if (spHeight.isTop()) {
                  // Copying from unknown non-stack location.  Top the target.
                  tf = TransferFunc::retopFunc(tf.target);
               } else {
                  int newOff = tf.from.off() + (int) spHeight.height();
                  tf.from = Absloc(newOff, 0, NULL);
               }
            }
            if (tf.target.type() == Absloc::Stack) {
               // Ignore writes to unresolvable locations
               if (spHeight.isBottom() || spHeight.isTop()) continue;

               if (tf.target.off() < 0) {
                  fprintf(stderr, "Function summary writes to own frame\n");
                  fprintf(stderr, "%s\n", tf.format().c_str());
                  STACKANALYSIS_ASSERT(false);
               }
               int newOff = tf.target.off() + (int) spHeight.height();
               tf.target = Absloc(newOff, 0, NULL);
               summaryLoc = tf.target;
            }
            std::map<Absloc, std::pair<long, bool> > newFromRegs;
            for (auto frIter = tf.fromRegs.begin(); frIter != tf.fromRegs.end();
               frIter++) {
               const Absloc &loc = frIter->first;
               const std::pair<long, bool> &pair = frIter->second;
               if (loc.type() == Absloc::Stack) {
                  if (spHeight.isBottom() || loc.off() < 0) {
                     // fromRegs contains an unresolvable stack slot.  Bottom
                     // the target.
                     tf = TransferFunc::bottomFunc(tf.target);
                     break;
                  }
                  // Ignore topped locations
                  if (spHeight.isTop()) continue;

                  int newOff = loc.off() + (int) spHeight.height();
                  newFromRegs[Absloc(newOff, 0, NULL)] = pair;
               } else {
                  newFromRegs[loc] = pair;
               }
            }
            if (!tf.isBottom()) tf.fromRegs = newFromRegs;

            stackanalysis_printf("%s\n", tf.format().c_str());
            funcSummary[summaryLoc] = tf;
         }
         return true;
      }
   }

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
      Architecture arch = insn.getArch();
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
            handleDefault(insn, block, off, xferFuncs);
            return true;
      }
      if ((*iter).first.regClass() == gpr) {
         if (callWritten.test((*iter).second)) {
            Absloc loc((*iter).first);
            if (toppableFunctions.find(calledAddr) == toppableFunctions.end() &&
               returnRegs.test((*iter).second)) {
               // Bottom return registers if the called function isn't marked as
               // toppable.
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

   for (auto eit = outs.begin(); eit != outs.end(); ++eit) {
      Edge *cur_edge = (Edge*)*eit;

      Absloc sploc(sp());

      if (cur_edge->type() == DIRECT) {
         // For some reason we're treating this
         // call as a branch. So it shifts the stack
         // like a push (heh) and then we're done.
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
         xferFuncs.push_back(TransferFunc::bottomFunc(sploc));
         bottomBaseSubReg(sp(), xferFuncs);
      } else {
         xferFuncs.push_back(TransferFunc::deltaFunc(sploc, h.height()));
         copyBaseSubReg(sp(), xferFuncs);
      }
      return true;
   }
   return true;
}

// Create transfer functions for tail calls
bool StackAnalysis::handleJump(Instruction insn, Block *block, Offset off,
                               TransferFuncs &xferFuncs, TransferSet &funcSummary) {
   Address calledAddr = 0;
   boost::lock_guard<Block> g(*block);
   const Block::edgelist &outs = block->targets();
   for (auto eit = outs.begin(); eit != outs.end(); eit++) {
      Edge *edge = *eit;
      if (!edge->interproc() || edge->type() != DIRECT) continue;

      if (callResolutionMap.find(off) != callResolutionMap.end()) {
         // This tail call has already been resolved with PLT info available, so
         // we should use that resolution information.
         calledAddr = callResolutionMap[off];
      } else {
         // This tail call has not been resolved yet.
         Block *calledBlock = edge->trg();
         calledAddr = calledBlock->start();
      }
      if (intervals_ != NULL &&
         functionSummaries.find(calledAddr) != functionSummaries.end()) {
         stackanalysis_printf("\t\t\tFound function summary for %lx\n",
            calledAddr);

         // Update stack slots in the summary to line up with this stack frame,
         // and then add the modified transfer functions to xferFuncs.
         Absloc sploc(sp());
         const DefHeightSet &spSet = (*intervals_)[block][off][sploc];
         const Height &spHeight = spSet.getHeightSet();
         const TransferSet &fs = functionSummaries[calledAddr];
         for (auto fsIter = fs.begin(); fsIter != fs.end(); fsIter++) {
            Absloc summaryLoc = fsIter->first;
            TransferFunc tf = fsIter->second;
            if (tf.from.type() == Absloc::Stack) {
               if (spHeight.isBottom() || tf.from.off() < 0) {
                  // Copying from unknown stack slot.  Bottom the target.
                  tf = TransferFunc::bottomFunc(tf.target);
               } else if (spHeight.isTop()) {
                  // Copying from unknown non-stack location.  Top the target.
                  tf = TransferFunc::retopFunc(tf.target);
               } else {
                  int newOff = tf.from.off() + (int) spHeight.height();
                  tf.from = Absloc(newOff, 0, NULL);
               }
            }
            if (tf.target.type() == Absloc::Stack) {
               // Ignore writes to unresolvable locations
               if (spHeight.isBottom() || spHeight.isTop()) continue;

               if (tf.target.off() < 0) {
                  fprintf(stderr, "Function summary writes to own frame\n");
                  fprintf(stderr, "%s\n", tf.format().c_str());
                  STACKANALYSIS_ASSERT(false);
               }
               int newOff = tf.target.off() + (int) spHeight.height();
               tf.target = Absloc(newOff, 0, NULL);
               summaryLoc = tf.target;
            }
            std::map<Absloc, std::pair<long, bool> > newFromRegs;
            for (auto frIter = tf.fromRegs.begin(); frIter != tf.fromRegs.end();
               frIter++) {
               const Absloc &loc = frIter->first;
               const std::pair<long, bool> &pair = frIter->second;
               if (loc.type() == Absloc::Stack) {
                  if (spHeight.isBottom() || loc.off() < 0) {
                     // fromRegs contains an unresolvable stack slot.  Bottom
                     // the target.
                     tf = TransferFunc::bottomFunc(tf.target);
                     break;
                  }
                  // Ignore topped locations
                  if (spHeight.isTop()) continue;

                  int newOff = loc.off() + (int) spHeight.height();
                  newFromRegs[Absloc(newOff, 0, NULL)] = pair;
               } else {
                  newFromRegs[loc] = pair;
               }
            }
            if (!tf.isBottom()) tf.fromRegs = newFromRegs;

            stackanalysis_printf("%s\n", tf.format().c_str());
            funcSummary[summaryLoc] = tf;
         }
         return true;
      }
   }

   if (calledAddr == 0) {
      // Not a tail call
      return false;
   }

   // This is a tail call, but we don't have a function summary for the called
   // function.  Therefore, we handle it as a call, followed by a return.
   //
   // i.e. jmp <foo> is equivalent to call <foo>; ret

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
      Architecture arch = insn.getArch();
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
            handleDefault(insn, block, off, xferFuncs);
            return true;
      }
      if ((*iter).first.regClass() == gpr) {
         if (callWritten.test((*iter).second)) {
            Absloc loc((*iter).first);
            if (toppableFunctions.find(calledAddr) == toppableFunctions.end() &&
               returnRegs.test((*iter).second)) {
               // Bottom return registers if the called function isn't marked as
               // toppable.
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

   // Adjust the stack pointer for the implicit return (see comment above).
   xferFuncs.push_back(TransferFunc::deltaFunc(Absloc(sp()), word_size));
   copyBaseSubReg(sp(), xferFuncs);

   return true;
}

bool StackAnalysis::handleThunkCall(Instruction insn, Block *block,
                                    const Offset off, TransferFuncs &xferFuncs) {

   // We know that we're not a normal call, so it depends on whether the CFT is
   // "next instruction" or not.
   if (insn.getCategory() != c_CallInsn || !insn.getControlFlowTarget()) {
      return false;
   }

   // Eval of getCFT(0) == size?
   Expression::Ptr cft = insn.getControlFlowTarget();
   cft->bind(thePC.get(), Result(u32, 0));
   Result res = cft->eval();
   if (!res.defined) return false;
   if (res.convert<unsigned>() == insn.size()) {
      Absloc sploc(sp());
      xferFuncs.push_back(TransferFunc::deltaFunc(sploc, -1 * word_size));
      copyBaseSubReg(sp(), xferFuncs);
   }
   // Else we're calling a mov, ret thunk that has no effect on the stack
   // pointer

   // Check the next instruction to see which register is holding the PC.
   // Assumes next instruction is add thunk_reg, offset.
   const Address pc = off + insn.size();
   Instruction thunkAddInsn = block->getInsn(pc);
   if (!thunkAddInsn.isValid()) return true;
   if (thunkAddInsn.getOperation().getID() != e_add) return true;
   std::set<RegisterAST::Ptr> writtenRegs;
   thunkAddInsn.getOperand(0).getWriteSet(writtenRegs);
   if (writtenRegs.size() != 1) return true;
   const MachRegister &thunkTarget = (*writtenRegs.begin())->getID();

   xferFuncs.push_back(TransferFunc::absFunc(Absloc(thunkTarget), pc));
   copyBaseSubReg(thunkTarget, xferFuncs);
   return true;
}


void StackAnalysis::createEntryInput(AbslocState &input) {
   // FIXME for POWER/non-IA32
   // IA32 - the in height includes the return address and therefore
   // is <wordsize>
   // POWER - the in height is 0
#if defined(arch_power)
   input[Absloc(sp())].addInitSet(Height(0));
#elif (defined(arch_x86) || defined(arch_x86_64))
   input[Absloc(sp())].addInitSet(Height(-word_size));
   if (sp() == x86_64::rsp) {
      input[Absloc(x86_64::esp)].addInitSet(Height(-word_size));
   }
#else
   DYNINST_SUPPRESS_UNUSED_VARIABLE(input);
   STACKANALYSIS_ASSERT(0 && "Unimplemented architecture");
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
   return blockSummaryOutputs[b];
}

void StackAnalysis::meetInputs(Block *block, AbslocState &blockInput,
   AbslocState &input) {
   input.clear();
   intra_nosink_nocatch epred2;

   stackanalysis_printf("\t ... In edges: ");
   boost::lock_guard<Block> g(*block);

   const Block::edgelist & inEdges = block->sources();
   std::for_each(
      boost::make_filter_iterator(epred2, inEdges.begin(), inEdges.end()),
      boost::make_filter_iterator(epred2, inEdges.end(), inEdges.end()),
      boost::bind(&StackAnalysis::meet, this,
         boost::bind(&StackAnalysis::getSrcOutputLocs, this, boost::placeholders::_1),
         boost::ref(input)));
   stackanalysis_printf("\n");

   meet(blockInput, input);
}


void StackAnalysis::meetSummaryInputs(Block *block, TransferSet &blockInput,
   TransferSet &input) {
   input.clear();
   intra_nosink_nocatch epred2;
   boost::lock_guard<Block> g(*block);

   const Block::edgelist & inEdges = block->sources();
   std::for_each(
      boost::make_filter_iterator(epred2, inEdges.begin(), inEdges.end()),
      boost::make_filter_iterator(epred2, inEdges.end(), inEdges.end()),
      boost::bind(&StackAnalysis::meetSummary, this,
         boost::bind(&StackAnalysis::getSummarySrcOutputLocs, this, boost::placeholders::_1),
         boost::ref(input)));

   meetSummary(blockInput, input);
}


// Keep track of up to DEF_LIMIT multiple definitions/heights, then bottom
StackAnalysis::DefHeightSet StackAnalysis::meetDefHeights(
   const DefHeightSet &s1, const DefHeightSet &s2) {
   DefHeightSet newSet;
   if (s1.size() == 1 && s1.begin()->height.isTop()) return s2;
   if (s2.size() == 1 && s2.begin()->height.isTop()) return s1;
   if ((s1.size() == 1 && s1.begin()->height.isBottom()) ||
      (s2.size() == 1 && s2.begin()->height.isBottom())) {
      newSet.makeBottomSet();
      return newSet;
   }

   // At this point, we know that both sets contain only heights since we ensure
   // that all sets containing TOP/BOTTOM are size 1.
   newSet = s1;
   for (auto iter = s2.begin();
      iter != s2.end() && newSet.size() <= DEF_LIMIT; iter++) {
      newSet.insert(*iter);
   }
   if (newSet.size() > DEF_LIMIT) {
      newSet.makeBottomSet();
   }
   return newSet;
}


void StackAnalysis::meet(const AbslocState &input, AbslocState &accum) {
   for (auto iter = input.begin(); iter != input.end(); ++iter) {
      const Absloc &loc = iter->first;
      accum[loc] = meetDefHeights(iter->second, accum[loc]);
      if (accum[loc].begin()->height.isTop()) {
         accum.erase(loc);
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

   STACKANALYSIS_ASSERT(lhs.target == rhs.target);
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
         STACKANALYSIS_ASSERT(false);
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
            std::map<Absloc, std::pair<long, bool> > fromRegs;
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
         STACKANALYSIS_ASSERT(false);
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
         STACKANALYSIS_ASSERT(false);
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
         STACKANALYSIS_ASSERT(false);
      }
   } else {
      STACKANALYSIS_ASSERT(false);
   }
   return ret;
}


bool StackAnalysis::TransferFunc::isBaseRegCopy() const {
   return isCopy() && !isTopBottom() && target.type() == Absloc::Register &&
      from.type() == Absloc::Register && target.reg().size() == 4 &&
      from.reg().size() == 8 && target.reg().getBaseRegister() == from.reg();
}

bool StackAnalysis::TransferFunc::isBaseRegSIB() const {
   return isSIB() && fromRegs.size() == 2 &&
      target.type() == Absloc::Register && target.reg().size() == 4 &&
      target.reg().getBaseRegister().size() == 8 &&
      fromRegs.find(target) != fromRegs.end() &&
      fromRegs.find(Absloc(target.reg().getBaseRegister())) != fromRegs.end();
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

void StackAnalysis::DefHeightSet::makeTopSet() {
   defHeights.clear();
   defHeights.insert(DefHeight(Definition(), Height::top));
}

void StackAnalysis::DefHeightSet::makeBottomSet() {
   defHeights.clear();
   Definition d;
   d.type = Definition::BOTTOM;
   defHeights.insert(DefHeight(d, Height::bottom));
}

void StackAnalysis::DefHeightSet::makeNewSet(Block *b, Address addr,
   const Absloc &origLoc, const Height &h) {
   defHeights.clear();
   defHeights.insert(DefHeight(Definition(b, addr, origLoc), h));
}

void StackAnalysis::DefHeightSet::addInitSet(const Height &h) {
   defHeights.insert(DefHeight(Definition(), h));
}

void StackAnalysis::DefHeightSet::addDeltaSet(long delta) {
   std::set<DefHeight> temp = defHeights;
   defHeights.clear();
   for (auto iter = temp.begin(); iter != temp.end(); iter++) {
      const Definition &d = iter->def;
      const Height &h = iter->height;
      defHeights.insert(DefHeight(d, h + delta));
   }
}

StackAnalysis::Height StackAnalysis::DefHeightSet::getHeightSet() const {
   Height h;
   if (defHeights.size() == 0) {
      h = Height::top;
   } else if (defHeights.size() == 1) {
      h = defHeights.begin()->height;
   } else {
      h = Height::bottom;
   }
   return h;
}

StackAnalysis::Definition StackAnalysis::DefHeightSet::getDefSet() const {
   Definition d;
   if (defHeights.size() == 0) {
      d.type = Definition::TOP;
   } else if (defHeights.size() == 1) {
      d = defHeights.begin()->def;
   } else {
      d.type = Definition::BOTTOM;
   }
   return d;
}

bool StackAnalysis::DefHeightSet::isTopSet() const {
   if (defHeights.size() == 0) return true;
   return defHeights.size() == 1 &&
      defHeights.begin()->def.type == Definition::TOP &&
      defHeights.begin()->height.isTop();
}

bool StackAnalysis::DefHeightSet::isBottomSet() const {
   return defHeights.size() == 1 &&
      defHeights.begin()->def.type == Definition::BOTTOM &&
      defHeights.begin()->height.isBottom();
}


// Destructive update of the input map. Assumes inputs are absolute,
// uninitialized, or bottom; no deltas.
StackAnalysis::DefHeightSet StackAnalysis::TransferFunc::apply(
   const AbslocState &inputs) const {
   STACKANALYSIS_ASSERT(target.isValid());
   DefHeightSet inputSet;
   // Bottom stomps everything
   if (isBottom()) {
      inputSet.makeBottomSet();
      return inputSet;
   }

   AbslocState::const_iterator iter_ = inputs.find(target);
   if (iter_ != inputs.end()) {
      inputSet = iter_->second;
   } else {
      inputSet.makeTopSet();
   }

   bool isTopBottomOrig = isTopBottom();

   if (isSIB()) {
      inputSet.makeTopSet();  // SIB overwrites, so start at TOP
      for (auto iter = fromRegs.begin(); iter != fromRegs.end(); ++iter) {
         Absloc curLoc = (*iter).first;
         long curScale = (*iter).second.first;
         //bool curTopBottom = (*iter).second.second;
         auto findLoc = inputs.find(curLoc);
         Height locInput;
         if (findLoc == inputs.end()) {
            locInput = Height::top;
         } else {
            locInput = findLoc->second.getHeightSet();
         }

         if (locInput == Height::top) {
            // This term doesn't affect our end result, so it can be safely
            // ignored.
         } else {
            if (curScale == 1) {
               // If the scale isn't 1, then any stack height is obfuscated,
               // and we can safely ignore the term.
               inputSet.makeBottomSet();
               break;
            }
         }
      }
   }

   if (isAbs()) {
      // We cannot be a copy, as the absolute removes that.
      STACKANALYSIS_ASSERT(!isCopy());
      // Apply the absolute
      // NOTE: an absolute is not a stack height, set input to top
      //input = abs;
      inputSet.makeTopSet();
   }
   if (isCopy()) {
      // Cannot be absolute
      STACKANALYSIS_ASSERT(!isAbs());
      // Copy the input value from whatever we're a copy of.
      AbslocState::const_iterator iter2 = inputs.find(from);
      if (iter2 != inputs.end()) {
         //const Definition &def = iter2->second.getDefSet();
         const Height &h = iter2->second.getHeightSet();
         if (!h.isBottom() && !h.isTop()) {
            if ((from.isSP() || from.isFP()) &&
               (target.type() != Absloc::Register ||
                  (!target.reg().getBaseRegister().isStackPointer() &&
                     !target.reg().getBaseRegister().isFramePointer()))) {
               // Create new definitions when based on SP or FP
               inputSet.makeNewSet(NULL, 0, target, h);
            } else {
               // Reuse base definition otherwise
               inputSet = iter2->second;
            }
         } else if (h.isBottom()) {
            inputSet.makeBottomSet();
         } else {
            inputSet.makeTopSet();
         }
      } else {
         inputSet.makeTopSet();
      }
   }
   if (isDelta()) {
      inputSet.addDeltaSet(delta);
   }
   if (isRetop()) {
      inputSet.makeTopSet();
   }
   if (isTopBottomOrig) {
      auto iter = inputSet.begin();
      if (!iter->height.isTop()) {
         inputSet.makeBottomSet();
      }
   }
   return inputSet;
}

// Returns accumulated transfer function without modifying inputs
StackAnalysis::TransferFunc StackAnalysis::TransferFunc::summaryAccumulate(
   const TransferSet &inputs) const {
   TransferFunc input;
   auto findTarget = inputs.find(target);
   if (findTarget != inputs.end()) {
      input = findTarget->second;
   }
   if (input.target.isValid()) STACKANALYSIS_ASSERT(input.target == target);
   input.target = target; // Default constructed TransferFuncs won't have this
   STACKANALYSIS_ASSERT(target.isValid());

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
            STACKANALYSIS_ASSERT(!fromRegFunc.isBottom());  // Should be special case
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
         STACKANALYSIS_ASSERT(!orig.isCopy());
         input = orig;
         input.target = target;
         input.topBottom = false;
         STACKANALYSIS_ASSERT(input.target.isValid());

         if (isDelta()) {
            input.delta += delta;
         }
         return input;
      }
      if (orig.isCopy()) {
         STACKANALYSIS_ASSERT(!orig.isAbs());
         input = orig;
         input.target = target;
         STACKANALYSIS_ASSERT(input.target.isValid());

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
      STACKANALYSIS_ASSERT(!orig.isDelta());
      input.delta = 0;
      input.topBottom = isTopBottomOrig || orig.isTopBottom();
      input.fromRegs.clear();
   }

   STACKANALYSIS_ASSERT(!isDelta());

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


void StackAnalysis::SummaryFunc::apply(Block *block, const AbslocState &in,
   AbslocState &out) const {
   // Copy all the elements we don't have xfer funcs for.
   out = in;

   // Apply in parallel since all summary funcs are from the start of the block
   for (TransferSet::const_iterator iter = accumFuncs.begin();
      iter != accumFuncs.end(); ++iter) {
      STACKANALYSIS_ASSERT(iter->first.isValid());
      out[iter->first] = iter->second.apply(in);
      DefHeightSet &s = out[iter->first];
      const Definition &def = s.begin()->def;
      const Height &h = s.begin()->height;
      if (def.type == Definition::DEF && def.block == NULL) {
         // New definition
         s.makeNewSet(block, 0, def.origLoc, h);
      }
      if (h.isTop()) {
         out.erase(iter->first);
      }
   }
}


void StackAnalysis::SummaryFunc::accumulate(const TransferSet &in,
   TransferSet &out) const {

   // Copy all the elements we don't have xfer funcs for.
   out = in;

   // Apply in parallel since all summary funcs are from the start of the block
   for (auto iter = accumFuncs.begin(); iter != accumFuncs.end(); ++iter) {
      STACKANALYSIS_ASSERT(iter->first.isValid());
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
      TransferFunc &func_ = *iter;
      func_.accumulate(accumFuncs);
   }
   validate();
}

void StackAnalysis::SummaryFunc::addSummary(const TransferSet &summary) {
   // Accumulate all transfer functions in the summary atomically.
   TransferSet newAccumFuncs = accumFuncs;
   for (auto iter = summary.begin(); iter != summary.end(); ++iter) {
      const Absloc &loc = iter->first;
      const TransferFunc &func_ = iter->second;
      newAccumFuncs[loc] = func_.summaryAccumulate(accumFuncs);
   }
   accumFuncs = newAccumFuncs;
   validate();
}

void StackAnalysis::SummaryFunc::validate() const {
   for (TransferSet::const_iterator iter = accumFuncs.begin(); 
      iter != accumFuncs.end(); ++iter) {
      const TransferFunc &func_ = iter->second;
      STACKANALYSIS_ASSERT(func_.target.isValid());
      if (func_.isCopy()) STACKANALYSIS_ASSERT(!func_.isAbs());
      if (func_.isAbs()) STACKANALYSIS_ASSERT(!func_.isCopy());
      if (func_.isBottom()) STACKANALYSIS_ASSERT(!func_.isCopy());
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
   for (auto iter = input.begin(); iter != input.end(); ++iter) {
      const DefHeightSet &s = iter->second;
      ret << iter->first.format() << " := {";
      for (auto iter2 = s.begin(); iter2 != s.end(); iter2++) {
         ret << "(" << iter2->def.format() << ", " <<
            iter2->height.format() << "), ";
      }
      ret << "}, ";
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
         STACKANALYSIS_ASSERT(0);
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
