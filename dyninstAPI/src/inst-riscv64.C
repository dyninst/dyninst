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

#include "RegisterConversion.h"
#include "common/src/arch-riscv64.h"
#include "common/src/headers.h"
#include "common/src/stats.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst-riscv64.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/registerSpace.h"
#include "dyninstAPI/src/util.h"
#include "parseAPI/h/CFG.h"

#include "emit-riscv64.h"
#include "emitter.h"

#include <boost/assign/list_of.hpp>
using namespace boost::assign;
#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x, y) ((x < y) ? (y - x) : (x - y))

Address getMaxBranch() { return JTYPE_IMM_MAX; }

std::unordered_map<std::string, unsigned> funcFrequencyTable;

/************************************* Register Space
 * **************************************/

void registerSpace::initialize32() {
  assert(!"No 32-bit implementation for the RISCV architecture!");
}

void registerSpace::initialize64() {
  static bool done = false;
  if (done)
    return;

  constexpr bool offLimit = true;
  std::vector<registerSlot *> registers;

  // RISC-V GPRs
  
  // The initialization order matters
  // When getScratchRegister is called, we want temporary registers to be retrieved first
  for (unsigned idx = GPR_T0; idx <= GPR_T2; idx++) {
    char name[32];
    sprintf(name, "r%u", idx);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::deadABI, registerSlot::GPR));
  }
  for (unsigned idx = GPR_T3; idx <= GPR_T6; idx++) {
    char name[32];
    sprintf(name, "r%u", idx);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::deadABI, registerSlot::GPR));
  }
  for (unsigned idx = GPR_A0; idx <= GPR_A7; idx++) {
    char name[32];
    sprintf(name, "r%u", idx);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::liveAlways, registerSlot::GPR));
  }
  for (unsigned idx = GPR_S0; idx <= GPR_S1; idx--) {
    char name[32];
    sprintf(name, "r%u", idx);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::liveAlways, registerSlot::GPR));
  }
  for (unsigned idx = GPR_S2; idx < GPR_S11; idx--) {
    char name[32];
    sprintf(name, "r%u", idx);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::liveAlways, registerSlot::GPR));
  }

  // RISC-V does not provide dedicated scratch registers. Compared to other
  // architectures, it often requires scratch registers because of its fundamental design.
  // This includes limited immediate sizes for jump, small immediates for memory
  // load/store offset, multi-instruction sequences, and so on
  // As a result, unfortunately, we often ran out of scratch registers to use

  // The solution to this issue is rather simple: we dedicate a fixed scratch register to store temporary values
  // This way, we don't need to worry about scratch registers scarcity as we are using the same temporary register all the time
  // Here, we choose S11 for scratch registers, and we mark it off limit
  registers.push_back(new registerSlot(
      GPR_S11, "r31", offLimit, registerSlot::liveAlways, registerSlot::GPR));

  // We don't want to mess with the following registers
  // So we set the offLimit to true
  registers.push_back(new registerSlot(
      GPR_ZERO, "r0", offLimit, registerSlot::liveAlways, registerSlot::GPR));
  registers.push_back(new registerSlot(
      GPR_RA, "r1", offLimit, registerSlot::liveAlways, registerSlot::GPR));
  registers.push_back(new registerSlot(
      GPR_SP, "r2", offLimit, registerSlot::liveAlways, registerSlot::GPR));
  registers.push_back(new registerSlot(
      GPR_GP, "r3", offLimit, registerSlot::liveAlways, registerSlot::GPR));
  registers.push_back(new registerSlot(
      GPR_TP, "r4", offLimit, registerSlot::liveAlways, registerSlot::GPR));

  // RISC-V FPRs
  for (unsigned idx = fpr0; idx <= fpr31; idx++) {
    char name[32];
    sprintf(name, "fpr%u", idx - fpr0);
    registers.push_back(new registerSlot(
        idx, name, !offLimit, registerSlot::liveAlways, registerSlot::FPR));
  }

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize() { initialize64(); }

/************************************************************************************************/
/************************************************************************************************/

/********************************* EmitterRISCV64SaveRegs
 * ***************************************/

