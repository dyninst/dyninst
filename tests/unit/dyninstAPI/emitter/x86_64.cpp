#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include "Architecture.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/emit-x86.h"
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

  auto *emitter = static_cast<EmitterAMD64Stat*>(gen.emitter());

  bool failed = false;

  // Move value from rbp to rax
  emitter->emitMoveRegToReg(REGNUM_EBP, REGNUM_EAX, gen);
  failed |= !check(gen, buffer<3>{{
    0x48, 0x8b, 0xc5  // mov rax, rbp
  }});

  // Load immediate into rax (of size 64 bits; "movabs")
  emitter->emitLoadConst(REGNUM_EAX, 0x12345678, gen);
  failed |= !check(gen, buffer<10>{{
    0x48, 0xb8, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0  // mov rax, 0x12345678
  }});

  // Store an immediate value at a fixed address
  emitter->emitStoreImm(0x12345678, 0x9abcdef, gen, false);
  failed |= !check(gen, buffer<11>{{
    // mov dword ptr [0x12345678], 0x9abcdef
    0xc7, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0xef, 0xcd, 0xab, 0x09
  }});

  // Load value from fixed address into rax (tmp is assumed to be r15)
  emitter->emitLoad(REGNUM_EAX, 0x12345678, 8, gen);
  failed |= !check(gen, buffer<13>{{
    0x49, 0xbf, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0, // mov tmp, 0x12345678
    0x49, 0x8b, 0x07                                        // mov rax, qword ptr [tmp]
  }});

  // Store at a fixed address the value contained in rax (tmp is assumed to be r15)
  emitter->emitStore(0x12345678, REGNUM_EAX, 0x8, gen);
  failed |= !check(gen, buffer<13>{{
    0x49, 0xbf, 0x78, 0x56, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0,   // mov tmp, 0x12345678
    0x49, 0x89, 0x07                                          // mov qword ptr [tmp], rax
  }});

  // Load value into rax using register-indirect addressing w/o displacement
  emitter->emitLoadIndir(REGNUM_EAX, REGNUM_ESI, 0x4, gen);
  failed |= !check(gen, buffer<2>{{
    0x8b, 0x06    // mov eax, dword ptr [rsi]
  }});

  // Store value contained in rax using register-indirect addressing w/o displacement
  emitter->emitStoreIndir(REGNUM_EBP, REGNUM_EAX, 0x8, gen);
  failed |= !check(gen, buffer<4>{{
    0x48, 0x89, 0x45, 0x0   // mov qword ptr [rbp], rax
  }});

  // Load value into rax using register-indirect addressing with displacement
  emitter->emitLoadRelative(REGNUM_EAX, 0x12345678, REGNUM_EBP, 0x8, gen);
  failed |= !check(gen, buffer<7>{{
    0x48, 0x8b, 0x85, 0x78, 0x56, 0x34, 0x12   // mov rax, qword ptr [rbp+0x12345678]
  }});

  // Store value contained in rax using register-indirect addressing with displacement
  emitter->emitStoreRelative(REGNUM_EAX, 0x12345678, REGNUM_EBP, 0x8, gen);
  failed |= !check(gen, buffer<7>{{
    0x48, 0x89, 0x85, 0x78, 0x56, 0x34, 0x12   // mov qword ptr [rbp+0x12345678], rax
  }});

  // Load value into rax using direct addressing with with segment override
  emitter->emitLoadRelativeSegReg(REGNUM_EAX, 0x12345678, REGNUM_GS, 0x8, gen);
  failed |= !check(gen, buffer<9>{{
    0x65, 0x49, 0x8b, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12   // mov rax, qword ptr gs:[0x12345678]
  }});

  // Multiply rax by e
  emitter->emitTimesImm(REGNUM_EAX, REGNUM_EAX, 0x03, gen);
  failed |= !check(gen, buffer<7>{{
    0x48, 0x69, 0xc0, 0x03, 0x0, 0x0, 0x0   // imul rax, rax, 3
  }});

  //  Divide rdx by rcx
  emitter->emitDiv(REGNUM_EAX, REGNUM_EDX, REGNUM_ECX, gen, false);
  failed |= !check(gen, buffer<8>{{
    0x48, 0x8b, 0xc2,   // mov rax, rdx
    0x48, 0x99,         // cqo  # sign-extend rax
    0x48, 0xf7, 0xf1    // div rcx
  }});

  // Divide rsi by 0x12345678, store results in rdx
  emitter->emitDivImm(REGNUM_EDX, REGNUM_ESI, 0x12345678, gen, false);
  failed |= !check(gen, buffer<29>{{
    0x48, 0x8b, 0xc6,                                     // mov rax, rsi
    0x48, 0xba, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,   // mov rdx, 0x0
    0x68, 0x78, 0x56, 0x34, 0x12,                         // push 0x12345678
    0x48, 0xf7, 0x34, 0x24,                               // div qword ptr [rsp]
    0x48, 0x8b, 0xd0,                                     // mov rdx, rax
    0x48, 0x83, 0xc4, 0x08                                // add rsp, 8
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

  // Restore flags that are located at 'rbp + (4 * 8)`
  emitter->emitRestoreFlags(gen, 0x4);
  failed |= !check(gen, buffer<4>{{
    0xff, 0x75, 0x20, // push	qword ptr [rbp + 0x20]
    0x9d              // popfq
  }});

  // Pop eflags from stack (popqf)
  emitter->emitRestoreFlags(gen, 0x0);
  failed |= !check(gen, buffer<1>{{0x9d}});

  //  Move stack pointer by 0x20 and align it to AMD64_STACK_ALIGNMENT
  emitter->emitStackAlign(0x20, gen);
  failed |= !check(gen, buffer<48>{{
    0x48, 0x8d, 0x64, 0x24, 0xb0,       // lea	rsp, [rsp - 0x50]
    0x48, 0x89, 0x44, 0x24, 0x20,       // mov	qword ptr [rsp + 0x20], rax
    0x9f,                               // lahf
    0x0f, 0x90, 0xc0,                   // seto	al
    0x48, 0x89, 0x44, 0x24, 0x28,       // mov	qword ptr [rsp + 0x28], rax
    0x48, 0x8d, 0x44, 0x24, 0x50,       // lea	rax, [rsp + 0x50]
    0x48, 0x83, 0xe4, 0xe0,             // and	rsp, 0xffffffffffffffe0
    0x48, 0x89, 0x04, 0x24,             // mov	qword ptr [rsp], rax
    0x48, 0x8b, 0x40, 0xd8,             // mov	rax, qword ptr [rax - 0x28]
    0x80, 0xc0, 0x7f,                   // add	al, 0x7f
    0x9e,                               // sahf
    0x48, 0x8b, 0x04, 0x24,             // mov	rax, qword ptr [rsp]
    0x48, 0x8b, 0x40, 0xd0,             // mov	rax, qword ptr [rax - 0x30]
  }});

  // Restore rflags from stack
  emitter->emitRestoreFlagsFromStackSlot(gen);
  failed |= !check(gen, buffer<4>{{
    0xff, 0x75, 0x0, 0x9d   // push qword ptr [rbp]
  }});

  // Restore rflags from stack at offset 0x1234
  emitter->emitRestoreFlags(gen, 0x1234);
  failed |= !check(gen, buffer<7>{{
    0xff, 0xb5, 0xa0, 0x91, 0x0, 0x0, 0x9d  // push qword ptr [rbp+0x91A0]
  }});

  //  lea rax, qword ptr [rsi+rbx*4+0x12345678]
  emitter->emitLEA(REGNUM_ESI, REGNUM_EBX, 0X2, 0X12345678, REGNUM_EAX, gen);
  failed |= !check(gen, buffer<8>{{0x48, 0x8d, 0x84, 0x9e, 0x78, 0x56, 0x34, 0x12}});

  // xor rax, rcx
  emitter->emitXorRegReg(REGNUM_EAX, REGNUM_ECX, gen);
  failed |= !check(gen, buffer<3>{{0x48, 0x33, 0xc1}});

  // xor rax, qword ptr [rcx+0x12]
  emitter->emitXorRegRM(REGNUM_EAX, REGNUM_ECX, 0x12, gen);
  failed |= !check(gen, buffer<4>{{0x48, 0x33, 0x41, 0x12}});

  // xor eax, 0x12345678   (Why no REX?)
  emitter->emitXorRegImm(REGNUM_EAX, 0x12345678, gen);
  failed |= !check(gen, buffer<6>{{0x81, 0xf0, 0x78, 0x56, 0x34, 0x12}});

  // xor rax, qword ptr gs:[0x0000000012345678]
  emitter->emitXorRegSegReg(REGNUM_EAX, REGNUM_GS, 0x12345678, gen);
  failed |= !check(gen, buffer<9>{{0x65, 0x49, 0x33, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12}});

  // add rsp, 0x91A0
  emitter->emitAdjustStackPointer(0x1234, gen);
  failed |= !check(gen, buffer<7>{{0x48, 0x81, 0xc4, 0xa0, 0x91, 0x0, 0x0}});

  // add qword ptr [0x12345678], 0x04
  emitter->emitAddSignedImm(0x12345678, 0x4, gen, false);
  failed |= !check(gen, buffer<9>{{0x48, 0x83, 0x04, 0x25, 0x78, 0x56, 0x34, 0x12, 0x04}});

  // Load BPatch_countSpec_NP (I don't actually know what this is supposed to do)
  emitter->emitCSload(-1, -1, 4, 0x1234, REGNUM_EAX, gen);
  failed |= !check(gen, buffer<10>{{
    0x48, 0xb8, 0x34, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0   // mov rax, 0x1234
  }});

  // Load BPatch_addrSpec_NP (I don't actually know what this is supposed to do)
  emitter->emitASload(-1, -1, 4, 0x1234, REGNUM_EAX, 0x0, gen);
  failed |= !check(gen, buffer<5>{{
    0xb8, 0x34, 0x12, 0x0, 0x0   // mov eax, 0x1234
  }});

  // Load return value into ABI return register (rax)
  emitter->emitGetRetVal(REGNUM_ESI, false, gen);
  failed |= !check(gen, buffer<3>{{
    0x48, 0x8b, 0xf0    // mov rsi, rax
  }});

  // Load return address from stack into rsi
  emitter->emitGetRetAddr(REGNUM_ESI, gen);
  failed |= !check(gen, buffer<4>{{
    0x48, 0x8d, 0x75, 0x00    // lea rsi, qword ptr [rbp]
  }});

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
