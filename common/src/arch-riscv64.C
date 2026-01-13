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

// Auipc Instruction

constexpr int32_t AUIPC_INSN_MASK = 0x0000007f;
constexpr int32_t AUIPC_INSN = 0x00000017;
constexpr int32_t AUIPC_IMM_MASK = 0xfffff000;
constexpr int32_t AUIPC_REG_MASK = 0x00000f80;
constexpr int32_t AUIPC_REG_SHIFT = 7;

// Jump/Branch instructions

constexpr int32_t BRANCH_COND_EQ = NS_riscv64::BEQFunct3;
constexpr int32_t BRANCH_COND_NE = NS_riscv64::BNEFunct3;

constexpr int32_t BRANCH_INSN_MASK = 0x0000007f;
constexpr int32_t BRANCH_INSNS = 0x00000063;
constexpr int32_t CBEQZ_INSN = 0xc001;
constexpr int32_t CBNEZ_INSN = 0xe001;
constexpr int32_t CBRANCH_INSN_MASK = 0xe003;
constexpr int32_t CJAL_INSN = 0x2001;
constexpr int32_t CJALR_INSN = 0x9002;
constexpr int32_t CJR_INSN = 0x8002;
constexpr int32_t CJR_INSN_MASK = 0xf003;
constexpr int32_t CJUMP_INSN = 0xa001;
constexpr int32_t CJUMP_INSN_MASK = 0xe003;
constexpr int32_t JAL_INSN = 0x0000006f;
constexpr int32_t JALR_INSN = 0x00000067;
constexpr int32_t JUMP_INSN_MASK = 0x0000007f;

// The following are the indices of the immediates' offset
const int32_t JAL_IMM_OFF = 12;
const int32_t JALR_IMM_OFF = 20;
const int32_t CJUMP_IMM_OFF = 2;
const int32_t BRANCH_IMM_OFF1 = 7;
const int32_t BRANCH_IMM_OFF2 = 25;
const int32_t CBRANCH_IMM_OFF1 = 2;
const int32_t CBRANCH_IMM_OFF2 = 10;

// Register mask for jump/branch instructions
constexpr int32_t BRANCH_COND_MASK = 0x00007000;
constexpr int32_t BRANCH_REG1_MASK = 0x000f8000;
constexpr int32_t BRANCH_REG2_MASK = 0x01f00000;
constexpr int32_t CBRANCH_REG1_MASK = 0x0380;
constexpr int32_t CJR_REG_MASK = 0x0f80;
constexpr int32_t JALR_REG_MASK = 0x000f8000;

// Register shift for jump/branch instructions
constexpr int32_t BRANCH_COND_SHIFT = 12;
constexpr int32_t BRANCH_REG1_SHIFT = 15;
constexpr int32_t BRANCH_REG2_SHIFT = 20;
constexpr int32_t CBRANCH_REG1_ADD = 8;
constexpr int32_t CBRANCH_REG1_SHIFT = 7;
constexpr int32_t CJR_REG_SHIFT = 7;
constexpr int32_t JALR_REG_SHIFT = 15;
constexpr int32_t JUMP_LNK_MASK = 0x00000f80;
constexpr int32_t JUMP_LNK_SHIFT = 7;

// Atomic Instructions
constexpr int32_t ATOMIC_INSN_MASK = 0x0000007f;
constexpr int32_t ATOMIC_INSNS = 0x0000002f;
constexpr int32_t ATOMIC_OP_MASK = 0xf8000000;
constexpr int32_t ATOMIC_OP_LR = 0x10000000;
constexpr int32_t ATOMIC_OP_SC = 0x18000000;

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

bool instruction::isBranchReg() const {
  if (isCompressed()) {
    int32_t result = insn_.raw & CJR_INSN_MASK;
    return result == CJR_INSN || result == CJALR_INSN;
  } else {
    int32_t result = insn_.raw & JUMP_INSN_MASK;
    return result == JALR_INSN;
  }
}

