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

#ifndef DYNINST_DYNINSTAPI_ASTVARIABLENODE_H
#define DYNINST_DYNINSTAPI_ASTVARIABLENODE_H

#include "AstNode.h"
#include "dyn_register.h"

#include <boost/make_shared.hpp>
#include <string>
#include <utility>
#include <vector>

class BPatch_type;
class BPatch_function;
class codeGen;

namespace Dyninst { namespace DyninstAPI {

class AstVariableNode : public AstNode {
public:
  using range_t = std::pair<Dyninst::Offset, Dyninst::Offset>;

  AstVariableNode(std::vector<AstNodePtr> &ast_wrappers, std::vector<range_t> *ranges)
      : ranges_{ranges} {
    children = ast_wrappers;
    assert(!children.empty());
  }

  std::string format(std::string indent) override;

  BPatch_type *checkType(BPatch_function * = NULL) override {
    return getType();
  }

  bool canBeKept() const override {
    return children[index]->canBeKept();
  }

  operandType getoType() const override {
    return children[index]->getoType();
  }

  AstNodePtr operand() const override {
    return children[index]->operand();
  }

  const void *getOValue() const override {
    return children[index]->getOValue();
  }

  void setVariableAST(codeGen &gen) override;

  bool containsFuncCall() const override {
    return children[index]->containsFuncCall();
  }

  bool usesAppRegister() const override {
    return children[index]->usesAppRegister();
  }

private:
  bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &addr,
                           Dyninst::Register &retReg) override {
    return children[index]->generateCode_phase2(gen, noCost, addr, retReg);
  }

  std::vector<range_t> *ranges_{};
  unsigned index{};
};

namespace VariableNode {

  inline AstNodePtr variable(std::vector<AstNodePtr> &asts) {
    return boost::make_shared<AstVariableNode>(asts, nullptr);
  }

  inline AstNodePtr variableWithRanges(std::vector<AstNodePtr> &asts,
                                       std::vector<AstVariableNode::range_t> *ranges) {
    return boost::make_shared<AstVariableNode>(asts, ranges);
  }

}

}}

#endif
