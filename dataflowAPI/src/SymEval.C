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

#include <string>
#include <iostream>
#include <memory>

#include "../h/SymEval.h"
#include "SymEvalPolicy.h"

#include "DynAST.h"

#include "parseAPI/h/CFG.h"

#include "../rose/x86InstructionSemantics.h"
#include "../rose/x86_64InstructionSemantics.h"

#include "../rose/SgAsmInstruction.h"
#include "../h/stackanalysis.h"
#include "SymEvalVisitors.h"

#include "RoseInsnFactory.h"
#include "SymbolicExpansion.h"

#include "../h/Absloc.h"

#include "../h/slicing.h" // SliceNode

#include "debug_dataflow.h"

#include "boost/tuple/tuple.hpp"

using namespace std;
using namespace Dyninst;
using namespace InstructionAPI;
using namespace DataflowAPI;
using namespace rose::BinaryAnalysis::InstructionSemantics2;


std::pair<AST::Ptr, bool> SymEval::expand(const Assignment::Ptr &assignment, bool applyVisitors) {
    // This is a shortcut version for when we only want a
    // single assignment

    Result_t res;
    // Fill it in to mark it as existing
    res[assignment] = AST::Ptr();
    std::set<Instruction> ignored;
    bool succ = expand(res, ignored, applyVisitors);
    return std::make_pair(res[assignment], succ);
}

bool SymEval::expand(Result_t &res, 
        std::set<InstructionAPI::Instruction> &failedInsns,
        bool applyVisitors) {
    // Symbolic evaluation works off an Instruction
    // so we have something to hand to ROSE. 
    failedInsns.clear();
    for (Result_t::iterator i = res.begin(); i != res.end(); ++i) {
        if (i->second != AST::Ptr()) {
            // Must've already filled it in from a previous instruction crack
            continue;
        }
        Assignment::Ptr ptr = i->first;

        bool success = expandInsn(ptr->insn(),
                ptr->addr(),
                res);
        if (!success) failedInsns.insert(ptr->insn());
    }

    if (applyVisitors) {
        // Must apply the visitor to each filled in element
        for (Result_t::iterator i = res.begin(); i != res.end(); ++i) {
            if (!i->second) continue;
            AST::Ptr tmp = simplifyStack(i->second, i->first->addr(), i->first->func(), i->first->block());
            BooleanVisitor b;
            AST::Ptr tmp2 = tmp->accept(&b);
            i->second = tmp2;
        }
    }
    return (failedInsns.empty());
}

bool edgeSort(Edge::Ptr ptr1, Edge::Ptr ptr2) {
    Address addr1 = ptr1->target()->addr();
    Address addr2 = ptr2->target()->addr();

    return (addr1 < addr2);
}

void dfs(Node::Ptr source,
        std::map<Node::Ptr, int> &state,
        std::set<Edge::Ptr> &skipEdges) {

    // DFS from the node given by source
    // If we meet a node twice without having to backtrack first,
    // insert that incoming edge into skipEdges.
    //
    // A node n has state[n] > 0 if it is on the path currently
    // being explored.

    EdgeIterator b, e;
    source->outs(b, e);

    vector<Edge::Ptr> edges;
    for ( ; b!=e; ++b) {
        Edge::Ptr edge = *b;
        edges.push_back(edge);
    }
    std::stable_sort(edges.begin(), edges.end(), edgeSort);

    //state[source]++;
    std::map<Node::Ptr, int>::iterator ssit = state.find(source);
    if(ssit == state.end())
        boost::tuples::tie(ssit,boost::tuples::ignore) = 
            state.insert(make_pair(source,1));
    else
        (*ssit).second++;

    vector<Edge::Ptr>::iterator eit = edges.begin();
    for ( ; eit != edges.end(); ++eit) {
        Edge::Ptr edge = *eit;
        Node::Ptr cur = edge->target();

        std::map<Node::Ptr, int>::iterator sit = state.find(cur);
        bool done = (sit != state.end());

        if(done && (*sit).second > 0)
            skipEdges.insert(edge);

        if(!done)
            dfs(cur, state, skipEdges);
    }

    //state[source]--;
    (*ssit).second--;
}

/*
 * Optimal ordering for visiting the slicing
 * nodes during expansion; this is possible to do
 * because we have removed loops
 */
class ExpandOrder {
    public:
        ExpandOrder() { }
        ~ExpandOrder() { }