bool instruction::isBranchOffset() const {
  if (isCondBranch()) {
    return true;
  } else if (isCompressed()) {
    int32_t result = insn_.raw & CJUMP_INSN_MASK;
    return result == CJUMP_INSN || result == CJAL_INSN;
  } else {
    int32_t result = insn_.raw & JUMP_INSN_MASK;
    return result == JAL_INSN || result == JALR_INSN;
  }
}

bool instruction::isUncondBranch() const {
  if (isCompressed()) {
    int32_t resultCJ = insn_.raw & CJUMP_INSN_MASK;
    int32_t resultCJR = insn_.raw & CJR_INSN_MASK;
    return resultCJ == CJUMP_INSN || resultCJ == CJAL_INSN ||
           resultCJR == CJR_INSN || resultCJR == CJALR_INSN;
  } else {
    int32_t result = insn_.raw & JUMP_INSN_MASK;
    return result == JAL_INSN || result == JALR_INSN;
  }
}

bool instruction::isCondBranch() const {
  if (isCompressed()) {
    int32_t result = insn_.raw & CBRANCH_INSN_MASK;
    return result == CBEQZ_INSN || result == CBNEZ_INSN;
  } else {
    int32_t result = insn_.raw & BRANCH_INSN_MASK;
    return result == BRANCH_INSNS;
  }
}

