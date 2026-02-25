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

#ifndef DYNINST_DYNINSTAPI_FUNCTIONCALLAST_H
#define DYNINST_DYNINSTAPI_FUNCTIONCALLAST_H

#include "codeGenAST.h"
#include "dyn_register.h"

#include <boost/make_shared.hpp>
#include <string>
#include <vector>

class BPatch_type;
class BPatch_function;
class codeGen;
class func_instance;

namespace Dyninst { namespace DyninstAPI {

class functionCallAST : public codeGenAST {
public:
  using Ptr = boost::shared_ptr<functionCallAST>;

  static Ptr namedCall(std::string name, std::vector<codeGenASTPtr> &args, AddressSpace *addrSpace);

  static Ptr namedCall(std::string name, std::vector<codeGenASTPtr> &args) {
    return boost::make_shared<functionCallAST>(std::move(name), args);
  }

  static Ptr call(func_instance *func, std::vector<codeGenASTPtr> &args) {
    if(!func) {
      return {};
    }
    return boost::make_shared<functionCallAST>(func, args);
  }

  static Ptr replacement(func_instance *func) {
    if(!func) {
      return {};
    }
    return boost::make_shared<functionCallAST>(func);
  }

  static Ptr target(Dyninst::Address addr, std::vector<codeGenASTPtr> &args) {
    return boost::make_shared<functionCallAST>(addr, args);
  }

  functionCallAST(std::string name) : func_name_{std::move(name)} {}

  functionCallAST(func_instance *func, std::vector<codeGenASTPtr> &args) : func_{func} {
    set_args(args);
  }

  functionCallAST(std::string name, std::vector<codeGenASTPtr> &args) : func_name_{std::move(name)} {
    set_args(args);
  }

  functionCallAST(Dyninst::Address addr, std::vector<codeGenASTPtr> &args) : func_addr_{addr} {
    set_args(args);
  }

  functionCallAST(func_instance *func) : func_{func}, callReplace_{true} {}

  virtual ~functionCallAST() = default;

  std::string format(std::string indent) override;

  BPatch_type *checkType(BPatch_function *func = NULL) override;

  bool containsFuncCall() const override {
    return true;
  }

  bool initRegisters(codeGen &gen) override;

private:
  bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                           Dyninst::Register &retReg) override;

  void set_args(std::vector<codeGenASTPtr> &args) {
    children = args;
  }

  std::string func_name_{};
  Dyninst::Address func_addr_{Dyninst::ADDR_NULL};
  func_instance *func_{};

  bool callReplace_{false}; // Node is intended for function call replacement
};

}}

#endif