        // remove an element from the next-lowest queue
        // and return it and its order
        pair<SliceNode::Ptr,int> pop_next()
        {
            SliceNode::Ptr rn = SliceNode::Ptr();
            int ro = -1;

            map<int,order_queue>::iterator qit = queues.begin();
            for( ; qit != queues.end(); ++qit) {
                order_queue & q = (*qit).second;
                if(!q.nodes.empty()) {
                    rn = *q.nodes.begin();
                    ro = q.order; 
                    remove(rn);
                    break;
                }
            }
            return make_pair(rn,ro);
        }

        // removes a node from the structure
        // returns true if the node was there
        bool remove(SliceNode::Ptr n) {
            map<SliceNode::Ptr, int>::iterator it = order_map.find(n);
            if(it != order_map.end()) {
                queues[ (*it).second ].nodes.erase(n);
                order_map.erase(it);
                return true;
            } 
            return false;
        }

        // places a node in the structure -- its
        // order is computed
        void insert(SliceNode::Ptr n, bool force_done = false) {
            // compute the order of this node --- the number of its parents
            // not on the skipedges list and not done
            EdgeIterator begin, end;
            n->ins(begin,end);
            int pcnt = 0;
            for( ; begin != end; ++begin) {
                Edge::Ptr edge = *begin;
                if(skip_edges.find(edge) == skip_edges.end()) {
                    SliceNode::Ptr parent =
                        boost::static_pointer_cast<SliceNode>(
                                edge->source());
                    if(done.find(parent) == done.end())
                        ++pcnt;
                }
            }

            queues[pcnt].nodes.insert(n);
            queues[pcnt].order = pcnt;
            order_map[n] = pcnt;

            if(force_done)
                done.insert(n);
        }

        // Mark a node complete, updating its children.
        // Removes the node from the data structure
        void mark_done(SliceNode::Ptr n) {
            // First pull all of the children of this node
            // that are not on the skip list

            set<SliceNode::Ptr> children;

            EdgeIterator begin, end;
            n->outs(begin, end);
            for (; begin != end; ++begin) {
                Edge::Ptr edge = *begin;
                if(skip_edges.find(edge) == skip_edges.end()) {
                    SliceNode::Ptr child = 
                        boost::static_pointer_cast<SliceNode>(
                                edge->target());
                    if(remove(child))
                        children.insert(child);
                }
            }

            // remove n and set done
            remove(n);
            done.insert(n);

            // put the children back
            set<SliceNode::Ptr>::iterator cit = children.begin();
            for( ; cit != children.end(); ++cit) {
                insert(*cit); 
            }
        }

        bool is_done(SliceNode::Ptr n) const {
            return done.find(n) == done.end();
        }

        set<Edge::Ptr> & skipEdges() { return skip_edges; }

    private:
        struct order_queue {
            int order;
            set<SliceNode::Ptr> nodes; 
        };

        set<Edge::Ptr> skip_edges; 
        map<int,order_queue> queues;
        map<SliceNode::Ptr, int> order_map;
        set<SliceNode::Ptr> done;
};

// implements < , <= causes failures when used to sort Windows vectors
bool vectorSort(SliceNode::Ptr ptr1, SliceNode::Ptr ptr2) {

    AssignmentPtr assign1 = ptr1->assign();
    AssignmentPtr assign2 = ptr2->assign();

    if (!assign2) return false;
    else if (!assign1) return true;

    Address addr1 = assign1->addr();
    Address addr2 = assign2->addr();

    if (addr1 == addr2) {
        AbsRegion &out1 = assign1->out();
        AbsRegion &out2 = assign2->out();
        return out1 < out2;
    } else {
        return addr1 < addr2;
    }
}