unsigned
EmitterRISCV64SaveRestoreRegs::getStackHeight(codeGen & /*gen*/,
                                              registerSpace *theRegSpace) {
  unsigned height = 0;
  for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
    registerSlot *reg = theRegSpace->GPRs()[idx];
    // zero, sp, gp, and tp are not stored
    if (reg->number == GPR_ZERO || reg->number == GPR_RA || reg->number == GPR_GP || reg->number == GPR_TP) {
      continue;
    }
    if (reg->liveState == registerSlot::spilled) {
      height++;
    }
  }
  return height;
}

int EmitterRISCV64SaveRestoreRegs::getStackSpillIndex(codeGen & /*gen*/,
                                               registerSpace *theRegSpace,
                                               Register regNum) {
  if (theRegSpace->GPRs()[regNum]->liveState != registerSlot::spilled ||
      (regNum >= GPR_X0 && regNum <= GPR_X4)) {
    return -1;
  }
  return stack_spill_index.at(regNum);
}

unsigned EmitterRISCV64SaveRestoreRegs::saveGPRegisters(
    codeGen &gen, registerSpace *theRegSpace, baseTramp *bt, int offset) {
  std::vector<registerSlot *> regs;
  for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
    registerSlot *reg = theRegSpace->GPRs()[idx];
    // Prevent storing: zero, sp, gp, and tp
    if (reg->number == GPR_ZERO || reg->number == GPR_RA || reg->number == GPR_GP || reg->number == GPR_TP) {
      continue;
    }
    if (bt->definedRegs.size() == 0 || (reg->liveState == registerSlot::live &&
                                        bt->definedRegs[reg->encoding()])) {
      regs.push_back(reg);
    }
  }

  pushStack(gen, regs.size() * GPRSIZE_64);

  for (unsigned idx = 0; idx < regs.size(); idx++) {
    registerSlot *reg = regs[idx];
    int offset_from_sp = offset + idx * GPRSIZE_64;
    stack_spill_index[reg->number] = offset_from_sp;
    insnCodeGen::saveRegister(gen, reg->number, offset_from_sp,
                              gen.useCompressed());
    theRegSpace->markSavedRegister(reg->number, offset_from_sp);
  }

  return regs.size();
}

unsigned EmitterRISCV64SaveRestoreRegs::saveFPRegisters(
    codeGen &gen, registerSpace *theRegSpace, int offset) {
  unsigned ret = 0;

  for (int idx = 0; idx < theRegSpace->numFPRs(); idx++) {
    registerSlot *reg = theRegSpace->FPRs()[idx];

    // no liveness for floating points currently, so save all
    int offset_from_sp =
        offset + (reg->encoding() * (idx < 32 ? FPRSIZE_32 : FPRSIZE_64));
    insnCodeGen::generateMemStoreFp(gen, GPR_SP, reg->number, offset_from_sp, 8,
                                    gen.useCompressed());
    theRegSpace->markSavedRegister(reg->number, offset_from_sp);
    ret++;
  }

  return ret;
}

unsigned EmitterRISCV64SaveRestoreRegs::saveSPRegisters(
    codeGen & /*gen*/, registerSpace * /*theRegSpace*/, int /*offset*/,
    bool /*force_save*/) {
  // Currently no SPRs are saved on the stack
  return 0;
}

unsigned EmitterRISCV64SaveRestoreRegs::restoreGPRegisters(
    codeGen &gen, registerSpace *theRegSpace, int offset) {
  std::vector<registerSlot *> regs;

  for (int idx = 0; idx < theRegSpace->numGPRs(); idx++) {
    // Do not restore zero (x0), sp (x2)
    registerSlot *reg = theRegSpace->GPRs()[idx];
    if (reg->number == GPR_ZERO || reg->number == GPR_SP ||
        reg->number == GPR_GP || reg->number == GPR_TP) {
      continue;
    }
    if (reg->liveState == registerSlot::spilled) {
      regs.push_back(reg);
    }
  }
  for (int idx = regs.size() - 1; idx >= 0; idx--) {
    registerSlot *reg = regs[idx];
    int offset_from_sp = stack_spill_index.at(reg->number);
    insnCodeGen::restoreRegister(gen, reg->number, offset_from_sp,
                                 gen.useCompressed());
    stack_spill_index.erase(reg->number);
  }
  // Tear down the stack frame.
  popStack(gen, regs.size() * GPRSIZE_64);

  return regs.size();
}

