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

#include "common/src/arch-loongarch64.h"
#include "unaligned_memory_access.h"

namespace NS_loongarch64 {

int instruction::signExtend(unsigned int i, unsigned int pos)
{
    int ret;
    if (((i >> (--pos)) & 0x1) == 0x1) {
        ret = i | (~0u << pos);
    } else {
        ret = i & ~(~0u << pos);
    }
    return ret;
}

instruction *instruction::copy() const {
    assert(0);
    return new instruction(*this);
}

unsigned instruction::getTargetReg() const {
    if (isBranchReg()) {
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

void instruction::setBranchOffset(Dyninst::Address /*newOffset*/) {
    assert(0);
}

bool instruction::isCall() const {
    // BL instruction: opcode 0x6 (bits 31:26)
    return (insn_.raw & 0xFC000000) == 0x54000000;
}

void instruction::setInstruction(codeBuf_t * /*ptr*/, Dyninst::Address) {
    assert(0);
}

void instruction::setInstruction(unsigned char *ptr, Dyninst::Address) {
    insn_ = Dyninst::read_memory_as<instructUnion>(ptr);
}

bool instruction::isBranchReg() const {
    // JIRL instruction: opcode 0x13 (bits 31:26)
    return (insn_.raw & 0xFC000000) == 0x4C000000;
}

bool instruction::isUncondBranch() const {
    // B instruction: opcode 0x6 (bits 31:26)
    if ((insn_.raw & 0xFC000000) == 0x50000000)
        return true;
    // JIRL
    if (isBranchReg())
        return true;
    return false;
}

bool instruction::isCondBranch() const {
    // BEQ/BNE/BLT/BGE/BLTU/BGEU: opcode 0x4/0x5 (bits 31:26)
    unsigned int op = (insn_.raw >> 26) & 0x3F;
    if (op == 0x10 || op == 0x11 || op == 0x12 || op == 0x13 ||
        op == 0x14 || op == 0x15 || op == 0x16 || op == 0x17)
        return true;
    // BEQZ/BNEZ: opcode 0x4/0x5 with specific format
    if (op == 0x04 || op == 0x05)
        return true;
    return false;
}

unsigned instruction::jumpSize(Dyninst::Address /*from*/, Dyninst::Address /*to*/, unsigned /*addr_width*/) {
    assert(0);
    return -1;
}

unsigned instruction::jumpSize(Dyninst::Address /*disp*/, unsigned /*addr_width*/) {
    assert(0);
    return instruction::size();
}

unsigned instruction::maxJumpSize(unsigned addr_width) {
    assert(0);
    if (addr_width == 4)
        return 30 * instruction::size();
    else
        return 7 * instruction::size();
}

unsigned instruction::maxInterFunctionJumpSize(unsigned addr_width) {
    assert(0);
    if (addr_width == 8)
        return 7 * instruction::size();
    else
        return 4 * instruction::size();
}

unsigned instruction::spaceToRelocate() const {
    assert(0);
    return instruction::size();
}

bool instruction::getUsedRegs(std::vector<int> &) {
    assert(0);
    return false;
}

bool instruction::isThunk() const {
    assert(0);
    return true;
}

unsigned instruction::getBranchTargetReg() const {
    assert(isUncondBranch());
    // For JIRL, rd field (bits 4:0) is the target register
    if (isBranchReg()) {
        return insn_.raw & 0x1F;
    }
    return -1;
}

Dyninst::Address instruction::getBranchOffset() const {
    if (isUncondBranch()) {
        // B instruction: 26-bit signed offset (bits 25:0) << 2
        if ((insn_.raw & 0xFC000000) == 0x50000000) {
            return signExtend(insn_.raw & 0x03FFFFFF, 26 + 2);
        }
        // JIRL: 16-bit signed offset (bits 25:10) << 2
        if (isBranchReg()) {
            return signExtend((insn_.raw >> 10) & 0xFFFF, 16 + 2);
        }
    } else if (isCondBranch()) {
        // BEQ/BNE/etc: 16-bit signed offset (bits 25:10) << 2
        if ((insn_.raw & 0xFC000000) == 0x58000000) {
            return signExtend((insn_.raw >> 10) & 0xFFFF, 16 + 2);
        }
        // BEQZ/BNEZ: 21-bit signed offset (bits 25:10, 4:0) << 2
        if ((insn_.raw & 0xFC000000) == 0x40000000) {
            return signExtend((insn_.raw >> 10) & 0xFFFF, 16 + 2);
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
    // LL.W/LL.D instructions
    return (insn_.raw & 0xFF000000) == 0x20000000;
}

bool instruction::isAtomicStore() const {
    // SC.W/SC.D instructions
    return (insn_.raw & 0xFF000000) == 0x21000000;
}

} // namespace NS_loongarch64