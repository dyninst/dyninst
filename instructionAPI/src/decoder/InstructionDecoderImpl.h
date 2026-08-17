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

#ifndef DYNINST_INSTRUCTIONAPI_INSTRUCTIONDECODERIMPL_H
#define DYNINST_INSTRUCTIONAPI_INSTRUCTIONDECODERIMPL_H

#include "Architecture.h"
#include "entryIDs.h"
#include "Expression.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include <stdint.h>

namespace Dyninst { namespace InstructionAPI {
  namespace {
    constexpr bool CFT_CALL = true;
    constexpr bool CFT_INDIRECT = true;
    constexpr bool CFT_CONDITIONAL = true;
    constexpr bool CFT_FALLTHROUGH = true;

    constexpr bool OP_READ = true;
    constexpr bool OP_WRITTEN = true;
    constexpr bool OP_IMPLICIT = true;
  }

  class InstructionDecoderImpl {
  public:
    using Ptr = boost::shared_ptr<InstructionDecoderImpl>;

    InstructionDecoderImpl(Architecture a) : m_Arch(a) {}

    virtual ~InstructionDecoderImpl() {}

    virtual Instruction decode(InstructionDecoder::buffer &b) = 0;

    static Ptr makeDecoderImpl(Architecture a);

  protected:
    virtual Expression::Ptr makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                                              Result_Type resultType);

    virtual Expression::Ptr makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                                                   Result_Type resultType);

    virtual Expression::Ptr makeLeftShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                                                    Result_Type resultType);

    virtual Expression::Ptr makeRightArithmeticShiftExpression(Expression::Ptr lhs,
                                                               Expression::Ptr rhs,
                                                               Result_Type resultType);

    virtual Expression::Ptr makeRightLogicalShiftExpression(Expression::Ptr lhs,
                                                            Expression::Ptr rhs,
                                                            Result_Type resultType);

    virtual Expression::Ptr makeRightRotateExpression(Expression::Ptr lhs, Expression::Ptr rhs,
                                                      Result_Type resultType);

    virtual Expression::Ptr makeDereferenceExpression(Expression::Ptr addrToDereference,
                                                      Result_Type resultType);

    virtual Expression::Ptr makeMultiRegisterExpression(MachRegister reg, uint32_t num_elements);

    virtual Expression::Ptr makeRegisterExpression(MachRegister reg, uint32_t num_elements = 1);

    // Load partial values out of register
    virtual Expression::Ptr makeRegisterExpression(MachRegister reg, unsigned int start,
                                                   unsigned int end);

    virtual Expression::Ptr makeMaskRegisterExpression(MachRegister reg);

    virtual Expression::Ptr makeRegisterExpression(MachRegister reg, Result_Type extendFrom);

    virtual Expression::Ptr makeTernaryExpression(Expression::Ptr cond, Expression::Ptr first,
                                                  Expression::Ptr second, Result_Type resultType);

    boost::shared_ptr<Instruction> makeInstruction(entryID opcode, const char *mnem,
                                                   unsigned int decodedSize,
                                                   const unsigned char *raw);

    template<typename... Args>
    void add_operand(Args &&...args) {
      m_Operands.emplace_back(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void add_cft_target(Args &&...args) {
      m_CFT_Targets.emplace_back(std::forward<Args>(args)...);
    }

    void add_successor(Expression::Ptr e, bool isCall, bool isIndirect, bool isConditional,
                       bool isFallthrough, bool isImplicit = false) {
      add_cft_target(e, isCall, isIndirect, isConditional, isFallthrough);
      if(!isFallthrough) {
        add_operand(e, OP_READ, !OP_WRITTEN, isImplicit);
      }
    }

    virtual void reset_operands() {
      // The member variables are moved-from if 'decode' was called before.
      // Explicitly construct new ones to prevent UB.
      m_Operands = {};
      m_CFT_Targets = {};
    }

    Operation m_Operation;
    Architecture m_Arch;
    std::vector<Operand> m_Operands;
    std::vector<Instruction::CFT> m_CFT_Targets;
  };

}}

#endif
