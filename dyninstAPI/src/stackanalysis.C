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

const StackAnalysis::Height StackAnalysis::Height::bottom(StackAnalysis::Height::notUnique);
const StackAnalysis::Height StackAnalysis::Height::top(StackAnalysis::Height::uninitialized);

const StackAnalysis::Presence StackAnalysis::Presence::bottom(StackAnalysis::Presence::bottom_t);
const StackAnalysis::Presence StackAnalysis::Presence::top(StackAnalysis::Presence::top_t);

const StackAnalysis::InsnTransferFunc StackAnalysis::InsnTransferFunc::bottom(StackAnalysis::InsnTransferFunc::notUnique, false);
const StackAnalysis::InsnTransferFunc StackAnalysis::InsnTransferFunc::top(StackAnalysis::InsnTransferFunc::uninitialized, false);

const StackAnalysis::BlockTransferFunc StackAnalysis::BlockTransferFunc::bottom(StackAnalysis::BlockTransferFunc::notUnique, false, false);
const StackAnalysis::BlockTransferFunc StackAnalysis::BlockTransferFunc::top(StackAnalysis::BlockTransferFunc::uninitialized, false, false);

const StackAnalysis::BlockTransferFunc StackAnalysis::BlockTransferFunc::initial(0, true, true);

const StackAnalysis::Range StackAnalysis::defaultRange(std::make_pair(0, 0), 0, 0);
const StackAnalysis::Range::range_t StackAnalysis::Range::infinite(std::make_pair(MINLONG, MAXLONG));

AnnotationClass <StackAnalysis::HeightTree> HeightAnno(std::string("HeightAnno"));
AnnotationClass <StackAnalysis::PresenceTree> PresenceAnno(std::string("PresenceAnno"));


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
    //stackanalysis_printf("Beginning stack analysis for function %s\n",
    //func->symTabName().c_str());
    blocks = func->blocks();
    if (blocks.empty()) return false;

    blocks = func->blocks();
    
    //stackanalysis_printf("\tSummarizing block effects\n");
    summarizeBlocks();
    
    //stackanalysis_printf("\tPerforming fixpoint analysis\n");
    fixpoint();

    //stackanalysis_printf("\tCreating interval trees\n");
    createPresenceIntervals();
    createHeightIntervals();

    func->addAnnotation(heightIntervals_, HeightAnno);
    func->addAnnotation(presenceIntervals_, PresenceAnno);

    if (dyn_debug_stackanalysis) {
        debugHeights();
        debugPresences();
    }

    //stackanalysis_printf("Finished stack analysis for function %s\n",
    //func->symTabName().c_str());

    return true;
}

// We want to create a transfer function for the block as a whole. 
// This will allow us to perform our fixpoint calculation over
// blocks (thus, O(B^2)) rather than instructions (thus, O(I^2)). 
//
// Handling the stack height is straightforward. We also accumulate
// region changes in terms of a stack of Region objects.

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

    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;

        // Accumulators. They have the following behavior:
        // 
        // New region: add to the end of the regions list
        // Offset to stack pointer: accumulate to delta.
        // Setting the stack pointer: zero delta, set set_value.
        // Creating a stack frame: indicate in presence. 
        // Destroying a stack frame: indicate in presence. 

        BlockTransferFunc &bFunc = blockEffects[block];
        Presence &bPresence = blockPresences[block];

        bFunc = BlockTransferFunc::top;
        bPresence = Presence::top;

        //stackanalysis_printf("\t Block starting at 0x%lx: %s, %s\n", block->firstInsnOffset(),
        //bPresence.format().c_str(), bFunc.format().c_str());

        std::vector<std::pair<InstructionAPI::Instruction::Ptr, Offset> > instances;
        block->getInsnInstances(instances);

        for (unsigned j = 0; j < instances.size(); j++) {
            InstructionAPI::Instruction::Ptr insn = instances[j].first;
            Offset &off = instances[j].second;
            Offset next;
            if (j < (instances.size() - 1)) {
                next = instances[j+1].second;
            }
            else {
                next = block->endOffset();
            }
            
            InsnTransferFunc iFunc;
            Presence presence;

            computeInsnEffects(block, insn, off, 
                               iFunc, presence);

            if (iFunc != InsnTransferFunc::top) {
                iFunc.apply(bFunc);
                blockToInsnFuncs[block][next] = iFunc;
            }
            if (presence != Presence::top) {
                presence.apply(bPresence); 
                blockToInsnPresences[block][next] = presence;
            }
            //stackanalysis_printf("\t\t\t At 0x%lx: %s, %s\n",
            //off, bPresence.format().c_str(), bFunc.format().c_str());
        }
        //stackanalysis_printf("\t Block summary for 0x%lx: %s\n", block->firstInsnOffset(), bFunc.format().c_str());
    }
}


