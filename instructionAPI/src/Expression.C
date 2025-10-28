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

#include "Expression.h"
#include "InstructionAST.h"

namespace Dyninst { namespace InstructionAPI {
  Expression::Expression(Result_Type t) : userSetValue(t) {}

  static Result sizeToResult(uint32_t size) {
    switch(size) {
      case 1:
        return Result(u8);
      case 2:
        return Result(u16);
      case 4:
        return Result(u32);
      case 6:
        return Result(u48);
      case 8:
        return Result(u64);
      case 10:
        return Result(dp_float);
      case 12:
        return Result(m96);
      case 16:
        return Result(dbl128);
      case 32:
        return Result(m256);
      case 64:
        return Result(m512);
      case 0:
        return Result(bit_flag);
      default:
        assert(!"unexpected machine register size!");
    }
    return {};
  }

  Expression::Expression(uint32_t size) { userSetValue = sizeToResult(size); }

  Expression::Expression(std::vector<MachRegister> rs) {
    uint32_t totalSize = 0;
    for(auto& mReg : rs)
      totalSize += mReg.size();
    Result result = sizeToResult(totalSize);
    this->setValue(result);
  }

  Expression::Expression(MachRegister r) : Expression::Expression(r.size()) {}

  Expression::Expression(MachRegister r, uint32_t len) : Expression::Expression(r.size() * len) {}

  Expression::~Expression() {}

  const Result& Expression::eval() const { return userSetValue; }

  void Expression::getUses(std::set<Expression::Ptr> &uses) {
    for(auto reg : getUsedRegisters(shared_from_this())) {
      uses.insert(reg);
    }
  }

  void Expression::setValue(const Result& knownValue) { userSetValue = knownValue; }

  void Expression::clearValue() { userSetValue.defined = false; }

  int Expression::size() const { return userSetValue.size(); }

  bool Expression::bind(Expression* expr, const Result& value) {
    // bool retVal = false;
    if(*expr == *this) {
      setValue(value);
      return true;
    }
    return false;
  }

  bool Expression::isFlag() const { return false; }

}}
