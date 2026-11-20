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

#ifndef AMDGPU_GFX90A_DETAILS_H
#define AMDGPU_GFX90A_DETAILS_H

#include "codegen/codegen.h"
#include <stdint.h>

namespace Dyninst {
namespace amdgpu_gfx90a {

using Register = Dyninst::Register;

// clang-format off
enum ContentMask32 : uint32_t {
  Mask_Sop1_Encoding  = 0xC0000000,    // 0b11000000000000000000000000000000
  Mask_Sop1_FixedBits = 0x3F800000,    // 0b00111111100000000000000000000000
  Mask_Sop1_Dst       = 0x007F0000,    // 0b00000000011111110000000000000000
  Mask_Sop1_Opcode    = 0x0000FF00,    // 0b00000000000000001111111100000000
  Mask_Sop1_Src0      = 0x000000FF,    // 0b00000000000000000000000011111111

  Mask_Sop2_Encoding  = 0xC0000000,    // 0b11000000000000000000000000000000
  Mask_Sop2_Opcode    = 0x3F800000,    // 0b00111111100000000000000000000000
  Mask_Sop2_Dst       = 0x007F0000,    // 0b00000000011111110000000000000000
  Mask_Sop2_Src1      = 0x0000FF00,    // 0b00000000000000001111111100000000
  Mask_Sop2_Src0      = 0x000000FF,    // 0b00000000000000000000000011111111

  Mask_SopC_Encoding  = 0xC0000000,    // 0b11000000000000000000000000000000
  Mask_SopC_FixedBits = 0x3F800000,    // 0b00111111100000000000000000000000
  Mask_SopC_Opcode    = 0x007F0000,    // 0b00000000011111110000000000000000
  Mask_SopC_Src1      = 0x0000FF00,    // 0b00000000000000001111111100000000
  Mask_SopC_Src0      = 0x000000FF,    // 0b00000000000000000000000011111111

  Mask_SopK_Encoding  = 0xC0000000,    // 0b11000000000000000000000000000000
  Mask_SopK_FixedBits = 0x30000000,    // 0b00110000000000000000000000000000
  Mask_SopK_Opcode    = 0x0F800000,    // 0b00001111100000000000000000000000
  Mask_SopK_Dst       = 0x007F0000,    // 0b00000000011111110000000000000000
  Mask_SopK_SImm16    = 0x0000FFFF,    // 0b00000000000000001111111111111111

