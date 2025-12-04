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

#ifndef INSTRUCTIONAPI_RISCV_CATEGORIES_H
#define INSTRUCTIONAPI_RISCV_CATEGORIES_H

#include "Instruction.h"
#include "capstone/capstone.h"
#include "capstone/riscv.h"
#include "decoder.h"

namespace Dyninst {
namespace InstructionAPI {
namespace riscv {

namespace di = Dyninst::InstructionAPI;

inline std::vector<di::InsnCategory>
decode_categories(di::Instruction &insn, di::InstructionDecoder_riscv64::disassem const &dis,
                  std::vector<cs_riscv_op> const &operands) {
  auto const num_categories = dis.insn->detail->groups_count;
  auto const groups = dis.insn->detail->groups;

  std::vector<di::InsnCategory> categories{};
  categories.reserve(num_categories);

  for (auto i = uint8_t{}; i != num_categories; i++) {
    switch (groups[i]) {
    case RISCV_GRP_JUMP:
    case RISCV_GRP_BRANCH_RELATIVE:
      categories.push_back(di::c_BranchInsn);
      break;
    case RISCV_GRP_CALL:
      categories.push_back(di::c_CallInsn);
      break;
    case RISCV_GRP_RET:
      categories.push_back(di::c_ReturnInsn);
      break;
    case RISCV_GRP_INT:
      categories.push_back(di::c_InterruptInsn);
      break;
    case RISCV_GRP_IRET:
      categories.push_back(di::c_InterruptInsn);
      categories.push_back(di::c_ReturnInsn);
      break;
    case RISCV_GRP_ISRV32:
    case RISCV_GRP_ISRV64:
      categories.push_back(di::c_VectorInsn);
      break;
    }
  }

  // Relative calls aren't branches
  {
    auto is_call = [&categories]() {
      auto itr =
          std::find(categories.begin(), categories.end(), di::c_CallInsn);
      return itr != categories.end();
    }();
    auto is_branch = [&categories]() {
      auto itr =
          std::find(categories.begin(), categories.end(), di::c_BranchInsn);
      return itr != categories.end();
    }();

    if (is_call && is_branch) {
      auto itr =
          std::remove(categories.begin(), categories.end(), di::c_BranchInsn);
      categories.erase(itr);
    }
  }

  // Capstone's instruction categories do not take operands into consideration
  // For example, "c.jr ra" is a return instruction, but Capstone returns a
  // branch
  switch (insn.getOperation().getID()) {
  case riscv64_op_jal: {
    if (operands[0].reg == RISCV_REG_ZERO) {
      // jal zero, ... is branch
      categories.erase(std::remove(categories.begin(), categories.end(), di::c_CallInsn), categories.end());
      // ret (jalr zero, ra, 0)
      categories.push_back(di::c_BranchInsn);
    }
    break;
  }
  case riscv64_op_jalr: {
    if (operands[0].reg == RISCV_REG_ZERO) {
      categories.erase(std::remove(categories.begin(), categories.end(), di::c_CallInsn), categories.end());
      categories.erase(std::remove(categories.begin(), categories.end(), di::c_BranchInsn), categories.end());
      // ret (jalr zero, ra, 0)
      if (operands[1].reg == RISCV_REG_RA && operands[2].imm == 0) {
        categories.push_back(di::c_ReturnInsn);
      }
      // jalr zero, ... is branch
      else {
        categories.push_back(di::c_BranchInsn);
      }
    }
    break;
  }
  case riscv64_op_beq:
  case riscv64_op_bne:
  case riscv64_op_blt:
  case riscv64_op_bge:
  case riscv64_op_bltu:
  case riscv64_op_bgeu: {
    if (operands[0].reg != RISCV_REG_ZERO ||
        operands[1].reg != RISCV_REG_ZERO) {
      categories.push_back(di::c_ConditionalInsn);
    }
    break;
  }
  case riscv64_op_ebreak: {
    categories.push_back(di::c_InterruptInsn);
    break;
  }
  default: {
    break;
  }
  }

  // Remove any duplicates
  std::sort(categories.begin(), categories.end());
  categories.erase(std::unique(categories.begin(), categories.end()),
                   categories.end());

  return categories;
}

} // namespace riscv
} // namespace InstructionAPI
} // namespace Dyninst

#endif