unsigned EmitterRISCV64SaveRestoreRegs::restoreFPRegisters(
    codeGen &gen, registerSpace *theRegSpace, int offset) {

  for (int idx = theRegSpace->numFPRs() - 1; idx >= 0; idx--) {
    registerSlot *reg = theRegSpace->FPRs()[idx];

    // No liveness for floating points currently, so save all
    int offset_from_sp =
        offset + (reg->encoding() * (idx < 32 ? FPRSIZE_32 : FPRSIZE_64));
    insnCodeGen::generateMemLoadFp(gen, reg->number, GPR_SP, offset_from_sp, 8,
                                   gen.useCompressed());
  }

  return theRegSpace->numFPRs();
}

unsigned EmitterRISCV64SaveRestoreRegs::restoreSPRegisters(
    codeGen & /*gen*/, registerSpace * /*theRegSpace*/, int /*offset*/,
    int /*force_save*/) {
  int ret = 0;
  // TODO RISC-V speical purpose register currently not supported
  return ret;
}

void pushStack(codeGen &gen, int size) {
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, -size,
                              gen.useCompressed());
}

void popStack(codeGen &gen, int size) {
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, size,
                              gen.useCompressed());
}

/*********************************** Base Tramp
 * ***********************************************/
bool baseTramp::generateSaves(codeGen &gen, registerSpace *) {

  EmitterRISCV64SaveRestoreRegs saveRestoreRegs;

  saveRestoreRegs.saveGPRegisters(gen, gen.rs(), this, 0);

  // Dyninst instrumentation frame has a different structure
  // compared to stack frame created by the compiler.
  // Dyninst instrumentation frame makes sure that FP and SP are the same.
  // During stack walk, the FP retrived from the previous frame is
  // the SP of the current instrumentation frame.
  //
  // If the implementation of the instrumentation frame layout
  // needs to be changed, DyninstDynamicStepperImpl::getCallerFrameArch
  // in stackwalk/src/riscv64-swk.C also likely needs to be changed accordingly

  // insnCodeGen::generateMove(gen, REG_FP, GPR_SP, gen.useCompressed());
  // gen.markRegDefined(REG_FP);

  bool saveFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                  (BPatch::bpatch->isSaveFPROn() &&
                   gen.rs()->anyLiveFPRsAtEntry() && this->saveFPRs());

  if (saveFPRs)
    saveRestoreRegs.saveFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(FPRSIZE_64));
  this->savedFPRs = saveFPRs;

  return true;
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace *) {
  EmitterRISCV64SaveRestoreRegs restoreRegs;

  if (this->savedFPRs) {
    restoreRegs.restoreFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(FPRSIZE_64));
  }
  restoreRegs.restoreGPRegisters(gen, gen.rs(), 0);

  return true;
}

