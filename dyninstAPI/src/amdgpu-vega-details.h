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

#ifndef AMDGPU_VEGA_DETAILS_H
#define AMDGPU_VEGA_DETAILS_H

#include "codegen.h"
#include <stdint.h>

namespace Vega {

using Register = Dyninst::Register;

// TODO : Map scalar regs to abstract registers?
enum ScalarRegs {
  SGPR_0 = 0,
  SGPR_1,
  SGPR_2,
  SGPR_3,
  SGPR_4,
  SGPR_5,
  SGPR_6,
  SGPR_7,
  SGPR_8,
  SGPR_9,
  SGPR_10,
  SGPR_11,
  SGPR_12,
  SGPR_13,
  SGPR_14,
  SGPR_15,
  SGPR_16,
  SGPR_17,
  SGPR_18,
  SGPR_19,
  SGPR_20,
  SGPR_21,
  SGPR_22,
  SGPR_23,
  SGPR_24,
  SGPR_25,
  SGPR_26,
  SGPR_27,
  SGPR_28,
  SGPR_29,
  SGPR_30,
  SGPR_31,
  SGPR_32,
  SGPR_33,
  SGPR_34,
  SGPR_35,
  SGPR_36,
  SGPR_37,
  SGPR_38,
  SGPR_39,
  SGPR_40,
  SGPR_41,
  SGPR_42,
  SGPR_43,
  SGPR_44,
  SGPR_45,
  SGPR_46,
  SGPR_47,
  SGPR_48,
  SGPR_49,
  SGPR_50,
  SGPR_51,
  SGPR_52,
  SGPR_53,
  SGPR_54,
  SGPR_55,
  SGPR_56,
  SGPR_57,
  SGPR_58,
  SGPR_59,
  SGPR_60,
  SGPR_61,
  SGPR_62,
  SGPR_63,
  SGPR_64,
  SGPR_65,
  SGPR_66,
  SGPR_67,
  SGPR_68,
  SGPR_69,
  SGPR_70,
  SGPR_71,
  SGPR_72,
  SGPR_73,
  SGPR_74,
  SGPR_75,
  SGPR_76,
  SGPR_77,
  SGPR_78,
  SGPR_79,
  SGPR_80,
  SGPR_81,
  SGPR_82,
  SGPR_83,
  SGPR_84,
  SGPR_85,
  SGPR_86,
  SGPR_87,
  SGPR_88,
  SGPR_89,
  SGPR_90,
  SGPR_91,
  SGPR_92,
  SGPR_93,
  SGPR_94,
  SGPR_95,
  SGPR_96,
  SGPR_97,
  SGPR_98,
  SGPR_99,
  SGPR_100,
  SGPR_101,
}; // ScalarRegs

enum ContentKind {
  CK_Sop1_Encoding,
  CK_Sop1_FixedBits,
  CK_Sop1_Opcode,
  CK_Sop1_Dst,
  CK_Sop1_Src0,

  CK_Sop2_Encoding,
  CK_Sop2_Opcode,
  CK_Sop2_Dst,
  CK_Sop2_Src1,
  CK_Sop2_Src0,

  CK_SopC_Encoding,
  CK_SopC_FixedBits,
  CK_SopC_Opcode,
  CK_SopC_Src1,
  CK_SopC_Src0,

  CK_SopK_Encoding,
  CK_SopK_FixedBits,
  CK_SopK_Opcode,
  CK_SopK_Dst,
  CK_SopK_SImm16,

  CK_SopP_Encoding,
  CK_SopP_FixedBits,
  CK_SopP_Opcode,
  CK_SopP_SImm16,

  CK_Smem_Encoding,
  CK_Smem_Opcode,
  CK_Smem_Imm,
  CK_Smem_Glc,
  CK_Smem_Nv,
  CK_Smem_Soe,
  CK_Smem_R1,
  CK_Smem_Sdata,
  CK_Smem_Sbase,
  CK_Smem_Soffset,
  CK_Smem_R4,
  CK_Smem_Offset
};

// === SOP1 BEGIN ===

// SOP1 instruction format in memory: [encoding] [opcode] [sdst] [ssrc1] [ssrc0]
//                   bits (total 32):   2(10)       7       7       8       8
// This enum contains particular SOP1 instructions of interest.
// Extend it later as needed.
enum SOP1_Opcode {
  S_MOV_B32 = 0,
  S_GETPC_B64 = 28,
  S_SETPC_B64 = 29,
  S_SWAPPC_B64 = 30
};

uint32_t getMaskSop1(ContentKind k);
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
enum SOP2_Opcode {
  S_ADD_U32 = 0,
  S_SUB_U32 = 1,
  S_ADD_I32 = 2,
  S_SUB_I32 = 3,
  S_ADDC_U32 = 4,
  S_SUBB_U32 = 5,
  S_MUL_I32 = 36,
  S_CSELE3CT_B32 = 10,
  S_CSELECT_B64 = 11,

