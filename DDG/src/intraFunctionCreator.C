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
#include <list>
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

void intraFunctionDDGCreator::initializeGenSets(std::set<Block *> &allBlocks) {
    assert(allGens.empty());

    cerr << "Initializing gen sets..." << endl;
    
    for (std::set<Block *>::iterator iter = allBlocks.begin();
         iter != allBlocks.end(); 
         iter++) {
        Block *curBlock = *iter;
        // GEN: all abstract locations defined in this block
        // KILL: implicit; we kill all other such locations.
        
        // First, check to see if we've already traversed this block. That would
        // be... bad.
        
        cerr << "Block " << *iter << endl;

        if (allGens.find(curBlock) != allGens.end()) {
            fprintf(stderr, "BITCH MOAN WHINE!!!!!\n"); 
            continue;
        }
        
        std::vector<std::pair<Address,Instruction> > insns;
        curBlock->getInstructions(insns);
        
        for (unsigned i = 0; i < insns.size(); i++) {
            fprintf(stderr, "Instruction %d\n", i+1);
            std::set<RegisterAST::Ptr> cur_written;
            insns[i].second.getWriteSet(cur_written);
            
            for (std::set<RegisterAST::Ptr>::const_iterator w = cur_written.begin();
                 w != cur_written.end();
                 w++) {
                // We have 'defined' this Absloc
                AbslocPtr aP = Absloc::getAbsloc(*w);
                // So record that this instance (insns[i]) defined this
                // absloc...
                // Note that this overrides any previous insn, as the last
                // definition wins for our purposes.
                (allGens[curBlock])[aP] = std::make_pair(aP,InsnInstance(insns[i]));
            }
        }
    }

    cerr << allGens.size() << " gen sets created" << endl;
    
    assert(allGens.size() == allBlocks.size());
}

void intraFunctionDDGCreator::merge(ReachingDefsLocal &target,
                                    const ReachingDefsLocal &source) {
    // For each absloc A in Source
    //   target[A] = target[A] U source[A]
    for (ReachingDefsLocal::const_iterator iter = source.begin();
         iter != source.end();
         iter++) {
        AbslocPtr A = (*iter).first;
        
        target[A].insert((*iter).second.begin(),
                         (*iter).second.end());
    }
}