/***********************************************************************************************/
/***********************************************************************************************/

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest,
             codeGen &gen, bool /*noCost*/, registerSpace * /* rs */, bool s) {
  switch (op) {
  case plusOp: {
    if (src2imm >= ITYPE_IMM_MIN && src2imm < ITYPE_IMM_MAX) {
      insnCodeGen::generateAddImm(gen, dest, src1, src2imm & ITYPE_IMM_MASK,
                                  gen.useCompressed());
    } else {
      insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
      insnCodeGen::generateAdd(gen, dest, src1, dest,
                               gen.useCompressed());
    }
    break;
  }
  case minusOp: {
    if (-src2imm >= ITYPE_IMM_MIN && -src2imm < ITYPE_IMM_MAX) {
      insnCodeGen::generateAddImm(gen, dest, src1, (-src2imm) & ITYPE_IMM_MASK,
                                  gen.useCompressed());
    } else {
      insnCodeGen::loadImmIntoReg(gen, dest, -src2imm, gen.useCompressed());
      insnCodeGen::generateAdd(gen, dest, src1, dest,
                               gen.useCompressed());
    }
    break;
  }
  case timesOp: {
    insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
    insnCodeGen::generateMul(gen, dest, src1, dest,
                             gen.useCompressed());
    break;
  }
  case divOp: {
    insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
    insnCodeGen::generateDiv(gen, dest, src1, dest,
                             gen.useCompressed());
    break;
  }
  case xorOp: {
    if (src2imm >= ITYPE_IMM_MIN && src2imm < ITYPE_IMM_MAX) {
      insnCodeGen::generateXori(gen, dest, src1, src2imm & ITYPE_IMM_MASK,
                                gen.useCompressed());
    } else {
      insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
      insnCodeGen::generateXor(gen, dest, src1, dest,
                               gen.useCompressed());
    }
    break;
  }
  case orOp: {
    if (src2imm >= ITYPE_IMM_MIN && src2imm < ITYPE_IMM_MAX) {
      insnCodeGen::generateOri(gen, dest, src1, src2imm & ITYPE_IMM_MASK,
                               gen.useCompressed());
    } else {
      insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
      insnCodeGen::generateOr(gen, dest, src1, dest, gen.useCompressed());
    }
    break;
  }
  case andOp: {
    if (src2imm >= ITYPE_IMM_MIN && src2imm < ITYPE_IMM_MAX) {
      insnCodeGen::generateAndi(gen, dest, src1, src2imm & ITYPE_IMM_MASK,
                                gen.useCompressed());
    } else {
      insnCodeGen::loadImmIntoReg(gen, dest, src2imm, gen.useCompressed());
      insnCodeGen::generateAnd(gen, dest, src1, dest, gen.useCompressed());
    }
    break;
  }
  case eqOp:
  case neOp:
  case lessOp:
  case leOp:
  case greaterOp:
  case geOp:
    gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen, s);
    return;
  default:
    cerr << "Invalid op " << op << " in emitImm" << endl;
    assert(0);
    break;
  }
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

Register emitFuncCall(opCode, codeGen &, std::vector<AstNodePtr> &, bool,
                      Address) {
  // This function is not used anywhere
  // But need to exist because it is virtual
  return 0;
}

Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
  return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}

Register EmitterRISCV64::emitCallReplacement(opCode, codeGen &, bool,
                                             func_instance *) {
  // This function is not used anywhere
  // But need to exist because it is virtual
  return 0;
}

Register EmitterRISCV64::emitCall(opCode op, codeGen &gen,
                                  const std::vector<AstNodePtr> &operands, bool,
                                  func_instance *callee) {
  assert(op == callOp);
  assert(callee);

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
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP,
                              -savedRegs.size() * GPRSIZE_64,
                              gen.useCompressed());
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
    insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, -remain_op_size * GPRSIZE_64, gen.useCompressed());
    for (unsigned id = 0; id < remain_op_size; id++) {
        Address unnecessary = ADDR_NULL;
        operands[id]->generateCode_phase2(gen, false, unnecessary, scratch);
        int offset_from_sp = (remain_op_size - id - 1) * GPRSIZE_64;
        insnCodeGen::saveRegister(gen, scratch, offset_from_sp, gen.useCompressed());
    }
  }

  if (gen.addrSpace()->edit() != NULL &&
      (gen.func()->obj() != callee->obj() || gen.addrSpace()->needsPIC())) {
    long disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true,
                                 gen.useCompressed());
    insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true,
                                 gen.useCompressed());
  } else {
    insnCodeGen::loadImmIntoReg(gen, GPR_RA, callee->addr(),
                                gen.useCompressed());
  }

  // Generate jalr
  insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0,
                            gen.useCompressed());

  // Move the return value to the scratch register
  insnCodeGen::generateMove(gen, scratch, GPR_A0, gen.useCompressed());


  // Finally, we restore all necessary registers
  if (operands.size() > RISCV_MAX_ARG_REGS) {
    unsigned remain_op_size = operands.size() - RISCV_MAX_ARG_REGS;
    insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, remain_op_size * GPRSIZE_64, gen.useCompressed());
  }
  for (size_t id = 0; id < savedRegs.size(); id++) {
    // No need to restore the return register!
    if (savedRegs[id] == scratch) continue;
    insnCodeGen::restoreRegister(gen, savedRegs[id], id * GPRSIZE_64,
                                 gen.useCompressed());
  }
  insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP,
                              savedRegs.size() * GPRSIZE_64,
                              gen.useCompressed());
  return scratch;
}

