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

#include <bitset>
#include <iostream>

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

// === VOP1 BEGIN ===
void setEncodingVop1(uint32_t &rawInst) {
  uint32_t mask = Mask_Vop1_Encoding;
  // Encoding bit must be 0, so no need to set a value.
  rawInst = (rawInst & ~mask);
}

void setFixedBitsVop1(uint32_t &rawInst) {
  uint32_t mask = Mask_Vop1_FixedBits;
  rawInst = (rawInst & ~mask) | ((uint32_t)(0b111111 << 25));
}

void setVdstVop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop1_Vdst;
  rawInst = (rawInst & ~mask) | ((uint32_t)(value << 17) & mask);
}

void setOpcodeVop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop1_Opcode;
  rawInst = (rawInst & ~mask) | ((uint32_t)(value << 9) & mask);
}

void setSrc0Vop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop1_Src0;
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitVop1(unsigned opcode, Register vdst, Register src0, bool hasLiteral, uint32_t literal,
              codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingVop1(newRawInst);
  setFixedBitsVop1(newRawInst);
  setOpcodeVop1(opcode, newRawInst);
  setVdstVop1(vdst, newRawInst);
  setSrc0Vop1(src0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);

  if (hasLiteral) {
    append_memory_as(rawInstBuffer, literal);
  }

  gen.update(rawInstBuffer);
}
// === VOP1 END ===

// === VOP2 BEGIN ===
void setEncodingVop2(uint32_t &rawInst) {
  uint32_t mask = Mask_Vop2_Encoding;
  // Encoding bit must be 0, so no need to set a value.
  rawInst = (rawInst & ~mask);
}

void setOpcodeVop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop2_Opcode;
  rawInst = (rawInst & ~mask) | ((value << 25) & mask);
}

void setVdstVop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop2_Vdst;
  rawInst = (rawInst & ~mask) | ((value << 17) & mask);
}

void setVsrc1Vop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop2_Vsrc1;
  rawInst = (rawInst & ~mask) | ((value << 9) & mask);
}

void setSrc0Vop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = Mask_Vop2_Src0;
  rawInst = (rawInst & ~mask) | (value);
}

void emitVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0,
    codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingVop2(newRawInst);
  setOpcodeVop2(opcode, newRawInst);
  setVdstVop2(vdst, newRawInst);
  setVsrc1Vop2(vsrc1, newRawInst);
  setSrc0Vop2(src0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
}

void emitVop2WithSrc0Literal(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0Literal,
                             codeGen &gen) {
  emitVop2(opcode, vdst, vsrc1, /* src0*/ 255, gen);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, src0Literal);
  gen.update(rawInstBuffer);
}
// === VOP2 END ===

// === VOP3A BEGIN ===
void setEncodingVop3a(uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Encoding;
  rawInst = (rawInst & ~mask) | (((uint64_t)(0b110100) << 26) & mask);
}

void setOpcodeVop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Opcode;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 16) & mask);
}

void setClmpVop3a(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Clmp;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 15) & mask);
}

void setOpSelVop3a(uint32_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_OpSel;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 11) & mask);
}

void setAbsVop3a(uint32_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Abs;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 8) & mask);
}

void setVdstVop3a(uint32_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Vdst;
  rawInst = (rawInst & ~mask) | ((uint64_t)(value) & mask);
}

void setSrc0Vop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Src0;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 32) & mask);
}

void setSrc1Vop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Src1;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 41) & mask);
}

void setSrc2Vop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Src2;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 50) & mask);
}

void setOmodVop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Omod;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 59) & mask);
}

void setNegVop3a(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Vop3a_Neg;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 61) & mask);
}

void emitVop3a(unsigned opcode, uint64_t vdst, uint64_t src0, uint64_t src1, uint64_t src2,
    codeGen &gen) {
  uint64_t newRawInst = 0xFFFFFFFFFFFFFFFF;
  setEncodingVop3a(newRawInst);
  setOpcodeVop3a(opcode, newRawInst);
  setClmpVop3a(0, newRawInst);
  setOpSelVop3a(0, newRawInst);
  setAbsVop3a(0, newRawInst);
  setVdstVop3a(vdst, newRawInst);
  setSrc0Vop3a(src0, newRawInst);
  setSrc1Vop3a(src1, newRawInst);
  setSrc2Vop3a(src2, newRawInst);
  setOmodVop3a(0, newRawInst);
  setNegVop3a(0, newRawInst);

  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
}
// === VOP3A END ===

