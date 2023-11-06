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

#ifndef _ARCH_AARCH64_H
#define _ARCH_AARCH64_H

// Code generation

#include "dyntypes.h"
#include "registers/aarch64_regs.h"
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_aarch64 {
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

#define BOp             0x05
#define BCondOp         0x2A
#define BRegOp          0xD61F
#define NOOP            0xD503201F

#define ADDShiftOp      0x2B
#define ADDImmOp        0x11
#define SUBShiftOp      0x6B
#define SUBImmOp        0x51
#define MULOp           0xD8
#define SDIVOp          0xD6

#define ORRShiftOp      0x2A
#define ANDShiftOp      0x0A
#define EORShiftOp      0x4A

#define LDRImmOp        0x1C2
#define STRImmOp        0x1C0

#define LDRFPImmOp      0x1E2
#define STRFPImmOp      0x1E0

#define LDRFPImmUOp     0xF5
#define STRFPImmUOp     0xF4

#define LDRImmUIOp      0xE5
#define STRImmUIOp      0xE4

#define LDRSWImmUIOp    0xE6

#define MSROp           0xD51
#define MRSOp           0xD53
#define MSROp           0xD51
#define MOVSPOp         0x44000

#define MIN_IMM8    (-128)
#define MAX_IMM8    (127)
#define MIN_IMM16   (-32768)
#define MAX_IMM16   (32767)
#define MIN_IMM32   (-2147483647 - 1)
#define MAX_IMM32   (2147483647)
#define MAX_IMM48   ((long)(-1 >> 17))
#define MIN_IMM48   ((long)(~MAX_IMM48))
#define MAX_IMM52   ((long)(1 << 52))
#define MIN_IMM52   ((long)(~MAX_IMM52))

//Would probably want to use the register category as well (FPR/SPR/GPR), but for the uses of these macros, this should suffice
#define SPR_LR      (((Dyninst::aarch64::x29).val()) & 0x1F)
#define SPR_NZCV    (((Dyninst::aarch64::pstate).val()) & 0x1F)
#define SPR_FPCR    (((Dyninst::aarch64::fpcr).val()) & 0x1F)
#define SPR_FPSR    (((Dyninst::aarch64::fpsr).val()) & 0x1F)

#define INSN_SET(I, s, e, v)    ((I).setBits(s, e - s + 1, (v)))

#define INSN_GET_ISCALL(I)          ((unsigned int) ((I).asInt() & 0x80000000))
#define INSN_GET_CBRANCH_OFFSET(I)  ((unsigned int) (((I).asInt() >> 5) & 0x7ffff))

#define MAX_BRANCH_OFFSET      0x07ffffff  // 128MB  Used for B
#define MAX_CBRANCH_OFFSET     0x000fffff  //   1MB  Used for B.cond, CBZ and CBNZ
#define MAX_TBRANCH_OFFSET     0x0007ffff  //  32KB  Used for TBZ and TBNZ

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
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

#define maxGPR 31           /* More space than is needed */
#define maxFPR 32           /* Save FPRs 0-13 */

// Helps to mitigate host/target endian mismatches
unsigned int swapBytesIfNeeded(unsigned int i);

class COMMON_EXPORT instruction {
	private:
    instructUnion insn_;

	public:
    instruction() { insn_.raw = 0; }
    instruction(unsigned int raw) {
        // Don't flip bits here.  Input is already in host byte order.
        insn_.raw = raw;
    }
    // Pointer creation method
    instruction(const void *ptr) {
      insn_ = *((const instructUnion *)ptr);
    }
    instruction(const void *ptr, bool) {
      insn_ = *((const instructUnion *)ptr);
    }

    instruction(const instruction &insn) :        insn_(insn.insn_) {}
    instruction(instructUnion &insn) :
        insn_(insn) {}

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


    // To solve host/target endian mismatches
    static int signExtend(unsigned int i, unsigned int pos);
    static instructUnion &swapBytes(instructUnion &i);

    // We need instruction::size() all _over_ the place.
    static unsigned size() { return sizeof(instructUnion); }

    Dyninst::Address getBranchOffset() const;
    Dyninst::Address getBranchTargetAddress() const;
    void setBranchOffset(Dyninst::Address newOffset);

    // And tell us how much space we'll need...
    // Returns -1 if we can't do a branch due to architecture limitations
    static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
    static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width);
    static unsigned maxJumpSize(unsigned addr_width);

    static unsigned maxInterFunctionJumpSize(unsigned addr_width);

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


  	bool isCleaningRet() const {return false; }

    bool isAtomicLoad( ) const;
    bool isAtomicStore( ) const;

    // inferface for being called outside this class
    unsigned getTargetReg()const ;
    unsigned getBranchTargetReg() const;
};

}
//end of NS_aarch64

#endif
