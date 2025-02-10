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

#include "common/src/arch-riscv64.h"
#include "unaligned_memory_access.h"

namespace NS_riscv64 {

ATOMIC_t ATOMIC;
UNCOND_BR_t UNCOND_BR;
COND_BR_t COND_BR;

unsigned int swapBytesIfNeeded(unsigned int i)
{
        assert(0);
    return i;
}


// i = signed int value to be extended
// pos = the total length of signed value to be extended
int instruction::signExtend(unsigned int i, unsigned int pos)
{
    int ret;
    if (((i >> (--pos)) & 0x1) == 0x1) {
        ret = i |  (~0u << pos);
    } else {
        ret = i & ~(~0u << pos);
    }
        return ret;
}

instructUnion &instruction::swapBytes(instructUnion &i)
{
        assert(0);
    return i;
}

instruction *instruction::copy() const {
        assert(0);
    return new instruction(*this);
}

unsigned instruction::getTargetReg() const {
    if( isBranchReg() ){
        // for this instruction, the reg contains the target address directly.
        return getBranchTargetReg();
    }
    return -1;
}

Dyninst::Address instruction::getTarget(Dyninst::Address addr) const {
    if (isUncondBranch() || isCondBranch()) {
        return getBranchOffset() + addr;
    }

    return 0;
}

// TODO: argument _needs_ to be an int, or ABS() doesn't work.
void instruction::setBranchOffset(Dyninst::Address /*newOffset*/) {
                assert(0);
}


bool instruction::isCall() const
{
                assert(0);
#define CALLmatch 0x48000001 /* bl */
    return false;
}

void instruction::setInstruction(codeBuf_t * /*ptr*/, Dyninst::Address) {
                assert(0);
}

void instruction::setInstruction(unsigned char *ptr, Dyninst::Address) {
    // We don't need the addr on this platform
    insn_ = Dyninst::read_memory_as<instructUnion>(ptr);
}

bool instruction::isBranchReg() const{
    if (insn_.is_compressed) {
        short code = insn_.craw & UNCOND_BR_t.CJUMP_MASK;
        return code == UNCOND_BR_t.CJR || code == UNCOND_BR_t.CJALR;
    } else {
        int code = insn_.raw & UNCOND_BR_t.JUMP_MASK;
        return code == UNCOND_BR_t.JUMP_LINK_REG;
    }
}

bool instruction::isUncondBranch() const {
    if (insn_.is_compressed) {
        short code = insn_.craw & UNCOND_BR_t.CJUMP_MASK;
        return code == UNCOND_BR_t.CJ  || code == UNCOND_BR_t.CJAL ||
               code == UNCOND_BR_t.CJR || code == UNCOND_BR_t.CJALR;
    } else {
        int code = insn_.raw & UNCOND_BR_t.JUMP_MASK;
        return code == UNCOND_BR_t.JUMP_LINK || code == UNCOND_BR_t.JUMP_LINK_REG;
    }
}

bool instruction::isCondBranch() const {
    if (insn_.is_compressed) {
        short code = insn_.craw & COND_BR_t.CBRANCH_MASK;
        return code == COND_BR_t.CBEQZ || code == COND_BR_t.CBNEZ ||
    } else {
        int code = insn_.raw & COND_BR_t.BRANCH_MASK;
        return code == COND_BR_t.BRANCH;
    }
}

unsigned instruction::jumpSize(Dyninst::Address /*from*/, Dyninst::Address /*to*/, unsigned /*addr_width*/) {
                assert(0);
                return 0;
}

// -1 is infinite, don't ya know.
unsigned instruction::jumpSize(Dyninst::Address /*disp*/, unsigned /*addr_width*/) {
                assert(0);
                return 0;
}

unsigned instruction::maxJumpSize(unsigned addr_width) {
                assert(0);
                return 0;
}

unsigned instruction::maxInterFunctionJumpSize(unsigned addr_width) {
                assert(0);
                return 0;
}

unsigned instruction::spaceToRelocate() {
                assert(0);
                return 0;
}

bool instruction::getUsedRegs(std::vector<int> &) {
                assert(0);
                return 0;
}

// A thunk is a "get PC" operation. We consider
// an instruction to be a thunk if it fulfills the following
// requirements:
//  1) It is unconditional or a "branch always" conditional
//  2) It has an offset of 4
//  3) It saves the return address in the link register
bool instruction::isThunk() const {
        assert(0);
        return true;
}

unsigned instruction::getBranchTargetReg() const{
    // keep sure this instruction is uncond b reg.
    assert (isBranchReg());
    if (insn_.is_compressed) {
        return (insn_ & UNCOND_BR_t.CJUMP_REG_MASK) >> UNCOND_BR_t.CJUMP_REG_SHIFT;
    } else {
        return (insn_ & UNCOND_BR_t.JUMP_REG_MASK) >> UNCOND_BR_t.JUMP_REG_SHIFT;
    }
}

Dyninst::Address instruction::getBranchOffset() const {
    if (isUncondBranch()) {
        if (insn_.is_compressed) {
            Dyninst::Address addr = (insn_.craw & UNCOND_BR_t.CJUMP_IMM_MASK) >> UNCOND_BR_t.CJUMP_IMM_SHIFT;;
            if ((insn_.craw & UNCOND_BR_t.CJUMP_MASK) == UNCOND_BR_t.CJ) {

            }
            else if ((insn_.craw & UNCOND_BR_t.CJUMP_MASK) == UNCOND_BR_t.CJAL) {

            }
            // Not a valid instruction
            assert(0);
        }
        else {
            if ((insn_.raw & UNCOND_BR_t.JUMP_MASK) == UNCOND_BR_T.JUMP_LINK) {

            }
            else if ((insn_.raw & UNCOND_BR_t.JUMP_MASK) == UNCOND_BR_T.JUMP_LINK_REG) {

            }
        }
    }
    else if (isUncondBranch()) {
        if (insn_.is_compressed) {

        }
        else {

        }
    }
    assert(0);
    return 0;
}

unsigned instruction::opcode() const {
    assert(0);
    return -1;
}

bool instruction::isAtomicLoad() const {
    if (insn_.is_compressed) {
        short code = insn_.craw & ATOMIC_t.CLDST_MASK;
        return code == ATOMIC_t.CLW   || code == ATOMIC_t.CLD   ||
               code == ATOMIC_t.CLWSP || code == ATOMIC_t.CLDSP ||
    } else {
        int code = insn_.raw & ATOMIC_t.LDST_MASK;
        return code == ATOMIC_t.LD;
    }
}

bool instruction::isAtomicStore() const {
    if (insn_.is_compressed) {
        short code = insn_.craw & ATOMIC_t.CLDST_MASK;
        return code == ATOMIC_t.CSW   || code == ATOMIC_t.CSD   ||
               code == ATOMIC_t.CSWSP || code == ATOMIC_t.CSDSP ||
    } else {
        int code = insn_.raw & ATOMIC_t.LDST_MASK;
        return code == ATOMIC_t.ST;
    }
}

} // namespace NS_riscv64