codeBufIndex_t emitA(opCode op, Register src1, Register, long dest,
                     codeGen &gen, RegControl rc, bool) {
  codeBufIndex_t retval = 0;

  switch (op) {
  case ifOp: {
    retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
    break;
  }
  case branchOp: {
    insnCodeGen::generateBranch(gen, dest,
                                gen.useCompressed());
    retval = gen.getIndex();
    break;
  }
  default:
    cerr << "Invalid op " << op << " in emitA" << endl;
    assert(0);
  }

  return retval;
}

Register emitR(opCode op, Register src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/, const instPoint *,
               bool /*for_MT*/) {
  registerSlot *regSlot = NULL;

  switch (op) {
  case getRetValOp:
    regSlot = gen.rs()->GPRs()[GPR_A0];
    break;
  case getParamOp:
    if (src1 < RISCV_MAX_ARG_REGS) {
      regSlot = gen.rs()->GPRs()[GPR_A0 + src1];
      break;

    } else {
      int stkOffset = TRAMP_FRAME_SIZE_64 + (src1 - RISCV_MAX_ARG_REGS) * GPRSIZE_64;
      if (src2 != Null_Register)
        insnCodeGen::saveRegister(gen, src2, stkOffset,
                                  gen.useCompressed());
      insnCodeGen::restoreRegister(gen, dest, stkOffset,
                                   gen.useCompressed());

      return dest;
    }
    break;
  default:
    cerr << "Invalid op " << op << " in emitR" << endl;
    assert(0);
  }

  assert(regSlot);

  Register reg = regSlot->number;

  switch (regSlot->liveState) {
  case registerSlot::spilled: {
    EmitterRISCV64SaveRestoreRegs saveRestoreRegs;
    int idx = saveRestoreRegs.getStackSpillIndex(gen, gen.rs(), reg);
    assert(idx != -1);
    insnCodeGen::restoreRegister(gen, dest, idx * GPRSIZE_64,
                                 gen.useCompressed());
    return dest;
  }
  case registerSlot::live: {
    // Something is wrong
    // A live register does not need to be restored
    cerr << "emitR state:" << reg << " live" << endl;
    assert(0);
  }
  case registerSlot::dead: {
    // Something is wrong
    // Dead register cannot be restored from stack
    cerr << "emitR state" << reg << ": dead" << endl;
    assert(0);
  }
  }
  return reg;
}

void emitJmpMC(int condition, int offset, codeGen &gen) {
  
}

void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
                codeGen &gen, bool noCost) {
  assert(stackShift == 0);
  Address addr = as->getImm();
  int ra = as->getReg(0);
  int rb = as->getReg(1);
  gen.codeEmitter()->emitLoadConst(dest, addr, gen);
  if(ra > -1) {
      gen.codeEmitter()->emitOp(plusOp, dest, ra, dest, gen);
  }
  if(rb > -1) {
      gen.codeEmitter()->emitOp(plusOp, dest, rb, dest, gen);
  }
  return;
}

void emitCSload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen, bool noCost) {
  emitASload(as, dest, 0, gen, noCost);
}

void emitVload(opCode op, Address src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/, registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace *) {
  switch (op) {
  case loadConstOp:
    gen.codeEmitter()->emitLoadConst(dest, src1, gen);
    break;
  case loadOp:
    gen.codeEmitter()->emitLoad(dest, src1, size, gen);
    break;
  case loadRegRelativeAddr:
    gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
    break;
  case loadRegRelativeOp:
    gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
    break;
  default:
    cerr << "Invalid op " << op << " in emitVload" << endl;
    assert(0);
    break;
  }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
                codeGen &gen, bool, registerSpace * /* rs */, int size,
                const instPoint * /* location */, AddressSpace *) {
  if (op == storeOp) {
    // [dest] = src1
    // dest has the address where src1 is to be stored
    // src1 is a temporary
    // src2 is a "scratch" register, we don't need it in this architecture
    gen.codeEmitter()->emitStore(dest, src1, size, gen);
  } else {
    cerr << "Invalid op " << op << " in emitVstore" << endl;
    assert(0);
  }
  return;
}

