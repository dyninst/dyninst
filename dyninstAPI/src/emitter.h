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

#ifndef _EMITTER_H
#define _EMITTER_H

#include "trampolines/baseTramp.h"
#include "BPatch_memoryAccess_NP.h"
#include "ASTs/codeGenAST.h"
#include "codegen/RegControl.h"
#include "patching/instPoint.h"

#include <cassert>
#include <vector>

class codeGen;
class registerSlot;
class registerSpace;

class Emitter {
public:
  virtual ~Emitter() {}

  /*** Function calls and arguments ***/
  virtual bool clobberAllFuncCall(registerSpace *rs,func_instance *callee) = 0;
  virtual Dyninst::Register emitCall(opCode op, codeGen &gen, const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands, func_instance *callee) = 0;
  virtual void emitGetParam(Dyninst::Register dest, Dyninst::Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen) = 0;
  virtual bool emitPLTCall(func_instance *, codeGen &) { assert(0); return false;}
  virtual bool emitPLTJump(func_instance *, codeGen &) { assert(0); return false;}
  virtual Dyninst::Address getInterModuleFuncAddr(func_instance *func, codeGen& gen) = 0;
  virtual Dyninst::Address getInterModuleVarAddr(const image_variable *var, codeGen& gen) = 0;


  /*** Arithmetic ***/
  virtual void emitTimesImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen) = 0;


  /*** Basic load operations ***/
  virtual void emitLoad(Dyninst::Register dest, Dyninst::Address addr, int size, codeGen &gen) = 0;
  virtual void emitLoadConst(Dyninst::Register dest, Dyninst::Address imm, codeGen &gen) = 0;
  virtual void emitLoadIndir(Dyninst::Register dest, Dyninst::Register addr_reg, int size, codeGen &gen) = 0;
  // These implicitly use the stored original/non-inst value
  virtual void emitLoadOrigRegRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, codeGen &gen, bool store) = 0;
  virtual void emitLoadOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen) = 0;
  virtual bool emitLoadRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen) = 0;
  virtual void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Dyninst::Address offset) = 0;


  /*** Basic store operations ***/
  virtual void emitStore(Dyninst::Address addr, Dyninst::Register src, int size, codeGen &gen) = 0;
  virtual void emitStoreIndir(Dyninst::Register addr_reg, Dyninst::Register src, int size, codeGen &gen) = 0;
  virtual void emitStoreRelative(Dyninst::Register source, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen) = 0;
  virtual void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen) = 0;


  /*** For BPatch_memoryAccess ***/
  virtual void emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen) = 0;
  virtual void emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest, int stackShift, codeGen &gen) = 0;
  virtual void emitCountSpecLoad(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen) = 0;
  virtual void emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest, codeGen &gen) = 0;


  /*** Register-direct operations ***/
  // TODO : Make all targets use this instead of having this functionality floating around in the codebase.
  virtual void emitMovePCtoReg(Dyninst::Register /* reg */, codeGen & /* gen */) {}
  virtual bool emitMoveRegToReg(Dyninst::Register src, Dyninst::Register dest, codeGen &gen) = 0;
  virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) = 0;


  /*** Generic operations ***/
  // Handles emission of things like ifs that need to be updated later.
  virtual codegen_arch::codeBufIndex_t emitA(opCode op, Dyninst::Register src1, long dst, codeGen &gen, Dyninst::DyninstAPI::RegControl rc) = 0;
  // for operations requiring a Dyninst::Register to be returned (e.g., getRetValOp, getRetAddrOp, getParamOp)
  virtual Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, const instPoint *location) = 0;
  // for general arithmetic and logic operations which return nothing
  virtual void emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
                    codeGen &gen, int size = 4, AddressSpace * proc = NULL, bool s = true) = 0;
  // for loadOp and loadConstOp (reading from an Dyninst::Address)
  virtual void emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                         codeGen &gen, int size = 4, AddressSpace * proc = NULL) = 0;
  // for storeOp (writing to an Dyninst::Address)
  virtual void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                          codeGen &gen, int size = 4, AddressSpace * proc = NULL) = 0;
  virtual void emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst, codeGen &gen, bool isSigned = true) = 0;
  virtual void emitOp(unsigned opcode, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen) = 0;
  virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen) = 0;
  virtual void emitRelOp(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s) = 0;
  virtual void emitRelOpImm(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s) = 0;

  /*** Complete if/then/else operation ***/
  virtual codegen_arch::codeBufIndex_t emitIf(Dyninst::Register expr_reg, Dyninst::Register target, Dyninst::DyninstAPI::RegControl rc, codeGen &gen) = 0;


  /*** AMDGPU-only ***/
  virtual void emitEndProgram(codeGen & /* gen */) {}
  virtual void emitNops(unsigned /* numNops */, codeGen & /* gen */) {}
  virtual void emitConditionalBranch(bool /* onConditionTrue */, int16_t /* wordOffset */, codeGen & /* gen */) {}
  virtual void emitLongJump(Dyninst::Register /* reg */, uint64_t /* fromAddress */, uint64_t /* toAddress */, codeGen & /* gen */) {}
  virtual void emitShortJump(int16_t /* wordOffset */, codeGen & /* gen */) {}
  virtual void emitMovLiteral(Dyninst::Register /* reg */, uint32_t /* value */, codeGen & /* gen */) {}
  virtual void emitOpImmSimple(unsigned /* op */, Dyninst::Register /* dest */, Dyninst::Register /* src1 */, Dyninst::RegValue /* src2imm */, codeGen & /* gen */) {}


  /*** x86-only ***/
  virtual void emitAddSignedImm(Dyninst::Address addr, int imm, codeGen &gen) = 0;
  virtual void emitDiv(Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s) = 0;
  virtual void emitDivImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s) = 0;
  virtual void emitGetRetAddr(Dyninst::Register dest, codeGen &gen) = 0;
  virtual void emitGetRetVal(Dyninst::Register dest, bool addr_of, codeGen &gen) = 0;
  virtual void emitLoadFrameAddr(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen) = 0;
  virtual void emitLoadOrigFrameRelative(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen) = 0;
  virtual void emitRestoreFlags(codeGen &gen, unsigned offset) = 0;
  virtual void emitRestoreFlagsFromStackSlot(codeGen &gen) = 0; // Built-in offset...
  virtual void emitStoreFrameRelative(Dyninst::Address offset, Dyninst::Register src, Dyninst::Register scratch, int size, codeGen &gen) = 0;
  virtual void emitStoreImm(Dyninst::Address addr, int imm, codeGen &gen) = 0;
  virtual void emitStoreOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen) = 0;
  virtual bool emitAdjustStackPointer(int index, codeGen &gen) = 0;
  virtual bool emitPop(codeGen &, Dyninst::Register) = 0;
  virtual bool emitPush(codeGen &, Dyninst::Register) = 0;
  virtual void emitPushFlags(codeGen &gen) = 0;
  virtual bool emitBTRestores(baseTramp* bt, codeGen &gen) = 0;
  virtual bool emitBTSaves(baseTramp* bt, codeGen &gen) = 0;


  /*** PowerPC-only ***/
  virtual bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &) = 0;
  virtual bool emitTOCCall(block_instance *, codeGen &) { assert(0); return false; }
  virtual bool emitTOCJump(block_instance *, codeGen &) { assert(0); return false; }
};

#endif
