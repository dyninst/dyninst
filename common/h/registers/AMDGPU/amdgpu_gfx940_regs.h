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

#ifndef DYNINST_AMDGPU_GFX940_REGS_H
#define DYNINST_AMDGPU_GFX940_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace amdgpu_gfx940 {

  /**
   * For interpreting constants:
   *  Lowest 16 bits (0x000000ff) is base register ID
   *  Next 16 bits (0x0000ff00) is the aliasing and subrange ID-
   *    used on x86/x86_64 to distinguish between things like EAX and AH
   *  Next 16 bits (0x00ff0000) are the register category, GPR/FPR/MMX/...
   *  Top 16 bits (0xff000000) are the architecture.
   *
   *  These values/layout are not guaranteed to remain the same as part of the
   *  public interface, and may change.
   **/

  // 0xff000000  0x00ff0000      0x0000ff00      0x000000ff
  // arch        reg cat:GPR     alias&subrange  reg ID
const signed int SGPR      = 0x00010000;
const signed int VGPR      = 0x00060000;

const signed int MISC      = 0x000A0000;
const signed int ACC_VGPR  = 0x000B0000;
const signed int HWR       = 0x000C0000;
const signed int TTMP_SGPR = 0x000D0000;
const signed int WAITCNT   = 0x000E0000;
const signed int PC        = 0x000F0000;
const signed int SYSREG    = 0x00100000;
const signed int TGT       = 0x00110000; // I have no idea what TGT is yet
const signed int ATTR      = 0x00120000;
const signed int PARAM     = 0x00130000; // LDS Parameter
const signed int INFO      = 0x00130000;  // Additional Info

// aliasing for flags
// if we found out that it is a flag, we no longer need to use the cat  0x00ff0000
// so we use that part to encode the low offset in the base register
//

