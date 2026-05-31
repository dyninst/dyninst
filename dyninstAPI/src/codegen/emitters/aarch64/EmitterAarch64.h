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

#ifndef DYNINST_DYNINSTAPI_CODEGEN_EMITTER_EMITTERAARCH64_H
#define DYNINST_DYNINSTAPI_CODEGEN_EMITTER_EMITTERAARCH64_H

/*
 *  Common code for emitting instructions on aarch64
 */

#include "emitter.h"
#include "function_cache.h"

namespace Dyninst { namespace DyninstAPI {

  class EmitterAarch64 : public Emitter {
  public:
    virtual ~EmitterAarch64() = default;

    bool clobberAllFuncCall(registerSpace *rs, func_instance *callee) override;

    codeBufIndex_t emitA(opCode op, Dyninst::Register src1, long dst, codeGen &gen,
                         Dyninst::DyninstAPI::RegControl rc) override;

    void emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift,
                          codeGen &gen) override;

    void emitAddSignedImm(Address, int, codeGen &) override;

    bool emitAdjustStackPointer(int, codeGen &) override;

    void emitASload(int, int, int, long, Dyninst::Register, int, codeGen &) override;

    bool emitBTRestores(baseTramp *, codeGen &) override;

    bool emitBTSaves(baseTramp *, codeGen &) override;

    Dyninst::Register emitCall(opCode, codeGen &,
                               const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &,
                               func_instance *) override;

    bool emitCallRelative(Dyninst::Register, Address, Dyninst::Register, codeGen &) override;

    void emitCountSpecLoad(const BPatch_countSpec_NP *, Dyninst::Register, codeGen &) override;

    void emitCSload(int, int, int, long, Dyninst::Register, codeGen &) override;

    void emitDiv(Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool) override;

    void emitDivImm(Dyninst::Register, Dyninst::Register, RegValue, codeGen &, bool) override;

    void emitGetParam(Dyninst::Register, Dyninst::Register, instPoint::Type, opCode, bool,
                      codeGen &) override;

    void emitGetRetAddr(Dyninst::Register, codeGen &) override;

    void emitGetRetVal(Dyninst::Register, bool, codeGen &) override;

    codeBufIndex_t emitIf(Dyninst::Register, Dyninst::Register, Dyninst::DyninstAPI::RegControl,
                          codeGen &) override;

    void emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst,
                 codeGen &gen, bool isSigned = true) override;

    void emitLoad(Dyninst::Register, Address, int, codeGen &) override;

    void emitLoadConst(Dyninst::Register, Address, codeGen &) override;

    void emitLoadFrameAddr(Dyninst::Register, Address, codeGen &) override;

    void emitLoadIndir(Dyninst::Register, Dyninst::Register, int, codeGen &) override;

    void emitLoadOrigFrameRelative(Dyninst::Register, Address, codeGen &) override;

    void emitLoadOrigRegister(Address, Dyninst::Register, codeGen &) override;

    void emitLoadOrigRegRelative(Dyninst::Register, Address, Dyninst::Register, codeGen &,
                                 bool) override;

    bool emitLoadRelative(Dyninst::Register, Address, Dyninst::Register, int, codeGen &) override;

    void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local,
                        int size, codeGen &gen, Address offset) override;

    Address emitMovePCToReg(Dyninst::Register, codeGen &gen) override;

    bool emitMoveRegToReg(Dyninst::Register, Dyninst::Register, codeGen &) override;

    bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) override;

    void emitOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register,
                codeGen &) override;

    void emitOpImm(unsigned, unsigned, Dyninst::Register, Dyninst::Register, RegValue,
                   codeGen &) override;

    bool emitPop(codeGen &, Dyninst::Register) override;

    bool emitPush(codeGen &, Dyninst::Register) override;

    void emitPushFlags(codeGen &) override;

    Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2,
                            Dyninst::Register dst, codeGen &gen,
                            const instPoint *location) override;

    void emitRelOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &,
                   bool) override;

    void emitRelOpImm(unsigned, Dyninst::Register, Dyninst::Register, RegValue, codeGen &,
                      bool) override;

    void emitRestoreFlags(codeGen &, unsigned) override;

    void emitRestoreFlagsFromStackSlot(codeGen &) override;

    void emitStore(Address, Dyninst::Register, int, codeGen &) override;

    void emitStoreFrameRelative(Address, Dyninst::Register, Dyninst::Register, int,
                                codeGen &) override;

    void emitStoreImm(Address, int, codeGen &) override;

    void emitStoreIndir(Dyninst::Register, Dyninst::Register, int, codeGen &) override;

    void emitStoreOrigRegister(Address, Dyninst::Register, codeGen &) override;

    void emitStoreRelative(Dyninst::Register, Address, Dyninst::Register, int, codeGen &) override;

    void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local,
                         int size, codeGen &gen) override;

    void emitTimesImm(Dyninst::Register, Dyninst::Register, RegValue, codeGen &) override;

    bool emitTOCCall(block_instance *dest, codeGen &gen) override;

    bool emitTOCJump(block_instance *dest, codeGen &gen) override;

    void emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen, int size = 4, AddressSpace *proc = NULL, bool s = true) override;

    void emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen, int size = 4, AddressSpace *proc = NULL) override;

    void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                    codeGen &gen, int size = 4, AddressSpace *proc = NULL) override;

    Address getInterModuleFuncAddr(func_instance *func, codeGen &gen) override;

    Address getInterModuleVarAddr(const image_variable *var, codeGen &gen) override;

  protected:
    bool emitCallInstruction(codeGen &, func_instance *, bool, Address);

    Dyninst::Register emitCallReplacement(opCode, codeGen &, func_instance *);

  private:
    // clobberAllFuncCall can be expensive, so don't re-analyze functions
    Dyninst::DyninstAPI::function_cache clobbered_functions;
  };

}}

#endif
