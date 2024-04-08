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

#include "dyninstAPI/src/inst-amdgpu.h"
#include "dyninstAPI/src/emit-amdgpu.h"
#include "dyninstAPI/src/registerSpace.h"

using namespace Dyninst;
using namespace Vega;

// ===== EmitterAmdgpuVega implementation begin =====

unsigned EmitterAmdgpuVega::emitIf(Register expr_reg, Register target,
                               RegControl rc, codeGen &gen) {
  // Caller must ensure that target is even; and target, target+1 hold the
  // target address.

  assert(target >= SGPR_0 && target <= SGPR_101 && "target must be an SGPR");
  assert(target % 2 == 0 &&
         "target must be even as we will use target, target+1 in pair");

  emitSopK(S_CMPK_EQ_U32, expr_reg, 0, gen);
  emitConditionalBranch(/* onConditionTrue = */ false, 0, gen);

  size_t setPcInstOffset = gen.getIndex();
  emitSop1(S_SETPC_B64, /* dest = */ 0, target, /* hasLiteral = */ false, 0,
           gen);

  return setPcInstOffset;
}

// EmitterAmdgpuVega implementation
void EmitterAmdgpuVega::emitOp(unsigned opcode, Register dest, Register src1,
                           Register src2, codeGen &gen) {
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
    assert(false && "opcode must correspond to a supported SOP2 operation");
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
    assert(false && "opcode must correspond to a supported SOP2 operation");
  }
  emitSop2(opcodeSop2, dest, src1, src2, gen);
}

void EmitterAmdgpuVega::emitOpImmSimple(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen) {
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
    assert(false &&
           "opcode must correspond to a supported SOPK operation");
  }
  emitSopK(opcodeSopK, src1, src2imm, gen);

}
void EmitterAmdgpuVega::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest,
                              Register src1, RegValue src2imm, codeGen &gen) {
  printf("not implemented yet\n");
}

void EmitterAmdgpuVega::emitRelOp(unsigned opcode, Register dest, Register src1,
                              Register src2, codeGen &gen, bool s) {
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
    assert(false && "opcode must correspond to a supported SOPC operation");
  }
  emitSopC(opcodeSopC, src1, src2, gen);
}

void EmitterAmdgpuVega::emitRelOpImm(unsigned op, Register dest, Register src1,
                                 RegValue src2imm, codeGen &gen, bool s) {
  uint32_t opcodeSopK = 0;
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
    assert(false &&
           "opcode must correspond to a supported SOPK relational operation");
  }
}

void EmitterAmdgpuVega::emitDiv(Register dest, Register src1, Register src2,
                            codeGen &gen, bool s) {
  printf("emitDiv not implemented yet\n");
}

void EmitterAmdgpuVega::emitTimesImm(Register dest, Register src1, RegValue src2imm,
                                 codeGen &gen) {
  assert(dest == src1 && "SOPK instructions require dest = src1");
  Vega::emitSopK(Vega::S_MULK_I32, dest, src2imm, gen);
}

void EmitterAmdgpuVega::emitDivImm(Register dest, Register src1, RegValue src2imm,
                               codeGen &gen, bool s) {
  printf("emitDivImm not implemented yet\n");
}

void EmitterAmdgpuVega::emitLoad(Register dest, Address addr, int size,
                             codeGen &gen) {
  printf("emitLoad not implemented yet\n");
}

void EmitterAmdgpuVega::emitLoadConst(Register dest, Address imm, codeGen &gen) {
  // Caller must ensure that dest is even; and dest, dest+1 are available.

  assert(sizeof(Address) == 8); // must be a 64-bit address
  Register reg0 = dest;
  Register reg1 = dest + 1;

  assert(dest >= SGPR_0 && dest <= SGPR_101 && "reg0 must be an SGPR");
  assert(reg0 % 2 == 0 &&
         "reg0 must be even as we will use reg0, reg1 in pair");

  uint32_t lowerAddress = imm;
  uint32_t upperAddress = (imm >> 32);

  emitMovLiteral(reg0, lowerAddress, gen);
  emitMovLiteral(reg1, upperAddress, gen);
}

void EmitterAmdgpuVega::emitLoadIndir(Register dest, Register addr_reg, int size,
                                  codeGen &gen) {
  emitLoadRelative(dest, /* offset =*/0, addr_reg, size, gen);
}

bool EmitterAmdgpuVega::emitCallRelative(Register, Address, Register, codeGen &) {
  printf("emitCallRelative not implemented yet\n");
  return 0;
}

