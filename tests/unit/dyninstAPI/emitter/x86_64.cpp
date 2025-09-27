#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emitter.h"
#include "dyninstAPI/src/registerSpace.h"

#include <iostream>
#include "unaligned_memory_access.h"

namespace {
  constexpr auto arch = Dyninst::Arch_x86_64;

  auto *rs = registerSpace::getRegisterSpace(Dyninst::getArchAddressWidth(arch));
}

template<size_t N>
using buffer = std::array<uint8_t, N>;

template <size_t N>
static bool check(codeGen &gen, std::array<uint8_t, N> const& expected) {
  auto *buf = static_cast<uint8_t*>(gen.start_ptr());

  bool const equal = [&]() {
    if(gen.used() != N) {
      return false;
    }
    return std::equal(expected.begin(), expected.end(), buf);
  }();

  std::cerr << "Encoded " << std::dec << gen.used() << " bytes 0x";
  for(auto i=0U; i<gen.used(); i++) {
    std::cerr << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint32_t>(buf[i]);
  }

  if(!equal) {
    std::cerr << ", FAILED: expected " << N << " bytes 0x";
    for(auto b : expected) {
      std::cerr << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint32_t>(b);
    }
    std::cerr << "\n";
  } else {
    std::cerr << ", OK\n";
  }

  // Reset the codegen's internal buffer so it can be reused
  gen.setIndex(0);
  std::memset(buf, 0, N);

  // Deallocate all registers
  rs->cleanSpace();

  return equal;
}

int main() {
  BinaryEdit bin_edit{};
  
  constexpr auto buffer_size = 128;
  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);

  auto *emitter = gen.emitter();

  bool failed = false;

  // mov rax, 0x12345678
  emitter->emitLoadConst(REGNUM_EAX, Dyninst::Address{0x12345678}, gen);
  failed |= !check(gen, buffer<10>{{0x48, 0xb8, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0}});

  // imul rax, rax, 3
  emitter->emitTimesImm(REGNUM_EAX, REGNUM_EAX, 0x03, gen);
  failed |= !check(gen, buffer<7>{{0x48, 0x69, 0xc0, 0x03, 0x0, 0x0, 0x0}});

  /*  Divide rdx by rcx
   *    mov rax, rdx
   *    cqo  # sign-extend rax
   *    div rcx
   */
  emitter->emitDiv(REGNUM_EAX, REGNUM_EDX, REGNUM_ECX, gen, false);
  failed |= !check(gen, buffer<8>{{
    0x48, 0x8b, 0xc2, 0x48, 0x99, 0x48, 0xf7, 0xf1
  }});

  /* Divide rsi by 0x12345678, store results in rdx
   *    mov rax, rsi
   *    mov rdx, 0x0
   *    push 0x12345678
   *    div qword ptr [rsp]
   *    mov rdx, rax
   *    add rsp, 8
   */
  emitter->emitDivImm(REGNUM_EDX, REGNUM_ESI, 0x12345678, gen, false);
  failed |= !check(gen, buffer<29>{{
    0x48, 0x8b, 0xc6, 
    0x48, 0xba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x68, 0x78, 0x56, 0x34, 0x12,
    0x48, 0xf7, 0x34, 0x24, 
    0x48, 0x8b, 0xd0, 
    0x48, 0x83, 0xc4, 0x08
    }});
    
  // pushfq
  emitter->emitPushFlags(gen);
  failed |= !check(gen, buffer<1>{{0x9c}});

  // push rax
  emitter->emitPush(gen, REGNUM_EAX);
  failed |= !check(gen, buffer<1>{{0x50}});

  // pop rax
  emitter->emitPop(gen, REGNUM_EAX);
  failed |= !check(gen, buffer<1>{{0x58}});

  // mov rax, rbp
  emitter->emitMoveRegToReg(REGNUM_EBP, REGNUM_EAX, gen);
  failed |= !check(gen, buffer<3>{{0x48, 0x8b, 0xc5}});

  /* Load qword at 0x12345678 into rax
   *    mov tmp, 0x12345678
   *    mov rax, qword ptr [tmp]
   * 
   * 'tmp' is assumed to be R15 here. It seems to be the
   * first scratch register used.
   */
  emitter->emitLoad(REGNUM_EAX, 0X12345678, 8, gen);
  failed |= !check(gen, buffer<13>{{
    0x49, 0xbf, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0,
    0x49, 0x8b, 0x07
  }});

  /* Store literal 0x9abcdef at memory address 0x12345678
   *    mov dword ptr [0x12345678], 0x9abcdef
   */
  emitter->emitStoreImm(0x12345678, 0x9abcdef, gen, false);
  failed |= !check(gen, buffer<11>{{
    0xc7, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab, 0x09  
  }});
  
  /* Pop eflags from stack
   *    popfq
   */
  emitter->emitRestoreFlags(gen, 0x0);
  failed |= !check(gen, buffer<1>{{0x9d}});
  
  /* Restore flags that are located at 'rbp + (0x4 * 8)`
   *    push	qword ptr [rbp + 0x20]
   *    popfq
   */
  emitter->emitRestoreFlags(gen, 0x4);
  failed |= !check(gen, buffer<4>{{
    0xff, 0x75, 0x20,
    0x9d
  }});
  
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

//codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen);
//void emitOp(unsigned opcode, Register dest, Register src1, Register src2, codeGen &gen);
//void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s);
//void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen);
//void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s);
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
//void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost);
//bool emitAdjustStackPointer(int index, codeGen &gen);
//void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen);
//bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen);
//bool emitXorRegReg(Register dest, Register base, codeGen &gen);
//bool emitXorRegImm(Register dest, int imm, codeGen &gen);
//bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen);
//bool emitPLTCall(func_instance *dest, codeGen &gen);
//bool emitPLTJump(func_instance *dest, codeGen &gen);
