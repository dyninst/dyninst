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

#ifndef DYNINST_AMDGPU_GFX1250_REGS_H
#define DYNINST_AMDGPU_GFX1250_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace amdgpu_gfx1250 {

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
const signed int HWR       = 0x000C0000;
const signed int TTMP_SGPR = 0x000D0000;
const signed int PC        = 0x000F0000;
const signed int SYSREG    = 0x00100000;
const signed int TGT       = 0x00110000;

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
  DEF_REGISTER(                     tid,   0 | BITS_32 |    SYSREG |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                 invalid,   1 | BITS_32 |    SYSREG |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  pc_all,   0 | BITS_48 |        PC |Arch_amdgpu_gfx1250);
  // Index comes from OPR_HWREG in ISA XML
  DEF_REGISTER(                hw_reg_wave_mode,   1 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(              hw_reg_wave_status,   2 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(          hw_reg_wave_state_priv,   4 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(           hw_reg_wave_gpr_alloc,   5 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(           hw_reg_wave_lds_alloc,   6 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(       hw_reg_perf_snapshot_data,  10 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_perf_snapshot_pc_lo,  11 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_perf_snapshot_pc_hi,  12 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_perf_snapshot_data1,  15 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_perf_snapshot_data2,  16 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_wave_excp_flag_priv,  17 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(      hw_reg_wave_excp_flag_user,  18 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(           hw_reg_wave_trap_ctrl,  19 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(     hw_reg_wave_scratch_base_lo,  20 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(     hw_reg_wave_scratch_base_hi,  21 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(              hw_reg_wave_hw_id1,  23 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(              hw_reg_wave_hw_id2,  24 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(    hw_reg_wave_xnack_state_priv,  33 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(          hw_reg_wave_xnack_mask,  34 | BITS_32 |       HWR |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                 src_scc,   0 |  BITS_1 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                src_vccz,   1 |  BITS_1 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  vcc_lo,   2 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  vcc_hi,   3 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);

  DEF_REGISTER(               src_execz,   4 |  BITS_1 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                 exec_lo,   5 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                 exec_hi,   6 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                      m0,   9 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);

  DEF_REGISTER(             src_literal,  10 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);

  DEF_REGISTER(         src_shared_base,  14 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(        src_shared_limit,  15 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                   dsmem,  19 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  gpumem,  20 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(src_flat_scratch_base_lo,  21 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(src_flat_scratch_base_hi,  22 | BITS_32 |      MISC |Arch_amdgpu_gfx1250);
  DEF_REGISTER(           src_literal64,  23 | BITS_64 |      MISC |Arch_amdgpu_gfx1250);


  DEF_REGISTER(                   ttmp0,   0 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp1,   1 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp2,   2 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp3,   3 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp4,   4 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp5,   5 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp6,   6 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp7,   7 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp8,   8 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                   ttmp9,   9 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp10,  10 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp11,  11 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp12,  12 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp13,  13 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp14,  14 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                  ttmp15,  15 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                    null,   9 | BITS_32 |       TGT |Arch_amdgpu_gfx1250);

  DEF_REGISTER(                      s0,   0 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s1,   1 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s2,   2 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s3,   3 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s4,   4 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s5,   5 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s6,   6 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s7,   7 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s8,   8 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      s9,   9 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s10,  10 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s11,  11 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s12,  12 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s13,  13 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s14,  14 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s15,  15 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s16,  16 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s17,  17 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s18,  18 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s19,  19 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s20,  20 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s21,  21 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s22,  22 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s23,  23 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s24,  24 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s25,  25 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s26,  26 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s27,  27 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s28,  28 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s29,  29 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s30,  30 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s31,  31 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s32,  32 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s33,  33 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s34,  34 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s35,  35 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s36,  36 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s37,  37 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s38,  38 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s39,  39 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s40,  40 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s41,  41 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s42,  42 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s43,  43 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s44,  44 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s45,  45 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s46,  46 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s47,  47 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s48,  48 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s49,  49 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s50,  50 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s51,  51 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s52,  52 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s53,  53 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s54,  54 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s55,  55 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s56,  56 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s57,  57 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s58,  58 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s59,  59 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s60,  60 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s61,  61 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s62,  62 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s63,  63 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s64,  64 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s65,  65 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s66,  66 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s67,  67 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s68,  68 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s69,  69 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s70,  70 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s71,  71 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s72,  72 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s73,  73 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s74,  74 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s75,  75 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s76,  76 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s77,  77 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s78,  78 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s79,  79 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s80,  80 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s81,  81 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s82,  82 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s83,  83 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s84,  84 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s85,  85 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s86,  86 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s87,  87 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s88,  88 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s89,  89 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s90,  90 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s91,  91 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s92,  92 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s93,  93 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s94,  94 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s95,  95 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s96,  96 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s97,  97 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s98,  98 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     s99,  99 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s100, 100 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s101, 101 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s102, 102 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s103, 103 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s104, 104 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    s105, 105 | BITS_32 |      SGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v0,   0 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v1,   1 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v2,   2 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v3,   3 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v4,   4 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v5,   5 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v6,   6 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v7,   7 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v8,   8 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                      v9,   9 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v10,  10 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v11,  11 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v12,  12 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v13,  13 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v14,  14 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v15,  15 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v16,  16 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v17,  17 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v18,  18 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v19,  19 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v20,  20 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v21,  21 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v22,  22 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v23,  23 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v24,  24 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v25,  25 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v26,  26 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v27,  27 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v28,  28 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v29,  29 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v30,  30 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v31,  31 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v32,  32 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v33,  33 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v34,  34 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v35,  35 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v36,  36 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v37,  37 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v38,  38 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v39,  39 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v40,  40 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v41,  41 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v42,  42 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v43,  43 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v44,  44 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v45,  45 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v46,  46 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v47,  47 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v48,  48 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v49,  49 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v50,  50 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v51,  51 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v52,  52 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v53,  53 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v54,  54 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v55,  55 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v56,  56 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v57,  57 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v58,  58 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v59,  59 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v60,  60 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v61,  61 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v62,  62 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v63,  63 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v64,  64 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v65,  65 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v66,  66 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v67,  67 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v68,  68 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v69,  69 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v70,  70 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v71,  71 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v72,  72 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v73,  73 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v74,  74 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v75,  75 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v76,  76 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v77,  77 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v78,  78 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v79,  79 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v80,  80 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v81,  81 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v82,  82 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v83,  83 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v84,  84 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v85,  85 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v86,  86 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v87,  87 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v88,  88 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v89,  89 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v90,  90 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v91,  91 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v92,  92 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v93,  93 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v94,  94 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v95,  95 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v96,  96 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v97,  97 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v98,  98 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                     v99,  99 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v100, 100 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v101, 101 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v102, 102 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v103, 103 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v104, 104 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v105, 105 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v106, 106 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v107, 107 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v108, 108 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v109, 109 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v110, 110 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v111, 111 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v112, 112 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v113, 113 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v114, 114 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v115, 115 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v116, 116 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v117, 117 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v118, 118 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v119, 119 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v120, 120 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v121, 121 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v122, 122 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v123, 123 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v124, 124 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v125, 125 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v126, 126 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v127, 127 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v128, 128 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v129, 129 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v130, 130 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v131, 131 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v132, 132 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v133, 133 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v134, 134 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v135, 135 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v136, 136 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v137, 137 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v138, 138 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v139, 139 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v140, 140 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v141, 141 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v142, 142 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v143, 143 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v144, 144 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v145, 145 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v146, 146 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v147, 147 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v148, 148 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v149, 149 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v150, 150 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v151, 151 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v152, 152 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v153, 153 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v154, 154 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v155, 155 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v156, 156 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v157, 157 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v158, 158 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v159, 159 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v160, 160 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v161, 161 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v162, 162 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v163, 163 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v164, 164 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v165, 165 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v166, 166 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v167, 167 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v168, 168 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v169, 169 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v170, 170 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v171, 171 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v172, 172 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v173, 173 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v174, 174 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v175, 175 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v176, 176 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v177, 177 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v178, 178 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v179, 179 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v180, 180 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v181, 181 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v182, 182 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v183, 183 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v184, 184 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v185, 185 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v186, 186 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v187, 187 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v188, 188 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v189, 189 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v190, 190 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v191, 191 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v192, 192 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v193, 193 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v194, 194 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v195, 195 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v196, 196 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v197, 197 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v198, 198 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v199, 199 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v200, 200 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v201, 201 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v202, 202 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v203, 203 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v204, 204 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v205, 205 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v206, 206 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v207, 207 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v208, 208 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v209, 209 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v210, 210 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v211, 211 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v212, 212 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v213, 213 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v214, 214 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v215, 215 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v216, 216 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v217, 217 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v218, 218 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v219, 219 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v220, 220 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v221, 221 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v222, 222 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v223, 223 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v224, 224 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v225, 225 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v226, 226 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v227, 227 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v228, 228 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v229, 229 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v230, 230 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v231, 231 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v232, 232 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v233, 233 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v234, 234 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v235, 235 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v236, 236 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v237, 237 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v238, 238 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v239, 239 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v240, 240 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v241, 241 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v242, 242 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v243, 243 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v244, 244 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v245, 245 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v246, 246 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v247, 247 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v248, 248 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v249, 249 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v250, 250 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v251, 251 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v252, 252 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v253, 253 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v254, 254 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);
  DEF_REGISTER(                    v255, 255 | BITS_32 |      VGPR |Arch_amdgpu_gfx1250);

}}

#endif
