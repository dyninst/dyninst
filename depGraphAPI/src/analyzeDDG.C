/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include <map>
#include <algorithm>
#include <queue>

#include "analyzeDDG.h"

#include "Absloc.h"
#include "Instruction.h"
#include "Operation.h"

#include "BPatch_basicBlock.h"
#include "BPatch_edge.h"
#include "BPatch_function.h"

#include "dataflowAPI/h/stackanalysis.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "dyninstAPI/src/addressSpace.h"
#include "Annotatable.h"

// Prototype intra-function DDG creator



// TODO
//   Handle calls
//     Trust the ABI to start
//     Analyze the callee function
//   Interprocedural

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace dyn_detail::boost;

AnnotationClass <DDG::Ptr> DDGAnno(std::string("DDGAnno"));

// Let me take an aside to discuss aliases and the issues they raise.
// An alias pair are two abstract locations that can actually "refer" to
// the same concrete location. We see this when memory is used, primarily
// due to incomplete information. Consider the following example:
// 
// S1 is an abstract location that refers to stack slot 1.
// S* is an abstract location that refers to an unknown slot on the stack.
// S* aliases S1 since it _could_ refer to S1. However, it does not
// necessarily refer to S1. Similarly, S1 aliases S; if an instruction is
// reading from an unknown location on the stack it may read from S1.
//
// When we build the DDG we need to include edges from all aliases of
// a given used absloc. However, this will _excessively_ overapproximate
// the DDG. Consider the following:
// B = <i1, i2, i3>
// i1 defines S*
// i2 defines S1
// i3 uses S1
// i4 uses S2
//
// Consider i3. It uses S1, and therefore has a dependence on i2. However,
// there is no dependence on i1. Conversely, i4 _does_ have a dependence on
// i1. 
// 
// The second example is more complex. 
//
// B1 = (i1), B2 = (j1), B3 = (k1)
// i1 defines S*
// j1 defines S1
// k1 uses S1
// 
// Note that k1 here has a dependence on both i1 and j1.
//
// So our alias handling needs to deal with this. 
//
// We do so by extending the classic definition of GEN and KILL.
//
// Recall that dataflow analysis can be put into the following
// framework:
// OUT(i_j) = GEN(i_j) U (IN(i_j) - KILL(i_j))
// 
// where the solution to the analysis is the set of OUT(i_j) that 
// satisfies the equations given.
// 
// Classically, we define GEN and KILL as follows:
// GEN(i_j) = {i_j} if i_j defines S_k
// KILL(i_j) = {i_j}^c if i_j defines S_k (where {x}^c is the complement set of {x})
//
// These are the GEN and KILL sets of S_k. We now expand that definition. Let
// GEN : Absloc -> {Insn} and Kill : Absloc -> {Insn} be maps where the considered
// absloc must be explicit (as opposed to implicit, above). We define these as follows:
// GEN(i,X) = 
//  if X = S_i : S_i = {i}, S = {i}
//  if X = X   : S = {i}, S_1 = {i}, ..., S_n = {i}
// KILL(i,X) = 
//  if X = S_i : {i}^c
//  if X = S   : 0
//
// The final piece of this is an optimization for a "forward" definition. 
// A definition of S at i_n is a "forward definition" of S_i if there is no
// prior definition of S_i. This matters to us because we create abstract locations
// lazily, and thus we won't _have_ an absloc for S_i...
// However, consider the following. Let i_n be a forward definition to S_i. Then
// IN(i_n, S_i) = OUT(i_{n-1}, S_i) = GEN(i_{n-1},S_i) U (IN(i_{n-2},S_i) - KILL(i_{n-1},S_i))
//     Since i_n is a forward definition, then all GEN sets must have come from a 
//     definition of S (and therefore GEN(i_j,S_i) = GEN(i_j,S)) and all KILL sets 
//     must have come from a kill of S (and are thus empty). Thus, by induction,
//     IN(i_n, S_i) = IN(i_n, S). 


DDG::Ptr DDGAnalyzer::analyze() {
    if (ddg) return ddg;

    if (func_ == NULL) return DDG::Ptr();

    DDG::Ptr *ret;
    func_->getAnnotation(ret, DDGAnno);
    if (ret) {
        ddg = *ret;
        return ddg;
    }

    DDGAnalyzer::BlockSet blocks;
    func_->getCFG()->getAllBasicBlocks(blocks);

    std::vector<DDGAnalyzer::Block *> entry;
    func_->getCFG()->getEntryBasicBlock(entry);    

    // We build the DDG from reaching definitions performed for
    // all points (instructions) and program variables. 

    // Create us a DDG
    ddg = DDG::createGraph(func_);

    // For complexity and efficiency, we perform the initial analysis on
    // basic blocks and then do intra-block analysis to build the insn-level
    // DDG. 

    // Initialize GEN and KILL sets for all basic blocks.

    summarizeGenKillSets(blocks);

    // We now have GEN for each block. Propagate reaching
    // definitions.

    // Generate the reaching defs for each block
    generateInterBlockReachingDefs(entry[0]);

    //debugBlockDefs();

    // inSets now contains the following:
    // FOR EACH Absloc A,
    //   The list of blocks with reaching definitions of A

    // We want to build the subnode-level graph.
    generateNodes(blocks);

    // And do a final pass over to ensure all nodes are reachable...
    ddg->insertVirtualEdges();

    return ddg;
}


DDGAnalyzer::DDGAnalyzer(Function *f) :
    func_(f), addr_width(0) {};


/***********************************************************************************
 ******* PHASE 1 *******************************************************************
 ***********************************************************************************/


// Copied from above
// GEN(i,X) = 
//  if X = S_i : S_i = {i}, S = {i}
//  if X = X   : S = {i}, S_1 = {i}, ..., S_n = {i}
// KILL(i,X) = 
//  if X = S_i : S_i = {i}^c, S = 0
//  if X = S   : 0

// Summarize gen and kill set information for the whole function.
// Recurses to summarizeBlockGenKill, below.

