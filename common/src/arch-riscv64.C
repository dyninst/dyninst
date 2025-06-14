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

signed long instruction::signExtend(unsigned long i, unsigned int pos) {
    signed long ret;
    if (((i >> (pos - 1)) & 0x1) == 0x1) {
        ret = i | (~0UL << (pos - 1));
    } else {
        ret = i & ~(~0UL << (pos - 1));
    }
    return ret;
}

unsigned instruction::getTargetReg() const {
    if (isBranchReg()){
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

bool instruction::isBranchReg() const {
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t result = (*p) & CJR_INSN_MASK;
        return result == CJR_INSN || result == CJALR_INSN;
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        rvInsn_t result = (*p) & J_INSN_MASK;
        return result == JALR_INSN;
    }
}

bool instruction::isBranchOffset() const {
    if (isCondBranch()) {
        return true;
    }
    else if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t result = (*p) & CJ_INSN_MASK;
        return result == CJ_INSN || result == CJAL_INSN;
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        rvInsn_t result = (*p) & J_INSN_MASK;
        return result == JAL_INSN || result == JALR_INSN;
    }
}

bool instruction::isUncondBranch() const {
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t resultCJ = (*p) & CJ_INSN_MASK;
        rvcInsn_t resultCJR = (*p) & CJR_INSN_MASK;
        return resultCJ == CJ_INSN || resultCJ == CJAL_INSN
                || resultCJR == CJR_INSN || resultCJR == CJALR_INSN;
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        rvInsn_t result = (*p) & J_INSN_MASK;
        return result == JAL_INSN || result == JALR_INSN;
    }
}

bool instruction::isCondBranch() const {
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        printf("%lx\n", *p);
        rvcInsn_t result = (*p) & CB_INSN_MASK;
        return result == CBEQZ_INSN || result == CBNEZ_INSN;
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        printf("%lx\n", *p);
        rvInsn_t result = (*p) & B_INSN_MASK;
        return result == B_INSNS;
    }
}

bool instruction::getUsedRegs(std::vector<int> &regs) {
    if (isCondBranch()) {
        if (isRVC()) {
            const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
            unsigned int rs2 = CB_REG1_ADD + (((*p) & CB_REG1_MASK) >> CB_REG1_SHIFT);
            regs.push_back(rs2);
        } else {
            const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
            unsigned int rs1 = ((*p) & B_REG1_MASK) >> B_REG1_SHIFT;
            unsigned int rs2 = ((*p) & B_REG2_MASK) >> B_REG2_SHIFT;
            regs.push_back(rs1);
            regs.push_back(rs2);
        }
        return true;
    } else if (isBranchReg()) {
        regs.push_back(getBranchTargetReg());
        return true;
    } else if (isUncondBranch()) {
        return true;
    }
   
    assert(0);
    return 0;
}

bool instruction::isCall() const {
    assert(isUncondBranch());
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t result = (*p) & CJ_INSN_MASK;
        if (result == CJAL_INSN) {
            return true;
        }
        result = (*p) & CJR_INSN;
        if (result == CJALR_INSN) {
            return true;
        }
        return false;
    } else {
        return getLinkReg() == GPR_RA;
    }
}

unsigned instruction::getLinkReg() const {
    assert(isUncondBranch());
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t result = (*p) & CJ_INSN_MASK;
        if (result == CJ_INSN || result == CJR_INSN) {
            return GPR_ZERO;
        } else {
            return GPR_RA;
        }
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        return (((*p) & J_LNK_MASK) >> J_LNK_SHIFT);
    }
}

unsigned instruction::getBranchTargetReg() const {
    // keep sure this instruction is uncond b reg.
    assert(isBranchReg());
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        return (((*p) & CJR_REG_MASK) >> CJR_REG_SHIFT);
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        return (((*p) & JALR_REG_MASK) >> JALR_REG_SHIFT);
    }
}

