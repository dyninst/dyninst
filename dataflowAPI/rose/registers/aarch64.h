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
      switch(baseID) {
        case Dyninst::aarch64::ix0: return armv8_gpr_r0;
        case Dyninst::aarch64::ix1: return armv8_gpr_r1;
        case Dyninst::aarch64::ix2: return armv8_gpr_r2;
        case Dyninst::aarch64::ix3: return armv8_gpr_r3;
        case Dyninst::aarch64::ix4: return armv8_gpr_r4;
        case Dyninst::aarch64::ix5: return armv8_gpr_r5;
        case Dyninst::aarch64::ix6: return armv8_gpr_r6;
        case Dyninst::aarch64::ix7: return armv8_gpr_r7;
        case Dyninst::aarch64::ix8: return armv8_gpr_r8;
        case Dyninst::aarch64::ix9: return armv8_gpr_r9;
        case Dyninst::aarch64::ix10: return armv8_gpr_r10;
        case Dyninst::aarch64::ix11: return armv8_gpr_r11;
        case Dyninst::aarch64::ix12: return armv8_gpr_r12;
        case Dyninst::aarch64::ix13: return armv8_gpr_r13;
        case Dyninst::aarch64::ix14: return armv8_gpr_r14;
        case Dyninst::aarch64::ix15: return armv8_gpr_r15;
        case Dyninst::aarch64::ix16: return armv8_gpr_r16;
        case Dyninst::aarch64::ix17: return armv8_gpr_r17;
        case Dyninst::aarch64::ix18: return armv8_gpr_r18;
        case Dyninst::aarch64::ix19: return armv8_gpr_r19;
        case Dyninst::aarch64::ix20: return armv8_gpr_r20;
        case Dyninst::aarch64::ix21: return armv8_gpr_r21;
        case Dyninst::aarch64::ix22: return armv8_gpr_r22;
        case Dyninst::aarch64::ix23: return armv8_gpr_r23;
        case Dyninst::aarch64::ix24: return armv8_gpr_r24;
        case Dyninst::aarch64::ix25: return armv8_gpr_r25;
        case Dyninst::aarch64::ix26: return armv8_gpr_r26;
        case Dyninst::aarch64::ix27: return armv8_gpr_r27;
        case Dyninst::aarch64::ix28: return armv8_gpr_r28;
        case Dyninst::aarch64::ix29: return armv8_gpr_r29;
        case Dyninst::aarch64::ix30: return armv8_gpr_r30;
      }
      convert_printf("Unknown aarch64 GPR '%d'\n", baseID);
      return static_cast<ARMv8GeneralPurposeRegister>(-1);
    }

    ARMv8SimdFpRegister fpr(int32_t baseID) {
      switch(baseID) {
        case Dyninst::aarch64::iq0: return armv8_simdfpr_v0;
        case Dyninst::aarch64::iq1: return armv8_simdfpr_v1;
        case Dyninst::aarch64::iq2: return armv8_simdfpr_v2;
        case Dyninst::aarch64::iq3: return armv8_simdfpr_v3;
        case Dyninst::aarch64::iq4: return armv8_simdfpr_v4;
        case Dyninst::aarch64::iq5: return armv8_simdfpr_v5;
        case Dyninst::aarch64::iq6: return armv8_simdfpr_v6;
        case Dyninst::aarch64::iq7: return armv8_simdfpr_v7;
        case Dyninst::aarch64::iq8: return armv8_simdfpr_v8;
        case Dyninst::aarch64::iq9: return armv8_simdfpr_v9;
        case Dyninst::aarch64::iq10: return armv8_simdfpr_v10;
        case Dyninst::aarch64::iq11: return armv8_simdfpr_v11;
        case Dyninst::aarch64::iq12: return armv8_simdfpr_v12;
        case Dyninst::aarch64::iq13: return armv8_simdfpr_v13;
        case Dyninst::aarch64::iq14: return armv8_simdfpr_v14;
        case Dyninst::aarch64::iq15: return armv8_simdfpr_v15;
        case Dyninst::aarch64::iq16: return armv8_simdfpr_v16;
        case Dyninst::aarch64::iq17: return armv8_simdfpr_v17;
        case Dyninst::aarch64::iq18: return armv8_simdfpr_v18;
        case Dyninst::aarch64::iq19: return armv8_simdfpr_v19;
        case Dyninst::aarch64::iq20: return armv8_simdfpr_v20;
        case Dyninst::aarch64::iq21: return armv8_simdfpr_v21;
        case Dyninst::aarch64::iq22: return armv8_simdfpr_v22;
        case Dyninst::aarch64::iq23: return armv8_simdfpr_v23;
        case Dyninst::aarch64::iq24: return armv8_simdfpr_v24;
        case Dyninst::aarch64::iq25: return armv8_simdfpr_v25;
        case Dyninst::aarch64::iq26: return armv8_simdfpr_v26;
        case Dyninst::aarch64::iq27: return armv8_simdfpr_v27;
        case Dyninst::aarch64::iq28: return armv8_simdfpr_v28;
        case Dyninst::aarch64::iq29: return armv8_simdfpr_v29;
        case Dyninst::aarch64::iq30: return armv8_simdfpr_v30;
        case Dyninst::aarch64::iq31: return armv8_simdfpr_v31;
      }
      convert_printf("Unknown aarch64 FPR '%d'\n", baseID);
      return static_cast<ARMv8SimdFpRegister>(-1);
    }

    ARMv8PstateFields pstate_field(int32_t baseID) {
      switch(baseID) {
        case Dyninst::aarch64::in: return armv8_pstatefield_n;
        case Dyninst::aarch64::iz: return armv8_pstatefield_z;
        case Dyninst::aarch64::ic: return armv8_pstatefield_c;
        case Dyninst::aarch64::iv: return armv8_pstatefield_v;
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
        auto const c = armv8_regclass_pstate;
        auto const n = 0;
        auto const p = aarch64_rose::pstate_field(baseID);
        return std::make_tuple(c, n, p, num_bits);
      }
    }
    convert_printf("Unknown aarch64 category '%d'\n", category);
    return std::make_tuple(armv8_regclass_unknown, -1, pos, 0);
  }
}

#endif
