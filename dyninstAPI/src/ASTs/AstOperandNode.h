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

#include "AstNode.h"
#include "dyn_register.h"
#include "opcode.h"
#include "OperandType.h"

#include <boost/make_shared.hpp>
#include <string>

class AddressSpace;
class instPoint;
class func_instance;
class int_variable;
class codeGen;
class image_variable;

namespace Dyninst { namespace DyninstAPI {

class AstOperandNode : public AstNode {
  friend class AstOperatorNode;

public:
  // Direct operand
  AstOperandNode(operandType ot, void *arg);

  // And an indirect (say, a load)
  AstOperandNode(operandType ot, AstNodePtr l);

  AstOperandNode(operandType ot, const image_variable *iv);

  ~AstOperandNode() {
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

  AstNodePtr operand() const override {
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
  AstNodePtr operand_{};
};

namespace OperandNode {

  inline AstNodePtr Constant(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::Constant, v);
  }

  inline AstNodePtr ConstantString(const char *str) {
    auto val = static_cast<void *>(const_cast<char *>(str));
    return boost::make_shared<AstOperandNode>(operandType::ConstantString, val);
  }

  inline AstNodePtr DataIndir(AstNodePtr l) {
    return boost::make_shared<AstOperandNode>(operandType::DataIndir, l);
  }

  inline AstNodePtr Param(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::Param, v);
  }

  inline AstNodePtr ParamAtCall(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::ParamAtCall, v);
  }

  inline AstNodePtr ParamAtEntry(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::ParamAtEntry, v);
  }

  inline AstNodePtr ReturnVal(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::ReturnVal, v);
  }

  inline AstNodePtr ReturnAddr(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::ReturnAddr, v);
  }

  inline AstNodePtr DataAddr(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::DataAddr, v);
  }

  inline AstNodePtr FrameAddr(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::FrameAddr, v);
  }

  inline AstNodePtr RegOffset(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::RegOffset, v);
  }

  inline AstNodePtr RegOffset(AstNodePtr arg) {
    return boost::make_shared<AstOperandNode>(operandType::RegOffset, arg);
  }

  inline AstNodePtr origRegister(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::origRegister, v);
  }

  inline AstNodePtr variableAddr(image_variable const *addr) {
    return boost::make_shared<AstOperandNode>(operandType::variableAddr, addr);
  }

  inline AstNodePtr variableValue(image_variable const *v) {
    return boost::make_shared<AstOperandNode>(operandType::variableValue, v);
  }

  inline AstNodePtr undefOperandType(void *v) {
    return boost::make_shared<AstOperandNode>(operandType::undefOperandType, v);
  }

  inline AstNodePtr AddressAsPlaceholderRegAndOffset(AstNodePtr arg) {
    return boost::make_shared<AstOperandNode>(operandType::AddressAsPlaceholderRegAndOffset, arg);
  }

}

}}

#endif
