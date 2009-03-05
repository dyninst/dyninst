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
 * This license is for researczh uses.  For such uses, there is no
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

#include <map>
#include <algorithm>
#include <queue>
#include "Absloc.h"
#include "intraFunctionCreator.h"

// Dyninst!
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_edge.h"

// InstructionAPI
#include "Instruction.h"

// Example intra-function DDG creator
// Heavily borrows from the Dyninst internal liveness.C file
// Performed over BPatch objects (for now) but designed
// to be pretty agnostic of inputs. 

// TODO
//   Handle calls
//     Trust the ABI to start
//     Analyze the callee function
//   AMD-64
//   Interprocedural

using namespace Dyninst;
using namespace Dyninst::DDG;
using namespace Dyninst::InstructionAPI;
using namespace std;

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

void intraFunctionDDGCreator::buildDDG() {
    
    // We build the DDG from reaching definitions performed for
    // all points (instructions) and program variables. 

    // Create us a DDG
    DDG = Graph::createGraph();

    // For complexity and efficiency, we perform the initial analysis on
    // basic blocks and then do intra-block analysis to build the insn-level
    // DDG. 

    Flowgraph *CFG = func->getCFG();

    // Initialize GEN and KILL sets for all basic blocks.
    std::set<Block *> allBlocks;
    CFG->getAllBasicBlocks(allBlocks);

    // Create the GEN (generated) set for each basic block. 
    initializeGenKillSets(allBlocks);

    // We now have GEN for each block. Propagate reaching
    // definitions.

    // Generate the reaching defs for each block
    generateInterBlockReachingDefs(CFG);

    // inSets now contains the following:
    // FOR EACH Absloc A,
    //   The list of blocks with reaching definitions of A

    // We want to build the subnode-level graph.
    generateIntraBlockReachingDefs(allBlocks);
}

// Copied from above
// GEN(i,X) = 
//  if X = S_i : S_i = {i}, S = {i}
//  if X = X   : S = {i}, S_1 = {i}, ..., S_n = {i}
// KILL(i,X) = 
//  if X = S_i : S_i = {i}^c, S = 0
//  if X = S   : 0

void intraFunctionDDGCreator::initializeGenKillSets(std::set<Block *> &allBlocks) {
    assert(allGens.empty());

    for (std::set<Block *>::iterator iter = allBlocks.begin();
         iter != allBlocks.end(); 
         iter++) {
        Block *curBlock = *iter;

        std::vector<std::pair<Instruction,Address> > insns;
        curBlock->getInstructions(insns);
        
        for (unsigned i = 0; i < insns.size(); i++) {
            AbslocSet writtenAbslocs;
            Absloc::getDefinedAbslocs(insns[i].first, func, insns[i].second, writtenAbslocs);

            for (AbslocSet::iterator iter = writtenAbslocs.begin();
                 iter != writtenAbslocs.end();
                 iter++) {                
                // We have two cases: if the absloc is precise or an alias (S_i vs S above)
                Absloc::Ptr A = *iter;
                cNode cnode = std::make_pair(A, InsnInstance(insns[i]));

                AbslocSet aliases = A->getAliases();

                if (A->isPrecise()) {
                    // S_i case...
                    // OUT = GEN U (IN - KILL)
                    // OUT[S_i] = {i} (as KILL = ALL)
                    allGens[curBlock][A].clear(); // apply KILL
                    allGens[curBlock][A].insert(cnode);

                    // We also record that this block kills this absLoc.
                    // It doesn't matter which instruction does it, since
                    // that will be summarized in the gen information.
                    allKills[curBlock][A] = true;
                }
                for (AbslocSet::iterator al = aliases.begin();
                     al != aliases.end(); al++) {
                    // This handles both the S case if we have a precise
                    // absloc, as well as S_1, ..., S_n if we have an imprecise
                    // absloc. 
                    allGens[curBlock][*al].insert(cnode);
                }
            }
        }
    }
}

void intraFunctionDDGCreator::merge(ReachingDefsLocal &target,
                                    ReachingDefsLocal &source) {
    // See optimization note at the top of the file. 
    //
    // For each absloc A in target
    //   If source.defines[A]
    //     target[A] = target[A] U source[A]
    //   Else
    //       (note: we have a forward definition at this point)
    //     Let aliases = a.Aliases
    //     For each a' in aliases
    //       target[A] = target[A] U source[a']

    for (ReachingDefsLocal::iterator iter = target.begin(); 
         iter != target.end();
         iter++) {
        AbslocPtr A = (*iter).first;
        if (source.find(A) != source.end()) {
            target[A].insert(source[A].begin(), source[A].end());
        }
        else {
            // See if we have an aliasing set for A...
            AbslocSet aliases = A->getAliases();
            for (AbslocSet::iterator a_iter = aliases.begin(); 
                 a_iter != aliases.end(); a_iter++) {
                target[A].insert(source[*a_iter].begin(), 
                                 source[*a_iter].end());
            }
        }
    }
}


