// The value a riscv64 store writes to memory.
#include "Absloc.h"
#include "InstructionDecoder.h"
#include "SymEval.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {
  struct store_case {
    char const *text;
    uint32_t encoding;
    char const *expected; // the stored value, as AST::format() prints it
  };

  // The low 8 * width bits of rs2 (a5), read at 0x1000; extract's end bit is exclusive.
  // clang-format off
  store_case const cases[] = {
    {"sw a5, 8(a4)", 0x00f72423, "<extract:32>(<V([x15]:1000)>,<0:64>,<32:64>,)"},
    {"sd a5, 8(a4)", 0x00f73423, "<extract:64>(<V([x15]:1000)>,<0:64>,<64:64>,)"},
  };
  // clang-format on

  std::string stored(char const *text, uint32_t encoding, bool &ok) {
    using namespace Dyninst;
    std::array<unsigned char, 4> bytes{};
    for(int i = 0; i < 4; ++i) {
      bytes[i] = static_cast<unsigned char>(encoding >> (8 * i));
    }
    InstructionAPI::InstructionDecoder dec(bytes.data(), bytes.size(), Arch_riscv64);
    InstructionAPI::Instruction insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << text << ": could not decode\n";
      ok = false;
      return "";
    }
    Assignment::Ptr a =
        Assignment::makeAssignment(insn, 0x1000, nullptr, nullptr, AbsRegion(Absloc::Heap));
    AST::Ptr e = DataflowAPI::SymEval::expand(a, false).first;
    return e ? e->format() : "(not written)";
  }
}

int main() {
  bool ok = true;
  for(auto const &c : cases) {
    std::string const value = stored(c.text, c.encoding, ok);
    if(!value.empty() && value != c.expected) {
      std::cerr << c.text << ": memory = " << value << ", expected " << c.expected << "\n";
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
