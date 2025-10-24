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

#include "../h/BinaryFunction.h"
#include "../h/Dereference.h"
#include "../h/Expression.h"
#include "../h/Immediate.h"
#include "../h/MultiRegister.h"
#include "../h/Operand.h"
#include "../h/Register.h"
#include "../h/Result.h"
#include "InstructionAST.h"

#include <iostream>

using namespace std;

namespace Dyninst { namespace InstructionAPI {

  DYNINST_EXPORT void Operand::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const {
    for(auto r : getUsedRegisters(op_value)) {
      if(m_isRead || !(*r == *op_value)) {
        regsRead.insert(r);
      }
    }
  }

  DYNINST_EXPORT void Operand::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const {
    if(m_isWritten) {
      RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
      if(op_as_reg) {
        regsWritten.insert(op_as_reg);
      } else {

        MultiRegisterAST::Ptr op_as_multireg =
            boost::dynamic_pointer_cast<MultiRegisterAST>(op_value);
        if(op_as_multireg) {
          for(auto reg : op_as_multireg->getRegs()) {
            regsWritten.insert(reg);
          }
        }
      }
    }
  }

  DYNINST_EXPORT RegisterAST::Ptr Operand::getPredicate() const {
    RegisterAST::Ptr op_as_reg = boost::dynamic_pointer_cast<RegisterAST>(op_value);
    if(m_isTruePredicate || m_isFalsePredicate) {
      return op_as_reg;
    }
    return nullptr;
  }

  DYNINST_EXPORT bool Operand::isRead(Expression::Ptr candidate) const {
    // The whole expression of a read, any subexpression of a write
    return op_value->isUsed(candidate) && (m_isRead || !(*candidate == *op_value));
  }

  DYNINST_EXPORT bool Operand::isWritten(Expression::Ptr candidate) const {
    // Whole expression of a write
    return m_isWritten && (*op_value == *candidate);
  }

  DYNINST_EXPORT bool Operand::readsMemory() const {
    return (boost::dynamic_pointer_cast<Dereference>(op_value) && m_isRead);
  }

  DYNINST_EXPORT bool Operand::writesMemory() const {
    return (boost::dynamic_pointer_cast<Dereference>(op_value) && m_isWritten);
  }

  DYNINST_EXPORT void
  Operand::addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const {
    auto deref = boost::dynamic_pointer_cast<Dereference>(op_value);
    if(deref && m_isRead) {
      for(auto se : deref->getSubexpressions()) {
        memAccessors.insert(se);
      }
    }
  }

  DYNINST_EXPORT void
  Operand::addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const {
    auto deref = boost::dynamic_pointer_cast<Dereference>(op_value);
    if(deref && m_isWritten) {
      for(auto se : deref->getSubexpressions()) {
        memAccessors.insert(se);
      }
    }
  }

  DYNINST_EXPORT std::string Operand::format(Architecture arch, Address addr) const {
    if(!op_value)
      return "ERROR: format() called on empty operand!";
    if(addr) {
      Expression::Ptr thePC = Expression::Ptr(new RegisterAST(MachRegister::getPC(arch)));
      op_value->bind(thePC.get(), Result(u32, addr));
      Result res = op_value->eval();
      if(res.defined) {
        char hex[20];
        snprintf(hex, 20, "0x%lx", res.convert<uintmax_t>());
        return string(hex);
      }
    }
    auto s = op_value->format(arch);
    if(!s.compare(0, 2, "##")) {
      s.replace(0, 2, "0x0("); // fix-up ##X to indirection syntax 0x0(X)
      s += ')';
    }
    return s;
  }

  DYNINST_EXPORT Expression::Ptr Operand::getValue() const { return op_value; }
}}
