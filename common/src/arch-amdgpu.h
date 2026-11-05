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

#ifndef _ARCH_AMDGPU_H
#define _ARCH_AMDGPU_H

// We primarily use the emitter for AMDGPU.
// This is there to make the port build.

#include "dyntypes.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include "registers/AMDGPU/amdgpu_gfx950_regs.h"
#include "dyn_register.h"
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_amdgpu {
// AMDGPU GFX908 registers of interest for code generation. We will use liveness on these to
// allocate them for use in instrumentation.
// Note that this is for Dyninst's abstraction for code generation.
// The register ids are actual register numbers that are used in instructions. Since SGPRs and VGPRs have
// the same register numbers, they need to be distinguished.
//
// We map these 'Register's to MachRegisters in RegisterConversion-amdgpu.C
// For information on architectural registers see MachRegister in dyn_regs.h

using namespace Dyninst;

constexpr Alignment PAIR_ALIGNMENT(2);
constexpr Alignment QUAD_ALIGNMENT(4);

constexpr uint32_t MIN_SGPR_ID = 0;
constexpr uint32_t MAX_SGPR_ID = 101;

// Since we max out register allocation for GFX908 kernels in the kernel descriptor,
// vcc will alias with s[100:101] as it aliases to the last allocated SGPR pair.

// We don't want these registers to be used in instrumentation code.
constexpr uint32_t MAX_ALLOCATABLE_SGPR_ID = MAX_SGPR_ID - 2;

constexpr uint32_t MIN_VGPR_ID = 0;
constexpr uint32_t MAX_VGPR_ID = 255;


typedef const unsigned int insn_mask;

typedef union {
  unsigned char byte[4];
  unsigned int raw;
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

class DYNINST_EXPORT instruction {
private:
  instructUnion insn_;

public:
  instruction(const void *ptr, bool) { insn_ = *((const instructUnion *)ptr); }

  unsigned int asInt() const { return insn_.raw; }

  static unsigned size() { return sizeof(instructUnion); }
};

} // namespace NS_amdgpu
// end of NS_amdgpu

#endif
