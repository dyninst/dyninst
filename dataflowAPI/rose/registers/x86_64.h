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

#ifndef DYNINST_COMMON_REGISTERS_ROSE_X86_64_H
#define DYNINST_COMMON_REGISTERS_ROSE_X86_64_H

#include "external/rose/rose-compat.h"
#include "registers/x86_64_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {
  namespace x86_64_rose {

    X86PositionInRegister pos(int32_t subrange) {
      switch(subrange) {
        case Dyninst::x86_64::XMMS:
        case Dyninst::x86_64::YMMS:
        case Dyninst::x86_64::ZMMS:
        case Dyninst::x86_64::FPDBL:
        case Dyninst::x86_64::BIT: return x86_regpos_all; break;
        case Dyninst::x86_64::FULL:
        case Dyninst::x86_64::KMSKS:
        case Dyninst::x86_64::MMS: return x86_regpos_qword; break;
        case Dyninst::x86_64::H_REG: return x86_regpos_high_byte; break;
        case Dyninst::x86_64::L_REG: return x86_regpos_low_byte; break;
        case Dyninst::x86_64::W_REG: return x86_regpos_word; break;
        case Dyninst::x86_64::D_REG: return x86_regpos_dword; break;
      }
      convert_printf("Unknown x86_64 subrange value '%d'\n", subrange);
      return x86_regpos_unknown;
    }

    X86GeneralPurposeRegister gpr(int32_t baseID) {
      switch(baseID) {
        case Dyninst::x86_64::BASEA: return x86_gpr_ax;
        case Dyninst::x86_64::BASEC: return x86_gpr_cx;
        case Dyninst::x86_64::BASED: return x86_gpr_dx;
        case Dyninst::x86_64::BASEB: return x86_gpr_bx;
        case Dyninst::x86_64::BASESP: return x86_gpr_sp;
        case Dyninst::x86_64::BASEBP: return x86_gpr_bp;
        case Dyninst::x86_64::BASESI: return x86_gpr_si;
        case Dyninst::x86_64::BASEDI: return x86_gpr_di;
        case Dyninst::x86_64::BASE8: return x86_gpr_r8;
        case Dyninst::x86_64::BASE9: return x86_gpr_r9;
        case Dyninst::x86_64::BASE10: return x86_gpr_r10;
        case Dyninst::x86_64::BASE11: return x86_gpr_r11;
        case Dyninst::x86_64::BASE12: return x86_gpr_r12;
        case Dyninst::x86_64::BASE13: return x86_gpr_r13;
        case Dyninst::x86_64::BASE14: return x86_gpr_r14;
        case Dyninst::x86_64::BASE15: return x86_gpr_r15;
      }
      convert_printf("Unknown x86_64 GPR '%d'\n", baseID);
      return static_cast<X86GeneralPurposeRegister>(-1);
    }

    X86SegmentRegister seg(int32_t baseID) {
      switch(baseID) {
        case Dyninst::x86_64::BASEDS: return x86_segreg_ds;
        case Dyninst::x86_64::BASEES: return x86_segreg_es;
        case Dyninst::x86_64::BASEFS: return x86_segreg_fs;
        case Dyninst::x86_64::BASEGS: return x86_segreg_gs;
        case Dyninst::x86_64::BASECS: return x86_segreg_cs;
        case Dyninst::x86_64::BASESS: return x86_segreg_ss;
      }
      convert_printf("Unknown x86_64 segment register '%d'\n", baseID);
      return x86_segreg_none;
    }

    X86Flag flag(int32_t baseID) {
      switch(baseID) {
        case Dyninst::x86_64::CF: return x86_flag_cf;
        case Dyninst::x86_64::FLAG1: return x86_flag_1;
        case Dyninst::x86_64::PF: return x86_flag_pf;
        case Dyninst::x86_64::FLAG3: return x86_flag_3;
        case Dyninst::x86_64::AF: return x86_flag_af;
        case Dyninst::x86_64::FLAG5: return x86_flag_5;
        case Dyninst::x86_64::ZF: return x86_flag_zf;
        case Dyninst::x86_64::SF: return x86_flag_sf;
        case Dyninst::x86_64::TF: return x86_flag_tf;
        case Dyninst::x86_64::IF: return x86_flag_if;
        case Dyninst::x86_64::DF: return x86_flag_df;
        case Dyninst::x86_64::OF: return x86_flag_of;
        case Dyninst::x86_64::FLAGC: return x86_flag_iopl0;
        case Dyninst::x86_64::FLAGD: return x86_flag_iopl1;
        case Dyninst::x86_64::NT: return x86_flag_nt;
        case Dyninst::x86_64::FLAGF: return x86_flag_15;
        case Dyninst::x86_64::VM: return x86_flag_vm;
        case Dyninst::x86_64::RF: return x86_flag_rf;
        case Dyninst::x86_64::AC: return x86_flag_ac;
        case Dyninst::x86_64::VIF: return x86_flag_vif;
        case Dyninst::x86_64::VIP: return x86_flag_vip;
        case Dyninst::x86_64::ID: return x86_flag_id;
      }
      convert_printf("Unknown x86_64 flag register '%d'\n", baseID);
      return static_cast<X86Flag>(-1);
    }
  }

  std::tuple<X86RegisterClass, int, X86PositionInRegister, int>
  x8664Rose(int32_t category, int32_t baseID, int32_t subrange, int32_t num_bits) {
    auto const p = x86_64_rose::pos(subrange);

    switch(category) {
      case Dyninst::x86_64::GPR: {
        auto const c = x86_regclass_gpr;
        auto const n = x86_64_rose::gpr(baseID);
        return std::make_tuple(c, static_cast<int>(n), p, num_bits);
      }

      case Dyninst::x86_64::SEG: {
        auto const c = x86_regclass_segment;
        auto const n = x86_64_rose::seg(baseID);
        auto const pos = x86_regpos_dword;  // ROSE docs: only value allowed
        return std::make_tuple(c, static_cast<int>(n), pos, num_bits);
      }

      case Dyninst::x86_64::FLAG: {
        auto const c = x86_regclass_flags;
        auto const n = x86_64_rose::flag(baseID);
        auto const pos = static_cast<X86PositionInRegister>(0);  // ROSE docs: only value allowed is 0
        return std::make_tuple(c, static_cast<int>(n), pos, num_bits);
      }

      case Dyninst::x86_64::KMASK:
        return std::make_tuple(x86_regclass_kmask, baseID, p, num_bits);

      case Dyninst::x86_64::ZMM:
        return std::make_tuple(x86_regclass_zmm, baseID, p, num_bits);

      case Dyninst::x86_64::YMM:
        return std::make_tuple(x86_regclass_ymm, baseID, p, num_bits);

      case Dyninst::x86_64::XMM:
        return std::make_tuple(x86_regclass_xmm, baseID, p, num_bits);

      case Dyninst::x86_64::MMX:
        return std::make_tuple(x86_regclass_mm, baseID, p, num_bits);

      case Dyninst::x86_64::X87: {
        auto const pos = x86_regpos_all;  // ROSE docs: only value allowed
        return std::make_tuple(x86_regclass_st, baseID, pos, num_bits);
      }

      case Dyninst::x86_64::CTL:
        return std::make_tuple(x86_regclass_cr, baseID, p, num_bits);

      case Dyninst::x86_64::DBG:
        return std::make_tuple(x86_regclass_dr, baseID, p, num_bits);

      case Dyninst::x86_64::MISC:
      case Dyninst::x86_64::TST:
        return std::make_tuple(x86_regclass_unknown, -1, p, 0);

    }
    convert_printf("Unknown x86_64 category '%d'\n", category);
    return std::make_tuple(x86_regclass_unknown, -1, p, 0);
  }

}

#endif
