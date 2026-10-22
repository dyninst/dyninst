// Checks the ARM64 instruction semantics through SymEval: for each instruction, whether it
// writes a given register and, if it does, a fragment the written value must contain.
#include "Absloc.h"
#include "InstructionDecoder.h"
#include "SymEval.h"
#include "registers/aarch64_regs.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace Dyninst;

namespace {

  struct test_case {
    char const *text;
    uint32_t encoding;
    MachRegister reg;
    bool written;
    char const *contains; // a fragment of the written value, or nullptr
  };

  // The expression SymEval gives for what the instruction writes to 'reg', or "" if it
  // writes nothing there.
  std::string written(InstructionAPI::Instruction const &insn, MachRegister reg) {
    Assignment::Ptr a =
        Assignment::makeAssignment(insn, 0x1000, nullptr, nullptr, AbsRegion(Absloc(reg)));
    AST::Ptr e = DataflowAPI::SymEval::expand(a, false).first;
    return e ? e->format() : "";
  }

}

int main() {
  namespace a64 = Dyninst::aarch64;

  // clang-format off
  std::array<test_case, 27> const cases = {{
    // Only the pre- and post-index forms write the address back to the base register;
    // the unscaled (LDUR/STUR) and unprivileged (LDTR/STTR) forms do not.
    {"ldur  x0, [x1, #8]",   0xF8408020, a64::x1, false, nullptr},
    {"ldurb w0, [x1, #8]",   0x38408020, a64::x1, false, nullptr},
    {"stur  x0, [x1, #8]",   0xF8008020, a64::x1, false, nullptr},
    {"sturb w0, [x1, #8]",   0x38008020, a64::x1, false, nullptr},
    {"ldtr  x0, [x1, #8]",   0xF8408820, a64::x1, false, nullptr},
    {"sttr  x0, [x1, #8]",   0xF8008820, a64::x1, false, nullptr},
    {"ldr   x0, [x1], #8",   0xF8408420, a64::x1, true,  "<add>"},
    {"ldr   x0, [x1, #8]!",  0xF8408C20, a64::x1, true,  "<add>"},

    // Loads write their destination and stores do not, whatever bits 23:22 hold.
    {"ldrsb w0, [x1]",       0x39C00020, a64::w0, true,  "deref:8"},
    {"ldrsh w0, [x1]",       0x79C00020, a64::w0, true,  "deref:16"},
    {"ldr   w0, pc+8",       0x18000040, a64::w0, true,  "deref:32"},
    {"ldr   x0, pc+24",      0x580000C0, a64::x0, true,  "deref:64"},
    {"ldrsw x0, pc+8",       0x98000040, a64::x0, true,  "signExtend"},
    {"ldar  x0, [x1]",       0xC8DFFC20, a64::x0, true,  "deref:64"},
    {"ldar  w0, [x1]",       0x88DFFC20, a64::w0, true,  "deref:32"},
    {"stlr  x0, [x1]",       0xC89FFC20, a64::x0, false, nullptr},

    // Pairs: both registers, 64 bits each, the second 8 bytes on; the base is written
    // back only by the pre- and post-index forms.
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::x29, true,  "deref:64"},
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::x30, true,  "<8:"},
    {"ldp   x29, x30, [sp], #16",   0xA8C17BFD, a64::sp,  true,  "<16:"},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, a64::x29, false, nullptr},
    {"stp   x29, x30, [sp, #-16]!", 0xA9BF7BFD, a64::sp,  true,  "<add>"},
    {"ldp   x0, x1, [x2, #16]",     0xA9410440, a64::x2,  false, nullptr},
    {"ldpsw x0, x1, [x2]",          0x69400440, a64::x1,  true,  "signExtend"},
    {"ldnp  x0, x1, [x2, #16]",     0xA9410440 & ~0x01800000u, a64::x2, false, nullptr},

    // BR does not link; BLR does.
    {"br    x17",            0xD61F0220, a64::x30, false, nullptr},
    {"blr   x8",             0xD63F0100, a64::x30, true,  nullptr},

    // A ROR-shifted register operand is read, not rejected.
    {"eor   x0, x1, x2, ror #8", 0xCAC22020, a64::x0, true, "<rotR>"},
  }};
  // clang-format on

  bool failed = false;
  for(auto const &c : cases) {
    std::array<unsigned char, 4> bytes{};
    for(int i = 0; i < 4; ++i) {
      bytes[i] = static_cast<unsigned char>(c.encoding >> (8 * i));
    }
    InstructionAPI::InstructionDecoder dec(bytes.data(), bytes.size(), Arch_aarch64);
    InstructionAPI::Instruction insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << c.text << ": could not decode\n";
      failed = true;
      continue;
    }
    std::string const value = written(insn, c.reg);
    std::string const name = c.reg.name();
    if(value.empty() == c.written) {
      std::cerr << c.text << " (" << insn.format() << "): " << name
                << (c.written ? " not written" : " written as " + value) << '\n';
      failed = true;
    } else if(c.written && c.contains && value.find(c.contains) == std::string::npos) {
      std::cerr << c.text << " (" << insn.format() << "): " << name << " = " << value
                << ", expected it to contain " << c.contains << '\n';
      failed = true;
    }
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
