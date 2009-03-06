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

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/symtab.h"
#include "instPoint.h"

#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Result.h"
#include "instructionAPI/h/Instruction.h"

#include <queue>
#include <vector>

#include "stackanalysis.h"

#include "Annotatable.h"

#include "debug.h"

using namespace Dyninst;
using namespace InstructionAPI;

const StackAnalysis::StackHeight StackAnalysis::StackHeight::bottom(StackAnalysis::StackHeight::notUnique);
const StackAnalysis::StackHeight StackAnalysis::StackHeight::top(StackAnalysis::StackHeight::uninitialized);

AnnotationClass <StackAnalysis::HeightTree> StackHeightAnno(std::string("StackHeightAnno"));
AnnotationClass <StackAnalysis::PresenceTree> StackPresenceAnno(std::string("StackPresenceAnno"));


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
    stanalysis_printf("Beginning stack analysis for function %s\n",
                      func->symTabName().c_str());
    blocks = func->blocks();
    if (blocks.empty()) return false;
    
    blocks = func->blocks();
    
    stanalysis_printf("\tSummarizing block effects\n");
    summarizeBlockDeltas();
    
    stanalysis_printf("\tPerforming fixpoint analysis\n");
    calculateInterBlockDepth();

    stanalysis_printf("\tCreating interval trees\n");
    createIntervals();

    func->addAnnotation(heightIntervals_, StackHeightAnno);
    func->addAnnotation(presenceIntervals_, StackPresenceAnno);

    if (dyn_debug_stackanalysis) {
        debugStackHeights();
        debugStackPresences();
    }

    stanalysis_printf("Finished stack analysis for function %s\n",
                      func->symTabName().c_str());

    return true;
}
   
void StackAnalysis::summarizeBlockDeltas() {
    // STACK HEIGHT:
    // Foreach block B:
    //   Let acc = 0;
    //   Foreach (insn i, offset o) in B:
    //     Let d = change in stack height at i;
    //     Let acc += d;
    //     blockToInsnDeltas[B][o] = d;
    //   blockHeightDeltas[B] = acc;

    // STACK PRESENCE:
    // Foreach block B:
    //   Let d = uninitialized;
    //   Foreach (insn i, offset o) in B:
    //     d = change in stack presence at d;
    //     blockToInsnDeltas[B][o] = d;
    //   blockHeightDeltas[B] = d;

    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;

        StackHeight heightChange(0);
        StackPresence stackPresence;

        std::vector<std::pair<InstructionAPI::Instruction, Offset> > instances;
        block->getInsnInstances(instances);

        for (unsigned j = 0; j < instances.size(); j++) {
            InstructionAPI::Instruction insn = instances[j].first;
            Offset off = instances[j].second;

            // Can go top...
            StackHeight delta;
            computeInsnEffects(block, insn, off, delta, stackPresence);
            blockToInsnDeltas[block][off] = delta;
            blockToInsnPresence[block][off] = stackPresence;

            heightChange += delta;
        }
        blockHeightDeltas[block] = heightChange;
        blockPresenceDeltas[block] = stackPresence;
    }
}