  S_AND_B32 = 12,
  S_AND_B64 = 13,
  S_OR_B32 = 14,
  S_OR_B64 = 15,
  S_XOR_B32 = 16,
  S_XOR_B64 = 17,
  S_NAND_B32 = 22,
  S_NAND_B64 = 23,
  S_NOR_B32 = 24,
  S_NOR_B64 = 25,
  S_XNOR_B32 = 26,
  S_XNOR_B64 = 27

};

uint32_t getMaskSop2(ContentKind k);
void setEncodingSop2(uint32_t &rawInst);
void setOpcodeSop2(uint32_t value, uint32_t &rawInst);
void setDstSop2(uint32_t value, uint32_t &rawInst);
void setSrc1Sop2(uint32_t value, uint32_t &rawInst);
void setSrc0Sop2(uint32_t value, uint32_t &rawInst);

void emitSop2(unsigned opcode, Register dest, Register src1, Register src0,
              codeGen &gen);

// === SOP2 END ===

// === SOPC BEGIN ===
// SOPC instruction format in memory: [encoding] [7-fixed-bits] [opcode] [ssrc1] [ssrc0]
//                   bits (total 32):   2(10)      (1111110)        7       8       8
//
// SCC = S0 opcode S1
// This enum contains particular SOPC instructions of interest.
// Extend it later as needed.
enum SOPC_Opcode {
  S_CMP_EQ_I32 = 0,
  S_CMP_EQ_U32 = 6,
  S_CMP_EQ_U64 = 18,

  S_CMP_LG_I32 = 1,
  S_CMP_LG_U32 = 7,

  S_CMP_GT_I32 = 2,
  S_CMP_GT_U32 = 8,

  S_CMP_GE_I32 = 3,
  S_CMP_GE_U32 = 9,

  S_CMP_LT_I32 = 4,
  S_CMP_LT_U32 = 10,

  S_CMP_LE_I32 = 5,
  S_CMP_LE_U32 = 11

  // Leaving out S_BITCMP0/1 instructions
};

uint32_t getMaskSopC(ContentKind k);
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
enum SOPK_Opcode {
  S_CMPK_EQ_I32 = 2,
  S_CMPK_EQ_U32 = 8,

  S_CMPK_LG_I32 = 3,
  S_CMPK_LG_U32 = 9,

  S_CMPK_GT_I32 = 4,
  S_CMPK_GT_U32 = 10,

  S_CMPK_GE_I32 = 5,
  S_CMPK_GE_U32 = 11,

  S_CMPK_LT_I32 = 6,
  S_CMPK_LT_U32 = 12,

  S_CMPK_LE_I32 = 7,
  S_CMPK_LE_U32 = 13,

  S_ADDK_I32 = 14,
  S_MULK_I32 = 15
};

uint32_t getMaskSopK(ContentKind k);
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
enum SOPP_Opcode {
  S_NOP = 0,
  S_ENDPGM = 1,
  S_BRANCH = 2,
  S_CBRANCH_SCC0 = 4,
  S_CBRANCH_SCC1 = 5,
  S_WAITCNT = 12
};

uint32_t getMaskSopP(ContentKind k);
void setEncodingSopP(uint32_t &rawInst);
void setFixedBitsSopP(uint32_t &rawInst);
void setOpcodeSopP(uint32_t value, uint32_t &rawInst);
void setSImm16SopP(int16_t value, uint32_t &rawInst);

void emitSopP(unsigned opcode, bool hasImm, int16_t simm16, codeGen &gen);
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
enum SMEM_Opcode {
  S_LOAD_DWORD = 0,
  S_LOAD_DWORDX2 = 1,
  S_LOAD_DWORDX4 = 2,
  S_LOAD_DWORDX8 = 3,
  S_LOAD_DWORDX16 = 4,

  S_STORE_DWORD = 16,
  S_STORE_DWORDX2 = 17,
  S_STORE_DWORDX4 = 18,
};

uint64_t getMaskSmem(ContentKind k);
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
// We use glc = 0, imm = 1, soe = 0, nv = 0 (nv stands for non-volatile)
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

} // namespace Vega

#endif
