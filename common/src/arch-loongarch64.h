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

#ifndef _ARCH_LOONGARCH64_H
#define _ARCH_LOONGARCH64_H

#include "dyntypes.h"
#include "registers/loongarch64_regs.h"
#include "common/h/dyn_regs.h"
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_loongarch64 {

#define BREAK_POINT_INSN 0x002B0000  // BRK instruction

// LoongArch64 instruction opcodes (major opcode field)
#define BOp             0x10000    // B (unconditional branch)
#define BCondOp         0x1000     // BEQ/BNE/etc (conditional branch)
#define BLBOp           0x5000     // BL (branch and link)
#define JIROp           0x4C       // JIRL (jump register)
#define BEQOp           0x1000     // BEQ
#define BNEOp           0x1400     // BNE
#define BLTZOp          0x100      // BLTZ
#define BGEZOp          0x140      // BGEZ
#define BEQZOp          0x100      // BEQZ
#define BNEZOp          0x140      // BNEZ

// ALU operations
#define ADDIWOp         0x0A       // ADDI.W
#define ADDIOp          0x0A       // ADDI
#define ADDOp           0x20       // ADD
#define SUBOp           0x22       // SUB
#define LU12IWOp        0x0A       // LU12I.W
#define LU32IDOp        0x0B       // LU32I.D
#define ORIOp           0x0E       // ORI

// Load/Store operations
#define LDOp            0x0A       // LD
#define STOp            0x0A       // ST
#define LDWOp           0x0A       // LD.W
#define STWOp           0x0A       // ST.W

#define NOOP            0x03400000  // NOP (andi $r0, $r0, 0)

#define MIN_IMM12   (-2048)
#define MAX_IMM12   (2047)
#define MIN_IMM16   (-32768)
#define MAX_IMM16   (32767)
#define MIN_IMM20   (-524288)
#define MAX_IMM20   (524287)
#define MIN_IMM26   (-33554432)
#define MAX_IMM26   (33554431)

#define SPR_LR      1   // $ra (return address register)
#define SPR_SP      3   // $sp (stack pointer)

#define INSN_SET(I, s, e, v)    ((I).setBits(s, e - s + 1, (v)))

typedef union {
    unsigned char byte[4];
    unsigned int  raw;
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

#define maxGPR 32
#define maxFPR 32

class DYNINST_EXPORT instruction {
private:
    instructUnion insn_;

public:
    instruction() { insn_.raw = 0; }
    instruction(unsigned int raw) {
        insn_.raw = raw;
    }
    instruction(const void *ptr) {
        insn_ = *((const instructUnion *)ptr);
    }
    instruction(const void *ptr, bool) {
        insn_ = *((const instructUnion *)ptr);
    }

    instruction(const instruction &insn) : insn_(insn.insn_) {}
    instruction(instructUnion &insn) : insn_(insn) {}

    instruction *copy() const;

    void clear() { insn_.raw = 0; }
    void setInstruction(codeBuf_t *ptr, Dyninst::Address = 0);
    void setBits(unsigned int pos, unsigned int len, unsigned int value) {
        unsigned int mask;
        mask = ~((unsigned int)(~0) << len);
        value = value & mask;
        mask = ~(mask << pos);
        value = value << pos;
        insn_.raw = insn_.raw & mask;
        insn_.raw = insn_.raw | value;
    }
    unsigned int asInt() const { return insn_.raw; }
    void setInstruction(unsigned char *ptr, Dyninst::Address = 0);

    static int signExtend(unsigned int i, unsigned int pos);

    static unsigned size() { return sizeof(instructUnion); }

    Dyninst::Address getBranchOffset() const;
    Dyninst::Address getBranchTargetAddress() const;
    void setBranchOffset(Dyninst::Address newOffset);

    static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
    static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width);
    static unsigned maxJumpSize(unsigned addr_width);

    static unsigned maxInterFunctionJumpSize(unsigned addr_width);

    unsigned type() const;

    const unsigned char *ptr() const { return (const unsigned char *)&insn_; }

    unsigned opcode() const;

    bool isInsnType(const unsigned mask, const unsigned match) const {
        return ((insn_.raw & mask) == match);
    }

    Dyninst::Address getTarget(Dyninst::Address insnAddr) const;

    unsigned spaceToRelocate() const;
    bool getUsedRegs(std::vector<int> &regs);

    bool valid() const {
        assert(0);
        return false;
    }

    bool isCall() const;

    static bool isAligned(Dyninst::Address addr) {
        return !(addr & 0x3);
    }

    bool isBranchReg() const;
    bool isCondBranch() const;
    bool isUncondBranch() const;
    bool isThunk() const;

    bool isCleaningRet() const { return false; }

    bool isAtomicLoad() const;
    bool isAtomicStore() const;

    unsigned getTargetReg() const;
    unsigned getBranchTargetReg() const;
};

} // namespace NS_loongarch64

#endif