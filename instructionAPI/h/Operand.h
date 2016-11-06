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
#include <set>
#include <string>

#include "util.h"

namespace Dyninst {
namespace InstructionAPI {

/// An %Operand object contains an AST built from %RegisterAST and %Immediate
/// leaves,
/// and information about whether the %Operand
/// is read, written, or both. This allows us to determine which of the
/// registers
/// that appear in the %Operand are read and which are written, as well as
/// whether
/// any memory accesses are reads, writes, or both.
/// An %Operand, given full knowledge of the values of the leaves of the AST,
/// and knowledge of
/// the logic associated with the tree's internal nodes, can determine the
/// result of any computations that are encoded in it.  It will rarely be the
/// case
/// that an %Instruction is built with its %Operands' state fully specified.
/// This mechanism is
/// instead intended to allow a user to fill in knowledge about the state of the
/// processor
/// at the time the %Instruction is executed.

class Operand {
 public:
  typedef boost::shared_ptr<Operand> Ptr;
  Operand() : m_isRead(false), m_isWritten(false) {}
  /// \brief Create an operand from a %Expression and flags describing whether
  /// the %ValueComputation
  /// is read, written or both.
  /// \param val Reference-counted pointer to the %Expression that will be
  /// contained in the %Operand being constructed
  /// \param read True if this operand is read
  /// \param written True if this operand is written
  Operand(Expression::Ptr val, bool read, bool written)
      : op_value(val), m_isRead(read), m_isWritten(written) {}
  virtual ~Operand() { op_value.reset(); }
  Operand(const Operand& o)
      : op_value(o.op_value),
        m_isRead(o.m_isRead),
        m_isWritten(o.m_isWritten) {}
  const Operand& operator=(const Operand& rhs) {
    op_value = rhs.op_value;
    m_isRead = rhs.m_isRead;
    m_isWritten = rhs.m_isWritten;
    return *this;
  }

  /// \brief Get the registers read by this operand
  /// \param regsRead Has the registers read inserted into it
  INSTRUCTION_EXPORT void getReadSet(
      std::set<RegisterAST::Ptr>& regsRead) const;
  /// \brief Get the registers written by this operand
  /// \param regsWritten Has the registers written  inserted into it
  INSTRUCTION_EXPORT void getWriteSet(
      std::set<RegisterAST::Ptr>& regsWritten) const;

  /// Returns true if this operand is read
  INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) const;
  /// Returns true if this operand is written
  INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) const;

  INSTRUCTION_EXPORT bool isRead() const { return m_isRead; }
  INSTRUCTION_EXPORT bool isWritten() const { return m_isWritten; }

  /// Returns true if this operand reads memory
  INSTRUCTION_EXPORT bool readsMemory() const;
  /// Returns true if this operand writes memory
  INSTRUCTION_EXPORT bool writesMemory() const;
  /// \brief Inserts the effective addresses read by this operand into
  /// memAccessors
  /// \param memAccessors If this is a memory read operand, insert the \c
  /// %Expression::Ptr representing
  /// the address being read into \c memAccessors.
  INSTRUCTION_EXPORT void addEffectiveReadAddresses(
      std::set<Expression::Ptr>& memAccessors) const;
  /// \brief Inserts the effective addresses written by this operand into
  /// memAccessors
  /// \param memAccessors If this is a memory write operand, insert the \c
  /// %Expression::Ptr representing
  /// the address being written into \c memAccessors.
  INSTRUCTION_EXPORT void addEffectiveWriteAddresses(
      std::set<Expression::Ptr>& memAccessors) const;
  /// \brief Return a printable string representation of the operand
  /// \return The operand in a disassembly format
  INSTRUCTION_EXPORT std::string format(Architecture arch,
                                        Address addr = 0) const;
  /// The \c getValue method returns an %Expression::Ptr to the AST contained by
  /// the operand.
  INSTRUCTION_EXPORT Expression::Ptr getValue() const;

 private:
  Expression::Ptr op_value;
  bool m_isRead;
  bool m_isWritten;
};
};
};

#endif  //! defined(OPERAND_H)