void intraFunctionDDGCreator::generateInterBlockReachingDefs(Flowgraph *CFG) {
    std::vector<BPatch_basicBlock *> entryBlocks;
    CFG->getEntryBasicBlock(entryBlocks);
    BPatch_basicBlock *entryBlock = entryBlocks[0];

    std::queue<BPatch_basicBlock *> worklist;

    worklist.push(entryBlock);

    while (!worklist.empty()) {
        Block *working = worklist.front();
        worklist.pop();

        // Calculate the new in set 

        std::vector<BPatch_basicBlock *> preds;
        getPredecessors(working, preds);
        
        // NEW_IN = U (j \in pred) OUT(j,a)
        
        ReachingDefsLocal newIn;
        for (unsigned i = 0; i < preds.size(); i++) {
            merge(newIn, outSets[preds[i]]);
        }
        // Now: newIn = U (j \in pred) OUT(j)
        
        // OUT(i,a) = GEN(i,a) U (IN(i,a) - KILL(i,a))
        ReachingDefsLocal newOut;
        calcNewOut(newOut, working, 
                   allGens[working], 
                   allKills[working],
                   newIn);

        if (newOut != outSets[working]) {
            outSets[working] = newOut;
            std::vector<Block *> successors;
            getSuccessors(working, successors);
            for (unsigned i = 0; i < successors.size(); i++) {
                worklist.push(successors[i]);
            }
        }
    }    
}

void intraFunctionDDGCreator::generateIntraBlockReachingDefs(BlockSet &allBlocks) {
    // We have a set of inter-block reaching definitions. We now build the graph
    // of intra-block reaching defs. 
    
    // Algorithmically:
    // For each block B:
    //   Let localReachingDefs : Absloc -> Node be an empty map
    //   For each instruction instance i in B:
    //     Let def = i.defines();
    //     For each absloc D in def:
    //       Let T = NODE(I,D)
    //       Let used = abslocs I uses to define D.
    //       For each absloc U in used:
    //         Let Aliases = aliases to U.
    //         For each absloc A in Aliases
    //           If localReachingDefs[A] is defined
    //             then node S = localReachingDefs[A]
    //               Insert(S,T)
    //             else let RDS = inSets[B][A]
    //               For each pair (B', S) in RDS:
    //                 Insert(S,T)
    //     For each absloc D in def:
    //       localReachingDefs[D] = NODE(I,D)
    // See comment below for why we do this in two iterations.

    for (BlockSet::iterator b_iter = allBlocks.begin();
         b_iter != allBlocks.end();
         b_iter++) {
        Block *B = *b_iter;
        std::vector<std::pair<Instruction, Address> > insns;
        B->getInstructions(insns);
        NodeDefMap localReachingDefs;
        
        for (unsigned i = 0; i < insns.size(); i++) {
            Instruction I = insns[i].first;
            Address addr = insns[i].second;

            AbslocSet used;
            Absloc::getUsedAbslocs(I, func, addr, used);
            AbslocSet def;
            Absloc::getDefinedAbslocs(I, func, addr, def);

            for (AbslocSet::const_iterator d_iter = def.begin();
                 d_iter != def.end(); d_iter++) {
                AbslocPtr D = *d_iter;
                NodePtr T = DDG->makeNode(I, addr, D);

                // Get the set of abslocs we have to care about here..

                // TODO: used_to_define...
                // And move the getUsedAbslocs to here...
                
                for (AbslocSet::const_iterator u_iter = used.begin();
                     u_iter != used.end(); u_iter++) {
                    Absloc::Ptr U = *u_iter;

                    if (localReachingDefs.find(U) != localReachingDefs.end()) {
                        NodePtr S = localReachingDefs[U];
                        // By definition we know S is in nodes
                        DDG->insertPair(S,T);
                    }
                    else { 
                        ReachingDefSet RDS = inSets[B][U];
                        if (RDS.empty()) {
                            // An empty reachingDefSet means that we're an
                            // using an undefined value; assume that
                            // our caller set it.
                            NodePtr S = DDG->makeParamNode(U);
                            DDG->insertPair(S,T);
                        }
                        else {
                            for (ReachingDefSet::iterator r_iter = RDS.begin();
                                 r_iter != RDS.end(); r_iter++) {
                                ReachingDefEntry entry = *r_iter;
                                NodePtr S = makeNodeFromCandidate(entry.second);
                                DDG->insertPair(S,T);
                            }
                        }
                    }
                } // For U in used
            } // For D in def
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
            for (AbslocSet::const_iterator d_iter = def.begin();
                 d_iter != def.end(); d_iter++) {
                AbslocPtr D = *d_iter;
                NodePtr T = DDG->makeNode(I, addr, D);
                localReachingDefs[D] = T;
            }
        } // For I in insn
    } // For B in block
}


