/*
 * Copyright (c) 2007-2009 Barton P. Miller
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

#include "SymEvalVisitors.h"

using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace std;

AST::Ptr StackVisitor::visit(AST *t) {
  return t->ptr();
}

AST::Ptr StackVisitor::visit(BottomAST *b) {
  return b->ptr();
}

AST::Ptr StackVisitor::visit(ConstantAST *c) {
  return c->ptr();
}

AST::Ptr StackVisitor::visit(StackAST *s) {
  return s->ptr();
}

// Now we get to the interesting bits
AST::Ptr StackVisitor::visit(VariableAST *v) {
  // If we're an AbsRegion representing the
  // stack or frame pointer, return a
  // StackAST with the appropriate info
  const AbsRegion &reg = v->val().reg;
  if (reg.absloc() == Absloc()) {
    // Whoops
    return v->ptr();
  }

  const Absloc &aloc = reg.absloc();
  
  if (aloc.isSP()) {
    // Create a new ConstantAST
    return StackAST::create(stack_);
  }
  // If we're bottom it means "the FP register isn't the logical FP"
  else if (aloc.isFP() && (frame_ != StackAnalysis::Height::bottom)) {
    return StackAST::create(frame_);
  }
  else return v->ptr();
}

AST::Ptr StackVisitor::visit(RoseAST *r) {

  // Simplify children
  AST::Children newKids;
  for (unsigned i = 0; i < r->numChildren(); ++i) {
    newKids.push_back(r->child(i)->accept(this));
  }

  switch (r->val().op) {
  case ROSEOperation::derefOp: {
    // We may have a conditional dereference; that's awkward...
    if (r->numChildren() > 1)
      return RoseAST::create(r->val(), newKids);
    // Simplify the operand
    switch(newKids[0]->getID()) {
    case AST::V_ConstantAST:
      return VariableAST::create(Variable(AbsRegion(Absloc(ConstantAST::convert(newKids[0])->val().val)),
					  addr_));
    case AST::V_StackAST: {
      StackAST::Ptr s = StackAST::convert(newKids[0]);
      if (s->val() == StackAnalysis::Height::bottom) { 
	return VariableAST::create(Variable(AbsRegion(Absloc::Stack), 
					    addr_));
      }
      else 
	return VariableAST::create(Variable(AbsRegion(Absloc(s->val().height(),
							     s->val().region()->name(),
							     func_)),
					    addr_));
    }
    default:
      return RoseAST::create(r->val(), newKids);
    }
  }
  case ROSEOperation::addOp: {
    if (r->numChildren() != 2) 
      return RoseAST::create(r->val(), newKids);
    switch (newKids[0]->getID()) {
    case AST::V_ConstantAST:
      // Left is a constant; is the right something we can add?
      switch (newKids[1]->getID()) {
      case AST::V_ConstantAST: {
        Constant const0 = ConstantAST::convert(newKids[0])->val();
        Constant const1 = ConstantAST::convert(newKids[1])->val();
	return ConstantAST::create(Constant(const0.val + const1.val,
                                   ((const0.size > const1.size) ? const0.size : const1.size)));
      }
      case AST::V_StackAST:
	return StackAST::create(StackAST::convert(newKids[1])->val() +
				ConstantAST::convert(newKids[0])->val().val);
      default:
	return RoseAST::create(r->val(), newKids);
      }
    case AST::V_StackAST:
      // NewKids[0] is a constant; is the newKids[1] something we can add?
      switch (newKids[1]->getID()) {
      case AST::V_ConstantAST: {
	cerr << "Caught stackAST + constAST: " << StackAST::convert(newKids[0])->val() << " + " 
	     << ConstantAST::convert(newKids[1])->val().val << endl;
	return StackAST::create(StackAST::convert(newKids[0])->val() +
				ConstantAST::convert(newKids[1])->val().val);
      }
      default:
	return RoseAST::create(r->val(), newKids);
      }
    default:
      return RoseAST::create(r->val(), newKids);
    }
  }
  default:
    return RoseAST::create(r->val(), newKids);
  }
}