void emitV(opCode op, Register src1, Register src2, Register dest, codeGen &gen,
           bool /*noCost*/, registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc, bool s) {
  switch (op) {
  case plusOp:
  case minusOp:
  case timesOp:
  case orOp:
  case andOp:
  case xorOp:
    gen.codeEmitter()->emitOp(op, dest, src1, src2, gen);
    break;
  case divOp:
    insnCodeGen::generateDiv(gen, src2, src1, dest,
                             gen.useCompressed());
    break;
  case lessOp:
  case leOp:
  case greaterOp:
  case geOp:
  case eqOp:
  case neOp:
    gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen, s);
    break;
  case loadIndirOp:
    size = !size ? proc->getAddressWidth() : size;
    // same as loadOp, but the value to load is already in a register
    gen.codeEmitter()->emitLoadIndir(dest, src1, size, gen);
    break;
  case storeIndirOp:
    size = !size ? proc->getAddressWidth() : size;
    gen.codeEmitter()->emitStoreIndir(dest, src1, size, gen);
    break;
  default:
    // std::cout << "operation not implemented= " << op << endl;
    assert(0); // Not implemented
    break;
  }
  return;
}

int getInsnCost(opCode) {
  assert(0); // Not implemented
  return 0;
}

// This is used for checking wether immediate value should be encoded
// into a instruction.
// In RISC-V the immediates are already handled in codegen-riscv64.C
// So the immediates can be as big as possible
bool doNotOverflow(int64_t value) {
  return true;
}

void emitLoadPreviousStackFrameRegister(Address register_num, Register dest,
                                        codeGen &gen, int /*size*/, bool) {
  gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction insn,
                                          Address addr,
                                          std::vector<AstNodePtr> &args) {

  auto target_reg = boost::dynamic_pointer_cast<Dyninst::InstructionAPI::RegisterAST>(insn.getControlFlowTarget());
  assert(target_reg);
  auto branch_target = convertRegID(target_reg);

  if (branch_target == registerSpace::ignored) return false;

  args.push_back(AstNode::operandNode(AstNode::operandType::origRegister,(void *)(long)branch_target));
  args.push_back(AstNode::operandNode(AstNode::operandType::Constant, (void *) addr));

  return true;
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
  Address val_to_write = f->addr();
  return p->writeDataSpace((void *)addr, sizeof(Address), &val_to_write);
}

Emitter *AddressSpace::getEmitter() {
    static EmitterRISCV64Stat emitter64Stat;
    static EmitterRISCV64Dyn emitter64Dyn;

    if (proc()) return &emitter64Dyn;

    return &emitter64Stat;
}

bool EmitterRISCV64::emitCallRelative(Register dest, Address offset, Register base, codeGen &gen) {
  insnCodeGen::generateJalr(gen, dest, base, offset, gen.useCompressed());
  return true;
}

bool EmitterRISCV64::emitLoadRelative(Register dest, Address offset,
                                      Register baseReg, int size,
                                      codeGen &gen) {
    insnCodeGen::generateMemLoad(gen, dest, baseReg, offset, size, true, gen.useCompressed());
    return true;
}

void EmitterRISCV64::emitStoreRelative(Register source, Address offset,
                                       Register baseReg, int size,
                                       codeGen &gen) {
  insnCodeGen::generateMemStore(gen, source, baseReg, offset, size, gen.useCompressed());
  return;
}

bool EmitterRISCV64::emitMoveRegToReg(registerSlot *reg1, registerSlot *reg2,
                                      codeGen &gen) {
  insnCodeGen::generateMove(gen, reg1->number, reg2->number, gen.useCompressed());
  return true;
}

bool EmitterRISCV64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    Address disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true, gen.useCompressed());
    insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true, gen.useCompressed());
    insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0, gen.useCompressed());
    return true;
}