void intraFunctionDDGCreator::buildDDG() {
    
    // We build the DDG from reaching definitions performed for
    // all points (instructions) and program variables. 
    // 
    // Reminder of reaching definitions:
    // Forward flow dataflow analysis
    // IN(i,a) = U (j \in pred) OUT(j,a)
    // OUT(i,a) = GEN(i,a) U (IN(i,a) - KILL(i,a))
    // GEN(i,a) = IF a \in defs(i) then {i} ELSE {}
    // KILL(i,a) = IF a \in defs(i) then (DEFS(a) - {i}) ELSE {}
    // ... where DEFS(a) are the set of currently reaching definitions to a.

    // For complexity and efficiency, we perform the initial analysis on
    // basic blocks and then do intra-block analysis to build the insn-level
    // DDG. 

    Flowgraph *CFG = func->getCFG();

    // Initialize GEN and KILL sets for all basic blocks.
    std::set<Block *> allBlocks;
    CFG->getAllBasicBlocks(allBlocks);

    initializeGenSets(allBlocks);

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

void intraFunctionDDGCreator::generateInterBlockReachingDefs(Flowgraph *CFG) {
    std::vector<BPatch_basicBlock *> entryBlocks;
    CFG->getEntryBasicBlock(entryBlocks);
    BPatch_basicBlock *entryBlock = entryBlocks[0];

    std::list<BPatch_basicBlock *> worklist;

    worklist.push_back(entryBlock);

    while (!worklist.empty()) {
        Block *working = worklist.front();
        worklist.pop_front();

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
        calcNewOut(newOut, working, allGens[working], newIn);
        fprintf(stderr, "\t old out size: %d; new out size: %d\n", outSets[working].size(), newOut.size());
        if (newOut != outSets[working]) {
            fprintf(stderr, "\t Sets not equal, adding successors to block list\n");
            debugLocalSet(newOut, "\t");
            outSets[working] = newOut;
            std::vector<Block *> successors;
            getSuccessors(working, successors);
            for (unsigned i = 0; i < successors.size(); i++) {
                worklist.push_back(successors[i]);
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
    //       Let used = abslocs I uses to define A.
    //       For each absloc U in used:
    //         If localReachingDefs[U] is not NULL
    //           then node S = localReachingDefs[U]
    //             Insert(S,T)
    //           else let RDS = inSets[B][U]
    //             For each pair (B', S) in RDS:
    //               Insert(S,T)
    //       localReachingDefs[D] = T
    
    for (BlockSet::iterator b_iter = allBlocks.begin();
         b_iter != allBlocks.end();
         b_iter++) {
        Block *B = *b_iter;
        std::vector<std::pair<Address, Instruction> > insns;
        B->getInstructions(insns);
        NodeDefMap localReachingDefs;
        
        for (unsigned i = 0; i < insns.size(); i++) {
            Address addr = insns[i].first;
            Instruction I = insns[i].second;

            std::set<RegisterAST::Ptr> def;
            I.getWriteSet(def);
            
            for (std::set<RegisterAST::Ptr>::const_iterator d_iter = def.begin();
                 d_iter != def.end(); d_iter++) {
                AbslocPtr D = Absloc::getAbsloc(*d_iter);
                NodePtr T = DDG->makeNode(I, addr, D);
                std::set<RegisterAST::Ptr> used;
                //I.getUsedSet(*d_iter, used);
                I.getReadSet(used);

                for (std::set<RegisterAST::Ptr>::const_iterator u_iter = used.begin();
                     u_iter != used.end(); u_iter++) {
                    Absloc::Ptr U = Absloc::getAbsloc(*u_iter);
                    
                    if (localReachingDefs.find(U) != localReachingDefs.end()) {
                        NodePtr S = localReachingDefs[U];
                        // By definition we know S is in nodes
                        DDG->insertPair(S,T);
                    }
                    else { 
                        ReachingDefSet RDS = inSets[B][U];
                        for (ReachingDefSet::iterator r_iter = RDS.begin();
                             r_iter != RDS.end(); r_iter++) {
                            ReachingDefEntry entry = *r_iter;
                            NodePtr S = makeNodeFromCandidate(entry.second);
                            DDG->insertPair(S,T);
                        }
                    }
                } // For U in used
                localReachingDefs[D] = T;
            } // For D in def
        } // For I in insn
    } // For B in block
}


void intraFunctionDDGCreator::calcNewOut(ReachingDefsLocal &out,
                                         Block *current,
                                         DefMap &gens,
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
        if (gens.find(A) != gens.end()) {
            // Generated locally, so KILL the 
            // set from in and use GEN
            ReachingDefEntry entry = std::make_pair<Block *, cNode>(current, gens[A]);
            ReachingDefSet dummy;
            dummy.insert(entry);
            out[A] = dummy;
            fprintf(stderr, "\t\tAbsloc %s redefined at current block\n", A->name().c_str());
        }
        else {
            // Not generated locally, so pass through.
            fprintf(stderr, "\t\tAbsloc %s not defined, passing through\n", A->name().c_str());
            out[A] = in[A];
        }
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
            fprintf(stderr, "%s\t Block: %ld, insn addr 0x%lx, Absloc %s\n", 
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
    for (DefMap::const_iterator iter = d.begin();
         iter != d.end();
         iter++) {
        fprintf(stderr, "%s\t %s: 0x%lx\n", 
                str, 
                (*iter).first->name().c_str(), 
                (*iter).second.second.addr);
    }
}

