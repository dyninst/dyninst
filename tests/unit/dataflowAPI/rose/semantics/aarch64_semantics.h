#ifndef DYNINST_UNIT_TESTS_DATAFLOWAPI_ROSE_SEMANTICS_AARCH64_SEMANTICS_H
#define DYNINST_UNIT_TESTS_DATAFLOWAPI_ROSE_SEMANTICS_AARCH64_SEMANTICS_H

// Checks the ARM64 instruction semantics through SymEval: for each instruction, whether it
// writes a register or memory and, if it does, fragments its value must or must not contain.
//
// SymEval gives no value both when an instruction does not write a location and when the
// instruction has no semantics at all, and its success flag does not tell them apart. So a
// row that expects "not written" only means something next to a row for the same
// instruction that expects a write; every test keeps such a pair.

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

  inline Dyninst::AbsRegion reg(Dyninst::MachRegister r) {
    return Dyninst::AbsRegion(Dyninst::Absloc(r));
  }

  inline Dyninst::AbsRegion memory() {
    return Dyninst::AbsRegion(Dyninst::Absloc::Heap);
  }

  struct test_case {
    char const *text;
    uint32_t encoding;
    Dyninst::AbsRegion where;
    bool written;
    char const *contains;     // a fragment of the written value, or nullptr
    char const *not_contains; // a fragment the written value must not have, or nullptr
  };

  // The expression SymEval gives for what the instruction writes to 'where', or "" if it
  // gives none.
  inline std::string written(Dyninst::InstructionAPI::Instruction const &insn,
                             Dyninst::AbsRegion const &where) {
    using namespace Dyninst;
    Assignment::Ptr a = Assignment::makeAssignment(insn, 0x1000, nullptr, nullptr, where);
    AST::Ptr e = DataflowAPI::SymEval::expand(a, false).first;
    return e ? e->format() : "";
  }

  // Runs every case; reports each failure and returns false if any failed.
  template <std::size_t N>
  bool check(test_case const (&cases)[N]) {
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
      std::string const value = written(insn, c.where);
      std::string const name = c.where.format();
      if(value.empty() == c.written) {
        std::cerr << c.text << " (" << insn.format() << "): " << name
                  << (c.written ? " not written" : " written as " + value) << '\n';
        ok = false;
      } else if(c.written && c.contains && value.find(c.contains) == std::string::npos) {
        std::cerr << c.text << " (" << insn.format() << "): " << name << " = " << value
                  << ", expected it to contain " << c.contains << '\n';
        ok = false;
      } else if(c.written && c.not_contains &&
                value.find(c.not_contains) != std::string::npos) {
        std::cerr << c.text << " (" << insn.format() << "): " << name << " = " << value
                  << ", expected it not to contain " << c.not_contains << '\n';
        ok = false;
      }
    }
    return ok;
  }

}

#endif