void DDGAnalyzer::summarizeGenKillSets(const BlockSet &blocks) {
    // Assert we haven't been run before...
    assert(allGens.empty());
    //fprintf(stderr, "initializeGenKillSets:\n");

    for (BlockSet::const_iterator iter = blocks.begin();
         iter != blocks.end(); 
         ++iter) {
        Block *curBlock = *iter;

        //fprintf(stderr, "\t Block 0x%lx\n", curBlock->getStartAddress());

        summarizeBlockGenKill(curBlock);
    }
}

void DDGAnalyzer::summarizeBlockGenKill(Block *curBlock) {
    std::vector<std::pair<InsnPtr,Address> > insns;
    curBlock->getInstructions(insns);
    
    for (std::vector<std::pair<InsnPtr,Address> >::reverse_iterator i = insns.rbegin();
         i != insns.rend(); 
         ++i) {
        const DefSet &writtenAbslocs = getDefinedAbslocs(i->first, i->second);

        //fprintf(stderr, "Insn at 0x%lx\n", i->second);
        
        for (DefSet::iterator iter = writtenAbslocs.begin();
             iter != writtenAbslocs.end();
             ++iter) {                
            // We have two cases: if the absloc is precise or an alias (S_i vs S above)
            AbslocPtr D = *iter;
            //fprintf(stderr, "\t%s, %d\n", D->format().c_str(), allKills[curBlock][D]);
            if (allKills[curBlock][D]) {
                // We have already definitely defined
                // this absloc (as it was killed), so
                // don't do anything else.
                continue;
            }

            cNode cnode = cNode(i->second, D);
            updateDefSet(D, allGens[curBlock], cnode);
            updateKillSet(D, allKills[curBlock]);
        }

        // If we have a call instruction handle the effects of the callee
        // after the call instruction itself. This will either create
        // a set of gens and kills based on the ABI or will actually
        // analyze the callee. 
        if (isCall(i->first)) {
            summarizeCallGenKill(i->first, 
                                 i->second, 
                                 allGens[curBlock],
                                 allKills[curBlock]);
        }
    }
}

// We still handle aliases by adding a definition for anything
// they alias to... 
void DDGAnalyzer::updateDefSet(const AbslocPtr D,
                               DefMap &defMap,
                               cNode &cnode) {
    AbslocSet aliases = D->getAliases();

    defMap[D].insert(cnode);

    for (AbslocSet::iterator al = aliases.begin();
         al != aliases.end(); ++al) {
        // This handles both the S case if we have a precise
        // absloc, as well as S_1, ..., S_n if we have an imprecise
        // absloc. 
        defMap[*al].insert(cnode);
    }
}


// This function does the work of handling aliases...                    

void DDGAnalyzer::updateKillSet(const AbslocPtr D,
                                KillMap &kills) {
    if (D->isPrecise()) {
        // We also record that this block kills this absLoc.
        // It doesn't matter which instruction does it, since
        // that will be summarized in the gen information.
        kills[D] = true;
    }
}

void DDGAnalyzer::summarizeCallGenKill(const InsnPtr,
                                       const Address &addr,
                                       DefMap &gens,
                                       KillMap &kills) {
    // I know of no architecture where the call instruction is 1 byte. 
    // So let's use call+1 as a placeholder for the effects of the call.
    
    Address placeholder = addr+1;

    // This will eventually make the decision of which mechanism to use to 
    // summarize the call. 

    Function *callee = getCallee(addr);

    summarizeABIGenKill(placeholder, callee, gens, kills);

    // summarizeConservativeGenKill(gens, kills);
    // summarizeScanGenKill(callee, gens, kills);
    // summarizeAnalyzeGenKill(callee, gen, kills);

}

/**********************************************************************
 ******** PHASE 2 *****************************************************
 **********************************************************************/


void DDGAnalyzer::generateInterBlockReachingDefs(Block *entry) {
    std::queue<Block *> worklist;

    worklist.push(entry);

    while (!worklist.empty()) {
        Block *working = worklist.front();
        worklist.pop();

        //fprintf(stderr, "Considering block 0x%lx\n", working->getStartAddress());

        // Calculate the new in set 

        BlockSet preds;
        getPredecessors(working, preds);
        
        // NEW_IN = U (j \in pred) OUT(j,a)
        inSets[working].clear();
        
        merge(inSets[working], preds);

        //fprintf(stderr, "New in set:\n");
        //debugDefMap(inSets[working], "\t");

        // Now: newIn = U (j \in pred) OUT(j)
        
        // OUT(i,a) = GEN(i,a) U (IN(i,a) - KILL(i,a))
        DefMap newOut;
        calcNewOut(newOut,
                   allGens[working], 
                   allKills[working],
                   inSets[working]);
        //fprintf(stderr, "Old out set: \n");
        //debugDefMap(outSets[working], "\t");

        //fprintf(stderr, "New out set: \n");
        //debugDefMap(newOut, "\t");

        if (newOut != outSets[working]) {
            outSets[working] = newOut;
            BlockSet successors;
            getSuccessors(working, successors);
            for (BlockSet::iterator succ = successors.begin(); succ != successors.end(); ++succ) {
                worklist.push(*succ);
            }
        }
    }    
}

// Merge has to do two things. 
// First, handle the unknown parameter definition.
//   If any of the input blocks define a precise absloc A,
//   then we consider all other blocks to have a "parameter"
//   definition of A. This parameter definition will later be
//   removed if the block actually defines A.
// Second, the aliasing problem. 
//   If an block defines an abstract region we want to add
//   it as a possible definition of everything that it aliases
//   to that weren't also definitely defined.

void DDGAnalyzer::merge(DefMap &target,
                        const BlockSet &preds) {
    std::map<Absloc::Ptr, unsigned> beenDefined;

    for (BlockSet::const_iterator iter = preds.begin(); iter != preds.end(); ++iter) {
        DefMap &source = outSets[*iter];
        
        for (DefMap::const_iterator iter = source.begin();
             iter != source.end(); ++iter) {
            const Absloc::Ptr A = (*iter).first;
            
            target[A].insert(source[A].begin(), source[A].end());
            beenDefined[A]++;
            
            mergeAliases(A, source, target);
        }
    }
    for (std::map<Absloc::Ptr, unsigned>::const_iterator iter = beenDefined.begin();
         iter != beenDefined.end(); ++iter) {
        if (iter->second != preds.size())
            target[iter->first].insert(cNode(0, iter->first, formalParam));
    }

}

