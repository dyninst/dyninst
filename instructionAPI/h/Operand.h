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

#if !defined(OPERAND_H)
#define OPERAND_H

#include "Expression.h"
#include "Register.h"
#include "BinaryFunction.h"
#include "Immediate.h"
#include <set>
#include <string>

#include "util.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
    class Operand
    {
    public:
        typedef boost::shared_ptr<Operand> Ptr;

      explicit Operand(Expression::Ptr val = {}, bool read = false, bool written = false, bool implicit = false,
              bool trueP = false, bool falseP = false) noexcept :
          op_value(val), m_isRead(read), m_isWritten(written), m_isImplicit(implicit), m_isTruePredicate(trueP), m_isFalsePredicate(falseP) {}

      INSTRUCTION_EXPORT void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;

      INSTRUCTION_EXPORT void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

      INSTRUCTION_EXPORT RegisterAST::Ptr getPredicate() const;


      INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) const;

      INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) const;

      INSTRUCTION_EXPORT bool isRead() const { return m_isRead; }
      INSTRUCTION_EXPORT bool isWritten() const { return m_isWritten; }

      INSTRUCTION_EXPORT bool isImplicit() const { return m_isImplicit; }
      INSTRUCTION_EXPORT void setImplicit(bool i) { m_isImplicit = i; }

      INSTRUCTION_EXPORT bool isTruePredicate() const { return m_isTruePredicate; }
      INSTRUCTION_EXPORT bool isFalsePredicate() const { return m_isFalsePredicate; }
      
      INSTRUCTION_EXPORT bool readsMemory() const;

      INSTRUCTION_EXPORT bool writesMemory() const;

      INSTRUCTION_EXPORT void addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const;

      INSTRUCTION_EXPORT void addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const;

      INSTRUCTION_EXPORT std::string format(Architecture arch, Address addr = 0) const;

      INSTRUCTION_EXPORT Expression::Ptr getValue() const;
      
    private:
      Expression::Ptr op_value{};
      bool m_isRead{};
      bool m_isWritten{};
      bool m_isImplicit{};

      bool m_isTruePredicate{};
      bool m_isFalsePredicate{};
    };
  }
}




#endif //!defined(OPERAND_H)
