#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/emit-amdgpu.h"
#include "registerSpace.h"
#include "emitter_test.h"

int main() {
  using Dyninst::verify_emitter;
  using Dyninst::emitter_buffer_t;
  using namespace NS_amdgpu;

  constexpr auto arch = Dyninst::Arch_amdgpu_gfx908;
  auto const size = Dyninst::getArchAddressWidth(arch);
  auto *rs = registerSpace::getRegisterSpace(size);

  BinaryEdit bin_edit{};

  constexpr auto buffer_size = 128;

  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);

  auto *emitter = static_cast<EmitterAmdgpuGfx908*>(gen.emitter());

  bool failed = false;

  // Move value from s0 to s1
  emitter->emitMoveRegToReg(RegisterConstants::s0, RegisterConstants::s1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x00,0x00,0x81,0xbe                      // s_mov_b32 s1, s0
  }});

  // Load 64-bit immediate into register pair s[0:1]
  // s0 contains lower 32 bits
  // s1 contains upper 32 bits
  Register regBlock0To1 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(0), Dyninst::BlockSize(2));
  emitter->emitLoadConst(regBlock0To1, (uint64_t)0x123456789, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<16>{{
    0xff,0x00,0x80,0xbe,0x89,0x67,0x45,0x23, // s_mov_b32 s0, 0x23456789
    0xff,00,0x81,0xbe,0x01,0x00,0x00,0x00    // s_mov_b32 s1, 0x1
  }});

  // Load a dword from s[2:3] into s4
  Register regBlock2To3 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(2), Dyninst::BlockSize(2));
  emitter->emitLoadIndir(RegisterConstants::s4, regBlock2To3, 1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x02,0xc0,0x00,0x00,0x00,0x00, // s_load_dword s4, s[2:3], 0x0
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Load a dword from s[2:3] + 1234 into s4
  emitter->emitLoadRelative(RegisterConstants::s4, 0x1234, regBlock2To3, 1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x02,0xc0,0x34,0x12,0x00,0x00, // s_load_dword s4, s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Load 2 dwords from s[2:3] + 0x1234 into s[4:5]
  Register regBlock4To5 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(4), Dyninst::BlockSize(2));
  emitter->emitLoadRelative(regBlock4To5, 0x1234, regBlock2To3, 2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x06,0xc0,0x34,0x12,0x00,0x00, // s_load_dwordx2 s[4:5], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Load 4 dwords from s[2:3] + 0x1234 into s[4:7]
  Register regBlock4To7 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(4), Dyninst::BlockSize(4));
  emitter->emitLoadRelative(regBlock4To7, 0x1234, regBlock2To3, 4, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x0a,0xc0,0x34,0x12,0x00,0x00, // s_load_dwordx4 s[4:7], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Load 8 dwords from s[2:3] + 0x1234 into s[4:11]
  Register regBlock4To11 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(4), Dyninst::BlockSize(8));
  emitter->emitLoadRelative(regBlock4To11, 0x1234, regBlock2To3, 8, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x0e,0xc0,0x34,0x12,0x00,0x00, // s_load_dwordx8 s[4:11], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Load 16 dwords from s[2:3] + 0x1234 into s[4:19]
  Register regBlock4To19 = Dyninst::Register::makeScalarRegister(Dyninst::OperandRegId(4), Dyninst::BlockSize(16));
  emitter->emitLoadRelative(regBlock4To19, 0x1234, regBlock2To3, 16, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x12,0xc0,0x34,0x12,0x00,0x00, // s_load_dwordx16 s[4:19], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Store a dword from s4 to s[2:3]
  emitter->emitStoreIndir(regBlock2To3, RegisterConstants::s4, 1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x42,0xc0,0x00,0x00,0x00,0x00, // s_store_dword s4, s[2:3], 0x0
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Store a dword from s4 to s[2:3] + 1234
  emitter->emitStoreRelative(RegisterConstants::s4, 0x1234, regBlock2To3, 1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x42,0xc0,0x34,0x12,0x00,0x00, // s_store_dword s4, s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Store 2 dwords from s[4:5] to s[2:3] + 0x1234
  emitter->emitStoreRelative(regBlock4To5, 0x1234, regBlock2To3, 2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x46,0xc0,0x34,0x12,0x00,0x00, // s_store_dwordx2 s[4:5], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // Store 4 dwords from s[4:7] to s[2:3] + 0x1234
  emitter->emitStoreRelative(regBlock4To7, 0x1234, regBlock2To3, 4, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<12>{{
    0x01,0x01,0x4a,0xc0,0x34,0x12,0x00,0x00, // s_store_dwordx4 s[4:7], s[2:3], 0x1234
    0x00,0x00,0x8c,0xbf                      // s_waitcnt vmcnt(0) expcnt(0) lgkmcnt(0)
  }});

  // scc = s1 < s2
  emitter->emitRelOp(lessOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x04,0xbf                      // s_cmp_lt_i32 s1, s2
  }});

  // scc = s1 <= s2
  emitter->emitRelOp(leOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x05,0xbf                      // s_cmp_le_i32 s1, s2
  }});

  // scc = s1 > 2
  emitter->emitRelOp(greaterOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x02,0xbf                      // s_cmp_gt_i32 s1, s2
  }});

  // scc = s1 >= s2
  emitter->emitRelOp(geOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,03,0xbf                        // s_cmp_ge_i32 s1, s2
  }});

  // scc = s1 == s2
  emitter->emitRelOp(eqOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x00,0xbf                      // s_cmp_eq_i32 s1, s2
  }});

  // scc = s1 != s2
  emitter->emitRelOp(neOp, /*SCC_DUMMY=*/0, RegisterConstants::s1, RegisterConstants::s2, gen, 0);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x01,0xbf                      // s_cmp_lg_i32 s1, s2
  }});

  // s3 = s1 + s2
  emitter->emitOp(plusOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x03,0x81                      // s_add_i32 s3, s1, s2
  }});

  // s3 = s1 - s2
  emitter->emitOp(minusOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x83,0x81                      // s_sub_i32 s3, s1, s2
  }});

  // s3 = s1 * s2
  emitter->emitOp(timesOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x03,0x92                      // s_mul_i32 s3, s1, s2
  }});

  // s3 = s1 | s2
  emitter->emitOp(orOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x03,0x87                      // s_or_b32 s3, s1, s2
  }});

  // s3 = s1 & s2
  emitter->emitOp(andOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x03,0x86                      // s_and_b32 s3, s1, s2
  }});

  // s3 = s1 ^ s2
  emitter->emitOp(xorOp, RegisterConstants::s3, RegisterConstants::s1, RegisterConstants::s2, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x01,0x02,0x03,0x88                      // s_xor_b32 s3, s1, s2
  }});

  // emit 1 no op
  emitter->emitNops(1, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x00,0x00,0x80,0xbf                      // s_nop 0
  }});

  // emit 5 no ops
  emitter->emitNops(5, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x04,0x00,0x80,0xbf                      // s_nop 4
  }});

  // We add a 32-bit constant to a 64-bit value
  // s[2:3] = s[2:3] + 0x1234
  //
  // s2 contains lower 32 bits. The addition is broken into two operations as follows:
  // s2 = s2 + 0x1234, scc = carry
  // s3 = s3 + 0 + scc (add with carry)
  emitter->emitAddConstantToRegPair(regBlock2To3, 0x1234, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<16>{{
    0x02,0xff,0x02,0x80,0x34,0x12,0x00,0x00, // s_add_u32 s2, s2, 0x1234
    0x03,0xff,0x03,0x82,0x00,0x00,0x00,0x00  // s_addc_u32 s3, s3, 0
  }});

  emitter->emitEndProgram(gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<4>{{
    0x00,0x00,0x81,0xbf                      // s_endpgm
  }});

  // s0 = 0x1234
  emitter->emitMovLiteral(RegisterConstants::s0, 0x1234, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0xff,0x00,0x80,0xbe,0x34,0x12,0x00,0x00  // s_mov_b32 s0, 0x1234
  }});

  emitter->emitScalarDataCacheWriteback(gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x00,0x00,0x84,0xc0,0x00,0x00,0x00,0x00  // s_dcache_wb
  }});

  // Atomically add s0 to the 4-byte variable at address held in s[2:3]
  emitter->emitAtomicAdd(regBlock2To3, RegisterConstants::s0, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x01,0x00,0x0a,0xc2,0x00,0x00,0x00,0x00  // s_atomic_add s0, s[2:3], 0x0
  }});

  // Atomically subtract s0 from the 4-byte variable at address held in s[2:3]
  emitter->emitAtomicSub(regBlock2To3, RegisterConstants::s0, gen);
  failed |= !verify_emitter(gen, emitter_buffer_t<8>{{
    0x01,0x00,0x0e,0xc2,0x00,0x00,0x00,0x00  // s_atomic_sub s8, s[2:3], 0x0
  }});

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*