Dyninst::Address instruction::getBranchOffset() const {
    assert(isBranchOffset());

    int offset = 0;

    if (isUncondBranch()) {
        // c.j, c.jal
        if (isRVC()) {
            for (unsigned i = 0; i < CJ_REORDER.size(); i++) {
                size_t src_bit = CJ_IMM_OFF + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << CJ_REORDER[i]);
                }
            }
        }
        // jalr
        else if (isBranchReg()) {
            for (unsigned i = 0; i < JALR_REORDER.size(); i++) {
                //offset.set(JALR_REORDER[i], insn_buff[JALR_IMM_OFF + i]);
                size_t src_bit = JALR_IMM_OFF + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << JALR_REORDER[i]);
                }
            }
        }
        // jal
        else {
            for (unsigned i = 0; i < JAL_REORDER.size(); i++) {
                //offset.set(JAL_REORDER[i], insn_buff[JAL_IMM_OFF + i]);
                size_t src_bit = JAL_IMM_OFF + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << JAL_REORDER[i]);
                }
            }
        }
    }
    else if (isCondBranch()) {
        // c.beqz, c.bnez
        if (isRVC()) {
            for (unsigned i = 0; i < CB_REORDER1.size(); i++) {
                size_t src_bit = CB_IMM_OFF1 + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << CB_REORDER1[i]);
                }
            }
            for (unsigned i = 0; i < CB_REORDER2.size(); i++) {
                size_t src_bit = CB_IMM_OFF2 + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << CB_REORDER2[i]);
                }
            }
        }
        // beq, bne, blt, bge, bltu, bgeu
        else {
            for (unsigned i = 0; i < B_REORDER1.size(); i++) {
                size_t src_bit = B_IMM_OFF1 + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << B_REORDER1[i]);
                }
            }
            for (unsigned i = 0; i < B_REORDER2.size(); i++) {
                size_t src_bit = B_IMM_OFF2 + i;
                size_t src_byte_idx = src_bit / 8;
                size_t src_bit_idx = src_bit % 8;

                unsigned src_val = (insn_buff[src_byte_idx] >> src_bit_idx) & 1;
                if (src_val) {
                    offset |= (1 << B_REORDER2[i]);
                }
            }
        }
    }
    return offset;
}

unsigned instruction::getCondBranchOp() const {
    assert (isCondBranch());
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        rvcInsn_t result = (*p) & CB_INSN_MASK;
        if (result == CBEQZ_INSN) {
            return B_COND_EQ;
        } else {
            return B_COND_NE;
        }
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        return (((*p) & B_COND_MASK) >> B_COND_SHIFT);
    }
}

unsigned instruction::getCondBranchReg1() const {
    assert (isCondBranch());
    if (isRVC()) {
        const rvcInsn_t *p = reinterpret_cast<const rvcInsn_t *>(insn_buff);
        return CB_REG1_ADD + (((*p) & CB_REG1_MASK) >> CB_REG1_SHIFT);
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        return (((*p) & B_REG1_MASK) >> B_REG1_SHIFT);
    }
}

unsigned instruction::getCondBranchReg2() const {
    assert (isCondBranch());
    if (isRVC()) {
        return GPR_ZERO;
    } else {
        const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
        return ((*p) & B_REG2_MASK) >> B_REG2_SHIFT;
    }
}

bool instruction::isAtomic() const {
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    rvInsn_t result = (*p) & A_INSN_MASK;
    return result == A_INSNS;
}

bool instruction::isAtomicMemOp() const {
    if (!isAtomic()) {
        return false;
    }
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    rvInsn_t result = (*p) & A_OP_MASK;
    return result != A_OP_LR && result != A_OP_SC;
}

bool instruction::isAtomicLoad() const {
    if (isAtomicMemOp()) {
        return true;
    }
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    rvInsn_t result = (*p) & A_OP_MASK;
    return result == A_OP_LR;
}

bool instruction::isAtomicStore() const {
    if (isAtomicMemOp()) {
        return true;
    }
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    rvInsn_t result = (*p) & A_OP_MASK;
    return result == A_OP_SC;
}

bool instruction::isAuipc() const {
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    return ((*p) & AUIPC_INSN_MASK) == AUIPC_INSN;
}

Dyninst::Address instruction::getAuipcOffset() const {
    assert(isAuipc());
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    return (*p) & AUIPC_IMM_MASK;
}

unsigned instruction::getAuipcReg() const {
    assert(isAuipc());
    const rvInsn_t *p = reinterpret_cast<const rvInsn_t *>(insn_buff);
    return ((*p) & AUIPC_REG_MASK) >> AUIPC_REG_SHIFT;
}

} // namespace NS_riscv64
