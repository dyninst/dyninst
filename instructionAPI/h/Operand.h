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

#include "BinaryFunction.h"
#include "Expression.h"
#include "Immediate.h"
#include "Register.h"
#include "util.h"

#include <set>
#include <string>

namespace Dyninst { namespace InstructionAPI {
  class Operand {
  public:
    typedef boost::shared_ptr<Operand> Ptr;

    explicit Operand(Expression::Ptr val = {}, bool read = false, bool written = false, bool implicit = false,
                     bool trueP = false, bool falseP = false) noexcept
        : op_value(val), m_isRead(read), m_isWritten(written), m_isImplicit(implicit),
          m_isTruePredicate(trueP), m_isFalsePredicate(falseP) {}

    DYNINST_EXPORT void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;
    DYNINST_EXPORT void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

    DYNINST_EXPORT RegisterAST::Ptr getPredicate() const;
    DYNINST_EXPORT bool isTruePredicate() const { return m_isTruePredicate; }
    DYNINST_EXPORT bool isFalsePredicate() const { return m_isFalsePredicate; }

    DYNINST_EXPORT bool isRead(Expression::Ptr candidate) const;
    DYNINST_EXPORT bool isWritten(Expression::Ptr candidate) const;

    DYNINST_EXPORT bool isRead() const { return m_isRead; }
    DYNINST_EXPORT bool isWritten() const { return m_isWritten; }

    DYNINST_EXPORT bool isImplicit() const { return m_isImplicit; }

    DYNINST_EXPORT bool readsMemory() const;
    DYNINST_EXPORT bool writesMemory() const;

    DYNINST_EXPORT void addEffectiveReadAddresses(std::set<Expression::Ptr>& memAccessors) const;
    DYNINST_EXPORT void addEffectiveWriteAddresses(std::set<Expression::Ptr>& memAccessors) const;

    DYNINST_EXPORT std::string format(Architecture arch, Address addr = 0) const;

    DYNINST_EXPORT Expression::Ptr getValue() const;

  private:
    Expression::Ptr op_value{};
    bool m_isRead{};
    bool m_isWritten{};
    bool m_isImplicit{};

    // Used for GPU instructions with predicates
    bool m_isTruePredicate{};
    bool m_isFalsePredicate{};
  };
}}

#endif //! defined(OPERAND_H)