bool EmitterRISCV64Stat::emitPLTJump(func_instance *callee,
                                     codeGen &gen) {
    // Move the function call address into a scratch register
    Address disp = getInterModuleFuncAddr(callee, gen) - gen.currAddr();
    Register dest = gen.rs()->getScratchRegister(gen);
    if (dest == Null_Register) {
        bool useRVC;
        useRVC = insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, -GPRSIZE_64, gen.useCompressed());
        disp -= useRVC ? RISCVC_INSN_SIZE : RISCV_INSN_SIZE;
        useRVC = insnCodeGen::saveRegister(gen, GPR_RA, 0, true);
        disp -= useRVC ? RISCVC_INSN_SIZE : RISCV_INSN_SIZE;
        insnCodeGen::generateLoadImm(gen, GPR_RA, disp, true, true, gen.useCompressed());
        insnCodeGen::generateMemLoad(gen, GPR_RA, GPR_RA, 0, GPRSIZE_64, true, gen.useCompressed());
        insnCodeGen::generateJalr(gen, GPR_RA, GPR_RA, 0, gen.useCompressed());
        insnCodeGen::restoreRegister(gen, GPR_RA, 0, true);
        insnCodeGen::generateAddImm(gen, GPR_SP, GPR_SP, GPRSIZE_64, gen.useCompressed());
        insnCodeGen::generateJr(gen, GPR_RA, 0, gen.useCompressed());
    }
    else {
        gen.markRegDefined(dest);
        insnCodeGen::generateLoadImm(gen, dest, disp, true, true, gen.useCompressed());
        insnCodeGen::generateMemLoad(gen, dest, dest, 0, GPRSIZE_64, true, gen.useCompressed());
        insnCodeGen::generateJr(gen, dest, 0, gen.useCompressed());
    }
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
      emitMovePCToReg(dest, gen);
      emitLoadRelative(dest, varOffset, dest, gen.width(), gen);
      // Deference the pointer to get the variable
      insnCodeGen::generateMemLoad(gen, dest, dest, 0, 8, true,
                                   gen.useCompressed());
    } else {
      emitMovePCToReg(dest, gen);
      emitLoadRelative(dest, varOffset, dest, size, gen);
    }

  } else if (op == loadConstOp) {
    if (!is_local && (var != NULL)) {
      emitMovePCToReg(dest, gen);
      emitLoadRelative(dest, varOffset, dest, gen.width(), gen);
    } else {
      // Load effective address
      insnCodeGen::generateLoadImm(gen, dest, varOffset, true, true,
                                   gen.useCompressed());
    }
  } else {
    cerr << "Invalid op " << op << " in emitVstore" << endl;
    assert(0);
  }

  return;
}

void EmitterRISCV64::emitStoreShared(Register source, const image_variable *var,
                                     bool is_local, int size, codeGen &gen) {
  // Create or retrieve jump slot
  Address addr;
  int stackSize = 0;
  if (!is_local) {
    addr = getInterModuleVarAddr(var, gen);
  } else {
    addr = (Address)var->getOffset();
  }

  // Load register with address from jump slot
  Register baseReg = gen.rs()->getScratchRegister(gen, true);
  assert(baseReg != Null_Register && "cannot get a scratch register");

  Address varOffset = addr - gen.currAddr();

  if (!is_local) {
    std::vector<Register> exclude;
    exclude.push_back(baseReg);
    Register addrReg = gen.rs()->getScratchRegister(gen, exclude, true);
    assert(addrReg != Null_Register && "cannot get a scratch register");

    emitMovePCToReg(baseReg, gen);
    emitLoadRelative(addrReg, varOffset, baseReg, gen.width(), gen);
    emitStoreRelative(source, 0, addrReg, size, gen);
  } else {
    emitMovePCToReg(baseReg, gen);
    emitStoreRelative(source, varOffset, baseReg, size, gen);
  }

  assert(stackSize <= 0 && "stack not empty at the end");
}