void DDGAnalyzer::mergeAliases(const Absloc::Ptr &A,
                               DefMap &source,
                               DefMap &target) {
    if (!A->isPrecise()) {
        AbslocSet aliases(A->getAliases());
        
        for (AbslocSet::iterator a_iter = aliases.begin(); 
             a_iter != aliases.end(); ++a_iter) {
            
            if (source.find(*a_iter) == source.end()) {
                target[*a_iter].insert(source[A].begin(), 
                                       source[A].end());
            }
        }
    }
}


void DDGAnalyzer::calcNewOut(DefMap &out,
                             DefMap &gens,
                             KillMap &kills,
                             DefMap &in) {
    // OUT = GEN U (IN - KILL)

    

    AbslocSet definedAbslocs;
    for (DefMap::const_iterator iter = gens.begin();
         iter != gens.end();
         ++iter) {
        definedAbslocs.insert((*iter).first);
    }            

    for (DefMap::const_iterator iter = in.begin(); 
         iter != in.end();
         ++iter) {
        definedAbslocs.insert((*iter).first);
    }

    // Calculate the new OUT set
    for (AbslocSet::iterator iter = definedAbslocs.begin();
         iter != definedAbslocs.end();
         ++iter) {
        AbslocPtr A = *iter;

        // If we kill this AbslocPtr within this block, then
        // take the entry from the GEN set only.
        if (kills.find(A) != kills.end()) {
            out[A] = gens[A];
        }
        else {
            // We don't explicitly kill this, so take the union
            // of local generation with the INs. 
            out[A] = gens[A];
            out[A].insert(in[A].begin(), 
                          in[A].end());
        }
    }
}

/**********************************************************************
 ******** PHASE 3 *****************************************************
 **********************************************************************/


void DDGAnalyzer::generateNodes(const BlockSet &blocks) {
    // We have a set of inter-block reaching definitions. We now build the graph
    // of intra-block reaching defs. 
    
    // Algorithmically:
    // For each block B:
    //   Let localReachingDefs : Absloc -> Node = ins[B]
    //   For each instruction instance i in B:
    //     Let def = i.defines();
    //     For each absloc D in def:
    //       Let T = NODE(I,D)
    //       Let used = abslocs I uses to define D.
    //       For each absloc U in used:
    //         Let Aliases = aliases to U.
    //         For each absloc A in Aliases
    //           If localReachingDefs[A] neq NULL
    //             then foreach S in localReachingDefs[A]
    //               insert(S,T)
    //           else insert(parameterNode, T)
    //     For each absloc D in def:
    //       localReachingDefs[D] = GEN(i) U (localReachingDefs[D] - KILL(i))
    // See comment below for why we do this in two iterations.

    //fprintf(stderr, "generateIntraBlockReachingDefs...\n");

    for (BlockSet::const_iterator b_iter = blocks.begin();
         b_iter != blocks.end();
         ++b_iter) {

        Block *B = *b_iter;

        generateBlockNodes(B);

    }
}

void DDGAnalyzer::generateBlockNodes(Block *B) {
    std::vector<std::pair<InsnPtr, Address> > insns;
    B->getInstructions(insns);
    DefMap &localReachingDefs = inSets[B];
    //fprintf(stderr, "\tBlock 0x%lx\n", B->getStartAddress());
    
    
    for (unsigned i = 0; i < insns.size(); ++i) {
        InsnPtr I = insns[i].first;
        Address addr = insns[i].second;
        //fprintf(stderr, "\t\t Insn at 0x%lx\n", addr); 
        
        DefSet def = getDefinedAbslocs(I, addr);

        createInsnNodes(I, addr, 
                        def,
                        localReachingDefs);

        updateReachingDefs(addr, 
                           def,
                           localReachingDefs);

        //fprintf(stderr, "After 0x%lx, localReachingDefs:\n",
        //        addr);
        //debugDefMap(localReachingDefs, "\t");
        
        if (isCall(I)) {
            // Therefore we don't need to care about localReachingDefs
            // anymore since it will be disassembled. But we still need
            // to create nodes.
            createCallNodes(addr, localReachingDefs);
        }
        if (isReturn(I)) {
            // So again we can destroy localReachingDefs to make return nodes.
            createReturnNodes(addr, localReachingDefs);
        }
    }
}

// This is an interesting function. We need to create the "micro-graph" of 
// intra-instruction dependencies, and then hook up the entry nodes of that
// micro-graph to the appropriate reaching definitions. 
//
// This gets interesting when instructions define multiple abstract locations. 
// For a trivial example, consider the PC. 
//
// This gets _really_ interesting when some of the definitions by the instruction
// depend on other definitions by the instruction. Consider the IA-32 push instruction,
// which can be represented as follows:
// SP = SP - 4; (assuming stack 'grows' downward)
// *SP = <register>
//
// Note that the "*SP" operand depends on SP, which is updated. Thus, it depends
// on SP _as defined by the push_. 
//
// In a perfect world, there would be a separate library between the DDG and
// InstructionAPI that represents these. We're not in a perfect world, so this
// function is an approximation. 
//
// We currently represent common instructions correctly. Everything else gets a
// completely interconnected network. This is a safe overapproximation. See
// also, rep prefixes. 