void StackAnalysis::fixpoint() {

    std::queue<Block *> worklist;

    worklist.push(func->entryBlock());

    while (!worklist.empty()) {
        Block *block = worklist.front();
        //stackanalysis_printf("\t Fixpoint analysis: visiting block at 0x%lx\n", block->firstInsnOffset());

        worklist.pop();

        // Step 1: calculate the meet over the heights of all incoming
        // intraprocedural blocks.

        std::set<BlockTransferFunc> inEffects;
        std::set<Presence> inPresences;
        
        if (block->isEntryBlock(func)) {
            // The in height is 0
            // The set height is... 0
            // And there is no input region.
            inEffects.insert(BlockTransferFunc::initial);
            inPresences.insert(Presence(Presence::noFrame_t));
            //stackanalysis_printf("\t Primed initial block\n");
        }
        else {
            std::vector<Edge *> inEdges; block->getSources(inEdges);
            for (unsigned i = 0; i < inEdges.size(); i++) {
                Edge *edge = inEdges[i];
                if (edge->getType() == ET_CALL) continue;
                inEffects.insert(outBlockEffects[edge->getSource()]);
                //stackanalysis_printf("\t\t Inserting 0x%lx: %s\n", edge->getSource()->firstInsnOffset(),
                //outBlockEffects[edge->getSource()].format().c_str());
                inPresences.insert(outBlockPresences[edge->getSource()]);
            }
        }
        
        BlockTransferFunc newInEffect = meet(inEffects);
        Presence newInPresence = meet(inPresences);

        //stackanalysis_printf("\t New in meet: %s, %s\n",
        //newInPresence.format().c_str(),
        //newInEffect.format().c_str());

        // Step 2: see if the input has changed

        if ((newInEffect == inBlockEffects[block]) &&
            (newInPresence == inBlockPresences[block])) {
            // No new work here
            //stackanalysis_printf("\t ... equal to current, skipping block\n");
            continue;
        }
        
        //stackanalysis_printf("\t ... inequal to current %s, %s, analyzing block\n",
        //inBlockPresences[block].format().c_str(),
        //inBlockEffects[block].format().c_str());

        inBlockEffects[block] = newInEffect;
        inBlockPresences[block] = newInPresence;
        
        // Step 3: calculate our new outs
        
        blockEffects[block].apply(newInEffect, 
                                  outBlockEffects[block]);
        blockPresences[block].apply(newInPresence,
                                    outBlockPresences[block]);
        
        //stackanalysis_printf("\t ... output from block: %s, %s\n",
        //outBlockPresences[block].format().c_str(),
        //outBlockEffects[block].format().c_str());

        // Step 4: push all children on the worklist.

        std::vector<Edge *> outEdges; block->getTargets(outEdges);

        for (unsigned i = 0; i < outEdges.size(); i++) {
            if (outEdges[i]->getType() == ET_CALL) continue;
            worklist.push(outEdges[i]->getTarget());
        }
    }
}

