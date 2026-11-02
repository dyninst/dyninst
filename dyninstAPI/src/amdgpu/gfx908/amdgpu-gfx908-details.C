/*
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

#include "amdgpu-gfx908-details.h"
#include "unaligned_memory_access.h"
#include "amdgpu-gfx908-encoding.h"
#include "amdgpu-encode.hpp"

namespace Dyninst {
namespace amdgpu_gfx908 {

using namespace Dyninst;

// The emit* primitives below pack instruction words through a generated, table-driven encoder
// (packWordU64 over the ISA format tables in amdgpu-gfx908-encoding.h, namespace detail) rather than
// open-coded bit layouts, so field positions come from the ISA description instead of hand-maintained
// masks. The packed word is appended with codeGen::copy. The legacy set*/ContentMask bit helpers are
// retained for reference; the emit* functions no longer use them.

// === SOP1 BEGIN ===
void setEncodingSop1(uint32_t &rawInst) {
  uint32_t mask = Mask_Sop1_Encoding;
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSop1(uint32_t &rawInst) {
  uint32_t mask = Mask_Sop1_FixedBits;
  //                            ((0b1111101 << 23));
  rawInst = (rawInst & ~mask) | ((0x0000007D << 23));
}

void setDstSop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop1_Dst;
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setOpcodeSop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop1_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}

void setSrc0Sop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop1_Src0;
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSop1(unsigned opcode, Register dest, Register src0, bool hasLiteral, uint32_t literal,
              codeGen &gen) {
  uint32_t w = (uint32_t)Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SOP1, detail::NO_FIELD, opcode, {{detail::SDST, dest}, {detail::SSRC0, src0}});
  gen.copy(&w, sizeof w);
  if (hasLiteral)
    gen.copy(&literal, sizeof literal);
}
// === SOP1 END ===

// === SOP2 BEGIN ===
void setEncodingSop2(uint32_t &rawInst) {
  uint32_t mask = Mask_Sop2_Encoding;
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setOpcodeSop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop2_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 23) & mask);
}

void setDstSop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop2_Dst;
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}
void setSrc1Sop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop2_Src1;
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}
void setSrc0Sop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Sop2_Src0;
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSop2(unsigned opcode, Register dest, Register src0, Register src1, codeGen &gen) {
  // Source operand being 255 means the instruction is followed by a literal.
  // AmdgpuGfx908 supports only one literal operand in SOP2.
  // Thus src0 and src1 can't be 255 at the same time.
  assert(!(src0 == 255 && src1 == 255));
  uint32_t w = (uint32_t)Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SOP2, detail::NO_FIELD, opcode,
                                     {{detail::SDST, dest}, {detail::SSRC0, src0}, {detail::SSRC1, src1}});
  gen.copy(&w, sizeof w);
}

void emitSop2WithSrc1Literal(unsigned opcode, Register dest, Register src0, uint32_t src1Literal,
                             codeGen &gen) {
  emitSop2(opcode, dest, src0, /* src1 = */ 255, gen);
  gen.copy(&src1Literal, sizeof src1Literal);
}
// === SOP2 END ===

// === SOPC BEGIN ===
void setEncodingSopC(uint32_t &rawInst) {
  uint32_t mask = Mask_SopC_Encoding;
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSopC(uint32_t &rawInst) {
  uint32_t mask = Mask_SopC_FixedBits;
  //                            ((0b1111110 << 23));
  rawInst = (rawInst & ~mask) | ((0x7E << 23));
}

void setOpcodeSopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopC_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSrc1SopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopC_Src1;
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}
void setSrc0SopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopC_Src0;
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSopC(unsigned opcode, Register src0, Register src1, codeGen &gen) {
  uint32_t w = (uint32_t)Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SOPC, detail::NO_FIELD, opcode, {{detail::SSRC0, src0}, {detail::SSRC1, src1}});
  gen.copy(&w, sizeof w);
}
// === SOPC END ===

// === SOPK BEGIN ===
void setEncodingSopK(uint32_t &rawInst) {
  uint32_t mask = Mask_SopK_Encoding;
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSopK(uint32_t &rawInst) {
  uint32_t mask = Mask_SopK_FixedBits;
  //                            ((0b11 << 28));
  rawInst = (rawInst & ~mask) | ((0x00000003 << 28));
}

void setOpcodeSopK(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopK_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 23) & mask);
}