// Do the previous, but use a Graph as a guide for
// performing forward substitution on the AST results
SymEval::Retval_t SymEval::expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res) {
    bool failedTranslation = false;
    bool skippedInput = false;

    //cout << "Calling expand" << endl;
    // Other than the substitution this is pretty similar to the first example.
    NodeIterator gbegin, gend;
    slice->allNodes(gbegin, gend);

    // First, we'll sort the nodes in some deterministic order so that the loop removal
    // is deterministic
    std::vector<SliceNode::Ptr> sortVector;
    for ( ; gbegin != gend; ++gbegin) {
        Node::Ptr ptr = *gbegin;
        expand_cerr << "pushing " << (*gbegin)->format() << " to sortVector" << endl;
        SliceNode::Ptr cur = boost::static_pointer_cast<SliceNode>(ptr);
        sortVector.push_back(cur);
    }
    std::stable_sort(sortVector.begin(), sortVector.end(), vectorSort);

    // Optimal ordering of search
    ExpandOrder worklist;

    std::queue<Node::Ptr> dfs_worklist;
    std::vector<SliceNode::Ptr>::iterator vit = sortVector.begin();
    for ( ; vit != sortVector.end(); ++vit) {
        SliceNode::Ptr ptr = *vit;
        Node::Ptr cur = boost::static_pointer_cast<Node>(ptr);
        dfs_worklist.push(cur);
    }

    /* First, we'll do DFS to check for circularities in the graph;
     * if so, mark them so we don't do infinite substitution */
    std::map<Node::Ptr, int> state;
    while (!dfs_worklist.empty()) {
        Node::Ptr ptr = dfs_worklist.front(); dfs_worklist.pop();
        dfs(ptr, state, worklist.skipEdges());
    }

    slice->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        expand_cerr << "adding " << (*gbegin)->format() << " to worklist" << endl;
        Node::Ptr ptr = *gbegin;
        SliceNode::Ptr sptr = 
            boost::static_pointer_cast<SliceNode>(ptr);
        worklist.insert(sptr,false);
    }

    /* have a list
     * for each node, process
     * if processessing succeeded, remove the element
     * if the size of the list has changed, continue */

    while (1) {
        SliceNode::Ptr aNode;
        int order;

        boost::tie(aNode,order) = worklist.pop_next();
        if (order == -1) // empty
            break;

        if (!aNode->assign()) {
            worklist.mark_done(aNode);
            continue; // Could be a widen point
        }

        expand_cerr << "Visiting node " << aNode->assign()->format() 
            << " order " << order << endl;

        if (order != 0) {
            cerr << "ERROR: order is non zero: " << order << endl;
        }
        assert(order == 0); // there are no loops

        AST::Ptr prev = res[aNode->assign()];
        Retval_t result = process(aNode, res, worklist.skipEdges()); 
        AST::Ptr post = res[aNode->assign()];
        switch (result) {
            case FAILED:
                return FAILED;
                break;
            case WIDEN_NODE:
                // Okay...
                break;
            case FAILED_TRANSLATION:
                failedTranslation = true;
                break;
            case SKIPPED_INPUT:
                skippedInput = true;
                break;
            case SUCCESS:
                break;
        }

        // We've visited this node, freeing its children
        // to be visited in turn
        worklist.mark_done(aNode);

        if (post && !(post->equals(prev))) {
            expand_cerr << "Adding successors to list, as new expansion " << endl
                << "\t" << post->format() << endl 
                << " != " << endl
                << "\t" << (prev ? prev->format() : "<NULL>") << endl;
            EdgeIterator oB, oE;
            aNode->outs(oB, oE);
            for (; oB != oE; ++oB) {
                if(worklist.skipEdges().find(*oB) == worklist.skipEdges().end()) {
                    SliceNode::Ptr out =
                        boost::static_pointer_cast<SliceNode>(
                                (*oB)->target());
                    worklist.insert(out);
                }
            }
        }
    }
    if (failedTranslation) return FAILED_TRANSLATION;
    else if (skippedInput) return SKIPPED_INPUT;
    else return SUCCESS;
}