void StackAnalysis::createPresenceIntervals() {
    // We now have a summary of the state of the stack at
    // the entry to each block. We need to push that information
    // into the block. Assume that frame affecting instructions
    // are rare, where i_m, i_n affect the stack (m < n). Then
    // each interval runs from [i_m, i_n) or [i_m, i_{n-1}]. 

    presenceIntervals_ = new PresenceTree();

    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;

        //stackanalysis_printf("\t Interval creation (P): visiting block at 0x%lx\n", block->firstInsnOffset());
        
        Offset curLB = block->firstInsnOffset();
        Offset curUB = 0;
        Presence curPres = inBlockPresences[block];

        // We only cache points where the frame presence changes. 
        // Therefore, we can simply iterate through them. 
        for (InsnPresence::iterator iter = blockToInsnPresences[block].begin();
             iter != blockToInsnPresences[block].end(); iter++) {

            // We've changed the state of the stack pointer. End this interval
            // and start a new one. 
            
            curUB = iter->first;
            presenceIntervals_->insert(curLB, curUB, curPres);

            curLB = curUB;
            curPres = iter->second;
        }

        if (curLB != block->endOffset()) {
            // Cap off the extent for the current block
            curUB = block->endOffset();
            assert(curPres == outBlockPresences[block]);
            presenceIntervals_->insert(curLB, curUB, curPres);
        }
    }
}

void StackAnalysis::createHeightIntervals() {
    // We now have a summary of the state of the stack at
    // the entry to each block. We need to push that information
    // into the block. Assume that frame affecting instructions
    // are rare, where i_m, i_n affect the stack (m < n). Then
    // each interval runs from [i_m, i_n) or [i_m, i_{n-1}]. 

    heightIntervals_ = new HeightTree();

    for (Block::blockSet::iterator iter = blocks.begin(); iter != blocks.end(); iter++) {
        Block *block = *iter;

        //stackanalysis_printf("\t Interval creation (H): visiting block at 0x%lx\n", block->firstInsnOffset());
        
        Offset curLB = block->firstInsnOffset();
        Offset curUB = 0;
        BlockTransferFunc curHeight = inBlockEffects[block];

        //stackanalysis_printf("\t\t Block starting state: 0x%lx, %s\n", 
        //curLB, curHeight.format().c_str());

        // We only cache points where the frame height changes. 
        // Therefore, we can simply iterate through them. 
        for (InsnFuncs::iterator iter = blockToInsnFuncs[block].begin();
             iter != blockToInsnFuncs[block].end(); iter++) {

            // We've changed the state of the stack pointer. End this interval
            // and start a new one. 
            
            curUB = iter->first;
            

            heightIntervals_->insert(curLB, curUB, 
                                     Height(curHeight.delta(),
                                            getRegion(curHeight.ranges())));

            curLB = curUB;
            // Adjust height
            iter->second.apply(curHeight);

            //stackanalysis_printf("\t\t Block continuing state: 0x%lx, %s\n", 
            //curLB, curHeight.format().c_str());
        }

        if (curLB != block->endOffset()) {
            // Cap off the extent for the current block
            curUB = block->endOffset();
            if (curHeight != outBlockEffects[block]) {
                fprintf(stderr, "ERROR: accumulated state not equal to fixpoint state!\n");
                fprintf(stderr, "\t %s\n", curHeight.format().c_str());
                fprintf(stderr, "\t %s\n", outBlockEffects[block].format().c_str());
            }
                        
            assert(curHeight == outBlockEffects[block]);

            heightIntervals_->insert(curLB, curUB, 
                                     Height(curHeight.delta(),
                                            getRegion(curHeight.ranges())));
        }
    }
}


