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
 * emit-amdgpu.C - AMD GPU mi{25,50,100,200,300}  code generators (emitters)
 */

#include "dyninstAPI/src/emit-amdgpu.h"
#include "dyninstAPI/src/registerSpace.h"

using namespace Dyninst;
using namespace AmdgpuGfx908;

// ===== EmitterAmdgpuGfx908 implementation begin =====

// ==== Helper functions begin
bool EmitterAmdgpuGfx908::isValidSgpr(Register reg) const {
  auto regKind = reg.details.kind;
  auto numRegs = reg.details.count;
  auto baseRegId = reg.details.id;
  auto regUsage = reg.details.usage;
  return
  regKind == SCALAR &&
  numRegs < 2 &&
  baseRegId >= MIN_SGPR_ID && baseRegId <= MAX_SGPR_ID &&
  regUsage == GENERAL_PURPOSE;
}

bool EmitterAmdgpuGfx908::isValidSgprBlock(Register regBlock) const {
  auto regKind = regBlock.details.kind;
  auto numRegs = regBlock.details.count;
  auto baseRegId = regBlock.details.id;
  auto regUsage = regBlock.details.usage;
  auto lastReg = baseRegId + numRegs - 1;

  return regKind == SCALAR && numRegs > 0 && baseRegId >= MIN_SGPR_ID && lastReg <= MAX_SGPR_ID &&
         regUsage == GENERAL_PURPOSE;
}

// Register pairs must be even aligned
bool EmitterAmdgpuGfx908::isValidSgprPair(Register regBlock) const {
  auto numRegs = regBlock.details.count;
  auto regId = regBlock.details.id;
  return isValidSgprBlock(regBlock) && numRegs == 2 && regId % 2 == 0;
}

void EmitterAmdgpuGfx908::splitRegisterPair(Register regPair, std::vector<Register> &pieces) const {
  // TODO : This should also work for vectors later
  assert(isValidSgprPair(regPair) && "regPair must be a valid SGPR pair");
  uint32_t regId = regPair.details.id;
  uint32_t kind = regPair.details.kind;
  uint32_t usage = regPair.details.usage;

  Register r0(regId, static_cast<RegKind>(kind), static_cast<RegUsage>(usage), 0);
  Register r1(regId + 1, static_cast<RegKind>(kind), static_cast<RegUsage>(usage), 0);

  pieces.push_back(r0);
  pieces.push_back(r1);
}
// ==== Helper functions end

unsigned EmitterAmdgpuGfx908::emitIf(Register expr_reg, Register target, RegControl /* rc */,
                                     codeGen &gen) {
  assert(isValidSgprPair(target) && "target must be a valid SGPR pair");
  assert(isValidSgpr(expr_reg) && "expr_reg must be a valid single register");

  assert(target.getId() >= SGPR_0 && target.getId() <= SGPR_101 && "target must be an SGPR");
  assert(target.getId() % 2 == 0 && "target must be even as we will use target, target+1 in pair");

  emitSopK(S_CMPK_EQ_U32, expr_reg, 0, gen);
  emitConditionalBranch(/* onConditionTrue = */ false, 0, gen);

  size_t setPcInstOffset = gen.getIndex();
  emitSop1(S_SETPC_B64, /* dest = */ 0, target.getId(), /* hasLiteral = */ false, 0, gen);

  return setPcInstOffset;
}

// EmitterAmdgpuGfx908 implementation
void EmitterAmdgpuGfx908::emitOp(unsigned opcode, Register dest, Register src1, Register src2,
                                 codeGen &gen) {
  // TODO: We eventually want to get rid of this assert and generate code based on operand kind.
  assert(isValidSgpr(dest) && isValidSgpr(src1) && isValidSgpr(src2) && "each operand must be a valid SGPR");

  uint32_t opcodeSop2 = 0;
  switch (opcode) {
  case plusOp:
    opcodeSop2 = S_ADD_I32;
    break;

  case minusOp:
    opcodeSop2 = S_SUB_I32;
    break;

  case timesOp:
    opcodeSop2 = S_MUL_I32;
    break;

  case divOp:
    assert(!"opcode must correspond to a supported SOP2 operation");
    break;

  case andOp:
    opcodeSop2 = S_AND_B32;
    break;

  case orOp:
    opcodeSop2 = S_OR_B32;
    break;

  case xorOp:
    opcodeSop2 = S_XOR_B32;
    break;

  default:
    assert(!"opcode must correspond to a supported SOP2 operation");
  }

  const uint32_t destId = dest.details.id;
  const uint32_t src1Id = src1.details.id;
  const uint32_t src2Id = src2.details.id;
  emitSop2(opcodeSop2, destId, src1Id, src2Id, gen);
}

