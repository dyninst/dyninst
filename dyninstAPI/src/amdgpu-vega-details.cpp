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

#include "amdgpu-vega-details.h"

static void printBinary(unsigned int number) {
  if (number >> 1) {
    printBinary(number >> 1);
  }
  putc((number & 1) ? '1' : '0', stdout);
}

static void printBytes(unsigned int number) {
  uint8_t *n = (uint8_t *)&number;
  printf("%u ", n[0]);
  printf("%u ", n[1]);
  printf("%u ", n[2]);
  printf("%u \n", n[3]);
}

static void printBytes(uint64_t number) {
  uint8_t *n = (uint8_t *)&number;
  for (int i = 0; i < 8; ++i) {
    printf("%u ", n[i]);
  }
}

namespace Vega {

// === SOP1 BEGIN ===
uint32_t getMaskSop1(ContentKind k) {
  switch (k) {
  case CK_Sop1_Encoding:
    return 0b11000000000000000000000000000000;
  case CK_Sop1_FixedBits:
    return 0b00111111100000000000000000000000;
  case CK_Sop1_Dst:
    return 0b00000000011111110000000000000000;
  case CK_Sop1_Opcode:
    return 0b00000000000000001111111100000000;
  case CK_Sop1_Src0:
    return 0b00000000000000000000000011111111;
  default:
    assert(false && "not valid Sop1 content kind");
  }
}

void setEncodingSop1(uint32_t &rawInst) {
  uint32_t mask = getMaskSop1(CK_Sop1_Encoding);
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSop1(uint32_t &rawInst) {
  uint32_t mask = getMaskSop1(CK_Sop1_FixedBits);
  rawInst = (rawInst & ~mask) | ((0b1111101 << 23));
}

void setDstSop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop1(CK_Sop1_Dst);
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setOpcodeSop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop1(CK_Sop1_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}

void setSrc0Sop1(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop1(CK_Sop1_Src0);
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSop1(unsigned opcode, Register dest, Register src0, bool hasLiteral,
              uint32_t literal, codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSop1(newRawInst);
  setFixedBitsSop1(newRawInst);
  setOpcodeSop1(opcode, newRawInst);
  setDstSop1(dest, newRawInst);
  setSrc0Sop1(src0, newRawInst);

  printf("%#x ", newRawInst);
  printf("%u\n", newRawInst);
  printBytes(newRawInst);
  uint32_t *rawInstBuffer = (uint32_t *)gen.cur_ptr();
  *rawInstBuffer = newRawInst;
  ++rawInstBuffer;

  if (hasLiteral) {
    *rawInstBuffer = literal;
    ++rawInstBuffer;

    printf("literal : %#x ", literal);
    printf("%u\n", literal);
    printBytes(literal);
  }

  gen.update((codeBuf_t *)rawInstBuffer);
}
// === SOP1 END ===

// === SOP2 BEGIN ===
uint32_t getMaskSop2(ContentKind k) {
  switch (k) {
  case CK_Sop2_Encoding:
    return 0b11000000000000000000000000000000;
  case CK_Sop2_Opcode:
    return 0b00111111100000000000000000000000;
  case CK_Sop2_Dst:
    return 0b00000000011111110000000000000000;
  case CK_Sop2_Src1:
    return 0b00000000000000001111111100000000;
  case CK_Sop2_Src0:
    return 0b00000000000000000000000011111111;
  default:
    assert(false && "not valid SOP2 content kind");
  }
}

void setEncodingSop2(uint32_t &rawInst) {
  uint32_t mask = getMaskSop2(CK_Sop2_Encoding);
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setOpcodeSop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop2(CK_Sop2_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 23) & mask);
}

void setDstSop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop2(CK_Sop2_Dst);
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}
void setSrc1Sop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop2(CK_Sop2_Src1);
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}
void setSrc0Sop2(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSop2(CK_Sop2_Src0);
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSop2(unsigned opcode, Register dest, Register src0, Register src1,
              codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSop2(newRawInst);
  setOpcodeSop2(opcode, newRawInst);
  setDstSop2(dest, newRawInst);
  setSrc1Sop2(src1, newRawInst);
  setSrc0Sop2(src0, newRawInst);

  printf("%#x ", newRawInst);
  printf("%u\n", newRawInst);
  printBytes(newRawInst);
  uint32_t *rawInstBuffer = (uint32_t *)gen.cur_ptr();
  *rawInstBuffer = newRawInst;
  ++rawInstBuffer;

  gen.update((codeBuf_t *)rawInstBuffer);
}
// === SOP2 END ===

// === SOPC BEGIN ===
uint32_t getMaskSopC(ContentKind k) {
  switch (k) {
  case CK_SopC_Encoding:
    return 0b11000000000000000000000000000000;
  case CK_SopC_FixedBits:
    return 0b00111111100000000000000000000000;
  case CK_SopC_Opcode:
    return 0b00000000011111110000000000000000;
  case CK_SopC_Src1:
    return 0b00000000000000001111111100000000;
  case CK_SopC_Src0:
    return 0b00000000000000000000000011111111;
  default:
    assert(false && "not valid SOPC content kind");
  }
}

void setEncodingSopC(uint32_t &rawInst) {
  uint32_t mask = getMaskSopC(CK_SopC_Encoding);
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSopC(uint32_t &rawInst) {
  uint32_t mask = getMaskSopC(CK_SopC_FixedBits);
  rawInst = (rawInst & ~mask) | ((0b1111110 << 23));
}

void setOpcodeSopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopC(CK_SopC_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSrc1SopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopC(CK_SopC_Src1);
  rawInst = (rawInst & ~mask) | ((value << 8) & mask);
}
void setSrc0SopC(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopC(CK_SopC_Src0);
  rawInst = (rawInst & ~mask) | (value & mask);
}

void emitSopC(unsigned opcode, Register src0, Register src1, codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopC(newRawInst);
  setFixedBitsSopC(newRawInst);
  setOpcodeSopC(opcode, newRawInst);
  setSrc1SopC(src1, newRawInst);
  setSrc0SopC(src0, newRawInst);

  printf("%#x ", newRawInst);
  printf("%u\n", newRawInst);
  printBytes(newRawInst);
  uint32_t *rawInstBuffer = (uint32_t *)gen.cur_ptr();
  *rawInstBuffer = newRawInst;
  ++rawInstBuffer;
  gen.update((codeBuf_t *)rawInstBuffer);
}
// === SOPC END ===

// === SOPK BEGIN ===
uint32_t getMaskSopK(ContentKind k) {
  switch (k) {
  case CK_SopK_Encoding:
    return 0b11000000000000000000000000000000;
  case CK_SopK_FixedBits:
    return 0b00110000000000000000000000000000;
  case CK_SopK_Opcode:
    return 0b00001111100000000000000000000000;
  case CK_SopK_Dst:
    return 0b00000000011111110000000000000000;
  case CK_SopK_SImm16:
    return 0b00000000000000001111111111111111;
  default:
    assert(false && "not valid SopK content kind");
  }
}

void setEncodingSopK(uint32_t &rawInst) {
  uint32_t mask = getMaskSopK(CK_SopK_Encoding);
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSopK(uint32_t &rawInst) {
  uint32_t mask = getMaskSopK(CK_SopK_FixedBits);
  rawInst = (rawInst & ~mask) | ((0b11 << 28));
}

void setOpcodeSopK(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopK(CK_SopK_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 23) & mask);
}

void setDstSopK(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopK(CK_SopK_Dst);
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSImm16SopK(int16_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopK(CK_SopK_SImm16);
  rawInst = (rawInst & ~mask) | ((uint32_t)(value)&mask);
}

void emitSopK(unsigned opcode, Register dest, int16_t simm16, codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopK(newRawInst);
  setFixedBitsSopK(newRawInst);
  setOpcodeSopK(opcode, newRawInst);
  setDstSopK(dest, newRawInst);
  setSImm16SopK(simm16, newRawInst);

  printf("%#x ", newRawInst);
  printf("%u\n", newRawInst);
  printBytes(newRawInst);
  uint32_t *rawInstBuffer = (uint32_t *)gen.cur_ptr();
  *rawInstBuffer = newRawInst;
  ++rawInstBuffer;
  gen.update((codeBuf_t *)rawInstBuffer);
}
// === SOPK END ===

// === SOPP BEGIN ===
uint32_t getMaskSopP(ContentKind k) {
  switch (k) {
  case CK_SopP_Encoding:
    return 0b11000000000000000000000000000000;
  case CK_SopP_FixedBits:
    return 0b00111111100000000000000000000000;
  case CK_SopP_Opcode:
    return 0b00000000011111110000000000000000;
  case CK_SopP_SImm16:
    return 0b00000000000000001111111111111111;
  default:
    assert(false && "not valid SopP content kind");
  }
}

void setEncodingSopP(uint32_t &rawInst) {
  uint32_t mask = getMaskSopP(CK_SopP_Encoding);
  rawInst = (rawInst & ~mask) | ((1 << 31));
}

void setFixedBitsSopP(uint32_t &rawInst) {
  uint32_t mask = getMaskSopP(CK_SopP_FixedBits);
  rawInst = (rawInst & ~mask) | ((0b1111111 << 23));
}

void setOpcodeSopP(uint32_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopP(CK_SopP_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 16) & mask);
}

void setSImm16SopP(int16_t value, uint32_t &rawInst) {
  uint32_t mask = getMaskSopP(CK_SopP_SImm16);
  rawInst = (rawInst & ~mask) | ((uint32_t)(value)&mask);
}

void emitSopP(unsigned opcode, bool hasImm, int16_t simm16, codeGen &gen) {
  uint32_t newRawInst = 0xFFFFFFFF;
  setEncodingSopP(newRawInst);
  setFixedBitsSopP(newRawInst);
  setOpcodeSopP(opcode, newRawInst);

  if (hasImm)
    setSImm16SopP(simm16, newRawInst);
  else
    setSImm16SopP(0, newRawInst);

  printf("%#x ", newRawInst);
  printf("%u\n", newRawInst);
  printBytes(newRawInst);
  uint32_t *rawInstBuffer = (uint32_t *)gen.cur_ptr();
  *rawInstBuffer = newRawInst;
  ++rawInstBuffer;
  gen.update((codeBuf_t *)rawInstBuffer);
}
// === SOPP END ===

// === SMEM BEGIN ===
uint64_t getMaskSmem(ContentKind k) {
  switch (k) {
  case CK_Smem_Encoding:
    return 0b0000000000000000000000000000000011111100000000000000000000000000;
  case CK_Smem_Opcode:
    return 0b0000000000000000000000000000000000000011111111000000000000000000;
  case CK_Smem_Imm:
    return 0b0000000000000000000000000000000000000000000000100000000000000000;
  case CK_Smem_Glc:
    return 0b0000000000000000000000000000000000000000000000010000000000000000;
  case CK_Smem_Nv:
    return 0b0000000000000000000000000000000000000000000000001000000000000000;
  case CK_Smem_Soe:
    return 0b0000000000000000000000000000000000000000000000000100000000000000;
  case CK_Smem_R1:
    return 0b0000000000000000000000000000000000000000000000000010000000000000;
  case CK_Smem_Sdata:
    return 0b0000000000000000000000000000000000000000000000000001111111000000;
  case CK_Smem_Sbase:
    return 0b0000000000000000000000000000000000000000000000000000000000111111;
  case CK_Smem_Soffset:
    return 0b1111111000000000000000000000000000000000000000000000000000000000;
  case CK_Smem_R4:
    return 0b0000000111100000000000000000000000000000000000000000000000000000;
  case CK_Smem_Offset:
    return 0b0000000000011111111111111111111100000000000000000000000000000000;
  default:
    assert(false && "not valid SMEM content kind");
  }
}

void setEncodingSmem(uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Encoding);
  rawInst = (rawInst & ~mask) | (((uint64_t)(0b110000) << 26) & mask);
}

void setOpcodeSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Opcode);
  rawInst = (rawInst & ~mask) | ((value << 18) & mask);
}

void setImmSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Imm);
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 17) & mask);
}

void setGlcSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Glc);
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 16) & mask);
}

void setNvSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Nv);
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 15) & mask);
}

void setSoeSmem(bool value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Soe);
  rawInst = (rawInst & ~mask) | (((uint64_t)value << 14) & mask);
}

void setR1Smem(uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_R1);
  rawInst = (rawInst & ~mask) | (uint64_t(0) & mask);
}

void setSdataSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Sdata);
  rawInst = (rawInst & ~mask) | ((value << 6) & mask);
}

void setSbaseSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Sbase);
  rawInst = (rawInst & ~mask) | ((value)&mask);
}

void setSoffsetSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Soffset);
  rawInst = (rawInst & ~mask) | ((value << 57) & mask);
}

void setR4Smem(uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_R4);
  rawInst = (rawInst & ~mask) | ((uint64_t(0) << 53) & mask);
}

void setOffsetSmem(uint64_t value, uint64_t &rawInst) {
  uint64_t mask = getMaskSmem(CK_Smem_Offset);
  rawInst = (rawInst & ~mask) | ((value << 32) & mask);
}

void emitSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset,
              codeGen &gen) {
  uint64_t newRawInst = 0xFFFFFFFFFFFFFFFF;

  setEncodingSmem(newRawInst);
  setOpcodeSmem(opcode, newRawInst);

  setImmSmem(1, newRawInst);
  setGlcSmem(0, newRawInst);
  setNvSmem(0, newRawInst);
  setSoeSmem(0, newRawInst);
  setR1Smem(newRawInst);
  setSdataSmem(sdata, newRawInst);
  setSbaseSmem(sbase, newRawInst);
  setSoffsetSmem(0, newRawInst);
  setR4Smem(newRawInst);
  setOffsetSmem(offset, newRawInst);

  printf("%#lx ", newRawInst);
  printf("%lu\n", newRawInst);
  printBytes(newRawInst);

  uint64_t *rawInstBuffer = (uint64_t *)gen.cur_ptr();
  ++rawInstBuffer;
  gen.update((codeBuf_t *)rawInstBuffer);
}

// === SMEM END ===

} // namespace Vega
