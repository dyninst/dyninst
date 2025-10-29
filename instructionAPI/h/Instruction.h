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

#include "ArchSpecificFormatters.h"
#include "compiler_annotations.h"
#include "Expression.h"
#include "InstructionCategories.h"
#include "Operand.h"
#include "Operation_impl.h"
#include "dyninst_visibility.h"

#include <list>
#include <set>
#include <stddef.h>
#include <string.h>
#include <string>
#include <vector>

namespace Dyninst { namespace InstructionAPI {
  class Instruction {

    union raw_insn_T {
#if defined(__powerpc__) || defined(__powerpc64__)
      unsigned int small_insn;
#else
      uintptr_t small_insn;
#endif
      unsigned char* large_insn;
    };

  public:
    friend class x86_decoder;
    friend class InstructionDecoder_power;
    friend class InstructionDecoder_aarch64;
    friend class InstructionDecoder_amdgpu_gfx908;
    friend class InstructionDecoder_amdgpu_gfx90a;
    friend class InstructionDecoder_amdgpu_gfx940;

    struct CFT {
      Expression::Ptr target;
      bool isCall;
      bool isIndirect;
      bool isConditional;
      bool isFallthrough;

      CFT(Expression::Ptr t, bool call, bool indir, bool cond, bool ft)
          : target(t), isCall(call), isIndirect(indir), isConditional(cond), isFallthrough(ft) {}
    };

    class category_t final {
      friend class Instruction;
      std::vector<InsnCategory> categories{};

    public:
      explicit category_t(std::vector<InsnCategory> c) noexcept : categories{std::move(c)} {}
      category_t() = default;
      category_t& operator=(std::vector<InsnCategory>&& c) {
        categories = std::move(c);
        return *this;
      }

      bool satisfies(InsnCategory category) const {
        for(auto c : categories) {
          if(c == category) {
            return true;
          }
        }
        return false;
      }
    };

    DYNINST_EXPORT Instruction(Operation what, size_t size, const unsigned char* raw,
                               Dyninst::Architecture arch);
    DYNINST_EXPORT Instruction();
    DYNINST_EXPORT ~Instruction();
    DYNINST_EXPORT Instruction(const Instruction& o);
    DYNINST_EXPORT const Instruction& operator=(const Instruction& rhs);
    DYNINST_EXPORT Instruction(Instruction&&) = default;
    DYNINST_EXPORT Instruction& operator=(Instruction&&) = default;

    DYNINST_EXPORT Operation& getOperation();
    DYNINST_EXPORT const Operation& getOperation() const;

    DYNINST_EXPORT std::vector<Operand> getAllOperands() const;
    DYNINST_EXPORT std::vector<Operand> getExplicitOperands() const;
    DYNINST_EXPORT std::vector<Operand> getImplicitOperands() const;

    DYNINST_DEPRECATED("Use getallOperands()") DYNINST_EXPORT
    void getOperands(std::vector<Operand>& operands) const;
    DYNINST_EXPORT Operand getOperand(int index) const;

    DYNINST_EXPORT std::vector<Operand> getDisplayOrderedOperands() const;

    DYNINST_EXPORT Operand getPredicateOperand() const;
    DYNINST_EXPORT bool hasPredicateOperand() const;

    DYNINST_EXPORT unsigned char rawByte(unsigned int index) const;
    DYNINST_EXPORT const void* ptr() const;

    DYNINST_EXPORT size_t size() const;

    DYNINST_EXPORT void getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const;
    DYNINST_EXPORT void getReadSet(std::set<RegisterAST::Ptr>& regsRead) const;

    DYNINST_EXPORT bool isRead(Expression::Ptr candidate) const;
    DYNINST_EXPORT bool isWritten(Expression::Ptr candidate) const;

    DYNINST_EXPORT bool readsMemory() const;
    DYNINST_EXPORT bool writesMemory() const;

    DYNINST_EXPORT void getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const;
    DYNINST_EXPORT void getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const;

    DYNINST_EXPORT Expression::Ptr getControlFlowTarget() const;

    DYNINST_EXPORT bool allowsFallThrough() const;