bool EmitterAmdgpuVega::emitLoadRelative(Register dest, Address offset,
                                     Register base, int size, codeGen &gen) {
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

  int alignment = size >= 4 ? 4 : size;
  assert(dest % alignment == 0 && "destination register must be aligned");

  assert(dest + size - 1 <= SGPR_101 &&
         "must have consecutive registers to load <size> words");

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
    assert(false && "size can only be 1, 2, 4, 8 or 16");
  }

  emitSmem(loadOpcode, dest, (base >> 1), (uint64_t)offset, gen);

  // As per page 32 in the manual, 0 is the only legitimate value for scalar
  // memory reads. However, placement of waitcnt can be optimized later. Right
  // now set all counters to 0.
  //
  // TODO:
  // 1. Only set LGKM_CNT (simm16[11:8]) to 0 specifically.
  // 2. Optimize placement of waitcnt.
  emitSopP(S_WAITCNT, /* hasImm = */ true, 0, gen);

  return 0;
}

void EmitterAmdgpuVega::emitLoadShared(opCode op, Register dest,
                                       const image_variable *var, bool
                                       is_local, int size, codeGen &gen,
                                       Address offset) {
  printf("emitLoadShared not implemented yet\n");
}

void EmitterAmdgpuVega::emitLoadFrameAddr(Register dest, Address offset,
                                      codeGen &gen) {
  printf("emitLoadFrameAddr not implemented yet\n");
}

// These implicitly use the stored original/non-inst value
void EmitterAmdgpuVega::emitLoadOrigFrameRelative(Register dest, Address offset,
                                              codeGen &gen) {
  printf("emitLoadOrigFrameRelative not implemented yet\n");
}

void EmitterAmdgpuVega::emitLoadOrigRegRelative(Register dest, Address offset,
                                            Register base, codeGen &gen,
                                            bool store) {
  printf("emitLoadOrigRegRelative not implemented yet\n");
}

void EmitterAmdgpuVega::emitLoadOrigRegister(Address register_num, Register dest,
                                         codeGen &gen) {
  printf("emitLoadOrigRegister not implemented yet\n");
}

void EmitterAmdgpuVega::emitStoreOrigRegister(Address register_num, Register dest,
                                          codeGen &gen) {
  printf("emitStoreOrigRegister not implemented yet\n");
}

void EmitterAmdgpuVega::emitStore(Address addr, Register src, int size,
                              codeGen &gen) {
  printf("emitStore not implemented yet\n");
}

void EmitterAmdgpuVega::emitStoreIndir(Register addr_reg, Register src, int size,
                                   codeGen &gen) {
  emitStoreRelative(src, /*offset =*/0, addr_reg, size, gen);
}

void EmitterAmdgpuVega::emitStoreFrameRelative(Address offset, Register src,
                                           Register scratch, int size,
                                           codeGen &gen) {
  printf("emitStoreFrameRelative not implemented yet\n");
}

void EmitterAmdgpuVega::emitStoreRelative(Register source, Address offset,
                                      Register base, int size, codeGen &gen) {
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

  int alignment = size;
  assert(source % alignment == 0 && "source register must be aligned");

  assert(source + size - 1 <= SGPR_101 &&
         "must have <size> consecutive registers from source to store <size> "
         "words from");

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
    assert(false && "size can only be 1, 2, or 4");
  }

  emitSmem(storeOpcode, source, (base >> 1), (uint64_t)offset, gen);
}

void EmitterAmdgpuVega::emitStoreShared(Register source,
                                        const image_variable *var,
                                        bool is_local, int size, codeGen
                                        &gen) {
  printf("emitStoreShared not implemented yet\n");
}

bool EmitterAmdgpuVega::emitMoveRegToReg(Register src, Register dest,
                                     codeGen &gen) {

  emitSop1(S_MOV_B32, dest, src, /*hasLiteral =*/false, /*literal =*/0, gen);
  return 0;
}

bool EmitterAmdgpuVega::emitMoveRegToReg(registerSlot *src, registerSlot *dest,
                                         codeGen &gen) {
  printf("emitMoveRegToReg -- slot not implemented yet\n");
}

Register EmitterAmdgpuVega::emitCall(opCode op, codeGen &gen,
                                     const std::vector<AstNodePtr> &operands,
                                     bool noCost, func_instance *callee) {
  printf("emitCall not implemented yet\n");
}

void EmitterAmdgpuVega::emitGetRetVal(Register dest, bool addr_of, codeGen &gen) {
  printf("emitGetRetVal not implemented yet\n");
}

void EmitterAmdgpuVega::emitGetRetAddr(Register dest, codeGen &gen) {
  printf("emitGetRetAddr not implemented yet\n");
}

