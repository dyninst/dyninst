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
class AddressSpace;

namespace NS_riscv64 {
/*
 * Define arch64 instruction information.
 *
 */

//#define ATOMIC_MASK    (0x3f400000)
//#define ATOMIC_LD (0x08400000)
//#define ATOMIC_ST (0x08000000)
//
//#define UNCOND_BR_IMM_MASK  (0x7c000000)
//#define UNCOND_BR_IMM       (0x14000000)
//#define UNCOND_BR_REG_MASK  (0xfe000000)
//#define UNCOND_BR_REG       (0xd6000000)

#define BREAK_POINT_INSN 0xd4200000

#define ADDImmOp        0x0
#define SLLImmOp        0x1
#define XORImmOp        0x4
#define SRLImmOp        0x5
#define SRAImmOp        0x5
#define ORImmOp         0x6
#define ANDImmOp        0x7

#define GPR_RA          1
#define GPR_SP          2

#define INSN_SET(I, s, e, v)    ((I).setBits(s, e - s + 1, (v)))
#define INSN_C_SET(I, s, e, v)    ((I).setCBits(s, e - s + 1, (v)))

#define INSN_GET_ISCALL(I)          ((unsigned int) ((I).asInt() & 0x80000000))
#define INSN_GET_CBRANCH_OFFSET(I)  ((unsigned int) (((I).asInt() >> 5) & 0x7ffff))

#define MAX_BRANCH_OFFSET       0xfffff // 20 bits
#define MAX_BRANCH_LINK_OFFSET  0xfff // 12 bits

#define CHECK_INST(isInst) \
    !((insn_.raw&isInst##_MASK)^isInst)

#define GET_OFFSET32(thisInst) \
    (((insn_.raw&thisInst##_OFFSET_MASK)>>thisInst##_OFFSHIFT)<<2)

typedef const unsigned int insn_mask;
class ATOMIC_t {
public:
    static insn_mask LD_MASK =  (0x3f400000);
    static insn_mask ST_MASK =  (0x3f400000);
    static insn_mask LD =    (0x08400000);
    static insn_mask ST =    (0x08000000);
};

class UNCOND_BR_t {
public:
    static insn_mask IMM_MASK  =(0x7c000000);
    static insn_mask IMM       =(0x14000000);
    static insn_mask IMM_OFFSET_MASK   =(0x03ffffff);
    static insn_mask IMM_OFFSHIFT   = 0;
    static insn_mask REG_MASK  =(0xfe000000);
    static insn_mask REG       =(0xd6000000);
    static insn_mask REG_OFFSET_MASK   =(0x000001e0);
    static insn_mask REG_OFFSHIFT   =5;
};


class COND_BR_t {
public:
    static insn_mask BR_MASK = 0xfe000000; // conditional br mask
    static insn_mask CB_MASK = 0x7e000000; // comp&B
    static insn_mask TB_MASK = 0x7e000000; // test&B

    static insn_mask BR =      0x54000000; // Conditional B
    static insn_mask CB =      0x34000000; // Compare & B
    static insn_mask TB =      0x36000000; // Test & B

    static insn_mask CB_OFFSET_MASK = 0x07fffff0;
    static insn_mask TB_OFFSET_MASK = 0x0007fff0;
    static insn_mask BR_OFFSET_MASK = 0x07fffff0;

    static insn_mask CB_OFFSHIFT = 4;
    static insn_mask TB_OFFSHIFT = 4;
    static insn_mask BR_OFFSHIFT = 4;
};

typedef union {
    unsigned char byte[4];
    unsigned int  raw;
    unsigned char cbyte[2]; // Compressed Instructions
    unsigned short craw;    // Compressed Instructions
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

#define maxGPR 32           /* More space than is needed */
#define maxFPR 32           /* Save FPRs 0-13 */

// Helps to mitigate host/target endian mismatches
unsigned int swapBytesIfNeeded(unsigned int i);

class DYNINST_EXPORT instruction {
	private:
    instructUnion insn_;
    bool is_compressed;

	public:
    instruction(): insn_(), is_compressed(false) {}
    instruction(unsigned int raw) {
        // Don't flip bits here
        insn_.raw = raw;
        is_compressed = false;
    }
    instruction(unsigned short craw) {
        // Don't flip bits here
        insn_.craw = craw;
        is_compressed = true;
    }
    // Pointer creation method
    instruction(const void *ptr, const bool compressed) {
      insn_ = *((const instructUnion *)ptr);
      is_compressed = compressed;
    }

    instruction(const instruction &insn) : insn_(insn.insn_), is_compressed(insn.is_compressed) {}
    instruction(instructUnion &insn) :
        insn_(insn) {}

    instruction *copy() const;

    void clear() { insn_ = instructUnion(); }
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
    void setCBits(unsigned int pos, unsigned int len, unsigned short value) {
        unsigned short mask;

        mask = ~((unsigned short)(~0) << len);
        value = value & mask;

        mask = ~(mask << pos);
        value = value << pos;

        insn_.craw = insn_.craw & mask;
        insn_.craw = insn_.craw | value;
    }

    bool isCompressed() const { return is_compressed; }
    unsigned int asInt() const { return insn_.raw; }
    unsigned int asShort() const { return insn_.craw; }
    void setInstruction(unsigned char *ptr, Dyninst::Address = 0);


    // To solve host/target endian mismatches
    static int signExtend(unsigned int i, unsigned int pos);
    static instructUnion &swapBytes(instructUnion &i);

    // We need instruction::size() all _over_ the place.
    unsigned size() { return is_compressed ? 2 : 4; }

    Dyninst::Address getBranchOffset() const;
    Dyninst::Address getBranchTargetAddress() const;
    void setBranchOffset(Dyninst::Address newOffset);

    // And tell us how much space we'll need...
    // Returns -1 if we can't do a branch due to architecture limitations
    unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
    unsigned jumpSize(Dyninst::Address disp, unsigned addr_width);
    unsigned maxJumpSize(unsigned addr_width);

    unsigned maxInterFunctionJumpSize(unsigned addr_width);

    // return the type of the instruction
    unsigned type() const;

    // return a pointer to the instruction
    const unsigned char *ptr() const { return (const unsigned char *)&insn_; }

    // For external modification
    // Don't allow external modification anymore.  Host byte order may differ
    // from target byte order.
    //instructUnion &operator* () { return insn_; }
    //const instructUnion &operator* () const { return insn_; }
    //const unsigned int &raw() const { return insn_.raw; }

    unsigned opcode() const;

    // Local version
    bool isInsnType(const unsigned mask, const unsigned match) const {
        return ((insn_.raw & mask) == match);
    }
    bool isInsnType(const unsigned short mask, const unsigned short match) const {
        return ((insn_.craw & mask) == match);
    }

    Dyninst::Address getTarget(Dyninst::Address insnAddr) const;

    unsigned spaceToRelocate();
    bool getUsedRegs(std::vector<int> &regs);

    bool valid() const {
			assert(0);
			return false;
		}

    bool isCall() const;

    static bool isAligned(Dyninst::Address addr) {
        return !(addr & 0x1);
    }

    bool isBranchReg() const;
    bool isCondBranch() const;
    bool isUncondBranch() const;
    bool isThunk() const;


  	bool isCleaningRet() const {return false; }

    bool isAtomicLoad( ) const;
    bool isAtomicStore( ) const;

    // inferface for being called outside this class
    unsigned getTargetReg()const ;
    unsigned getBranchTargetReg() const;
};

}
//end of NS_riscv64

#endif
