#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emitter.h"
#include "dyninstAPI/src/registerSpace.h"

#include <iostream>
#include "unaligned_memory_access.h"

namespace {
#ifdef DYNINST_CODEGEN_ARCH_X86
  constexpr auto arch = Dyninst::Arch_x86;
#else
  constexpr auto arch = Dyninst::Arch_x86_64;
#endif
  auto *rs = registerSpace::getRegisterSpace(Dyninst::getArchAddressWidth(arch));

  constexpr auto buffer_size = 24;
}

template<size_t N>
using buffer = std::array<uint8_t, N>;

template <size_t N>
static bool check(codeGen &gen, std::array<uint8_t, N> const& expected) {
  auto *buf = static_cast<uint8_t*>(gen.start_ptr());
  bool const equal = std::equal(expected.begin(), expected.end(), buf);

  // For debugging. Remove once done.
  auto val = Dyninst::read_memory_as<uint64_t>(buf);
  std::printf("val = 0x%lx\n", val);
  std::cout << "equal: " << std::boolalpha << equal << '\n';

  // Reset the codegen's internal buffer so it can be reused
  gen.setIndex(0);
  std::memset(buf, 0, N);

  // Deallocate all registers
  rs->cleanSpace();

  return equal;
}

int main() {
  BinaryEdit bin_edit{};
  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);
  auto *emitter = gen.emitter();

  bool failed = false;

  // mov eax, 0x12345678
  emitter->emitLoadConst(REGNUM_EAX, Dyninst::Address{0x12345678}, gen);
  failed |= !check(gen, buffer<5>{{0xb8, 0x78, 0x56, 0x34, 0x12}});

  // imul eax, 3
  emitter->emitTimesImm(REGNUM_EAX, REGNUM_EAX, 0x03, gen);
  failed |= !check(gen, buffer<3>{{0x69, 0xc0, 0x03}});

  // pushf
  emitter->emitPushFlags(gen);
  failed |= !check(gen, buffer<1>{{0x9c}});

  // push eax
  emitter->emitPush(gen, REGNUM_EAX);
  failed |= !check(gen, buffer<1>{{0x50}});

  // pop eax
  emitter->emitPop(gen, REGNUM_EAX);
  failed |= !check(gen, buffer<1>{{0x58}});

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

//codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);
//void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
//void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
//void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s);
//void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen);
//void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
//void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s);
//void emitLoad(Register dest, Address addr, int size, codeGen &gen);
//void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen);
//bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen);
//bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen);
//void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset);
//void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen);
//void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen);
//void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store);
//void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen);
//void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen);
//void emitStore(Address addr, Register src, int size, codeGen &gen);
//void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen);
//void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen);
//void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen);
//void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen);
//Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee);
//int emitCallParams(codeGen &gen, const std::vector<AstNodePtr> &operands, func_instance *target, std::vector<Register> &extra_saves, bool noCost);
//bool emitCallCleanup(codeGen &gen, func_instance *target, int frame_size, std::vector<Register> &extra_saves);
//void emitGetRetVal(Register dest, bool addr_of, codeGen &gen);
//void emitGetRetAddr(Register dest, codeGen &gen);
//void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen);
//void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen);
//void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen);
//void emitRestoreFlags(codeGen &gen, unsigned offset);
//void emitRestoreFlagsFromStackSlot(codeGen &gen);
//void emitStackAlign(int offset, codeGen &gen);
//bool emitBTSaves(baseTramp *bt, codeGen &gen);
//bool emitBTRestores(baseTramp *bt, codeGen &gen);
//void emitLoadEffectiveAddress(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen);
//void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost);
//void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
//bool emitAdjustStackPointer(int index, codeGen &gen);
//bool emitMoveRegToReg(Register src, Register dest, codeGen &gen);
//void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen);
//bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen);
//bool emitXorRegReg(Register dest, Register base, codeGen &gen);
//bool emitXorRegImm(Register dest, int imm, codeGen &gen);
//bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen);
//bool emitPLTCall(func_instance *dest, codeGen &gen);
//bool emitPLTJump(func_instance *dest, codeGen &gen);
