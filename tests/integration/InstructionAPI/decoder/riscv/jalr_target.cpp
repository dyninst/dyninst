// The successor target of riscv64 jalr is rs + imm.
#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/riscv64_regs.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

namespace {
  struct jalr_case {
    std::vector<unsigned char> bytes;
    char const *text;
    Dyninst::MachRegister rs;
    int64_t imm;
  };

  // clang-format off
  std::vector<jalr_case> const cases = {
    {{0xe7,0x8d,0x0f,0x01}, "jalr s11, t6, 0x10", Dyninst::riscv64::t6, 0x10},
    {{0xe7,0x80,0x87,0xff}, "jalr ra, a5, -0x8", Dyninst::riscv64::a5, -0x8},
    {{0x67,0x00,0x0e,0x7f}, "jalr zero, t3, 0x7f0", Dyninst::riscv64::t3, 0x7f0},
    {{0x82,0x87},           "c.jr a5", Dyninst::riscv64::a5, 0},
  };
  // clang-format on
}

int main() {
  bool ok = true;
  constexpr uint64_t rs_value = 0x1000;
  for(auto const &c : cases) {
    di::InstructionDecoder d(c.bytes.data(), c.bytes.size(), Dyninst::Arch_riscv64);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << c.text << ": could not decode\n";
      ok = false;
      continue;
    }
    auto target = insn.getControlFlowTarget();
    if(!target) {
      std::cerr << c.text << ": no control flow target\n";
      ok = false;
      continue;
    }
    di::RegisterAST rs(c.rs);
    target->bind(&rs, di::Result(di::s64, rs_value));
    auto const value = target->eval();
    uint64_t const expected = rs_value + static_cast<uint64_t>(c.imm);
    if(!value.defined || value.convert<uint64_t>() != expected) {
      std::cerr << c.text << ": target " << target->format() << " = "
                << (value.defined ? value.format() : "undefined") << ", expected 0x" << std::hex
                << expected << std::dec << '\n';
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
