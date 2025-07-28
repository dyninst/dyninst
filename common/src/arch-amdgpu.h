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

#ifndef _ARCH_AMDGPU_H
#define _ARCH_AMDGPU_H

// We primarily use the emitter for AMDGPU.
// This is there to make codegen work for the port.

#include "dyntypes.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include <assert.h>
#include <vector>
class AddressSpace;

namespace NS_amdgpu {

typedef const unsigned int insn_mask;

typedef union {
    unsigned char byte[4];
    unsigned int  raw;
} instructUnion;

typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

// Helps to mitigate host/target endian mismatches
unsigned int swapBytesIfNeeded(unsigned int i);

class DYNINST_EXPORT instruction {
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
//end of NS_amdgpu

#endif
