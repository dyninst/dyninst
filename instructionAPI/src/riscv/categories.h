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
#include "capstone/riscv.h"
#include "decoder.h"
#include "syscalls.h"

namespace Dyninst { namespace InstructionAPI { namespace riscv {

  namespace di = Dyninst::InstructionAPI;

  inline std::vector<di::InsnCategory> decode_categories(di::Instruction &insn,
                                                         di::riscv_decoder::disassem const& dis) {
    auto const num_categories = dis.insn->detail->groups_count;
    auto const groups = dis.insn->detail->groups;

    std::vector<di::InsnCategory> categories{};
    categories.reserve(num_categories);

    for(auto i = uint8_t{}; i != num_categories; i++) {
      switch(groups[i]) {
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
