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
      if (i->second == AST::Ptr()) {
        // Must not have been filled in above
        continue;
      }
      Assignment::Ptr ptr = i->first;
      // Let's experiment with simplification
      StackAnalysis sA(ptr->func());
      StackAnalysis::Height sp = sA.findSP(ptr->addr());
      StackAnalysis::Height fp = sA.findFP(ptr->addr());

      StackVisitor sv(ptr->addr(), ptr->func()->name(), sp, fp);
      if (i->second) {
	AST::Ptr simplified = i->second->accept(&sv);
	i->second = simplified;
      }
    }
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
    for (; gbegin != gend; ++gbegin) {
      expand_cerr << "adding " << (*gbegin)->format() << " to worklist" << endl;
      worklist.push(*gbegin);
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
      
      process(aNode, res); 
    
      AST::Ptr post = res[aNode->assign()];

      if (post && !(post->equals(prev))) {
	// Oy
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
			 Result_t &dbase) {
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
        //cerr << "Substituting input " << iter->first << endl;
        // The region used by the current assignment...
        const AbsRegion &reg = ptr->assign()->inputs()[iter->first];

        // Create an AST around this one
        VariableAST::Ptr use = VariableAST::create(Variable(reg, ptr->addr()));

        // And substitute whatever we have in the database for that AST
        AST::Ptr definition = dbase[iter->second];

        if (!definition) {
            expand_cerr << "Odd; no expansion for " << iter->second->format() << endl;
            // Can happen if we're expanding out of order, and is generally harmless.
            continue;
        }

        expand_cerr << "Before substitution: " << (ast ? ast->format() : "<NULL AST>") << endl;

        if (!ast) {
            expand_cerr << "Skipping substitution because of null AST" << endl;
        } else {
            ast = AST::substitute(ast, use, definition);
            ret = true;
        }
        //expand_cerr << "\t result is " << res->format() << endl;
    }
    expand_cerr << "Result of subsitution: " << ptr->assign()->format() << " == " << (ast ? ast->format() : "<NULL AST>") << endl;
    dbase[ptr->assign()] = ast;
    return ret;
}


