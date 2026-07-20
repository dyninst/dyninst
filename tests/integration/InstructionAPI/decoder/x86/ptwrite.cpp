#include "Architecture.h"
#include "cft_tests.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "opcode_tests.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  Intel 64 and IA-32 Architectures Software Developer's Manual (SDM)
 *  June 2025
 *  PTWRITE - Write Data to a Processor Trace Packet (F3 0F AE /4)
 *
 *  ptwrite only reads its operand: its sole output is a PTW packet in the
 *  Intel PT trace stream, so it writes no architectural register, no flag,
 *  and no memory. Instrumentation analyses rely on that empty write set
 *  (issue #1523). The same group 15 /4 slot without an F3 prefix is xsave,
 *  so its decoding is checked here too.
 */

namespace di = Dyninst::InstructionAPI;

struct ptwrite_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

constexpr bool reads_memory = true;
constexpr bool writes_memory = true;

static std::vector<ptwrite_test> make_tests32();
static std::vector<ptwrite_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<ptwrite_test> const &);
static bool run_undefined_encodings();

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests32());

  if(!run(Dyninst::Arch_x86_64, make_tests64())) {
    ok = false;
  }
  if(!run_undefined_encodings()) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<ptwrite_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'ptwrite' in " << sarch << " mode\n";

  // ptwrite is never a control-flow instruction.
  const di::cft_test no_cft{!di::has_cft, {}};

  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.regs)) {
      failed = true;
    }
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
    if(!di::verify(insn, no_cft)) {
      failed = true;
    }
    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return !failed;
}

// Encodings in the same group 15 /4 slot that the SDM leaves undefined must
// not decode.
bool run_undefined_encodings() {
  const std::vector<std::pair<const char *, std::vector<unsigned char>>> tests = {
    {"unprefixed group 15 /4 register form", {0x0f, 0xae, 0xe7}},
    {"66-prefixed group 15 /4 register form", {0x66, 0x0f, 0xae, 0xe7}},
    {"F2-prefixed group 15 /4 register form", {0xf2, 0x0f, 0xae, 0xe7}},
  };

  bool failed = false;
  std::clog << "Running tests for undefined group 15 /4 encodings\n";
  for(auto const &t : tests) {
    di::InstructionDecoder d(t.second.data(), t.second.size(), Dyninst::Arch_x86_64);
    auto insn = d.decode();
    if(insn.isValid()) {
      std::cerr << t.first << ": expected decode failure, got '" << insn.format() << "'\n";
      failed = true;
    }
  }
  return !failed;
}

std::vector<ptwrite_test> make_tests32() {
  auto edi = Dyninst::x86::edi;

  using reg_set = Dyninst::register_set;

  const di::mem_test no_mem{!reads_memory, !writes_memory,
                            di::register_rw_test{reg_set{}, reg_set{}}};

  // clang-format off
  return {
    { // ptwrite %edi
      {0xf3, 0x0f, 0xae, 0xe7},
      di::opcode_test(e_ptwrite, "ptwrite %edi"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      no_mem
    },
    { // ptwrite (%edi)
      {0xf3, 0x0f, 0xae, 0x27},
      di::opcode_test(e_ptwrite, "ptwrite (%edi)"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{edi},
          reg_set{}
        }
      }
    },
  };
  // clang-format on
}

std::vector<ptwrite_test> make_tests64() {
  auto rdi = Dyninst::x86_64::rdi;
  auto edi = Dyninst::x86_64::edi;
  auto r13 = Dyninst::x86_64::r13;

  using reg_set = Dyninst::register_set;

  const di::mem_test no_mem{!reads_memory, !writes_memory,
                            di::register_rw_test{reg_set{}, reg_set{}}};

  // clang-format off
  return {
    { // ptwrite %rdi
      {0xf3, 0x48, 0x0f, 0xae, 0xe7},
      di::opcode_test(e_ptwrite, "ptwrite %rdi"),
      di::register_rw_test{
        reg_set{rdi},
        reg_set{}
      },
      no_mem
    },
    { // ptwrite %r13 (REX.B)
      {0xf3, 0x49, 0x0f, 0xae, 0xe5},
      di::opcode_test(e_ptwrite, "ptwrite %r13"),
      di::register_rw_test{
        reg_set{r13},
        reg_set{}
      },
      no_mem
    },
    { // ptwrite %edi (no REX.W: 32-bit operand)
      {0xf3, 0x0f, 0xae, 0xe7},
      di::opcode_test(e_ptwrite, "ptwrite %edi"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      no_mem
    },
    { // ptwrite (%rdi) (m64)
      {0xf3, 0x48, 0x0f, 0xae, 0x27},
      di::opcode_test(e_ptwrite, "ptwrite (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        reg_set{}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // xsave (%rdi): the unprefixed neighbor in the same group 15 /4 slot.
      // xsave also reads the requested-feature bitmap in EDX:EAX
      // (SDM Vol 1, Section 13.7).
      {0x0f, 0xae, 0x27},
      di::opcode_test(e_xsave, "xsave (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, Dyninst::x86_64::edx, Dyninst::x86_64::eax},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
  };
  // clang-format on
}