// === FLAT BEGIN ===

void setEncodingFlat(uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Encoding;
  rawInst = (rawInst & ~mask) | (((uint64_t)(0b110111) << 26) & mask);
}

void setFixedBitsFlat(uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_FixedBits;
  rawInst = (rawInst & ~mask) | (((uint64_t)(1) << 25) & mask);
}

void setOpcodeFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Opcode;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 18) & mask);
}

void setSlcFlat(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Glc;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 17) & mask);
}

void setGlcFlat(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Glc;
  rawInst = (rawInst & ~mask) | (((uint64_t)(value) << 16) & mask);
}

void setSegFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Seg;
  rawInst = (rawInst & ~mask) | ((value << 14) & mask);
}

void setLdsFlat(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Lds;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 13) & mask);
}

void setOffsetFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Offset;
  rawInst = (rawInst & ~mask) | (value & mask);
}

void setAddrFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Addr;
  rawInst = (rawInst & ~mask) | ((value << 32) & mask);
}

void setDataFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Data;
  rawInst = (rawInst & ~mask) | ((value << 40) & mask);
}

void setSaddrFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Saddr;
  rawInst = (rawInst & ~mask) | ((value << 48) & mask);
}

void setNvFlat(bool value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Nv;
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 55) & mask);
}

void setVdstFlat(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = Mask_Flat_Vdst;
  rawInst = (rawInst & ~mask) | ((value << 56) & mask);
}

void emitFlat(unsigned opcode, uint64_t vdst, uint64_t saddr, uint64_t data, uint64_t addr,
              codeGen &gen) {

  std::cerr << "emitFlat\n";
  std::cerr << std::hex << "opcode = " << opcode << " vdst = " << vdst << " saddr = " << saddr << " data = " << data << " addr = " << addr << "\n";

  uint64_t newRawInst = 0xFFFFFFFFFFFFFFFF;
  setEncodingFlat(newRawInst);
  std::cerr << "after encoding : " << newRawInst << '\n';
  setFixedBitsFlat(newRawInst);
  std::cerr << "after fixed bits : " << newRawInst << '\n';
  setOpcodeFlat(opcode, newRawInst);
  std::cerr << "after opcode : " << newRawInst << '\n';
  setSlcFlat(0, newRawInst);
  std::cerr << "after slc : " << newRawInst << '\n';
  setGlcFlat(0, newRawInst);
  std::cerr << "after glc : " << newRawInst << '\n';
  // We only care about global memory instructions, hence seg = 2
  setSegFlat(2, newRawInst);
  std::cerr << "after seg : " << newRawInst << '\n';
  // As mentioned above, we only care for global memory instructions, so LDS = 0
  setLdsFlat(0, newRawInst);
  std::cerr << "after lds : " << newRawInst << '\n';
  setOffsetFlat(0, newRawInst);
  std::cerr << "after offset : " << newRawInst << '\n';

  setVdstFlat(vdst, newRawInst);
  std::cerr << "after vdst : " << newRawInst << '\n';
  setNvFlat(0, newRawInst);
  std::cerr << "after nv : " << newRawInst << '\n';
  setSaddrFlat(saddr, newRawInst);
  std::cerr << "after saddr : " << newRawInst << '\n';
  setDataFlat(data, newRawInst);
  std::cerr << "after data : " << newRawInst << '\n';
  setAddrFlat(addr, newRawInst);
  std::cerr << "after Addr : " << newRawInst << '\n' << '\n';
  auto rawInstBuffer = gen.cur_ptr();
  append_memory_as(rawInstBuffer, newRawInst);
  gen.update(rawInstBuffer);
}

// === FLAT END ===
} // namespace AmdgpuGfx908