void setDstSopK(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopK_Dst;
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSImm16SopK(int16_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopK_SImm16;
  rawInst = (rawInst & ~mask) | ((uint32_t)(value)&mask);
}

void emitSopK(unsigned opcode, Register dest, int16_t simm16, codeGen &gen) {
  uint32_t w = (uint32_t)Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SOPK, detail::NO_FIELD, opcode,
                                     {{detail::SDST, dest}, {detail::SIMM16, (uint32_t)(uint16_t)simm16}});
  gen.copy(&w, sizeof w);
}
// === SOPK END ===

// === SOPP BEGIN ===
void setEncodingSopP(uint32_t &rawInst) {
  uint32_t mask = Mask_SopP_Encoding;
  rawInst = (rawInst & ~mask) | ((0x00000001 << 31));
}

void setFixedBitsSopP(uint32_t &rawInst) {
  uint32_t mask = Mask_SopP_FixedBits;
  //                            ((0b1111111 << 23));
  rawInst = (rawInst & ~mask) | ((0x7F << 23));
}

void setOpcodeSopP(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopP_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSImm16SopP(int16_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_SopP_SImm16;
  rawInst = (rawInst & ~mask) | ((uint32_t)(value)&mask);
}

void emitSopP(unsigned opcode, int16_t simm16, codeGen &gen) {
  uint32_t w = (uint32_t)Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SOPP, detail::NO_FIELD, opcode, {{detail::SIMM16, (uint32_t)(uint16_t)simm16}});
  gen.copy(&w, sizeof w);
}
// === SOPP END ===

// === SMEM BEGIN ===
unsigned getSmemImmBit(unsigned opcode) {
  switch (opcode) {
  case S_DCACHE_WB:
    return 0;
  default:
    return 1;
  }
}

void setEncodingSmem(uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Encoding;
  //                            (((uint64_t)(0b110000) << 26) & mask);
  rawInst = (rawInst & ~mask) | (((uint64_t)(0x0000000000000030) << 26) & mask);
}

void setOpcodeSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 18) & mask);
}

void setImmSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Imm;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 17) & mask);
}

void setGlcSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Glc;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 16) & mask);
}

void setNvSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Nv;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 15) & mask);
}

void setSoeSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Soe;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 14) & mask);
}

void setR1Smem(uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_R1;
  rawInst = (rawInst & ~mask) | (uint64_t(0) & mask);
}

void setSdataSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Sdata;
  rawInst = (rawInst & ~mask) | ((value << 6) & mask);
}

void setSbaseSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Sbase;
  rawInst = (rawInst & ~mask) | ((value)&mask);
}

void setSoffsetSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Soffset;
  rawInst = (rawInst & ~mask) | ((value << 57) & mask);
}

void setR4Smem(uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_R4;
  rawInst = (rawInst & ~mask) | ((uint64_t(0) << 53) & mask);
}

void setOffsetSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Smem_Offset;
  rawInst = (rawInst & ~mask) | ((value << 32) & mask);
}

void emitSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset, codeGen &gen) {
  // Callers pass `sbase` already halved (the SGPR-pair index); the generated SBASE field halves
  // internally (valueLo=1), so pass sbase<<1 to round-trip to the same 6-bit value. GLC/NV/SOFFSET_EN
  // default to 0 (unset).
  uint64_t w = Dyninst::amdgpu_enc::packWordU64(detail::formats, detail::ENC_SMEM, detail::NO_FIELD, opcode,
                           {{detail::SDATA, (uint32_t)sdata}, {detail::SBASE, (uint32_t)(sbase << 1)},
                            {detail::OFFSET, (uint32_t)offset}, {detail::IMM, getSmemImmBit(opcode)}});
  gen.copy(&w, sizeof w);
}
// === SMEM END ===

// ==== Instrumentation-stack emit primitives (folded from the GPU-instrumentation branch) ====
// These were written against a gfx908-bound packer (amdgpu_pack / amdgpu_gfx908_enc). Here they ride the
// gen-neutral templated packer (amdgpu_enc) over THIS gen's generated `detail` tables via a tiny local
// shim, so the bodies are unchanged and a sibling gen is a plain sed of this file (amdgpu_gfx908 ->
// amdgpu_gfx940). GE/PK name only `detail` (this gen), never a gen literal.
namespace GE = detail;
namespace PK {
  inline uint64_t packWordU64(detail::Format fmt, uint32_t opcode,
                              std::initializer_list<std::pair<detail::Field, uint32_t>> fieldVals) {
    return Dyninst::amdgpu_enc::packWordU64(detail::formats, fmt, detail::NO_FIELD, opcode, fieldVals);
  }
} // namespace PK

static inline void appendWord32(codeGen &gen, uint32_t w) {
  auto p = gen.cur_ptr();
  append_memory_as(p, w);
  gen.update(p);
}
static inline void appendWord64(codeGen &gen, uint64_t w) {
  auto p = gen.cur_ptr();
  append_memory_as(p, w);
  gen.update(p);
}

