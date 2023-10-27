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

#ifndef DYNINST_AMDGPU_GFX908_REGS_H
#define DYNINST_AMDGPU_GFX908_REGS_H

//clang-format: off

#include "Architecture.h"
#include "registers/reg_def.h"

namespace Dyninst { namespace amdgpu_gfx908 {

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
  const signed int SGPR     = 0x00010000;
  const signed int VGPR     = 0x00060000;
  const signed int ACC_VGPR = 0x000B0000;

  const signed int HWR       = 0x000C0000;
  const signed int TTMP_SGPR = 0x000D0000;
  const signed int FLAGS     = 0x000E0000;
  const signed int PC        = 0x000F0000;
  const signed int SYSREG    = 0x00100000;
  const signed int TGT       = 0x00110000; // I have no idea what TGT is yet
  const signed int ATTR      = 0x00120000;
  const signed int PARAM     = 0x00130000; // LDS Parameter
  const signed int INFO      = 0x00140000;  // Addition Info

  // aliasing for flags
  // if we found out that it is a flag, we no longer need to use the cat  0x00ff0000
  // so we use that part to encode the low offset in the base register
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

  DEF_REGISTER(                     tid, Arch_amdgpu_gfx908 |    SYSREG | BITS_32 |   0, "amdgpu_gfx908");

  DEF_REGISTER(                 invalid, Arch_amdgpu_gfx908 |    SYSREG | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                  pc_all, Arch_amdgpu_gfx908 |        PC | BITS_48 |   0, "amdgpu_gfx908");

  DEF_REGISTER(                 src_scc, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   0, "amdgpu_gfx908");

  DEF_REGISTER(                src_vccz, Arch_amdgpu_gfx908 |       HWR |  BITS_1 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                  vcc_lo, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                  vcc_hi, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                     vcc, Arch_amdgpu_gfx908 |       HWR | BITS_64 |   2, "amdgpu_gfx908");

  DEF_REGISTER(               src_execz, Arch_amdgpu_gfx908 |       HWR |  BITS_1 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                 exec_lo, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                 exec_hi, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                    exec, Arch_amdgpu_gfx908 |       HWR | BITS_64 |   5, "amdgpu_gfx908");

