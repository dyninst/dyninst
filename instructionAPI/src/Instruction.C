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

#include "../h/Instruction.h"
#include "../h/InstructionCategories.h"
#include "../h/Register.h"
#include "Dereference.h"
#include "InstructionDecoder.h"
#include "Operation_impl.h"
#include "common/src/arch-x86.h"
#include "dyninstversion.h"
#include "interrupts.h"
#include "entryIDs.h"

#include <algorithm>
#include <boost/iterator/indirect_iterator.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string>

using namespace std;
using namespace NS_x86;

#include "ArchSpecificFormatters.h"

namespace Dyninst { namespace InstructionAPI {

  namespace {
    bool is_valid_mnemonic(Dyninst::Architecture arch, entryID id) {
      switch(arch) {
        case Arch_x86:
        case Arch_x86_64:
          return id != e_No_Entry;

        case Arch_aarch64:
          return id != aarch64_op_INVALID;

        case Arch_ppc32:
        case Arch_ppc64:
          return id != power_op_INVALID;

        case Arch_cuda:
          return id != cuda_op_INVALID;

        case Arch_intelGen9:
          return id != intel_gpu_op_INVALID;

        case Arch_amdgpu_gfx908:
          return id != amdgpu_gfx908_op_INVALID;
        case Arch_amdgpu_gfx90a:
          return id != amdgpu_gfx90a_op_INVALID;
        case Arch_amdgpu_gfx940:
          return id != amdgpu_gfx940_op_INVALID;

        case Arch_riscv64:
          return id != riscv64_op_INVALID;

        case Arch_none:
        case Arch_aarch32:
          return false;
      }
      return false;
    }
  }

  DYNINST_EXPORT Instruction::Instruction(Operation what, size_t size, const unsigned char* raw,
                                          Dyninst::Architecture arch)
      : m_InsnOp(what), m_Valid(is_valid_mnemonic(arch, what.getID())), m_size{static_cast<decltype(m_size)>(size)},
        arch_decoded_from(arch), formatter(&ArchSpecificFormatter::getFormatter(arch)) {
    copyRaw(size, raw);
  }

  void Instruction::copyRaw(size_t size, const unsigned char* raw) {
    assert(size <= m_RawInsn.size() && "Requested size is larger than opcode buffer");

    auto last = std::copy_n(raw, size, m_RawInsn.data());

    // Zero-fill so 'operator<' and 'operator==' work correctly
    std::fill(last, m_RawInsn.end(), 0);
  }

  void Instruction::updateSize(const unsigned int new_size, const unsigned char * raw) {
    copyRaw(new_size, raw);
    this->m_size = new_size;
  }

  DYNINST_EXPORT Instruction::Instruction()
      : m_Valid(false), m_size(0), arch_decoded_from(Arch_none), formatter(nullptr) {
    copyRaw(0, nullptr);
  }

  DYNINST_EXPORT bool Instruction::isValid() const { return m_Valid; }

  DYNINST_EXPORT Operation& Instruction::getOperation() { return m_InsnOp; }

  DYNINST_EXPORT const Operation& Instruction::getOperation() const { return m_InsnOp; }

  std::vector<Operand> Instruction::getAllOperands() const {
    return std::vector<Operand>(m_Operands.begin(), m_Operands.end());
  }

  std::vector<Operand> Instruction::getExplicitOperands() const {
    std::vector<Operand> operands;
    for(auto const& o : m_Operands) {
      if(!o.isImplicit()) {
        operands.push_back(o);
      }
    }
    return operands;
  }
  std::vector<Operand> Instruction::getImplicitOperands() const {
    std::vector<Operand> operands;
    for(auto const& o : m_Operands) {
      if(o.isImplicit()) {
        operands.push_back(o);
      }
    }
    return operands;
  }

  void Instruction::getOperands(std::vector<Operand>& operands) const {
    for(auto const& o : getAllOperands()) {
      operands.push_back(o);
    }
  }

  DYNINST_EXPORT std::vector<Operand> Instruction::getDisplayOrderedOperands() const {
    auto operands = getExplicitOperands();

    if(formatter->operandPrintOrderReversed()) {
      std::reverse(operands.begin(), operands.end());
    }

    return operands;
  }

