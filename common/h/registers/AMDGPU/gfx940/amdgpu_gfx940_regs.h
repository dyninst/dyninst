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
  const signed int SGPR = 0x00010000;
  const signed int VGPR = 0x00060000;
  const signed int ACC_VGPR = 0x000B0000;

  const signed int HWR = 0x000C0000;
  const signed int TTMP_SGPR = 0x000D0000;
  const signed int FLAGS = 0x000E0000;
  const signed int PC = 0x000F0000;
  const signed int SYSREG = 0x00100000;
  const signed int TGT = 0x00110000; // I have no idea what TGT is yet
  const signed int ATTR = 0x00120000;
  const signed int PARAM = 0x00130000; // LDS Parameter
  const signed int INFO = 0x00130000;  // Additional Info

  // aliasing for flags
  // if we found out that it is a flag, we no longer need to use the cat  0x00ff0000
  // so we use that part to encode the low offset in the base register
  //

  const signed int BITS_1 = 0x00000100;
  const signed int BITS_2 = 0x00000200;
  const signed int BITS_3 = 0x00000300;
  const signed int BITS_4 = 0x00000400;
  const signed int BITS_6 = 0x00000500;
  const signed int BITS_7 = 0x00000600;
  const signed int BITS_8 = 0x00000700;
  const signed int BITS_9 = 0x00000800;
  const signed int BITS_15 = 0x00000900;
  const signed int BITS_16 = 0x00000A00;
  const signed int BITS_32 = 0x00000B00;
  const signed int BITS_48 = 0x00000C00;
  const signed int BITS_64 = 0x00000D00;
  const signed int BITS_128 = 0x00000E00;
  const signed int BITS_256 = 0x00000F00;
  const signed int BITS_512 = 0x00001000;

  DEF_REGISTER(tid, Arch_amdgpu_gfx940 | SYSREG | BITS_32 | 0, "amdgpu_gfx940");

  DEF_REGISTER(invalid, Arch_amdgpu_gfx940 | SYSREG | BITS_32 | 1, "amdgpu_gfx940");
  DEF_REGISTER(pc_all, Arch_amdgpu_gfx940 | PC | BITS_48 | 0, "amdgpu_gfx940");

  DEF_REGISTER(src_scc, Arch_amdgpu_gfx940 | HWR | BITS_32 | 0, "amdgpu_gfx940");

  DEF_REGISTER(src_vccz, Arch_amdgpu_gfx940 | HWR | BITS_1 | 1, "amdgpu_gfx940");
  DEF_REGISTER(vcc_lo, Arch_amdgpu_gfx940 | HWR | BITS_32 | 2, "amdgpu_gfx940");
  DEF_REGISTER(vcc_hi, Arch_amdgpu_gfx940 | HWR | BITS_32 | 3, "amdgpu_gfx940");
  DEF_REGISTER(vcc, Arch_amdgpu_gfx940 | HWR | BITS_64 | 2, "amdgpu_gfx940");

  DEF_REGISTER(src_execz, Arch_amdgpu_gfx940 | HWR | BITS_1 | 4, "amdgpu_gfx940");
  DEF_REGISTER(exec_lo, Arch_amdgpu_gfx940 | HWR | BITS_32 | 5, "amdgpu_gfx940");
  DEF_REGISTER(exec_hi, Arch_amdgpu_gfx940 | HWR | BITS_32 | 6, "amdgpu_gfx940");
  DEF_REGISTER(exec, Arch_amdgpu_gfx940 | HWR | BITS_64 | 5, "amdgpu_gfx940");

  DEF_REGISTER(flat_scratch_lo, Arch_amdgpu_gfx940 | HWR | BITS_64 | 7, "amdgpu_gfx940");
  DEF_REGISTER(flat_scratch_hi, Arch_amdgpu_gfx940 | HWR | BITS_32 | 8, "amdgpu_gfx940");
  DEF_REGISTER(flat_scratch_all, Arch_amdgpu_gfx940 | HWR | BITS_32 | 7, "amdgpu_gfx940");

  DEF_REGISTER(m0, Arch_amdgpu_gfx940 | HWR | BITS_32 | 10, "amdgpu_gfx940");

  DEF_REGISTER(src_literal, Arch_amdgpu_gfx940 | HWR | BITS_32 | 11, "amdgpu_gfx940"); // TODO
  DEF_REGISTER(src_pops_exiting_wave_id, Arch_amdgpu_gfx940 | HWR | BITS_32 | 12,
               "amdgpu_gfx940"); // TODO

  DEF_REGISTER(src_private_base, Arch_amdgpu_gfx940 | HWR | BITS_32 | 13, "amdgpu_gfx940");
  DEF_REGISTER(src_private_limit, Arch_amdgpu_gfx940 | HWR | BITS_32 | 14, "amdgpu_gfx940");
  DEF_REGISTER(src_shared_base, Arch_amdgpu_gfx940 | HWR | BITS_32 | 15, "amdgpu_gfx940");
  DEF_REGISTER(src_shared_limit, Arch_amdgpu_gfx940 | HWR | BITS_32 | 16, "amdgpu_gfx940");

  DEF_REGISTER(xnack_mask_lo, Arch_amdgpu_gfx940 | HWR | BITS_32 | 17, "amdgpu_gfx940");
  DEF_REGISTER(xnack_mask_hi, Arch_amdgpu_gfx940 | HWR | BITS_32 | 18, "amdgpu_gfx940");

  DEF_REGISTER(src_lds_direct, Arch_amdgpu_gfx940 | HWR | BITS_32 | 19, "amdgpu_gfx940");
  DEF_REGISTER(vmcnt, Arch_amdgpu_gfx940 | HWR | BITS_32 | 20, "amdgpu_gfx940");
  DEF_REGISTER(expcnt, Arch_amdgpu_gfx940 | HWR | BITS_32 | 21, "amdgpu_gfx940");
  DEF_REGISTER(lgkmcnt, Arch_amdgpu_gfx940 | HWR | BITS_32 | 22, "amdgpu_gfx940");
  DEF_REGISTER(dsmem, Arch_amdgpu_gfx940 | HWR | BITS_32 | 23, "amdgpu_gfx940");

  DEF_REGISTER(ttmp0, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 0, "amdgpu_gfx940");
  DEF_REGISTER(ttmp1, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 1, "amdgpu_gfx940");
  DEF_REGISTER(ttmp2, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 2, "amdgpu_gfx940");
  DEF_REGISTER(ttmp3, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 3, "amdgpu_gfx940");
  DEF_REGISTER(ttmp4, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 4, "amdgpu_gfx940");
  DEF_REGISTER(ttmp5, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 5, "amdgpu_gfx940");
  DEF_REGISTER(ttmp6, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 6, "amdgpu_gfx940");
  DEF_REGISTER(ttmp7, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 7, "amdgpu_gfx940");
  DEF_REGISTER(ttmp8, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 8, "amdgpu_gfx940");
  DEF_REGISTER(ttmp9, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 9, "amdgpu_gfx940");
  DEF_REGISTER(ttmp10, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 10, "amdgpu_gfx940");
  DEF_REGISTER(ttmp11, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 11, "amdgpu_gfx940");
  DEF_REGISTER(ttmp12, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 12, "amdgpu_gfx940");
  DEF_REGISTER(ttmp13, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 13, "amdgpu_gfx940");
  DEF_REGISTER(ttmp14, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 14, "amdgpu_gfx940");
  DEF_REGISTER(ttmp15, Arch_amdgpu_gfx940 | TTMP_SGPR | BITS_32 | 15, "amdgpu_gfx940");

  DEF_REGISTER(mrt0, Arch_amdgpu_gfx940 | TGT | BITS_32 | 0, "amdgpu_gfx940");
  DEF_REGISTER(mrt1, Arch_amdgpu_gfx940 | TGT | BITS_32 | 1, "amdgpu_gfx940");
  DEF_REGISTER(mrt2, Arch_amdgpu_gfx940 | TGT | BITS_32 | 2, "amdgpu_gfx940");
  DEF_REGISTER(mrt3, Arch_amdgpu_gfx940 | TGT | BITS_32 | 3, "amdgpu_gfx940");
  DEF_REGISTER(mrt4, Arch_amdgpu_gfx940 | TGT | BITS_32 | 4, "amdgpu_gfx940");
  DEF_REGISTER(mrt5, Arch_amdgpu_gfx940 | TGT | BITS_32 | 5, "amdgpu_gfx940");
  DEF_REGISTER(mrt6, Arch_amdgpu_gfx940 | TGT | BITS_32 | 6, "amdgpu_gfx940");
  DEF_REGISTER(mrt7, Arch_amdgpu_gfx940 | TGT | BITS_32 | 7, "amdgpu_gfx940");
  DEF_REGISTER(mrtz, Arch_amdgpu_gfx940 | TGT | BITS_32 | 8, "amdgpu_gfx940");
  DEF_REGISTER(null, Arch_amdgpu_gfx940 | TGT | BITS_32 | 9, "amdgpu_gfx940");
  DEF_REGISTER(pos0, Arch_amdgpu_gfx940 | TGT | BITS_32 | 12, "amdgpu_gfx940");
  DEF_REGISTER(pos1, Arch_amdgpu_gfx940 | TGT | BITS_32 | 13, "amdgpu_gfx940");
  DEF_REGISTER(pos2, Arch_amdgpu_gfx940 | TGT | BITS_32 | 14, "amdgpu_gfx940");
  DEF_REGISTER(pos3, Arch_amdgpu_gfx940 | TGT | BITS_32 | 15, "amdgpu_gfx940");
  DEF_REGISTER(param0, Arch_amdgpu_gfx940 | TGT | BITS_32 | 32, "amdgpu_gfx940");
  DEF_REGISTER(param1, Arch_amdgpu_gfx940 | TGT | BITS_32 | 33, "amdgpu_gfx940");
  DEF_REGISTER(param2, Arch_amdgpu_gfx940 | TGT | BITS_32 | 34, "amdgpu_gfx940");
  DEF_REGISTER(param3, Arch_amdgpu_gfx940 | TGT | BITS_32 | 35, "amdgpu_gfx940");
  DEF_REGISTER(param4, Arch_amdgpu_gfx940 | TGT | BITS_32 | 36, "amdgpu_gfx940");
  DEF_REGISTER(param5, Arch_amdgpu_gfx940 | TGT | BITS_32 | 37, "amdgpu_gfx940");
  DEF_REGISTER(param6, Arch_amdgpu_gfx940 | TGT | BITS_32 | 38, "amdgpu_gfx940");
  DEF_REGISTER(param7, Arch_amdgpu_gfx940 | TGT | BITS_32 | 39, "amdgpu_gfx940");
  DEF_REGISTER(param8, Arch_amdgpu_gfx940 | TGT | BITS_32 | 40, "amdgpu_gfx940");
  DEF_REGISTER(param9, Arch_amdgpu_gfx940 | TGT | BITS_32 | 41, "amdgpu_gfx940");
  DEF_REGISTER(param10, Arch_amdgpu_gfx940 | TGT | BITS_32 | 42, "amdgpu_gfx940");
  DEF_REGISTER(param11, Arch_amdgpu_gfx940 | TGT | BITS_32 | 43, "amdgpu_gfx940");
  DEF_REGISTER(param12, Arch_amdgpu_gfx940 | TGT | BITS_32 | 44, "amdgpu_gfx940");
  DEF_REGISTER(param13, Arch_amdgpu_gfx940 | TGT | BITS_32 | 45, "amdgpu_gfx940");
  DEF_REGISTER(param14, Arch_amdgpu_gfx940 | TGT | BITS_32 | 46, "amdgpu_gfx940");
  DEF_REGISTER(param15, Arch_amdgpu_gfx940 | TGT | BITS_32 | 47, "amdgpu_gfx940");
  DEF_REGISTER(param16, Arch_amdgpu_gfx940 | TGT | BITS_32 | 48, "amdgpu_gfx940");
  DEF_REGISTER(param17, Arch_amdgpu_gfx940 | TGT | BITS_32 | 49, "amdgpu_gfx940");
  DEF_REGISTER(param18, Arch_amdgpu_gfx940 | TGT | BITS_32 | 50, "amdgpu_gfx940");
  DEF_REGISTER(param19, Arch_amdgpu_gfx940 | TGT | BITS_32 | 51, "amdgpu_gfx940");
  DEF_REGISTER(param20, Arch_amdgpu_gfx940 | TGT | BITS_32 | 52, "amdgpu_gfx940");
  DEF_REGISTER(param21, Arch_amdgpu_gfx940 | TGT | BITS_32 | 53, "amdgpu_gfx940");
  DEF_REGISTER(param22, Arch_amdgpu_gfx940 | TGT | BITS_32 | 54, "amdgpu_gfx940");
  DEF_REGISTER(param23, Arch_amdgpu_gfx940 | TGT | BITS_32 | 55, "amdgpu_gfx940");
  DEF_REGISTER(param24, Arch_amdgpu_gfx940 | TGT | BITS_32 | 56, "amdgpu_gfx940");
  DEF_REGISTER(param25, Arch_amdgpu_gfx940 | TGT | BITS_32 | 57, "amdgpu_gfx940");
  DEF_REGISTER(param26, Arch_amdgpu_gfx940 | TGT | BITS_32 | 58, "amdgpu_gfx940");
  DEF_REGISTER(param27, Arch_amdgpu_gfx940 | TGT | BITS_32 | 59, "amdgpu_gfx940");
  DEF_REGISTER(param28, Arch_amdgpu_gfx940 | TGT | BITS_32 | 60, "amdgpu_gfx940");
  DEF_REGISTER(param29, Arch_amdgpu_gfx940 | TGT | BITS_32 | 61, "amdgpu_gfx940");
  DEF_REGISTER(param30, Arch_amdgpu_gfx940 | TGT | BITS_32 | 62, "amdgpu_gfx940");
  DEF_REGISTER(param31, Arch_amdgpu_gfx940 | TGT | BITS_32 | 63, "amdgpu_gfx940");

  DEF_REGISTER(attr0, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 0, "amdgpu_gfx940");
  DEF_REGISTER(attr1, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 1, "amdgpu_gfx940");
  DEF_REGISTER(attr2, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 2, "amdgpu_gfx940");
  DEF_REGISTER(attr3, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 3, "amdgpu_gfx940");
  DEF_REGISTER(attr4, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 4, "amdgpu_gfx940");
  DEF_REGISTER(attr5, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 5, "amdgpu_gfx940");
  DEF_REGISTER(attr6, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 6, "amdgpu_gfx940");
  DEF_REGISTER(attr7, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 7, "amdgpu_gfx940");
  DEF_REGISTER(attr8, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 8, "amdgpu_gfx940");
  DEF_REGISTER(attr9, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 9, "amdgpu_gfx940");
  DEF_REGISTER(attr10, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 10, "amdgpu_gfx940");
  DEF_REGISTER(attr11, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 11, "amdgpu_gfx940");
  DEF_REGISTER(attr12, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 12, "amdgpu_gfx940");
  DEF_REGISTER(attr13, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 13, "amdgpu_gfx940");
  DEF_REGISTER(attr14, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 14, "amdgpu_gfx940");
  DEF_REGISTER(attr15, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 15, "amdgpu_gfx940");
  DEF_REGISTER(attr16, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 16, "amdgpu_gfx940");
  DEF_REGISTER(attr17, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 17, "amdgpu_gfx940");
  DEF_REGISTER(attr18, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 18, "amdgpu_gfx940");
  DEF_REGISTER(attr19, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 19, "amdgpu_gfx940");
  DEF_REGISTER(attr20, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 20, "amdgpu_gfx940");
  DEF_REGISTER(attr21, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 21, "amdgpu_gfx940");
  DEF_REGISTER(attr22, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 22, "amdgpu_gfx940");
  DEF_REGISTER(attr23, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 23, "amdgpu_gfx940");
  DEF_REGISTER(attr24, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 24, "amdgpu_gfx940");
  DEF_REGISTER(attr25, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 25, "amdgpu_gfx940");
  DEF_REGISTER(attr26, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 26, "amdgpu_gfx940");
  DEF_REGISTER(attr27, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 27, "amdgpu_gfx940");
  DEF_REGISTER(attr28, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 28, "amdgpu_gfx940");
  DEF_REGISTER(attr29, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 29, "amdgpu_gfx940");
  DEF_REGISTER(attr30, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 30, "amdgpu_gfx940");
  DEF_REGISTER(attr31, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 31, "amdgpu_gfx940");
  DEF_REGISTER(attr32, Arch_amdgpu_gfx940 | ATTR | BITS_32 | 32, "amdgpu_gfx940");

  DEF_REGISTER(p10, Arch_amdgpu_gfx940 | PARAM | BITS_32 | 0, "amdgpu_gfx940");
  DEF_REGISTER(p20, Arch_amdgpu_gfx940 | PARAM | BITS_32 | 1, "amdgpu_gfx940");
  DEF_REGISTER(p0, Arch_amdgpu_gfx940 | PARAM | BITS_32 | 2, "amdgpu_gfx940");

  DEF_REGISTER(idxen, Arch_amdgpu_gfx940 | INFO | BITS_1 | 0, "amdgpu_gfx940");
  DEF_REGISTER(offen, Arch_amdgpu_gfx940 | INFO | BITS_1 | 1, "amdgpu_gfx940");
  DEF_REGISTER(off, Arch_amdgpu_gfx940 | INFO | BITS_1 | 2, "amdgpu_gfx940");

#include "registers/AMDGPU/gfx940/amdgpu_gfx940_sys_regs.h"
}}

#endif
