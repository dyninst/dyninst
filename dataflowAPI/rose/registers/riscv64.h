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

#ifndef DYNINST_COMMON_REGISTERS_ROSE_RISCV64_H
#define DYNINST_COMMON_REGISTERS_ROSE_RISCV64_H

#include "external/rose/riscv64InstructionEnum.h"
#include "registers/riscv64_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {

  namespace riscv64_rose {

    Riscv64GeneralPurposeRegister gpr(int32_t baseID) {
      switch(baseID) {
        case Dyninst::riscv64::ix0: return riscv64_gpr_x0;
        case Dyninst::riscv64::ix1: return riscv64_gpr_x1;
        case Dyninst::riscv64::ix2: return riscv64_gpr_x2;
        case Dyninst::riscv64::ix3: return riscv64_gpr_x3;
        case Dyninst::riscv64::ix4: return riscv64_gpr_x4;
        case Dyninst::riscv64::ix5: return riscv64_gpr_x5;
        case Dyninst::riscv64::ix6: return riscv64_gpr_x6;
        case Dyninst::riscv64::ix7: return riscv64_gpr_x7;
        case Dyninst::riscv64::ix8: return riscv64_gpr_x8;
        case Dyninst::riscv64::ix9: return riscv64_gpr_x9;
        case Dyninst::riscv64::ix10: return riscv64_gpr_x10;
        case Dyninst::riscv64::ix11: return riscv64_gpr_x11;
        case Dyninst::riscv64::ix12: return riscv64_gpr_x12;
        case Dyninst::riscv64::ix13: return riscv64_gpr_x13;
        case Dyninst::riscv64::ix14: return riscv64_gpr_x14;
        case Dyninst::riscv64::ix15: return riscv64_gpr_x15;
        case Dyninst::riscv64::ix16: return riscv64_gpr_x16;
        case Dyninst::riscv64::ix17: return riscv64_gpr_x17;
        case Dyninst::riscv64::ix18: return riscv64_gpr_x18;
        case Dyninst::riscv64::ix19: return riscv64_gpr_x19;
        case Dyninst::riscv64::ix20: return riscv64_gpr_x20;
        case Dyninst::riscv64::ix21: return riscv64_gpr_x21;
        case Dyninst::riscv64::ix22: return riscv64_gpr_x22;
        case Dyninst::riscv64::ix23: return riscv64_gpr_x23;
        case Dyninst::riscv64::ix24: return riscv64_gpr_x24;
        case Dyninst::riscv64::ix25: return riscv64_gpr_x25;
        case Dyninst::riscv64::ix26: return riscv64_gpr_x26;
        case Dyninst::riscv64::ix27: return riscv64_gpr_x27;
        case Dyninst::riscv64::ix28: return riscv64_gpr_x28;
        case Dyninst::riscv64::ix29: return riscv64_gpr_x29;
        case Dyninst::riscv64::ix30: return riscv64_gpr_x30;
      }
      convert_printf("Unknown riscv64 GPR '%d'\n", baseID);
      return static_cast<Riscv64GeneralPurposeRegister>(-1);
    }

    Riscv64FloatingPointRegister fpr(int32_t baseID) {
      switch(baseID) {
        case Dyninst::riscv64::if0: return riscv64_fpr_f0;
        case Dyninst::riscv64::if1: return riscv64_fpr_f1;
        case Dyninst::riscv64::if2: return riscv64_fpr_f2;
        case Dyninst::riscv64::if3: return riscv64_fpr_f3;
        case Dyninst::riscv64::if4: return riscv64_fpr_f4;
        case Dyninst::riscv64::if5: return riscv64_fpr_f5;
        case Dyninst::riscv64::if6: return riscv64_fpr_f6;
        case Dyninst::riscv64::if7: return riscv64_fpr_f7;
        case Dyninst::riscv64::if8: return riscv64_fpr_f8;
        case Dyninst::riscv64::if9: return riscv64_fpr_f9;
        case Dyninst::riscv64::if10: return riscv64_fpr_f10;
        case Dyninst::riscv64::if11: return riscv64_fpr_f11;
        case Dyninst::riscv64::if12: return riscv64_fpr_f12;
        case Dyninst::riscv64::if13: return riscv64_fpr_f13;
        case Dyninst::riscv64::if14: return riscv64_fpr_f14;
        case Dyninst::riscv64::if15: return riscv64_fpr_f15;
        case Dyninst::riscv64::if16: return riscv64_fpr_f16;
        case Dyninst::riscv64::if17: return riscv64_fpr_f17;
        case Dyninst::riscv64::if18: return riscv64_fpr_f18;
        case Dyninst::riscv64::if19: return riscv64_fpr_f19;
        case Dyninst::riscv64::if20: return riscv64_fpr_f20;
        case Dyninst::riscv64::if21: return riscv64_fpr_f21;
        case Dyninst::riscv64::if22: return riscv64_fpr_f22;
        case Dyninst::riscv64::if23: return riscv64_fpr_f23;
        case Dyninst::riscv64::if24: return riscv64_fpr_f24;
        case Dyninst::riscv64::if25: return riscv64_fpr_f25;
        case Dyninst::riscv64::if26: return riscv64_fpr_f26;
        case Dyninst::riscv64::if27: return riscv64_fpr_f27;
        case Dyninst::riscv64::if28: return riscv64_fpr_f28;
        case Dyninst::riscv64::if29: return riscv64_fpr_f29;
        case Dyninst::riscv64::if30: return riscv64_fpr_f30;
        case Dyninst::riscv64::if31: return riscv64_fpr_f31;
      }
      convert_printf("Unknown riscv64 FPR '%d'\n", baseID);
      return static_cast<Riscv64FloatingPointRegister>(-1);
    }
  }

  std::tuple<Riscv64RegisterClass, int, int, int>
  riscv64Rose(int32_t category, int32_t baseID, int32_t num_bits) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::riscv64::GPR: {
        auto const c = riscv64_regclass_gpr;
        auto const n = riscv64_rose::gpr(baseID);
        return std::make_tuple(c, n, pos, num_bits);
      }

      case Dyninst::riscv64::FPR: {
        auto const c = riscv64_regclass_fpr;
        auto const n = riscv64_rose::fpr(baseID);
        return std::make_tuple(c, n, pos, num_bits);
      }
    }
    convert_printf("Unknown riscv64 category '%d'\n", category);
    return std::make_tuple(riscv64_regclass_unknown, -1, pos, 0);
  }
}

#endif