  Mask_SopP_Encoding  = 0xC0000000,    // 0b11000000000000000000000000000000
  Mask_SopP_FixedBits = 0x3F800000,    // 0b00111111100000000000000000000000
  Mask_SopP_Opcode    = 0x007F0000,    // 0b00000000011111110000000000000000
  Mask_SopP_SImm16    = 0x0000FFFF     // 0b00000000000000001111111111111111
};

enum ContentMask64 : uint64_t {
  Mask_Smem_Encoding = 0x00000000FC000000,  // 0b0000000000000000000000000000000011111100000000000000000000000000
  Mask_Smem_Opcode   = 0x0000000003FC0000,  // 0b0000000000000000000000000000000000000011111111000000000000000000
  Mask_Smem_Imm      = 0x0000000000020000,  // 0b0000000000000000000000000000000000000000000000100000000000000000
  Mask_Smem_Glc      = 0x0000000000010000,  // 0b0000000000000000000000000000000000000000000000010000000000000000
  Mask_Smem_Nv       = 0x0000000000008000,  // 0b0000000000000000000000000000000000000000000000001000000000000000
  Mask_Smem_Soe      = 0x0000000000004000,  // 0b0000000000000000000000000000000000000000000000000100000000000000
  Mask_Smem_R1       = 0x0000000000002000,  // 0b0000000000000000000000000000000000000000000000000010000000000000
  Mask_Smem_Sdata    = 0x0000000000001FC0,  // 0b0000000000000000000000000000000000000000000000000001111111000000
  Mask_Smem_Sbase    = 0x000000000000003F,  // 0b0000000000000000000000000000000000000000000000000000000000111111
  Mask_Smem_Soffset  = 0xFE00000000000000,  // 0b1111111000000000000000000000000000000000000000000000000000000000
  Mask_Smem_R4       = 0x01E0000000000000,  // 0b0000000111100000000000000000000000000000000000000000000000000000
  Mask_Smem_Offset   = 0x001FFFFF00000000   // 0b0000000000011111111111111111111100000000000000000000000000000000
 };

// === SOP1 BEGIN ===

// SOP1 instruction format in memory: [encoding] [opcode] [sdst] [ssrc1] [ssrc0]
//                   bits (total 32):   2(10)       7       7       8       8
// This enum contains particular SOP1 instructions of interest.
// Extend it later as needed.

void setEncodingSop1(uint32_t &rawInst);
void setFixedBitsSop1(uint32_t &rawInst);
void setOpcodeSop1(uint32_t value, uint32_t &rawInst);
void setDstSop1(uint32_t value, uint32_t &rawInst);
void setSrc0Sop1(uint32_t value, uint32_t &rawInst);

void emitSop1(unsigned opcode, Register dest, Register src0, bool hasLiteral,
              uint32_t literal, codeGen &gen);

// === SOP1 END ===

// === SOP2 BEGIN ===

// SOP2 instruction format in memory: [encoding] [opcode] [sdst] [ssrc1] [ssrc0]
//                   bits (total 32):   2(10)       7       7       8       8
// This enum contains particular SOP2 instructions of interest.
// Extend it later as needed.
// NOTE: many of these operations are unused as the abstract opcode enum doesn't have such detailed options.

void setEncodingSop2(uint32_t &rawInst);
void setOpcodeSop2(uint32_t value, uint32_t &rawInst);
void setDstSop2(uint32_t value, uint32_t &rawInst);
void setSrc1Sop2(uint32_t value, uint32_t &rawInst);
void setSrc0Sop2(uint32_t value, uint32_t &rawInst);

void emitSop2(unsigned opcode, Register dest, Register src1, Register src0,
              codeGen &gen);

void emitSop2WithSrc1Literal(unsigned opcode, Register dest, Register src0,
                             uint32_t src1Literal, codeGen &gen);

// === SOP2 END ===

// === SOPC BEGIN ===
// SOPC instruction format in memory: [encoding] [7-fixed-bits] [opcode] [ssrc1] [ssrc0]
//                   bits (total 32):   2(10)      (1111110)        7       8       8
//
// SCC = S0 opcode S1
// This enum contains particular SOPC instructions of interest.
// Extend it later as needed.

void setEncodingSopC(uint32_t &rawInst);
void setFixedBitsSopC(uint32_t &rawInst);
void setOpcodeSopC(uint32_t value, uint32_t &rawInst);
void setSrc1SopC(uint32_t value, uint32_t &rawInst);
void setSrc0SopC(uint32_t value, uint32_t &rawInst);

void emitSopC(unsigned opcode, Register src1, Register src0, codeGen &gen);

// === SOPC END ===

// === SOPK BEGIN ===
// SOPK instruction format in memory: [encoding] [2-fixed-bits] [opcode] [sdst] [simm16]
//                   bits (total 32):   2(10)         (11)         5       7       16
//
// This enum contains particular SOPK instructions of interest.
// Extend it later as needed.

void setEncodingSopK(uint32_t &rawInst);
void setFixedBitsSopK(uint32_t &rawInst);
void setOpcodeSopK(uint32_t value, uint32_t &rawInst);
void setDstSopK(uint32_t value, uint32_t &rawInst);
void setSImm16SopK(int16_t value, uint32_t &rawInst);

void emitSopK(unsigned opcode, Register dest, int16_t simm16, codeGen &gen);
// === SOPK END ===

// === SOPP BEGIN ===
// SOPP instruction format in memory: [encoding] [7-fixed-bits] [opcode] [simm16]
//                   bits (total 32):   2(10)      (1111111)       7        16
//
// This enum contains particular SOPP instructions of interest.
// Extend it later as needed.

void setEncodingSopP(uint32_t &rawInst);
void setFixedBitsSopP(uint32_t &rawInst);
void setOpcodeSopP(uint32_t value, uint32_t &rawInst);
void setSImm16SopP(int16_t value, uint32_t &rawInst);

void emitSopP(unsigned opcode, int16_t simm16, codeGen &gen);
// === SOPP END ===

// === SMEM BEGIN ===
// SMEM instruction format in memory : (total 64 bits)

// 31                                                            0
// [encoding] [opcode] [imm] [glc] [nv] [soe] [r1] [sdata] [sbase]
//  6(110000)    8       1     1     1    1    1      7       6
//
// 63                   32
// [soffset] [r4] [offset]
//     7       4     21
//
// r1 and r4 are reserved.

// This enum contains particular SMEM instructions of interest.
// Extend it later as needed.

unsigned getSmemImmBit(unsigned opcode);
void setEncodingSmem(uint64_t &rawInst);
void setOpcodeSmem(uint64_t value, uint64_t &rawInst);
void setImmSmem(bool value, uint64_t &rawInst);
void setGlcSmem(bool value, uint64_t &rawInst);
void setNvSmem(bool value, uint64_t &rawInst);
void setSoeSmem(bool value, uint64_t &rawInst);
void setR1Smem(uint64_t &rawInst);
void setSdataSmem(uint64_t value, uint64_t &rawInst);
void setSbaseSmem(uint64_t value, uint64_t &rawInst);
void setSoffsetSmem(uint64_t value, uint64_t &rawInst);
void setR4Smem(uint64_t &rawInst);
void setOffsetSmem(uint64_t value, uint64_t &rawInst);

// We can load 1, 2, 4, 8 or 16 words at a time and store 1, 2, 4 words at a
// time.
//
// For S_DCACHE_WB, imm = 0.
//
// In other cases, we use glc = 0, imm = 1, soe = 0, nv = 0 (nv stands for non-volatile)
// so essentially address = sgpr[base] + signed 21-bit byte offset.
//
// ERRORS IN MANUAL:
// [1] imm = 1, soe = 0 on inspecting instruction (as per the actual assembled
// bytes). But page 55 in the manual says imm = 0, soe = 0 for using offset
// only. [2] Manual says 20-bit unsigned offset and 21 bit diagram, but XML spec
// says 21 bit signed offset.
void emitSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset,
              codeGen &gen);

// === SMEM END ===
// === VOP1 / VOP2 / FLAT(global/scratch) ===
void emitVop1Reg(unsigned opcode, uint32_t vdst, uint32_t src0, codeGen &gen);
void emitVop1Imm(unsigned opcode, uint32_t vdst, uint32_t literal, codeGen &gen);
void emitVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &gen);
void emitVop3a(unsigned opcode, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &gen);
void emitFlatGlobal(unsigned opcode, uint32_t vdst, uint32_t addr, uint32_t data, uint32_t saddr,
                    int32_t offset, codeGen &gen, bool glc, uint32_t seg);

} // namespace amdgpu_gfx90a
} // namespace Dyninst

#endif