void intraFunctionDDGCreator::calcNewOut(ReachingDefsLocal &out,
                                         Block *current,
                                         DefMap &gens,
                                         KillMap &kills,
                                         ReachingDefsLocal &in) {
    // OUT = GEN U (IN - KILL)

    // We get KILL implicitly from GEN. In effect:
    // FOREACH A \in Absloc, DO
    //   IF A \in GEN
    //     THEN OUT[A] = {this}
    //     ELSE OUT[A] = IN[A]

    // Calculate the set Absloc by taking the union
    // of gens and {in.first}
    AbslocSet definedAbslocs;
    for (DefMap::const_iterator iter = gens.begin();
         iter != gens.end();
         iter++) {
        definedAbslocs.insert((*iter).first);
    }            

    for (ReachingDefsLocal::const_iterator iter = in.begin(); 
         iter != in.end();
         iter++) {
        definedAbslocs.insert((*iter).first);
    }

    // Calculate the new OUT set
    for (AbslocSet::iterator iter = definedAbslocs.begin();
         iter != definedAbslocs.end();
         iter++) {
        Absloc::Ptr A = *iter;

        // If we kill this AbslocPtr within this block, then
        // take the entry from the GEN set only.
        if (kills.find(A) != kills.end()) {
            genSetToReachingDefs(current, gens[A], out[A]);
        }
        else {
            // We don't explicitly kill this, so take the union
            // of local generation with the INs. 
            genSetToReachingDefs(current, gens[A], out[A]);
            out[A].insert(in[A].begin(), in[A].end());
        }
    }
}

void intraFunctionDDGCreator::genSetToReachingDefs(Block *current,
                                                   const cNodeSet &gens,
                                                   ReachingDefSet &defs) {
    // This is strictly an impedance matching function.
    for (cNodeSet::const_iterator iter = gens.begin(); 
         iter != gens.end();
         iter++) {
        defs.insert(std::make_pair<Block *, cNode>(current, *iter));
    }
}


void intraFunctionDDGCreator::getPredecessors(BPatch_basicBlock *block,
                                              std::vector<BPatch_basicBlock *> &preds) {
    std::vector<BPatch_edge *> incEdges;
    block->getIncomingEdges(incEdges);
    for (unsigned i = 0; i < incEdges.size(); i++) {
        preds.push_back(incEdges[i]->getSource());
    }
}

void intraFunctionDDGCreator::getSuccessors(BPatch_basicBlock *block,
                                            std::vector<BPatch_basicBlock *> &succs) {
    std::vector<BPatch_edge *> outEdges;
    block->getOutgoingEdges(outEdges);
    for (unsigned i = 0; i < outEdges.size(); i++) {
        succs.push_back(outEdges[i]->getTarget());
    }
}

Graph::Ptr intraFunctionDDGCreator::createGraph(Function *func) {
    intraFunctionDDGCreator creator(func);

    creator.buildDDG();

    return creator.DDG;
}

void intraFunctionDDGCreator::debugLocalSet(const ReachingDefsLocal &s,
                                            char *str) {
    for (ReachingDefsLocal::const_iterator iter = s.begin();
         iter != s.end(); 
         iter++) {
        fprintf(stderr, "%s Absloc: %s\n", str, (*iter).first->name().c_str());
        for (ReachingDefSet::const_iterator iter2 = (*iter).second.begin();
             iter2 != (*iter).second.end();
             iter2++) {
            Block *block = (*iter2).first;
            Address addr = (*iter2).second.second.addr;
            AbslocPtr absloc = (*iter2).second.first;
            fprintf(stderr, "%s\t Block: %lx, insn addr 0x%lx, Absloc %s\n", 
                    str, 
                    block->getStartAddress(),
                    addr,
                    absloc->name().c_str());
        }
    }
}

void intraFunctionDDGCreator::debugAbslocSet(const AbslocSet &a,
                                             char *str) {
    fprintf(stderr, "%s Abslocs:\n", str);
    for (AbslocSet::const_iterator iter = a.begin();
         iter != a.end();
         iter++) {
        fprintf(stderr, "%s\t %s\n", str, (*iter)->name().c_str());
    }
}

void intraFunctionDDGCreator::debugDefMap(const DefMap &d,
                                          char *str) {
    fprintf(stderr, "%s Abslocs:\n", str);
    for (DefMap::const_iterator i = d.begin();
         i != d.end();
         i++) {
        fprintf(stderr, "%s\t%s\n", 
                str, 
                (*i).first->name().c_str());
        for (cNodeSet::const_iterator j = (*i).second.begin();
             j != (*i).second.end(); j++) {
            const cNode &c = (*j);
            fprintf(stderr, "%s\t\t%s, 0x%lx, %s\n",
                    str,
                    c.first->name().c_str(),
                    c.second.addr,
                    c.second.insn.format().c_str());
        }
    }
}

Node::Ptr intraFunctionDDGCreator::makeNodeFromCandidate(cNode cnode) {
    // We have our internal information about a graph node;
    // now make a real node from it. 

    // First, peel apart the cNode
    AbslocPtr absloc = cnode.first;
    InsnInstance insnI = cnode.second;
    Address addr = insnI.addr;
    Instruction insn = insnI.insn;

    return DDG->makeNode(insn, addr, absloc);
}
