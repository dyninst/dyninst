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

#include "SymEvalVisitors.h"

using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace std;

AST::Ptr StackVisitor::visit(AST *a) {
  return a->ptr();
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
  case ROSEOperation::signExtendOp: {
    if (newKids[0]->getID() == AST::V_ConstantAST) {
      assert(newKids[1]->getID() == AST::V_ConstantAST);

      ConstantAST::Ptr size = ConstantAST::convert(newKids[1]);
      ConstantAST::Ptr val = ConstantAST::convert(newKids[0]);
      return ConstantAST::create(Constant(val->val().val, size->val().val));
    }
      

    return RoseAST::create(r->val(), newKids);
  }
  case ROSEOperation::extractOp: {
    if (newKids[0]->getID() == AST::V_ConstantAST) {
      assert(newKids[1]->getID() == AST::V_ConstantAST);
      assert(newKids[2]->getID() == AST::V_ConstantAST);
      
      // Let's fix this...
      // newKids[1] is the "from", and newKids[2] is the "to". As in "mask from 0 to 16".
      ConstantAST::Ptr from = ConstantAST::convert(newKids[1]);
      ConstantAST::Ptr to = ConstantAST::convert(newKids[2]);
      ConstantAST::Ptr val = ConstantAST::convert(newKids[0]);
      

      auto lowBitPos{from->val().val};
      auto highBitPos{to->val().val};
      uint64_t newValue{val->val().val};
      if(highBitPos < 64)
        newValue &= ((1ULL << highBitPos) - 1);  // zero highBitPos and higher
      newValue >>= lowBitPos;                  // shift to bit 0, eliminating unwanted low bits

      return ConstantAST::create(Constant(newValue, highBitPos - lowBitPos));

    }
    return RoseAST::create(r->val(), newKids);

    break;
  }
  case ROSEOperation::derefOp: {
    // We may have a conditional dereference; that's awkward...
    if (r->numChildren() > 1) {
      // Let's see if that second is "true"...
      if (newKids[1]->getID() != AST::V_ConstantAST) {
	return RoseAST::create(r->val(), newKids);
      }
      ConstantAST::Ptr cond = ConstantAST::convert(newKids[1]);
      assert(cond->val().val != 0); // Why put in a conditional of 0...
    }
    
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
      else {
	return VariableAST::create(Variable(AbsRegion(Absloc(s->val().height(),
							     0,
							     func_)),
					    addr_));
      }
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
  assert(0);
  return r->ptr();
}

AST::Ptr BooleanVisitor::visit(AST *t) {
  return t->ptr();
}

AST::Ptr BooleanVisitor::visit(BottomAST *b) {
  return b->ptr();
}

AST::Ptr BooleanVisitor::visit(ConstantAST *c) {
  return c->ptr();
}

AST::Ptr BooleanVisitor::visit(StackAST *s) {
  return s->ptr();
}

AST::Ptr BooleanVisitor::visit(VariableAST *v) {
  return v->ptr();
}

AST::Ptr BooleanVisitor::visit(RoseAST *r) {
  // Okay. We want to handle the following:
  // or(x,x) -> x
  // and(x, x) -> x
  // if (true, x) -> x
  // because PPC has these operations _all over the place_. 
  AST::Children newKids;
  for (unsigned i = 0; i < r->numChildren(); ++i) {
    newKids.push_back(r->child(i)->accept(this));
  }

  switch(r->val().op) {
  case ROSEOperation::andOp:
  case ROSEOperation::orOp:
    assert(newKids.size() == 2);
    if (newKids[0]->equals(newKids[1])) {
      return newKids[0];
    }
    break;
  case ROSEOperation::ifOp:
    // Our "true" is a constAST of 1
    if (newKids[0]->getID() == AST::V_ConstantAST) {
      ConstantAST::Ptr c = ConstantAST::convert(newKids[0]);
      //cerr << "\t 0 was const, val " << c->val() << endl;
      if (c->val().val != 0) {
	return newKids[1];
      }
    }
    break;
  default:
    break;
  }
  return RoseAST::create(r->val(), newKids);
}
