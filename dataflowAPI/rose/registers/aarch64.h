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

  std::tuple<ARMv8RegisterClass, int, int, int>
  aarch64Rose(int32_t category, int32_t baseID, int32_t subrange, int32_t num_bits) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::aarch64::GPR: {
        auto const c = armv8_regclass_gpr;
        auto const regnum = baseID - (Dyninst::aarch64::x0 & 0xFF);
        auto const n = armv8_gpr_r0 + regnum;
        return std::make_tuple(c, n, pos, num_bits);
      }

      case Dyninst::aarch64::SPR: {
        int n = 0;
        ARMv8RegisterClass c;
        if(baseID == (Dyninst::aarch64::pstate & 0xFF)) {
          c = armv8_regclass_pstate;
        }
        return std::make_tuple(c, n, pos, num_bits);
      }

      case Dyninst::aarch64::FPR: {
        auto c = armv8_regclass_simd_fpr;
        auto p = 0;
        int firstRegId;
        switch(subrange) {
          case Dyninst::aarch64::Q_REG:
            firstRegId = (Dyninst::aarch64::q0 & 0xFF);
            break;
          case Dyninst::aarch64::HQ_REG:
            firstRegId = (Dyninst::aarch64::hq0 & 0xFF);
            p = 64;
            break;
          case Dyninst::aarch64::FULL:
            firstRegId = (Dyninst::aarch64::d0 & 0xFF);
            break;
          case Dyninst::aarch64::D_REG:
            firstRegId = (Dyninst::aarch64::s0 & 0xFF);
            break;
          case Dyninst::aarch64::W_REG:
            firstRegId = (Dyninst::aarch64::h0 & 0xFF);
            break;
          case Dyninst::aarch64::B_REG:
            firstRegId = (Dyninst::aarch64::b0 & 0xFF);
            break;
          default: assert(!"invalid register subcategory for ARM64!"); break;
        }
        auto n = armv8_simdfpr_v0 + (baseID - firstRegId);
        return std::make_tuple(c, n, p, num_bits);
      }

      case Dyninst::aarch64::FLAG: {
        auto c = armv8_regclass_pstate;
        int n = 0;
        int p = pos;
        switch(baseID) {
          case Dyninst::aarch64::N_FLAG: p = armv8_pstatefield_n; break;
          case Dyninst::aarch64::Z_FLAG: p = armv8_pstatefield_z; break;
          case Dyninst::aarch64::V_FLAG: p = armv8_pstatefield_v; break;
          case Dyninst::aarch64::C_FLAG: p = armv8_pstatefield_c; break;
          default:
            c = static_cast<ARMv8RegisterClass>(-1);
        }
        return std::make_tuple(c, n, p, num_bits);
      }
    }
    convert_printf("Unknown aarch64 category '%d'\n", category);
    return std::make_tuple(armv8_regclass_unknown, -1, pos, 0);
  }
}

#endif
