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

#ifndef DYNINST_COMMON_REGISTERS_ROSE_PPC64_H
#define DYNINST_COMMON_REGISTERS_ROSE_PPC64_H

#include "external/rose/powerpcInstructionEnum.h"
#include "registers/ppc64_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {

  std::tuple<PowerpcRegisterClass, int, int, int>
  ppc64Rose(int32_t category, int32_t reg, int32_t num_bits) {
    auto const baseID = reg & 0x0000FFFF;
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::ppc64::GPR:
        return std::make_tuple(powerpc_regclass_gpr, baseID, pos, num_bits);

      case Dyninst::ppc64::FPR:
      case Dyninst::ppc64::FSR:
        return std::make_tuple(powerpc_regclass_fpr, baseID, pos, num_bits);

      case Dyninst::ppc64::SPR: {
        if(baseID < 613) {
          return std::make_tuple(powerpc_regclass_spr, baseID, pos, num_bits);
        }
        if(baseID < 621) {
          return std::make_tuple(powerpc_regclass_sr, baseID, pos, num_bits);
        }
        // ROSE treats CR as one register, so `minor` is always 0.
        constexpr auto minor = 0;
        auto const offset = baseID - 621;
        auto const pos = 4 * offset;
        constexpr auto nbits = 4;
        return std::make_tuple(powerpc_regclass_cr, minor, pos, nbits);
      }
    }
    convert_printf("Unknown ppc64 category '%d'\n", category);
    return std::make_tuple(powerpc_regclass_unknown, baseID, 0, 0);
  }
}

#endif
