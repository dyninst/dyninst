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

  //          (                    name,  ID | alias   |      cat  |              arch,           arch )
  DEF_REGISTER(                     tid,   0 | BITS_32 |    SYSREG |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                 invalid,   1 | BITS_32 |    SYSREG |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  pc_all,   0 | BITS_48 |        PC |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                 src_scc,   0 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                src_vccz,   1 |  BITS_1 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  vcc_lo,   2 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  vcc_hi,   3 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     vcc,   2 | BITS_64 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(               src_execz,   4 |  BITS_1 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 exec_lo,   5 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 exec_hi,   6 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    exec,   5 | BITS_64 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(         flat_scratch_lo,   7 | BITS_64 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(         flat_scratch_hi,   8 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(        flat_scratch_all,   7 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                      m0,  10 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(             src_literal,  11 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");//TODO
  DEF_REGISTER(src_pops_exiting_wave_id,  12 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");//TODO

  DEF_REGISTER(        src_private_base,  13 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(       src_private_limit,  14 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(         src_shared_base,  15 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(        src_shared_limit,  16 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(           xnack_mask_lo,  17 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(           xnack_mask_hi,  18 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(          src_lds_direct,  19 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   vmcnt,  20 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  expcnt,  21 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 lgkmcnt,  22 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   dsmem,  23 | BITS_32 |       HWR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                   ttmp0,   0 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp1,   1 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp2,   2 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp3,   3 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp4,   4 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp5,   5 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp6,   6 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp7,   7 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp8,   8 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   ttmp9,   9 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp10,  10 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp11,  11 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp12,  12 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp13,  13 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp14,  14 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  ttmp15,  15 | BITS_32 | TTMP_SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                    mrt0,   0 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt1,   1 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt2,   2 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt3,   3 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt4,   4 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt5,   5 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt6,   6 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrt7,   7 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    mrtz,   8 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    null,   9 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    pos0,  12 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    pos1,  13 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    pos2,  14 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    pos3,  15 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param0,  32 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param1,  33 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param2,  34 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param3,  35 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param4,  36 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param5,  37 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param6,  38 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param7,  39 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param8,  40 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  param9,  41 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param10,  42 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param11,  43 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param12,  44 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param13,  45 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param14,  46 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param15,  47 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param16,  48 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param17,  49 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param18,  50 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param19,  51 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param20,  52 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param21,  53 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param22,  54 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param23,  55 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param24,  56 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param25,  57 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param26,  58 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param27,  59 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param28,  60 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param29,  61 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param30,  62 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                 param31,  63 | BITS_32 |       TGT |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                   attr0,   0 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr1,   1 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr2,   2 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr3,   3 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr4,   4 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr5,   5 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr6,   6 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr7,   7 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr8,   8 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   attr9,   9 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr10,  10 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr11,  11 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr12,  12 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr13,  13 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr14,  14 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr15,  15 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr16,  16 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr17,  17 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr18,  18 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr19,  19 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr20,  20 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr21,  21 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr22,  22 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr23,  23 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr24,  24 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr25,  25 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr26,  26 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr27,  27 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr28,  28 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr29,  29 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr30,  30 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr31,  31 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  attr32,  32 | BITS_32 |      ATTR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                     p10,   0 | BITS_32 |     PARAM |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     p20,   1 | BITS_32 |     PARAM |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      p0,   2 | BITS_32 |     PARAM |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                   idxen,   0 |  BITS_1 |      INFO |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   offen,   1 |  BITS_1 |      INFO |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     off,   2 |  BITS_1 |      INFO |Arch_amdgpu_gfx940, "amdgpu_gfx940");

  DEF_REGISTER(                      s0,   0 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s1,   1 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s2,   2 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s3,   3 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s4,   4 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s5,   5 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s6,   6 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s7,   7 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s8,   8 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      s9,   9 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s10,  10 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s11,  11 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s12,  12 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s13,  13 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s14,  14 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s15,  15 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s16,  16 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s17,  17 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s18,  18 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s19,  19 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s20,  20 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s21,  21 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s22,  22 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s23,  23 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s24,  24 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s25,  25 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s26,  26 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s27,  27 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s28,  28 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s29,  29 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s30,  30 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s31,  31 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s32,  32 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s33,  33 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s34,  34 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s35,  35 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s36,  36 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s37,  37 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s38,  38 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s39,  39 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s40,  40 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s41,  41 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s42,  42 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s43,  43 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s44,  44 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s45,  45 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s46,  46 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s47,  47 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s48,  48 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s49,  49 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s50,  50 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s51,  51 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s52,  52 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s53,  53 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s54,  54 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s55,  55 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s56,  56 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s57,  57 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s58,  58 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s59,  59 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s60,  60 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s61,  61 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s62,  62 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s63,  63 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s64,  64 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s65,  65 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s66,  66 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s67,  67 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s68,  68 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s69,  69 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s70,  70 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s71,  71 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s72,  72 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s73,  73 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s74,  74 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s75,  75 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s76,  76 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s77,  77 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s78,  78 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s79,  79 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s80,  80 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s81,  81 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s82,  82 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s83,  83 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s84,  84 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s85,  85 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s86,  86 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s87,  87 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s88,  88 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s89,  89 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s90,  90 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s91,  91 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s92,  92 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s93,  93 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s94,  94 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s95,  95 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s96,  96 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s97,  97 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s98,  98 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     s99,  99 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    s100, 100 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    s101, 101 | BITS_32 |      SGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v0,   0 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v1,   1 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v2,   2 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v3,   3 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v4,   4 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v5,   5 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v6,   6 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v7,   7 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v8,   8 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                      v9,   9 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v10,  10 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v11,  11 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v12,  12 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v13,  13 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v14,  14 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v15,  15 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v16,  16 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v17,  17 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v18,  18 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v19,  19 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v20,  20 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v21,  21 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v22,  22 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v23,  23 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v24,  24 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v25,  25 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v26,  26 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v27,  27 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v28,  28 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v29,  29 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v30,  30 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v31,  31 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v32,  32 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v33,  33 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v34,  34 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v35,  35 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v36,  36 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v37,  37 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v38,  38 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v39,  39 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v40,  40 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v41,  41 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v42,  42 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v43,  43 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v44,  44 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v45,  45 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v46,  46 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v47,  47 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v48,  48 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v49,  49 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v50,  50 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v51,  51 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v52,  52 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v53,  53 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v54,  54 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v55,  55 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v56,  56 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v57,  57 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v58,  58 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v59,  59 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v60,  60 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v61,  61 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v62,  62 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v63,  63 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v64,  64 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v65,  65 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v66,  66 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v67,  67 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v68,  68 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v69,  69 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v70,  70 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v71,  71 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v72,  72 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v73,  73 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v74,  74 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v75,  75 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v76,  76 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v77,  77 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v78,  78 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v79,  79 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v80,  80 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v81,  81 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v82,  82 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v83,  83 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v84,  84 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v85,  85 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v86,  86 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v87,  87 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v88,  88 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v89,  89 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v90,  90 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v91,  91 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v92,  92 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v93,  93 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v94,  94 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v95,  95 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v96,  96 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v97,  97 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v98,  98 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                     v99,  99 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v100, 100 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v101, 101 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v102, 102 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v103, 103 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v104, 104 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v105, 105 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v106, 106 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v107, 107 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v108, 108 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v109, 109 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v110, 110 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v111, 111 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v112, 112 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v113, 113 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v114, 114 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v115, 115 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v116, 116 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v117, 117 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v118, 118 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v119, 119 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v120, 120 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v121, 121 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v122, 122 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v123, 123 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v124, 124 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v125, 125 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v126, 126 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v127, 127 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v128, 128 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v129, 129 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v130, 130 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v131, 131 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v132, 132 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v133, 133 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v134, 134 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v135, 135 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v136, 136 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v137, 137 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v138, 138 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v139, 139 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v140, 140 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v141, 141 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v142, 142 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v143, 143 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v144, 144 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v145, 145 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v146, 146 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v147, 147 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v148, 148 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v149, 149 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v150, 150 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v151, 151 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v152, 152 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v153, 153 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v154, 154 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v155, 155 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v156, 156 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v157, 157 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v158, 158 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v159, 159 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v160, 160 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v161, 161 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v162, 162 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v163, 163 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v164, 164 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v165, 165 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v166, 166 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v167, 167 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v168, 168 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v169, 169 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v170, 170 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v171, 171 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v172, 172 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v173, 173 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v174, 174 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v175, 175 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v176, 176 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v177, 177 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v178, 178 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v179, 179 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v180, 180 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v181, 181 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v182, 182 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v183, 183 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v184, 184 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v185, 185 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v186, 186 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v187, 187 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v188, 188 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v189, 189 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v190, 190 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v191, 191 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v192, 192 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v193, 193 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v194, 194 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v195, 195 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v196, 196 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v197, 197 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v198, 198 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v199, 199 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v200, 200 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v201, 201 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v202, 202 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v203, 203 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v204, 204 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v205, 205 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v206, 206 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v207, 207 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v208, 208 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v209, 209 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v210, 210 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v211, 211 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v212, 212 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v213, 213 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v214, 214 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v215, 215 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v216, 216 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v217, 217 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v218, 218 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v219, 219 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v220, 220 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v221, 221 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v222, 222 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v223, 223 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v224, 224 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v225, 225 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v226, 226 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v227, 227 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v228, 228 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v229, 229 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v230, 230 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v231, 231 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v232, 232 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v233, 233 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v234, 234 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v235, 235 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v236, 236 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v237, 237 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v238, 238 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v239, 239 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v240, 240 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v241, 241 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v242, 242 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v243, 243 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v244, 244 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v245, 245 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v246, 246 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v247, 247 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v248, 248 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v249, 249 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v250, 250 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v251, 251 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v252, 252 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v253, 253 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v254, 254 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    v255, 255 | BITS_32 |      VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc0,   0 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc1,   1 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc2,   2 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc3,   3 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc4,   4 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc5,   5 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc6,   6 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc7,   7 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc8,   8 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                    acc9,   9 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc10,  10 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc11,  11 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc12,  12 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc13,  13 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc14,  14 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc15,  15 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc16,  16 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc17,  17 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc18,  18 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc19,  19 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc20,  20 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc21,  21 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc22,  22 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc23,  23 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc24,  24 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc25,  25 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc26,  26 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc27,  27 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc28,  28 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc29,  29 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc30,  30 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc31,  31 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc32,  32 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc33,  33 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc34,  34 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc35,  35 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc36,  36 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc37,  37 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc38,  38 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc39,  39 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc40,  40 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc41,  41 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc42,  42 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc43,  43 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc44,  44 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc45,  45 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc46,  46 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc47,  47 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc48,  48 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc49,  49 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc50,  50 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc51,  51 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc52,  52 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc53,  53 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc54,  54 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc55,  55 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc56,  56 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc57,  57 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc58,  58 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc59,  59 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc60,  60 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc61,  61 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc62,  62 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc63,  63 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc64,  64 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc65,  65 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc66,  66 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc67,  67 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc68,  68 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc69,  69 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc70,  70 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc71,  71 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc72,  72 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc73,  73 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc74,  74 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc75,  75 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc76,  76 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc77,  77 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc78,  78 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc79,  79 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc80,  80 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc81,  81 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc82,  82 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc83,  83 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc84,  84 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc85,  85 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc86,  86 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc87,  87 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc88,  88 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc89,  89 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc90,  90 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc91,  91 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc92,  92 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc93,  93 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc94,  94 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc95,  95 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc96,  96 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc97,  97 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc98,  98 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                   acc99,  99 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc100, 100 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc101, 101 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc102, 102 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc103, 103 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc104, 104 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc105, 105 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc106, 106 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc107, 107 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc108, 108 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc109, 109 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc110, 110 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc111, 111 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc112, 112 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc113, 113 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc114, 114 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc115, 115 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc116, 116 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc117, 117 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc118, 118 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc119, 119 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc120, 120 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc121, 121 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc122, 122 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc123, 123 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc124, 124 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc125, 125 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc126, 126 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc127, 127 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc128, 128 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc129, 129 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc130, 130 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc131, 131 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc132, 132 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc133, 133 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc134, 134 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc135, 135 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc136, 136 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc137, 137 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc138, 138 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc139, 139 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc140, 140 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc141, 141 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc142, 142 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc143, 143 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc144, 144 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc145, 145 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc146, 146 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc147, 147 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc148, 148 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc149, 149 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc150, 150 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc151, 151 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc152, 152 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc153, 153 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc154, 154 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc155, 155 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc156, 156 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc157, 157 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc158, 158 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc159, 159 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc160, 160 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc161, 161 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc162, 162 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc163, 163 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc164, 164 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc165, 165 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc166, 166 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc167, 167 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc168, 168 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc169, 169 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc170, 170 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc171, 171 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc172, 172 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc173, 173 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc174, 174 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc175, 175 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc176, 176 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc177, 177 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc178, 178 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc179, 179 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc180, 180 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc181, 181 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc182, 182 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc183, 183 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc184, 184 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc185, 185 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc186, 186 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc187, 187 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc188, 188 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc189, 189 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc190, 190 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc191, 191 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc192, 192 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc193, 193 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc194, 194 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc195, 195 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc196, 196 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc197, 197 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc198, 198 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc199, 199 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc200, 200 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc201, 201 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc202, 202 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc203, 203 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc204, 204 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc205, 205 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc206, 206 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc207, 207 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc208, 208 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc209, 209 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc210, 210 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc211, 211 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc212, 212 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc213, 213 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc214, 214 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc215, 215 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc216, 216 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc217, 217 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc218, 218 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc219, 219 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc220, 220 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc221, 221 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc222, 222 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc223, 223 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc224, 224 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc225, 225 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc226, 226 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc227, 227 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc228, 228 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc229, 229 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc230, 230 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc231, 231 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc232, 232 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc233, 233 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc234, 234 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc235, 235 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc236, 236 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc237, 237 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc238, 238 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc239, 239 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc240, 240 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc241, 241 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc242, 242 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc243, 243 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc244, 244 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc245, 245 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc246, 246 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc247, 247 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc248, 248 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc249, 249 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc250, 250 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc251, 251 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc252, 252 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc253, 253 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc254, 254 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");
  DEF_REGISTER(                  acc255, 255 | BITS_32 |  ACC_VGPR |Arch_amdgpu_gfx940, "amdgpu_gfx940");

}}

#endif
