/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string>
#include <iostream>

#include "../h/SymEval.h"
#include "SymEvalPolicy.h"

#include "AST.h"

#include "parseAPI/h/CFG.h"

#include "../rose/x86InstructionSemantics.h"
#include "../rose/powerpcInstructionSemantics.h"
#include "../rose/SgAsmInstruction.h"
#include "../h/stackanalysis.h"
#include "SymEvalVisitors.h"

#include "RoseInsnFactory.h"
#include "SymbolicExpansion.h"

#include "../h/Absloc.h"

#include "../h/slicing.h" // AssignNode

#include "debug_dataflow.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace DataflowAPI;


AST::Ptr SymEval::expand(const Assignment::Ptr &assignment) {
  // This is a shortcut version for when we only want a
  // single assignment
  
  Result_t res;
  // Fill it in to mark it as existing
  res[assignment] = AST::Ptr();
  expand(res);
  return res[assignment];
}

void SymEval::expand(Result_t &res, bool applyVisitors) {
  // Symbolic evaluation works off an Instruction
  // so we have something to hand to ROSE. 
  for (Result_t::iterator i = res.begin(); i != res.end(); ++i) {
    if (i->second != AST::Ptr()) {
      // Must've already filled it in from a previous instruction crack
      continue;
    }
    Assignment::Ptr ptr = i->first;
    
    expandInsn(ptr->insn(),
	       ptr->addr(),
	       res);
  }

  if (applyVisitors) {
    // Must apply the visitor to each filled in element
    for (Result_t::iterator i = res.begin(); i != res.end(); ++i) {
      if (!i->second) continue;
      AST::Ptr tmp = simplifyStack(i->second, i->first->addr(), i->first->func());
      i->second = tmp;
    }
  }
}

void dfs(Node::Ptr source, Node::Ptr node,
        set<Node::Ptr> & dfs_nodes,
        map<Node::Ptr, unsigned> & cycles) {
    // If we've already encountered this node, we've found a loop!
    // Mark as a circularity and break
    if (dfs_nodes.find(node) != dfs_nodes.end()) {
        expand_cerr << "Found cycle at " << node->format()
            << ", marking node as not for substitution" << endl;

        AssignNode::Ptr in = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(source);
        Assignment::Ptr assign = in->assign();
        AssignNode::Ptr cur_node = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(node);
        
        unsigned index = cur_node->getAssignmentIndex(in);
        cycles.insert(make_pair(node, index));
        return;
    } else {
        dfs_nodes.insert(node);
    }   

    // Continue DFS by following out-edges
    NodeIterator gbegin, gend;
    node->outs(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        dfs(node, *gbegin, dfs_nodes, cycles);
    }
}

// Do the previous, but use a Graph as a guide for
// performing forward substitution on the AST results
void SymEval::expand(Graph::Ptr slice, Result_t &res) {
    //cout << "Calling expand" << endl;
    // Other than the substitution this is pretty similar to the first example.
    NodeIterator gbegin, gend;
    slice->entryNodes(gbegin, gend);

    std::queue<Node::Ptr> worklist;
    std::queue<Node::Ptr> dfs_worklist;
    for (; gbegin != gend; ++gbegin) {
      expand_cerr << "adding " << (*gbegin)->format() << " to worklist" << endl;
      worklist.push(*gbegin);
      dfs_worklist.push(*gbegin);
    }

    /* First, we'll do DFS to check for circularities in the graph;
     * if so, mark them so we don't do infinite substitution */
    set<Node::Ptr> dfs_nodes;
    map<Node::Ptr, unsigned> cycles;
    while (!dfs_worklist.empty()) {
        Node::Ptr ptr = dfs_worklist.front(); dfs_worklist.pop();

        NodeIterator nbegin, nend;
        ptr->outs(nbegin, nend);

        for (; nbegin != nend; ++nbegin) {
            dfs(ptr, *nbegin, dfs_nodes, cycles);
        }
    }

    /* have a list
     * for each node, process
     * if processessing succeeded, remove the element
     * if the size of the list has changed, continue */

    while (!worklist.empty()) {
      Node::Ptr ptr = worklist.front(); worklist.pop();
      AssignNode::Ptr aNode = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(ptr);
      if (!aNode) continue; // They need to be AssignNodes
      
      if (!aNode->assign()) continue; // Could be a widen point
      
      expand_cerr << "Visiting node " << aNode->assign()->format() << endl;

      AST::Ptr prev = res[aNode->assign()];
      
      process(aNode, res, cycles); 
    
      AST::Ptr post = res[aNode->assign()];

      if (post && !(post->equals(prev))) {
	// Oy
	expand_cerr << "Adding successors to list, as new expansion " << endl
		    << "\t" << post->format() << endl 
		    << " != " << endl
		    << "\t" << (prev ? prev->format() : "<NULL>") << endl;
	NodeIterator oB, oE;
	aNode->outs(oB, oE);
	for (; oB != oE; ++oB) {
	  worklist.push(*oB);
	}
      }
    }
}