void DDGAnalyzer::createInsnNodes(const InsnPtr I, 
                                  const Address &addr,
                                  const DefSet &def,
                                  DefMap &localReachingDefs) {
    // We first create the micro-graph. Then, for each used, create
    // edges from reaching defs to "entry node". 

    // This is a map from <used absloc> to <nodes that use that absloc>
    NodeMap worklist;

    // Non-PC handling section
    switch(I->getOperation().getID()) {
    case e_push: {
        // SP = SP - 4 
        // *SP = <register>
 
        std::vector<Operand> operands;
        I->getOperands(operands);

        // According to the InstructionAPI, the first operand will be the argument, the second will be ESP.
        assert(operands.size() == 2);

        // The argument can be any of the following:
        // 1) a register (push eax);
        // 2) an immediate value (push $deadbeef)
        // 3) a memory location. 
        Absloc::Ptr arg;
        if (operands[0].readsMemory()) {
            // Evaluate 
        }
        else {
            std::set<RegisterAST::Ptr> readRegs;
            operands[0].getReadSet(readRegs);
            if (!readRegs.empty()) {
                RegisterAST::Ptr reg = *(readRegs.begin());
                arg = getAbsloc(reg);
            }
        }
        // Otherwise arg defaults to NULL

        std::set<RegisterAST::Ptr> spRegs;
        operands[1].getReadSet(spRegs);
        assert(!spRegs.empty());
        RegisterAST::Ptr sp = *(spRegs.begin());

        handlePushEquivalent(addr, arg, sp, worklist);
        
    }
        break;
    case e_call: {
        // This can be seen as a push of the PC...

        // So we need the PC and the SP
        RegisterAST::Ptr pc;
        RegisterAST::Ptr sp;
        std::set<RegisterAST::Ptr> readRegs = I->getOperation().implicitReads();
        for (std::set<RegisterAST::Ptr>::iterator iter = readRegs.begin(); iter != readRegs.end(); ++iter) {
            if (RegisterLoc::isSP(*iter))
                sp = *iter;
            else if (RegisterLoc::isPC(*iter))
                pc = *iter;
            else assert(0);
        }
        Absloc::Ptr aPC = getAbsloc(pc);
        handlePushEquivalent(addr, aPC, sp, worklist);
        break;
    }
    case e_pop: {
        // <reg> = *SP
        // SP = SP + 4/8
        // Amusingly... this doesn't have an intra-instruction dependence. It should to enforce
        // the order that <reg> = *SP happens before SP = SP - 4, but since the input to both 
        // uses of SP in this case are the, well, input values... no "sideways" edges. 
        // However, we still special-case it so that SP doesn't depend on the incoming stack value...
        // Also, we use the same logic for return, defining it as
        // PC = *SP
        // SP = SP + 4/8

        // As with push, eSP shows up as operand 1. 

        std::vector<Operand> operands;
        I->getOperands(operands);

        // According to the InstructionAPI, the first operand will be the explicit register, the second will be ESP.
        assert(operands.size() == 2);

        std::set<RegisterAST::Ptr> regs;
        operands[0].getWriteSet(regs);
        
        RegisterAST::Ptr reg = *(regs.begin());
        assert(reg);

        regs.clear();

        operands[1].getReadSet(regs);
        RegisterAST::Ptr sp = *(regs.begin());

        handlePopEquivalent(addr, reg, sp, worklist);
    } break;
    case e_leave: {
        // a leave is equivalent to:
        // mov ebp, esp
        // pop ebp
        // From a definition POV, we have the following:
        // SP = BP
        // BP = *SP
        
        // BP    STACK[newSP]
        //  |    |
        //  v    v
        // SP -> BP
        
        // This is going to give the stack analysis fits... for now, I think it just reverts the
        // stack depth to 0. 
        
        // Leave has no operands...
        RegisterAST::Ptr sp;
        RegisterAST::Ptr bp;
        std::set<RegisterAST::Ptr> regs = I->getOperation().implicitWrites();
        for (std::set<RegisterAST::Ptr>::iterator iter = regs.begin(); iter != regs.end(); ++iter) {
            if (RegisterLoc::isSP(*iter))
                sp = *iter;
            else
                bp = *iter;
        }
        Absloc::Ptr aSP = getAbsloc(sp);
        Absloc::Ptr aBP = getAbsloc(bp);

        // We need the stack...
        Operation::VCSet memReads = I->getOperation().getImplicitMemReads();
        // Use addr + 1 for now because we need the post-leave stack height...
        // This works because leave has a size of 1. It's ugly. I should fix this...
        Absloc::Ptr aStack = getAbsloc(*(memReads.begin()), addr+1);

        //fprintf(stderr, "%s, %s, %s\n", aSP->format().c_str(), aBP->format().c_str(), aStack->format().c_str());

        Node::Ptr nSP = makeNode(cNode(addr, aSP));
        Node::Ptr nBP = makeNode(cNode(addr, aBP));
        
        worklist[aBP].push_back(nSP);
        worklist[aStack].push_back(nBP);
        ddg->insertPair(nSP, nBP);

        break;
    }
    case e_ret_near:
    case e_ret_far: {
        // PC = *SP
        // SP = SP + 4/8
        // Like pop, except it's all implicit.

        // So we need the PC and the SP
        RegisterAST::Ptr pc;
        RegisterAST::Ptr sp;
        std::set<RegisterAST::Ptr> regs = I->getOperation().implicitWrites();
        for (std::set<RegisterAST::Ptr>::iterator iter = regs.begin(); iter != regs.end(); ++iter) {
            if (RegisterLoc::isSP(*iter))
                sp = *iter;
            else if (RegisterLoc::isPC(*iter))
                pc = *iter;
            else assert(0);
        }
        handlePopEquivalent(addr, pc, sp, worklist);
    }
        break;
    case e_xchg: {
        // xchg defines two abslocs, and uses them as appropriate...
        AbslocPtr D1;
        AbslocPtr D2;
        
        if (addr == 0x9124d2) {
            fprintf(stderr, "Here!\n");
        }

        AbslocSet::iterator iter = def.gprs.begin(); 
        D1 = *iter;
        iter++;
        if (iter != def.gprs.end()) {
            D2 = *iter;
        }
        else {
            // Check memory
            assert(!def.mem.empty());
            AbslocSet::iterator iter2 = def.mem.begin();
            D2 = *iter2;
        }
                
        assert(D1);
        assert(D2);
            
        NodePtr T1 = makeNode(cNode(addr, D1));
        NodePtr T2 = makeNode(cNode(addr, D2));
        worklist[D1].push_back(T2);
        worklist[D2].push_back(T1);
        break;
        }
        
    default:
        // Assume full intra-dependence of non-flag and non-pc registers. 
        const AbslocSet &used = getUsedAbslocs(I, addr);
        for (DefSet::iterator iter = def.beginGprsMem(); iter != def.end(); ++iter) {
            // This will give us only non-flag and non-PC registers...
            AbslocPtr D = *iter;
            NodePtr T = makeNode(cNode(addr, D));
            for (AbslocSet::const_iterator u_iter = used.begin(); u_iter != used.end(); ++u_iter) {
                AbslocPtr U = *u_iter;
                worklist[U].push_back(T);
            }
        }
    }
    // Now for flags...
    // According to Matt, the easiest way to represent dependencies for flags on 
    // IA-32/AMD-64 is to have them depend on the inputs to the instruction and 
    // not the outputs of the instruction; therefore, there's no intra-instruction
    // dependence. 
    const AbslocSet &used = getUsedAbslocs(I, addr);
    for (DefSet::iterator iter = def.beginFlags(); iter != def.end(); ++iter) {
        AbslocPtr D = *iter;
        NodePtr T = makeNode(cNode(addr, D));
        for (AbslocSet::const_iterator u_iter = used.begin(); u_iter != used.end(); ++u_iter) {
            AbslocPtr U = *u_iter;
            worklist[U].push_back(T);
        }
    }

    // PC-handling section
    // Most instructions use the PC to set the PC. This includes calls, relative branches,
    // and the like. So we're basically looking for indirect branches or absolute branches.
    // (are there absolutes on IA-32?).
    // Also, conditional branches and the flag registers they use. 

    if (def.defPC()) {
    // We're some sort of branch...
        switch(I->getOperation().getID()) {
            case e_ret_near:
            case e_ret_far: {
                // Read top of stack, define PC
                std::set<RegisterAST::Ptr> regs = I->getOperation().implicitReads();
                // Only one thing read...
                RegisterAST::Ptr sp = *(regs.begin());
                Absloc::Ptr aStack = getAbsloc(sp, addr); 

                Absloc::Ptr aPC = RegisterLoc::makePC();
                NodePtr T = makeNode(cNode(addr, aPC));
                worklist[aStack].push_back(T);
                break;
            }
            default:
                  // Whatever is in the operands gets used... PC gets written
                   Absloc::Ptr aPC = RegisterLoc::makePC();
                   NodePtr T = makeNode(cNode(addr, aPC));
                   std::vector<Operand> operands;
                   I->getOperands(operands);
                   for (unsigned i = 0; i < operands.size(); ++i) {
                       std::set<RegisterAST::Ptr> regs;
                       operands[i].getWriteSet(regs);
                       for (std::set<RegisterAST::Ptr>::iterator r_iter = regs.begin();
                       r_iter != regs.end(); r_iter++) {
                           Absloc::Ptr a_r = getAbsloc(*r_iter);
                           worklist[a_r].push_back(T);
                       }
                   }
        }
    }
    else {
        Absloc::Ptr aPC = RegisterLoc::makePC();
        NodePtr T = makeNode(cNode(addr, aPC));
        worklist[aPC].push_back(T);
    }

    
    
    //fprintf(stderr, "Creating nodes for addr 0x%lx\n", addr);
    // And now hook up reaching definitions
    for (NodeMap::iterator e_iter = worklist.begin(); e_iter != worklist.end(); ++e_iter) {
        for (NodeVec::iterator n_iter = e_iter->second.begin();
             n_iter != e_iter->second.end(); ++n_iter) {
             NodePtr T = *n_iter;
             //fprintf(stderr, "\tNode %s\n", T->format().c_str());
             AbslocPtr U = e_iter->first;

             if (localReachingDefs[U].empty()) {
                // Not sure this can actually happen...
                // Just build a parameter node
                cNode tmp(0, U, formalParam);
                NodePtr S = makeNode(tmp);
                ddg->insertPair(S, T);
                //fprintf(stderr, "\t\t from parameter node %s\n", S->format().c_str());
            }
            else {
                for (cNodeSet::iterator c_iter = localReachingDefs[U].begin();
                     c_iter != localReachingDefs[U].end(); ++c_iter) {
                    NodePtr S = makeNode(*c_iter);

                    //fprintf(stderr, "\t\t %s\n", S->format().c_str());
                    
                    ddg->insertPair(S,T);
                    //fprintf(stderr, "\t\t\t\t ... from local definition %s/0x%lx\n",
                    //c_iter->first->format().c_str(),
                    //c_iter->second.addr);
                }
            }
        }
    }
}

