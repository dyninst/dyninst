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

#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emit-riscv64.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/inst-riscv64.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/registerSpace.h"
#include <stdlib.h>

void insnCodeGen::generate(codeGen &gen, instruction &insn) {
  gen.copy(insn.ptr(), insn.size());
}

void insnCodeGen::generate(codeGen &gen, instruction &insn, unsigned position) {
  gen.insert(insn.ptr(), insn.size(), position);
}

// Basic RISC-V instruction type generation

int insnCodeGen::makeUTypeInsn(codeGen &gen, Register rd, RegValue imm,
                               unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 12, 31, imm);  // imm
  INSN_SET(insn, 7, 11, rd);    // rd
  INSN_SET(insn, 0, 6, opcode); // opcode

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::makeITypeInsn(codeGen &gen, Register rd, Register rs,
                               RegValue imm, unsigned funct3, unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 20, 31, imm);    // imm
  INSN_SET(insn, 15, 19, rs);     // rs
  INSN_SET(insn, 12, 14, funct3); // 111
  INSN_SET(insn, 7, 11, rd);      // rd
  INSN_SET(insn, 0, 6, opcode);   // opcode

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::makeRTypeInsn(codeGen &gen, Register rd, Register rs1,
                               Register rs2, unsigned funct7, unsigned funct3,
                               unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 25, 31, funct7); // funct7
  INSN_SET(insn, 20, 24, rs2);    // rs2
  INSN_SET(insn, 15, 19, rs1);    // rs1
  INSN_SET(insn, 12, 14, funct3); // funct3
  INSN_SET(insn, 7, 11, rd);      // rd
  INSN_SET(insn, 0, 6, opcode);   // opcode

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::makeBTypeInsn(codeGen &gen, Register rs1, Register rs2,
                               RegValue imm, unsigned funct3, unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 31, 31, (imm >> 11) & 0x1); // imm[11]
  INSN_SET(insn, 25, 30, (imm >> 4) & 0x3f); // imm[9:4]
  INSN_SET(insn, 20, 24, rs2);               // rs2
  INSN_SET(insn, 15, 19, rs1);               // rs1
  INSN_SET(insn, 12, 14, funct3);            // funct3
  INSN_SET(insn, 8, 11, imm & 0xf);          // imm[3:0]
  INSN_SET(insn, 7, 7, (imm >> 10) & 0x1);   // imm[10]
  INSN_SET(insn, 0, 6, opcode);              // opcode

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::makeJTypeInsn(codeGen &gen, Register rd, RegValue imm,
                               unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 31, 31, (imm >> 19) & 0x1);  // imm[19]
  INSN_SET(insn, 21, 30, (imm & 0x3ff));      // imm[9:0]
  INSN_SET(insn, 20, 20, (imm >> 10) & 0x1);  // imm[10]
  INSN_SET(insn, 12, 19, (imm >> 11) & 0xff); // imm[18:11]
  INSN_SET(insn, 7, 11, rd);                  // rd
  INSN_SET(insn, 0, 6, opcode);               // opcode

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::makeSTypeInsn(codeGen &gen, Register rs1, Register rs2,
                               RegValue imm, unsigned funct3, unsigned opcode) {
  instruction insn;

  INSN_SET(insn, 25, 31, (imm >> 5) & 0x7f); // imm[11:5]
  INSN_SET(insn, 20, 24, rs2);               // rs2
  INSN_SET(insn, 15, 19, rs1);               // rs1
  INSN_SET(insn, 12, 14, funct3);            // funct3
  INSN_SET(insn, 7, 11, imm & 0x1f);         // imm[4:0]
  INSN_SET(insn, 0, 6, opcode);              // opcode = 0100011 (store)

  insnCodeGen::generate(gen, insn);

  return RISCV_INSN_SIZE;
}

// Must have methods
// These methods are called throughout the DyninstAPI

int insnCodeGen::generateIllegal(codeGen &gen) {
  // All bytes 0 is an illegal instruction on RISC-V
  instruction insn;
  generate(gen, insn);

  return RISCV_INSN_SIZE;
}

int insnCodeGen::generateTrap(codeGen &gen) {
  if (gen.useCompressed()) {
    instruction insn;
    INSN_SET(insn, 0, 15, C_EBREAK_INSN_ENC);
    generate(gen, insn);
    return RISCVC_INSN_SIZE;
  }
  instruction insn;
  INSN_SET(insn, 0, 31, EBREAK_INSN_ENC);
  generate(gen, insn);
  return RISCV_INSN_SIZE;
}

int insnCodeGen::generateNOOP(codeGen &gen, unsigned size) {
  int num = 0;
  int code_size = 0;
  if (gen.useCompressed()) {
    assert((size % RISCVC_INSN_SIZE) == 0);
    num = (size / RISCVC_INSN_SIZE);
  } else {
    assert((size % RISCV_INSN_SIZE) == 0);
    num = (size / RISCV_INSN_SIZE);
  }
  for (int i = 0; i < num; i++) {
    code_size += generateNop(gen, gen.useCompressed());
  }
  return code_size;
}

int insnCodeGen::generateCall(codeGen &gen, Address from, Address to) {
  return generateBranch(gen, from, to);
}

int insnCodeGen::generateShortBranch(codeGen &gen, Address from, Address to,
                                     bool isCall) {
  RegValue disp = to - from;
  assert(disp >= JTYPE_IMM_MIN && disp < JTYPE_IMM_MAX);

  if (isCall) {
    return generateJal(gen, GPR_RA, disp, gen.useCompressed());
  }
  return generateJ(gen, disp, gen.useCompressed());
}

int insnCodeGen::generateLongBranch(codeGen &gen, Address from, Address to,
                                    bool isCall) {
  RegValue disp = (to - from);
  int code_size = 0;

  if (isCall) {
    // use the link register ra as scratch as it will be overwritten at return

    // load pc to ra
    if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
      RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
      RegValue offset = disp & ITYPE_IMM_MASK;
      if (offset & 0x800) {
        top = (top + 1) & UTYPE_IMM_MASK;
      }
      code_size += generateAuipc(gen, GPR_RA, top, gen.useCompressed());
      code_size +=
          generateAddi(gen, GPR_RA, GPR_RA, offset, gen.useCompressed());
      return code_size;
    }
    Register scratch = gen.rs()->getScratchRegister(gen, true);
    if (scratch == Null_Register) {
      return generateBranchViaTrap(gen, from, to);
    }
    code_size += generateAuipc(gen, GPR_RA, 0, gen.useCompressed());
    code_size += generateCalcImm(gen, scratch, disp, true, gen.useCompressed());
    code_size += generateAdd(gen, GPR_RA, GPR_RA, scratch, gen.useCompressed());
    // generate jalr
    code_size += generateJalr(gen, GPR_RA, GPR_RA, 0, gen.useCompressed());
    return code_size;
  }
  Register scratch = Null_Register;

  instPoint *point = gen.point();
  if (point) {
    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);
    scratch = rs->getScratchRegister(gen, true);
  }

  if (scratch == Null_Register) {
    return generateBranchViaTrap(gen, from, to);
  }

  if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
    RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
    RegValue offset = disp & ITYPE_IMM_MASK;
    if (offset & 0x800) {
      top = (top + 1) & UTYPE_IMM_MASK;
    }
    code_size += generateAuipc(gen, scratch, top, gen.useCompressed());
    code_size +=
        generateAddi(gen, scratch, scratch, offset, gen.useCompressed());
    return code_size;
  }
  std::vector<Register> exclude;
  exclude.push_back(scratch);
  Register scratch2 = gen.rs()->getScratchRegister(gen, exclude, true);
  if (scratch2 == Null_Register) {
    return generateBranchViaTrap(gen, from, to);
  }
  code_size += generateAuipc(gen, GPR_RA, 0, gen.useCompressed());
  code_size += generateCalcImm(gen, scratch2, disp, true, gen.useCompressed());
  code_size +=
      generateAdd(gen, scratch, scratch, scratch2, gen.useCompressed());

  code_size += generateJr(gen, scratch, 0, gen.useCompressed());
  return code_size;
}