void StackAnalysis::calculateInterBlockDepth() {

    std::queue<Block *> worklist;

    worklist.push(*(blocks.begin()));

    //BlockHeight_t blockHeights; // This by default initializes all entries to "bottom". 

    while (!worklist.empty()) {
        Block *block = worklist.front();
        worklist.pop();

        // Step 1: calculate the meet over the heights of all incoming
        // intraprocedural blocks.

        std::set<StackHeight> inHeights;
        std::set<StackPresence> inPresences;

        if (block->isEntryBlock(func)) {
            // The in height is 0
            inHeights.insert(StackHeight(0));
            inPresences.insert(StackPresence(StackPresence::noFrame));
        }
        else {
            std::vector<Edge *> inEdges; block->getSources(inEdges);
            for (unsigned i = 0; i < inEdges.size(); i++) {
                Edge *edge = inEdges[i];
                if (edge->getType() == ET_CALL) continue;
                inHeights.insert(outBlockHeights[edge->getSource()]);
                inPresences.insert(outBlockPresences[edge->getSource()]);
            }
        }
        
        StackHeight newInHeight = StackHeight::meet(inHeights);
        StackPresence newInPresence = StackPresence::meet(inPresences);

        // Step 2: see if the input has changed

        if ((newInHeight == inBlockHeights[block]) &&
            (newInPresence == inBlockPresences[block])) {
            // No new work here
            continue;
        }
        
        // Step 3: calculate our new out height

        inBlockHeights[block] = newInHeight;
        outBlockHeights[block] = newInHeight + blockHeightDeltas[block];

        inBlockPresences[block] = newInPresence;
        // If there was no change in the block then use the in presence; otherwise
        // use the change in the block. Effectively a 2-part meet.
        outBlockPresences[block] = (blockPresenceDeltas[block].isTop()) ? (newInPresence) : (blockPresenceDeltas[block]);

        // Step 4: push all children on the worklist.

        std::vector<Edge *> outEdges; block->getTargets(outEdges);

        for (unsigned i = 0; i < outEdges.size(); i++) {
            if (outEdges[i]->getType() == ET_CALL) continue;
            worklist.push(outEdges[i]->getTarget());
        }
    }
}

void StackAnalysis::createIntervals() {
    // Use the results summaries to calculate the 
    // stack heights. We expect that heights will
    // change infrequently and so summarize them at 
    // the function level. 

    heightIntervals_ = new HeightTree();
    presenceIntervals_ = new PresenceTree();

    StackHeight curHeight;
    Offset curLB = 0;
    Offset curUB = 0;

    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;
        
        curLB = block->firstInsnOffset();
        curUB = 0;
        curHeight = inBlockHeights[block];
        
        // Now create extents for instructions within this block.
        
        for (InsnDeltas::iterator iter = blockToInsnDeltas[block].begin();
             iter != blockToInsnDeltas[block].end(); 
             iter++) {
            // Now create extents for instructions within this block.
            if ((*iter).second == StackHeight(0)) {
                continue;
            }
            
            // We've changed the height of the stack. End this interval
            // and start a new one. 
            //
            // We need to determine UB. It's defined as the end of the 
            // current instruction. Therefore there are two cases:
            // if we're the last insn (take the block end) or not
            // the last insn (take the start of the next insn)
            InsnDeltas::iterator iter2 = iter;
            iter2++;
            if (iter2 == blockToInsnDeltas[block].end()) {
                curUB = block->endOffset();
            }
            else {
                curUB = (*iter2).first;
            }
            heightIntervals_->insert(curLB, curUB, curHeight);
            
            // Start a new one
            curLB = curUB;
            curUB = 0;
            curHeight += (*iter).second;

        }
        
        if (curLB != block->endOffset()) {
            // Cap off the extent for the current block
            curUB = block->endOffset();
            assert(curHeight == outBlockHeights[block]);
            heightIntervals_->insert(curLB, curUB, curHeight);
        }
    }

    StackPresence curPres;
    curUB = 0;
    curLB = 0;
    
    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;
        
        curLB = block->firstInsnOffset();
        curUB = 0;
        curPres = inBlockPresences[block];
        
        for (InsnPresence::iterator iter = blockToInsnPresence[block].begin();
             iter != blockToInsnPresence[block].end(); 
             iter++) {
            
            if ((*iter).second.isTop())
                continue;
            
            // We've changed the state of the stack pointer. End this interval
            // and start a new one. 
            //
            // We need to determine UB. It's defined as the end of the 
            // current instruction. Therefore there are two cases:
            // if we're the last insn (take the block end) or not
            // the last insn (take the start of the next insn)
            InsnPresence::iterator iter2 = iter;
            iter2++;
            if (iter2 == blockToInsnPresence[block].end()) {
                curUB = block->endOffset();
            }
            else {
                curUB = (*iter2).first;
            }
            presenceIntervals_->insert(curLB, curUB, curPres);
            
            // Start a new one
            curLB = curUB;
            curUB = 0;
            curPres = (*iter).second;
        }            

        if (curLB != block->endOffset()) {
            // Cap off the extent for the current block
            curUB = block->endOffset();
            assert(curPres == outBlockPresences[block]);
            presenceIntervals_->insert(curLB, curUB, curPres);
        }
    }
}