void DDGAnalyzer::updateReachingDefs(const Address &addr, 
                                     const DefSet &def, 
                                     DefMap &localReachingDefs) { 
    // We now update localReachingDefs. If we do it in the previous
    // loop we can get errors. Consider this example:
    // 
    // i1 defines r1, r2, r3
    // i2 uses r1 and defines r1, r2
    // 
    // When at i1 localReachingDefs contains (r1, i1), (r2, i1), (r3, i1)
    // 
    // We then consider i2.
    // Def set: (r1, r2)
    // Use set: (r1)
    //
    // Let D = r1
    //   Let U = r1
    //     Insert edge ((i1, r1), (i2, r1))
    //   Update localReachingDefs (r1, i2), (r2, i1), (r3, i1)
    // Let D = r2
    //   Let U = r1
    //     Insert edge ((i2, r1), (i2, r2))
    //
    // See the problem? Because we update localReachingDefs before we're done
    // with the instruction we can imply an incorrect ordering of assignments
    // within the instruction. Instead we update localReachingDefs afterwards.
    
    // Also, we have to be aware of aliasing issues within the block. 

    for (DefSet::iterator d_iter = def.begin();
         d_iter != def.end(); ++d_iter) {
        AbslocPtr D = *d_iter;
        
        cNode cnode(addr, D); 
        
        if (D->isPrecise()) {
            // Kill previous definitions
            // We didn't do this in phase 1 because we
            // were going backwards. 
            localReachingDefs[D].clear();
        }
        
        updateDefSet(D, localReachingDefs, cnode);
    }
}

void DDGAnalyzer::createCallNodes(const Address &A,
                                  const DefMap &reachingDefs) {

    // I know of no architecture where the call instruction is 1 byte. 
    // So let's use call+1 as a placeholder for the effects of the call.

    Address placeholder = A+1;

    Function *callee = getCallee(A);

    NodeVec actualParams;

    summarizeABIUsed(placeholder, callee, reachingDefs, actualParams);

    // That created all of the used nodes. Now we need to add all defined
    // nodes. 
    
    // We can ignore updating reachingDefs because we are guaranteeing we're
    // at the end of the block. So we want to look up (or just re-create)
    // actualReturn nodes for each of these...
    
    // So that we don't work too hard, just cache actualReturnNodes when they're
    // created and re-use them here. 

    for (cNodeSet::iterator i = actualReturnMap_[placeholder].begin(); 
         i != actualReturnMap_[placeholder].end(); 
         ++i) {
        Node::Ptr T = makeNode(*i);

        for (NodeVec::iterator j = actualParams.begin(); 
             j != actualParams.end(); ++j) {
            Node::Ptr S = *j;
            ddg->insertPair(S, T);
        }
    }
}