int insnCodeGen::generateBranch(codeGen &gen, long disp, bool link) {
  if (link) {
    return generateJal(gen, GPR_RA, disp, gen.useCompressed());
  }
  return generateJ(gen, disp, gen.useCompressed());
}

int insnCodeGen::generateBranch(codeGen &gen, Address from, Address to,
                                bool link) {
  long disp = (to - from);
  // If disp is within the range of 21-bits signed integer, we use jal
  if (disp >= JTYPE_IMM_MIN && disp < JTYPE_IMM_MAX) {
    return generateShortBranch(gen, from, to, link);
  }
  // Otherwise, we generate multi-sequence long branch
  return generateLongBranch(gen, from, to, link);
}

int insnCodeGen::generateBranchViaTrap(codeGen &gen, Address from, Address to) {
  assert(gen.addrSpace());
  gen.addrSpace()->trapMapping.addTrapMapping(from, to, true);
  return generateTrap(gen);
}

int insnCodeGen::generateCondBranch(codeGen &gen, int bCondOp, Register rs1,
                                    Register rs2, Address from, Address to) {
  int code_size = 0;
  RegValue disp = (to - from);
  if (disp >= BTYPE_IMM_MIN && disp < BTYPE_IMM_MAX) {
    return generateCmpBranch(gen, bCondOp, rs1, rs2, disp, gen.useCompressed());
  }
  Register scratch = Null_Register;
  instPoint *point = gen.point();
  if (point) {
    registerSpace *rs = registerSpace::actualRegSpace(point);
    gen.setRegisterSpace(rs);
    scratch = rs->getScratchRegister(gen, true);
  }

  // If no scratch register is available, use generate branch via trap
  if (scratch == Null_Register) {
    code_size +=
        generateCmpBranch(gen, bCondOp, rs1, rs2, RISCV_INSN_SIZE, false);
    code_size += generateBranchViaTrap(gen, from, to);
    return code_size;
  }
  if (disp >= UTYPE_IMM_MIN && disp < UTYPE_IMM_MAX) {
    RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
    RegValue offset = disp & UTYPE_IMM_MASK;
    if (offset & 0x800) {
      top = (top + 1) & UTYPE_IMM_MASK;
    }
    code_size +=
        generateCmpBranch(gen, bCondOp, rs1, rs2, 3 * RISCV_INSN_SIZE, false);
    code_size += generateAuipc(gen, scratch, top, false);
    code_size += generateAddi(gen, scratch, GPR_RA, offset, false);
    code_size += generateJr(gen, scratch, 0, false);
    return code_size;
  }
  // The li instruction will be expanded into 8 4-bytes instructions without
  // any optimization So we should generate
  //    bxx rs1, rs2, (1 auipc, 8 (for li), 1 for add, 1 (for jalr)) * 4 = 44
  //    auipc scratch1
  //    li scratch2, disp
  //    add scratch1, scratch2
  //    jalr x0, scratch, 0
  code_size +=
      generateCmpBranch(gen, bCondOp, rs1, rs2, 11 * RISCV_INSN_SIZE, false);

  std::vector<Register> exclude;
  exclude.push_back(scratch);
  Register scratch2 = gen.rs()->getScratchRegister(gen, true);
  if (scratch2 == Null_Register) {
    std::cerr << "Unexpected error: Not enough scratch registers to generate "
                 "long conditional branch"
              << std::endl;
    assert(0);
  }
  code_size += generateAuipc(gen, scratch, 0, false);
  code_size += generateCalcImm(gen, scratch2, disp, false, false);
  code_size += generateAdd(gen, scratch, scratch, scratch2, false);
  code_size += generateJr(gen, scratch, 0, false);
  return code_size;
}

int insnCodeGen::loadImmIntoReg(codeGen &gen, Register rd, RegValue value,
                                bool useCompressed) {
  return generateLi(gen, rd, value, true, useCompressed);
}

int insnCodeGen::modifyCall(Address target, NS_riscv64::instruction &insn,
                            codeGen &gen) {
  if (insn.isUncondBranch()) {
    return modifyJump(target, insn, gen);
  }
  return modifyJcc(target, insn, gen);
}

int insnCodeGen::modifyJump(Address target, NS_riscv64::instruction &insn,
                            codeGen &gen) {
  return generateBranch(gen, gen.currAddr(), target, insn.isCall());
}

int insnCodeGen::modifyJcc(Address target, NS_riscv64::instruction &insn,
                           codeGen &gen) {
  Register rs1 = insn.getCondBranchReg1();
  Register rs2 = insn.getCondBranchReg2();
  int bCondOp = insn.getCondBranchOp();

  return generateCondBranch(gen, bCondOp, rs1, rs2, gen.currAddr(), target);
}

// Other useful code generation methods

// Nop generation
int insnCodeGen::generateNop(codeGen &gen, bool useCompressed) {
  if (useCompressed) {
    return generateCNop(gen);
  }
  return generateMove(gen, GPR_ZERO, GPR_ZERO, false);
}

// Conditional Branch Generation

