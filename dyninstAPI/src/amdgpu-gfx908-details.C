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

namespace AmdgpuGfx908 {

using namespace Dyninst;

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
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSop1(newRawInst);
  setFixedBitsSop1(newRawInst);
  setOpcodeSop1(opcode, newRawInst);
  setDstSop1(dest, newRawInst);
  setSrc0Sop1(src0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);

  if (hasLiteral) {
    append_memory_as(rawInstBuffer, literal);
  }

  gen.update(rawInstBuffer);
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

  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSop2(newRawInst);
  setOpcodeSop2(opcode, newRawInst);
  setDstSop2(dest, newRawInst);
  setSrc1Sop2(src1, newRawInst);
  setSrc0Sop2(src0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
}

void emitSop2WithSrc1Literal(unsigned opcode, Register dest, Register src0, uint32_t src1Literal,
                             codeGen &gen) {
  emitSop2(opcode, dest, src0, /* src1 = */ 255, gen);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, src1Literal);
  gen.update(rawInstBuffer);
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
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopC(newRawInst);
  setFixedBitsSopC(newRawInst);
  setOpcodeSopC(opcode, newRawInst);
  setSrc1SopC(src1, newRawInst);
  setSrc0SopC(src0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
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
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopK(newRawInst);
  setFixedBitsSopK(newRawInst);
  setOpcodeSopK(opcode, newRawInst);
  setDstSopK(dest, newRawInst);
  setSImm16SopK(simm16, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
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
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopP(newRawInst);
  setFixedBitsSopP(newRawInst);
  setOpcodeSopP(opcode, newRawInst);
  setSImm16SopP(simm16, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
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
  uint64_t newRawInst = 0xFFFFFFFFFFFFFFFF;

  setEncodingSmem(newRawInst);
  setOpcodeSmem(opcode, newRawInst);
  setImmSmem(getSmemImmBit(opcode), newRawInst);
  setGlcSmem(0, newRawInst);
  setNvSmem(0, newRawInst);
  setSoeSmem(0, newRawInst);
  setR1Smem(newRawInst);
  setSdataSmem(sdata, newRawInst);
  setSbaseSmem(sbase, newRawInst);
  setSoffsetSmem(0, newRawInst);
  setR4Smem(newRawInst);
  setOffsetSmem(offset, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
}
// === SMEM END ===

} // namespace AmdgpuGfx908
