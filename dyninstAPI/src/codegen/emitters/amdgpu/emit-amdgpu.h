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

#include "codegen/RegControl.h"
#include "trampolines/baseTramp.h"
#include "patching/instPoint.h"
#include <assert.h>
#include "ASTs/codeGenAST.h"
#include <cstdint>
#include <vector>

#include "codegen/emitters/amdgpu/gfx908/amdgpu-gfx908-details.h"
#include "dyninstAPI/src/emitter.h"

class codeGen;

class registerSpace;

class baseTramp;

// The scalar opcodes the architecture-common emitter emits, as (format, mnemonic) pairs. This one list
// drives both the AmdgpuOp enum and every per-gen EmitterAmdgpu<Gen>::opcode() switch, so the opcode set
// lives in exactly one place. The concrete value for each (format, mnemonic) comes from that gen's
// GENERATED amdgpu_<gen>::op::<FORMAT>::<MNEMONIC> constant (AMDGEN), never a hand-written table -- a gen
// whose generated header lacks one of these fails to compile its subclass (the loud failure we want).
// (format, mnemonic) is the key because a mnemonic alone is not unique across formats.
#define DYNINST_AMDGPU_OPS(X)                                                                              \
  X(ENC_SOP1, S_MOV_B32) X(ENC_SOP1, S_GETPC_B64) X(ENC_SOP1, S_SETPC_B64) X(ENC_SOP1, S_SWAPPC_B64)      \
  X(ENC_SOP2, S_ADD_I32) X(ENC_SOP2, S_SUB_I32) X(ENC_SOP2, S_MUL_I32) X(ENC_SOP2, S_AND_B32)             \
  X(ENC_SOP2, S_OR_B32) X(ENC_SOP2, S_XOR_B32) X(ENC_SOP2, S_ADD_U32) X(ENC_SOP2, S_ADDC_U32)             \
  X(ENC_SOP2, S_CSELECT_B32)                                                                               \
  X(ENC_SOPK, S_ADDK_I32) X(ENC_SOPK, S_MULK_I32) X(ENC_SOPK, S_CMPK_LT_I32) X(ENC_SOPK, S_CMPK_LE_I32)   \
  X(ENC_SOPK, S_CMPK_GT_I32) X(ENC_SOPK, S_CMPK_GE_I32) X(ENC_SOPK, S_CMPK_EQ_I32)                         \
  X(ENC_SOPK, S_CMPK_LG_I32) X(ENC_SOPK, S_CMPK_EQ_U32) X(ENC_SOPK, S_GETREG_B32)                         \
  X(ENC_SOPC, S_CMP_LT_I32) X(ENC_SOPC, S_CMP_LE_I32) X(ENC_SOPC, S_CMP_GT_I32) X(ENC_SOPC, S_CMP_GE_I32) \
  X(ENC_SOPC, S_CMP_EQ_I32) X(ENC_SOPC, S_CMP_LG_I32)                                                     \
  X(ENC_SMEM, S_LOAD_DWORD) X(ENC_SMEM, S_LOAD_DWORDX2) X(ENC_SMEM, S_LOAD_DWORDX4)                        \
  X(ENC_SMEM, S_LOAD_DWORDX8) X(ENC_SMEM, S_LOAD_DWORDX16) X(ENC_SMEM, S_STORE_DWORD)                      \
  X(ENC_SMEM, S_STORE_DWORDX2) X(ENC_SMEM, S_STORE_DWORDX4) X(ENC_SMEM, S_DCACHE_WB)                       \
  X(ENC_SMEM, S_ATOMIC_ADD) X(ENC_SMEM, S_ATOMIC_SUB)                                                     \
  X(ENC_SOPP, S_WAITCNT) X(ENC_SOPP, S_NOP) X(ENC_SOPP, S_ENDPGM) X(ENC_SOPP, S_BRANCH)                   \
  X(ENC_SOPP, S_CBRANCH_SCC0) X(ENC_SOPP, S_CBRANCH_SCC1) X(ENC_SOPP, S_TTRACEDATA)                       \
  X(ENC_SOP2, S_LSHR_B32)                                                                                  \
  X(ENC_VOP1, V_MOV_B32) X(ENC_VOP1, V_READFIRSTLANE_B32)                                                  \
  X(ENC_VOP2, V_ADD_CO_U32) X(ENC_VOP2, V_ADDC_CO_U32)                                                     \
  X(ENC_VOP3, V_LSHL_OR_B32) X(ENC_VOP3, V_READLANE_B32) X(ENC_VOP3, V_WRITELANE_B32)                      \
  X(ENC_FLAT_GLBL, GLOBAL_LOAD_DWORD) X(ENC_FLAT_GLBL, GLOBAL_STORE_DWORD)

