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

#ifndef DYNINST_COMMON_REGISTERS_ROSE_PPC32_H
#define DYNINST_COMMON_REGISTERS_ROSE_PPC32_H

#include "external/rose/powerpcInstructionEnum.h"
#include "registers/ppc32_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {

  std::tuple<PowerpcRegisterClass, int, int, int>
  ppc32Rose(int32_t category, Dyninst::MachRegister reg, int32_t num_bits) {
    int const baseID = reg.val() & 0x0000FFFF;
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::ppc32::GPR:
        return std::make_tuple(powerpc_regclass_gpr, baseID, pos, num_bits);

      case Dyninst::ppc32::FPR:
      case Dyninst::ppc32::FSR:
        return std::make_tuple(powerpc_regclass_fpr, baseID, pos, num_bits);

      case Dyninst::ppc32::SPR: {
        if(baseID >= 1 && baseID <= 282) {
          // mq (0) doesn't have a ROSE representation
          // xer (1) to crt (9) map directly to the PowerpcSpecialPurposeRegister values
          // amr (13) and dscr (17) don't have ROSE representations
          // dsisr (18) to dec (22) map directly to the PowerpcSpecialPurposeRegister values
          // sdr1 (25) to ear (282) don't have ROSE representations
          return std::make_tuple(powerpc_regclass_spr, baseID, 0, num_bits);
        }
        if(reg == Dyninst::ppc32::tbl_wo || reg == Dyninst::ppc32::tbl_ro) {
          return std::make_tuple(powerpc_regclass_tbr, powerpc_tbr_tbl, 0, num_bits);
        }
        if(reg == Dyninst::ppc32::tbu_wo || reg == Dyninst::ppc32::tbu_ro) {
          return std::make_tuple(powerpc_regclass_tbr, powerpc_tbr_tbu, 0, num_bits);
        }
        if(reg == Dyninst::ppc32::pvr) {
          return std::make_tuple(powerpc_regclass_pvr, 0, 0, num_bits);
        }
        if(baseID >= 528 && baseID <= 600) {
          // ibat9u (528) to pc (600) don't have ROSE representations
          return std::make_tuple(powerpc_regclass_spr, baseID, 0, 0);
        }
        if(baseID >= 601 && baseID <= 609) {
          // ROSE doesn't have fpscw0 -> fpscw9, so map them all to fpscw
          return std::make_tuple(powerpc_regclass_fpscr, 0, 0, num_bits);
        }
        if(baseID == 610) {
          return std::make_tuple(powerpc_regclass_msr, 0, 0, num_bits);
        }
        if(baseID >= 611 && baseID <= 612) {
          // ivpr (611) and ivor8 (612) don't have ROSE representations
          return std::make_tuple(powerpc_regclass_spr, baseID, 0, 0);
        }
        if(baseID >= 613 && baseID <= 620) {
          auto const pos = baseID - 613;
          return std::make_tuple(powerpc_regclass_sr, 0, pos, num_bits);
        }
        if(baseID >= 621 && baseID <= 628) {
          auto const pos = baseID - 621;
          return std::make_tuple(powerpc_regclass_cr, 0, pos, num_bits);
        }
        if(baseID == 629) {
          return std::make_tuple(powerpc_regclass_cr, 0, 0, num_bits);
        }
        if(baseID >= 630 && baseID <= 631) {
          // or3 (630) and trap (631) don't have ROSE representations
          return std::make_tuple(powerpc_regclass_spr, baseID, 0, 0);
        }
        if(baseID >= 700 && baseID <= 731) {
          // cr0l to cr7s
          auto const pos = baseID - 700;
          return std::make_tuple(powerpc_regclass_cr, 0, pos, 1);
        }
        return std::make_tuple(powerpc_regclass_spr, baseID, 0, 0);
      }
    }
    convert_printf("Unknown ppc32 category '%d'\n", category);
    return std::make_tuple(powerpc_regclass_unknown, baseID, pos, num_bits);
  }
}

#endif
