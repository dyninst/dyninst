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

#if !defined(INSTRUCTION_H)
#define INSTRUCTION_H


#include <stddef.h>
#include <string>
#include <string.h>
#include <vector>
#include <set>
#include <list>
#include "Expression.h"
#include "Operation_impl.h"
#include "Operand.h"
#include "InstructionCategories.h"
#include "ArchSpecificFormatters.h"

#include "util.h"

namespace Dyninst
{
  namespace InstructionAPI
  {
      class InstructionDecoder;
    class Instruction
    {

        INSTRUCTION_EXPORT static void version(int& major, int& minor, int& maintenance);
      union raw_insn_T
      {
#if defined(__powerpc__) || defined(__powerpc64__)
	unsigned int small_insn;
#else
	uintptr_t small_insn;
#endif
	unsigned char* large_insn;
      };
    public:
        friend class InstructionDecoder_x86;
        friend class InstructionDecoder_power;
        friend class InstructionDecoder_aarch64;
        friend class InstructionDecoder_amdgpu_gfx908;
        friend class InstructionDecoder_amdgpu_gfx90a;
        friend class InstructionDecoder_amdgpu_gfx940;

        struct CFT
        {
            Expression::Ptr target;
            bool isCall;
            bool isIndirect;
            bool isConditional;
            bool isFallthrough;
            CFT(Expression::Ptr t, bool call, bool indir, bool cond, bool ft) :
                    target(t), isCall(call), isIndirect(indir), isConditional(cond), isFallthrough(ft) {}
        };

      INSTRUCTION_EXPORT Instruction(Operation what, size_t size, const unsigned char* raw,
                                     Dyninst::Architecture arch);
      INSTRUCTION_EXPORT Instruction();

      INSTRUCTION_EXPORT virtual ~Instruction();

      INSTRUCTION_EXPORT Instruction(const Instruction& o);
      INSTRUCTION_EXPORT const Instruction& operator=(const Instruction& rhs);


      INSTRUCTION_EXPORT Operation& getOperation();
      INSTRUCTION_EXPORT const Operation& getOperation() const;

      INSTRUCTION_EXPORT void getOperands(std::vector<Operand>& operands) const;

      INSTRUCTION_EXPORT std::vector<Operand> getDisplayOrderedOperands() const;

      INSTRUCTION_EXPORT Operand getOperand(int index) const;

      INSTRUCTION_EXPORT Operand getPredicateOperand() const;
      INSTRUCTION_EXPORT bool hasPredicateOperand() const;

      INSTRUCTION_EXPORT unsigned char rawByte(unsigned int index) const;

      INSTRUCTION_EXPORT size_t size() const;

      INSTRUCTION_EXPORT const void* ptr() const;

      INSTRUCTION_EXPORT void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;

      INSTRUCTION_EXPORT void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;

      INSTRUCTION_EXPORT bool isRead(Expression::Ptr candidate) const;

      INSTRUCTION_EXPORT bool isWritten(Expression::Ptr candidate) const;

      INSTRUCTION_EXPORT bool readsMemory() const;

      INSTRUCTION_EXPORT ArchSpecificFormatter& getFormatter() const;

      INSTRUCTION_EXPORT bool writesMemory() const;

      INSTRUCTION_EXPORT void getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const;

      INSTRUCTION_EXPORT void getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const;

      INSTRUCTION_EXPORT Expression::Ptr getControlFlowTarget() const;

      INSTRUCTION_EXPORT bool allowsFallThrough() const;

      INSTRUCTION_EXPORT std::string format(Address addr = 0) const;

      INSTRUCTION_EXPORT bool isValid() const;

      INSTRUCTION_EXPORT bool isLegalInsn() const;

      INSTRUCTION_EXPORT Architecture getArch() const;

      INSTRUCTION_EXPORT InsnCategory getCategory() const;

      typedef std::list<CFT>::const_iterator cftConstIter;
      INSTRUCTION_EXPORT cftConstIter cft_begin() const {
          return m_Successors.begin();
      }
        INSTRUCTION_EXPORT cftConstIter cft_end() const {
            return m_Successors.end();
        }
        INSTRUCTION_EXPORT bool operator<(const Instruction& rhs) const
        {
            if(m_size < rhs.m_size) return true;
            if(m_size <= sizeof(m_RawInsn.small_insn)) {
                return m_RawInsn.small_insn < rhs.m_RawInsn.small_insn;
            }
            return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) < 0;
        }
        INSTRUCTION_EXPORT bool operator==(const Instruction& rhs) const {
            if(m_size != rhs.m_size) return false;
            if(m_size <= sizeof(m_RawInsn.small_insn)) {
                return m_RawInsn.small_insn == rhs.m_RawInsn.small_insn;
            }
            return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) == 0;
        }
        INSTRUCTION_EXPORT void updateMnemonic(std::string new_mnemonic) {
                m_InsnOp.updateMnemonic(new_mnemonic);
            }


      typedef boost::shared_ptr<Instruction> Ptr;

    private:
      void updateSize(const unsigned int new_size) {m_size = new_size;}
      void decodeOperands() const;
      void addSuccessor(Expression::Ptr e, bool isCall, bool isIndirect, bool isConditional, bool isFallthrough, bool isImplicit = false) const;
      void appendOperand(Expression::Ptr e, bool isRead, bool isWritten, bool isImplicit = false, bool trueP = false, bool falseP = false) const;
      void copyRaw(size_t size, const unsigned char* raw);
      Expression::Ptr makeReturnExpression() const;
      mutable std::list<Operand> m_Operands;
      mutable Operation m_InsnOp;
      bool m_Valid;
      raw_insn_T m_RawInsn;
      unsigned int m_size;
      Architecture arch_decoded_from;
      mutable std::list<CFT> m_Successors;
      static int numInsnsAllocated;
      ArchSpecificFormatter* formatter;
    };
  }
}



#endif //!defined(INSTRUCTION_H)
