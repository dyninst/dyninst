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

#ifndef DYNINST_DYNINSTAPI_ASTOPERATORNODE_H
#define DYNINST_DYNINSTAPI_ASTOPERATORNODE_H

#include "AstNode.h"
#include "dyn_register.h"
#include "opcode.h"

#include <boost/make_shared.hpp>
#include <string>

class codeGen;

namespace Dyninst { namespace DyninstAPI {

class AstOperatorNode : public AstNode {
public:
  AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r = AstNodePtr(),
                  AstNodePtr e = AstNodePtr());

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
  AstNodePtr loperand{};
  AstNodePtr roperand{};
  AstNodePtr eoperand{};
};

namespace OperatorNode {
  // clang-format off
  inline AstNodePtr And(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(andOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr div(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(divOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr eq(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(eqOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr ge(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(geOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr getAddr(AstNodePtr arg) {
    return boost::make_shared<AstOperatorNode>(getAddrOp, std::move(arg));
  }
  inline AstNodePtr greater(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(greaterOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr ifMC(AstNodePtr arg) {
    return boost::make_shared<AstOperatorNode>(ifMCOp, std::move(arg));
  }
  inline AstNodePtr If(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(ifOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr If(AstNodePtr cond, AstNodePtr true_clause, AstNodePtr false_clause) {
    return boost::make_shared<AstOperatorNode>(ifOp, std::move(cond), std::move(true_clause), std::move(false_clause));
  }
  inline AstNodePtr le(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(leOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr less(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(lessOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr minus(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(minusOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr ne(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(neOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr Or(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(orOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr plus(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(plusOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr store(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(storeOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr times(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(timesOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr While(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(whileOp, std::move(lhs), std::move(rhs));
  }
  inline AstNodePtr Xor(AstNodePtr lhs, AstNodePtr rhs) {
    return boost::make_shared<AstOperatorNode>(xorOp, std::move(lhs), std::move(rhs));
  }

  // clang-format on
}

}}

#endif
