#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  x86/x86_64 register read/write set tests
 *
 *  These verify the register read/write sets produced by the decoder
 *  for a mix of arithmetic, stack, control-flow, mov, SSE, and lea
 *  instructions, in both 32- and 64-bit modes.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

bool run(Dyninst::Architecture arch) {
  constexpr auto num_tests = 11;

  // clang-format off
  constexpr std::array<const uint8_t, 40> buffer = {{
      0x05, 0xef, 0xbe, 0xad, 0xde,             // add eax, 0xdeadbeef
      0x50,                                     // push eax
      0x74, 0x10,                               // jz +0x10(8)
      0xe8, 0x20, 0x00, 0x00, 0x00,             // call +0x20(32)
      0xf8,                                     // clc
      0x04, 0x30,                               // add al, 0x30(8)
      0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00, // movl 0x01, -0x4(ebp)
      0x88, 0x55, 0xcc,                         // movb dl, -0x34(ebp)
      0xf2, 0x0f, 0x12, 0xc0,                   // movddup xmm0, xmm0
      0x66, 0x0f, 0x7c, 0xc9,                   // haddpd xmm1, xmm1
      0x8d, 0x83, 0x18, 0xff, 0xff, 0xff        // lea -0xe8(%ebx), %eax
  }};
  // clang-format on

  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests in " << sarch << " mode\n";

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  di::InstructionDecoder d(buffer.data(), buffer.size(), arch);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << (idx + 1) << '\n';
      return false;
    }
    decodedInsns.push_back(insn);
  }

  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto eax = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;
  auto ebx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto al = is_64 ? Dyninst::x86_64::al : Dyninst::x86::al;
  auto bp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
  auto dl = is_64 ? Dyninst::x86_64::dl : Dyninst::x86::dl;
  auto xmm0 = is_64 ? Dyninst::x86_64::xmm0 : Dyninst::x86::xmm0;
  auto xmm1 = is_64 ? Dyninst::x86_64::xmm1 : Dyninst::x86::xmm1;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  auto rax = Dyninst::x86_64::rax;

  using reg_set = Dyninst::register_set;

  std::vector<reg_set> expectedRead, expectedWritten;

  // add eax, 0xdeadbeef
  expectedRead.push_back(reg_set{eax});
  expectedWritten.push_back(reg_set{eax, af, zf, of, pf, sf, cf});

  // push eax
  expectedRead.push_back(reg_set{sp, (is_64 ? rax : eax)});
  expectedWritten.push_back(reg_set{sp});

  // jz +0x10(8)
  expectedRead.push_back(reg_set{zf, sf, cf, pf, of, ip});
  expectedWritten.push_back(reg_set{ip});

  // call +0x20(32)
  expectedRead.push_back(reg_set{sp, ip});
  expectedWritten.push_back(reg_set{sp, ip});

  // clc
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{cf});

  // add al, 0x30(8)
  expectedRead.push_back(reg_set{al});
  expectedWritten.push_back(reg_set{al, zf, cf, sf, of, pf, af});

  // movl 0x01, -0x4(ebp)
  expectedRead.push_back(reg_set{bp});
  expectedWritten.push_back(reg_set{});

  // movb dl, -0x34(ebp)
  expectedRead.push_back(reg_set{bp, dl});
  expectedWritten.push_back(reg_set{});

  // movddup xmm0, xmm0
  expectedRead.push_back(reg_set{xmm0});
  expectedWritten.push_back(reg_set{xmm0});

  // haddpd xmm1, xmm1
  expectedRead.push_back(reg_set{xmm1});
  expectedWritten.push_back(reg_set{xmm1});

  // lea -0xe8(%ebx), %eax
  expectedRead.push_back(reg_set{ebx});
  expectedWritten.push_back(reg_set{eax});

  if(expectedRead.size() != expectedWritten.size()) {
    std::cerr << "FATAL: expectedRead.size() != expectedWritten.size()\n";
    return false;
  }

  if(expectedRead.size() != decodedInsns.size()) {
    std::cerr << "FATAL: expectedRead.size() != decodedInsns.size()\n";
    return false;
  }

  bool failed = false;
  for(size_t i = 0; i < decodedInsns.size(); i++) {
    auto const &insn = decodedInsns[i];

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, di::register_rw_test{expectedRead[i], expectedWritten[i]})) {
      failed = true;
    }
  }

  return !failed;
}

bool run64_only() {
  constexpr auto arch = Dyninst::Arch_x86_64;
  constexpr auto num_tests = 1;

  // clang-format off
  constexpr std::array<const uint8_t, 4> buffer = {{
      0x44, 0x89, 0x45, 0xc4 // mov dword ptr [rbp - 0x3c], r8d
  }};
  // clang-format on

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  di::InstructionDecoder d(buffer.data(), buffer.size(), arch);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode x86_64 test " << (idx + 1) << '\n';
      return false;
    }
    decodedInsns.push_back(insn);
  }

  auto r8d = Dyninst::x86_64::r8d;
  auto rbp = Dyninst::x86_64::rbp;

  using reg_set = Dyninst::register_set;

  std::vector<reg_set> expectedRead, expectedWritten;

  // mov dword ptr [rbp - 0x3c], r8d
  expectedRead.push_back(reg_set{rbp, r8d});
  expectedWritten.push_back(reg_set{});

  bool failed = false;
  for(size_t i = 0; i < decodedInsns.size(); i++) {
    auto const &insn = decodedInsns[i];

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, di::register_rw_test{expectedRead[i], expectedWritten[i]})) {
      failed = true;
    }
  }

  return !failed;
}

} // namespace

int main() {
  bool ok = run(Dyninst::Arch_x86);

  if(!run(Dyninst::Arch_x86_64)) {
    ok = false;
  }
  if(!run64_only()) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}
