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

/*
 * emit-aarch64.C - ARMv8 code generators (emitters)
 */

/*
#include <assert.h>
#include <stdio.h>
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include "dyninstAPI/src/dynProcess.h"

#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/image.h"
// get_index...
#include "dyninstAPI/src/dynThread.h"
#include "ABI.h"
#include "liveness.h"
#include "RegisterConversion.h"
*/

#include "dyninstAPI/src/emit-riscv64.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst-riscv64.h"
#include "dyninstAPI/src/registerSpace.h"

class image_variable;

constexpr int isUnsigned = true;
constexpr int isRel = true;
constexpr int optimize = true;
constexpr int useCompressed = true;

codeBufIndex_t EmitterRISCV64::emitIf(Register expr_reg, Register target,
                                      RegControl /*rc*/, codeGen &gen) {
  // beq expr_reg, zero, 8
  // jalr zero, target, 0

  // Here, we don't use compressed instructions because we need to fix the
  // offset to 8

  insnCodeGen::generateBne(gen, expr_reg, GPR_ZERO, (2 * RISCV_INSN_SIZE) >> 1,
                           !useCompressed);
  insnCodeGen::generateJ(gen, target + 4, !useCompressed);

  // Retval: where the jump is in this sequence
  return gen.getIndex();
}