These require a valid baseTramp (codeGen::bt())

  bool emitBTRestores(baseTramp* bt, codeGen &gen);
  bool emitBTSaves(baseTramp* bt, codeGen &gen);
  void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
  void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
  void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);

These aren't tested.
  void emitConditionalBranch(bool onConditionTrue, int16_t wordOffset, codeGen &gen);
  void emitShortJump(int16_t wordOffset, codeGen &gen);
  void emitLongJump(Register reg, uint64_t fromAddress, uint64_t toAddress, codeGen &gen);
  codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);

These need to expand over many 'opcode' values

  void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm, bool is_64, codeGen &gen);
  void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);

These are not implemented
  bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
  Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee);
  bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret);
  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
  bool emitPLTCall(func_instance *dest, codeGen &gen);
  bool emitPLTJump(func_instance *dest, codeGen &gen);
  void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen);
  void emitLoad(Register dest, Address addr, int size, codeGen &gen);
  void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);
  bool emitCallRelative(Register, Address, Register, codeGen &) {assert (0); return false; }
  void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset);
  void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
  void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
  void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);
  void emitStore(Address addr, Register src, int size, codeGen &gen);
  void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
  void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);
  void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen);
  bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen);
  Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee);
  void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);
  void emitGetRetAddr(Register dest, codeGen &gen);
  void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
  void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);
  void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);
  void emitPushFlags(codeGen &gen);
  void emitRestoreFlags(codeGen &gen, unsigned offset);
  void emitRestoreFlagsFromStackSlot(codeGen &gen);
  bool emitBTSaves(baseTramp *bt, codeGen &gen);
  bool emitBTRestores(baseTramp *bt, codeGen &gen);
  void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);
  void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
  bool emitPush(codeGen &, Register);
  bool emitPop(codeGen &, Register);
  bool emitAdjustStackPointer(int index, codeGen &gen);
  bool clobberAllFuncCall(registerSpace *rs, func_instance *callee);
*/