bool SymEval::expandInsn(const Instruction &insn,
        const uint64_t addr,
        Result_t &res) {


    switch (insn.getArch()) {
        case Arch_x86: {
                           SymEvalPolicy policy(res, addr, insn.getArch(), insn);
                           RoseInsnX86Factory fac(Arch_x86);
                           auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                           if (!roseInsn) return false;

                           SymbolicExpansion exp;
                           exp.expandX86(roseInsn.get(), policy);
                           if (policy.failedTranslate()) {
                               cerr << "Warning: failed semantic translation of instruction " << insn.format() << endl;
                               return false;
                           }

                           break;
                       }
        case Arch_x86_64: {
                              SymEvalPolicy_64 policy(res, addr, insn.getArch(), insn);
                              RoseInsnX86Factory fac(Arch_x86_64);
                              auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                              if (!roseInsn) return false;

                              SymbolicExpansion exp;
                              exp.expandX86_64(roseInsn.get(), policy);
                              if (policy.failedTranslate()) {
                                  cerr << "Warning: failed semantic translation of instruction " << insn.format() << endl;
                                  return false;
                              }

                              break;

                          }
        case Arch_ppc32: {
                             RoseInsnPPCFactory fac;
                             auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                             if (!roseInsn) return false;

                             SymbolicExpansion exp;
                             const RegisterDictionary *reg_dict = RegisterDictionary::dictionary_powerpc();

                             BaseSemantics::SValuePtr protoval = SymEvalSemantics::SValue::instance(1, 0);
                             BaseSemantics::RegisterStatePtr registerState = SymEvalSemantics::RegisterStateASTPPC32::instance(protoval, reg_dict);
                             BaseSemantics::MemoryStatePtr memoryState = SymEvalSemantics::MemoryStateAST::instance(protoval, protoval);
                             BaseSemantics::StatePtr state = SymEvalSemantics::StateAST::instance(res, addr, insn.getArch(), insn, registerState, memoryState);
                             BaseSemantics::RiscOperatorsPtr ops = SymEvalSemantics::RiscOperatorsAST::instance(state);

                             exp.expandPPC32(roseInsn.get(), ops, insn.format());

                             break;
                         }
        case Arch_ppc64: {
                             RoseInsnPPCFactory fac;
                             auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                             if (!roseInsn) return false;

                             SymbolicExpansion exp;
                             const RegisterDictionary *reg_dict = RegisterDictionary::dictionary_powerpc();

                             BaseSemantics::SValuePtr protoval = SymEvalSemantics::SValue::instance(1, 0);
                             BaseSemantics::RegisterStatePtr registerState = SymEvalSemantics::RegisterStateASTPPC64::instance(protoval, reg_dict);
                             BaseSemantics::MemoryStatePtr memoryState = SymEvalSemantics::MemoryStateAST::instance(protoval, protoval);
                             BaseSemantics::StatePtr state = SymEvalSemantics::StateAST::instance(res, addr, insn.getArch(), insn, registerState, memoryState);
                             BaseSemantics::RiscOperatorsPtr ops = SymEvalSemantics::RiscOperatorsAST::instance(state);

                             exp.expandPPC64(roseInsn.get(), ops, insn.format());

                             break;

                         }
        case Arch_aarch64: {
                               RoseInsnArmv8Factory fac(Arch_aarch64);
                               auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                               if (!roseInsn) return false;

                               SymbolicExpansion exp;
                               const RegisterDictionary *reg_dict = RegisterDictionary::dictionary_armv8();

                               BaseSemantics::SValuePtr protoval = SymEvalSemantics::SValue::instance(1, 0);
                               BaseSemantics::RegisterStatePtr registerState = SymEvalSemantics::RegisterStateASTARM64::instance(protoval, reg_dict);
                               BaseSemantics::MemoryStatePtr memoryState = SymEvalSemantics::MemoryStateAST::instance(protoval, protoval);
                               BaseSemantics::StatePtr state = SymEvalSemantics::StateAST::instance(res, addr, insn.getArch(), insn, registerState, memoryState);
                               BaseSemantics::RiscOperatorsPtr ops = SymEvalSemantics::RiscOperatorsAST::instance(state);

                               exp.expandAarch64(roseInsn.get(), ops, insn.format());

                               break;
                           }
        case Arch_amdgpu_vega: {

                                   RoseInsnAmdgpuVegaFactory fac(Arch_amdgpu_vega);
                                   auto roseInsn = std::unique_ptr<SgAsmInstruction>(fac.convert(insn, addr));
                                   if (!roseInsn) return false;

                                   SymbolicExpansion exp;
                                   const RegisterDictionary *reg_dict = RegisterDictionary::dictionary_amdgpu_vega();

                                   BaseSemantics::SValuePtr protoval = SymEvalSemantics::SValue::instance(1, 0);
                                   BaseSemantics::RegisterStatePtr registerState = SymEvalSemantics::RegisterStateAST_AMDGPU_VEGA::instance(protoval, reg_dict);
                                   BaseSemantics::MemoryStatePtr memoryState = SymEvalSemantics::MemoryStateAST::instance(protoval, protoval);
                                   BaseSemantics::StatePtr state = SymEvalSemantics::StateAST::instance(res, addr, insn.getArch(), insn, registerState, memoryState);
                                   BaseSemantics::RiscOperatorsPtr ops = SymEvalSemantics::RiscOperatorsAST::instance(state);
                                   exp.expandAmdgpuVega(roseInsn.get(), ops, insn.format());

                                   break;
                               }
        default:
                               assert(0 && "Unimplemented symbolic expansion architecture");
                               break;
    }

    return true;
}