int insnCodeGen::generateCmpBranch(codeGen &gen, int bCond, Register rs1,
                                   Register rs2, RegValue imm,
                                   bool useCompressed) {
  RegValue shiftedImm = (imm >> BTYPE_IMM_SHIFT) & BTYPE_IMM_MASK;
  switch (bCond) {
  case B_COND_EQ: {
    return generateBeq(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  case B_COND_NE: {
    return generateBne(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  case B_COND_LT: {
    return generateBlt(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  case B_COND_GE: {
    return generateBge(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  case B_COND_LTU: {
    return generateBltu(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  case B_COND_GEU: {
    return generateBgeu(gen, rs1, rs2, shiftedImm, useCompressed);
  }
  default: {
    // Not gonna happen
    assert(0);
  }
  }
}

int insnCodeGen::generateMemLoad(codeGen &gen, Register rd, Register rs,
                                 RegValue offset, RegValue size,
                                 bool isUnsigned, bool useCompressed) {
  if (offset >= ITYPE_IMM_MIN && offset < ITYPE_IMM_MAX) {
    return generateLd(gen, rd, rs, offset, size, isUnsigned, useCompressed);
  }
  int code_size = 0;
  // rd <- offset
  code_size +=
      generateLoadImm(gen, rd, offset, true, true, gen.useCompressed());
  // rd <- rs + rd (rs + offset)
  code_size += generateAdd(gen, rd, rs, rd, gen.useCompressed());
  // dest <- [dest]
  code_size += generateMemLoad(gen, rd, rd, 0, size, true, gen.useCompressed());
  return code_size;
}

int insnCodeGen::generateMemStore(codeGen &gen, Register rs1, Register rs2,
                                  RegValue offset, RegValue size,
                                  bool useCompressed) {
  if (offset >= ITYPE_IMM_MIN && offset < ITYPE_IMM_MAX) {
    return generateSt(gen, rs1, rs2, offset, size, useCompressed);
  }

  int code_size = 0;
  Register scratch = gen.rs()->getScratchRegister(gen, true);
  if (scratch == Null_Register) {
    std::cerr << "Unexpected error: Not enough scratch registers to generate "
                 "memory store"
              << std::endl;
    return false;
  }
  // scratch <- offset
  code_size +=
      generateLoadImm(gen, scratch, offset, true, true, gen.useCompressed());
  // scartch <- rs1 + scratch (rs1 + offset)
  code_size += generateAdd(gen, scratch, rs1, scratch, gen.useCompressed());
  // [scratch] <- rs2
  code_size +=
      generateMemStore(gen, scratch, rs2, 0, size, gen.useCompressed());
  return code_size;
}

// Generate memory access through Load or Store
int insnCodeGen::generateLd(codeGen &gen, Register rd, Register rs,
                            RegValue offset, RegValue size, bool isUnsigned,
                            bool useCompressed) {
  assert(size == 1 || size == 2 || size == 4 || size == 8);

  if (useCompressed) {
    if (!isUnsigned && size == 4) {
      // c.lw
      if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
          offset % size == 0 && offset >= CLW_IMM_MIN && offset < CLW_IMM_MAX) {
        return generateCLw(gen, rd - GPR_X8, rs - GPR_X8,
                           (offset >> CLW_SHIFT) & CLW_MASK);
      }
      // c.lwsp
      if (rs == GPR_SP && offset % size == 0 && offset >= CLWSP_IMM_MIN &&
          offset < CLWSP_IMM_MAX) {
        return generateCLwsp(gen, rd, (offset >> CLWSP_SHIFT) & CLWSP_MASK);
      }
    }
    if (!isUnsigned && size == 8) {
      // c.ld
      if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
          offset % size == 0 && offset >= CLD_IMM_MIN && offset < CLD_IMM_MAX) {
        return generateCLd(gen, rd - GPR_X8, rs - GPR_X8,
                           (offset >> CLD_SHIFT) & CLD_MASK);
      }
      // c.ldsp
      if (rs == GPR_SP && offset % size == 0 && offset >= CLDSP_IMM_MIN &&
          offset < CLDSP_IMM_MAX) {
        return generateCLdsp(gen, rd, (offset >> CLDSP_SHIFT) & CLDSP_MASK);
      }
    }
  }

  // There is no "ldu" instruction, but treat "ldu" as ld
  // Uncomment the following line to not treat "ldu" as ld
  // assert(!(size == 8 && isUnsigned))

  RegValue funct3{};
  if (!isUnsigned) {
    switch (size) {
    case 1:
      funct3 = LBFunct3;
      break; // lb = 000
    case 2:
      funct3 = LHFunct3;
      break; // lh = 001
    case 4:
      funct3 = LWFunct3;
      break; // lw = 010
    case 8:
      funct3 = LDFunct3;
      break; // ld = 011
    default:
      break; // not gonna happen
    }
  } else {
    switch (size) {
    case 1:
      funct3 = LBUFunct3;
      break; // lbu = 100
    case 2:
      funct3 = LHUFunct3;
      break; // lhu = 101
    case 4:
      funct3 = LWUFunct3;
      break; // lwu = 110
    case 8:
      funct3 = LDFunct3;
      break; // Treat "ldu" as "ld"
    default:
      break; // not gonna happen
    }
  }
  return makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, funct3, LOADOp);
}

int insnCodeGen::generateSt(codeGen &gen, Register rs1, Register rs2,
                            RegValue offset, RegValue size,
                            bool useCompressed) {
  assert(size == 1 || size == 2 || size == 4 || size == 8);

  if (useCompressed) {
    if (size == 4) {
      // c.sw
      if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
          offset % size == 0 && offset >= CSW_IMM_MIN && offset < CSW_IMM_MAX) {
        return generateCSw(gen, rs1 - GPR_X8, rs2 - GPR_X8,
                           (offset >> CSW_SHIFT) & CSW_MASK);
      }
      // c.swsp
      if (rs1 == GPR_SP && offset % size == 0 && offset >= CSWSP_IMM_MIN &&
          offset < CSWSP_IMM_MAX) {
        return generateCSwsp(gen, rs2, (offset >> CSWSP_SHIFT) & CSWSP_MASK);
      }
    }
    if (size == 8) {
      // c.sd
      if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
          offset % size == 0 && offset >= CSD_IMM_MIN && offset < CSD_IMM_MAX) {
        return generateCSd(gen, rs1 - GPR_X8, rs2 - GPR_X8,
                           (offset >> CSD_SHIFT) & CSD_MASK);
      }
      // c.sdsp
      if (rs1 == GPR_SP && offset % size == 0 && offset >= CSDSP_IMM_MIN &&
          offset < CSDSP_IMM_MAX) {
        return generateCSdsp(gen, rs2, (offset >> CSDSP_SHIFT) & CSDSP_MASK);
      }
    }
  }

  RegValue funct3{};
  switch (size) {
  case 1:
    funct3 = SBFunct3;
    break; // sb = 000
  case 2:
    funct3 = SHFunct3;
    break; // sh = 001
  case 4:
    funct3 = SWFunct3;
    break; // sw = 010
  case 8:
    funct3 = SDFunct3;
    break; // sd = 011
  default:
    break; // not gonna happen
  }

  return makeSTypeInsn(gen, rs1, rs2, offset & STYPE_IMM_MASK, funct3, STOREOp);
}

int insnCodeGen::generateMemLoadFp(codeGen &gen, Register rd, Register rs,
                                   RegValue offset, RegValue size,
                                   bool useCompressed) {
  if (useCompressed) {
    if (size == 4) {
      // c.lw
      if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
          offset % size == 0 && offset >= CFLW_IMM_MIN &&
          offset < CFLW_IMM_MAX) {
        return generateCFlw(gen, rd - GPR_X8, rs - GPR_X8,
                            (offset >> CFLW_SHIFT) & CFLW_MASK);
      }
      // c.lwsp
      if (rs == GPR_SP && offset % size == 0 && offset >= CFLWSP_IMM_MIN &&
          offset < CFLWSP_IMM_MAX) {
        return generateCFlwsp(gen, rd, (offset >> CFLWSP_SHIFT) & CFLWSP_MASK);
      }
    }
    if (size == 8) {
      // c.ld
      if (rd >= GPR_X8 && rd < GPR_X16 && rs >= GPR_X8 && rs < GPR_X16 &&
          offset % size == 0 && offset >= CFLD_IMM_MIN &&
          offset < CFLD_IMM_MAX) {
        return generateCFld(gen, rd - GPR_X8, rs - GPR_X8,
                            (offset >> CFLD_SHIFT) & CFLD_MASK);
      }
      // c.ldsp
      if (rs == GPR_SP && offset % size == 0 && offset >= CFLDSP_IMM_MIN &&
          offset < CFLDSP_IMM_MAX) {
        return generateCFldsp(gen, rd, (offset >> CFLDSP_SHIFT) & CFLDSP_MASK);
      }
    }
  }
  return makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, FLDFunct3, FLDOp);
}

int insnCodeGen::generateMemStoreFp(codeGen &gen, Register rs1, Register rs2,
                                    RegValue offset, RegValue size,
                                    bool useCompressed) {
  if (useCompressed) {
    if (size == 4) {
      // c.sw
      if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
          offset % size == 0 && offset >= CFSW_IMM_MIN &&
          offset < CFSW_IMM_MAX) {
        return generateCFsw(gen, rs1 - GPR_X8, rs2 - GPR_X8,
                            (offset >> CFSW_SHIFT) & CFSW_MASK);
      }
      // c.swsp
      if (rs1 == GPR_SP && offset % size == 0 && offset >= CFSWSP_IMM_MIN &&
          offset < CFSWSP_IMM_MAX) {
        return generateCFswsp(gen, rs2, (offset >> CFSWSP_SHIFT) & CFSWSP_MASK);
      }
    }
    if (size == 8) {
      // c.sd
      if (rs1 >= GPR_X8 && rs1 < GPR_X16 && rs2 >= GPR_X8 && rs2 < GPR_X16 &&
          offset % size == 0 && offset >= CFSD_IMM_MIN &&
          offset < CFSD_IMM_MAX) {
        return generateCFsd(gen, rs1 - GPR_X8, rs2 - GPR_X8,
                            (offset >> CFSD_SHIFT) & CFSD_MASK);
      }
      // c.sdsp
      if (rs1 == GPR_SP && offset % size == 0 && offset >= CFSDSP_IMM_MIN &&
          offset < CFSDSP_IMM_MAX) {
        return generateCFsdsp(gen, rs2, (offset >> CFSDSP_SHIFT) & CFSDSP_MASK);
      }
    }
  }
  return makeSTypeInsn(gen, rs1, rs2, offset & STYPE_IMM_MASK, FSDFunct3,
                       FSDOp);
}

int insnCodeGen::generateLoadRelAddr(codeGen &gen, Register rd, RegValue disp,
                                     bool useCompressed) {
  RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
  RegValue offset = disp & ITYPE_IMM_MASK;
  int code_size = 0;
  if (offset & 0x800) {
    top = (top + 1) & UTYPE_IMM_MASK;
  }
  if (top >= ITYPE_IMM_MIN && top < ITYPE_IMM_MAX) {
    code_size += generateAuipc(gen, rd, top, useCompressed);
    code_size += generateAddi(gen, rd, rd, offset, useCompressed);
    return code_size;
  }
  code_size += generateAuipc(gen, rd, 0, useCompressed);
  code_size += generateAddImm(gen, rd, rd, disp, useCompressed);
  return code_size;
}

int insnCodeGen::generateMemLoadRelAddr(codeGen &gen, Register rd,
                                        RegValue disp, RegValue size,
                                        bool isUnsigned, bool useCompressed) {
  RegValue top = (disp >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
  RegValue offset = disp & ITYPE_IMM_MASK;
  int code_size = 0;
  if (offset & 0x800) {
    top = (top + 1) & UTYPE_IMM_MASK;
  }
  if (top >= ITYPE_IMM_MIN && top < ITYPE_IMM_MAX) {
    code_size += generateAuipc(gen, rd, top, useCompressed);
    code_size +=
        generateLd(gen, rd, rd, offset, size, isUnsigned, useCompressed);
    return code_size;
  }
  code_size += generateAuipc(gen, rd, 0, useCompressed);
  code_size += generateAddImm(gen, rd, rd, disp, useCompressed);
  code_size += generateLd(gen, rd, rd, 0, size, isUnsigned, useCompressed);
  return code_size;
}

int insnCodeGen::saveRegister(codeGen &gen, Register r, int sp_offset,
                              bool useCompressed) {
  return generateMemStore(gen, GPR_SP, r, sp_offset, gen.width(),
                          useCompressed);
}

int insnCodeGen::restoreRegister(codeGen &gen, Register r, int sp_offset,
                                 bool useCompressed) {
  return generateMemLoad(gen, r, GPR_SP, sp_offset, gen.width(), true,
                         useCompressed);
}

int insnCodeGen::modifyData(Address target, NS_riscv64::instruction &insn,
                            codeGen &gen) {
  assert(insn.isAuipc());

  RegValue auipcOff = insn.getAuipcOffset();
  Register rd = insn.getAuipcReg();

  // The displacement is target - currAddr
  // But we also need to minus the instruction length of auipc
  // As well as adding the offset of auipc back
  RegValue disp = (target - gen.currAddr() - RISCV_INSN_SIZE) + auipcOff;
  return generateLoadImm(gen, rd, disp, true, true, gen.useCompressed());
}

int insnCodeGen::generateAddImm(codeGen &gen, Register rd, Register rs,
                                RegValue sImm, bool useCompressed) {
  if (sImm >= ITYPE_IMM_MIN && sImm < ITYPE_IMM_MAX) {
    return generateAddi(gen, rd, rs, sImm & ITYPE_IMM_MASK, useCompressed);
  }
  int code_size = 0;
  code_size +=
      insnCodeGen::generateLoadImm(gen, rd, sImm, false, true, useCompressed);
  code_size += insnCodeGen::generateAdd(gen, rd, rs, rd, useCompressed);
  return code_size;
}

int insnCodeGen::generateLoadImm(codeGen &gen, Register rd, RegValue sImm,
                                 bool isRel, bool optimize,
                                 bool useCompressed) {
  int code_size = 0;
  if (isRel) {
    if (sImm >= UTYPE_IMM_MIN && sImm < UTYPE_IMM_MAX) {
      RegValue top = (sImm >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
      RegValue offset = sImm & ITYPE_IMM_MASK;
      if (offset & 0x800) {
        top = (top + 1) & UTYPE_IMM_MASK;
      }
      code_size += generateAuipc(gen, rd, top, useCompressed);
      code_size += generateAddi(gen, rd, rd, offset, useCompressed);
      return code_size;
    }
    Register scratch = gen.rs()->getScratchRegister(gen, true);
    if (scratch == Null_Register) {
      std::cerr << "Unexpected error: Not enough scratch registers to generate "
                   "load immediate"
                << std::endl;
      assert(0);
    }
    code_size += generateAuipc(gen, rd, 0, useCompressed);
    code_size += generateCalcImm(gen, scratch, sImm, optimize, useCompressed);
    code_size += generateAdd(gen, rd, rd, scratch, useCompressed);
    return code_size;
  }

  // c.li
  if (useCompressed && sImm >= CLI_IMM_MIN && sImm < CLI_IMM_MAX) {
    return generateCLi(gen, rd, sImm & CLI_IMM_MASK);
  }

  // addi
  if (sImm >= ITYPE_IMM_MIN && sImm < ITYPE_IMM_MAX) {
    return generateAddi(gen, rd, 0, sImm & ITYPE_IMM_MASK, useCompressed);
  }

  // If sImm is larger than 12 bits but less than 32 bits,
  // sImm must be loaded in two steps using lui and addi
  if (sImm >= INT_MIN && sImm <= INT_MAX) {
    RegValue lui_imm = (sImm >> UTYPE_IMM_SHIFT) & UTYPE_IMM_MASK;
    RegValue addi_imm = sImm & ITYPE_IMM_MASK;
    // If the most significant bit of addi_imm is 1 (addi_imm is negative),
    // we should add 1 to lui_imm
    if (addi_imm & 0x800) {
      lui_imm = (lui_imm + 1) & UTYPE_IMM_MASK;
    }
    code_size += generateLui(gen, rd, lui_imm, useCompressed);
    code_size += generateAddi(gen, rd, rd, addi_imm, useCompressed);
    return code_size;
  }
  return generateCalcImm(gen, rd, sImm, optimize, useCompressed);
}

int insnCodeGen::generateCalcImm(codeGen &gen, Register rd, RegValue sImm,
                                 bool optimize, bool useCompressed) {
  // If sImm is a 64 bit long, the sequence of instructions is more complicated
  // See the following functions for more information on how GCC generates
  // immediate integers
  // https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=gcc/config/riscv/riscv.cc;h=65e09842fde8b15b92a8399cea2493b5b239f93c;hb=HEAD#l828
  // But for the sake of simplicity, we will use a much simpler algorithm
  // Assume that we want to perform li t0, 0xdeadbeefcafebabe
  // We break the integer into the following
  //   lui  t0, 0xdeadc    / auipc t0, 0
  //   addi t0, t0, -0x110
  //   slli t0, t0, 12
  //   addi t0, t0, -0x250
  //   slli t0, t0, 12
  //   addi t0, t0, -0x186
  //   slli t0, t0, 8
  //   addi t0, t0, 0xef

  // Decompose the immediate into various chunks
  // For example, if sImm == 0xdeadbeefcafebabe:
  //  deadb     eef       caf       eba       be
  // ------- --------- --------- --------- ---------
  // lui_imm addi_imm0 addi_imm1 addi_imm2 addi_imm3

  // Upper 32 bits
  RegValue lui_imm = (sImm >> 44) & 0xfffff; // sImm[63:44]
  RegValue addi_imm0 = (sImm >> 32) & 0xfff; // sImm[43:32]
  // Lower 32 bits
  RegValue slli_imm1 = 11;
  RegValue addi_imm1 = (sImm >> 21) & 0x7ff; // sImm[31:21]
  RegValue slli_imm2 = 11;
  RegValue addi_imm2 = (sImm >> 10) & 0x7ff; // sImm[20:10]
  RegValue slli_imm3 = 10;
  RegValue addi_imm3 = sImm & 0x3ff; // sImm[9:0]

  // If the most significant bit of addi_imm is 1 (addi_imm is negative), we
  // should add 1 to lui_imm
  if (addi_imm0 & 0x800) {
    lui_imm = (lui_imm + 1) & 0xfffff;
  }
  // 12-bit sign extend
  if (lui_imm & 0x80000) {
    lui_imm |= 0xfffffffffff00000;
  }

  // Optimization: if any of the addi sImmediates are zero, we can omit them
  // We should also adjust the number of bits to shift accordingly
  if (optimize) {
    if (addi_imm2 == 0) {
      slli_imm2 += slli_imm3;
      slli_imm3 = 0;
    }
    if (addi_imm1 == 0) {
      slli_imm1 += slli_imm2;
      slli_imm2 = 0;
    }
  }

  int code_size = 0;

  // lui must be generated
  code_size += generateLui(gen, rd, lui_imm, useCompressed);

  // If any of the following sImmediates are zero, there's no point of
  // generating it
  if (addi_imm0 != 0 || !optimize) {
    code_size += generateAddi(gen, rd, rd, addi_imm0, useCompressed);
  }
  if (slli_imm1 != 0 || !optimize) {
    code_size += generateSlli(gen, rd, rd, slli_imm1, useCompressed);
  }
  if (addi_imm1 != 0 || !optimize) {
    code_size += generateAddi(gen, rd, rd, addi_imm1, useCompressed);
  }
  if (slli_imm2 != 0 || !optimize) {
    code_size += generateSlli(gen, rd, rd, slli_imm2, useCompressed);
  }
  if (addi_imm2 != 0 || !optimize) {
    code_size += generateAddi(gen, rd, rd, addi_imm2, useCompressed);
  }
  if (slli_imm3 != 0 || !optimize) {
    code_size += generateSlli(gen, rd, rd, slli_imm3, useCompressed);
  }
  if (addi_imm3 != 0 || !optimize) {
    code_size += generateAddi(gen, rd, rd, addi_imm3, useCompressed);
  }
  return code_size;
}

// RISC-V I-Type Instructions

int insnCodeGen::generateAddi(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool useCompressed) {
  // 12 bit sign extend
  int sImm = (imm & 0x800) ? (imm | 0xfffff000) : imm;

  if (useCompressed) {
    // c.nop
    if (rd == 0 && rs == 0 && sImm == 0) {
      return generateCNop(gen);
    }

    // c.li
    if (sImm >= CLI_IMM_MIN && sImm < CLI_IMM_MAX && rs == 0) {
      return generateCLi(gen, rd, imm & CLI_IMM_MASK);
    }

    // c.mv
    if (sImm == 0) {
      return generateCMv(gen, rd, rs);
    }

    // c.addi4spn
    if (rs == GPR_SP && rd >= GPR_X8 && rd < GPR_X16 && sImm % 4 == 0 &&
        sImm >= CADDI4SPN_IMM_MIN && sImm < CADDI4SPN_IMM_MAX) {
      return generateCAddi4spn(
          gen, rd, (imm >> CADDI4SPN_IMM_SHIFT) & CADDI4SPN_IMM_MASK);
    }

    // c.addi16sp
    if (rd == GPR_SP && rs == GPR_SP && sImm % 16 == 0 &&
        sImm >= CADDI16SP_IMM_MIN && sImm < CADDI16SP_IMM_MAX) {
      return generateCAddi16sp(gen, (imm >> CADDI16SP_IMM_SHIFT) &
                                        CADDI16SP_IMM_MASK);
    }

    // c.addi
    if (rd == rs && sImm >= CADDI_IMM_MIN && sImm < CADDI_IMM_MAX) {
      return generateCAddi(gen, rd, imm & CADDI_IMM_MASK);
    }
  }
  // addi
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ADDFunct3, IMMOp);
}

int insnCodeGen::generateSlli(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool useCompressed) {
  if (useCompressed) {
    // c.slli
    if (rd == rs && imm >= CSLLI_IMM_MIN && imm < CSLLI_IMM_MAX) {
      return generateCSlli(gen, rd, imm & CSLLI_IMM_MASK);
    }
  }
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SLLFunct3, IMMOp);
}

int insnCodeGen::generateSrli(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool useCompressed) {
  if (useCompressed) {
    // use c.srli
    if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 && imm >= CSRLI_IMM_MIN &&
        imm < CSRLI_IMM_MAX) {
      return generateCSrli(gen, rd - GPR_X8, imm);
    }
  }
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SRLFunct3, IMMOp);
}

