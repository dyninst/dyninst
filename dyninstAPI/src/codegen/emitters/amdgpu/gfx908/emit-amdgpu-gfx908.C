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
 * emit-amdgpu-gfx908.C - the gfx908 concrete emitter: this gen's arch + its ENC-typed packers.
 * All arch-common codegen lives in the EmitterAmdgpu base (emit-amdgpu.C).
 */

#include "emit-amdgpu-gfx908.h"
#include "amdgpu-gfx908-details.h"
#include "amdgpu-gfx908-encoding.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

using namespace Dyninst;

// Adapt a codegen Register to a gfx908 MachRegister: number from the id, category from vector-ness, width
// from the dword count -- all via amdgpu_gfx908's own namespace constants (no macro, no arch switch; the
// base names no gen). A sibling gen is a sed of this file: amdgpu_gfx908 -> amdgpu_gfx940.
MachRegister EmitterAmdgpuGfx908::toMachRegister(Register r) const {
  const int32_t num = static_cast<int32_t>(r.getId() & 0xffu);
  const int32_t category = r.isVector() ? amdgpu_gfx908::VGPR : amdgpu_gfx908::SGPR;
  int32_t sizeBits = amdgpu_gfx908::BITS_32;
  switch (r.getCount()) {
  case 2: sizeBits = amdgpu_gfx908::BITS_64; break;
  case 4: sizeBits = amdgpu_gfx908::BITS_128; break;
  case 8: sizeBits = amdgpu_gfx908::BITS_256; break;
  case 16: sizeBits = amdgpu_gfx908::BITS_512; break;
  default: break; // 1 (and unexpected) -> BITS_32
  }
  return MachRegister(static_cast<int32_t>(Arch_amdgpu_gfx908) | category | sizeBits | num);
}

// Map each gen-neutral AmdgpuOp to this gen's generated opcode value (amdgpu_gfx908::op::<FORMAT>::<NAME>).
// The DYNINST_AMDGPU_OPS list (emit-amdgpu.h) is the single source; a missing or renamed op in this gen's
// generated header fails to compile here (-Wswitch), which is the intended loud failure.
uint16_t EmitterAmdgpuGfx908::toOpCode(AmdgpuOp o) const {
  using namespace Dyninst::amdgpu_gfx908::op;
  switch (o) {
#define DYNINST_AMDGPU_OP_CASE(fmt, name) case AmdgpuOp::name: return fmt::name;
    DYNINST_AMDGPU_OPS(DYNINST_AMDGPU_OP_CASE)
#undef DYNINST_AMDGPU_OP_CASE
  }
  assert(!"EmitterAmdgpuGfx908::toOpCode: unhandled AmdgpuOp");
  return 0;
}

// Thin per-gen packers (forward to gfx908's detail-typed helpers).
// A sibling gen (EmitterAmdgpuGfx940) is a sed of this file: amdgpu_gfx908 -> amdgpu_gfx940.
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
void EmitterAmdgpuGfx908::rawSopP(unsigned opcode, int16_t simm16, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSopP(opcode, simm16, gen);
}
void EmitterAmdgpuGfx908::rawSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset,
                                  codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitSmem(opcode, sdata, sbase, offset, gen);
}
void EmitterAmdgpuGfx908::rawVop1Reg(unsigned opcode, uint32_t vdst, uint32_t src0, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitVop1Reg(opcode, vdst, src0, gen);
}
void EmitterAmdgpuGfx908::rawVop1Imm(unsigned opcode, uint32_t vdst, uint32_t literal, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitVop1Imm(opcode, vdst, literal, gen);
}
void EmitterAmdgpuGfx908::rawVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitVop2(opcode, vdst, vsrc1, src0, gen);
}
void EmitterAmdgpuGfx908::rawVop3a(unsigned opcode, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &gen) {
  Dyninst::amdgpu_gfx908::emitVop3a(opcode, vdst, src0, src1, src2, gen);
}
void EmitterAmdgpuGfx908::rawFlatGlobal(unsigned opcode, uint32_t vdst, uint32_t addr, uint32_t data, uint32_t saddr,
                       int32_t offset, codeGen &gen, bool glc, uint32_t seg) {
  Dyninst::amdgpu_gfx908::emitFlatGlobal(opcode, vdst, addr, data, saddr, offset, gen, glc, seg);
}
