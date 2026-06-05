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

#ifndef INSTRUCTIONAPI_X86_CATEGORIES_H
#define INSTRUCTIONAPI_X86_CATEGORIES_H

#include "Instruction.h"
#include "capstone/capstone.h"
#include "capstone/x86.h"
#include "decoder.h"
#include "syscalls.h"

namespace Dyninst { namespace InstructionAPI { namespace x86 {

  namespace di = Dyninst::InstructionAPI;

  inline std::vector<di::InsnCategory> decode_categories(di::Instruction &insn,
                                                         di::x86_decoder::disassem const& dis) {
    auto const num_categories = dis.insn->detail->groups_count;
    auto const groups = dis.insn->detail->groups;

    std::vector<di::InsnCategory> categories{};
    categories.reserve(num_categories);

    for(auto i = uint8_t{}; i != num_categories; i++) {
      switch(groups[i]) {
        case X86_GRP_JUMP:
        case X86_GRP_BRANCH_RELATIVE:
          categories.push_back(di::c_BranchInsn);
          break;
        case X86_GRP_CALL:
          categories.push_back(di::c_CallInsn);
          break;
        case X86_GRP_RET:
          categories.push_back(di::c_ReturnInsn);
          break;
        case X86_GRP_INT:
          categories.push_back(di::c_InterruptInsn);
          break;
        case X86_GRP_IRET:
          categories.push_back(di::c_InterruptInsn);
          categories.push_back(di::c_ReturnInsn);
          break;
        case X86_GRP_3DNOW:
        case X86_GRP_AVX:
        case X86_GRP_AVX2:
        case X86_GRP_AVX512:
        case X86_GRP_FMA:
        case X86_GRP_FMA4:
        case X86_GRP_MMX:
        case X86_GRP_SSE1:
        case X86_GRP_SSE2:
        case X86_GRP_SSE3:
        case X86_GRP_SSE41:
        case X86_GRP_SSE42:
        case X86_GRP_SSE4A:
        case X86_GRP_SSSE3:
        case X86_GRP_F16C:   // 16-bit float extension
        case X86_GRP_PCLMUL: // PCLMULQDQ
        case X86_GRP_CDI:    // AVX-512 Conflict Detection
        case X86_GRP_DQI:    // AVX-512 Doubleword and Quadword
        case X86_GRP_BWI:    // AVX-512 Byte and Word
        case X86_GRP_PFI:    // AVX-512 Prefetch
        case X86_GRP_ERI:    // AVX-512 Exponential and Reciprocal (Knight's Landing)
        case X86_GRP_VLX:    // AVX-512 Vector Length Extensions
        case X86_GRP_NOVLX:  // Opcode shared with VLX, but doesn't use VLX
        case X86_GRP_XOP:
          categories.push_back(di::c_VectorInsn);
          break;
        case X86_GRP_HLE: // TSX Hardware Lock Elision
        case X86_GRP_RTM: // TSX Restricted Transactional Memory
          categories.push_back(di::c_TransactionalInsn);
          break;
        case X86_GRP_CMOV:
          categories.push_back(di::c_ConditionalInsn);
      }

      if(groups[i] == X86_GRP_PFI) {
        // Capstone currently only sets this for AVX-512PF instructions
        categories.push_back(di::c_PrefetchInsn);
      } else {
        switch(insn.getOperation().getID()) {
          case e_prefetch:
          case e_prefetchnta:
          case e_prefetcht0:
          case e_prefetcht1:
          case e_prefetcht2:
          case e_prefetchw:
          case e_prefetchwt1:
            categories.push_back(di::c_PrefetchInsn);
            break;
          default:
            break;
        }
      }
    }

    // Relative calls aren't branches
    {
      auto is_call = [&categories](){
        auto itr = std::find(categories.begin(), categories.end(), di::c_CallInsn);
        return itr != categories.end();
      }();
      auto is_branch = [&categories](){
        auto itr = std::find(categories.begin(), categories.end(), di::c_BranchInsn);
        return itr != categories.end();
      }();

      if(is_call && is_branch) {
        auto itr = std::remove(categories.begin(), categories.end(), di::c_BranchInsn);
        categories.erase(itr);
      }
    }

    // vector comparison instructions
    auto const& x86 = dis.insn->detail->x86;
    if(x86.xop_cc != X86_XOP_CC_INVALID || x86.sse_cc != X86_SSE_CC_INVALID ||
       x86.avx_cc != X86_AVX_CC_INVALID) {
      categories.push_back(di::c_CompareInsn);
    }

    // non-vector comparison instruction
    switch(insn.getOperation().getID()) {
      case e_cmp:
      case e_cmppd:
      case e_cmpps:
      case e_cmpsb:
      case e_cmpsd:
      case e_cmpsq:
      case e_cmpss:
      case e_cmpsw:
      case e_cmpxchg:
      case e_cmpxchg16b:
      case e_cmpxchg8b:
        categories.push_back(di::c_CompareInsn);
        break;
      default:
        break;
    }

    // Any branch that reads from the flags register is conditional.
    if(cs_reg_read(dis.handle, dis.insn, X86_REG_EFLAGS)) {
      const auto itr = std::find(categories.begin(), categories.end(), di::c_BranchInsn);
      const bool is_branch = itr != categories.end();
      if(is_branch) {
        categories.push_back(di::c_ConditionalInsn);
      }
    }

    // Some conditional instructions don't read from the flag register
    switch(insn.getOperation().getID()) {
      case e_loop:
      case e_jcxz:
        categories.push_back(di::c_ConditionalInsn);
        break;
      default:
        break;
    }

    // Masked vector operations
    {
      const bool is_masked = [&]() {
        auto* d = dis.insn->detail;
        for(uint8_t i = 0; i < d->x86.op_count; ++i) {
          auto const& operand = d->x86.operands[i];
          if(operand.avx_zero_opmask)
            return true;
        }
        return false;
      }();
      if(is_masked) {
        categories.push_back(di::c_ConditionalInsn);
      }
    }

    // Software exceptions
    switch(insn.getOperation().getID()) {
      case e_hlt:
      case e_int3:
      case e_ud2:
        categories.push_back(di::c_SoftwareExceptionInsn);
        break;
      default:
        break;
    }

    // Only for backward compatibility
    {
      switch(insn.getOperation().getID()) {
        case e_sysenter:
          // Instruction::isSysEnter()
          categories.push_back(di::c_SysEnterInsn);
          break;

        case e_syscall:
        case e_int:
          // Instruction::isSyscall()
          categories.push_back(di::c_SyscallInsn);
          break;

        default:
          break;
      }
    }

    // Remove any duplicates
    std::sort(categories.begin(), categories.end());
    categories.erase(std::unique(categories.begin(), categories.end()), categories.end());

    // None were found
    if(!categories.size()) {
      categories.push_back(di::c_NoCategory);
    }

    return categories;
  }

}}}

#endif