    DYNINST_EXPORT ArchSpecificFormatter& getFormatter() const;
    DYNINST_EXPORT std::string format(Address addr = 0) const;

    DYNINST_EXPORT bool isValid() const;
    DYNINST_EXPORT bool isLegalInsn() const;

    DYNINST_EXPORT Architecture getArch() const;

    DYNINST_EXPORT InsnCategory getCategory() const;
    DYNINST_EXPORT bool isCall() const { return checked_category(c_CallInsn); }
    DYNINST_EXPORT bool isReturn() const { return checked_category(c_ReturnInsn); }
    DYNINST_EXPORT bool isBranch() const { return checked_category(c_BranchInsn); }
    DYNINST_EXPORT bool isConditional() const { return checked_category(c_ConditionalInsn); }
    DYNINST_EXPORT bool isCompare() const { return checked_category(c_CompareInsn); }
    DYNINST_EXPORT bool isPrefetch() const { return checked_category(c_PrefetchInsn); }
    DYNINST_EXPORT bool isSysEnter() const { return checked_category(c_SysEnterInsn); }
    DYNINST_EXPORT bool isSyscall() const { return checked_category(c_SyscallInsn); }
    DYNINST_EXPORT bool isInterrupt() const { return checked_category(c_InterruptInsn); }
    DYNINST_EXPORT bool isVector() const { return checked_category(c_VectorInsn); }
    DYNINST_EXPORT bool isGPUKernelExit() const { return checked_category(c_GPUKernelExitInsn); }
    DYNINST_EXPORT bool isTransactional() const { return checked_category(c_TransactionalInsn); }
    DYNINST_EXPORT bool isSoftwareException() const { return isGPUKernelExit() || checked_category(c_SoftwareExceptionInsn); }

    typedef std::list<CFT>::const_iterator cftConstIter;
    DYNINST_EXPORT cftConstIter cft_begin() const { return m_Successors.begin(); }
    DYNINST_EXPORT cftConstIter cft_end() const { return m_Successors.end(); }

    DYNINST_EXPORT bool operator<(const Instruction& rhs) const {
      if(m_size < rhs.m_size)
        return true;
      if(m_size <= sizeof(m_RawInsn.small_insn)) {
        return m_RawInsn.small_insn < rhs.m_RawInsn.small_insn;
      }
      return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) < 0;
    }

    DYNINST_EXPORT bool operator==(const Instruction& rhs) const {
      if(m_size != rhs.m_size)
        return false;
      if(m_size <= sizeof(m_RawInsn.small_insn)) {
        return m_RawInsn.small_insn == rhs.m_RawInsn.small_insn;
      }
      return memcmp(m_RawInsn.large_insn, rhs.m_RawInsn.large_insn, m_size) == 0;
    }

    DYNINST_EXPORT void updateMnemonic(std::string new_mnemonic) { m_InsnOp.updateMnemonic(new_mnemonic); }

    typedef boost::shared_ptr<Instruction> Ptr;

  private:
    void updateSize(const unsigned int new_size, const unsigned char * raw);

    void addSuccessor(Expression::Ptr e, bool isCall, bool isIndirect, bool isConditional, bool isFallthrough,
                      bool isImplicit = false) const;
    void appendOperand(Expression::Ptr e, bool isRead, bool isWritten, bool isImplicit = false,
                       bool trueP = false, bool falseP = false) const;
    void copyRaw(size_t size, const unsigned char* raw);

    bool checked_category(InsnCategory c) const {
      if(arch_decoded_from == Arch_x86_64 || arch_decoded_from == Arch_x86) {
        return categories.satisfies(c);
      }
      return getCategory() == c;
    }

    mutable std::list<Operand> m_Operands;
    mutable Operation m_InsnOp;
    bool m_Valid;
    raw_insn_T m_RawInsn;
    unsigned int m_size{};
    Architecture arch_decoded_from;
    mutable std::list<CFT> m_Successors;
    // formatter is a non-owning pointer to a singleton object
    ArchSpecificFormatter* formatter;
    mutable category_t categories;
  };
}}

#endif //! defined(INSTRUCTION_H)