SymEval::Retval_t SymEval::process(SliceNode::Ptr ptr,
        Result_t &dbase,
        std::set<Edge::Ptr> &skipEdges) {
    bool failedTranslation;
    bool skippedEdge = false;
    bool skippedInput = false;
    bool success = false;

    std::map<const AbsRegion, std::set<Assignment::Ptr> > inputMap;

    expand_cerr << "Calling process on " << ptr->format() << endl;

    // Don't try an expansion of a widen node...
    if (!ptr->assign()) return WIDEN_NODE;

    EdgeIterator begin, end;
    ptr->ins(begin, end);

    for (; begin != end; ++begin) {
        SliceEdge::Ptr edge = boost::static_pointer_cast<SliceEdge>(*begin);
        SliceNode::Ptr source = boost::static_pointer_cast<SliceNode>(edge->source());

        // Skip this one to break a cycle.
        if (skipEdges.find(edge) != skipEdges.end()) {
            expand_cerr << "In process, skipping edge from "
                << source->format() << endl;
            skippedEdge = true;
            continue;
        }

        Assignment::Ptr assign = source->assign();
        if (!assign) continue; // widen node

        expand_cerr << "Assigning input " << edge->data().format()
            << " from assignment " << assign->format() << endl;
        inputMap[edge->data()].insert(assign);
    }

    expand_cerr << "\t Input map has size " << inputMap.size() << endl;

    // All of the expanded inputs are in the parameter dbase
    // If not (like this one), add it

    AST::Ptr ast;
    boost::tie(ast, failedTranslation) = SymEval::expand(ptr->assign());
    // expand_cerr << "\t ... resulting in " << dbase.format() << endl;

    // We have an AST. Now substitute in all of its predecessors.
    for (std::map<const AbsRegion, std::set<Assignment::Ptr> >::iterator iter = inputMap.begin();
            iter != inputMap.end(); ++iter) {
        // If we have multiple secondary definitions, we:
        //   if all definitions are equal, use the first
        //   otherwise, use nothing
        AST::Ptr definition;

        for (std::set<Assignment::Ptr>::iterator iter2 = iter->second.begin(); 
                iter2 != iter->second.end(); ++iter2) {
            AST::Ptr newDef = dbase[*iter2];
            if (!definition) {
                definition = newDef;
                continue;
            } else if (definition->equals(newDef)) {
                continue;
            } else {
                // Not equal
                definition = AST::Ptr();
                skippedInput = true;
                break;
            }
        }

        // The region used by the current assignment...
        const AbsRegion &reg = iter->first;

        // Create an AST around this one
        VariableAST::Ptr use = VariableAST::create(Variable(reg, ptr->addr()));

        if (!definition) {
            // Can happen if we're expanding out of order, and is generally harmless.
            continue;
        }
        expand_cerr << "Before substitution: " << (ast ? ast->format() : "<NULL AST>") << endl;

        if (!ast) {
            expand_cerr << "Skipping substitution because of null AST" << endl;
        } else {
            ast = AST::substitute(ast, use, definition);
            success = true;
        }
        expand_cerr << "\t result is " << (ast ? ast->format() : "<NULL AST>") << endl;
    }
    expand_cerr << "Result of substitution: " << ptr->assign()->format() << " == " 
        << (ast ? ast->format() : "<NULL AST>") << endl;

    // And attempt simplification again
    ast = simplifyStack(ast, ptr->addr(), ptr->func(), ptr->block());
    expand_cerr << "Result of post-substitution simplification: " << ptr->assign()->format() << " == " 
        << (ast ? ast->format() : "<NULL AST>") << endl;

    dbase[ptr->assign()] = ast;
    if (failedTranslation) return FAILED_TRANSLATION;
    else if (skippedEdge || skippedInput) return SKIPPED_INPUT;
    else if (success) return SUCCESS;
    else return FAILED;
}

AST::Ptr SymEval::simplifyStack(AST::Ptr ast, Address addr, ParseAPI::Function *func, ParseAPI::Block *block) {
    if (!ast) return ast;
    // Let's experiment with simplification
    StackAnalysis sA(func);
    StackAnalysis::Height sp = sA.findSP(block, addr);
    StackAnalysis::Height fp = sA.find(block, addr, MachRegister::getFramePointer(func->isrc()->getArch()));

    StackVisitor sv(addr, func, sp, fp);

    AST::Ptr simplified = ast->accept(&sv);

    return simplified;
}
