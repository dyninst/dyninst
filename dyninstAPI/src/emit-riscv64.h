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

#ifndef _EMITTER_RISCV64_H
#define _EMITTER_RISCV64_H

#include "common/src/headers.h"
#include "dyninstAPI/src/ast.h"
#include "baseTramp.h"
#include "dyninstAPI/src/instPoint.h"
#include <assert.h>
#include <vector>

#include "dyninstAPI/src/emitter.h"

class codeGen;

class registerSpace;

class baseTramp;

// class for encapsulating
// platform dependent code generation functions
class EmitterRISCV64 : public Emitter {

public:
  virtual ~EmitterRISCV64() {}

  virtual codeBufIndex_t emitIf(Register expr_reg, Register target,
                                RegControl rc, codeGen &gen);

  virtual void emitOp(unsigned opcode, Register dest, Register src1,
                      Register src2, codeGen &gen);

  virtual void emitOpImm(unsigned opcode, unsigned opcode2, Register src1,
                         Register src2, RegValue src2imm, codeGen &gen);

  virtual void emitRelOp(unsigned opcode, Register dest, Register src1,
                         Register src2, codeGen &gen, bool is_signed);

  virtual void emitRelOpImm(unsigned opcode, Register dest, Register src1,
                            RegValue src2, codeGen &gen, bool is_signed);

  virtual void emitDiv(Register dest, Register src1, Register src2,
                       codeGen &gen, bool is_signed);

  virtual void emitTimesImm(Register dest, Register src1, RegValue src2imm,
                            codeGen &gen);

  virtual void emitDivImm(Register dest, Register src1, RegValue src2imm,
                          codeGen &gen, bool is_signed);

  virtual void emitLoad(Register dest, Address addr, int size, codeGen &gen);

  virtual void emitLoadConst(Register dest, Address imm, codeGen &gen);

  virtual void emitLoadIndir(Register dest, Register addr_src, int size,
                             codeGen &gen);

  virtual bool emitCallRelative(Register dest, Address offset, Register baseReg,
                                codeGen &gen);

  virtual bool emitLoadRelative(Register dest, Address offset, Register baseReg,
                                int size, codeGen &gen);

  virtual void emitLoadShared(opCode op, Register dest,
                              const image_variable *var, bool is_local,
                              int size, codeGen &gen, Address offset);

  virtual void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);

  // These implicitly use the stored original/non-inst value
  virtual void emitLoadOrigFrameRelative(Register dest, Address offset,
                                         codeGen &gen);

  virtual void emitLoadOrigRegRelative(Register dest, Address offset,
                                       Register base, codeGen &gen, bool deref);

  virtual void emitLoadOrigRegister(Address register_num, Register destination,
                                    codeGen &gen);

  virtual void emitStore(Address addr, Register src, int size, codeGen &gen);

  virtual void emitStoreIndir(Register addr_reg, Register src, int size,
                              codeGen &gen);

  virtual void emitStoreFrameRelative(Address dest, Register src1,
                                      Register src2, int size, codeGen &gen);

  virtual void emitStoreRelative(Register source, Address offset,
                                 Register baseReg, int size, codeGen &gen);

  virtual void emitStoreShared(Register source, const image_variable *var,
                               bool is_local, int size, codeGen &gen);

  virtual void emitStoreOrigRegister(Address register_num, Register destination,
                                     codeGen &gen);

  virtual bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);

  virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest,
                                codeGen &gen);

  virtual Address emitMovePCToReg(Register dest, codeGen &gen);

  // This one we actually use now.
  virtual Register emitCall(opCode op, codeGen &gen,
                            const std::vector<AstNodePtr> &operands,
                            bool noCost, func_instance *callee);
  // virtual bool emitPIC(codeGen& /*gen*/, Address, Address )=0;

  virtual void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);

  virtual void emitGetRetAddr(Register dest, codeGen &gen);

  virtual void emitGetParam(Register dest, Register param_num,
                            instPoint::Type pt_type, opCode op, bool addr_of,
                            codeGen &gen);

  virtual void emitASload(int ra, int rb, int sc, long imm, Register dest,
                          int stackShift, codeGen &gen);

  virtual void emitCSload(int ra, int rb, int sc, long imm, Register dest,
                          codeGen &gen);

  virtual void emitPushFlags(codeGen &gen);

  virtual void emitRestoreFlags(codeGen &gen, unsigned offset);

  // Built-in offset...
  virtual void emitRestoreFlagsFromStackSlot(codeGen &gen);

  virtual bool emitBTSaves(baseTramp *bt, codeGen &gen);

  virtual bool emitBTRestores(baseTramp *bt, codeGen &gen);

  virtual void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);

  virtual void emitAddSignedImm(Address addr, int imm, codeGen &gen,
                                bool noCost);

  virtual bool emitPush(codeGen &gen, Register pushee);

  virtual bool emitPop(codeGen &gen, Register popee);

  virtual bool emitAdjustStackPointer(int index, codeGen &gen);

  virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
};

class EmitterRISCV64Dyn : public EmitterRISCV64 {
public:
  virtual ~EmitterRISCV64Dyn() {}
};

class EmitterRISCV64Stat : public EmitterRISCV64 {
public:
  virtual ~EmitterRISCV64Stat() {}

  virtual bool emitPLTCall(func_instance *dest, codeGen &gen);

  virtual bool emitPLTJump(func_instance *dest, codeGen &gen);
};

class EmitterRISCV64SaveRestoreRegs {
public:
  virtual ~EmitterRISCV64SaveRestoreRegs() {}

  unsigned saveGPRegisters(codeGen &gen, registerSpace *theRegSpace,
                           baseTramp *bt, int offset);

  unsigned saveFPRegisters(codeGen &gen, registerSpace *theRegSpace,
                           int offset);

  unsigned saveSPRegisters(codeGen &gen, registerSpace *, int offset,
                           bool force_save);

  void createFrame(codeGen &gen);

  unsigned restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace,
                              int offset);

  unsigned restoreFPRegisters(codeGen &gen, registerSpace *theRegSpace,
                              int offset);

  unsigned restoreSPRegisters(codeGen &gen, registerSpace *, int offset,
                              int force_save);

  void tearFrame(codeGen &gen);

  void restoreSPR(codeGen &gen, Register scratchReg, int sprnum, int stkOffset);

  void restoreFPRegister(codeGen &gen, Register reg, int save_off);

  void saveFPRegister(codeGen &gen, Register reg, int save_off);
};

#endif