void EmitterAmdgpuGfx908::emitOpImmSimple(unsigned op, Register dest, Register src1,
                                          RegValue src2imm, codeGen &gen) {

  assert(isValidSgpr(dest) && isValidSgpr(src1) && "dest and src1 must be valid SGPRs");
  assert(dest == src1 && "dest and src1 must be the same for SOPK");

  uint32_t opcodeSopK = 0;
  switch (op) {
  case plusOp:
    opcodeSopK = S_ADDK_I32;
    break;
  case timesOp:
    opcodeSopK = S_MULK_I32;
    break;
  case lessOp:
    opcodeSopK = S_CMPK_LT_I32;
    break;
  case leOp:
    opcodeSopK = S_CMPK_LE_I32;
    break;
  case greaterOp:
    opcodeSopK = S_CMPK_GT_I32;
    break;
  case geOp:
    opcodeSopK = S_CMPK_GE_I32;
    break;
  case eqOp:
    opcodeSopK = S_CMPK_EQ_I32;
    break;
  case neOp:
    opcodeSopK = S_CMPK_LG_I32;
    break;
  default:
    assert(!"opcode must correspond to a supported SOPK operation");
  }

  uint32_t src1Id = src1.details.id;
  emitSopK(opcodeSopK, src1Id, src2imm, gen);
}

void EmitterAmdgpuGfx908::emitOpImm(unsigned /* opcode1 */, unsigned /* opcode2 */,
                                    Register /* dest */, Register /* src1 */,
                                    RegValue /* src2imm */, codeGen & /* gen */) {
  assert(!"not implemented yet");
}

void EmitterAmdgpuGfx908::emitRelOp(unsigned opcode, Register /* dest */, Register src1,
                                    Register src2, codeGen &gen, bool /* s */) {
  assert(isValidSgpr(src1) && isValidSgpr(src1) && "src1 and src2 must be valid SGPRs");

  uint32_t opcodeSopC = 0;
  switch (opcode) {
  case lessOp:
    opcodeSopC = S_CMP_LT_I32;
    break;

  case leOp:
    opcodeSopC = S_CMP_LE_I32;
    break;

  case greaterOp:
    opcodeSopC = S_CMP_GT_I32;
    break;

  case geOp:
    opcodeSopC = S_CMP_GE_I32;
    break;

  case eqOp:
    opcodeSopC = S_CMP_EQ_I32;
    break;

  case neOp:
    opcodeSopC = S_CMP_LG_I32;
    break;

  default:
    assert(!"opcode must correspond to a supported SOPC operation");
  }

  uint32_t src1Id = src1.details.id;
  uint32_t src2Id = src2.details.id;
  emitSopC(opcodeSopC, src1Id, src2Id, gen);
}

void EmitterAmdgpuGfx908::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
                                       codeGen &gen, bool /* s */) {
  switch (op) {
  case lessOp:
  case leOp:
  case greaterOp:
  case geOp:
  case eqOp:
  case neOp:
    emitOpImmSimple(op, dest, src1, src2imm, gen);
    break;
  default:
    assert(!"opcode must correspond to a supported SOPK relational operation");
  }
}

void EmitterAmdgpuGfx908::emitDiv(Register /* dest */, Register /* src1 */, Register /* src2 */,
                                  codeGen & /* gen */, bool /* s */) {
  assert(!"emitDiv not implemented yet");
}

void EmitterAmdgpuGfx908::emitTimesImm(Register dest, Register src1, RegValue src2imm,
                                       codeGen &gen) {
  assert(isValidSgpr(dest) && "dest must be a valid SGPR");
  assert(dest == src1 && "SOPK instructions require dest = src1");

  uint32_t destId = dest.details.id;
  AmdgpuGfx908::emitSopK(AmdgpuGfx908::S_MULK_I32, destId, src2imm, gen);
}