void StackAnalysis::computeInsnEffects(const Block *block,
                                       const Instruction::Ptr &insn,
                                       Offset off,
                                       InsnTransferFunc &iFunc,
                                       Presence &pres)
{
    //stackanalysis_printf("\t\tInsn at 0x%lx\n", off); 
    static Expression::Ptr theStackPtr(new RegisterAST(r_eSP));
    static Expression::Ptr stackPtr32(new RegisterAST(r_ESP));
    static Expression::Ptr stackPtr64(new RegisterAST(r_RSP));
    
    static Expression::Ptr theFramePtr(new RegisterAST(r_eBP));
    static Expression::Ptr framePtr32(new RegisterAST(r_EBP));
    static Expression::Ptr framePtr64(new RegisterAST(r_RBP));
    
    //TODO: Integrate entire test into analysis lattice
    entryID what = insn->getOperation().getID();

    if (insn->isWritten(theFramePtr) || insn->isWritten(framePtr32) || insn->isWritten(framePtr64)) {
      //stackanalysis_printf("\t\t\t FP written\n");
        if (what == e_mov &&
            (insn->isRead(theStackPtr) || insn->isRead(stackPtr32) || insn->isRead(stackPtr64))) {
	  pres = Presence(Presence::frame_t);
	  //stackanalysis_printf("\t\t\t Frame created\n");
        }
        else {
            pres = Presence(Presence::noFrame_t);
            //stackanalysis_printf("\t\t\t Frame destroyed\n");
        }
    }

    if (what == e_call) {
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
            Height h = getStackCleanAmount(target_func);
            if (h == Height::bottom) {
                iFunc == InsnTransferFunc::bottom;
            }
            else {
                iFunc.delta() = h.height();
            }

            //stackanalysis_printf("\t\t\t Stack height changed by self-cleaning function: %s\n", iFunc.format().c_str());
            return;
        }
        //stackanalysis_printf("\t\t\t Stack height assumed unchanged by call\n");
        return;
    }

    int word_size = func->img()->getAddressWidth();
    
    if(!insn->isWritten(theStackPtr) && !insn->isWritten(stackPtr32)) {
         return;
    }
    int sign = 1;
    switch(what) {
    case e_push:
        sign = -1;
    case e_pop: {
        Operand arg = insn->getOperand(0);
        if (arg.getValue()->eval().defined) {
            iFunc.delta() = sign * word_size;
            //stackanalysis_printf("\t\t\t Stack height changed by evaluated push/pop: %s\n", iFunc.format().c_str());
            return;
        }
        iFunc.delta() = sign * arg.getValue()->size();
        //stackanalysis_printf("\t\t\t Stack height changed by unevalled push/pop: %s\n", iFunc.format().c_str());
        return;
    }
    case e_ret_near:
    case e_ret_far:
        iFunc.delta() = word_size;
        //stackanalysis_printf("\t\t\t Stack height changed by return: %s\n", iFunc.format().c_str());
        return;
    case e_sub:
        sign = -1;
    case e_add: {
        // Add/subtract are op0 += (or -=) op1
        Operand arg = insn->getOperand(1);
        Result delta = arg.getValue()->eval();
        if(delta.defined) {
            switch(delta.type) {
            case u8:
                iFunc.delta() = (sign * delta.val.u8val);
                break;
            case u16:
                iFunc.delta() = (sign * delta.val.u16val);
                break;
            case u32:
                iFunc.delta() = (sign * delta.val.u32val);
                break;
            case u64:
                iFunc.delta() = (sign * delta.val.u64val);
                break;
            case s8:
                iFunc.delta() = (sign * delta.val.s8val);
                break;
            case s16:
                iFunc.delta() = (sign * delta.val.s16val);
                break;
            case s32:
                iFunc.delta() = (sign * delta.val.s32val);
                break;
            case s64:
                iFunc.delta() = (sign * delta.val.s64val);
                break;
            default:
                // Add of anything that's not a "normal" integral type
                // means we don't know what happened
                iFunc = InsnTransferFunc::bottom;
                break;
            }
            //stackanalysis_printf("\t\t\t Stack height changed by evalled add/sub: %s\n", iFunc.format().c_str());
            return;
        }
        iFunc.range() = Range(Range::infinite, 0, off);
        //stackanalysis_printf("\t\t\t Stack height changed by unevalled add/sub: %s\n", iFunc.format().c_str());
        return;
    }
        // We treat return as zero-modification right now
    case e_leave:
        iFunc.abs() = true;
        iFunc.delta() = 0;
        //stackanalysis_printf("\t\t\t Stack height reset by leave: %s\n", iFunc.format().c_str());
        return;
    default:
        iFunc.range() = Range(Range::infinite, 0, off);
        //stackanalysis_printf("\t\t\t Stack height changed by unhandled insn \"%s\": %s\n", 
        //insn->format().c_str(), iFunc.format().c_str());
        return;
    }
    assert(0);
    return;
}

