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

#ifndef DYNINST_DYNINSTAPI_ASTOPERANDNODE_H
#define DYNINST_DYNINSTAPI_ASTOPERANDNODE_H

#include "dyn_register.h"
#include "opcode.h"
#include "OperandType.h"

#include <boost/make_shared.hpp>
#include "codeGenAST.h"
#include <string>

class AddressSpace;
class instPoint;
class func_instance;
class int_variable;
class codeGen;
class image_variable;

namespace Dyninst { namespace DyninstAPI {

class operandAST : public codeGenAST {
  friend class operatorAST;

public:
  using Ptr = boost::shared_ptr<operandAST>;

  static Ptr Constant(void *v) {
    return boost::make_shared<operandAST>(operandType::Constant, v);
  }

  static Ptr ConstantString(const char *str) {
    auto val = static_cast<void *>(const_cast<char *>(str));
    return boost::make_shared<operandAST>(operandType::ConstantString, val);
  }

  static Ptr DataIndir(codeGenASTPtr l) {
    return boost::make_shared<operandAST>(operandType::DataIndir, l);
  }

  static Ptr Param(void *v) {
    return boost::make_shared<operandAST>(operandType::Param, v);
  }

  static Ptr ParamAtCall(void *v) {
    return boost::make_shared<operandAST>(operandType::ParamAtCall, v);
  }

  static Ptr ParamAtEntry(void *v) {
    return boost::make_shared<operandAST>(operandType::ParamAtEntry, v);
  }

  static Ptr ReturnVal(void *v) {
    return boost::make_shared<operandAST>(operandType::ReturnVal, v);
  }

  static Ptr ReturnAddr(void *v) {
    return boost::make_shared<operandAST>(operandType::ReturnAddr, v);
  }

  static Ptr DataAddr(void *v) {
    return boost::make_shared<operandAST>(operandType::DataAddr, v);
  }

  static Ptr FrameAddr(void *v) {
    return boost::make_shared<operandAST>(operandType::FrameAddr, v);
  }

  static Ptr RegOffset(void *v) {
    return boost::make_shared<operandAST>(operandType::RegOffset, v);
  }

  static Ptr RegOffset(codeGenASTPtr arg) {
    return boost::make_shared<operandAST>(operandType::RegOffset, arg);
  }

  static Ptr origRegister(void *v) {
    return boost::make_shared<operandAST>(operandType::origRegister, v);
  }

  static Ptr variableAddr(image_variable const *addr) {
    return boost::make_shared<operandAST>(operandType::variableAddr, addr);
  }

  static Ptr variableValue(image_variable const *v) {
    return boost::make_shared<operandAST>(operandType::variableValue, v);
  }

  static Ptr undefOperandType(void *v) {
    return boost::make_shared<operandAST>(operandType::undefOperandType, v);
  }

  static Ptr AddressAsPlaceholderRegAndOffset(codeGenASTPtr arg) {
    return boost::make_shared<operandAST>(operandType::AddressAsPlaceholderRegAndOffset, arg);
  }

  // Direct operand
  operandAST(operandType ot, void *arg);

  // And an indirect (say, a load)
  operandAST(operandType ot, codeGenASTPtr l);

  operandAST(operandType ot, const image_variable *iv);

  ~operandAST() {
    if(oType == operandType::ConstantString) {
      free((char *)oValue);
    }
  }

  std::string format(std::string indent) override;

  operandType getoType() const override {
    return oType;
  }

  void setOValue(void *o) override {
    oValue = o;
  }

  const void *getOValue() const override {
    return oValue;
  }

  const image_variable *getOVar() const override {
    return oVar;
  }

  codeGenASTPtr operand() const override {
    return operand_;
  }

  BPatch_type *checkType(BPatch_function *func = NULL) override;

  bool canBeKept() const override;

  bool usesAppRegister() const override;

  void emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen,
                         bool noCost, registerSpace *rs, int size, const instPoint *point,
                         AddressSpace *as) override;
  void emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen &gen,
                        bool noCost, registerSpace *rs, int size, const instPoint *point,
                        AddressSpace *as) override;

  bool initRegisters(codeGen &gen) override;

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  static int lastOffset; // Last ofsfet in our GPU memory buffer.
  static std::map<std::string, int> allocTable;

  static void addToTable(const std::string &variableName, int size) {
    // We shouldn't allocate more than once
    assert(allocTable.find(variableName) == allocTable.end() && "Can't allocate variable twice");
    assert(size > 0);
    allocTable[variableName] = lastOffset;
    std::cerr << "inserted " << variableName << " of " << size << " bytes at " << lastOffset
              << "\n";
    lastOffset += size;
  }

  static int getOffset(const std::string &variableName) {
    assert(allocTable.find(variableName) != allocTable.end() && "Variable must be allocated");
    return allocTable[variableName];
  }
#endif

private:
  bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                           Dyninst::Register &retReg) override;
  int_variable *lookUpVar(AddressSpace *as);

  operandType oType{operandType::undefOperandType};
  void *oValue{};
  const image_variable *oVar{};
  codeGenASTPtr operand_{};
};

}}

#endif