void StackAnalysis::computeInsnEffects(const Block *block,
                                       const Instruction &insn,
                                       Offset off,
                                       StackHeight &height,
                                       StackPresence &pres) 
{
    stanalysis_printf("\t\tInsn at 0x%lx\n", off); 
    Expression::Ptr theStackPtr(new RegisterAST(r_eSP));
    Expression::Ptr stackPtr32(new RegisterAST(r_ESP));
    Expression::Ptr stackPtr64(new RegisterAST(r_RSP));
    
    Expression::Ptr theFramePtr(new RegisterAST(r_eBP));
    Expression::Ptr framePtr32(new RegisterAST(r_EBP));
    Expression::Ptr framePtr64(new RegisterAST(r_RBP));
    
    //TODO: Integrate entire test into analysis lattice
    entryID what = insn.getOperation().getID();

    if (insn.isWritten(theFramePtr) || insn.isWritten(framePtr32) || insn.isWritten(framePtr64)) {
        stanalysis_printf("\t\t\t FP written\n");
        if (what == e_mov &&
            (insn.isRead(theStackPtr) || insn.isRead(stackPtr32) || insn.isRead(stackPtr64))) {
            pres = StackPresence::frame;
            stanalysis_printf("\t\t\t Frame created\n");
        }
        else {
            pres = StackPresence::noFrame;
            stanalysis_printf("\t\t\t Frame destroyed\n");
        }
    }
    

   if (what == e_call)
   {
      pdvector<image_edge *> outs;
      block->getTargets(outs);
      for (unsigned i=0; i<outs.size(); i++) {
         image_edge *cur_edge = outs[i];
         if (cur_edge->getType() != ET_CALL) 
             continue;
         
         image_basicBlock *target_bbl = cur_edge->getTarget();
         image_func *target_func = target_bbl->getEntryFunc();
         if (!target_func)
             continue;
         height = getStackCleanAmount(target_func);
         stanalysis_printf("\t\t\t Stack height changed by self-cleaning function: %s\n", height.getString().c_str());
         return;
      }
      height = StackHeight(0);
      stanalysis_printf("\t\t\t Stack height assumed unchanged by call\n");
      return;
   }

   int word_size = func->img()->getAddressWidth();
   
   if(!insn.isWritten(theStackPtr) && !insn.isWritten(stackPtr32)) {
       height = StackHeight(0);
       return;
   }
   int sign = 1;
   switch(what)
   {
   case e_push:
       sign = -1;
   case e_pop: {
       Operand arg = insn.getOperand(0);
       if (arg.getValue()->eval().defined) {
           height = StackHeight(sign * word_size); 
           stanalysis_printf("\t\t\t Stack height changed by evaluated push/pop: %s\n", height.getString().c_str());
           return;
       }
       height = StackHeight(sign * arg.getValue()->size());
       stanalysis_printf("\t\t\t Stack height changed by unevalled push/pop: %s\n", height.getString().c_str());
       return;
   }
   case e_sub:
       sign = -1;
   case e_add: {
       // Add/subtract are op0 += (or -=) op1
       Operand arg = insn.getOperand(1);
       Result delta = arg.getValue()->eval();
       if(delta.defined) {
           switch(delta.type) {
           case u8:
               height = StackHeight(sign * delta.val.u8val);
               break;
           case u16:
               height = StackHeight(sign * delta.val.u16val);
               break;
           case u32:
               height = StackHeight(sign * delta.val.u32val);
               break;
           case u64:
               height = StackHeight(sign * delta.val.u64val);
               break;
           case s8:
               height = StackHeight(sign * delta.val.s8val);
               break;
           case s16:
               height = StackHeight(sign * delta.val.s16val);
               break;
           case s32:
               height = StackHeight(sign * delta.val.s32val);
               break;
           case s64:
               height = StackHeight(sign * delta.val.s64val);
               break;
           default:
               // Add of anything that's not a "normal" integral type
               // means we don't know what happened
               height = StackHeight(StackHeight::bottom);
               break;
           }
           stanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %s\n", height.getString().c_str());
           return;
       }
   }
       height = StackHeight(StackHeight::bottom);
       stanalysis_printf("\t\t\t Stack height changed by unevalled add/sub: %s\n", height.getString().c_str());
       return;
       // We treat return as zero-modification right now
   case e_ret_near:
   case e_ret_far:
       height = StackHeight(0);
       stanalysis_printf("\t\t\t Stack height changed by ret_near/ret_far %s\n", height.getString().c_str());
       return;
   default:
       stanalysis_printf("\t\t\t Stack height changed by unhandled insn %s: %s\n", 
                         insn.format().c_str(), height.getString().c_str());
       height = StackHeight(StackHeight::bottom);
       return;
   }
   assert(0);
   return;
}