// Gen-neutral opcode identity. The base emitter names opcodes through this enum; each per-gen subclass
// maps it to that gen's generated value via opcode() (below). Mnemonics in the base set are unique, so
// the enumerator is the bare mnemonic.
enum class AmdgpuOp : uint16_t {
#define DYNINST_AMDGPU_OP_ENUMERATOR(fmt, name) name,
  DYNINST_AMDGPU_OPS(DYNINST_AMDGPU_OP_ENUMERATOR)
#undef DYNINST_AMDGPU_OP_ENUMERATOR
};

class EmitterAmdgpu : public Emitter {
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
  // Note that Dyninst::DyninstAPI::RegControl is unused on AMDGPU, simply pass Dyninst::DyninstAPI::RegControl::rc_no_control.
  codeBufIndex_t emitIf(Register expr_reg, Register target, Dyninst::DyninstAPI::RegControl rc, codeGen &gen);

  codeBufIndex_t emitA(opCode op, Dyninst::Register src1, long dst, codeGen &gen, Dyninst::DyninstAPI::RegControl rc);

  Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, const instPoint *location);

  void emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
             codeGen &gen, int size = 4, AddressSpace * proc = NULL, bool s = true);

  void emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                 codeGen &gen, int size = 4, AddressSpace * proc = NULL);

  void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                  codeGen &gen, int size = 4, AddressSpace * proc = NULL);

  void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);

  // What is opcode2?
  void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm,
                 codeGen &gen);

  void emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst, codeGen &gen, bool isSigned = true);

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

  Register emitCall(opCode op, codeGen &gen, const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands,
                    func_instance *callee);

  void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);

  void emitGetRetAddr(Register dest, codeGen &gen);

  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op,
                    bool addr_of, codeGen &gen);

  void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);

  void emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen);

  void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);

  void emitCountSpecLoad(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen);

  void emitPushFlags(codeGen &gen);

  void emitRestoreFlags(codeGen &gen, unsigned offset);
  // Built-in offset...

  void emitRestoreFlagsFromStackSlot(codeGen &gen);

  bool emitBTSaves(baseTramp *bt, codeGen &gen);

  bool emitBTRestores(baseTramp *bt, codeGen &gen);

  // TODO: requires allocating / deallocating register (see next comment).
  void emitStoreImm(Address addr, int imm, codeGen &gen);

  // TODO: Implementation requires full 'codeGen' and 'registerSpace' class to
  // allocate register.
  void emitAddSignedImm(Address addr, int imm, codeGen &gen);

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

  // Long (64-bit) PC-relative jump via the s_getpc/s_setpc idiom. Takes TWO independent SGPR pairs
  // (the PC pair and the diff-constant pair) rather than one contiguous 4-block: each is independently
  // even-aligned, and 2+2 non-contiguous SGPRs can be found where 4 contiguous cannot -- and it lets
  // the caller place the pairs clear of registers reserved by the call path.
  void emitLongJump(Register pcPair, Register diffPair, uint64_t fromAddress, uint64_t toAddress,
                    codeGen &gen);

  void emitScalarDataCacheWriteback(codeGen &gen);

  void emitAtomicAdd(Register baseAddrReg, Register src0, codeGen &gen);

  void emitAtomicSub(Register baseAddrReg, Register src0, codeGen &gen);

  // ---- named snippet/trampoline sequences (gen-neutral; used by instrumentation) ----
  // Stream a 32-bit marker into the SQTT/ATT thread trace: `s_mov_b32 m0, <marker>; s_ttracedata`.
  // M0 must be dead across this sequence (it is clobbered).
  void emitTTraceData(uint32_t marker, codeGen &gen);
  // Stream a live register's value into the trace: `s_mov_b32 m0, <srcOperand>; s_ttracedata`.
  // srcOperand is a raw SGPR operand code (SOP1 SSRC0 field).
  void emitTTraceDataReg(Register srcOperand, codeGen &gen);
  // Save SCC into an SGPR: `s_cselect_b32 saveReg, 1, 0` (saveReg = SCC ? 1 : 0).
  // saveReg is a raw SGPR operand code.
  void emitSccSave(Register saveReg, codeGen &gen);
  // Restore SCC from a saved SGPR: `s_and_b32 saveReg, saveReg, 1` (sets SCC = saveReg & 1).
  // saveReg is a raw SGPR operand code.
  void emitSccRestore(Register saveReg, codeGen &gen);
  // Emit a relocation-fixed PC load: `s_getpc_b64 sdst; s_add_u32 sdst, sdst, lo;
  // s_addc_u32 sdst+1, sdst+1, hi`. sdst is a raw SGPR-pair base operand code; lo/hi are the
  // 64-bit correction split applied after layout. See PCWidget-amdgpu.
  void emitGetpcRelocated(Register sdst, uint32_t lo, uint32_t hi, codeGen &gen);