void EmitterAmdgpuVega::emitGetParam(Register dest, Register param_num,
                                     instPoint::Type pt_type, opCode op,
                                     bool addr_of, codeGen &gen) {
  printf("emitGetParam not implemented yet\n");
}

void EmitterAmdgpuVega::emitASload(int ra, int rb, int sc, long imm, Register dest,
                               int stackShift, codeGen &gen) {
  printf("emitASload not implemented yet\n");
}

void EmitterAmdgpuVega::emitCSload(int ra, int rb, int sc, long imm, Register dest,
                               codeGen &gen) {
  printf("emitCSload not implemented yet\n");
}

void EmitterAmdgpuVega::emitPushFlags(codeGen &gen) {
  printf("emitPushFlags not implemented yet\n");
}

void EmitterAmdgpuVega::emitRestoreFlags(codeGen &gen, unsigned offset) {
  printf("emitRestoreFlags not implemented yet\n");
}

// Built-in offset...
void EmitterAmdgpuVega::emitRestoreFlagsFromStackSlot(codeGen &gen) {
  printf("emitRestoreFlagsFromStackSlot not implemented yet\n");
}

bool EmitterAmdgpuVega::emitBTSaves(baseTramp *bt, codeGen &gen) {
  printf("emitBTSaves not implemented yet\n");
  return false;
}

bool EmitterAmdgpuVega::emitBTRestores(baseTramp *bt, codeGen &gen) {
  printf("emitBTRestores not implemented yet\n");
  return false;
}

void EmitterAmdgpuVega::emitStoreImm(Address addr, int imm, codeGen &gen,
                                 bool noCost) {
  printf("emitStoreImm not implemented yet\n");
}

void EmitterAmdgpuVega::emitAddSignedImm(Address addr, int imm, codeGen &gen,
                                     bool noCost) {
  printf("emitAddSignedImm not implemented yet\n");
}

bool EmitterAmdgpuVega::emitPush(codeGen &, Register) {
  printf("emitPush not implemented yet\n");
  return 0;
}

bool EmitterAmdgpuVega::emitPop(codeGen &, Register) {
  printf("emitPop not implemented yet\n");
  return 0;
}

bool EmitterAmdgpuVega::emitAdjustStackPointer(int index, codeGen &gen) {
  printf("emitAdjustStackPointer not implemented yet\n");
  return 0;
}

bool EmitterAmdgpuVega::clobberAllFuncCall(registerSpace *rs,
                                           func_instance *callee) {
  printf("clobberAllFuncCall not implemented yet\n");
  return false;
}

// Additional interfaces

void EmitterAmdgpuVega::emitNops(unsigned numNops, codeGen &gen) {
  assert(numNops >= 1 && numNops <= 16);
  // 0x0 inserts 1 nop, and 0xFF (15) inserts 16 nops, so subtract 1
  emitSopP(S_NOP, /* hasImm = */ true, (numNops - 1), gen);
}

void EmitterAmdgpuVega::emitEndProgram(codeGen &gen) {
  // Passing 0 as immediate value.
  // Value of immediate passed here doesn't matter as the instruction won't have
  // an immediate.
  emitSopP(S_ENDPGM, /* hasImm = */ false, 0, gen);
}

void EmitterAmdgpuVega::emitMovLiteral(Register reg, uint32_t literal,
                                   codeGen &gen) {
  // s_mov_b32 reg, < 32-bit constant literal >
  // The literal follows the instruction, so set src0 = 0xFF just like the
  // assembler does.
  emitSop1(S_MOV_B32, /* dest = */ reg, /* src0 = */ 0xFF,
           /* hasLiteral = */ true, literal, gen);
}

void EmitterAmdgpuVega::emitConditionalBranch(bool onConditionTrue,
                                          int16_t wordOffset, codeGen &gen) {
  unsigned opcode = onConditionTrue ? S_CBRANCH_SCC0 : S_CBRANCH_SCC1;
  emitSopP(opcode, /* hasImm = */ true, wordOffset, gen);
}

void EmitterAmdgpuVega::emitShortJump(int16_t wordOffset, codeGen &gen) {
  emitSopP(S_BRANCH, /* hasImm = */ true, wordOffset, gen);
}

void EmitterAmdgpuVega::emitLongJump(Register reg, uint64_t toAddress,
                                 codeGen &gen) {
  emitLoadConst(reg, toAddress, gen);

  // S_SETPC_B64 writes to the PC, so dest = 0 just like the assembler does.
  emitSop1(S_SETPC_B64, /* dest = */ 0, reg, /* hasLiteral = */ false, 0, gen);
}

// ===== EmitterAmdgpuVega implementation end =====