void DDGAnalyzer::createReturnNodes(const Address &,
                                    const DefMap &reachingDefs) {
    // I believe we want to create a return node for each reaching definition
    // to this point...

    // This is an overapproximation but definitely safe. 

    for (DefMap::const_iterator iter = reachingDefs.begin(); 
         iter != reachingDefs.end(); ++iter) {
        Absloc::Ptr a = iter->first;
        
        cNode cnode(0, a, formalReturn);

        Node::Ptr T = makeNode(cnode);

        for (cNodeSet::const_iterator c_iter = iter->second.begin();
             c_iter != iter->second.end(); ++c_iter) {
            NodePtr S = makeNode(*c_iter);
            ddg->insertPair(S, T);
        }
    }
}

/**********************************************************
 ********* Absloc creation functions **********************
 **********************************************************/

Absloc::Ptr DDGAnalyzer::getAbsloc(const InstructionAPI::Expression::Ptr exp,
                                   Address addr) {
    // We want to simplify the expression as much as possible given 
    // currently known state, and then quantify it as one of the following:
    // 
    // Stack: a memory access based off the current frame pointer (FP) or
    //   stack pointer (SP). If we can determine an offset from the "top"
    //   of the stack we create a stack slot location. Otherwise we create
    //   a "stack" location that represents all stack locations.
    //
    // Heap: a memory access to a generic pointer.
    //
    // Memory: a memory access to a known address. 
    //
    // TODO: aliasing relations. Aliasing SUCKS. 

    // Since we have an Expression as input, we don't have the dereference
    // operator.

    // Here's the logic:
    // If no registers are used:
    //   If only immediates are used:
    //     Evaluate and create a MemLoc.
    //   If a dereference exists:
    //     WTF???
    // If registers are used:
    //   If the only register is the FP AND the function has a stack frame:
    //     Set FP to 0, eval, and create a specific StackLoc.
    //   If the only register is the SP:
    //     If we know the contents of SP:
    //       Eval and create a specific StackLoc
    //     Else create a generic StackLoc.
    //   If a non-stack register is used:
    //     Create a generic MemLoc.

    long spHeight = 0;
    int spRegion = 0;
    bool stackExists = getCurrentStackHeight(addr, spHeight, spRegion);

    bool frameExists = getCurrentFrameStatus(addr);

    bool isStack = false;
    bool isFrame = false;

    static RegisterAST *spRegs32[2] = {NULL, NULL};
    static RegisterAST *fpRegs32[2] = {NULL, NULL};

    static RegisterAST *spRegs64[2] = {NULL, NULL};
    static RegisterAST *fpRegs64[2] = {NULL, NULL};

    if (spRegs32[0] == NULL) {
        spRegs32[0] = new RegisterAST(r_eSP);
        spRegs32[1] = new RegisterAST(r_ESP);

        spRegs64[0] = new RegisterAST(r_rSP);
        spRegs64[1] = new RegisterAST(r_RSP);

        fpRegs32[0] = new RegisterAST(r_eBP);
        fpRegs32[1] = new RegisterAST(r_EBP);

        fpRegs64[0] = new RegisterAST(r_rBP);
        fpRegs64[1] = new RegisterAST(r_RBP);
    }

    // We currently have to try and bind _every_ _single_ _alias_
    // of the stack pointer...
    if (stackExists) {
        if (exp->bind(spRegs32[0], Result(u32, spHeight)) ||
            exp->bind(spRegs32[1], Result(u32, spHeight)) ||
            exp->bind(spRegs64[0], Result(u64, spHeight)) ||
            exp->bind(spRegs64[1], Result(u64, spHeight))) {
            isStack = true;
        }
    }
    if (frameExists) {
        if (exp->bind(fpRegs32[0], Result(u32, 0)) ||
            exp->bind(fpRegs32[1], Result(u32, 0)) ||
            exp->bind(fpRegs64[0], Result(u64, 0)) ||
            exp->bind(fpRegs64[1], Result(u64, 0))) {
            isFrame = true;
        }
    }

    Result res = exp->eval();

    if (!res.defined)
        return MemLoc::getMemLoc();
    
    Address resAddr;
    if (!convertResultToAddr(res, resAddr))
        return MemLoc::getMemLoc();

    if (isStack)
        return StackLoc::getStackLoc(resAddr, spRegion);
    
    // Frame-based accesses are always from region 0...
    if (isFrame)
        return StackLoc::getStackLoc(resAddr, 0);

    return MemLoc::getMemLoc(resAddr);
}


// Things are a lot easier if we know it's a register...
Absloc::Ptr DDGAnalyzer::getAbsloc(const InstructionAPI::RegisterAST::Ptr reg) {
    return RegisterLoc::getRegLoc(reg);
}

void DDGAnalyzer::getUsedAbslocs(const InsnPtr insn,
                                 Address addr,
                                 AbslocSet &uses) {
    std::set<RegisterAST::Ptr> regReads;
    insn->getReadSet(regReads);

    // Registers are nice and easy. The next clause is for memory... now
    // that sucks.

    for (std::set<RegisterAST::Ptr>::const_iterator r = regReads.begin();
         r != regReads.end();
         ++r) {
        // We have 'used' this Absloc
        uses.insert(getAbsloc(*r));
    }

    // Also handle memory writes
    if (insn->readsMemory()) {
        std::set<Expression::Ptr> memReads;
        insn->getMemoryReadOperands(memReads);
        for (std::set<Expression::Ptr>::const_iterator r = memReads.begin();
             r != memReads.end();
             ++r) {
            uses.insert(getAbsloc(*r, addr));
        }
    }
}