bool instruction::getUsedRegs(std::vector<int> &regs) {
  if (isCondBranch()) {
    if (isCompressed()) {
      unsigned int rs2 = CBRANCH_REG1_ADD + ((insn_.raw & CBRANCH_REG1_MASK) >>
                                             CBRANCH_REG1_SHIFT);
      regs.push_back(rs2);
    } else {
      unsigned int rs1 = ((insn_.raw & BRANCH_REG1_MASK) >> BRANCH_REG1_SHIFT);
      unsigned int rs2 = ((insn_.raw & BRANCH_REG2_MASK) >> BRANCH_REG2_SHIFT);
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

  return 0;
}

bool instruction::isCall() const {
  if (!isUncondBranch()) {
    return false;
  }
  if (isCompressed()) {
    int32_t result = insn_.raw & CJUMP_INSN_MASK;
    if (result == CJAL_INSN) {
      return true;
    }
    result = insn_.raw & CJR_INSN;
    if (result == CJALR_INSN) {
      return true;
    }
    return false;
  } else {
    return getLinkReg() == GPR_RA;
  }
}

unsigned instruction::getLinkReg() const {
  if (!isUncondBranch()) {
    return false;
  }
  if (isCompressed()) {
    int32_t result = insn_.raw & CJUMP_INSN_MASK;
    if (result == CJUMP_INSN || result == CJR_INSN) {
      return GPR_ZERO;
    } else {
      return GPR_RA;
    }
  } else {
    return ((insn_.raw & JUMP_LNK_MASK) >> JUMP_LNK_SHIFT);
  }
}

unsigned instruction::getBranchTargetReg() const {
  // keep sure this instruction is uncond b reg.
  if (!isBranchReg()) {
    return false;
  }
  if (isCompressed()) {
    return ((insn_.raw & CJR_REG_MASK) >> CJR_REG_SHIFT);
  } else {
    return ((insn_.raw & JALR_REG_MASK) >> JALR_REG_SHIFT);
  }
}

Dyninst::Address instruction::getBranchOffset() const {
  if (!isBranchOffset()) {
    return -1;
  }

  uint32_t offset = 0;

  if (isUncondBranch()) {
    // c.j, c.jal
    if (isCompressed()) {
      for (unsigned i = 0; i < CJUMP_REORDER.size(); i++) {
        offset |= ((insn_.raw >> (CJUMP_IMM_OFF + i)) & 1) << CJUMP_REORDER[i];
      }
    }
    // jalr
    else if (isBranchReg()) {
      for (unsigned i = 0; i < JALR_REORDER.size(); i++) {
        offset |= ((insn_.raw >> (JALR_IMM_OFF + i)) & 1) << JALR_REORDER[i];
      }
    }
    // jal
    else {
      for (unsigned i = 0; i < JAL_REORDER.size(); i++) {
        offset |= ((insn_.raw >> (JAL_IMM_OFF + i)) & 1) << JAL_REORDER[i];
      }
    }
  } else if (isCondBranch()) {
    // c.beqz, c.bnez
    if (isCompressed()) {
      for (unsigned i = 0; i < CBRANCH_REORDER1.size(); i++) {
        offset |= ((insn_.raw >> (CBRANCH_IMM_OFF1 + i)) & 1)
                  << CBRANCH_REORDER1[i];
      }
      for (unsigned i = 0; i < CBRANCH_REORDER2.size(); i++) {
        offset |= ((insn_.raw >> (CBRANCH_IMM_OFF2 + i)) & 1)
                  << CBRANCH_REORDER2[i];
      }
    }
    // beq, bne, blt, bge, bltu, bgeu
    else {
      for (unsigned i = 0; i < BRANCH_REORDER1.size(); i++) {
        offset |= ((insn_.raw >> (BRANCH_IMM_OFF1 + i)) & 1)
                  << BRANCH_REORDER1[i];
      }
      for (unsigned i = 0; i < BRANCH_REORDER2.size(); i++) {
        offset |= ((insn_.raw >> (BRANCH_IMM_OFF2 + i)) & 1)
                  << BRANCH_REORDER2[i];
      }
    }
  }
  return offset;
}

unsigned instruction::getCondBranchOp() const {
  if (!isCondBranch()) {
    return false;
  }
  if (isCompressed()) {
    int32_t result = insn_.raw & CBRANCH_INSN_MASK;
    if (result == CBEQZ_INSN) {
      return BRANCH_COND_EQ;
    } else {
      return BRANCH_COND_NE;
    }
  } else {
    return ((insn_.raw & BRANCH_COND_MASK) >> BRANCH_COND_SHIFT);
  }
}

unsigned instruction::getCondBranchReg1() const {
  if (isCondBranch()) {
    return false;
  }
  if (isCompressed()) {
    return CBRANCH_REG1_ADD +
           ((insn_.raw & CBRANCH_REG1_MASK) >> CBRANCH_REG1_SHIFT);
  } else {
    return ((insn_.raw & BRANCH_REG1_MASK) >> BRANCH_REG1_SHIFT);
  }
}

unsigned instruction::getCondBranchReg2() const {
  if (isCondBranch()) {
    return false;
  }
  if (isCompressed()) {
    return GPR_ZERO;
  } else {
    return ((insn_.raw & BRANCH_REG2_MASK) >> BRANCH_REG2_SHIFT);
  }
}

bool instruction::isAtomic() const {
  int32_t result = insn_.raw & ATOMIC_INSN_MASK;
  return result == ATOMIC_INSNS;
}

bool instruction::isAtomicMemOp() const {
  if (!isAtomic()) {
    return false;
  }
  int32_t result = insn_.raw & ATOMIC_OP_MASK;
  return result != ATOMIC_OP_LR && result != ATOMIC_OP_SC;
}

bool instruction::isAtomicLoad() const {
  if (isAtomicMemOp()) {
    return true;
  }
  int32_t result = insn_.raw & ATOMIC_OP_MASK;
  return result == ATOMIC_OP_LR;
}

bool instruction::isAtomicStore() const {
  if (isAtomicMemOp()) {
    return true;
  }
  int32_t result = insn_.raw & ATOMIC_OP_MASK;
  return result == ATOMIC_OP_SC;
}

bool instruction::isAuipc() const {
  return (insn_.raw & AUIPC_INSN_MASK) == AUIPC_INSN;
}

Dyninst::Address instruction::getAuipcOffset() const {
  if (isAuipc()) {
    return -1;
  }
  return static_cast<Dyninst::Address>(insn_.raw & AUIPC_IMM_MASK);
}

unsigned instruction::getAuipcReg() const {
  if (isAuipc()) {
    return -1;
  }
  return ((insn_.raw & AUIPC_REG_MASK) >> AUIPC_REG_SHIFT);
}

} // namespace NS_riscv64
