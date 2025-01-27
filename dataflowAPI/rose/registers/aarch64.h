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

#ifndef DYNINST_COMMON_REGISTERS_ROSE_AARCH64_H
#define DYNINST_COMMON_REGISTERS_ROSE_AARCH64_H

#include "external/rose/armv8InstructionEnum.h"
#include "registers/aarch64_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {

  namespace aarch64_rose {

    ARMv8GeneralPurposeRegister gpr(int32_t baseID) {
      // The GPRs are in sequential order, so we can map
      // directly to an offset relative to x0.
      // NOTE: `baseID` is guaranteed to be from the
      //       most-basal GPR at this point.
      auto const id = baseID - (Dyninst::aarch64::x0 & 0x000000ff);
      switch(id) {
        case 0: return armv8_gpr_r0;
        case 1: return armv8_gpr_r1;
        case 2: return armv8_gpr_r2;
        case 3: return armv8_gpr_r3;
        case 4: return armv8_gpr_r4;
        case 5: return armv8_gpr_r5;
        case 6: return armv8_gpr_r6;
        case 7: return armv8_gpr_r7;
        case 8: return armv8_gpr_r8;
        case 9: return armv8_gpr_r9;
        case 10: return armv8_gpr_r10;
        case 11: return armv8_gpr_r11;
        case 12: return armv8_gpr_r12;
        case 13: return armv8_gpr_r13;
        case 14: return armv8_gpr_r14;
        case 15: return armv8_gpr_r15;
        case 16: return armv8_gpr_r16;
        case 17: return armv8_gpr_r17;
        case 18: return armv8_gpr_r18;
        case 19: return armv8_gpr_r19;
        case 20: return armv8_gpr_r20;
        case 21: return armv8_gpr_r21;
        case 22: return armv8_gpr_r22;
        case 23: return armv8_gpr_r23;
        case 24: return armv8_gpr_r24;
        case 25: return armv8_gpr_r25;
        case 26: return armv8_gpr_r26;
        case 27: return armv8_gpr_r27;
        case 28: return armv8_gpr_r28;
        case 29: return armv8_gpr_r29;
        case 30: return armv8_gpr_r30;
      }
      convert_printf("Unknown aarch64 GPR '%d'\n", baseID);
      return static_cast<ARMv8GeneralPurposeRegister>(-1);
    }

    ARMv8SimdFpRegister fpr(int32_t baseID) {
      // The FPRs are in sequential order, so we can map
      // directly to an offset relative to q0.
      // NOTE: `baseID` is guaranteed to be from the
      //       most-basal FPR at this point.
      auto const id = baseID - (Dyninst::aarch64::q0 & 0x000000ff);
      switch(id) {
        case 0: return armv8_simdfpr_v0;
        case 1: return armv8_simdfpr_v1;
        case 2: return armv8_simdfpr_v2;
        case 3: return armv8_simdfpr_v3;
        case 4: return armv8_simdfpr_v4;
        case 5: return armv8_simdfpr_v5;
        case 6: return armv8_simdfpr_v6;
        case 7: return armv8_simdfpr_v7;
        case 8: return armv8_simdfpr_v8;
        case 9: return armv8_simdfpr_v9;
        case 10: return armv8_simdfpr_v10;
        case 11: return armv8_simdfpr_v11;
        case 12: return armv8_simdfpr_v12;
        case 13: return armv8_simdfpr_v13;
        case 14: return armv8_simdfpr_v14;
        case 15: return armv8_simdfpr_v15;
        case 16: return armv8_simdfpr_v16;
        case 17: return armv8_simdfpr_v17;
        case 18: return armv8_simdfpr_v18;
        case 19: return armv8_simdfpr_v19;
        case 20: return armv8_simdfpr_v20;
        case 21: return armv8_simdfpr_v21;
        case 22: return armv8_simdfpr_v22;
        case 23: return armv8_simdfpr_v23;
        case 24: return armv8_simdfpr_v24;
        case 25: return armv8_simdfpr_v25;
        case 26: return armv8_simdfpr_v26;
        case 27: return armv8_simdfpr_v27;
        case 28: return armv8_simdfpr_v28;
        case 29: return armv8_simdfpr_v29;
        case 30: return armv8_simdfpr_v30;
        case 31: return armv8_simdfpr_v31;
      }
      convert_printf("Unknown aarch64 FPR '%d'\n", baseID);
      return static_cast<ARMv8SimdFpRegister>(-1);
    }

    ARMv8PstateFields pstate_field(int32_t baseID) {
      switch(baseID) {
        case 0: return armv8_pstatefield_pstate;
        case 1: return armv8_pstatefield_n;
        case 2: return armv8_pstatefield_z;
        case 3: return armv8_pstatefield_c;
        case 4: return armv8_pstatefield_v;
      }
      convert_printf("Unknown aarch64 pstate register '%d'\n", baseID);
      return static_cast<ARMv8PstateFields>(-1);
    }
  }

  std::tuple<ARMv8RegisterClass, int, int, int>
  aarch64Rose(int32_t category, int32_t baseID, int32_t subrange, int32_t num_bits) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::aarch64::GPR: {
        auto const c = armv8_regclass_gpr;
        auto const n = aarch64_rose::gpr(baseID);
        return std::make_tuple(c, n, pos, num_bits);
      }

      case Dyninst::aarch64::FPR: {
        auto const c = armv8_regclass_simd_fpr;
        auto const n = aarch64_rose::fpr(baseID);
        auto const p = (subrange == Dyninst::aarch64::HQ_REG) ? 64 : pos;
        return std::make_tuple(c, n, p, num_bits);
      }

      case Dyninst::aarch64::FLAG: {
        // ROSE supports NZCV, but not DAIF
        auto const c = armv8_regclass_pstate;
        auto const n = 0;  // ROSE docs: only minor value allowed is 0
        auto const p = aarch64_rose::pstate_field(baseID);
        return std::make_tuple(c, n, p, num_bits);
      }
    }
    convert_printf("Unknown aarch64 category '%d'\n", category);
    return std::make_tuple(armv8_regclass_unknown, -1, pos, 0);
  }
}

#endif