StackAnalysis::Height StackAnalysis::getStackCleanAmount(image_func *func) {
    // Cache previous work...
    if (funcCleanAmounts.find(func) != funcCleanAmounts.end()) {
        return funcCleanAmounts[func];
    }

    if (!func->cleansOwnStack()) {
        funcCleanAmounts[func] = 0;
        return funcCleanAmounts[func];
    }

    InstructionDecoder decoder;   
    unsigned char *cur;

    std::set<Height> returnCleanVals;
    
    for (unsigned i=0; i < func->funcExits().size(); i++) {
        cur = (unsigned char *) func->getPtrToInstruction(func->funcExits()[i]->offset());
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
    func(NULL), heightIntervals_(NULL), presenceIntervals_(NULL), 
    rt(Region::Ptr(new Region())) {};
    

StackAnalysis::StackAnalysis(Function *f) : func(f),
                                            rt(Region::Ptr(new Region())) 
{
    blocks = func->blocks();
    heightIntervals_ = NULL;
    presenceIntervals_ = NULL;
}

const StackAnalysis::HeightTree *StackAnalysis::heightIntervals() {
    if (func == NULL) return NULL;

    // Check the annotation
    HeightTree *ret;
    func->getAnnotation(ret, HeightAnno);
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
    func->getAnnotation(ret, PresenceAnno);
    if (ret) return ret;

    if (presenceIntervals_ == NULL) {
        if (!analyze()) return NULL;
        ret = presenceIntervals_;
    }

    return ret;
}

void StackAnalysis::debugHeights() {
    if (!heightIntervals_) return;
    std::vector<std::pair<std::pair<Offset, Offset>, Height> > elements;
    heightIntervals_->elements(elements);

    for (unsigned i = 0; i < elements.size(); i++) {
        fprintf(stderr, "0x%lx - 0x%lx: %s\n",
                elements[i].first.first,
                elements[i].first.second,
                elements[i].second.format().c_str());
    }
}

void StackAnalysis::debugPresences() {
    if (!presenceIntervals_) return;

    std::vector<std::pair<std::pair<Offset, Offset>, Presence> > elements;
    presenceIntervals_->elements(elements);

    for (unsigned i = 0; i < elements.size(); i++) {
        fprintf(stderr, "0x%lx - 0x%lx: %s\n",
                elements[i].first.first,
                elements[i].first.second,
                elements[i].second.format().c_str());
    }
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
            sprintf(buf, "<%d>", delta_);
        }
        else {
            sprintf(buf, "Abs: %d", delta_);
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

void StackAnalysis::Presence::apply(const Presence &in, Presence &out) const {
    out = in;
    apply(out);
}

void StackAnalysis::Presence::apply(Presence &out) const {
    if (presence_ == top_t) return;
    out.presence_ = presence_;
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
            //stackanalysis_printf("\t Creating new node for range %s\n", 
            //str[i].format().c_str());
            // Need to create a new node...
            Node *newNode = new Node(Region::Ptr(new Region(getNewRegionID(),
                                                            str[i],
                                                            cur->region)));
            cur->children[str[i]] = newNode;
            cur = newNode;
        }
        else {
            //stackanalysis_printf("\t Found existing node for range %s\n",
            //str[i].format().c_str());
            cur = iter->second;
        }
    }
    //stackanalysis_printf("\t Returning region %s\n", cur->region->format().c_str());
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