  DYNINST_EXPORT Operand Instruction::getOperand(int index) const {
    if(index < 0 || index >= (int)(m_Operands.size())) {
      // Out of range = empty operand
      return Operand(Expression::Ptr(), false, false);
    }
    std::list<Operand>::const_iterator found = m_Operands.begin();
    std::advance(found, index);
    return *found;
  }

  DYNINST_EXPORT const void* Instruction::ptr() const {
    return m_RawInsn.data();
  }

  DYNINST_EXPORT unsigned char Instruction::rawByte(unsigned int index) const {
    if(index >= m_size) {
      return 0;
    }
    return m_RawInsn[index];
  }

  DYNINST_EXPORT size_t Instruction::size() const { return m_size; }

  DYNINST_EXPORT void Instruction::getReadSet(std::set<RegisterAST::Ptr>& regsRead) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      curOperand->getReadSet(regsRead);
    }
    std::copy(m_InsnOp.implicitReads().begin(), m_InsnOp.implicitReads().end(),
              std::inserter(regsRead, regsRead.begin()));
  }

  DYNINST_EXPORT void Instruction::getWriteSet(std::set<RegisterAST::Ptr>& regsWritten) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      curOperand->getWriteSet(regsWritten);
    }
    std::copy(m_InsnOp.implicitWrites().begin(), m_InsnOp.implicitWrites().end(),
              std::inserter(regsWritten, regsWritten.begin()));
  }

  DYNINST_EXPORT bool Instruction::isRead(Expression::Ptr candidate) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      // Check if the candidate is read as an explicit operand
      if(curOperand->isRead(candidate)) {
        return true;
      }
    }
    // Check if the candidate is read as an implicit operand
    return m_InsnOp.isRead(candidate);
  }

  DYNINST_EXPORT bool Instruction::isWritten(Expression::Ptr candidate) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      if(curOperand->isWritten(candidate)) {
        return true;
      }
    }
    return m_InsnOp.isWritten(candidate);
  }

  DYNINST_EXPORT bool Instruction::readsMemory() const {
    if(isPrefetch()) {
      return false;
    }
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      if(curOperand->readsMemory()) {
        return true;
      }
    }
    return !m_InsnOp.getImplicitMemReads().empty();
  }

  DYNINST_EXPORT bool Instruction::writesMemory() const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      if(curOperand->writesMemory()) {
        return true;
      }
    }
    return !m_InsnOp.getImplicitMemWrites().empty();
  }

  DYNINST_EXPORT void
  Instruction::getMemoryReadOperands(std::set<Expression::Ptr>& memAccessors) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      curOperand->addEffectiveReadAddresses(memAccessors);
    }
    std::copy(m_InsnOp.getImplicitMemReads().begin(), m_InsnOp.getImplicitMemReads().end(),
              std::inserter(memAccessors, memAccessors.begin()));
  }

  DYNINST_EXPORT void
  Instruction::getMemoryWriteOperands(std::set<Expression::Ptr>& memAccessors) const {
    for(std::list<Operand>::const_iterator curOperand = m_Operands.begin();
        curOperand != m_Operands.end(); ++curOperand) {
      curOperand->addEffectiveWriteAddresses(memAccessors);
    }
    std::copy(m_InsnOp.getImplicitMemWrites().begin(), m_InsnOp.getImplicitMemWrites().end(),
              std::inserter(memAccessors, memAccessors.begin()));
  }

  DYNINST_EXPORT Operand Instruction::getPredicateOperand() const {
    for(auto const& op : m_Operands) {
      if(op.isTruePredicate() || op.isFalsePredicate()) {
        return op;
      }
    }

    return Operand(Expression::Ptr(), false, false);
  }

  DYNINST_EXPORT bool Instruction::hasPredicateOperand() const {
    for(auto const& op : m_Operands) {
      if(op.isTruePredicate() || op.isFalsePredicate()) {
        return true;
      }
    }

    return false;
  }

  DYNINST_EXPORT Expression::Ptr Instruction::getControlFlowTarget() const {
    // We assume control flow transfer instructions have the PC as
    // an implicit write, and that we have decoded the control flow
    // target's full location as the first and only operand.
    // If this is not the case, we'll squawk for the time being...
    if(getCategory() == c_NoCategory || isCompare() || isPrefetch()) {
      return Expression::Ptr();
    }
    if(m_Successors.empty()) {
      return Expression::Ptr();
    }
    return m_Successors.front().target;
  }

  DYNINST_EXPORT ArchSpecificFormatter& Instruction::getFormatter() const { return *formatter; }

  DYNINST_EXPORT std::string Instruction::format(Address addr) const {
    // if Arch_none, this is an error and the formatter is nullptr,
    // so return an error string (could also abort or except)
    if(arch_decoded_from == Arch_none) {
      return "ERROR_NO_ARCH_SET_FOR_INSTRUCTION";
    }

    // remove this once ArchSpecificFormatter is extended for all architectures

    std::string opstr = m_InsnOp.format();
    opstr += " ";
    std::list<Operand>::const_iterator currOperand;
    std::vector<std::string> formattedOperands;
    for(currOperand = m_Operands.begin(); currOperand != m_Operands.end(); ++currOperand) {
      /* If this operand is implicit, don't put it in the list of operands. */
      if(currOperand->isImplicit())
        continue;

      formattedOperands.push_back(currOperand->format(getArch(), addr));
    }

    return opstr + formatter->getInstructionString(formattedOperands);
  }

  DYNINST_EXPORT bool Instruction::allowsFallThrough() const {
    switch(m_InsnOp.getID()) {
      case e_ret_far:
      case e_ret_near:
      case e_iret:
      case e_jmp:
      case e_hlt:
      case e_sysret:
      case e_sysexit:
      case e_call:
      case e_syscall:
      case amdgpu_gfx908_op_S_SETPC_B64:
      case amdgpu_gfx908_op_S_SWAPPC_B64:
      case amdgpu_gfx90a_op_S_SETPC_B64:
      case amdgpu_gfx90a_op_S_SWAPPC_B64:
      case amdgpu_gfx940_op_S_SETPC_B64:
      case amdgpu_gfx940_op_S_SWAPPC_B64: return false;
      case e_jae:
      case e_jb:
      case e_jb_jnaej_j:
      case e_jbe:
      case e_jcxz_jec:
      case e_jl:
      case e_jle:
      case e_jnb_jae_j:
      case e_ja:
      case e_jge:
      case e_jg:
      case e_jno:
      case e_jnp:
      case e_jns:
      case e_jne:
      case e_jo:
      case e_jp:
      case e_js:
      case e_je: return true;
      default: {
        for(cftConstIter targ = m_Successors.begin(); targ != m_Successors.end(); ++targ) {
          if(targ->isFallthrough)
            return true;
        }
        return m_Successors.empty();
      }
    }
    // can't happen but make the compiler happy
    return false;
  }

  DYNINST_EXPORT bool Instruction::isLegalInsn() const { return m_Valid; }

  DYNINST_EXPORT Architecture Instruction::getArch() const { return arch_decoded_from; }

  DYNINST_EXPORT InsnCategory Instruction::getCategory() const {
    if(m_InsnOp.isVectorInsn)
      return c_VectorInsn;
    InsnCategory c = entryToCategory(m_InsnOp.getID());
    if(c == c_BranchInsn && (arch_decoded_from == Arch_ppc32 || arch_decoded_from == Arch_ppc64)) {
      for(cftConstIter cft = cft_begin(); cft != cft_end(); ++cft) {
        if(cft->isCall) {
          return c_CallInsn;
        }
      }
    }
    if(m_InsnOp.getID() == power_op_bclr) {
      if(this->allowsFallThrough()) {
        return c_BranchInsn;
      }
      return c_ReturnInsn;
    }
    if(isSoftwareInterrupt(*this)) {
      return c_InterruptInsn;
    }
    return c;
  }

  void Instruction::addSuccessor(Expression::Ptr e, bool isCall, bool isIndirect,
                                 bool isConditional, bool isFallthrough, bool isImplicit) const {
    CFT c(e, isCall, isIndirect, isConditional, isFallthrough);
    m_Successors.push_back(c);
    if(!isFallthrough)
      appendOperand(e, true, false, isImplicit);
  }

  void Instruction::appendOperand(Expression::Ptr e, bool isRead, bool isWritten, bool isImplicit,
                                  bool trueP, bool falseP) const {
    m_Operands.push_back(Operand(e, isRead, isWritten, isImplicit, trueP, falseP));
  }

}}
