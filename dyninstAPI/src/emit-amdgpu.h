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

#ifndef _EMIT_AMDGPU_H
#define _EMIT_AMDGPU_H

#include "common/src/headers.h"
#include "dyninstAPI/src/ast.h"
#include "baseTramp.h"
#include "dyninstAPI/src/instPoint.h"
#include <assert.h>
#include <vector>

#include "dyninstAPI/src/amdgpu-gfx908-details.h"
#include "dyninstAPI/src/emitter.h"

class codeGen;

class registerSpace;

class baseTramp;

class EmitterAmdgpuGfx908 : public Emitter {
public:
  // emitIf semantics:
  // if (expr_reg == 0)
  //   jump to target address
  //

  // emitIf implementation approach:
  //
  // if (expr_reg != 0)
  //   jump to <else_block>
  //
  // jump_to_then_block (which jumps to target address)
  //
  // else_block:
  //

  // Actual instruction sequence:
  //
  // s_cmp_ne_u32 <expr_reg>, 0
  // s_cbranch_scc0 0
  //
  // jump_to_then_block:
  //   s_setpc_b64 [t:t+1]  (t, t+1 hold the target address)
  //
  // else_block:

  // The target address is held in an even aligned register pair
  // Return the index of s_setpc_b64 in the codegen memory buffer.
  // Note that RegControl is unused on AMDGPU, simply pass rc_no_control.
  codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);

  void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);

  // What is opcode2?
  void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
                 codeGen &gen);

  // The above signature is wierd, hence use the one below. TODO: later on, clean up the above mess
  void emitOpImmSimple(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen);

  // SALU relational operations on AMDGPU use SOPC encoding, and destination is
  // always SCC, have placeholder value as 0 for dest, mention in comment /*
  // SCC_DUMMY = */ when calling this.
  // ALSO FIXME: bool s seems like a redundant parameter
  void emitRelOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen,
                 bool s);

  void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen,
                    bool s);

  void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);

  void emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen);

  void emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);

  // TODO: Implementation requires full 'codeGen' and 'registerSpace' class to
  // allocate register. This method doesn't fit AMDGPU because addr needs to be
  // stored in 2 additional registers. Use emitLoadConst followed by
  // emitLoadIndir for the same effect.
  void emitLoad(Register dest, Address addr, int size, codeGen &gen);

  // Consider dest, dest+1 pairs to load the value
  // dest = lower 32 bits of imm (LSBs)
  // dest + 1 = upper 32 bits of imm (MSBs)
  void emitLoadConst(Register dest, Address imm, codeGen &gen);

  // s_load_dword, x2, x4, x8, x16
  void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);

  bool emitCallRelative(Register, Address, Register, codeGen &);

  bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);

  void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size,
                      codeGen &gen, Address offset);

  void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);

  // These implicitly use the stored original/non-inst value
  void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);

  void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen,
                               bool store);

  void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);

  void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

  void emitStore(Address addr, Register src, int size, codeGen &gen);

  void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);

  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size,
                              codeGen &gen);

  void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);

  void emitStoreShared(Register source, const image_variable *var, bool is_local, int size,
                       codeGen &gen);

  bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);

  bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);

  Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost,
                    func_instance *callee);

  void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);

  void emitGetRetAddr(Register dest, codeGen &gen);

  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op,
                    bool addr_of, codeGen &gen);

  void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);

  void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);

  void emitPushFlags(codeGen &gen);

  void emitRestoreFlags(codeGen &gen, unsigned offset);
  // Built-in offset...

  void emitRestoreFlagsFromStackSlot(codeGen &gen);

  bool emitBTSaves(baseTramp *bt, codeGen &gen);

  bool emitBTRestores(baseTramp *bt, codeGen &gen);

  // TODO: requires allocating / deallocating register (see next comment).
  void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);

  // TODO: Implementation requires full 'codeGen' and 'registerSpace' class to
  // allocate register. ALSO FIXME: bool noCost seems like a redundant
  // parameter.
  void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);

  bool emitPush(codeGen &, Register);

  bool emitPop(codeGen &, Register);

  bool emitAdjustStackPointer(int index, codeGen &gen);

  bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);

  // The additional interfaces

  // Emit numNops nops (numNops >= 1 & numNops <=16)
  void emitNops(unsigned numNops, codeGen &gen);

  // s[reg:reg+1] += constant
  void emitAddConstantToRegPair(Register reg, int constant, codeGen &gen);

  void emitEndProgram(codeGen &gen);

  // Set 32-bit reg = 32-bit literal
  void emitMovLiteral(Register reg, uint32_t literal, codeGen &gen);

  // wordOffset can be positive or negetive 16 bit value
  // conditionally set PC = PC + SignExtend(wordOffset * 4) + 4
  void emitConditionalBranch(bool onConditionTrue, int16_t wordOffset, codeGen &gen);

  // set PC = PC + SignExtend(wordOffset * 4) + 4
  void emitShortJump(int16_t wordOffset, codeGen &gen);

  void emitLongJump(Register reg, uint64_t fromAddress, uint64_t toAddress, codeGen &gen);

  void emitScalarDataCacheWriteback(codeGen &gen);

  void emitAtomicAdd(Register baseAddrReg, Register src0, codeGen &gen);

  void emitAtomicSub(Register baseAddrReg, Register src0, codeGen &gen);
private:
  // Some helper functions
  bool isValidSgpr(Register reg) const;
  bool isValidSgprBlock(Register regBlock) const;
  bool isValidSgprPair(Register regBlock) const;
};
#endif