int insnCodeGen::generateSrai(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool useCompressed) {
  if (useCompressed) {
    // c.srai
    if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 && imm >= CSRLI_IMM_MIN &&
        imm < CSRLI_IMM_MAX) {
      return generateCSrai(gen, rd - GPR_X8, imm);
    }
  }
  // srai is essentially srli with bit 30 set to 1
  imm |= 0x400;

  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, SRAFunct3, IMMOp);
}
int insnCodeGen::generateAndi(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool useCompressed) {
  if (useCompressed) {
    // c.andi
    if (rd == rs && rd >= GPR_X8 && rd < GPR_X16 && imm >= CANDI_IMM_MIN &&
        imm < CANDI_IMM_MAX) {
      return generateCAndi(gen, rd - GPR_X8, imm);
    }
  }
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ANDFunct3, IMMOp);
}

int insnCodeGen::generateOri(codeGen &gen, Register rd, Register rs,
                             RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, ORFunct3, IMMOp);
}

int insnCodeGen::generateXori(codeGen &gen, Register rd, Register rs,
                              RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeITypeInsn(gen, rd, rs, imm & ITYPE_IMM_MASK, XORFunct3, IMMOp);
}

int insnCodeGen::generateLui(codeGen &gen, Register rd, RegValue offset,
                             bool useCompressed) {
  RegValue imm = offset >> 12;
  if (useCompressed) {
    // c.lui
    if ((imm >= CLUI_IMM_MIN1 && imm < CLUI_IMM_MAX1) ||
        (imm >= CLUI_IMM_MIN2 && imm < CLUI_IMM_MAX2)) {
      return generateCLui(gen, rd, imm);
    }
  }
  return makeUTypeInsn(gen, rd, imm & UTYPE_IMM_MASK, LUIOp);
}

