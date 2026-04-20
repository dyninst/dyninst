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
#include "function_cache.h"

namespace Dyninst { namespace DyninstAPI {

  class EmitterIA32 : public Emitterx86 {
  public:
    EmitterIA32() = default;

    virtual ~EmitterIA32() = default;

    bool clobberAllFuncCall(registerSpace *rs, func_instance *callee) override;

    void emitAddSignedImm(Address addr, int imm, codeGen &gen) override;

    bool emitAdjustStackPointer(int index, codeGen &gen) override;

    void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen) override;

    bool emitBTRestores(baseTramp *bt, codeGen &gen) override;

    bool emitBTSaves(baseTramp *bt, codeGen &gen) override;

    virtual Register emitCall(opCode op, codeGen &gen, const std::vector<codeGenASTPtr> &operands,
                              func_instance *callee) override;

    bool emitCallCleanup(codeGen &gen, func_instance *target, int frame_size,
                         std::vector<Register> &extra_saves);

    int emitCallParams(codeGen &gen, const std::vector<codeGenASTPtr> &operands, func_instance *target,
                       std::vector<Register> &extra_saves);

    bool emitCallRelative(Register, Address, Register, codeGen &) override;

    void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen) override;

    void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s) override;

    void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s) override;

    void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of,
                      codeGen &gen) override;

    void emitGetRetAddr(Register dest, codeGen &gen) override;

    void emitGetRetVal(Register dest, bool addr_of, codeGen &gen) override;

    codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen) override;

    void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest,
                 codeGen &gen) override;

    void emitLoad(Register dest, Address addr, int size, codeGen &gen) override;

    void emitLoadConst(Register dest, Address imm, codeGen &gen) override;

    void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) override;

    void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen) override;

    void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen) override;

    void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen) override;

    void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen,
                                 bool store) override;

    bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen) override;

    bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size,
                                codeGen &gen) override;

    void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size,
                        codeGen &gen, Address offset) override;

    bool emitMoveRegToReg(Register src, Register dest, codeGen &gen) override;

    bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) override;

    void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen) override;

    void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
                   codeGen &gen) override;

    bool emitPop(codeGen &gen, Register popee) override;

    bool emitPush(codeGen &gen, Register pushee) override;

    void emitPushFlags(codeGen &gen) override;

    void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s) override;

    void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen,
                      bool s) override;

    void emitRestoreFlags(codeGen &gen, unsigned offset) override;

    void emitRestoreFlagsFromStackSlot(codeGen &gen) override;

    void emitStackAlign(int offset, codeGen &gen);

    void emitStore(Address addr, Register src, int size, codeGen &gen) override;

    void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size,
                                codeGen &gen) override;

    void emitStoreImm(Address addr, int imm, codeGen &gen) override;

    void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) override;

    void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen) override;

    void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen) override;

    void emitStoreShared(Register source, const image_variable *var, bool is_local, int size,
                         codeGen &gen) override;

    void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen) override;

    bool emitXorRegImm(Register dest, int imm, codeGen &gen) override;

    bool emitXorRegReg(Register dest, Register base, codeGen &gen) override;

    bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen) override;

    bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen) override;

 private:
    // clobberAllFuncCall can be expensive, so don't re-analyze functions
    Dyninst::DyninstAPI::function_cache clobbered_functions;
  };

}}

#endif
