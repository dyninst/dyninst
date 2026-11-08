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

/*
 * emit-amdgpu-gfx908.C - the gfx908 concrete emitter: this gen's ENC-typed packers.
 * All arch-common codegen lives in the EmitterAmdgpu base (emit-amdgpu.C).
 */

#include "emit-amdgpu-gfx908.h"
#include "amdgpu-gfx908-details.h"

using namespace Dyninst;

// Thin per-gen packers (forward to gfx908's enc-typed helpers).
// A sibling gen (EmitterAmdgpuGfx942) is a sed of this file: amdgpu_gfx908 -> amdgpu_gfx942.
void EmitterAmdgpuGfx908::rawSop1(unsigned opcode, Register dest, Register src0, bool hasLiteral,
                                  uint32_t literal, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSop1(opcode, dest, src0, hasLiteral, literal, gen);
}
void EmitterAmdgpuGfx908::rawSop2(unsigned opcode, Register dest, Register src1, Register src0,
                                  codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSop2(opcode, dest, src1, src0, gen);
}
void EmitterAmdgpuGfx908::rawSop2WithSrc1Literal(unsigned opcode, Register dest, Register src0,
                                                 uint32_t literal, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSop2WithSrc1Literal(opcode, dest, src0, literal, gen);
}
void EmitterAmdgpuGfx908::rawSopK(unsigned opcode, Register dest, int16_t simm16, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSopK(opcode, dest, simm16, gen);
}
void EmitterAmdgpuGfx908::rawSopC(unsigned opcode, Register src0, Register src1, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSopC(opcode, src0, src1, gen);
}
void EmitterAmdgpuGfx908::rawSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset,
                                  codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSmem(opcode, sdata, sbase, offset, gen);
}