int insnCodeGen::generateAuipc(codeGen &gen, Register rd, RegValue offset,
                               bool /*useCompressed*/) {
  // No available RVC optimization
  return makeUTypeInsn(gen, rd, offset & UTYPE_IMM_MASK, AUIPCOp);
}

int insnCodeGen::generateAdd(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool useCompressed) {
  if (useCompressed) {
    // c.add
    if (rs1 == rs2) {
      generateCAdd(gen, rd, rs1);
      return true;
    }
  }
  return makeRTypeInsn(gen, rd, rs1, rs2, ADDFunct7, ADDFunct3, REGOp);
}

int insnCodeGen::generateSub(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, SUBFunct7, SUBFunct3, REGOp);
}

int insnCodeGen::generateSll(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, SLLFunct7, SLLFunct3, REGOp);
}

int insnCodeGen::generateSrl(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, SRLFunct7, SRLFunct3, REGOp);
}

int insnCodeGen::generateSra(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, SRAFunct7, SRAFunct3, REGOp);
}

int insnCodeGen::generateAnd(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool useCompressed) {
  if (useCompressed) {
    // c.and
    if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
      return generateCAnd(gen, rd, rs1 - GPR_X8);
    }
  }
  return makeRTypeInsn(gen, rd, rs1, rs2, ANDFunct7, ANDFunct3, REGOp);
}