  DEF_REGISTER(         flat_scratch_lo, Arch_amdgpu_gfx908 |       HWR | BITS_64 |   7, "amdgpu_gfx908");
  DEF_REGISTER(         flat_scratch_hi, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(        flat_scratch_all, Arch_amdgpu_gfx908 |       HWR | BITS_32 |   7, "amdgpu_gfx908");

  DEF_REGISTER(                      m0, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  10, "amdgpu_gfx908");

  DEF_REGISTER(             src_literal, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  11, "amdgpu_gfx908");//TODO
  DEF_REGISTER(src_pops_exiting_wave_id, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  12, "amdgpu_gfx908");//TODO

  DEF_REGISTER(        src_private_base, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(       src_private_limit, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(         src_shared_base, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(        src_shared_limit, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  16, "amdgpu_gfx908");

  DEF_REGISTER(           xnack_mask_lo, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  17, "amdgpu_gfx908");
  DEF_REGISTER(           xnack_mask_hi, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  18, "amdgpu_gfx908");

  DEF_REGISTER(          src_lds_direct, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  19, "amdgpu_gfx908");
  DEF_REGISTER(                   vmcnt, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  20, "amdgpu_gfx908");
  DEF_REGISTER(                  expcnt, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  21, "amdgpu_gfx908");
  DEF_REGISTER(                 lgkmcnt, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  22, "amdgpu_gfx908");
  DEF_REGISTER(                   dsmem, Arch_amdgpu_gfx908 |       HWR | BITS_32 |  23, "amdgpu_gfx908");

  DEF_REGISTER(                   ttmp0, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp1, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp2, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp3, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp4, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp5, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp6, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp7, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp8, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                   ttmp9, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp10, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  10, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp11, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  11, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp12, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp13, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp14, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                  ttmp15, Arch_amdgpu_gfx908 | TTMP_SGPR | BITS_32 |  15, "amdgpu_gfx908");

  DEF_REGISTER(                    mrt0, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt1, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt2, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt3, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt4, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt5, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt6, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                    mrt7, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                    mrtz, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                    null, Arch_amdgpu_gfx908 |       TGT | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                    pos0, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                    pos1, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                    pos2, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                    pos3, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(                  param0, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  32, "amdgpu_gfx908");
  DEF_REGISTER(                  param1, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  33, "amdgpu_gfx908");
  DEF_REGISTER(                  param2, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  34, "amdgpu_gfx908");
  DEF_REGISTER(                  param3, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  35, "amdgpu_gfx908");
  DEF_REGISTER(                  param4, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  36, "amdgpu_gfx908");
  DEF_REGISTER(                  param5, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  37, "amdgpu_gfx908");
  DEF_REGISTER(                  param6, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  38, "amdgpu_gfx908");
  DEF_REGISTER(                  param7, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  39, "amdgpu_gfx908");
  DEF_REGISTER(                  param8, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  40, "amdgpu_gfx908");
  DEF_REGISTER(                  param9, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  41, "amdgpu_gfx908");
  DEF_REGISTER(                 param10, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  42, "amdgpu_gfx908");
  DEF_REGISTER(                 param11, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  43, "amdgpu_gfx908");
  DEF_REGISTER(                 param12, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  44, "amdgpu_gfx908");
  DEF_REGISTER(                 param13, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  45, "amdgpu_gfx908");
  DEF_REGISTER(                 param14, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  46, "amdgpu_gfx908");
  DEF_REGISTER(                 param15, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  47, "amdgpu_gfx908");
  DEF_REGISTER(                 param16, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  48, "amdgpu_gfx908");
  DEF_REGISTER(                 param17, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  49, "amdgpu_gfx908");
  DEF_REGISTER(                 param18, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  50, "amdgpu_gfx908");
  DEF_REGISTER(                 param19, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  51, "amdgpu_gfx908");
  DEF_REGISTER(                 param20, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  52, "amdgpu_gfx908");
  DEF_REGISTER(                 param21, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  53, "amdgpu_gfx908");
  DEF_REGISTER(                 param22, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  54, "amdgpu_gfx908");
  DEF_REGISTER(                 param23, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  55, "amdgpu_gfx908");
  DEF_REGISTER(                 param24, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  56, "amdgpu_gfx908");
  DEF_REGISTER(                 param25, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  57, "amdgpu_gfx908");
  DEF_REGISTER(                 param26, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  58, "amdgpu_gfx908");
  DEF_REGISTER(                 param27, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  59, "amdgpu_gfx908");
  DEF_REGISTER(                 param28, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  60, "amdgpu_gfx908");
  DEF_REGISTER(                 param29, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  61, "amdgpu_gfx908");
  DEF_REGISTER(                 param30, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  62, "amdgpu_gfx908");
  DEF_REGISTER(                 param31, Arch_amdgpu_gfx908 |       TGT | BITS_32 |  63, "amdgpu_gfx908");

  DEF_REGISTER(                   attr0, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                   attr1, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                   attr2, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                   attr3, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                   attr4, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                   attr5, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                   attr6, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                   attr7, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                   attr8, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                   attr9, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                  attr10, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  10, "amdgpu_gfx908");
  DEF_REGISTER(                  attr11, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  11, "amdgpu_gfx908");
  DEF_REGISTER(                  attr12, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                  attr13, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                  attr14, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                  attr15, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(                  attr16, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  16, "amdgpu_gfx908");
  DEF_REGISTER(                  attr17, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  17, "amdgpu_gfx908");
  DEF_REGISTER(                  attr18, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  18, "amdgpu_gfx908");
  DEF_REGISTER(                  attr19, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  19, "amdgpu_gfx908");
  DEF_REGISTER(                  attr20, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  20, "amdgpu_gfx908");
  DEF_REGISTER(                  attr21, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  21, "amdgpu_gfx908");
  DEF_REGISTER(                  attr22, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  22, "amdgpu_gfx908");
  DEF_REGISTER(                  attr23, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  23, "amdgpu_gfx908");
  DEF_REGISTER(                  attr24, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  24, "amdgpu_gfx908");
  DEF_REGISTER(                  attr25, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  25, "amdgpu_gfx908");
  DEF_REGISTER(                  attr26, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  26, "amdgpu_gfx908");
  DEF_REGISTER(                  attr27, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  27, "amdgpu_gfx908");
  DEF_REGISTER(                  attr28, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  28, "amdgpu_gfx908");
  DEF_REGISTER(                  attr29, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  29, "amdgpu_gfx908");
  DEF_REGISTER(                  attr30, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  30, "amdgpu_gfx908");
  DEF_REGISTER(                  attr31, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  31, "amdgpu_gfx908");
  DEF_REGISTER(                  attr32, Arch_amdgpu_gfx908 |      ATTR | BITS_32 |  32, "amdgpu_gfx908");

  DEF_REGISTER(                     p10, Arch_amdgpu_gfx908 |     PARAM | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                     p20, Arch_amdgpu_gfx908 |     PARAM | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                      p0, Arch_amdgpu_gfx908 |     PARAM | BITS_32 |   2, "amdgpu_gfx908");

  DEF_REGISTER(                   idxen, Arch_amdgpu_gfx908 |      INFO |  BITS_1 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                   offen, Arch_amdgpu_gfx908 |      INFO |  BITS_1 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                     off, Arch_amdgpu_gfx908 |      INFO |  BITS_1 |   2, "amdgpu_gfx908");

  DEF_REGISTER(                      s0, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                      s1, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                      s2, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                      s3, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                      s4, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                      s5, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                      s6, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                      s7, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                      s8, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                      s9, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                     s10, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  10, "amdgpu_gfx908");
  DEF_REGISTER(                     s11, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  11, "amdgpu_gfx908");
  DEF_REGISTER(                     s12, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                     s13, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                     s14, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                     s15, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(                     s16, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  16, "amdgpu_gfx908");
  DEF_REGISTER(                     s17, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  17, "amdgpu_gfx908");
  DEF_REGISTER(                     s18, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  18, "amdgpu_gfx908");
  DEF_REGISTER(                     s19, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  19, "amdgpu_gfx908");
  DEF_REGISTER(                     s20, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  20, "amdgpu_gfx908");
  DEF_REGISTER(                     s21, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  21, "amdgpu_gfx908");
  DEF_REGISTER(                     s22, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  22, "amdgpu_gfx908");
  DEF_REGISTER(                     s23, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  23, "amdgpu_gfx908");
  DEF_REGISTER(                     s24, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  24, "amdgpu_gfx908");
  DEF_REGISTER(                     s25, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  25, "amdgpu_gfx908");
  DEF_REGISTER(                     s26, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  26, "amdgpu_gfx908");
  DEF_REGISTER(                     s27, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  27, "amdgpu_gfx908");
  DEF_REGISTER(                     s28, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  28, "amdgpu_gfx908");
  DEF_REGISTER(                     s29, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  29, "amdgpu_gfx908");
  DEF_REGISTER(                     s30, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  30, "amdgpu_gfx908");
  DEF_REGISTER(                     s31, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  31, "amdgpu_gfx908");
  DEF_REGISTER(                     s32, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  32, "amdgpu_gfx908");
  DEF_REGISTER(                     s33, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  33, "amdgpu_gfx908");
  DEF_REGISTER(                     s34, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  34, "amdgpu_gfx908");
  DEF_REGISTER(                     s35, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  35, "amdgpu_gfx908");
  DEF_REGISTER(                     s36, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  36, "amdgpu_gfx908");
  DEF_REGISTER(                     s37, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  37, "amdgpu_gfx908");
  DEF_REGISTER(                     s38, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  38, "amdgpu_gfx908");
  DEF_REGISTER(                     s39, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  39, "amdgpu_gfx908");
  DEF_REGISTER(                     s40, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  40, "amdgpu_gfx908");
  DEF_REGISTER(                     s41, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  41, "amdgpu_gfx908");
  DEF_REGISTER(                     s42, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  42, "amdgpu_gfx908");
  DEF_REGISTER(                     s43, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  43, "amdgpu_gfx908");
  DEF_REGISTER(                     s44, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  44, "amdgpu_gfx908");
  DEF_REGISTER(                     s45, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  45, "amdgpu_gfx908");
  DEF_REGISTER(                     s46, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  46, "amdgpu_gfx908");
  DEF_REGISTER(                     s47, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  47, "amdgpu_gfx908");
  DEF_REGISTER(                     s48, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  48, "amdgpu_gfx908");
  DEF_REGISTER(                     s49, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  49, "amdgpu_gfx908");
  DEF_REGISTER(                     s50, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  50, "amdgpu_gfx908");
  DEF_REGISTER(                     s51, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  51, "amdgpu_gfx908");
  DEF_REGISTER(                     s52, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  52, "amdgpu_gfx908");
  DEF_REGISTER(                     s53, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  53, "amdgpu_gfx908");
  DEF_REGISTER(                     s54, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  54, "amdgpu_gfx908");
  DEF_REGISTER(                     s55, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  55, "amdgpu_gfx908");
  DEF_REGISTER(                     s56, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  56, "amdgpu_gfx908");
  DEF_REGISTER(                     s57, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  57, "amdgpu_gfx908");
  DEF_REGISTER(                     s58, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  58, "amdgpu_gfx908");
  DEF_REGISTER(                     s59, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  59, "amdgpu_gfx908");
  DEF_REGISTER(                     s60, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  60, "amdgpu_gfx908");
  DEF_REGISTER(                     s61, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  61, "amdgpu_gfx908");
  DEF_REGISTER(                     s62, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  62, "amdgpu_gfx908");
  DEF_REGISTER(                     s63, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  63, "amdgpu_gfx908");
  DEF_REGISTER(                     s64, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  64, "amdgpu_gfx908");
  DEF_REGISTER(                     s65, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  65, "amdgpu_gfx908");
  DEF_REGISTER(                     s66, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  66, "amdgpu_gfx908");
  DEF_REGISTER(                     s67, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  67, "amdgpu_gfx908");
  DEF_REGISTER(                     s68, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  68, "amdgpu_gfx908");
  DEF_REGISTER(                     s69, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  69, "amdgpu_gfx908");
  DEF_REGISTER(                     s70, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  70, "amdgpu_gfx908");
  DEF_REGISTER(                     s71, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  71, "amdgpu_gfx908");
  DEF_REGISTER(                     s72, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  72, "amdgpu_gfx908");
  DEF_REGISTER(                     s73, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  73, "amdgpu_gfx908");
  DEF_REGISTER(                     s74, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  74, "amdgpu_gfx908");
  DEF_REGISTER(                     s75, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  75, "amdgpu_gfx908");
  DEF_REGISTER(                     s76, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  76, "amdgpu_gfx908");
  DEF_REGISTER(                     s77, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  77, "amdgpu_gfx908");
  DEF_REGISTER(                     s78, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  78, "amdgpu_gfx908");
  DEF_REGISTER(                     s79, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  79, "amdgpu_gfx908");
  DEF_REGISTER(                     s80, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  80, "amdgpu_gfx908");
  DEF_REGISTER(                     s81, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  81, "amdgpu_gfx908");
  DEF_REGISTER(                     s82, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  82, "amdgpu_gfx908");
  DEF_REGISTER(                     s83, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  83, "amdgpu_gfx908");
  DEF_REGISTER(                     s84, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  84, "amdgpu_gfx908");
  DEF_REGISTER(                     s85, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  85, "amdgpu_gfx908");
  DEF_REGISTER(                     s86, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  86, "amdgpu_gfx908");
  DEF_REGISTER(                     s87, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  87, "amdgpu_gfx908");
  DEF_REGISTER(                     s88, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  88, "amdgpu_gfx908");
  DEF_REGISTER(                     s89, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  89, "amdgpu_gfx908");
  DEF_REGISTER(                     s90, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  90, "amdgpu_gfx908");
  DEF_REGISTER(                     s91, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  91, "amdgpu_gfx908");
  DEF_REGISTER(                     s92, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  92, "amdgpu_gfx908");
  DEF_REGISTER(                     s93, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  93, "amdgpu_gfx908");
  DEF_REGISTER(                     s94, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  94, "amdgpu_gfx908");
  DEF_REGISTER(                     s95, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  95, "amdgpu_gfx908");
  DEF_REGISTER(                     s96, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  96, "amdgpu_gfx908");
  DEF_REGISTER(                     s97, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  97, "amdgpu_gfx908");
  DEF_REGISTER(                     s98, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  98, "amdgpu_gfx908");
  DEF_REGISTER(                     s99, Arch_amdgpu_gfx908 |      SGPR | BITS_32 |  99, "amdgpu_gfx908");
  DEF_REGISTER(                    s100, Arch_amdgpu_gfx908 |      SGPR | BITS_32 | 100, "amdgpu_gfx908");
  DEF_REGISTER(                    s101, Arch_amdgpu_gfx908 |      SGPR | BITS_32 | 101, "amdgpu_gfx908");
  DEF_REGISTER(                      v0, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                      v1, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                      v2, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                      v3, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                      v4, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                      v5, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                      v6, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                      v7, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                      v8, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                      v9, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                     v10, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  10, "amdgpu_gfx908");
  DEF_REGISTER(                     v11, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  11, "amdgpu_gfx908");
  DEF_REGISTER(                     v12, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                     v13, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                     v14, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                     v15, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(                     v16, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  16, "amdgpu_gfx908");
  DEF_REGISTER(                     v17, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  17, "amdgpu_gfx908");
  DEF_REGISTER(                     v18, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  18, "amdgpu_gfx908");
  DEF_REGISTER(                     v19, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  19, "amdgpu_gfx908");
  DEF_REGISTER(                     v20, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  20, "amdgpu_gfx908");
  DEF_REGISTER(                     v21, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  21, "amdgpu_gfx908");
  DEF_REGISTER(                     v22, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  22, "amdgpu_gfx908");
  DEF_REGISTER(                     v23, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  23, "amdgpu_gfx908");
  DEF_REGISTER(                     v24, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  24, "amdgpu_gfx908");
  DEF_REGISTER(                     v25, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  25, "amdgpu_gfx908");
  DEF_REGISTER(                     v26, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  26, "amdgpu_gfx908");
  DEF_REGISTER(                     v27, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  27, "amdgpu_gfx908");
  DEF_REGISTER(                     v28, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  28, "amdgpu_gfx908");
  DEF_REGISTER(                     v29, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  29, "amdgpu_gfx908");
  DEF_REGISTER(                     v30, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  30, "amdgpu_gfx908");
  DEF_REGISTER(                     v31, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  31, "amdgpu_gfx908");
  DEF_REGISTER(                     v32, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  32, "amdgpu_gfx908");
  DEF_REGISTER(                     v33, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  33, "amdgpu_gfx908");
  DEF_REGISTER(                     v34, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  34, "amdgpu_gfx908");
  DEF_REGISTER(                     v35, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  35, "amdgpu_gfx908");
  DEF_REGISTER(                     v36, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  36, "amdgpu_gfx908");
  DEF_REGISTER(                     v37, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  37, "amdgpu_gfx908");
  DEF_REGISTER(                     v38, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  38, "amdgpu_gfx908");
  DEF_REGISTER(                     v39, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  39, "amdgpu_gfx908");
  DEF_REGISTER(                     v40, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  40, "amdgpu_gfx908");
  DEF_REGISTER(                     v41, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  41, "amdgpu_gfx908");
  DEF_REGISTER(                     v42, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  42, "amdgpu_gfx908");
  DEF_REGISTER(                     v43, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  43, "amdgpu_gfx908");
  DEF_REGISTER(                     v44, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  44, "amdgpu_gfx908");
  DEF_REGISTER(                     v45, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  45, "amdgpu_gfx908");
  DEF_REGISTER(                     v46, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  46, "amdgpu_gfx908");
  DEF_REGISTER(                     v47, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  47, "amdgpu_gfx908");
  DEF_REGISTER(                     v48, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  48, "amdgpu_gfx908");
  DEF_REGISTER(                     v49, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  49, "amdgpu_gfx908");
  DEF_REGISTER(                     v50, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  50, "amdgpu_gfx908");
  DEF_REGISTER(                     v51, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  51, "amdgpu_gfx908");
  DEF_REGISTER(                     v52, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  52, "amdgpu_gfx908");
  DEF_REGISTER(                     v53, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  53, "amdgpu_gfx908");
  DEF_REGISTER(                     v54, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  54, "amdgpu_gfx908");
  DEF_REGISTER(                     v55, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  55, "amdgpu_gfx908");
  DEF_REGISTER(                     v56, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  56, "amdgpu_gfx908");
  DEF_REGISTER(                     v57, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  57, "amdgpu_gfx908");
  DEF_REGISTER(                     v58, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  58, "amdgpu_gfx908");
  DEF_REGISTER(                     v59, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  59, "amdgpu_gfx908");
  DEF_REGISTER(                     v60, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  60, "amdgpu_gfx908");
  DEF_REGISTER(                     v61, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  61, "amdgpu_gfx908");
  DEF_REGISTER(                     v62, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  62, "amdgpu_gfx908");
  DEF_REGISTER(                     v63, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  63, "amdgpu_gfx908");
  DEF_REGISTER(                     v64, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  64, "amdgpu_gfx908");
  DEF_REGISTER(                     v65, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  65, "amdgpu_gfx908");
  DEF_REGISTER(                     v66, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  66, "amdgpu_gfx908");
  DEF_REGISTER(                     v67, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  67, "amdgpu_gfx908");
  DEF_REGISTER(                     v68, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  68, "amdgpu_gfx908");
  DEF_REGISTER(                     v69, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  69, "amdgpu_gfx908");
  DEF_REGISTER(                     v70, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  70, "amdgpu_gfx908");
  DEF_REGISTER(                     v71, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  71, "amdgpu_gfx908");
  DEF_REGISTER(                     v72, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  72, "amdgpu_gfx908");
  DEF_REGISTER(                     v73, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  73, "amdgpu_gfx908");
  DEF_REGISTER(                     v74, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  74, "amdgpu_gfx908");
  DEF_REGISTER(                     v75, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  75, "amdgpu_gfx908");
  DEF_REGISTER(                     v76, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  76, "amdgpu_gfx908");
  DEF_REGISTER(                     v77, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  77, "amdgpu_gfx908");
  DEF_REGISTER(                     v78, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  78, "amdgpu_gfx908");
  DEF_REGISTER(                     v79, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  79, "amdgpu_gfx908");
  DEF_REGISTER(                     v80, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  80, "amdgpu_gfx908");
  DEF_REGISTER(                     v81, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  81, "amdgpu_gfx908");
  DEF_REGISTER(                     v82, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  82, "amdgpu_gfx908");
  DEF_REGISTER(                     v83, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  83, "amdgpu_gfx908");
  DEF_REGISTER(                     v84, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  84, "amdgpu_gfx908");
  DEF_REGISTER(                     v85, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  85, "amdgpu_gfx908");
  DEF_REGISTER(                     v86, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  86, "amdgpu_gfx908");
  DEF_REGISTER(                     v87, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  87, "amdgpu_gfx908");
  DEF_REGISTER(                     v88, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  88, "amdgpu_gfx908");
  DEF_REGISTER(                     v89, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  89, "amdgpu_gfx908");
  DEF_REGISTER(                     v90, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  90, "amdgpu_gfx908");
  DEF_REGISTER(                     v91, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  91, "amdgpu_gfx908");
  DEF_REGISTER(                     v92, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  92, "amdgpu_gfx908");
  DEF_REGISTER(                     v93, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  93, "amdgpu_gfx908");
  DEF_REGISTER(                     v94, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  94, "amdgpu_gfx908");
  DEF_REGISTER(                     v95, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  95, "amdgpu_gfx908");
  DEF_REGISTER(                     v96, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  96, "amdgpu_gfx908");
  DEF_REGISTER(                     v97, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  97, "amdgpu_gfx908");
  DEF_REGISTER(                     v98, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  98, "amdgpu_gfx908");
  DEF_REGISTER(                     v99, Arch_amdgpu_gfx908 |      VGPR | BITS_32 |  99, "amdgpu_gfx908");
  DEF_REGISTER(                    v100, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 100, "amdgpu_gfx908");
  DEF_REGISTER(                    v101, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 101, "amdgpu_gfx908");
  DEF_REGISTER(                    v102, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 102, "amdgpu_gfx908");
  DEF_REGISTER(                    v103, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 103, "amdgpu_gfx908");
  DEF_REGISTER(                    v104, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 104, "amdgpu_gfx908");
  DEF_REGISTER(                    v105, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 105, "amdgpu_gfx908");
  DEF_REGISTER(                    v106, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 106, "amdgpu_gfx908");
  DEF_REGISTER(                    v107, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 107, "amdgpu_gfx908");
  DEF_REGISTER(                    v108, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 108, "amdgpu_gfx908");
  DEF_REGISTER(                    v109, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 109, "amdgpu_gfx908");
  DEF_REGISTER(                    v110, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 110, "amdgpu_gfx908");
  DEF_REGISTER(                    v111, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 111, "amdgpu_gfx908");
  DEF_REGISTER(                    v112, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 112, "amdgpu_gfx908");
  DEF_REGISTER(                    v113, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 113, "amdgpu_gfx908");
  DEF_REGISTER(                    v114, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 114, "amdgpu_gfx908");
  DEF_REGISTER(                    v115, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 115, "amdgpu_gfx908");
  DEF_REGISTER(                    v116, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 116, "amdgpu_gfx908");
  DEF_REGISTER(                    v117, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 117, "amdgpu_gfx908");
  DEF_REGISTER(                    v118, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 118, "amdgpu_gfx908");
  DEF_REGISTER(                    v119, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 119, "amdgpu_gfx908");
  DEF_REGISTER(                    v120, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 120, "amdgpu_gfx908");
  DEF_REGISTER(                    v121, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 121, "amdgpu_gfx908");
  DEF_REGISTER(                    v122, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 122, "amdgpu_gfx908");
  DEF_REGISTER(                    v123, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 123, "amdgpu_gfx908");
  DEF_REGISTER(                    v124, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 124, "amdgpu_gfx908");
  DEF_REGISTER(                    v125, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 125, "amdgpu_gfx908");
  DEF_REGISTER(                    v126, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 126, "amdgpu_gfx908");
  DEF_REGISTER(                    v127, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 127, "amdgpu_gfx908");
  DEF_REGISTER(                    v128, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 128, "amdgpu_gfx908");
  DEF_REGISTER(                    v129, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 129, "amdgpu_gfx908");
  DEF_REGISTER(                    v130, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 130, "amdgpu_gfx908");
  DEF_REGISTER(                    v131, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 131, "amdgpu_gfx908");
  DEF_REGISTER(                    v132, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 132, "amdgpu_gfx908");
  DEF_REGISTER(                    v133, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 133, "amdgpu_gfx908");
  DEF_REGISTER(                    v134, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 134, "amdgpu_gfx908");
  DEF_REGISTER(                    v135, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 135, "amdgpu_gfx908");
  DEF_REGISTER(                    v136, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 136, "amdgpu_gfx908");
  DEF_REGISTER(                    v137, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 137, "amdgpu_gfx908");
  DEF_REGISTER(                    v138, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 138, "amdgpu_gfx908");
  DEF_REGISTER(                    v139, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 139, "amdgpu_gfx908");
  DEF_REGISTER(                    v140, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 140, "amdgpu_gfx908");
  DEF_REGISTER(                    v141, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 141, "amdgpu_gfx908");
  DEF_REGISTER(                    v142, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 142, "amdgpu_gfx908");
  DEF_REGISTER(                    v143, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 143, "amdgpu_gfx908");
  DEF_REGISTER(                    v144, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 144, "amdgpu_gfx908");
  DEF_REGISTER(                    v145, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 145, "amdgpu_gfx908");
  DEF_REGISTER(                    v146, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 146, "amdgpu_gfx908");
  DEF_REGISTER(                    v147, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 147, "amdgpu_gfx908");
  DEF_REGISTER(                    v148, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 148, "amdgpu_gfx908");
  DEF_REGISTER(                    v149, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 149, "amdgpu_gfx908");
  DEF_REGISTER(                    v150, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 150, "amdgpu_gfx908");
  DEF_REGISTER(                    v151, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 151, "amdgpu_gfx908");
  DEF_REGISTER(                    v152, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 152, "amdgpu_gfx908");
  DEF_REGISTER(                    v153, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 153, "amdgpu_gfx908");
  DEF_REGISTER(                    v154, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 154, "amdgpu_gfx908");
  DEF_REGISTER(                    v155, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 155, "amdgpu_gfx908");
  DEF_REGISTER(                    v156, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 156, "amdgpu_gfx908");
  DEF_REGISTER(                    v157, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 157, "amdgpu_gfx908");
  DEF_REGISTER(                    v158, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 158, "amdgpu_gfx908");
  DEF_REGISTER(                    v159, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 159, "amdgpu_gfx908");
  DEF_REGISTER(                    v160, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 160, "amdgpu_gfx908");
  DEF_REGISTER(                    v161, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 161, "amdgpu_gfx908");
  DEF_REGISTER(                    v162, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 162, "amdgpu_gfx908");
  DEF_REGISTER(                    v163, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 163, "amdgpu_gfx908");
  DEF_REGISTER(                    v164, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 164, "amdgpu_gfx908");
  DEF_REGISTER(                    v165, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 165, "amdgpu_gfx908");
  DEF_REGISTER(                    v166, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 166, "amdgpu_gfx908");
  DEF_REGISTER(                    v167, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 167, "amdgpu_gfx908");
  DEF_REGISTER(                    v168, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 168, "amdgpu_gfx908");
  DEF_REGISTER(                    v169, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 169, "amdgpu_gfx908");
  DEF_REGISTER(                    v170, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 170, "amdgpu_gfx908");
  DEF_REGISTER(                    v171, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 171, "amdgpu_gfx908");
  DEF_REGISTER(                    v172, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 172, "amdgpu_gfx908");
  DEF_REGISTER(                    v173, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 173, "amdgpu_gfx908");
  DEF_REGISTER(                    v174, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 174, "amdgpu_gfx908");
  DEF_REGISTER(                    v175, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 175, "amdgpu_gfx908");
  DEF_REGISTER(                    v176, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 176, "amdgpu_gfx908");
  DEF_REGISTER(                    v177, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 177, "amdgpu_gfx908");
  DEF_REGISTER(                    v178, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 178, "amdgpu_gfx908");
  DEF_REGISTER(                    v179, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 179, "amdgpu_gfx908");
  DEF_REGISTER(                    v180, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 180, "amdgpu_gfx908");
  DEF_REGISTER(                    v181, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 181, "amdgpu_gfx908");
  DEF_REGISTER(                    v182, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 182, "amdgpu_gfx908");
  DEF_REGISTER(                    v183, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 183, "amdgpu_gfx908");
  DEF_REGISTER(                    v184, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 184, "amdgpu_gfx908");
  DEF_REGISTER(                    v185, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 185, "amdgpu_gfx908");
  DEF_REGISTER(                    v186, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 186, "amdgpu_gfx908");
  DEF_REGISTER(                    v187, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 187, "amdgpu_gfx908");
  DEF_REGISTER(                    v188, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 188, "amdgpu_gfx908");
  DEF_REGISTER(                    v189, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 189, "amdgpu_gfx908");
  DEF_REGISTER(                    v190, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 190, "amdgpu_gfx908");
  DEF_REGISTER(                    v191, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 191, "amdgpu_gfx908");
  DEF_REGISTER(                    v192, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 192, "amdgpu_gfx908");
  DEF_REGISTER(                    v193, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 193, "amdgpu_gfx908");
  DEF_REGISTER(                    v194, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 194, "amdgpu_gfx908");
  DEF_REGISTER(                    v195, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 195, "amdgpu_gfx908");
  DEF_REGISTER(                    v196, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 196, "amdgpu_gfx908");
  DEF_REGISTER(                    v197, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 197, "amdgpu_gfx908");
  DEF_REGISTER(                    v198, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 198, "amdgpu_gfx908");
  DEF_REGISTER(                    v199, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 199, "amdgpu_gfx908");
  DEF_REGISTER(                    v200, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 200, "amdgpu_gfx908");
  DEF_REGISTER(                    v201, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 201, "amdgpu_gfx908");
  DEF_REGISTER(                    v202, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 202, "amdgpu_gfx908");
  DEF_REGISTER(                    v203, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 203, "amdgpu_gfx908");
  DEF_REGISTER(                    v204, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 204, "amdgpu_gfx908");
  DEF_REGISTER(                    v205, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 205, "amdgpu_gfx908");
  DEF_REGISTER(                    v206, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 206, "amdgpu_gfx908");
  DEF_REGISTER(                    v207, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 207, "amdgpu_gfx908");
  DEF_REGISTER(                    v208, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 208, "amdgpu_gfx908");
  DEF_REGISTER(                    v209, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 209, "amdgpu_gfx908");
  DEF_REGISTER(                    v210, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 210, "amdgpu_gfx908");
  DEF_REGISTER(                    v211, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 211, "amdgpu_gfx908");
  DEF_REGISTER(                    v212, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 212, "amdgpu_gfx908");
  DEF_REGISTER(                    v213, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 213, "amdgpu_gfx908");
  DEF_REGISTER(                    v214, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 214, "amdgpu_gfx908");
  DEF_REGISTER(                    v215, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 215, "amdgpu_gfx908");
  DEF_REGISTER(                    v216, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 216, "amdgpu_gfx908");
  DEF_REGISTER(                    v217, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 217, "amdgpu_gfx908");
  DEF_REGISTER(                    v218, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 218, "amdgpu_gfx908");
  DEF_REGISTER(                    v219, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 219, "amdgpu_gfx908");
  DEF_REGISTER(                    v220, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 220, "amdgpu_gfx908");
  DEF_REGISTER(                    v221, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 221, "amdgpu_gfx908");
  DEF_REGISTER(                    v222, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 222, "amdgpu_gfx908");
  DEF_REGISTER(                    v223, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 223, "amdgpu_gfx908");
  DEF_REGISTER(                    v224, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 224, "amdgpu_gfx908");
  DEF_REGISTER(                    v225, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 225, "amdgpu_gfx908");
  DEF_REGISTER(                    v226, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 226, "amdgpu_gfx908");
  DEF_REGISTER(                    v227, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 227, "amdgpu_gfx908");
  DEF_REGISTER(                    v228, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 228, "amdgpu_gfx908");
  DEF_REGISTER(                    v229, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 229, "amdgpu_gfx908");
  DEF_REGISTER(                    v230, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 230, "amdgpu_gfx908");
  DEF_REGISTER(                    v231, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 231, "amdgpu_gfx908");
  DEF_REGISTER(                    v232, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 232, "amdgpu_gfx908");
  DEF_REGISTER(                    v233, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 233, "amdgpu_gfx908");
  DEF_REGISTER(                    v234, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 234, "amdgpu_gfx908");
  DEF_REGISTER(                    v235, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 235, "amdgpu_gfx908");
  DEF_REGISTER(                    v236, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 236, "amdgpu_gfx908");
  DEF_REGISTER(                    v237, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 237, "amdgpu_gfx908");
  DEF_REGISTER(                    v238, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 238, "amdgpu_gfx908");
  DEF_REGISTER(                    v239, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 239, "amdgpu_gfx908");
  DEF_REGISTER(                    v240, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 240, "amdgpu_gfx908");
  DEF_REGISTER(                    v241, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 241, "amdgpu_gfx908");
  DEF_REGISTER(                    v242, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 242, "amdgpu_gfx908");
  DEF_REGISTER(                    v243, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 243, "amdgpu_gfx908");
  DEF_REGISTER(                    v244, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 244, "amdgpu_gfx908");
  DEF_REGISTER(                    v245, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 245, "amdgpu_gfx908");
  DEF_REGISTER(                    v246, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 246, "amdgpu_gfx908");
  DEF_REGISTER(                    v247, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 247, "amdgpu_gfx908");
  DEF_REGISTER(                    v248, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 248, "amdgpu_gfx908");
  DEF_REGISTER(                    v249, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 249, "amdgpu_gfx908");
  DEF_REGISTER(                    v250, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 250, "amdgpu_gfx908");
  DEF_REGISTER(                    v251, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 251, "amdgpu_gfx908");
  DEF_REGISTER(                    v252, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 252, "amdgpu_gfx908");
  DEF_REGISTER(                    v253, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 253, "amdgpu_gfx908");
  DEF_REGISTER(                    v254, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 254, "amdgpu_gfx908");
  DEF_REGISTER(                    v255, Arch_amdgpu_gfx908 |      VGPR | BITS_32 | 255, "amdgpu_gfx908");
  DEF_REGISTER(                    acc0, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   0, "amdgpu_gfx908");
  DEF_REGISTER(                    acc1, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   1, "amdgpu_gfx908");
  DEF_REGISTER(                    acc2, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   2, "amdgpu_gfx908");
  DEF_REGISTER(                    acc3, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   3, "amdgpu_gfx908");
  DEF_REGISTER(                    acc4, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   4, "amdgpu_gfx908");
  DEF_REGISTER(                    acc5, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   5, "amdgpu_gfx908");
  DEF_REGISTER(                    acc6, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   6, "amdgpu_gfx908");
  DEF_REGISTER(                    acc7, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   7, "amdgpu_gfx908");
  DEF_REGISTER(                    acc8, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   8, "amdgpu_gfx908");
  DEF_REGISTER(                    acc9, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |   9, "amdgpu_gfx908");
  DEF_REGISTER(                   acc10, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  10, "amdgpu_gfx908");
  DEF_REGISTER(                   acc11, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  11, "amdgpu_gfx908");
  DEF_REGISTER(                   acc12, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  12, "amdgpu_gfx908");
  DEF_REGISTER(                   acc13, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  13, "amdgpu_gfx908");
  DEF_REGISTER(                   acc14, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  14, "amdgpu_gfx908");
  DEF_REGISTER(                   acc15, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  15, "amdgpu_gfx908");
  DEF_REGISTER(                   acc16, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  16, "amdgpu_gfx908");
  DEF_REGISTER(                   acc17, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  17, "amdgpu_gfx908");
  DEF_REGISTER(                   acc18, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  18, "amdgpu_gfx908");
  DEF_REGISTER(                   acc19, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  19, "amdgpu_gfx908");
  DEF_REGISTER(                   acc20, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  20, "amdgpu_gfx908");
  DEF_REGISTER(                   acc21, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  21, "amdgpu_gfx908");
  DEF_REGISTER(                   acc22, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  22, "amdgpu_gfx908");
  DEF_REGISTER(                   acc23, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  23, "amdgpu_gfx908");
  DEF_REGISTER(                   acc24, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  24, "amdgpu_gfx908");
  DEF_REGISTER(                   acc25, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  25, "amdgpu_gfx908");
  DEF_REGISTER(                   acc26, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  26, "amdgpu_gfx908");
  DEF_REGISTER(                   acc27, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  27, "amdgpu_gfx908");
  DEF_REGISTER(                   acc28, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  28, "amdgpu_gfx908");
  DEF_REGISTER(                   acc29, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  29, "amdgpu_gfx908");
  DEF_REGISTER(                   acc30, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  30, "amdgpu_gfx908");
  DEF_REGISTER(                   acc31, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  31, "amdgpu_gfx908");
  DEF_REGISTER(                   acc32, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  32, "amdgpu_gfx908");
  DEF_REGISTER(                   acc33, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  33, "amdgpu_gfx908");
  DEF_REGISTER(                   acc34, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  34, "amdgpu_gfx908");
  DEF_REGISTER(                   acc35, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  35, "amdgpu_gfx908");
  DEF_REGISTER(                   acc36, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  36, "amdgpu_gfx908");
  DEF_REGISTER(                   acc37, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  37, "amdgpu_gfx908");
  DEF_REGISTER(                   acc38, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  38, "amdgpu_gfx908");
  DEF_REGISTER(                   acc39, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  39, "amdgpu_gfx908");
  DEF_REGISTER(                   acc40, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  40, "amdgpu_gfx908");
  DEF_REGISTER(                   acc41, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  41, "amdgpu_gfx908");
  DEF_REGISTER(                   acc42, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  42, "amdgpu_gfx908");
  DEF_REGISTER(                   acc43, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  43, "amdgpu_gfx908");
  DEF_REGISTER(                   acc44, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  44, "amdgpu_gfx908");
  DEF_REGISTER(                   acc45, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  45, "amdgpu_gfx908");
  DEF_REGISTER(                   acc46, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  46, "amdgpu_gfx908");
  DEF_REGISTER(                   acc47, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  47, "amdgpu_gfx908");
  DEF_REGISTER(                   acc48, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  48, "amdgpu_gfx908");
  DEF_REGISTER(                   acc49, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  49, "amdgpu_gfx908");
  DEF_REGISTER(                   acc50, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  50, "amdgpu_gfx908");
  DEF_REGISTER(                   acc51, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  51, "amdgpu_gfx908");
  DEF_REGISTER(                   acc52, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  52, "amdgpu_gfx908");
  DEF_REGISTER(                   acc53, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  53, "amdgpu_gfx908");
  DEF_REGISTER(                   acc54, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  54, "amdgpu_gfx908");
  DEF_REGISTER(                   acc55, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  55, "amdgpu_gfx908");
  DEF_REGISTER(                   acc56, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  56, "amdgpu_gfx908");
  DEF_REGISTER(                   acc57, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  57, "amdgpu_gfx908");
  DEF_REGISTER(                   acc58, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  58, "amdgpu_gfx908");
  DEF_REGISTER(                   acc59, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  59, "amdgpu_gfx908");
  DEF_REGISTER(                   acc60, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  60, "amdgpu_gfx908");
  DEF_REGISTER(                   acc61, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  61, "amdgpu_gfx908");
  DEF_REGISTER(                   acc62, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  62, "amdgpu_gfx908");
  DEF_REGISTER(                   acc63, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  63, "amdgpu_gfx908");
  DEF_REGISTER(                   acc64, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  64, "amdgpu_gfx908");
  DEF_REGISTER(                   acc65, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  65, "amdgpu_gfx908");
  DEF_REGISTER(                   acc66, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  66, "amdgpu_gfx908");
  DEF_REGISTER(                   acc67, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  67, "amdgpu_gfx908");
  DEF_REGISTER(                   acc68, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  68, "amdgpu_gfx908");
  DEF_REGISTER(                   acc69, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  69, "amdgpu_gfx908");
  DEF_REGISTER(                   acc70, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  70, "amdgpu_gfx908");
  DEF_REGISTER(                   acc71, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  71, "amdgpu_gfx908");
  DEF_REGISTER(                   acc72, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  72, "amdgpu_gfx908");
  DEF_REGISTER(                   acc73, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  73, "amdgpu_gfx908");
  DEF_REGISTER(                   acc74, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  74, "amdgpu_gfx908");
  DEF_REGISTER(                   acc75, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  75, "amdgpu_gfx908");
  DEF_REGISTER(                   acc76, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  76, "amdgpu_gfx908");
  DEF_REGISTER(                   acc77, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  77, "amdgpu_gfx908");
  DEF_REGISTER(                   acc78, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  78, "amdgpu_gfx908");
  DEF_REGISTER(                   acc79, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  79, "amdgpu_gfx908");
  DEF_REGISTER(                   acc80, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  80, "amdgpu_gfx908");
  DEF_REGISTER(                   acc81, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  81, "amdgpu_gfx908");
  DEF_REGISTER(                   acc82, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  82, "amdgpu_gfx908");
  DEF_REGISTER(                   acc83, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  83, "amdgpu_gfx908");
  DEF_REGISTER(                   acc84, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  84, "amdgpu_gfx908");
  DEF_REGISTER(                   acc85, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  85, "amdgpu_gfx908");
  DEF_REGISTER(                   acc86, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  86, "amdgpu_gfx908");
  DEF_REGISTER(                   acc87, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  87, "amdgpu_gfx908");
  DEF_REGISTER(                   acc88, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  88, "amdgpu_gfx908");
  DEF_REGISTER(                   acc89, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  89, "amdgpu_gfx908");
  DEF_REGISTER(                   acc90, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  90, "amdgpu_gfx908");
  DEF_REGISTER(                   acc91, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  91, "amdgpu_gfx908");
  DEF_REGISTER(                   acc92, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  92, "amdgpu_gfx908");
  DEF_REGISTER(                   acc93, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  93, "amdgpu_gfx908");
  DEF_REGISTER(                   acc94, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  94, "amdgpu_gfx908");
  DEF_REGISTER(                   acc95, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  95, "amdgpu_gfx908");
  DEF_REGISTER(                   acc96, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  96, "amdgpu_gfx908");
  DEF_REGISTER(                   acc97, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  97, "amdgpu_gfx908");
  DEF_REGISTER(                   acc98, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  98, "amdgpu_gfx908");
  DEF_REGISTER(                   acc99, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 |  99, "amdgpu_gfx908");
  DEF_REGISTER(                  acc100, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 100, "amdgpu_gfx908");
  DEF_REGISTER(                  acc101, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 101, "amdgpu_gfx908");
  DEF_REGISTER(                  acc102, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 102, "amdgpu_gfx908");
  DEF_REGISTER(                  acc103, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 103, "amdgpu_gfx908");
  DEF_REGISTER(                  acc104, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 104, "amdgpu_gfx908");
  DEF_REGISTER(                  acc105, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 105, "amdgpu_gfx908");
  DEF_REGISTER(                  acc106, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 106, "amdgpu_gfx908");
  DEF_REGISTER(                  acc107, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 107, "amdgpu_gfx908");
  DEF_REGISTER(                  acc108, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 108, "amdgpu_gfx908");
  DEF_REGISTER(                  acc109, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 109, "amdgpu_gfx908");
  DEF_REGISTER(                  acc110, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 110, "amdgpu_gfx908");
  DEF_REGISTER(                  acc111, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 111, "amdgpu_gfx908");
  DEF_REGISTER(                  acc112, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 112, "amdgpu_gfx908");
  DEF_REGISTER(                  acc113, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 113, "amdgpu_gfx908");
  DEF_REGISTER(                  acc114, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 114, "amdgpu_gfx908");
  DEF_REGISTER(                  acc115, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 115, "amdgpu_gfx908");
  DEF_REGISTER(                  acc116, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 116, "amdgpu_gfx908");
  DEF_REGISTER(                  acc117, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 117, "amdgpu_gfx908");
  DEF_REGISTER(                  acc118, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 118, "amdgpu_gfx908");
  DEF_REGISTER(                  acc119, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 119, "amdgpu_gfx908");
  DEF_REGISTER(                  acc120, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 120, "amdgpu_gfx908");
  DEF_REGISTER(                  acc121, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 121, "amdgpu_gfx908");
  DEF_REGISTER(                  acc122, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 122, "amdgpu_gfx908");
  DEF_REGISTER(                  acc123, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 123, "amdgpu_gfx908");
  DEF_REGISTER(                  acc124, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 124, "amdgpu_gfx908");
  DEF_REGISTER(                  acc125, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 125, "amdgpu_gfx908");
  DEF_REGISTER(                  acc126, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 126, "amdgpu_gfx908");
  DEF_REGISTER(                  acc127, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 127, "amdgpu_gfx908");
  DEF_REGISTER(                  acc128, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 128, "amdgpu_gfx908");
  DEF_REGISTER(                  acc129, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 129, "amdgpu_gfx908");
  DEF_REGISTER(                  acc130, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 130, "amdgpu_gfx908");
  DEF_REGISTER(                  acc131, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 131, "amdgpu_gfx908");
  DEF_REGISTER(                  acc132, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 132, "amdgpu_gfx908");
  DEF_REGISTER(                  acc133, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 133, "amdgpu_gfx908");
  DEF_REGISTER(                  acc134, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 134, "amdgpu_gfx908");
  DEF_REGISTER(                  acc135, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 135, "amdgpu_gfx908");
  DEF_REGISTER(                  acc136, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 136, "amdgpu_gfx908");
  DEF_REGISTER(                  acc137, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 137, "amdgpu_gfx908");
  DEF_REGISTER(                  acc138, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 138, "amdgpu_gfx908");
  DEF_REGISTER(                  acc139, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 139, "amdgpu_gfx908");
  DEF_REGISTER(                  acc140, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 140, "amdgpu_gfx908");
  DEF_REGISTER(                  acc141, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 141, "amdgpu_gfx908");
  DEF_REGISTER(                  acc142, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 142, "amdgpu_gfx908");
  DEF_REGISTER(                  acc143, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 143, "amdgpu_gfx908");
  DEF_REGISTER(                  acc144, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 144, "amdgpu_gfx908");
  DEF_REGISTER(                  acc145, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 145, "amdgpu_gfx908");
  DEF_REGISTER(                  acc146, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 146, "amdgpu_gfx908");
  DEF_REGISTER(                  acc147, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 147, "amdgpu_gfx908");
  DEF_REGISTER(                  acc148, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 148, "amdgpu_gfx908");
  DEF_REGISTER(                  acc149, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 149, "amdgpu_gfx908");
  DEF_REGISTER(                  acc150, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 150, "amdgpu_gfx908");
  DEF_REGISTER(                  acc151, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 151, "amdgpu_gfx908");
  DEF_REGISTER(                  acc152, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 152, "amdgpu_gfx908");
  DEF_REGISTER(                  acc153, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 153, "amdgpu_gfx908");
  DEF_REGISTER(                  acc154, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 154, "amdgpu_gfx908");
  DEF_REGISTER(                  acc155, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 155, "amdgpu_gfx908");
  DEF_REGISTER(                  acc156, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 156, "amdgpu_gfx908");
  DEF_REGISTER(                  acc157, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 157, "amdgpu_gfx908");
  DEF_REGISTER(                  acc158, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 158, "amdgpu_gfx908");
  DEF_REGISTER(                  acc159, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 159, "amdgpu_gfx908");
  DEF_REGISTER(                  acc160, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 160, "amdgpu_gfx908");
  DEF_REGISTER(                  acc161, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 161, "amdgpu_gfx908");
  DEF_REGISTER(                  acc162, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 162, "amdgpu_gfx908");
  DEF_REGISTER(                  acc163, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 163, "amdgpu_gfx908");
  DEF_REGISTER(                  acc164, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 164, "amdgpu_gfx908");
  DEF_REGISTER(                  acc165, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 165, "amdgpu_gfx908");
  DEF_REGISTER(                  acc166, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 166, "amdgpu_gfx908");
  DEF_REGISTER(                  acc167, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 167, "amdgpu_gfx908");
  DEF_REGISTER(                  acc168, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 168, "amdgpu_gfx908");
  DEF_REGISTER(                  acc169, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 169, "amdgpu_gfx908");
  DEF_REGISTER(                  acc170, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 170, "amdgpu_gfx908");
  DEF_REGISTER(                  acc171, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 171, "amdgpu_gfx908");
  DEF_REGISTER(                  acc172, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 172, "amdgpu_gfx908");
  DEF_REGISTER(                  acc173, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 173, "amdgpu_gfx908");
  DEF_REGISTER(                  acc174, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 174, "amdgpu_gfx908");
  DEF_REGISTER(                  acc175, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 175, "amdgpu_gfx908");
  DEF_REGISTER(                  acc176, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 176, "amdgpu_gfx908");
  DEF_REGISTER(                  acc177, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 177, "amdgpu_gfx908");
  DEF_REGISTER(                  acc178, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 178, "amdgpu_gfx908");
  DEF_REGISTER(                  acc179, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 179, "amdgpu_gfx908");
  DEF_REGISTER(                  acc180, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 180, "amdgpu_gfx908");
  DEF_REGISTER(                  acc181, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 181, "amdgpu_gfx908");
  DEF_REGISTER(                  acc182, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 182, "amdgpu_gfx908");
  DEF_REGISTER(                  acc183, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 183, "amdgpu_gfx908");
  DEF_REGISTER(                  acc184, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 184, "amdgpu_gfx908");
  DEF_REGISTER(                  acc185, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 185, "amdgpu_gfx908");
  DEF_REGISTER(                  acc186, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 186, "amdgpu_gfx908");
  DEF_REGISTER(                  acc187, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 187, "amdgpu_gfx908");
  DEF_REGISTER(                  acc188, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 188, "amdgpu_gfx908");
  DEF_REGISTER(                  acc189, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 189, "amdgpu_gfx908");
  DEF_REGISTER(                  acc190, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 190, "amdgpu_gfx908");
  DEF_REGISTER(                  acc191, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 191, "amdgpu_gfx908");
  DEF_REGISTER(                  acc192, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 192, "amdgpu_gfx908");
  DEF_REGISTER(                  acc193, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 193, "amdgpu_gfx908");
  DEF_REGISTER(                  acc194, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 194, "amdgpu_gfx908");
  DEF_REGISTER(                  acc195, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 195, "amdgpu_gfx908");
  DEF_REGISTER(                  acc196, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 196, "amdgpu_gfx908");
  DEF_REGISTER(                  acc197, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 197, "amdgpu_gfx908");
  DEF_REGISTER(                  acc198, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 198, "amdgpu_gfx908");
  DEF_REGISTER(                  acc199, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 199, "amdgpu_gfx908");
  DEF_REGISTER(                  acc200, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 200, "amdgpu_gfx908");
  DEF_REGISTER(                  acc201, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 201, "amdgpu_gfx908");
  DEF_REGISTER(                  acc202, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 202, "amdgpu_gfx908");
  DEF_REGISTER(                  acc203, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 203, "amdgpu_gfx908");
  DEF_REGISTER(                  acc204, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 204, "amdgpu_gfx908");
  DEF_REGISTER(                  acc205, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 205, "amdgpu_gfx908");
  DEF_REGISTER(                  acc206, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 206, "amdgpu_gfx908");
  DEF_REGISTER(                  acc207, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 207, "amdgpu_gfx908");
  DEF_REGISTER(                  acc208, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 208, "amdgpu_gfx908");
  DEF_REGISTER(                  acc209, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 209, "amdgpu_gfx908");
  DEF_REGISTER(                  acc210, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 210, "amdgpu_gfx908");
  DEF_REGISTER(                  acc211, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 211, "amdgpu_gfx908");
  DEF_REGISTER(                  acc212, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 212, "amdgpu_gfx908");
  DEF_REGISTER(                  acc213, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 213, "amdgpu_gfx908");
  DEF_REGISTER(                  acc214, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 214, "amdgpu_gfx908");
  DEF_REGISTER(                  acc215, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 215, "amdgpu_gfx908");
  DEF_REGISTER(                  acc216, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 216, "amdgpu_gfx908");
  DEF_REGISTER(                  acc217, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 217, "amdgpu_gfx908");
  DEF_REGISTER(                  acc218, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 218, "amdgpu_gfx908");
  DEF_REGISTER(                  acc219, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 219, "amdgpu_gfx908");
  DEF_REGISTER(                  acc220, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 220, "amdgpu_gfx908");
  DEF_REGISTER(                  acc221, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 221, "amdgpu_gfx908");
  DEF_REGISTER(                  acc222, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 222, "amdgpu_gfx908");
  DEF_REGISTER(                  acc223, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 223, "amdgpu_gfx908");
  DEF_REGISTER(                  acc224, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 224, "amdgpu_gfx908");
  DEF_REGISTER(                  acc225, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 225, "amdgpu_gfx908");
  DEF_REGISTER(                  acc226, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 226, "amdgpu_gfx908");
  DEF_REGISTER(                  acc227, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 227, "amdgpu_gfx908");
  DEF_REGISTER(                  acc228, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 228, "amdgpu_gfx908");
  DEF_REGISTER(                  acc229, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 229, "amdgpu_gfx908");
  DEF_REGISTER(                  acc230, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 230, "amdgpu_gfx908");
  DEF_REGISTER(                  acc231, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 231, "amdgpu_gfx908");
  DEF_REGISTER(                  acc232, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 232, "amdgpu_gfx908");
  DEF_REGISTER(                  acc233, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 233, "amdgpu_gfx908");
  DEF_REGISTER(                  acc234, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 234, "amdgpu_gfx908");
  DEF_REGISTER(                  acc235, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 235, "amdgpu_gfx908");
  DEF_REGISTER(                  acc236, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 236, "amdgpu_gfx908");
  DEF_REGISTER(                  acc237, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 237, "amdgpu_gfx908");
  DEF_REGISTER(                  acc238, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 238, "amdgpu_gfx908");
  DEF_REGISTER(                  acc239, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 239, "amdgpu_gfx908");
  DEF_REGISTER(                  acc240, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 240, "amdgpu_gfx908");
  DEF_REGISTER(                  acc241, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 241, "amdgpu_gfx908");
  DEF_REGISTER(                  acc242, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 242, "amdgpu_gfx908");
  DEF_REGISTER(                  acc243, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 243, "amdgpu_gfx908");
  DEF_REGISTER(                  acc244, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 244, "amdgpu_gfx908");
  DEF_REGISTER(                  acc245, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 245, "amdgpu_gfx908");
  DEF_REGISTER(                  acc246, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 246, "amdgpu_gfx908");
  DEF_REGISTER(                  acc247, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 247, "amdgpu_gfx908");
  DEF_REGISTER(                  acc248, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 248, "amdgpu_gfx908");
  DEF_REGISTER(                  acc249, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 249, "amdgpu_gfx908");
  DEF_REGISTER(                  acc250, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 250, "amdgpu_gfx908");
  DEF_REGISTER(                  acc251, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 251, "amdgpu_gfx908");
  DEF_REGISTER(                  acc252, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 252, "amdgpu_gfx908");
  DEF_REGISTER(                  acc253, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 253, "amdgpu_gfx908");
  DEF_REGISTER(                  acc254, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 254, "amdgpu_gfx908");
  DEF_REGISTER(                  acc255, Arch_amdgpu_gfx908 |  ACC_VGPR | BITS_32 | 255, "amdgpu_gfx908");

}}

#endif
