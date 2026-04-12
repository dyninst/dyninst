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

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_IA32_EMITTERIA32_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTERS_X86_IA32_EMITTERIA32_H

/*
 *  Common code for emitting instructions on IA32
 */

#include "ASTs/codeGenAST.h"
#include "codegen/emitters/x86/Emitterx86.h"

namespace Dyninst { namespace DyninstAPI {

  class EmitterIA32 : public Emitterx86 {
  public:
    EmitterIA32() = default;

    virtual ~EmitterIA32() = default;

    bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);

    void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);

    bool emitAdjustStackPointer(int index, codeGen &gen);

    void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);

    bool emitBTRestores(baseTramp *bt, codeGen &gen);

    bool emitBTSaves(baseTramp *bt, codeGen &gen);

    virtual Register emitCall(opCode op, codeGen &gen, const std::vector<codeGenASTPtr> &operands,
                              bool noCost, func_instance *callee);

    bool emitCallCleanup(codeGen &gen, func_instance *target, int frame_size,
                         std::vector<Register> &extra_saves);

    int emitCallParams(codeGen &gen, const std::vector<codeGenASTPtr> &operands, func_instance *target,
                       std::vector<Register> &extra_saves, bool noCost);

    bool emitCallRelative(Register, Address, Register, codeGen &);

    void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);

    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);

    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s);

    void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of,
                      codeGen &gen);

    void emitGetRetAddr(Register dest, codeGen &gen);

    void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);

    codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);

    void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen);

    void emitLoad(Register dest, Address addr, int size, codeGen &gen);

    void emitLoadConst(Register dest, Address imm, codeGen &gen);

    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);

    void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);

    void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);

    void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);

    void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);

    bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);

    bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen);

    void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size,
                        codeGen &gen, Address offset);

    bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);

    bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);

    void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);

    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
                   codeGen &gen);

    bool emitPop(codeGen &gen, Register popee);

    bool emitPush(codeGen &gen, Register pushee);

    void emitPushFlags(codeGen &gen);

    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);

    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);

    void emitRestoreFlags(codeGen &gen, unsigned offset);

    void emitRestoreFlagsFromStackSlot(codeGen &gen);

    void emitStackAlign(int offset, codeGen &gen);

    void emitStore(Address addr, Register src, int size, codeGen &gen);

    void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);

    void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);

    void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);

    void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

    void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);

    void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen);

    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen);

    bool emitXorRegImm(Register dest, int imm, codeGen &gen);

    bool emitXorRegReg(Register dest, Register base, codeGen &gen);

    bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen);

    bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen);

  protected:
    virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret) = 0;
  };

}}

#endif