int insnCodeGen::generateOr(codeGen &gen, Register rd, Register rs1,
                            Register rs2, bool useCompressed) {
  if (useCompressed) {
    // c.or
    if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
      return generateCOr(gen, rd - GPR_X8, rs1);
    }
  }
  return makeRTypeInsn(gen, rd, rs1, rs2, ORFunct7, ORFunct3, REGOp);
}
int insnCodeGen::generateXor(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool useCompressed) {
  if (useCompressed) {
    // c.xor
    if (rs1 == rs2 && rs1 >= GPR_X8 && rs1 < GPR_X16) {
      return generateCXor(gen, rd - GPR_X8, rs1);
    }
  }
  return makeRTypeInsn(gen, rd, rs1, rs2, XORFunct7, XORFunct3, REGOp);
}

int insnCodeGen::generateMul(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, MULFunct7, MULFunct3, REGOp);
}

int insnCodeGen::generateDiv(codeGen &gen, Register rd, Register rs1,
                             Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, DIVFunct7, DIVFunct3, REGOp);
}

int insnCodeGen::generateDivu(codeGen &gen, Register rd, Register rs1,
                              Register rs2, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeRTypeInsn(gen, rd, rs1, rs2, DIVUFunct7, DIVUFunct3, REGOp);
}

int insnCodeGen::generateLi(codeGen &gen, Register rd, RegValue imm,
                            bool optimize, bool useCompressed) {
  return generateLoadImm(gen, rd, imm, false, optimize, useCompressed);
}

int insnCodeGen::generateMove(codeGen &gen, Register rd, Register rs,
                              bool useCompressed) {
  if (useCompressed) {
    return generateCMv(gen, rd, rs);
  }
  return generateAdd(gen, rd, rs, 0, useCompressed);
}

