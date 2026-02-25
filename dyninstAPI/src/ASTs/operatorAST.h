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

#ifndef DYNINST_DYNINSTAPI_OPERATORAST_H
#define DYNINST_DYNINSTAPI_OPERATORAST_H

#include "dyn_register.h"
#include "opcode.h"

#include <boost/make_shared.hpp>
#include "codeGenAST.h"
#include <string>

class codeGen;

namespace Dyninst { namespace DyninstAPI {

class operatorAST : public codeGenAST {
public:
  operatorAST(opCode opC, codeGenASTPtr l, codeGenASTPtr r = codeGenASTPtr(),
                  codeGenASTPtr e = codeGenASTPtr());

  std::string format(std::string indent) override;

  BPatch_type *checkType(BPatch_function *func = NULL) override;

  bool canBeKept() const override;

  // We override initRegisters in the case of writing to an original register.
  bool initRegisters(codeGen &gen) override;

private:
  bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                           Dyninst::Register &retReg) override;

  bool generateOptimizedAssignment(codeGen &gen, int size, bool noCost);

  opCode op{};
  codeGenASTPtr loperand{};
  codeGenASTPtr roperand{};
  codeGenASTPtr eoperand{};
};

namespace OperatorNode {
  // clang-format off
  inline codeGenASTPtr And(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(andOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr div(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(divOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr eq(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(eqOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr ge(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(geOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr getAddr(codeGenASTPtr arg) {
    return boost::make_shared<operatorAST>(getAddrOp, std::move(arg));
  }
  inline codeGenASTPtr greater(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(greaterOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr ifMC(codeGenASTPtr arg) {
    return boost::make_shared<operatorAST>(ifMCOp, std::move(arg));
  }
  inline codeGenASTPtr If(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(ifOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr If(codeGenASTPtr cond, codeGenASTPtr true_clause, codeGenASTPtr false_clause) {
    return boost::make_shared<operatorAST>(ifOp, std::move(cond), std::move(true_clause), std::move(false_clause));
  }
  inline codeGenASTPtr le(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(leOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr less(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(lessOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr minus(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(minusOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr ne(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(neOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr Or(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(orOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr plus(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(plusOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr store(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(storeOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr times(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(timesOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr While(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(whileOp, std::move(lhs), std::move(rhs));
  }
  inline codeGenASTPtr Xor(codeGenASTPtr lhs, codeGenASTPtr rhs) {
    return boost::make_shared<operatorAST>(xorOp, std::move(lhs), std::move(rhs));
  }

  // clang-format on
}

}}

#endif