StackAnalysis::StackHeight StackAnalysis::getStackCleanAmount(image_func *func) {
    // Cache previous work...
    if (funcCleanAmounts.find(func) != funcCleanAmounts.end()) {
        return funcCleanAmounts[func];
    }

    if (!func->cleansOwnStack()) {
        funcCleanAmounts[func] = StackHeight(0);
        return StackHeight(0);
    }

    InstructionDecoder decoder;   
    unsigned char *cur;

    std::set<StackHeight> returnCleanVals;
    
    for (unsigned i=0; i < func->funcExits().size(); i++) {
        cur = (unsigned char *) func->getPtrToInstruction(func->funcExits()[i]->offset());
        size_t size = 0;
        Instruction insn = decoder.decode(cur, size);
        
        entryID what = insn.getOperation().getID();
        if (what != e_ret_near)
            continue;
        
        int val;
        std::vector<Operand> ops;
        insn.getOperands(ops);
        if (ops.size() == 0) {
            val = 0;
        }
        else {      
            Result imm = ops[0].getValue()->eval();
            assert(imm.defined);
            val = (int) imm.val.s16val;
        }
        returnCleanVals.insert(StackHeight(val));
    }

    funcCleanAmounts[func] = StackHeight::meet(returnCleanVals);
    return funcCleanAmounts[func];
}

StackAnalysis::StackAnalysis(Function *f) : func(f) {
    blocks = func->blocks();
    heightIntervals_ = NULL;
    presenceIntervals_ = NULL;
}

const StackAnalysis::HeightTree *StackAnalysis::heightIntervals() {
    if (func == NULL) return NULL;

    // Check the annotation
    HeightTree *ret;
    func->getAnnotation(ret, StackHeightAnno);
    if (ret) return ret;
    if (heightIntervals_ == NULL) {
        if (!analyze()) return NULL;
        ret = heightIntervals_;
    }

    return ret;
}


const StackAnalysis::PresenceTree *StackAnalysis::presenceIntervals() {
    if (func == NULL) return NULL;

    // Check the annotation
    PresenceTree *ret;
    func->getAnnotation(ret, StackPresenceAnno);
    if (ret) return ret;

    if (presenceIntervals_ == NULL) {
        if (!analyze()) return NULL;
        ret = presenceIntervals_;
    }

    return ret;
}

void StackAnalysis::debugStackHeights() {
    if (!heightIntervals_) return;
    std::vector<std::pair<std::pair<Offset, Offset>, StackHeight> > elements;
    heightIntervals_->elements(elements);

    for (unsigned i = 0; i < elements.size(); i++) {
        fprintf(stderr, "0x%lx - 0x%lx: %s\n",
                elements[i].first.first,
                elements[i].first.second,
                elements[i].second.getString().c_str());
    }
}

void StackAnalysis::debugStackPresences() {
    if (!presenceIntervals_) return;

    std::vector<std::pair<std::pair<Offset, Offset>, StackPresence> > elements;
    presenceIntervals_->elements(elements);

    for (unsigned i = 0; i < elements.size(); i++) {
        fprintf(stderr, "0x%lx - 0x%lx: %s\n",
                elements[i].first.first,
                elements[i].first.second,
                elements[i].second.getString().c_str());
    }
}
                