int insnCodeGen::generateBeq(codeGen &gen, Register rs1, Register rs2,
                             RegValue imm, bool useCompressed) {
  // If both rs1 and rs2 are x0, it is equivalent to unconditional branch
  if (rs1 == GPR_ZERO && rs2 == GPR_ZERO) {
    return generateBranch(gen, imm, false);
  }
  if (useCompressed) {
    if ((rs1 == GPR_ZERO && rs2 >= GPR_X8 && rs2 < GPR_X16) &&
        imm >= CBEQZ_IMM_MIN && imm < CBEQZ_IMM_MAX) {
      int shiftedImm = (imm >> CBEQZ_IMM_SHIFT) & CBEQZ_IMM_MASK;
      return generateCBeqz(gen, rs1 - GPR_X8, shiftedImm);
    }
    if ((rs2 == GPR_ZERO && rs1 >= GPR_X8 && rs1 < GPR_X16) &&
        imm >= CBEQZ_IMM_MIN && imm < CBEQZ_IMM_MAX) {
      int shiftedImm = (imm >> CBEQZ_IMM_SHIFT) & CBEQZ_IMM_MASK;
      return generateCBeqz(gen, rs2 - GPR_X8, shiftedImm);
    }
  }
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BEQFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateBne(codeGen &gen, Register rs1, Register rs2,
                             RegValue imm, bool useCompressed) {
  // If both rs1 and rs2 are x0, it is equivalent to unconditional branch
  if (rs1 == GPR_ZERO && rs2 == GPR_ZERO) {
    return generateJal(gen, GPR_ZERO, imm, gen.useCompressed());
  }
  if (useCompressed) {
    if ((rs1 == GPR_ZERO && rs2 >= GPR_X8 && rs2 < GPR_X16) &&
        imm >= CBNEZ_IMM_MIN && imm < CBNEZ_IMM_MAX) {
      int shiftedImm = (imm >> CBNEZ_IMM_SHIFT) & CBNEZ_IMM_MASK;
      return generateCBnez(gen, rs1 - GPR_X8, shiftedImm);
    }
    if ((rs2 == GPR_ZERO && rs1 >= GPR_X8 && rs1 < GPR_X16) &&
        imm >= CBNEZ_IMM_MIN && imm < CBNEZ_IMM_MAX) {
      int shiftedImm = (imm >> CBNEZ_IMM_SHIFT) & CBNEZ_IMM_MASK;
      return generateCBnez(gen, rs2 - GPR_X8, shiftedImm);
    }
  }
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BNEFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateBlt(codeGen &gen, Register rs1, Register rs2,
                             RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BLTFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateBge(codeGen &gen, Register rs1, Register rs2,
                             RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BGEFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateBltu(codeGen &gen, Register rs1, Register rs2,
                              RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BLTUFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateBgeu(codeGen &gen, Register rs1, Register rs2,
                              RegValue imm, bool /*useCompressed*/) {
  // No available RVC optimization
  return makeBTypeInsn(gen, rs1, rs2, imm & BTYPE_IMM_MASK, BGEUFunct3,
                       BRANCHOp);
}

int insnCodeGen::generateJ(codeGen &gen, RegValue offset, bool useCompressed) {
  if (useCompressed) {
    if (offset >= -CJ_IMM_MIN && offset < CJ_IMM_MAX) {
      // use c.j
      return generateCJ(gen, (offset >> CJ_IMM_SHIFT) & CJ_IMM_MASK);
    }
  }
  return generateJal(gen, GPR_ZERO, offset, false);
}

int insnCodeGen::generateJal(codeGen &gen, Register rd, RegValue offset,
                             bool /*useCompressed*/) {
  // No optimization for RV64 (c.jal is RV32)
  RegValue shiftedImm = (offset >> JTYPE_IMM_SHIFT) & JTYPE_IMM_MASK;
  return makeJTypeInsn(gen, rd, shiftedImm, JALOp);
}

int insnCodeGen::generateJr(codeGen &gen, Register rs, RegValue offset,
                            bool useCompressed) {
  if (useCompressed && offset == 0) {
    // c.j
    return generateCJr(gen, rs);
  }
  return generateJalr(gen, GPR_ZERO, rs, offset, false);
}

int insnCodeGen::generateJalr(codeGen &gen, Register rd, Register rs,
                              RegValue offset, bool useCompressed) {
  if (useCompressed && offset == 0 && rd == GPR_RA) {
    // c.jalr
    return generateCJalr(gen, rs);
  }

  // JALR is an I-Type instruction
  return makeITypeInsn(gen, rd, rs, offset & ITYPE_IMM_MASK, JALRFunct3,
                       JALROp);
}

int insnCodeGen::generateCAdd(codeGen &gen, Register rd, Register rs) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x4); // func3 = 100
  INSN_SET(insn, 12, 12, 0x1); // imm[5] != 0
  INSN_SET(insn, 7, 11, rd);   // rsi/rd != 0
  INSN_SET(insn, 2, 6, rs);    // imm[4:0] != 0
  INSN_SET(insn, 0, 1, 0x1);   // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCAddi(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x0);            // func3 = 000
  INSN_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5] != 0
  INSN_SET(insn, 7, 11, rd);              // rsi/rd != 0
  INSN_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0] != 0
  INSN_SET(insn, 0, 1, 0x1);              // opcode = 01
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCAddi4spn(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x0);              // func3 = 000
  INSN_SET(insn, 11, 12, (imm >> 2) & 0x3); // imm[3:2]
  INSN_SET(insn, 7, 10, (imm >> 4) & 0xf);  // imm[7:4]
  INSN_SET(insn, 6, 6, (imm >> 0) & 0x1);   // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 1) & 0x1);   // imm[1]
  INSN_SET(insn, 2, 4, rd);                 // rd
  INSN_SET(insn, 0, 1, 0x0);                // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCAddi16sp(codeGen &gen, RegValue imm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x3);              // func3 = 011
  INSN_SET(insn, 12, 12, (imm >> 5) & 0x1); // imm[5]
  INSN_SET(insn, 7, 11, 0x2);               // 00010
  INSN_SET(insn, 6, 6, (imm >> 0) & 0x1);   // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 2) & 0x1);   // imm[2]
  INSN_SET(insn, 3, 4, (imm >> 3) & 0x3);   // imm[4:3]
  INSN_SET(insn, 2, 2, (imm >> 1) & 0x1);   // imm[1]
  INSN_SET(insn, 0, 1, 0x1);                // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCAnd(codeGen &gen, Register rd, Register rs) {
  instruction insn;

  INSN_SET(insn, 10, 15, 0x23); // 100011
  INSN_SET(insn, 7, 9, rd);     // rd
  INSN_SET(insn, 5, 6, 0x3);    // 11
  INSN_SET(insn, 2, 4, rs);     // rs
  INSN_SET(insn, 0, 1, 0x1);    // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCAndi(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4);            // func3 = 100
  INSN_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
  INSN_SET(insn, 10, 11, 0x10);           // 10
  INSN_SET(insn, 7, 9, rd);               // rd
  INSN_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
  INSN_SET(insn, 0, 1, 0x1);              // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLw(codeGen &gen, Register rd, Register rs1,
                             RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x2);              // func3 = 010
  INSN_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
  INSN_SET(insn, 7, 9, rs1);                // rs1
  INSN_SET(insn, 6, 6, imm & 0x1);          // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
  INSN_SET(insn, 2, 4, rd);                 // rd
  INSN_SET(insn, 0, 1, 0x0);                // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLwsp(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x2);              // func3 = 010
  INSN_SET(insn, 12, 12, (imm >> 3) & 0x1); // imm[3]
  INSN_SET(insn, 7, 11, rd);                // rd
  INSN_SET(insn, 4, 6, imm & 0x7);          // imm[2:0]
  INSN_SET(insn, 2, 3, (imm >> 4) & 0x3);   // imm[5:4]
  INSN_SET(insn, 0, 1, 0x2);                // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLd(codeGen &gen, Register rd, Register rs1,
                             RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x3);            // func3 = 011
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, rs1);              // rs1
  INSN_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
  INSN_SET(insn, 2, 4, rd);               // rd
  INSN_SET(insn, 0, 1, 0x0);              // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLdsp(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x3);              // func3 = 011
  INSN_SET(insn, 12, 12, (imm >> 2) & 0x1); // imm[2]
  INSN_SET(insn, 7, 11, rd);                // rd
  INSN_SET(insn, 5, 6, imm & 0x3);          // imm[1:0]
  INSN_SET(insn, 2, 4, (imm >> 3) & 0x7);   // imm[5:3]
  INSN_SET(insn, 0, 1, 0x2);                // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFlw(codeGen &gen, Register rd, Register rs1,
                              RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x3);              // func3 = 011
  INSN_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
  INSN_SET(insn, 7, 9, rs1);                // rs1
  INSN_SET(insn, 6, 6, imm & 0x1);          // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
  INSN_SET(insn, 2, 4, rd);                 // rd
  INSN_SET(insn, 0, 1, 0x0);                // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFlwsp(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x3);              // func3 = 011
  INSN_SET(insn, 12, 12, (imm >> 3) & 0x1); // imm[3]
  INSN_SET(insn, 7, 11, rd);                // rd
  INSN_SET(insn, 4, 6, imm & 0x7);          // imm[2:0]
  INSN_SET(insn, 2, 3, (imm >> 4) & 0x3);   // imm[5:4]
  INSN_SET(insn, 0, 1, 0x2);                // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFld(codeGen &gen, Register rd, Register rs1,
                              RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x1);            // func3 = 001
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, rs1);              // rs1
  INSN_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
  INSN_SET(insn, 2, 4, rd);               // rd
  INSN_SET(insn, 0, 1, 0x0);              // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFldsp(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x1);              // func3 = 001
  INSN_SET(insn, 12, 12, (imm >> 2) & 0x1); // imm[2]
  INSN_SET(insn, 7, 11, rd);                // rd
  INSN_SET(insn, 5, 6, imm & 0x3);          // imm[1:0]
  INSN_SET(insn, 2, 4, (imm >> 3) & 0x7);   // imm[5:3]
  INSN_SET(insn, 0, 1, 0x2);                // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLi(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x2);            // func3 = 010
  INSN_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
  INSN_SET(insn, 7, 11, rd);              // rd
  INSN_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
  INSN_SET(insn, 0, 1, 0x1);              // opcode = 01
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCLui(codeGen &gen, Register rd, RegValue imm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x3);            // func3 = 011
  INSN_SET(insn, 12, 12, (imm >> 5) & 1); // imm[5]
  INSN_SET(insn, 7, 11, rd);              // rd
  INSN_SET(insn, 2, 6, imm & 0x1f);       // imm[4:0]
  INSN_SET(insn, 0, 1, 0x1);              // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSw(codeGen &gen, Register rd, Register rs1,
                             RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x6);              // func3 = 110
  INSN_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
  INSN_SET(insn, 7, 9, rs1);                // rs1
  INSN_SET(insn, 6, 6, imm & 0x1);          // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
  INSN_SET(insn, 2, 4, rd);                 // rd
  INSN_SET(insn, 0, 1, 0x0);                // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSwsp(codeGen &gen, Register rs2, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x6);            // func3 = 110
  INSN_SET(insn, 9, 12, imm & 0xf);       // imm[3:0]
  INSN_SET(insn, 7, 8, (imm >> 4) & 0x3); // imm[5:4]
  INSN_SET(insn, 2, 6, rs2);              // rs2
  INSN_SET(insn, 0, 1, 0x2);              // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSd(codeGen &gen, Register rd, Register rs1,
                             RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x7);            // func3 = 111
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, rs1);              // rs1
  INSN_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
  INSN_SET(insn, 2, 4, rd);               // rd
  INSN_SET(insn, 0, 1, 0x0);              // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSdsp(codeGen &gen, Register rs2, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x7);            // func3 = 111
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, (imm >> 3) & 0x7); // imm[5:3]
  INSN_SET(insn, 2, 6, rs2);              // rs2
  INSN_SET(insn, 0, 1, 0x2);              // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFsw(codeGen &gen, Register rd, Register rs1,
                              RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x7);              // func3 = 111
  INSN_SET(insn, 10, 12, (imm >> 1) & 0x7); // imm[3:1]
  INSN_SET(insn, 7, 9, rs1);                // rs1
  INSN_SET(insn, 6, 6, imm & 0x1);          // imm[0]
  INSN_SET(insn, 5, 5, (imm >> 4) & 0x1);   // imm[4]
  INSN_SET(insn, 2, 4, rd);                 // rd
  INSN_SET(insn, 0, 1, 0x0);                // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFswsp(codeGen &gen, Register rs2, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x7);            // func3 = 111
  INSN_SET(insn, 9, 12, imm & 0xf);       // imm[3:0]
  INSN_SET(insn, 7, 8, (imm >> 4) & 0x3); // imm[5:4]
  INSN_SET(insn, 2, 6, rs2);              // rs2
  INSN_SET(insn, 0, 1, 0x2);              // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFsd(codeGen &gen, Register rd, Register rs1,
                              RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x5);            // func3 = 101
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, rs1);              // rs1
  INSN_SET(insn, 5, 6, (imm >> 3) & 0x3); // imm[4:3]
  INSN_SET(insn, 2, 4, rd);               // rd
  INSN_SET(insn, 0, 1, 0x0);              // opcode = 00
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCFsdsp(codeGen &gen, Register rs2, RegValue imm) {
  instruction insn;
  INSN_SET(insn, 13, 15, 0x5);            // func3 = 101
  INSN_SET(insn, 10, 12, imm & 0x7);      // imm[2:0]
  INSN_SET(insn, 7, 9, (imm >> 3) & 0x7); // imm[5:3]
  INSN_SET(insn, 2, 6, rs2);              // rs2
  INSN_SET(insn, 0, 1, 0x2);              // opcode = 10
  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSlli(codeGen &gen, Register rd, RegValue uimm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x2);             // func3 = 010
  INSN_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
  INSN_SET(insn, 7, 11, rd);               // rd
  INSN_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
  INSN_SET(insn, 0, 1, 0x2);               // opcode = 10

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSrli(codeGen &gen, Register rd, RegValue uimm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4);             // func3 = 100
  INSN_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
  INSN_SET(insn, 10, 11, 0x0);             // 00
  INSN_SET(insn, 7, 9, rd);                // rd
  INSN_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
  INSN_SET(insn, 0, 1, 0x1);               // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCSrai(codeGen &gen, Register rd, RegValue uimm) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4);             // func3 = 100
  INSN_SET(insn, 12, 12, (uimm >> 5) & 1); // uimm[5]
  INSN_SET(insn, 10, 11, 0x1);             // 01
  INSN_SET(insn, 7, 9, rd);                // rd
  INSN_SET(insn, 2, 6, uimm & 0x1f);       // uimm[4:0]
  INSN_SET(insn, 0, 1, 0x1);               // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCMv(codeGen &gen, Register rd, Register rs) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4); // func3 = 100
  INSN_SET(insn, 12, 12, 0x0); // 0
  INSN_SET(insn, 7, 11, rd);   // rd
  INSN_SET(insn, 2, 6, rs);    // rs
  INSN_SET(insn, 0, 1, 0x2);   // opcode = 10

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCOr(codeGen &gen, Register rd, Register rs) {
  instruction insn;

  INSN_SET(insn, 10, 15, 0x23); // 100011
  INSN_SET(insn, 7, 9, rd);     // rd
  INSN_SET(insn, 5, 6, 0x2);    // 10
  INSN_SET(insn, 2, 4, rs);     // rs
  INSN_SET(insn, 0, 1, 0x1);    // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCJ(codeGen &gen, RegValue offset) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x5);                  // func3 = 101
  INSN_SET(insn, 12, 12, (offset >> 10) & 0x1); // imm[10]
  INSN_SET(insn, 11, 11, (offset >> 3) & 0x1);  // imm[3]
  INSN_SET(insn, 9, 10, (offset >> 7) & 0x3);   // imm[8:7]
  INSN_SET(insn, 8, 8, (offset >> 9) & 0x1);    // imm[9]
  INSN_SET(insn, 7, 7, (offset >> 5) & 0x1);    // imm[5]
  INSN_SET(insn, 6, 6, (offset >> 6) & 0x1);    // imm[6]
  INSN_SET(insn, 3, 5, offset & 0x7);           // imm[2:0]
  INSN_SET(insn, 2, 2, (offset >> 4) & 0x1);    // imm[4]
  INSN_SET(insn, 0, 1, 0x1);                    // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCJal(codeGen &gen, RegValue offset) {
  instruction insn;

  INSN_SET(insn, 12, 12, (offset >> 10) & 0x1); // imm[10]
  INSN_SET(insn, 11, 11, (offset >> 3) & 0x1);  // imm[3]
  INSN_SET(insn, 9, 10, (offset >> 7) & 0x3);   // imm[8:7]
  INSN_SET(insn, 8, 8, (offset >> 9) & 0x1);    // imm[9]
  INSN_SET(insn, 7, 7, (offset >> 5) & 0x1);    // imm[5]
  INSN_SET(insn, 6, 6, (offset >> 6) & 0x1);    // imm[6]
  INSN_SET(insn, 3, 5, offset & 0x7);           // imm[2:0]
  INSN_SET(insn, 2, 2, (offset >> 4) & 0x1);    // imm[4]
  INSN_SET(insn, 0, 1, 0x1);                    // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCJr(codeGen &gen, Register rs) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4); // func3 = 100
  INSN_SET(insn, 12, 12, 0x0); // 0
  INSN_SET(insn, 7, 11, rs);   // rs
  INSN_SET(insn, 0, 6, 0x2);   // 0000010

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCJalr(codeGen &gen, Register rs) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x4); // func3 = 100
  INSN_SET(insn, 12, 12, 0x1); // 1
  INSN_SET(insn, 7, 11, rs);   // rs
  INSN_SET(insn, 0, 6, 0x2);   // 0000010

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCBeqz(codeGen &gen, Register rs, RegValue offset) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x6);                 // func3 = 110
  INSN_SET(insn, 12, 12, (offset >> 7) & 0x1); // offset[7]
  INSN_SET(insn, 10, 11, (offset >> 2) & 0x3); // offset[3:2]
  INSN_SET(insn, 7, 9, rs);                    // rs
  INSN_SET(insn, 5, 6, (offset >> 5) & 0x3);   // offset[6:5]
  INSN_SET(insn, 3, 4, offset & 0x3);          // offset[1:0]
  INSN_SET(insn, 2, 2, (offset >> 4) & 0x1);   // offset[4]
  INSN_SET(insn, 0, 1, 0x1);                   // 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCBnez(codeGen &gen, Register rs, RegValue offset) {
  instruction insn;

  INSN_SET(insn, 13, 15, 0x7);                 // func3 = 111
  INSN_SET(insn, 12, 12, (offset >> 7) & 0x1); // offset[7]
  INSN_SET(insn, 10, 11, (offset >> 2) & 0x3); // offset[3:2]
  INSN_SET(insn, 7, 9, rs);                    // rs
  INSN_SET(insn, 5, 6, (offset >> 5) & 0x3);   // offset[6:5]
  INSN_SET(insn, 3, 4, offset & 0x3);          // offset[1:0]
  INSN_SET(insn, 2, 2, (offset >> 4) & 0x1);   // offset[4]
  INSN_SET(insn, 0, 1, 0x1);                   // 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCNop(codeGen &gen) {
  instruction insn;

  INSN_SET(insn, 0, 15, C_NOP_INSN_ENC);

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}

int insnCodeGen::generateCXor(codeGen &gen, Register rd, Register rs) {
  instruction insn;

  INSN_SET(insn, 10, 15, 0x23); // 100011
  INSN_SET(insn, 7, 9, rd);     // rd
  INSN_SET(insn, 5, 6, 0x1);    // 01
  INSN_SET(insn, 2, 4, rs);     // rs
  INSN_SET(insn, 0, 1, 0x1);    // opcode = 01

  insnCodeGen::generate(gen, insn);

  return RISCVC_INSN_SIZE;
}
