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
    if (is_compressed) {
        unsigned short code = insn_.craw & UNCOND_BR.CJR_INSN_MASK;
        return code == UNCOND_BR.CJR_INSN || code == UNCOND_BR.CJALR_INSN;
    } else {
        unsigned int code = insn_.raw & UNCOND_BR.J_INSN_MASK;
        return code == UNCOND_BR.JALR_INSN;
    }
}

bool instruction::isBranchOffset() const{
    if (isCondBranch()) {
        return true;
    }
    else if (is_compressed) {
        unsigned short code = insn_.craw & UNCOND_BR.CJR_INSN_MASK;
        return code == UNCOND_BR.CJ_INSN || code == UNCOND_BR.CJAL_INSN;
    } else {
        unsigned int code = insn_.raw & UNCOND_BR.J_INSN_MASK;
        return code == UNCOND_BR.JAL_INSN || code == UNCOND_BR.JALR_INSN;
    }
}

bool instruction::isUncondBranch() const {
    if (is_compressed) {
        unsigned short code = insn_.craw & UNCOND_BR.CJ_INSN_MASK;
        return code == UNCOND_BR.CJ_INSN || code == UNCOND_BR.CJAL_INSN
                || code == UNCOND_BR.CJR_INSN || code == UNCOND_BR.CJALR_INSN;
    } else {
        unsigned int code = insn_.raw & UNCOND_BR.J_INSN_MASK;
        return code == UNCOND_BR.JAL_INSN || code == UNCOND_BR.JALR_INSN;
    }
}

bool instruction::isCondBranch() const {
    if (is_compressed) {
        unsigned short code = insn_.craw & COND_BR.CBRANCH_MASK;
        return code == COND_BR.CBEQZ_INSN || code == COND_BR.CBNEZ_INSN;
    } else {
        unsigned int code = insn_.raw & COND_BR.BRANCH_MASK;
        return code == COND_BR.BRANCH_INSNS;
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

unsigned instruction::maxJumpSize(unsigned /*addr_width*/) {
                assert(0);
                return 0;
}

unsigned instruction::maxInterFunctionJumpSize(unsigned /*addr_width*/) {
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
    if (is_compressed) {
        return (insn_.craw & UNCOND_BR.CJR_REG_MASK) >> UNCOND_BR.CJR_REG_SHIFT;
    } else {
        return (insn_.raw & UNCOND_BR.JALR_REG_MASK) >> UNCOND_BR.JALR_REG_SHIFT;
    }
}

Dyninst::Address instruction::getBranchOffset() const {
    assert( isBranchOffset() );
    Dyninst::Address offset = 0;
    if (isUncondBranch()) {
        // c.j, c.jal
        if (is_compressed) {
            Dyninst::Address imm = (insn_.craw & UNCOND_BR.JALR_IMM_MASK) >> UNCOND_BR.JALR_IMM_SHIFT;
            // TODO refactor
            offset |= ((imm >> 10) & 0x1) << 11;
            offset |= ((imm >>  9) & 0x1) << 4;
            offset |= ((imm >>  8) & 0x1) << 9;
            offset |= ((imm >>  7) & 0x1) << 8;
            offset |= ((imm >>  6) & 0x1) << 10;
            offset |= ((imm >>  5) & 0x1) << 6;
            offset |= ((imm >>  4) & 0x1) << 7;
            offset |= ((imm >>  3) & 0x1) << 3;
            offset |= ((imm >>  2) & 0x1) << 2;
            offset |= ((imm >>  1) & 0x1) << 1;
            offset |= ((imm >>  0) & 0x1) << 5;
        }
        // jalr
        else if (isBranchReg()) {
            offset = (insn_.craw & UNCOND_BR.JALR_IMM_MASK) >> UNCOND_BR.JALR_IMM_SHIFT;
        }
        // jal
        else {
            Dyninst::Address imm = (insn_.craw & UNCOND_BR.JAL_IMM_MASK) >> UNCOND_BR.JAL_IMM_SHIFT;
            offset |= ((imm >> 19) &   0x1) << 20;
            offset |= ((imm >>  9) & 0x1ff) <<  1;
            offset |= ((imm >>  8) &   0x1) << 11;
            offset |= ((imm >>  0) &  0xff) << 12;
        }
    }
    else if (isUncondBranch()) {
        // c.beqz, c.bnez
        if (is_compressed) {
            Dyninst::Address imm = (insn_.craw & COND_BR.CBRANCH_IMM_MASK);
            offset |= ((imm >> 12) &  0x1) << 8;
            offset |= ((imm >> 10) &  0x3) << 3;
            offset |= ((imm >>  5) &  0x3) << 6;
            offset |= ((imm >>  3) &  0x3) << 1;
            offset |= ((imm >>  2) &  0x1) << 5;
        }
        // beq, bne, blt, bge, bltu, bgeu
        else {
            Dyninst::Address imm = (insn_.craw & COND_BR.BRANCH_IMM_MASK);
            offset |= ((imm >> 31) &  0x1) << 12;
            offset |= ((imm >> 25) & 0x3f) <<  5;
            offset |= ((imm >>  8) &  0xf) <<  1;
            offset |= ((imm >>  7) &  0x1) << 11;
        }
    }
    return offset;
}

unsigned instruction::opcode() const {
    assert(0);
    return -1;
}

bool instruction::isAtomicLoad() const {
    if (is_compressed) {
        unsigned short code = insn_.craw & ATOMIC.CLDST_INSN_MASK;
        return code == ATOMIC.CLW_INSN   || code == ATOMIC.CLD_INSN   ||
               code == ATOMIC.CLWSP_INSN || code == ATOMIC.CLDSP_INSN;
    } else {
        unsigned int code = insn_.raw & ATOMIC.LDST_INSN_MASK;
        return code == ATOMIC.LD_INSN;
    }
}

bool instruction::isAtomicStore() const {
    if (is_compressed) {
        unsigned short code = insn_.craw & ATOMIC.CLDST_INSN_MASK;
        return code == ATOMIC.CSW_INSN   || code == ATOMIC.CSD_INSN   ||
               code == ATOMIC.CSWSP_INSN || code == ATOMIC.CSDSP_INSN;
    } else {
        unsigned int code = insn_.raw & ATOMIC.LDST_INSN_MASK;
        return code == ATOMIC.ST_INSN;
    }
}

} // namespace NS_riscv64
