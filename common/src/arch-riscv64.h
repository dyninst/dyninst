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

// $Id: arch-power.h,v 1.45 2008/03/25 19:24:23 bernat Exp $

#ifndef _ARCH_RISCV64_H
#define _ARCH_RISCV64_H

// Code generation

#include "dyntypes.h"
#include "registers/riscv64_regs.h"
#include <assert.h>
#include <vector>
#include <bitset>
class AddressSpace;

namespace NS_riscv64 {

/*
 * Define riscv64 instruction information.
 *
 */

// In RISC-V, instruction length can be a multiple of 2 bytes

// Standard RISC-V instructions (RVI, RVA, RVM, RVF, RVD, ...) are 4 bytes
constexpr int RV_INSN_SIZE = 4;     // Standard instructions
// Compressed instructions (RVC) are 2 bytes
constexpr int RVC_INSN_SIZE = 2;    // Compressed instructions
// The instruction buffer length should be the maximum instruction length currently supported.
// Change this value if we want to support longer instructions like LLI extension
constexpr int INSN_BUFF_SIZE = (8 * RV_INSN_SIZE);
typedef std::bitset<INSN_BUFF_SIZE> insnBuf_t;


typedef unsigned short rvInsnMin_t;


constexpr int BREAK_POINT_INSN = 0x00100073; // ebreak

constexpr int LUIOp      = 0x37;
constexpr int AUIPCOp    = 0x17;

constexpr int IMMOp      = 0x13;
constexpr int REGOp      = 0x33;
constexpr int JALOp      = 0x6f;
constexpr int JALROp     = 0x67;
constexpr int LOADOp     = 0x03;
constexpr int STOREOp    = 0x23;
constexpr int BRANCHOp   = 0x63;

constexpr int ADDFunct7  = 0x00;
constexpr int SUBFunct7  = 0x20;
constexpr int SLLFunct7  = 0x00;
constexpr int XORFunct7  = 0x00;
constexpr int SRLFunct7  = 0x00;
constexpr int SRAFunct7  = 0x20;
constexpr int ORFunct7   = 0x00;
constexpr int ANDFunct7  = 0x00;
constexpr int MULFunct7  = 0x01;
constexpr int DIVFunct7  = 0x01;

constexpr int ADDFunct3  = 0x0;
constexpr int SUBFunct3  = 0x0;
constexpr int SLLFunct3  = 0x1;
constexpr int XORFunct3  = 0x4;
constexpr int SRLFunct3  = 0x5;
constexpr int SRAFunct3  = 0x5;
constexpr int ORFunct3   = 0x6;
constexpr int ANDFunct3  = 0x7;
constexpr int JALRFunct3 = 0x0;
constexpr int MULFunct3  = 0x0;
constexpr int DIVFunct3  = 0x4;

constexpr int BEQFunct3  = 0x0;
constexpr int BNEFunct3  = 0x1;
constexpr int BLTFunct3  = 0x4;
constexpr int BGEFunct3  = 0x5;
constexpr int BLTUFunct3 = 0x6;
constexpr int BGEUFunct3 = 0x7;

constexpr int GPR_ZERO   = 0;
constexpr int GPR_RA     = 1;
constexpr int GPR_SP     = 2;
constexpr int GPR_FP     = 8;

constexpr int MAX_BRANCH_OFFSET      = 0xfffff; // 20 bits
constexpr int MAX_BRANCH_LINK_OFFSET = 0xfff;   // 12 bits

constexpr insnBuf_t J_INSN_MASK    = insnBuf_t(0x0000007f);
constexpr insnBuf_t B_INSN_MASK    = insnBuf_t(0x0000007f);
constexpr insnBuf_t CJ_INSN_MASK   = insnBuf_t(0xe003);
constexpr insnBuf_t CJR_INSN_MASK  = insnBuf_t(0xf003);
constexpr insnBuf_t CB_INSN_MASK   = insnBuf_t(0xe003);

constexpr insnBuf_t B_INSNS        = insnBuf_t(0x00000063);
constexpr insnBuf_t JALR_INSN      = insnBuf_t(0x00000067);
constexpr insnBuf_t JAL_INSN       = insnBuf_t(0x0000006f);
constexpr insnBuf_t CBEQZ_INSN     = insnBuf_t(0xc001);
constexpr insnBuf_t CBNEZ_INSN     = insnBuf_t(0xe001);
constexpr insnBuf_t CJALR_INSN     = insnBuf_t(0x9002);
constexpr insnBuf_t CJAL_INSN      = insnBuf_t(0x2001);
constexpr insnBuf_t CJR_INSN       = insnBuf_t(0x8002);
constexpr insnBuf_t CJ_INSN        = insnBuf_t(0xa001);

// RISC-V immediates in jump/branch instructions are scrambled:
// > the instruction format was chosen to keep all register specifiers at the same position in all formats
// The following arrays store the corresponding indexes to reorder the immediates back

const std::vector<int> JAL_REORDER  = {12, 13, 14, 15, 16, 17, 18, 19, 11,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20};
const std::vector<int> JALR_REORDER = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const std::vector<int> B_REORDER1   = {11, 1, 2, 3, 4};
const std::vector<int> B_REORDER2   = {5, 6, 7, 8, 9, 10, 12};
const std::vector<int> CJ_REORDER   = {5, 1, 2, 3, 7, 6, 10, 8, 9, 4, 11};
const std::vector<int> CB_REORDER1  = {5, 1, 2, 6, 7};
const std::vector<int> CB_REORDER2  = {3, 4, 8};

// The following are the indices of the immediates' offset

const int JAL_IMM_OFF  = 12;
const int JALR_IMM_OFF = 20;
const int CJ_IMM_OFF   = 2; 

const int B_IMM_OFF1   = 7;
const int B_IMM_OFF2   = 25;
const int CB_IMM_OFF1  = 2;
const int CB_IMM_OFF2  = 10;

constexpr insnBuf_t JALR_REG_MASK = insnBuf_t(0x000f8000);
constexpr insnBuf_t CJR_REG_MASK  = insnBuf_t(0x0f80);
constexpr int JALR_REG_SHIFT    = 15;
constexpr int CJR_REG_SHIFT     = 7;

// Atomic Instructions
constexpr insnBuf_t A_INSN_MASK = insnBuf_t(0x0000007f);
constexpr insnBuf_t A_INSNS     = insnBuf_t(0x0000002f);
constexpr insnBuf_t A_OP_MASK   = insnBuf_t(0xf8000000);
constexpr insnBuf_t A_OP_LR     = insnBuf_t(0x10000000);
constexpr insnBuf_t A_OP_SC     = insnBuf_t(0x18000000);

#define INSN_BUFF_SET(I, s, e, v)    ((I).setInsnBuf((s), (e - s + 1), (v)))

typedef union {
    unsigned char byte[sizeof(rvInsnMin_t)];
    rvInsnMin_t raw;
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

#define maxGPR 32
#define maxFPR 32

class DYNINST_EXPORT instruction {
private:
    // Due to the way codegen.C works, codeBuf_t should be a fixed size.
    // Therefore, we write opcodes to our own instruction buffer `insn_buff` instead.
    // Then, whever we generate the instruction, we call `flushInsnBuffer` to flush the
    // instruction buffer into the 2-byte code buffer `code_buff` 2-bytes by 2-bytes.

    codeBuf_t code_buff;
    insnBuf_t insn_buff;
    bool isRVC;

public:
    instruction(): code_buff(), isRVC(false) { insn_buff.set(); }
    instruction(rvInsnMin_t raw): isRVC(false) {
        code_buff.raw = raw;
        insn_buff.set();
    }
    instruction(const void *ptr, bool) {
        code_buff = *((const codeBuf_t *)ptr);
    }

    instruction(const instruction &insn) : code_buff(insn.code_buff), isRVC(insn.isRVC) {}
    instruction(codeBuf_t &insn) : code_buff(insn) {}

    void clear() { 
        code_buff = instructUnion();
        insn_buff.reset();
    }
    void setInsnBuf(unsigned int pos, unsigned int len, unsigned int value) {
        std::bitset<INSN_BUFF_SIZE> bits(value);
        for (unsigned i = 0; i < len; i++) {
            insn_buff.set(pos + i, bits[i]);
        }
    }

    void flushInsnBuff(int begin) {
        std::bitset<INSN_BUFF_SIZE> mask;
        for (unsigned i = 0; i < 8 * sizeof(rvInsnMin_t); i++) {
            mask.set(begin + i);
        }
        code_buff.raw = static_cast<rvInsnMin_t>(((insn_buff & mask) >> begin).to_ulong());
    }

    bool getIsRVC() const { return isRVC; }
    void setIsRVC(bool isRVC_) { isRVC = isRVC_; }

    unsigned size() { return isRVC ? RVC_INSN_SIZE : RV_INSN_SIZE; }

    // return a pointer to the instruction
    const unsigned char *ptr() const { return (const unsigned char *)&code_buff; }

    static bool isAligned(Dyninst::Address addr) {
        return !(addr & 0x1);
    }

    unsigned getTargetReg() const;
    Dyninst::Address getTarget(Dyninst::Address addr) const;

    bool isBranchReg() const;
    bool isBranchOffset() const;
    bool isUncondBranch() const;
    bool isCondBranch() const;
    unsigned getBranchTargetReg() const;
    Dyninst::Address getBranchOffset() const;
    bool isAtomic() const;
    bool isAtomicMemOp() const;
    bool isAtomicLoad() const;
    bool isAtomicStore() const;
};

}
//end of NS_riscv64

#endif