void DDGAnalyzer::getDefinedAbslocsInt(const InsnPtr insn,
                                       Address addr,
                                       DefSet &defs) {
    std::set<RegisterAST::Ptr> regWrites;
    insn->getWriteSet(regWrites);            

    // Registers are nice and easy. The next clause is for memory... now
    // that sucks.
    
    for (std::set<RegisterAST::Ptr>::const_iterator w = regWrites.begin();
         w != regWrites.end();
         ++w) {
        Absloc::Ptr a = getAbsloc(*w);
        RegisterLoc::Ptr r = dynamic_pointer_cast<RegisterLoc>(a);
        if (r->isFlag())
            defs.flags.insert(a);
        else if (r->isPC()) {
            defs.sprs.insert(a);
            defs.defPC_ = true;
        }
        else 
            defs.gprs.insert(a);
    }

    if (!defs.defPC_) {
// InstructionAPI doesn't explicitly present PC writes in normal fall-through
// execution. We can safely assume that all instructions update the PC, though...
        defs.sprs.insert(RegisterLoc::makePC());
    }

    // Also handle memory writes
    if (insn->writesMemory()) {
        std::set<Expression::Ptr> memWrites;
        insn->getMemoryWriteOperands(memWrites);
        for (std::set<Expression::Ptr>::const_iterator w = memWrites.begin();
             w != memWrites.end();
             ++w) {
            // A memory write. Who knew?
            Absloc::Ptr A = getAbsloc(*w, addr);
            defs.mem.insert(A);
        }
    }
}

bool DDGAnalyzer::getCurrentStackHeight(Address addr,
                                        long &height,
                                        int &region) {
    StackAnalysis sA(func_->lowlevel_func()->ifunc());
    const Dyninst::StackAnalysis::HeightTree *hT = sA.heightIntervals();
    
    StackAnalysis::Height heightSA;
    
    Offset off = func_->lowlevel_func()->addrToOffset(addr);
    
    if (!hT->find(off, heightSA)) {
        return false;
    }

    // Ensure that analysis has been performed.
    assert(!heightSA.isTop());

    if (heightSA.isBottom()) {
        return false;
    }

    height = heightSA.height();
    region = heightSA.region()->name();

    return true;
}

bool DDGAnalyzer::convertResultToAddr(const InstructionAPI::Result &res, Address &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (Address) res.val.u8val;
        return true;
    case s8:
        addr = (Address) res.val.s8val;
        return true;
    case u16:
        addr = (Address) res.val.u16val;
        return true;
    case s16:
        addr = (Address) res.val.s16val;
        return true;
    case u32:
        addr = (Address) res.val.u32val;
        return true;
    case s32:
        addr = (Address) res.val.s32val;
        return true;
    case u48:
        addr = (Address) res.val.u48val;
        return true;
    case s48:
        addr = (Address) res.val.s48val;
        return true;
    case u64:
        addr = (Address) res.val.u64val;
        return true;
    case s64:
        addr = (Address) res.val.s64val;
        return true;
    default:
        return false;
    }
}

bool DDGAnalyzer::convertResultToSlot(const InstructionAPI::Result &res, int &addr) {
    assert(res.defined);
    switch (res.type) {
    case u8:
        addr = (int) res.val.u8val;
        return true;
    case s8:
        addr = (int) res.val.s8val;
        return true;
    case u16:
        addr = (int) res.val.u16val;
        return true;
    case s16:
        addr = (int) res.val.s16val;
        return true;
    case u32:
        addr = (int) res.val.u32val;
        return true;
    case s32:
        addr = (int) res.val.s32val;
        return true;
        // I'm leaving these in because they may get used, but
        // we're definitely truncating them down.
    case u48:
        addr = (int) res.val.u48val;
        return true;
    case s48:
        addr = (int) res.val.s48val;
        return true;
    case u64:
        addr = (int) res.val.u64val;
        return true;
    case s64:
        addr = (int) res.val.s64val;
        return true;
    default:
        return false;
    }
}

bool DDGAnalyzer::getCurrentFrameStatus(Address addr) {
    StackAnalysis sA(func_->lowlevel_func()->ifunc()); 
    
    const Dyninst::StackAnalysis::PresenceTree *pT = sA.presenceIntervals();
    Dyninst::StackAnalysis::Presence exists;

    Offset off = func_->lowlevel_func()->addrToOffset(addr);

    if (!pT->find(off, exists)) return false;

    assert(!exists.isTop());

    return (exists.presence() == StackAnalysis::Presence::frame_t);
}

/**********************************************************
 ********* Utility functions ******************************
 **********************************************************/

void DDGAnalyzer::getPredecessors(Block *block,
                                  BlockSet &preds) {
    std::vector<BPatch_edge *> incEdges;
    block->getIncomingEdges(incEdges);
    for (unsigned i = 0; i < incEdges.size(); ++i) {
        preds.insert(incEdges[i]->getSource());
    }
}

void DDGAnalyzer::getSuccessors(Block *block,
                                BlockSet &succs) {
    std::vector<BPatch_edge *> outEdges;
    block->getOutgoingEdges(outEdges);
    for (unsigned i = 0; i < outEdges.size(); ++i) {
        succs.insert(outEdges[i]->getTarget());
    }
}

Node::Ptr DDGAnalyzer::makeNode(const cNode &cnode) {
    // We have our internal information about a graph node;
    // now make a real node from it. 

    if (nodeMap.find(cnode) == nodeMap.end()) {
        switch(cnode.type) {
        case normal:
            nodeMap[cnode] = OperationNode::createNode(cnode.addr, cnode.absloc);
            break;
        case formalParam: {
            Node::Ptr n = FormalParamNode::createNode(cnode.absloc);
            nodeMap[cnode] = n;
            ddg->insertFormalParamNode(n);
            break;
        }
        case formalReturn: {
            Node::Ptr n = FormalReturnNode::createNode(cnode.absloc);
            nodeMap[cnode] = n;
            ddg->insertFormalReturnNode(n);
            break;
        }
        case actualParam: {
            Node::Ptr n = ActualParamNode::createNode(cnode.addr, cnode.func, cnode.absloc);
            nodeMap[cnode] = n;
            ddg->insertActualParamNode(n);
            break;
        }
        case actualReturn: {
            Node::Ptr n = ActualReturnNode::createNode(cnode.addr, cnode.func, cnode.absloc);
            nodeMap[cnode] = n;
            ddg->insertActualReturnNode(n);
            break;
        }
        default:
            assert(0);
            break;
        }
    }
    return nodeMap[cnode];
}