void EmitterRISCV64::emitOp(unsigned opcode, Register dest, Register src1,
                            Register src2, codeGen &gen) {
  switch (opcode) {

  case plusOp: {
    insnCodeGen::generateAdd(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  case minusOp: {
    insnCodeGen::generateSub(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  case timesOp: {
    insnCodeGen::generateMul(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  case andOp: {
    insnCodeGen::generateAnd(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  case orOp: {
    insnCodeGen::generateOr(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  case xorOp: {
    insnCodeGen::generateXor(gen, src1, src2, dest, gen.useCompressed());

    break;
  }
  default: {
    std::cerr << "Invalid op " << opcode << " in emitOp" << std::endl;
    assert(0);
  }
  }
}

void EmitterRISCV64::emitOpImm(unsigned /*opcode*/, unsigned /*opcode2*/,
                               Register /*dest*/, Register /*src1*/,
                               RegValue /*src2imm*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.

  // PLEASE USE emitImm IN inst-riscv64.C INSTEAD
  assert(0);
}

void EmitterRISCV64::emitRelOp(unsigned opcode, Register dest, Register src1,
                               Register src2, codeGen &gen, bool is_signed) {
  // make dest = 1, meaning true
  insnCodeGen::loadImmIntoReg(gen, dest, 0x1, gen.useCompressed());

  // Insert conditional jump to skip dest = 0 in case the comparison resulted
  // true Therefore keeping dest = 1
  switch (opcode) {
  case lessOp: {
    if (is_signed) {
      insnCodeGen::generateBlt(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                               false);
    } else {
      insnCodeGen::generateBltu(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                                false);
    }
    break;
  }
  case leOp: {
    if (is_signed) {
      insnCodeGen::generateBge(gen, src2, src1, (RISCV_INSN_SIZE * 2) >> 1,
                               false);
    } else {
      insnCodeGen::generateBgeu(gen, src2, src1, (RISCV_INSN_SIZE * 2) >> 1,
                                false);
    }
    break;
  }
  case greaterOp: {
    if (is_signed) {
      insnCodeGen::generateBlt(gen, src2, src1, (RISCV_INSN_SIZE * 2) >> 1,
                               false);
    } else {
      insnCodeGen::generateBltu(gen, src2, src1, (RISCV_INSN_SIZE * 2) >> 1,
                                false);
    }
    break;
  }
  case geOp: {
    if (is_signed) {
      insnCodeGen::generateBge(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                               false);
    } else {
      insnCodeGen::generateBgeu(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                                false);
    }
    break;
  }
  case eqOp: {
    insnCodeGen::generateBeq(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                             false);
    break;
  }
  case neOp: {
    insnCodeGen::generateBne(gen, src1, src2, (RISCV_INSN_SIZE * 2) >> 1,
                             false);
    break;
  }
  default: {
    assert(0); // wrong condition passed
    break;
  }
  }

  // Make dest = 0, in case it fails the branch
  insnCodeGen::loadImmIntoReg(gen, dest, 0x0, false);
}

void EmitterRISCV64::emitRelOpImm(unsigned opcode, Register dest, Register src1,
                                  RegValue src2imm, codeGen &gen,
                                  bool is_signed) {
  insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());

  // Insert conditional jump to skip dest = 0 in case the comparison resulted
  // true, therefore keeping dest = 1
  switch (opcode) {
  case lessOp: {
    if (is_signed) {
      insnCodeGen::generateBlt(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                               !useCompressed);
    } else {
      insnCodeGen::generateBltu(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                                false);
    }
    break;
  }
  case leOp: {
    if (is_signed) {
      insnCodeGen::generateBge(gen, dest, src1, (RISCV_INSN_SIZE * 3) >> 1,
                               !useCompressed);
    } else {
      insnCodeGen::generateBgeu(gen, dest, src1, (RISCV_INSN_SIZE * 3) >> 1,
                                !useCompressed);
    }
    break;
  }
  case greaterOp: {
    if (is_signed) {
      insnCodeGen::generateBlt(gen, dest, src1, (RISCV_INSN_SIZE * 3) >> 1,
                               !useCompressed);
    } else {
      insnCodeGen::generateBltu(gen, dest, src1, (RISCV_INSN_SIZE * 3) >> 1,
                                !useCompressed);
    }
    break;
  }
  case geOp: {
    if (is_signed) {
      insnCodeGen::generateBge(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                               !useCompressed);
    } else {
      insnCodeGen::generateBgeu(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                                !useCompressed);
    }
    break;
  }
  case eqOp: {
    insnCodeGen::generateBeq(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                             !useCompressed);
    break;
  }
  case neOp: {
    insnCodeGen::generateBne(gen, src1, dest, (RISCV_INSN_SIZE * 3) >> 1,
                             !useCompressed);
    break;
  }
  default: {
    std::cerr << "Invalid op " << opcode << " in emitRelOpImm" << std::endl;
    assert(0);
    break;
  }
  }

  // Make dest = 0, in case it fails the branch
  insnCodeGen::generateLoadImm(gen, dest, 0x1, !isRel, !optimize,
                               !useCompressed);
  insnCodeGen::generateJ(gen, 2 * RISCV_INSN_SIZE, !useCompressed);
  insnCodeGen::generateLoadImm(gen, dest, 0x0, !isRel, !optimize,
                               !useCompressed);
}

void EmitterRISCV64::emitDiv(Register dest, Register src1, Register src2,
                             codeGen &gen, bool is_signed) {
  if (is_signed) {
    insnCodeGen::generateDiv(gen, dest, src1, src2, gen.useCompressed());
  } else {
    insnCodeGen::generateDivu(gen, dest, src1, src2, gen.useCompressed());
  }
}

void EmitterRISCV64::emitTimesImm(Register dest, Register src1,
                                  RegValue src2imm, codeGen &gen) {
  insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
  insnCodeGen::generateMul(gen, dest, src1, dest, gen.useCompressed());
}

void EmitterRISCV64::emitDivImm(Register dest, Register src1, RegValue src2imm,
                                codeGen &gen, bool is_signed) {
  insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
  if (is_signed) {
    insnCodeGen::generateDiv(gen, dest, src1, dest, gen.useCompressed());
  } else {
    insnCodeGen::generateDivu(gen, dest, src1, dest, gen.useCompressed());
  }
}

void EmitterRISCV64::emitLoad(Register dest, Address addr, int size,
                              codeGen &gen) {
  insnCodeGen::loadImmIntoReg(gen, dest, addr, gen.useCompressed());
  // Zero extend the dest register, so set isUnsigned to true
  insnCodeGen::generateMemLoad(gen, dest, dest, 0, size, isUnsigned,
                               gen.useCompressed());
}

void EmitterRISCV64::emitLoadConst(Register dest, Address imm, codeGen &gen) {
  insnCodeGen::loadImmIntoReg(gen, dest, imm, gen.useCompressed());
}

void EmitterRISCV64::emitLoadIndir(Register dest, Register addr_src, int size,
                                   codeGen &gen) {
  insnCodeGen::generateMemLoad(gen, dest, addr_src, 0, size, true,
                               gen.useCompressed());
  gen.markRegDefined(dest);
}

bool EmitterRISCV64::emitCallRelative(Register dest, Address offset,
                                      Register base, codeGen &gen) {
  insnCodeGen::generateJalr(gen, dest, base, offset, gen.useCompressed());
  return true;
}

bool EmitterRISCV64::emitLoadRelative(Register dest, Address offset,
                                      Register baseReg, int size,
                                      codeGen &gen) {
  insnCodeGen::generateMemLoad(gen, dest, baseReg, offset, size, isUnsigned,
                               gen.useCompressed());
  return true;
}

void EmitterRISCV64::emitLoadShared(opCode op, Register dest,
                                    const image_variable *var, bool is_local,
                                    int size, codeGen &gen, Address offset) {
  // create or retrieve jump slot
  Address addr;

  if (var == NULL) {
    addr = offset;
  } else if (!is_local) {
    addr = getInterModuleVarAddr(var, gen);
  } else {
    addr = (Address)var->getOffset();
  }

  Address varOffset = addr - gen.currAddr();

  if (op == loadOp) {
    if (!is_local && (var != NULL)) {
      insnCodeGen::generateMemLoadRelAddr(gen, dest, varOffset, gen.width(),
                                          isUnsigned, gen.useCompressed());
      // Deference the pointer to get the variable
      insnCodeGen::generateMemLoad(gen, dest, dest, 0, size, true,
                                   gen.useCompressed());
    } else {
      insnCodeGen::generateMemLoadRelAddr(gen, dest, varOffset, size,
                                          isUnsigned, gen.useCompressed());
    }

  } else if (op == loadConstOp) {
    if (!is_local && (var != NULL)) {
      insnCodeGen::generateMemLoadRelAddr(gen, dest, varOffset, gen.width(),
                                          isUnsigned, gen.useCompressed());
    } else {
      // Load effective address
      insnCodeGen::generateLoadRelAddr(gen, dest, varOffset,
                                       gen.useCompressed());
    }
  } else {
    std::cerr << "Invalid op " << op << " in emitVstore" << std::endl;
    assert(0);
  }

  return;
}

void EmitterRISCV64::emitLoadFrameAddr(Register /*dest*/, Address /*offset*/,
                                       codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitLoadOrigFrameRelative(Register /*dest*/,
                                               Address /*offset*/,
                                               codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitLoadOrigRegRelative(Register dest, Address offset,
                                             Register base, codeGen &gen,
                                             bool deref) {
  gen.markRegDefined(dest);
  Register scratch = gen.rs()->getScratchRegister(gen);
  assert(scratch);
  gen.markRegDefined(scratch);

  // Either load the address or the contents at that address
  if (deref) {
    // Load the stored register 'base' into scratch
    emitLoadOrigRegister(base, scratch, gen);
    // move offset(%scratch), %dest
    insnCodeGen::generateMemLoad(gen, dest, scratch, offset, 8, true,
                                 gen.useCompressed());
  } else {
    // load the stored register 'base' into dest
    emitLoadOrigRegister(base, scratch, gen);
    insnCodeGen::loadImmIntoReg(gen, dest, offset, gen.useCompressed());
    insnCodeGen::generateAdd(gen, dest, scratch, dest, gen.useCompressed());
  }
}

void EmitterRISCV64::emitLoadOrigRegister(Address register_num,
                                          Register destination, codeGen &gen) {
  registerSlot *src = (*gen.rs())[register_num];
  assert(src);
  registerSlot *dest = (*gen.rs())[destination];
  assert(dest);

  if (src->number == GPR_SP) {
    insnCodeGen::generateAddi(gen, TRAMP_FRAME_SIZE_64, GPR_SP, destination,
                              gen.useCompressed());
    return;
  }

  if (src->spilledState == registerSlot::unspilled) {
    // Not on the stack. Directly move the value
    insnCodeGen::generateMove(gen, destination, (Register)register_num,
                              gen.useCompressed());
    return;
  }

  EmitterRISCV64SaveRestoreRegs saveRestoreRegs;
  // Its on the stack so load it.
  insnCodeGen::restoreRegister(gen, destination, src->saveOffset,
                               gen.useCompressed());
}

void EmitterRISCV64::emitStore(Address addr, Register src, int size,
                               codeGen &gen) {
  Register scratch = gen.rs()->getScratchRegister(gen);

  if (scratch == Null_Register) {
    std::cerr << "Unexpected error: Not enough scratch registers to generate "
                 "emitStore"
              << std::endl;
    assert(0);
  }
  insnCodeGen::loadImmIntoReg(gen, scratch, addr, gen.useCompressed());
  insnCodeGen::generateMemStore(gen, scratch, src, 0, size,
                                gen.useCompressed());

  gen.rs()->freeRegister(scratch);
  gen.markRegDefined(scratch);
}

void EmitterRISCV64::emitStoreIndir(Register addr_reg, Register src, int size,
                                    codeGen &gen) {
  insnCodeGen::generateMemStore(gen, addr_reg, src, 0, size,
                                gen.useCompressed());
}

void EmitterRISCV64::emitStoreFrameRelative(Address /*dest*/, Register /*src1*/,
                                            Register /*src2*/, int /*size*/,
                                            codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitStoreRelative(Register source, Address offset,
                                       Register baseReg, int size,
                                       codeGen &gen) {
  insnCodeGen::generateMemStore(gen, source, baseReg, offset, size,
                                gen.useCompressed());
}

bool EmitterRISCV64::emitMoveRegToReg(registerSlot *dest, registerSlot *src,
                                      codeGen &gen) {
  insnCodeGen::generateMove(gen, dest->number, src->number,
                            gen.useCompressed());
  return true;
}

bool EmitterRISCV64::emitMoveRegToReg(Register dest, Register src,
                                      codeGen &gen) {
  insnCodeGen::generateMove(gen, dest, src, gen.useCompressed());
  return true;
}

void EmitterRISCV64::emitStoreOrigRegister(Address register_num,
                                           Register destination, codeGen &gen) {
  registerSlot *src = (*gen.rs())[register_num];
  assert(src);
  registerSlot *dest = (*gen.rs())[destination];
  assert(dest);

  if (src->number == GPR_SP) {
    insnCodeGen::generateAddi(gen, -TRAMP_FRAME_SIZE_64, GPR_SP, destination,
                              gen.useCompressed());
    return;
  }

  EmitterRISCV64SaveRestoreRegs saveRestoreRegs;
  insnCodeGen::saveRegister(gen, destination, src->saveOffset,
                            gen.useCompressed());
}

Address EmitterRISCV64::emitMovePCToReg(Register dest, codeGen &gen) {
  // auipc rd, 0
  insnCodeGen::generateAuipc(gen, dest, 0, gen.useCompressed());
  Address ret = gen.currAddr();
  return ret;
}

Register EmitterRISCV64::emitCall(opCode op, codeGen &gen,
                                  const std::vector<AstNodePtr> &operands, bool,
                                  func_instance *callee) {
  assert(op == callOp);
  assert(callee);

  Register scratch = gen.rs()->getScratchRegister(gen);
  if (scratch == Null_Register) {
    std::cerr
        << "Unexpected error: Not enough scratch registers to generate emitCall"
        << std::endl;
    assert(0);
  }

  // We first save all necessary registers
  std::vector<Register> savedRegs;
  for (int i = 0; i < gen.rs()->numGPRs(); i++) {
    registerSlot *reg = gen.rs()->GPRs()[i];
    // Ignore zero, sp, gp, tp
    if (reg->number == GPR_ZERO || reg->number == GPR_SP ||
        reg->number == GPR_GP || reg->number == GPR_TP) {
      continue;
    }
    // Save caller saved registers if either
    // * link register
    // * refCount > 0
    // * keptValue == true (keep over the call)
    // * liveState == live (except for callee-saved registers)
    if (reg->number == GPR_RA || (reg->refCount > 0) || reg->keptValue ||
        (reg->liveState == registerSlot::live)) {
      savedRegs.push_back(reg->number);
    }
  }
  insnCodeGen::generateAddImm(
      gen, GPR_SP, GPR_SP, -savedRegs.size() * GPRSIZE_64, gen.useCompressed());
  for (unsigned i = 0; i < savedRegs.size(); i++) {
    insnCodeGen::saveRegister(gen, savedRegs[i], i * GPRSIZE_64,
                              gen.useCompressed());
  }

  // Generate code that handles operand registers

  // The first 8 arguments are put inside registers a0 ~ a7
  int op_size = operands.size();
  for (int id = 0; id < std::min(op_size, RISCV_MAX_ARG_REGS); id++) {
    Register reg = GPR_A0 + id;
    gen.markRegDefined(reg);

    Address unnecessary = ADDR_NULL;
    operands[id]->generateCode_phase2(gen, false, unnecessary, reg);
    assert(reg != Null_Register);
  }
  // The rest of them are put on the stack
  if (operands.size() > 8) {
    unsigned remain_op_size = operands.size() - 8;
    insnCodeGen::generateAddImm(
        gen, GPR_SP, GPR_SP, -remain_op_size * GPRSIZE_64, gen.useCompressed());
    for (unsigned id = 0; id < remain_op_size; id++) {
      Address unnecessary = ADDR_NULL;
      operands[id]->generateCode_phase2(gen, false, unnecessary, scratch);
      int offset_from_sp = (remain_op_size - id - 1) * GPRSIZE_64;
      insnCodeGen::saveRegister(gen, scratch, offset_from_sp,
                                gen.useCompressed());
    }
  }

  if (gen.addrSpace()->edit() != NULL &&
      (gen.func()->obj() != callee->obj() || gen.addrSpace()->needsPIC())) {
    RegValue disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true,
                                 gen.useCompressed());
    insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true,
                                 gen.useCompressed());
  } else {
    insnCodeGen::loadImmIntoReg(gen, GPR_RA, callee->addr(),
                                gen.useCompressed());
  }

  // Generate jalr
  insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0, gen.useCompressed());

  // Move the return value to the scratch register
  insnCodeGen::generateMove(gen, scratch, GPR_A0, gen.useCompressed());

  // Finally, we restore all necessary registers
  if (operands.size() > RISCV_MAX_ARG_REGS) {
    unsigned remain_op_size = operands.size() - RISCV_MAX_ARG_REGS;
    insnCodeGen::generateAddImm(
        gen, GPR_SP, GPR_SP, remain_op_size * GPRSIZE_64, gen.useCompressed());
  }
  for (size_t id = 0; id < savedRegs.size(); id++) {
    // No need to restore the return register!
    if (savedRegs[id] == scratch)
      continue;
    insnCodeGen::restoreRegister(gen, savedRegs[id], id * GPRSIZE_64,
                                 gen.useCompressed());
  }
  insnCodeGen::generateAddImm(
      gen, GPR_SP, GPR_SP, savedRegs.size() * GPRSIZE_64, gen.useCompressed());
  return scratch;
}

void EmitterRISCV64::emitGetRetVal(Register /*dest*/, bool /*addr_of*/,
                                   codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitGetRetAddr(Register /*dest*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitGetParam(Register /*dest*/, Register /*param_num*/,
                                  instPoint::Type /*pt_type*/, opCode /*op*/,
                                  bool /*addr_of*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitASload(int /*ra*/, int /*rb*/, int /*sc*/,
                                long /*imm*/, Register /*dest*/,
                                int /*stackShift*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.

  // Please use the overloaded version of emitASload in inst-riscv64.C instead
  assert(0);
}

void EmitterRISCV64::emitCSload(int /*ra*/, int /*rb*/, int /*sc*/,
                                long /*imm*/, Register /*dest*/,
                                codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.

  // Please use the overloaded version of emitCSload in inst-riscv64.C instead
  assert(0);
}

void EmitterRISCV64::emitPushFlags(codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitRestoreFlags(codeGen & /*gen*/, unsigned /*offset*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitRestoreFlagsFromStackSlot(codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

bool EmitterRISCV64::emitBTSaves(baseTramp * /*bt*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

bool EmitterRISCV64::emitBTRestores(baseTramp * /*bt*/, codeGen & /*gen*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

void EmitterRISCV64::emitStoreImm(Address addr, int imm, codeGen &gen,
                                  bool /*noCost*/) {
  Register scratch = gen.rs()->getScratchRegister(gen);

  if (scratch == Null_Register) {
    std::cerr << "Unexpected error: Not enough scratch registers to generate "
                 "emitStore"
              << std::endl;
    assert(0);
  }
  insnCodeGen::loadImmIntoReg(gen, scratch, addr, gen.useCompressed());
  Register scratch2 = gen.rs()->getScratchRegister(gen);
  if (scratch2 == Null_Register) {
    std::cerr << "Unexpected error: Not enough scratch registers to generate "
                 "emitStore"
              << std::endl;
    assert(0);
  }
  insnCodeGen::loadImmIntoReg(gen, scratch2, imm, gen.useCompressed());
  insnCodeGen::generateMemStore(gen, scratch, scratch2, 0, GPRSIZE_64,
                                gen.useCompressed());

  gen.rs()->freeRegister(scratch);
  gen.rs()->freeRegister(scratch2);
  gen.markRegDefined(scratch);
  gen.markRegDefined(scratch2);
}

void EmitterRISCV64::emitAddSignedImm(Address /*addr*/, int /*imm*/,
                                      codeGen & /*gen*/, bool /*noCost*/) {
  // Not used by the RISC-V code generator.
  // Provided as a dummy implementation to satisfy pure virtual function.
  assert(0);
}

bool EmitterRISCV64::emitPush(codeGen &gen, Register pushee) {
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, -GPRSIZE_64,
                              gen.useCompressed());
  insnCodeGen::generateMemLoad(gen, GPR_SP, pushee, 0, GPRSIZE_64, isUnsigned,
                               gen.useCompressed());
  return true;
}

bool EmitterRISCV64::emitPop(codeGen &gen, Register popee) {
  insnCodeGen::generateMemStore(gen, popee, GPR_SP, 0, GPRSIZE_64,
                                gen.useCompressed());
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, GPRSIZE_64,
                              gen.useCompressed());
  return true;
}

bool EmitterRISCV64::emitAdjustStackPointer(int index, codeGen &gen) {
  int popVal = index * gen.addrSpace()->getAddressWidth();
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, popVal, gen.useCompressed());
  return true;
}

bool EmitterRISCV64::clobberAllFuncCall(registerSpace *rs,
                                        func_instance *callee) {
  if (!callee) {
    return true;
  }

  if (callee->ifunc()->isLeafFunc()) {
    std::set<Register> *gpRegs = callee->ifunc()->usedGPRs();
    for (std::set<Register>::iterator itr = gpRegs->begin();
         itr != gpRegs->end(); itr++) {
      rs->GPRs()[*itr]->beenUsed = true;
    }

    std::set<Register> *fpRegs = callee->ifunc()->usedFPRs();
    for (std::set<Register>::iterator itr = fpRegs->begin();
         itr != fpRegs->end(); itr++) {
      if (*itr <= rs->FPRs().size()) {
        rs->FPRs()[*itr]->beenUsed = true;
      } else {
        rs->FPRs()[*itr & 0xff]->beenUsed = true;
      }
    }
  } else {
    for (int idx = 0; idx < rs->numGPRs(); idx++) {
      rs->GPRs()[idx]->beenUsed = true;
    }
    for (int idx = 0; idx < rs->numFPRs(); idx++) {
      rs->FPRs()[idx]->beenUsed = true;
    }
  }

  return false;
}