Address Emitter::getInterModuleVarAddr(const image_variable *var,
                                       codeGen &gen) {
  AddressSpace *addrSpace = gen.addrSpace();
  if (!addrSpace)
    assert(0 && "No AddressSpace associated with codeGen object");

  BinaryEdit *binEdit = addrSpace->edit();
  Address relocation_address;

  unsigned int jump_slot_size;
  switch (addrSpace->getAddressWidth()) {
  case 4:
    jump_slot_size = 4;
    break;
  case 8:
    jump_slot_size = 8;
    break;
  default:
    assert(0 && "Encountered unknown address width");
  }

  if (!binEdit || !var) {
    assert(!"Invalid variable load (variable info is missing)");
  }

  // find the Symbol corresponding to the int_variable
  std::vector<SymtabAPI::Symbol *> syms;
  var->svar()->getSymbols(syms);

  if (syms.size() == 0) {
    cerr << "Cannot find symbol " << var->symTabName() << endl;
    assert(0);
  }

  // Try to find a dynamic symbol first
  // If none are found, take the first static symbol
  SymtabAPI::Symbol *referring = syms[0];
  for (unsigned k = 0; k < syms.size(); k++) {
    if (syms[k]->isInDynSymtab()) {
      referring = syms[k];
      break;
    }
  }

  // Create relocation if not exist
  relocation_address = binEdit->getDependentRelocationAddr(referring);
  if (!relocation_address) {
    relocation_address = binEdit->inferiorMalloc(jump_slot_size);
    unsigned char dat[8] = {0};
    binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);
    binEdit->addDependentRelocation(relocation_address, referring);
  }

  return relocation_address;
}

Address EmitterRISCV64::emitMovePCToReg(Register dest, codeGen &gen) {
  // auipc rd, 0
  insnCodeGen::generateAuipc(gen, dest, 0, gen.useCompressed());
  Address ret = gen.currAddr();
  return ret;
}

Address Emitter::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
  // from POWER64 getInterModuleFuncAddr

  AddressSpace *addrSpace = gen.addrSpace();
  if (!addrSpace)
    assert(0 && "No AddressSpace associated with codeGen object");

  BinaryEdit *binEdit = addrSpace->edit();
  Address relocation_address;

  unsigned int jump_slot_size;
  switch (addrSpace->getAddressWidth()) {
  case 4:
    jump_slot_size = 4;
    break; // l: not needed
  case 8:
    jump_slot_size = 24;
    break;
  default:
    assert(0 && "Encountered unknown address width");
  }

  if (!binEdit || !func) {
    assert(!"Invalid function call (function info is missing)");
  }

  // find the Symbol corresponding to the func_instance
  std::vector<SymtabAPI::Symbol *> syms;
  func->ifunc()->func()->getSymbols(syms);

  if (syms.size() == 0) {
    cerr << "Cannot find symbol " << func->symTabName() << endl;
    assert(0);
  }

  // Try to find a dynamic symbol first
  // If none are found, take the first static symbol
  SymtabAPI::Symbol *referring = syms[0];
  for (unsigned k = 0; k < syms.size(); k++) {
    if (syms[k]->isInDynSymtab()) {
      referring = syms[k];
      break;
    }
  }

  // Create relocation if not exist
  relocation_address = binEdit->getDependentRelocationAddr(referring);
  if (!relocation_address) {
    relocation_address = binEdit->inferiorMalloc(jump_slot_size);
    unsigned char dat[24] = {0};
    binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);
    binEdit->addDependentRelocation(relocation_address, referring);
  }
  return relocation_address;
}

regState_t::regState_t() : pc_rel_offset(-1), timeline(0), stack_height(0) {
  for (unsigned i = 0; i < 8; i++) {
    RealRegsState r;
    r.is_allocatable = (i != GPR_ZERO && i != GPR_RA && i != GPR_SP &&
                        i != GPR_GP && i != GPR_TP);
    r.been_used = false;
    r.last_used = 0;
    r.contains = NULL;
    registerStates.push_back(r);
  }
}

void registerSpace::initRealRegSpace() {
  for (unsigned i = 0; i < regStateStack.size(); i++) {
    if (regStateStack[i])
      delete regStateStack[i];
  }
  regStateStack.clear();

  regState_t *new_regState = new regState_t();
  regStateStack.push_back(new_regState);
  regs_been_spilled.clear();

  pc_rel_reg = Null_Register;
  pc_rel_use_count = 0;
}
