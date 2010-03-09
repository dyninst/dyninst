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

#if !defined(_BIND_EVAL_H)
#define _BIND_EVAL_H

#include "dyninstAPI/src/stackanalysis.h"
#include "dynutil/h/AST.h"
#include "symEval/h/Absloc.h"
#include "SymEvalPolicy.h"
#include <string>

// A quick and not-very-efficient implementation of bind/eval folding
// for ASTs.
namespace Dyninst {
namespace SymbolicEvaluation {

class StackBindEval {
  typedef dyn_detail::boost::shared_ptr<VariableAST<AbsRegion> > vPtr;
  typedef dyn_detail::boost::shared_ptr<ConstantAST<uint64_t> > cPtr;
  typedef dyn_detail::boost::shared_ptr<ConstantAST<StackAnalysis::Height> > hPtr;
  typedef dyn_detail::boost::shared_ptr<UnaryAST<ROSEOperation> > uPtr;
  typedef dyn_detail::boost::shared_ptr<BinaryAST<ROSEOperation> > bPtr;
  

 public:

  StackBindEval(const char *funcname,
		StackAnalysis::Height &stackHeight,
		StackAnalysis::Height &frameHeight);

  AST::Ptr simplify(AST::Ptr in);

 private:
  AST::Ptr dispatch(AST::Ptr in);

  //AST::Ptr simplifyConstant(ConstantAST<uint64_t>::Ptr ptr);
  //AST::Ptr simplifyConstant(ConstantAST<StackAnalysis::Height>::Ptr ptr);
  AST::Ptr simplifyVariable(vPtr ptr);
  AST::Ptr simplifyUnaryOp(uPtr ptr);
  AST::Ptr simplifyBinaryOp(bPtr ptr);
  //AST::Ptr simplifyTernaryOp(TernaryAST<ROSEOperation>::Ptr ptr);

  std::string func_;
  StackAnalysis::Height stack_;
  StackAnalysis::Height frame_;
};

class StackCanonical {
  typedef dyn_detail::boost::shared_ptr<VariableAST<AbsRegion> > vPtr;
  typedef dyn_detail::boost::shared_ptr<ConstantAST<uint64_t> > cPtr;
  typedef dyn_detail::boost::shared_ptr<ConstantAST<StackAnalysis::Height> > hPtr;
  typedef dyn_detail::boost::shared_ptr<UnaryAST<ROSEOperation> > uPtr;
  typedef dyn_detail::boost::shared_ptr<BinaryAST<ROSEOperation> > bPtr;
  typedef dyn_detail::boost::shared_ptr<TernaryAST<ROSEOperation> > tPtr;

 public:

  StackCanonical() {};

  void addPair(AbsRegion &old, AbsRegion &n) {
    repl[old] = n;
  }

  AST::Ptr simplify(AST::Ptr in);

 private:
  AST::Ptr dispatch(AST::Ptr in);

  //AST::Ptr simplifyConstant(ConstantAST<uint64_t>::Ptr ptr);
  //AST::Ptr simplifyConstant(ConstantAST<StackAnalysis::Height>::Ptr ptr);
  AST::Ptr simplifyVariable(vPtr ptr);
  AST::Ptr simplifyUnaryOp(uPtr ptr);
  AST::Ptr simplifyBinaryOp(bPtr ptr);
  AST::Ptr simplifyTernaryOp(tPtr ptr);

  std::map<AbsRegion, AbsRegion> repl;
};

};
};

#endif