bool DDGAnalyzer::isCall(InsnPtr i) const {
    entryID what = i->getOperation().getID();
    return (what == e_call);
}
 
bool DDGAnalyzer::isReturn(InsnPtr i) const {
    entryID what = i->getOperation().getID();
    return ((what == e_ret_far) ||
            (what == e_ret_near));
}

const DDGAnalyzer::DefSet &DDGAnalyzer::getDefinedAbslocs(const InsnPtr insn,
                                                          const Address &a) {
    if (defCache.find(a) == defCache.end()) {
        assert(defCache.find(a) == defCache.end());
        getDefinedAbslocsInt(insn, a, defCache[a]);
    }
    return defCache[a];
}

const DDGAnalyzer::AbslocSet &DDGAnalyzer::getUsedAbslocs(const InsnPtr insn,
                                                          const Address &a) {
    if (globalUsed.find(a) == globalUsed.end()) {
        getUsedAbslocs(insn, a, globalUsed[a]);
    }
    return globalUsed[a];
}

                                              
DDGAnalyzer::Function *DDGAnalyzer::getCallee(const Address &a) {
    // This is hardcore BPatch_function specific. FIXME...

    std::vector<BPatch_point *> *points = func_->findPoint(BPatch_subroutine);
    for (unsigned i = 0; i < points->size(); ++i) {
        if ((*points)[i]->getAddress() == (void *) a) {
            return (*points)[i]->getCalledFunction();
        }
    }
    return NULL;
}

InstructionAPI::RegisterAST::Ptr DDGAnalyzer::makeRegister(int id) {
    return InstructionAPI::RegisterAST::Ptr(new InstructionAPI::RegisterAST(id));
}


/**********************************************************
 ********* Debug functions ********************************
 **********************************************************/

void DDGAnalyzer::debugAbslocSet(const AbslocSet &a,
                                 char *str) {
    fprintf(stderr, "%s Abslocs:\n", str);
    for (AbslocSet::const_iterator iter = a.begin();
         iter != a.end();
         ++iter) {
        fprintf(stderr, "%s\t %s\n", str, (*iter)->format().c_str());
    }
}

void DDGAnalyzer::debugDefMap(const DefMap &d,
                              char *str) {
    fprintf(stderr, "%s Abslocs:\n", str);
    for (DefMap::const_iterator i = d.begin();
         i != d.end();
         ++i) {
        fprintf(stderr, "%s\t%s\n", 
                str, 
                (*i).first->format().c_str());
        for (cNodeSet::const_iterator j = (*i).second.begin();
             j != (*i).second.end(); ++j) {
            const cNode &c = (*j);
            fprintf(stderr, "%s\t\t%s, 0x%lx\n",
                    str,
                    c.absloc->format().c_str(),
                    c.addr);
        }
    }
}

void DDGAnalyzer::debugLocalSet(const DefMap &s,
                                char *str) {
    for (DefMap::const_iterator iter = s.begin();
         iter != s.end(); 
         ++iter) {
        fprintf(stderr, "%s Absloc: %s\n", str, (*iter).first->format().c_str());
        for (cNodeSet::const_iterator iter2 = (*iter).second.begin();
             iter2 != (*iter).second.end();
             ++iter2) {
            Address addr = (*iter2).addr;
            AbslocPtr absloc = (*iter2).absloc;
            fprintf(stderr, "%s\t insn addr 0x%lx, Absloc %s\n", 
                    str, 
                    addr,
                    absloc->format().c_str());
        }
    }
}

void DDGAnalyzer::debugBlockDefs() {
    for (ReachingDefsGlobal::iterator iter = inSets.begin(); iter != inSets.end(); iter++) {
        fprintf(stderr, "Block 0x%lx (IN)\n", iter->first->getStartAddress());
        debugDefMap(iter->second, "\t");
    }
    for (ReachingDefsGlobal::iterator iter = outSets.begin(); iter != outSets.end(); iter++) {
        fprintf(stderr, "Block 0x%lx (OUT)\n", iter->first->getStartAddress());
        debugDefMap(iter->second, "\t");
    }
}

//////////////////////////////////
//

void DDGAnalyzer::handlePushEquivalent(Address addr,
                                       Absloc::Ptr read,
                                       RegisterAST::Ptr sp,
                                       NodeMap &worklist) {
    assert(sp);
    // Stack pointer...
    AbslocPtr aSP = getAbsloc(sp);
    // And top of the stack. 
    Absloc::Ptr aStack = getAbsloc(sp, addr);
    
    // Okay, now what we're writing. We have two: *SP and SP. 
    // We can get those pretty easily, since we already have the SP
    // register from the above. 
    
    
    // Now do the graphlet. We have the worklist map for "hook this up in 
    // the future". That's nice. 
    
    Node::Ptr nSP = makeNode(cNode(addr, aSP));
    Node::Ptr nStack = makeNode(cNode(addr, aStack));
    
    worklist[aSP].push_back(nSP);
    if (read) {
        // If we push an immediate this may be nothing...
        worklist[read].push_back(nStack);
    }
    ddg->insertPair(nSP, nStack);
}

void DDGAnalyzer::handlePopEquivalent(Address addr,
                                      RegisterAST::Ptr writtenReg,
                                      RegisterAST::Ptr sp,
                                      NodeMap &worklist) {
    assert(writtenReg);
    assert(sp);
    // Stack pointer...
    AbslocPtr aSP = getAbsloc(sp);
    // Read register...
    AbslocPtr aReg = getAbsloc(writtenReg);
    // And top of the stack. 
    Absloc::Ptr aStack = getAbsloc(sp, addr);
    

    // We're reading aStack and aSP to write aReg;
    // also, reading aSP to write aSP. 
    // No intra- definitions.
    
    // Now do the graphlet. We have the worklist map for "hook this up in 
    // the future". That's nice. 
    
    Node::Ptr nSP = makeNode(cNode(addr, aSP));
    Node::Ptr nReg = makeNode(cNode(addr, aReg));
    
    worklist[aSP].push_back(nSP);
    worklist[aSP].push_back(nReg);
    worklist[aStack].push_back(nReg);
}