void SymEval::expandInsn(const InstructionAPI::Instruction::Ptr insn,
			 const uint64_t addr,
			 Result_t &res) {

  SymEvalPolicy policy(res, addr, insn->getArch());

  SgAsmInstruction *roseInsn;
  switch(insn->getArch()) {
  case Arch_x86:  {
    RoseInsnX86Factory fac;
    roseInsn = fac.convert(insn, addr);
    
    SymbolicExpansion exp;
    exp.expandX86(roseInsn, policy);
    break;
  }
  case Arch_ppc32: {
    RoseInsnPPCFactory fac;
    roseInsn = fac.convert(insn, addr);

    SymbolicExpansion exp;
    exp.expandPPC(roseInsn, policy);
    break;
  }
  default:
    assert(0 && "Unimplemented symbolic expansion architecture");
    break;
  }
  return;
}


bool SymEval::process(AssignNode::Ptr ptr,
		      Result_t &dbase,
                      map<Node::Ptr, unsigned> & cycles) {
    bool ret = false;
    
    std::map<unsigned, Assignment::Ptr> inputMap;

    expand_cerr << "Calling process on " << ptr->format() << endl;

    // Don't try an expansion of a widen node...
    if (!ptr->assign()) return ret;

    NodeIterator begin, end;
    ptr->ins(begin, end);

    for (; begin != end; ++begin) {
        AssignNode::Ptr in = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*begin);
        if (!in) continue;

        Assignment::Ptr assign = in->assign();

        if (!assign) continue;

        // Find which input this assignNode maps to
        unsigned index = ptr->getAssignmentIndex(in);
        expand_cerr << "Assigning input " << index << " from assignment " << assign->format() << endl;
        if (inputMap.find(index) == inputMap.end()) {
            inputMap[index] = assign;
        }
        else {
            // Need join operator!
            expand_cerr << "\t Overlap in inputs, setting to null assignment pointer" << endl;
            inputMap[index] = Assignment::Ptr(); // Null equivalent
        }
    }

    expand_cerr << "\t Input map has size " << inputMap.size() << endl;

    // All of the expanded inputs are in the parameter dbase
    // If not (like this one), add it

    AST::Ptr ast = SymEval::expand(ptr->assign());
    //expand_cerr << "\t ... resulting in " << dbase.format() << endl;

    // We have an AST. Now substitute in all of its predecessors.
    for (std::map<unsigned, Assignment::Ptr>::iterator iter = inputMap.begin();
            iter != inputMap.end(); ++iter) {
        if (!iter->second) {
            // Colliding definitions; skip.
            //cerr << "Skipping subsitution for input " << iter->first << endl;
            continue;
        }
        expand_cerr << "Substituting input " << iter->first << " with assignment " << iter->second->format() << endl;
        // The region used by the current assignment...
        const AbsRegion &reg = ptr->assign()->inputs()[iter->first];

        // Create an AST around this one
        VariableAST::Ptr use = VariableAST::create(Variable(reg, ptr->addr()));

        // And substitute whatever we have in the database for that AST
        AST::Ptr definition = dbase[iter->second];

        if (!definition) {
            // Can happen if we're expanding out of order, and is generally harmless.
            continue;
        }

        expand_cerr << "Before substitution: " << (ast ? ast->format() : "<NULL AST>") << endl;

        if (!ast) {
            expand_cerr << "Skipping substitution because of null AST" << endl;
        } else {
            // Check if we have a circular dependency here; if yes, don't substitute
            map<Node::Ptr, unsigned>::iterator cit;
            cit = cycles.find(ptr);
            if (cit != cycles.end()) {
                if (cit->second == iter->first) {
                    expand_cerr << "Found in-edge that's a cycle, skipping substitution" << endl;
                    break;
                }
            }

            ast = AST::substitute(ast, use, definition);
            ret = true;
        }	
        //expand_cerr << "\t result is " << res->format() << endl;
    }
    expand_cerr << "Result of substitution: " << ptr->assign()->format() << " == " << (ast ? ast->format() : "<NULL AST>") << endl;

    // And attempt simplification again
    ast = simplifyStack(ast, ptr->addr(), ptr->func());
    expand_cerr << "Result of post-substitution simplification: " << ptr->assign()->format() << " == " 
		<< (ast ? ast->format() : "<NULL AST>") << endl;
    
    dbase[ptr->assign()] = ast;
    return ret;
}

AST::Ptr SymEval::simplifyStack(AST::Ptr ast, Address addr, ParseAPI::Function *func) {
  if (!ast) return ast;
  // Let's experiment with simplification
  StackAnalysis sA(func);
  StackAnalysis::Height sp = sA.findSP(addr);
  StackAnalysis::Height fp = sA.findFP(addr);
  
  StackVisitor sv(addr, func, sp, fp);

  AST::Ptr simplified = ast->accept(&sv);

  return simplified;
}
