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
#include "registers/AMDGPU/amdgpu_gfx940_regs.def"

}}

#endif
