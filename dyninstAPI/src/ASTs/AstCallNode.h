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

#ifndef DYNINST_DYNINSTAPI_ASTCALLNODE_H
#define DYNINST_DYNINSTAPI_ASTCALLNODE_H

#include "AstNode.h"
#include "dyn_register.h"

#include <boost/make_shared.hpp>
#include <string>
#include <vector>

class BPatch_type;
class BPatch_function;
class codeGen;
class func_instance;

namespace Dyninst { namespace DyninstAPI {

class AstCallNode : public AstNode {
public:
  AstCallNode(std::string name) : func_name_{std::move(name)} {}

  AstCallNode(func_instance *func, std::vector<AstNodePtr> &args) : func_{func} {
    set_args(args);
  }

  AstCallNode(std::string name, std::vector<AstNodePtr> &args) : func_name_{std::move(name)} {
    set_args(args);
  }

  AstCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args) : func_addr_{addr} {
    set_args(args);
  }

  AstCallNode(func_instance *func) : func_{func}, callReplace_{true} {}

  virtual ~AstCallNode() = default;

  std::string format(std::string indent) override;

  BPatch_type *checkType(BPatch_function *func = NULL) override;

  bool containsFuncCall() const override {
    return true;
  }

  bool initRegisters(codeGen &gen) override;

private:
  bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                           Dyninst::Register &retReg) override;

  void set_args(std::vector<AstNodePtr> &args) {
    children = args;
  }

  std::string func_name_{};
  Dyninst::Address func_addr_{Dyninst::ADDR_NULL};
  func_instance *func_{};

  bool callReplace_{false}; // Node is intended for function call replacement
};

namespace CallNode {

  AstNodePtr namedCall(std::string name, std::vector<AstNodePtr> &args, AddressSpace *addrSpace);

  inline AstNodePtr namedCall(std::string name, std::vector<AstNodePtr> &args) {
    return boost::make_shared<AstCallNode>(std::move(name), args);
  }

  inline AstNodePtr call(func_instance *func, std::vector<AstNodePtr> &args) {
    if(!func) {
      return {};
    }
    return boost::make_shared<AstCallNode>(func, args);
  }

  inline AstNodePtr replace(func_instance *func) {
    if(!func) {
      return {};
    }
    return boost::make_shared<AstCallNode>(func);
  }

  inline AstNodePtr target(Dyninst::Address addr, std::vector<AstNodePtr> &args) {
    return boost::make_shared<AstCallNode>(addr, args);
  }
}

}}

#endif