const signed int BITS_1   = 0x00000100;
const signed int BITS_2   = 0x00000200;
const signed int BITS_3   = 0x00000300;
const signed int BITS_4   = 0x00000400;
const signed int BITS_6   = 0x00000500;
const signed int BITS_7   = 0x00000600;
const signed int BITS_8   = 0x00000700;
const signed int BITS_9   = 0x00000800;
const signed int BITS_15  = 0x00000900;
const signed int BITS_16  = 0x00000A00;
const signed int BITS_32  = 0x00000B00;
const signed int BITS_48  = 0x00000C00;
const signed int BITS_64  = 0x00000D00;
const signed int BITS_128 = 0x00000E00;
const signed int BITS_256 = 0x00000F00;
const signed int BITS_512 = 0x00001000;

  //          (                    name,  ID | alias   |      cat  |              arch)
  // Single source of truth for this arch's registers: (name, encoding).
  // Expanded to the constexpr constants AND all_regs[] -- no duplication.
  #define DYNINST_AMDGPU_GFX940_REG_LIST(X) \
      X(tid, 0 | BITS_32 |    SYSREG |Arch_amdgpu_gfx940) \
      X(invalid, 1 | BITS_32 |    SYSREG |Arch_amdgpu_gfx940) \
      X(pc_all, 0 | BITS_48 |        PC |Arch_amdgpu_gfx940) \
      X(hw_reg_mode, 1 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_status, 2 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_trapsts, 3 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_hw_id, 4 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_gpr_alloc, 5 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_lds_alloc, 6 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_ib_sts, 7 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_pc_lo, 8 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_pc_hi, 9 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_inst_dw0, 10 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_inst_dw1, 11 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_ib_dbg0, 12 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_ib_dbg1, 13 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_flush_ib, 14 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sh_mem_bases, 15 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_shader_tba_lo, 16 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_shader_tba_hi, 17 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_shader_tma_lo, 18 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_shader_tma_hi, 19 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_xcc_id, 20 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_perf_snapshot_data, 21 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_perf_snapshot_data1, 22 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_perf_snapshot_pc_lo, 23 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(hw_reg_sq_perf_snapshot_pc_hi, 24 | BITS_32 |       HWR |Arch_amdgpu_gfx940) \
      X(src_scc, 0 |  BITS_1 |      MISC |Arch_amdgpu_gfx940) \
      X(src_vccz, 1 |  BITS_1 |      MISC |Arch_amdgpu_gfx940) \
      X(vcc_lo, 2 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(vcc_hi, 3 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_execz, 4 |  BITS_1 |      MISC |Arch_amdgpu_gfx940) \
      X(exec_lo, 5 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(exec_hi, 6 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(flat_scratch_lo, 7 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(flat_scratch_hi, 8 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(m0, 9 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_literal, 10 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_pops_exiting_wave_id, 11 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_private_base, 12 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_private_limit, 13 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_shared_base, 14 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_shared_limit, 15 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(xnack_mask_lo, 16 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(xnack_mask_hi, 17 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(src_lds_direct, 18 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(dsmem, 19 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(gpumem, 20 | BITS_32 |      MISC |Arch_amdgpu_gfx940) \
      X(vmcnt, 0 | BITS_32 |   WAITCNT |Arch_amdgpu_gfx940) \
      X(expcnt, 1 | BITS_32 |   WAITCNT |Arch_amdgpu_gfx940) \
      X(lgkmcnt, 2 | BITS_32 |   WAITCNT |Arch_amdgpu_gfx940) \
      X(ttmp0, 0 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp1, 1 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp2, 2 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp3, 3 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp4, 4 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp5, 5 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp6, 6 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp7, 7 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp8, 8 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp9, 9 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp10, 10 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp11, 11 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp12, 12 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp13, 13 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp14, 14 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(ttmp15, 15 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940) \
      X(mrt0, 0 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt1, 1 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt2, 2 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt3, 3 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt4, 4 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt5, 5 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt6, 6 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrt7, 7 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(mrtz, 8 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(null, 9 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(pos0, 12 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(pos1, 13 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(pos2, 14 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(pos3, 15 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param0, 32 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param1, 33 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param2, 34 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param3, 35 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param4, 36 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param5, 37 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param6, 38 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param7, 39 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param8, 40 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param9, 41 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param10, 42 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param11, 43 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param12, 44 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param13, 45 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param14, 46 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param15, 47 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param16, 48 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param17, 49 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param18, 50 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param19, 51 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param20, 52 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param21, 53 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param22, 54 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param23, 55 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param24, 56 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param25, 57 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param26, 58 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param27, 59 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param28, 60 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param29, 61 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param30, 62 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(param31, 63 | BITS_32 |       TGT |Arch_amdgpu_gfx940) \
      X(attr0, 0 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr1, 1 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr2, 2 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr3, 3 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr4, 4 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr5, 5 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr6, 6 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr7, 7 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr8, 8 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr9, 9 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr10, 10 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr11, 11 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr12, 12 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr13, 13 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr14, 14 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr15, 15 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr16, 16 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr17, 17 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr18, 18 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr19, 19 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr20, 20 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr21, 21 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr22, 22 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr23, 23 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr24, 24 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr25, 25 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr26, 26 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr27, 27 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr28, 28 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr29, 29 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr30, 30 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr31, 31 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(attr32, 32 | BITS_32 |      ATTR |Arch_amdgpu_gfx940) \
      X(p10, 0 | BITS_32 |     PARAM |Arch_amdgpu_gfx940) \
      X(p20, 1 | BITS_32 |     PARAM |Arch_amdgpu_gfx940) \
      X(p0, 2 | BITS_32 |     PARAM |Arch_amdgpu_gfx940) \
      X(idxen, 0 |  BITS_1 |      INFO |Arch_amdgpu_gfx940) \
      X(offen, 1 |  BITS_1 |      INFO |Arch_amdgpu_gfx940) \
      X(off, 2 |  BITS_1 |      INFO |Arch_amdgpu_gfx940) \
      X(s0, 0 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s1, 1 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s2, 2 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s3, 3 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s4, 4 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s5, 5 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s6, 6 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s7, 7 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s8, 8 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s9, 9 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s10, 10 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s11, 11 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s12, 12 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s13, 13 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s14, 14 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s15, 15 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s16, 16 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s17, 17 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s18, 18 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s19, 19 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s20, 20 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s21, 21 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s22, 22 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s23, 23 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s24, 24 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s25, 25 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s26, 26 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s27, 27 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s28, 28 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s29, 29 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s30, 30 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s31, 31 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s32, 32 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s33, 33 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s34, 34 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s35, 35 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s36, 36 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s37, 37 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s38, 38 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s39, 39 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s40, 40 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s41, 41 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s42, 42 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s43, 43 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s44, 44 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s45, 45 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s46, 46 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s47, 47 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s48, 48 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s49, 49 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s50, 50 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s51, 51 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s52, 52 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s53, 53 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s54, 54 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s55, 55 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s56, 56 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s57, 57 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s58, 58 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s59, 59 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s60, 60 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s61, 61 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s62, 62 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s63, 63 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s64, 64 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s65, 65 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s66, 66 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s67, 67 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s68, 68 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s69, 69 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s70, 70 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s71, 71 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s72, 72 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s73, 73 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s74, 74 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s75, 75 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s76, 76 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s77, 77 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s78, 78 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s79, 79 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s80, 80 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s81, 81 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s82, 82 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s83, 83 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s84, 84 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s85, 85 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s86, 86 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s87, 87 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s88, 88 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s89, 89 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s90, 90 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s91, 91 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s92, 92 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s93, 93 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s94, 94 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s95, 95 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s96, 96 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s97, 97 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s98, 98 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s99, 99 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s100, 100 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(s101, 101 | BITS_32 |      SGPR |Arch_amdgpu_gfx940) \
      X(v0, 0 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v1, 1 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v2, 2 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v3, 3 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v4, 4 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v5, 5 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v6, 6 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v7, 7 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v8, 8 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v9, 9 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v10, 10 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v11, 11 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v12, 12 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v13, 13 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v14, 14 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v15, 15 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v16, 16 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v17, 17 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v18, 18 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v19, 19 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v20, 20 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v21, 21 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v22, 22 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v23, 23 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v24, 24 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v25, 25 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v26, 26 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v27, 27 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v28, 28 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v29, 29 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v30, 30 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v31, 31 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v32, 32 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v33, 33 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v34, 34 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v35, 35 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v36, 36 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v37, 37 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v38, 38 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v39, 39 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v40, 40 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v41, 41 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v42, 42 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v43, 43 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v44, 44 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v45, 45 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v46, 46 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v47, 47 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v48, 48 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v49, 49 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v50, 50 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v51, 51 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v52, 52 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v53, 53 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v54, 54 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v55, 55 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v56, 56 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v57, 57 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v58, 58 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v59, 59 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v60, 60 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v61, 61 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v62, 62 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v63, 63 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v64, 64 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v65, 65 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v66, 66 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v67, 67 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v68, 68 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v69, 69 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v70, 70 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v71, 71 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v72, 72 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v73, 73 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v74, 74 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v75, 75 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v76, 76 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v77, 77 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v78, 78 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v79, 79 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v80, 80 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v81, 81 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v82, 82 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v83, 83 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v84, 84 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v85, 85 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v86, 86 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v87, 87 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v88, 88 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v89, 89 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v90, 90 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v91, 91 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v92, 92 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v93, 93 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v94, 94 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v95, 95 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v96, 96 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v97, 97 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v98, 98 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v99, 99 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v100, 100 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v101, 101 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v102, 102 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v103, 103 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v104, 104 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v105, 105 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v106, 106 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v107, 107 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v108, 108 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v109, 109 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v110, 110 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v111, 111 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v112, 112 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v113, 113 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v114, 114 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v115, 115 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v116, 116 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v117, 117 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v118, 118 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v119, 119 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v120, 120 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v121, 121 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v122, 122 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v123, 123 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v124, 124 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v125, 125 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v126, 126 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v127, 127 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v128, 128 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v129, 129 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v130, 130 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v131, 131 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v132, 132 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v133, 133 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v134, 134 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v135, 135 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v136, 136 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v137, 137 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v138, 138 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v139, 139 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v140, 140 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v141, 141 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v142, 142 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v143, 143 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v144, 144 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v145, 145 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v146, 146 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v147, 147 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v148, 148 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v149, 149 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v150, 150 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v151, 151 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v152, 152 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v153, 153 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v154, 154 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v155, 155 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v156, 156 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v157, 157 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v158, 158 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v159, 159 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v160, 160 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v161, 161 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v162, 162 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v163, 163 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v164, 164 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v165, 165 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v166, 166 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v167, 167 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v168, 168 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v169, 169 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v170, 170 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v171, 171 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v172, 172 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v173, 173 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v174, 174 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v175, 175 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v176, 176 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v177, 177 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v178, 178 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v179, 179 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v180, 180 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v181, 181 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v182, 182 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v183, 183 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v184, 184 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v185, 185 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v186, 186 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v187, 187 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v188, 188 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v189, 189 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v190, 190 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v191, 191 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v192, 192 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v193, 193 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v194, 194 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v195, 195 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v196, 196 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v197, 197 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v198, 198 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v199, 199 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v200, 200 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v201, 201 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v202, 202 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v203, 203 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v204, 204 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v205, 205 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v206, 206 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v207, 207 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v208, 208 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v209, 209 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v210, 210 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v211, 211 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v212, 212 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v213, 213 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v214, 214 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v215, 215 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v216, 216 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v217, 217 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v218, 218 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v219, 219 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v220, 220 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v221, 221 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v222, 222 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v223, 223 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v224, 224 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v225, 225 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v226, 226 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v227, 227 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v228, 228 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v229, 229 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v230, 230 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v231, 231 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v232, 232 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v233, 233 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v234, 234 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v235, 235 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v236, 236 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v237, 237 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v238, 238 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v239, 239 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v240, 240 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v241, 241 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v242, 242 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v243, 243 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v244, 244 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v245, 245 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v246, 246 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v247, 247 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v248, 248 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v249, 249 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v250, 250 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v251, 251 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v252, 252 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v253, 253 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v254, 254 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(v255, 255 | BITS_32 |      VGPR |Arch_amdgpu_gfx940) \
      X(acc0, 0 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc1, 1 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc2, 2 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc3, 3 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc4, 4 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc5, 5 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc6, 6 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc7, 7 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc8, 8 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc9, 9 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc10, 10 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc11, 11 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc12, 12 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc13, 13 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc14, 14 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc15, 15 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc16, 16 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc17, 17 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc18, 18 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc19, 19 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc20, 20 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc21, 21 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc22, 22 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc23, 23 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc24, 24 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc25, 25 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc26, 26 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc27, 27 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc28, 28 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc29, 29 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc30, 30 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc31, 31 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc32, 32 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc33, 33 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc34, 34 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc35, 35 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc36, 36 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc37, 37 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc38, 38 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc39, 39 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc40, 40 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc41, 41 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc42, 42 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc43, 43 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc44, 44 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc45, 45 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc46, 46 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc47, 47 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc48, 48 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc49, 49 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc50, 50 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc51, 51 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc52, 52 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc53, 53 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc54, 54 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc55, 55 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc56, 56 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc57, 57 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc58, 58 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc59, 59 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc60, 60 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc61, 61 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc62, 62 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc63, 63 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc64, 64 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc65, 65 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc66, 66 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc67, 67 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc68, 68 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc69, 69 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc70, 70 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc71, 71 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc72, 72 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc73, 73 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc74, 74 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc75, 75 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc76, 76 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc77, 77 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc78, 78 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc79, 79 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc80, 80 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc81, 81 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc82, 82 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc83, 83 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc84, 84 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc85, 85 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc86, 86 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc87, 87 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc88, 88 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc89, 89 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc90, 90 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc91, 91 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc92, 92 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc93, 93 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc94, 94 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc95, 95 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc96, 96 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc97, 97 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc98, 98 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc99, 99 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc100, 100 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc101, 101 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc102, 102 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc103, 103 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc104, 104 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc105, 105 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc106, 106 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc107, 107 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc108, 108 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc109, 109 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc110, 110 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc111, 111 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc112, 112 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc113, 113 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc114, 114 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc115, 115 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc116, 116 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc117, 117 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc118, 118 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc119, 119 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc120, 120 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc121, 121 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc122, 122 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc123, 123 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc124, 124 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc125, 125 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc126, 126 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc127, 127 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc128, 128 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc129, 129 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc130, 130 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc131, 131 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc132, 132 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc133, 133 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc134, 134 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc135, 135 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc136, 136 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc137, 137 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc138, 138 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc139, 139 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc140, 140 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc141, 141 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc142, 142 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc143, 143 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc144, 144 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc145, 145 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc146, 146 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc147, 147 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc148, 148 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc149, 149 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc150, 150 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc151, 151 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc152, 152 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc153, 153 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc154, 154 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc155, 155 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc156, 156 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc157, 157 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc158, 158 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc159, 159 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc160, 160 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc161, 161 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc162, 162 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc163, 163 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc164, 164 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc165, 165 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc166, 166 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc167, 167 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc168, 168 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc169, 169 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc170, 170 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc171, 171 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc172, 172 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc173, 173 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc174, 174 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc175, 175 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc176, 176 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc177, 177 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc178, 178 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc179, 179 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc180, 180 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc181, 181 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc182, 182 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc183, 183 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc184, 184 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc185, 185 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc186, 186 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc187, 187 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc188, 188 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc189, 189 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc190, 190 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc191, 191 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc192, 192 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc193, 193 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc194, 194 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc195, 195 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc196, 196 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc197, 197 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc198, 198 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc199, 199 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc200, 200 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc201, 201 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc202, 202 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc203, 203 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc204, 204 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc205, 205 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc206, 206 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc207, 207 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc208, 208 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc209, 209 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc210, 210 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc211, 211 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc212, 212 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc213, 213 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc214, 214 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc215, 215 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc216, 216 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc217, 217 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc218, 218 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc219, 219 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc220, 220 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc221, 221 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc222, 222 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc223, 223 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc224, 224 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc225, 225 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc226, 226 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc227, 227 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc228, 228 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc229, 229 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc230, 230 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc231, 231 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc232, 232 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc233, 233 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc234, 234 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc235, 235 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc236, 236 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc237, 237 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc238, 238 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc239, 239 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc240, 240 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc241, 241 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc242, 242 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc243, 243 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc244, 244 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc245, 245 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc246, 246 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc247, 247 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc248, 248 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc249, 249 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc250, 250 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc251, 251 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc252, 252 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc253, 253 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc254, 254 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940) \
      X(acc255, 255 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940)

  #define DEF_ONE(n, v) DEF_REGISTER(n, v);
  DYNINST_AMDGPU_GFX940_REG_LIST(DEF_ONE)
  #undef DEF_ONE

  #define NAME_ONE(n, v) n,
  constexpr MachRegister all_regs[] = { DYNINST_AMDGPU_GFX940_REG_LIST(NAME_ONE) };
  #undef NAME_ONE


}}

#endif
