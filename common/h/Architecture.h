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

#ifndef DYNINST_ARCHITECTURE_H
#define DYNINST_ARCHITECTURE_H

#include <cassert>

namespace Dyninst {

  // 0xff000000 is used to encode architecture
  typedef enum {
    Arch_none = 0x00000000,
    Arch_x86 = 0x14000000,
    Arch_x86_64 = 0x18000000,
    Arch_ppc32 = 0x24000000,
    Arch_ppc64 = 0x28000000,
    Arch_aarch32 = 0x44000000, // for later use
    Arch_aarch64 = 0x48000000,
    Arch_cuda = 0x88000000,
    Arch_amdgpu_gfx908 = 0x94000000, // future support for gfx908
    Arch_amdgpu_gfx90a = 0x98000000, // future support for gfx90a
    Arch_amdgpu_gfx940 = 0x9c000000, // future support for gfx940
    Arch_intelGen9 = 0xb6000000      // same as machine no. retrevied from eu-readelf
  } Architecture;

  inline unsigned getArchAddressWidth(Architecture arch) {
    switch(arch) {
      case Arch_none: return 0;
      case Arch_x86:
      case Arch_ppc32: return 4;
      case Arch_x86_64:
      case Arch_ppc64:
      case Arch_aarch64:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940: return 8;
      default: assert(0); return 0;
    }
    return 0;
  }

}

#endif
