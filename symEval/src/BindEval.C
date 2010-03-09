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

#include "BindEval.h"

using namespace Dyninst;
using namespace Dyninst::SymbolicEvaluation;
using namespace std;

StackBindEval::StackBindEval(const char *fname,
			     StackAnalysis::Height &s,
			     StackAnalysis::Height &f) :
  func_(fname),
  stack_(s),
  frame_(f) {};

AST::Ptr StackBindEval::simplify(AST::Ptr in) {
  //cerr << "Entry: simplify of " << in->format() << endl;
  
  // Bottom-up traversal
  
  // With a _pile_ of RTTI...
  vPtr p3 = dyn_detail::boost::dynamic_pointer_cast<VariableAST<AbsRegion> >(in);
  if (p3) {
    //cerr << "\t returning variable handler" << endl;
    return simplifyVariable(p3);
  }

  uPtr p4 = dyn_detail::boost::dynamic_pointer_cast<UnaryAST<ROSEOperation> >(in);
  if (p4) {
    //cerr << "\t returning unary handler" << endl;
    return simplifyUnaryOp(p4);
  }

  bPtr p5 = dyn_detail::boost::dynamic_pointer_cast<BinaryAST<ROSEOperation> >(in);
  if (p5) {
    //cerr << "\t returning binary handler" << endl;
    return simplifyBinaryOp(p5);
  }

  //cerr << "\t no change, returning" << endl;
  return in;
}

AST::Ptr StackBindEval::simplifyVariable(vPtr ptr) {
  // If this region represents the stack or frame pointer
  // (that is, an Absloc wrapper of esp/ebp)
  // substitute a ConstantAST around the Height provided
  
  const AbsRegion &reg = ptr->val();
  if (reg.absloc() == Absloc()) {
    //cerr << "\t variable with multiple abslocs, returning" << endl;
    return ptr;
  }

  const Absloc &aloc = reg.absloc();
  
  if (aloc.isSP()) {
    // Create a new ConstantAST
    //cerr << "\t found stack pointer, returning constant" << endl;
    return ConstantAST<StackAnalysis::Height>::create(stack_);
  }
  else if (aloc.isFP()) {
    //cerr << "\t found frame pointer, returning constant" << endl;
    return ConstantAST<StackAnalysis::Height>::create(frame_);
  }
  else return ptr;
}

// Unary etc.
  
AST::Ptr StackBindEval::simplifyUnaryOp(uPtr ptr) {
  // Simplify the child operand
  AST::Ptr newOperand = simplify(ptr->operand());

  // Deref is the only thing we care about
  ROSEOperation op = ptr->op();
  if (op.op != ROSEOperation::derefOp) {
    return UnaryAST<ROSEOperation>::create(op, newOperand);
  }

  // See if it's a constant
  cPtr con = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<uint64_t> >(newOperand);
  if (con) {
    return VariableAST<AbsRegion>::create(AbsRegion(Absloc(con->val())));
  }
  
  hPtr h = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<StackAnalysis::Height> >(ptr->operand());

  if (h) {
    if (h->val() == StackAnalysis::Height::bottom) {
      return VariableAST<AbsRegion>::create(AbsRegion(Absloc::Stack));
    }
    else {
      return VariableAST<AbsRegion>::create(AbsRegion(Absloc(h->val().height(),
							     h->val().region()->name(),
							     func_)));
    }
  }

  return ptr;
}



AST::Ptr StackBindEval::simplifyBinaryOp(bPtr ptr) {
  AST::Ptr left = simplify(ptr->left());
  AST::Ptr right = simplify(ptr->right());

  // Get the operands
  cPtr cLeft = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<uint64_t> >(left);
  cPtr cRight = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<uint64_t> >(right);

  hPtr hLeft = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<StackAnalysis::Height> >(left);
  hPtr hRight = dyn_detail::boost::dynamic_pointer_cast<ConstantAST<StackAnalysis::Height> >(right);

  // Deref is the only thing we care about
  if (ptr->op().op == ROSEOperation::derefOp) {
    // Left is the addr, right is the condition
    // So we ignore right
    if (cLeft) {
      return VariableAST<AbsRegion>::create(AbsRegion(Absloc(cLeft->val())));
    }
    else if (hLeft) {
      return VariableAST<AbsRegion>::create(AbsRegion(Absloc(hLeft->val().height(),
							     hLeft->val().region()->name(),
							     func_)));
    }
  }

  if (ptr->op().op != ROSEOperation::addOp) {
    return BinaryAST<ROSEOperation>::create(ptr->op(), left, right);
  }

  if (hLeft && hRight) {
    // Don't know how to handle fp + sp...
    return BinaryAST<ROSEOperation>::create(ptr->op(), left, right);
  }

  if (hLeft && cRight) {
    return ConstantAST<StackAnalysis::Height>::create(hLeft->val() + cRight->val());
  }
  if (cLeft && hRight) {
    return ConstantAST<StackAnalysis::Height>::create(hRight->val() + cLeft->val());
  }
  if (cLeft && cRight) {
    return ConstantAST<uint64_t>::create(cLeft->val() + cRight->val());
  }
  return ptr;
}
  

AST::Ptr StackCanonical::simplify(AST::Ptr in) {
  //cerr << "Entry: simplify of " << in->format() << endl;
  
  // Bottom-up traversal
  
  // With a _pile_ of RTTI...
  vPtr p3 = dyn_detail::boost::dynamic_pointer_cast<VariableAST<AbsRegion> >(in);
  if (p3) {
    //cerr << "\t returning variable handler" << endl;
    return simplifyVariable(p3);
  }

  uPtr p4 = dyn_detail::boost::dynamic_pointer_cast<UnaryAST<ROSEOperation> >(in);
  if (p4) {
    //cerr << "\t returning unary handler" << endl;
    return simplifyUnaryOp(p4);
  }

  bPtr p5 = dyn_detail::boost::dynamic_pointer_cast<BinaryAST<ROSEOperation> >(in);
  if (p5) {
    //cerr << "\t returning binary handler" << endl;
    return simplifyBinaryOp(p5);
  }

  tPtr p6 = dyn_detail::boost::dynamic_pointer_cast<TernaryAST<ROSEOperation> >(in);
  if (p6) {
    //cerr << "\t returning binary handler" << endl;
    return simplifyTernaryOp(p6);
  }

  //cerr << "\t no change, returning" << endl;
  return in;
}

AST::Ptr StackCanonical::simplifyVariable(vPtr ptr) {
  std::map<AbsRegion, AbsRegion>::iterator iter = repl.find(ptr->val());
  if (iter != repl.end()) {
    return VariableAST<AbsRegion>::create(iter->second);
  }
  return ptr;
}

// Unary etc.
  
AST::Ptr StackCanonical::simplifyUnaryOp(uPtr ptr) {
  // Simplify the child operand
  AST::Ptr newOperand = simplify(ptr->operand());
  return UnaryAST<ROSEOperation>::create(ptr->op(), newOperand);
}


AST::Ptr StackCanonical::simplifyBinaryOp(bPtr ptr) {
  AST::Ptr left = simplify(ptr->left());
  AST::Ptr right = simplify(ptr->right());

  return BinaryAST<ROSEOperation>::create(ptr->op(), left, right);
}
  
AST::Ptr StackCanonical::simplifyTernaryOp(tPtr ptr) {
  AST::Ptr left = simplify(ptr->left());
  AST::Ptr center = simplify(ptr->center());
  AST::Ptr right = simplify(ptr->right());

  return TernaryAST<ROSEOperation>::create(ptr->op(), left, center, right);
}
  