void EmitterAmdgpuGfx908::emitDivImm(Register /* dest */, Register /* src1 */,
                                     RegValue /* src2imm */, codeGen & /* gen */, bool /* s */) {
  assert(!"emitDivImm not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoad(Register /* dest */, Address /* addr */, int /* size */,
                                   codeGen & /* gen */) {
  assert(!"emitLoad not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadConst(Register dest, Address imm, codeGen &gen) {
  // Caller must ensure that dest is a SGPR pair beginning at an even reg id.
  assert(isValidSgprPair(dest) && "dest must be a valid SGPR pair");
  assert(sizeof(Address) == 8); // must be a 64-bit address
  Register reg0 = dest;
  Register reg1 = dest + 1;

  assert(dest.getId() >= SGPR_0 && dest.getId() <= SGPR_101 && "reg0 must be an SGPR");
  assert(reg0.getId() % 2 == 0 && "reg0 must be even as we will use reg0, reg1 in pair");

  uint32_t lowerAddress = imm;
  uint32_t upperAddress = (imm >> 32);

  std::vector<Register> regs;
  splitRegisterPair(dest, regs);

  emitMovLiteral(regs[0], lowerAddress, gen);
  emitMovLiteral(regs[1], upperAddress, gen);
}

void EmitterAmdgpuGfx908::emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen) {
  emitLoadRelative(dest, /* offset =*/0, addr_reg, size, gen);
}

bool EmitterAmdgpuGfx908::emitCallRelative(Register, Address, Register, codeGen &) {
  assert(!"emitCallRelative not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitLoadRelative(Register dest, Address offset, Register base, int size,
                                           codeGen &gen) {
  // Caller must ensure the following:
  //
  // 1. base is even aligned and base, base + 1 contain the address.
  // 2. <size> registers starting from dest are available.
  // 3. offset must be a 21-bit signed value. Sign-extend to int64_t and cast to
  // uint64_t before calling if necessary (this is bad).
  // 3. Alignment requirement for dest:
  //    size = 1  : 1
  //    size = 2  : 2
  //    size >= 4 : 4
  // size = 1, 2, 4, 8, 16

  assert(size == 1 || size == 2 || size == 4 || size == 8 || size == 16);

  if (size == 1)
    assert(isValidSgpr(dest) && "dest must be a valid SGPR");
  else
    assert(isValidSgprBlock(dest) && dest.details.count == size &&
           "dest must be a register block of size 'size'");

  uint32_t alignment = size >= 4 ? 4 : size;
  uint32_t destId = dest.details.id;

  assert(dest.details.kind == SCALAR && "dest must be scalar");
  assert(destId % alignment == 0 && "destination register must be aligned");

  assert(isValidSgprPair(base) && "base must be a valid SGPR pair");

  unsigned loadOpcode = 0;
  switch (size) {
  case 1:
    loadOpcode = S_LOAD_DWORD;
    break;
  case 2:
    loadOpcode = S_LOAD_DWORDX2;
    break;
  case 4:
    loadOpcode = S_LOAD_DWORDX4;
    break;
  case 8:
    loadOpcode = S_LOAD_DWORDX8;
    break;
  case 16:
    loadOpcode = S_LOAD_DWORDX16;
    break;
  default:
    assert(!"size can only be 1, 2, 4, 8 or 16");
  }

  uint32_t baseId = base.details.id;
  emitSmem(loadOpcode, destId, (baseId >> 1), (uint64_t)offset, gen);

  // As per page 32 in the manual, 0 is the only legitimate value for scalar
  // memory reads. However, placement of waitcnt can be optimized later. Right
  // now set all counters to 0.
  //
  // TODO:
  // 1. Only set LGKM_CNT (simm16[11:8]) to 0 specifically.
  // 2. Optimize placement of waitcnt.
  emitSopP(S_WAITCNT, /* simm16 = */ 0, gen);

  return false;
}

void EmitterAmdgpuGfx908::emitLoadShared(opCode /* op */, Register /* dest */,
                                         const image_variable * /* var */,
                                         bool
                                         /* is_local */,
                                         int /* size */, codeGen & /* gen */,
                                         Address /* offset */) {
  assert(!"emitLoadShared not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadFrameAddr(Register /* dest */, Address /* offset */,
                                            codeGen & /* gen */) {
  assert(!"emitLoadFrameAddr not implemented yet");
}

// These implicitly use the stored original/non-inst value
void EmitterAmdgpuGfx908::emitLoadOrigFrameRelative(Register /* dest */, Address /* offset */,
                                                    codeGen & /* gen */) {
  assert(!"emitLoadOrigFrameRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadOrigRegRelative(Register /* dest */, Address /* offset */,
                                                  Register /* base */, codeGen & /* gen */,
                                                  bool /* store */) {
  assert(!"emitLoadOrigRegRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitLoadOrigRegister(Address /* register_num */, Register /* dest */,
                                               codeGen & /* gen */) {
  assert(!"emitLoadOrigRegister not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreOrigRegister(Address /* register_num */, Register /* dest */,
                                                codeGen & /* gen */) {
  assert(!"emitStoreOrigRegister not implemented yet");
}

void EmitterAmdgpuGfx908::emitStore(Address /* addr */, Register /* src */, int /* size */,
                                    codeGen & /* gen */) {
  assert(!"emitStore not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) {
  emitStoreRelative(src, /*offset =*/0, addr_reg, size, gen);
}

void EmitterAmdgpuGfx908::emitStoreFrameRelative(Address /* offset */, Register /* src */,
                                                 Register /* scratch */, int /* size */,
                                                 codeGen & /* gen */) {
  assert(!"emitStoreFrameRelative not implemented yet");
}

void EmitterAmdgpuGfx908::emitStoreRelative(Register source, Address offset, Register base,
                                            int size, codeGen &gen) {
  // Caller must ensure the following:
  //
  // 1. base is even aligned and base, base + 1 contain the address.
  // 2. <size> registers starting from source hold the value to be stored.
  // 3. offset must be a 21-bit signed value. Sign-extend to int64_t and cast to
  // uint64_t before calling if necessary (this is bad).
  // 3. Alignment requirement for source:
  //     alignment = size
  //     size = 1, 2, 4

  assert(size == 1 || size == 2 || size == 4);

  if (size == 1)
    assert(isValidSgpr(source) && "source must be a valid SGPR");
  else
    assert(isValidSgprBlock(source) && source.details.count == size &&
           "source must be a register block of size 'size'");

  assert(isValidSgprPair(base) && "base must be a valid SGPR pair");

  uint32_t baseId = base.details.id;
  uint32_t sourceId = source.details.id;
  uint32_t alignment = static_cast<uint32_t>(size);

  assert(sourceId % alignment == 0 && "source register block must be properly aligned");

  unsigned storeOpcode = 0;
  switch (size) {
  case 1:
    storeOpcode = S_STORE_DWORD;
    break;
  case 2:
    storeOpcode = S_STORE_DWORDX2;
    break;
  case 4:
    storeOpcode = S_STORE_DWORDX4;
    break;
  default:
    assert(!"size can only be 1, 2, or 4");
  }

  emitSmem(storeOpcode, sourceId, (baseId >> 1), (uint64_t)offset, gen);

  emitSopP(S_WAITCNT, /* simm16 = */ 0, gen);
}

void EmitterAmdgpuGfx908::emitStoreShared(Register /* source */, const image_variable * /* var */,
                                          bool /* is_local */, int /* size */,
                                          codeGen & /* gen */) {
  assert(!"emitStoreShared not implemented yet");
}

bool EmitterAmdgpuGfx908::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
  // TODO:
  // 1. Allow this to move entire register blocks at the same time (using mulple instructions)
  // 2. Make this work with VGPR-VGPR movs
  assert(isValidSgpr(src) && isValidSgpr(dest) && "src and dest must be valid SGPRs");
  uint32_t destId = dest.details.id;
  uint32_t srcId = src.details.id;
  emitSop1(S_MOV_B32, destId, srcId, /*hasLiteral =*/false, /*literal =*/0, gen);
  return false;
}

bool EmitterAmdgpuGfx908::emitMoveRegToReg(registerSlot * /* src */, registerSlot * /* dest */,
                                           codeGen & /* gen */) {
  assert(!"emitMoveRegToReg -- slot not implemented yet");
  return false;
}

Register EmitterAmdgpuGfx908::emitCall(opCode /* op */, codeGen & /* gen */,
                                       const std::vector<AstNodePtr> & /* operands */,
                                       bool /* noCost */, func_instance * /* callee */) {
  assert(!"emitCall not implemented yet");
  return 0;
}

void EmitterAmdgpuGfx908::emitGetRetVal(Register /* dest */, bool /* addr_of */,
                                        codeGen & /* gen */) {
  assert(!"emitGetRetVal not implemented yet");
}

void EmitterAmdgpuGfx908::emitGetRetAddr(Register /* dest */, codeGen & /* gen */) {
  assert(!"emitGetRetAddr not implemented yet");
}

void EmitterAmdgpuGfx908::emitGetParam(Register /* dest */, Register /* param_num */,
                                       instPoint::Type /* pt_type */, opCode /* op */,
                                       bool /* addr_of */, codeGen & /* gen */) {
  assert(!"emitGetParam not implemented yet");
}

void EmitterAmdgpuGfx908::emitASload(int /* ra */, int /* rb */, int /* sc */, long /* imm */,
                                     Register /* dest */, int /* stackShift */,
                                     codeGen & /* gen */) {
  assert(!"emitASload not implemented yet");
}

void EmitterAmdgpuGfx908::emitCSload(int /* ra */, int /* rb */, int /* sc */, long /* imm */,
                                     Register /* dest */, codeGen & /* gen */) {
  assert(!"emitCSload not implemented yet");
}

void EmitterAmdgpuGfx908::emitPushFlags(codeGen & /* gen */) {
  assert(!"emitPushFlags not implemented yet");
}

void EmitterAmdgpuGfx908::emitRestoreFlags(codeGen & /* gen */, unsigned /* offset */) {
  assert(!"emitRestoreFlags not implemented yet");
}

// Built-in offset...
void EmitterAmdgpuGfx908::emitRestoreFlagsFromStackSlot(codeGen & /* gen */) {
  assert(!"emitRestoreFlagsFromStackSlot not implemented yet");
}

bool EmitterAmdgpuGfx908::emitBTSaves(baseTramp * /* bt */, codeGen & /* gen */) {
  assert(!"emitBTSaves not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitBTRestores(baseTramp * /* bt */, codeGen & /* gen */) {
  assert(!"emitBTRestores not implemented yet");
  return false;
}

void EmitterAmdgpuGfx908::emitStoreImm(Address /* addr */, int /* imm */, codeGen & /* gen */,
                                       bool /* noCost */) {
  assert(!"emitStoreImm not implemented yet");
}

void EmitterAmdgpuGfx908::emitAddSignedImm(Address /* addr */, int /* imm */, codeGen & /* gen */,
                                           bool /* noCost */) {
  assert(!"emitAddSignedImm not implemented yet");
}

bool EmitterAmdgpuGfx908::emitPush(codeGen &, Register) {
  assert(!"emitPush not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitPop(codeGen &, Register) {
  assert(!"emitPop not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::emitAdjustStackPointer(int /* index */, codeGen & /* gen */) {
  assert(!"emitAdjustStackPointer not implemented yet");
  return false;
}

bool EmitterAmdgpuGfx908::clobberAllFuncCall(registerSpace * /* rs */,
                                             func_instance * /* callee */) {
  assert(!"clobberAllFuncCall not implemented yet");
  return false;
}

// Additional interfaces

void EmitterAmdgpuGfx908::emitNops(unsigned numNops, codeGen &gen) {
  assert(numNops >= 1 && numNops <= 16);
  // 0x0 inserts 1 nop, and 0xFF (15) inserts 16 nops, so subtract 1
  emitSopP(S_NOP, /* simm16 = */ (numNops - 1), gen);
}

void EmitterAmdgpuGfx908::emitEndProgram(codeGen &gen) {
  // Passing 0 as immediate value.
  // Value of immediate passed here doesn't matter as the instruction won't have
  // an immediate.
  emitSopP(S_ENDPGM, /* simm16 = */ 0, gen);
}

void EmitterAmdgpuGfx908::emitMovLiteral(Register reg, uint32_t literal, codeGen &gen) {
  assert(isValidSgpr(reg) && "reg must be a valid SGPR");
  // s_mov_b32 reg, < 32-bit constant literal >
  // The literal follows the instruction, so set src0 = 0xFF just like the
  // assembler does.
  uint32_t regId = reg.details.id;
  emitSop1(S_MOV_B32, /* dest = */ regId, /* src0 = */ 0xFF,
           /* hasLiteral = */ true, literal, gen);
}

void EmitterAmdgpuGfx908::emitConditionalBranch(bool onConditionTrue, int16_t wordOffset,
                                                codeGen &gen) {
  unsigned opcode = onConditionTrue ? S_CBRANCH_SCC0 : S_CBRANCH_SCC1;
  emitSopP(opcode, /* simm16 = */ wordOffset, gen);
}

void EmitterAmdgpuGfx908::emitShortJump(int16_t wordOffset, codeGen &gen) {
  emitSopP(S_BRANCH, /* simm16 = */ wordOffset, gen);
}

// For this we need reg, reg+1, reg+2, reg+3.
void EmitterAmdgpuGfx908::emitLongJump(Register reg, uint64_t fromAddress, uint64_t toAddress,
                                       codeGen &gen) {
  assert(reg.getId() >= SGPR_0 && reg.getId() <= SGPR_101 && "reg must be an SGPR");
  assert(reg.getId() % 2 == 0 && "reg must be even as we will use reg, reg+1 in pair");

  // s_getpc_b64 will give us beginning of next instruction.
  // So our fromAddress must be incremented by 4 to accomodate for this.
  int64_t signedFromAddress = (int64_t)fromAddress + 4;
  int64_t signedToAddress = (int64_t)toAddress;

  assert(signedFromAddress > 0 && signedToAddress > 0 && "Both addresses must be positive");
  int64_t diff = signedToAddress - signedFromAddress;

  emitSop1(S_GETPC_B64, /* dest = */ regId, /* src0 =*/0, /* hasLiteral = */ false, /* literal=*/0,
           gen);

  Register diffReg(regId + 2, SCALAR, GENERAL_PURPOSE, 2);
  this->emitLoadConst(diffReg, (uint64_t)diff, gen);
  // Now we have:
  // reg+2 = lower bits of diff
  // reg+3 = upper bits of diff

  // reg = reg + <reg+2>, SCC = carry
  emitSop2(S_ADD_U32, regId, regId, regId + 2, gen);
  // <reg+1> = <reg+1> + <reg+3> + carry
  emitSop2(S_ADDC_U32, regId + 1, regId + 1, regId + 3, gen);

  // S_SETPC_B64 writes to the PC, so dest = 0 just like the assembler does.
  emitSop1(S_SETPC_B64, /* dest = */ 0, regId, /* hasLiteral = */ false, /* literal=*/0, gen);
}

void EmitterAmdgpuGfx908::emitAddConstantToRegPair(Register reg, int constant, codeGen &gen) {
  assert(isValidSgprPair(reg) && "reg must be a valid SGPR pair");
  uint32_t regId = reg.details.id;
  // reg has lower bits
  emitSop2WithSrc1Literal(S_ADD_U32, regId, regId, constant, gen);

  // reg+1 has upper bits. Add 0 with carry.
  emitSop2WithSrc1Literal(S_ADDC_U32, regId + 1, regId + 1, 0, gen);
}

void EmitterAmdgpuGfx908::emitScalarDataCacheWriteback(codeGen &gen) {
  emitSmem(S_DCACHE_WB, 0, 0, 0, gen);
}

void EmitterAmdgpuGfx908::emitAtomicAdd(Register baseAddrReg, Register src0, codeGen &gen) {
  assert(isValidSgprPair(baseAddrReg) && "baseAddrReg must be a valid SGPR pair");
  assert(isValidSgpr(src0) && "src0 must be a valid SGPR");

  uint32_t baseAddrRegId = baseAddrReg.details.id;
  uint32_t src0Id = src0.details.id;
  emitSmem(S_ATOMIC_ADD, src0Id, baseAddrRegId >> 1, /* offset = */ 0, gen);
}

void EmitterAmdgpuGfx908::emitAtomicSub(Register baseAddrReg, Register src0, codeGen &gen) {
  assert(isValidSgprPair(baseAddrReg) && "baseAddrReg must be a valid SGPR pair");
  assert(isValidSgpr(src0) && "src0 must be a valid SGPR");

  uint32_t baseAddrRegId = baseAddrReg.details.id;
  uint32_t src0Id = src0.details.id;
  emitSmem(S_ATOMIC_SUB, src0Id, baseAddrRegId >> 1, /* offset = */ 0, gen);
}
// ===== EmitterAmdgpuGfx908 implementation end =====