public:
  // ---- per-gen hooks: the ONLY things a concrete gen subclass supplies ----
  // These are also the public low-level emission surface for OPEN-CODED emitters that live outside the
  // class hierarchy (the kernel scratch-entry prologue, the inter-module call): such code holds an
  // EmitterAmdgpu* for the instrumented object and open-codes instructions via these packers + toOpCode,
  // so each gen gets its own encodings through the one runtime-selected emitter. Named emit* methods
  // (above) remain the convenience API for fixed sequences.
  // The gen's ENC-typed instruction packers (pack via that gen's <arch>::detail tables). Thin, sed-able.
  virtual void rawSop1(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0, bool hasLiteral,
                       uint32_t literal, codeGen &gen) = 0;
  virtual void rawSop2(unsigned opcode, Dyninst::Register dest, Dyninst::Register src1,
                       Dyninst::Register src0, codeGen &gen) = 0;
  virtual void rawSop2WithSrc1Literal(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0,
                                      uint32_t literal, codeGen &gen) = 0;
  virtual void rawSopK(unsigned opcode, Dyninst::Register dest, int16_t simm16, codeGen &gen) = 0;
  virtual void rawSopC(unsigned opcode, Dyninst::Register src0, Dyninst::Register src1, codeGen &gen) = 0;
  virtual void rawSopP(unsigned opcode, int16_t simm16, codeGen &gen) = 0;
  virtual void rawSmem(unsigned opcode, uint64_t sdata, uint64_t sbase, uint64_t offset, codeGen &gen) = 0;
  // Vector (VOP1/VOP2) and flat (global/scratch) packers -- the instruction classes instrumentation
  // snippets use beyond the scalar set (per-wave counters, address math, loads/stores).
  virtual void rawVop1Reg(unsigned opcode, uint32_t vdst, uint32_t src0, codeGen &gen) = 0;
  virtual void rawVop1Imm(unsigned opcode, uint32_t vdst, uint32_t literal, codeGen &gen) = 0;
  virtual void rawVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &gen) = 0;
  // Three-source VOP3a (e.g. V_LSHL_OR_B32, and the lane ops the call spill uses). src0/src1/src2 are
  // raw 9-bit VOP3 source operand codes; vdst is an 8-bit VGPR/SGPR dst code.
  virtual void rawVop3a(unsigned opcode, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2,
                        codeGen &gen) = 0;
  virtual void rawFlatGlobal(unsigned opcode, uint32_t vdst, uint32_t addr, uint32_t data,
                             uint32_t saddr, int32_t offset, codeGen &gen, bool glc, uint32_t seg) = 0;
  // Resolve a gen-neutral AmdgpuOp to THIS gen's ISA opcode value, from the gen's generated
  // amdgpu_<gen>::op::<FORMAT>::<MNEMONIC> constants. Per-gen for the same reason as the packers: each
  // subclass names only its own gen's namespace. The base never names a gen. (Named toOpCode, not opcode,
  // so it does not collide with the pervasive `unsigned opcode` parameters/locals in the emit* bodies.)
  virtual uint16_t toOpCode(AmdgpuOp op) const = 0;
  // Adapt a codegen Register to a MachRegister of THIS gen's arch. Per-gen because the category/size
  // constants (VGPR/SGPR, BITS_*) live in the gen's own register namespace, so each subclass names only
  // its own gen -- no macro, no arch switch. The base never names a gen.
  virtual Dyninst::MachRegister toMachRegister(Dyninst::Register r) const = 0;

public:
  // SGPR-operand validity checks. public so the open-coded gfx9-family call emitter (the shared
  // amdgpuEmitCall / amdgpuEmitIndirectCall free functions, delegated to by both the gfx908 and
  // gfx940 emitters) can assert on their operands.
  bool isValidSgpr(Register reg) const;
  bool isValidSgprBlock(Register regBlock) const;
  bool isValidSgprPair(Register regBlock) const;

  // Inter-module address resolution (GOT/relocation). public so the open-coded call emitter can
  // resolve an external callee's address through the runtime-selected emitter.
  Address getInterModuleFuncAddr(func_instance *func, codeGen& gen) /* override */;
  Address getInterModuleVarAddr(const image_variable *var, codeGen& gen) /* override */;
};

// The concrete per-gen emitters (EmitterAmdgpuGfx908, ...) subclass EmitterAmdgpu and live under
// amdgpu/<gen>/ (e.g. amdgpu/gfx908/emit-amdgpu-gfx908.h). Include that header where a gen is constructed.

// Resolve the AMDGPU emitter for the object a PatchAPI point belongs to: point back the instrumented
// object's architecture (gfx908/gfx940/...) by installing its emitter on `gen` via setAddrSpace, then
// hand back that EmitterAmdgpu. Shared by the AMDGPU prologue/epilogue snippets, which only need the
// gen-neutral base interface (emitLoadRelative, emitScalarDataCacheWriteback, ...).
EmitterAmdgpu* getAmdgpuEmitter(codeGen& gen, Dyninst::PatchAPI::Point* point);

// Same, for call sites that have no PatchAPI point but whose `gen` is already bound to the
// instrumented AMDGPU address space (e.g. relocation-widget apply, baseTramp emission): the per-gen
// emitter has already been installed on `gen` via setAddrSpace, so just recover it.
EmitterAmdgpu* getAmdgpuEmitter(codeGen& gen);

#endif
