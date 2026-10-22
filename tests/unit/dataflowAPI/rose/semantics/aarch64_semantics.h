#ifndef DYNINST_TESTS_UNIT_ROSE_SEMANTICS_AARCH64_H
#define DYNINST_TESTS_UNIT_ROSE_SEMANTICS_AARCH64_H

// Checks the ARM64 instruction semantics through SymEval: for each instruction, whether it
// writes a given register and, if it does, a fragment the written value must contain.

#include "Absloc.h"
#include "InstructionDecoder.h"
#include "SymEval.h"
#include "registers/aarch64_regs.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

namespace rose_semantics_aarch64 {

  struct test_case {
    char const *text;
    uint32_t encoding;
    Dyninst::MachRegister reg;
    bool written;
    char const *contains; // a fragment of the written value, or nullptr
  };

  // The expression SymEval gives for what the instruction writes to 'reg', or "" if it
  // writes nothing there.
  inline std::string written(Dyninst::InstructionAPI::Instruction const &insn,
                             Dyninst::MachRegister reg) {
    using namespace Dyninst;
    Assignment::Ptr a =
        Assignment::makeAssignment(insn, 0x1000, nullptr, nullptr, AbsRegion(Absloc(reg)));
    AST::Ptr e = DataflowAPI::SymEval::expand(a, false).first;
    return e ? e->format() : "";
  }

  // Runs every case; reports each failure and returns false if any failed.
  template <std::size_t N>
  bool check(std::array<test_case, N> const &cases) {
    using namespace Dyninst;
    bool ok = true;
    for(auto const &c : cases) {
      std::array<unsigned char, 4> bytes{};
      for(int i = 0; i < 4; ++i) {
        bytes[i] = static_cast<unsigned char>(c.encoding >> (8 * i));
      }
      InstructionAPI::InstructionDecoder dec(bytes.data(), bytes.size(), Arch_aarch64);
      InstructionAPI::Instruction insn = dec.decode();
      if(!insn.isValid()) {
        std::cerr << c.text << ": could not decode\n";
        ok = false;
        continue;
      }
      std::string const value = written(insn, c.reg);
      std::string const name = c.reg.name();
      if(value.empty() == c.written) {
        std::cerr << c.text << " (" << insn.format() << "): " << name
                  << (c.written ? " not written" : " written as " + value) << '\n';
        ok = false;
      } else if(c.written && c.contains && value.find(c.contains) == std::string::npos) {
        std::cerr << c.text << " (" << insn.format() << "): " << name << " = " << value
                  << ", expected it to contain " << c.contains << '\n';
        ok = false;
      }
    }
    return ok;
  }

}

#endif