void emitSop1Raw(unsigned opcode, uint32_t sdst, uint32_t ssrc0, codeGen &gen) {
  appendWord32(gen, (uint32_t)PK::packWordU64(GE::ENC_SOP1, opcode, {{GE::SDST, sdst}, {GE::SSRC0, ssrc0}}));
}
void emitSop2Raw(unsigned opcode, uint32_t sdst, uint32_t ssrc0, uint32_t ssrc1, codeGen &gen) {
  appendWord32(gen, (uint32_t)PK::packWordU64(GE::ENC_SOP2, opcode,
                                              {{GE::SDST, sdst}, {GE::SSRC0, ssrc0}, {GE::SSRC1, ssrc1}}));
}
void emitSop2RawWithLiteral(unsigned opcode, uint32_t sdst, uint32_t ssrc0, uint32_t literal, codeGen &gen) {
  emitSop2Raw(opcode, sdst, ssrc0, /*ssrc1 = literal marker*/ 255, gen);
  auto p = gen.cur_ptr();
  append_memory_as(p, literal);
  gen.update(p);
}

// VOP1 with a trailing 32-bit literal (src0 = 0xFF selects the literal word): e.g. v_mov_b32 vdst, <lit>.
void emitVop1Imm(unsigned opcode, uint32_t vdst, uint32_t literal, codeGen &gen) {
  uint32_t raw = (uint32_t)PK::packWordU64(GE::ENC_VOP1, opcode, {{GE::VDST, vdst}, {GE::SRC0, 0xFF}});
  auto p = gen.cur_ptr();
  append_memory_as(p, raw);
  append_memory_as(p, literal);
  gen.update(p);
}
void emitVop1Reg(unsigned opcode, uint32_t vdst, uint32_t src0, codeGen &gen) {
  appendWord32(gen, (uint32_t)PK::packWordU64(GE::ENC_VOP1, opcode, {{GE::VDST, vdst}, {GE::SRC0, src0}}));
}

// s_mov_b32 m0(=124), <value|reg> ; s_ttracedata  (SQTT/ATT marker).
void emitTTraceData(uint32_t m0_value, codeGen &gen) {
  emitSop1(S_MOV_B32, /*dest=M0*/ 124, /*src0=literal*/ 255, /*hasLiteral=*/true, m0_value, gen);
  emitSopP(S_TTRACEDATA, /*simm16=*/0, gen);
}
void emitTTraceDataReg(uint32_t src_reg, codeGen &gen) {
  emitSop1(S_MOV_B32, /*dest=M0*/ 124, /*src0=*/src_reg, /*hasLiteral=*/false, /*literal=*/0, gen);
  emitSopP(S_TTRACEDATA, /*simm16=*/0, gen);
}

void emitVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &gen) {
  appendWord32(gen, (uint32_t)PK::packWordU64(GE::ENC_VOP2, opcode,
                    {{GE::VDST, vdst}, {GE::VSRC1, vsrc1}, {GE::SRC0, src0}}));
}
void emitVop3a(unsigned opcode, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &gen) {
  // ABS/OPSEL/CLMP/OMOD/NEG default to 0. V_READLANE/WRITELANE ride the 8-bit VDST field.
  appendWord64(gen, PK::packWordU64(GE::ENC_VOP3, opcode,
                    {{GE::VDST, vdst}, {GE::SRC0, src0}, {GE::SRC1, src1}, {GE::SRC2, src2}}));
}
void emitFlatGlobal(unsigned opcode, uint32_t vdst, uint32_t addr, uint32_t data,
                    uint32_t saddr, int32_t offset, codeGen &gen, bool glc, uint32_t seg) {
  // SEG is baked into the format's fixed bits: global (seg=2) -> ENC_FLAT_GLBL, scratch (seg=1) ->
  // ENC_FLAT_SCRATCH. OFFSET is the 13-bit signed field. LDS/SLC/NV default to 0.
  GE::Format fmt = (seg == 1) ? GE::ENC_FLAT_SCRATCH : GE::ENC_FLAT_GLBL;
  appendWord64(gen, PK::packWordU64(fmt, opcode,
                    {{GE::OFFSET, (uint32_t)((uint32_t)offset & 0x1fff)}, {GE::GLC, glc ? 1u : 0u},
                     {GE::ADDR, addr}, {GE::DATA, data}, {GE::SADDR, saddr}, {GE::VDST, vdst}}));
}

} // namespace amdgpu_gfx908
} // namespace Dyninst
