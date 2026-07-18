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
#include <iterator>
#include <utility>
#include <vector>

/*
 *  Decoding checks for x86 instructions, verified against the Intel 64
 *  and IA-32 Architectures Software Developer's Manual (SDM, June 2025)
 *  and the AMD64 Architecture Programmer's Manual (APM). Each case pins
 *  the mnemonic, the instruction length, the exact register read/write
 *  sets (including implicit operands and flags), and the memory access
 *  direction with its address registers; reserved encodings must not
 *  decode. Coverage:
 *
 *  - Group 15 (0F AE; SDM Vol 2D, Table A-6): every slot selected by
 *    ModRM.reg, ModRM.mod, and the mandatory prefix, including the
 *    xsave family's implicit EDX:EAX feature-bitmap read (SDM Vol 1,
 *    Section 13.7) and the 64-bit-mode-only FSGSBASE forms.
 *  - Group 9 (0F C7): xrstors/xsavec/xsaves memory direction and
 *    implicit reads; rdrand's flag writes.
 *  - Group 7 (0F 01, mod == 11): rdtscp, swapgs, monitor/mwait,
 *    xgetbv/xsetbv, with their implicit register effects.
 *  - Group 11 (C6/C7 F8): xabort imm8 and xbegin's relative branch
 *    target (SDM Vol 2C).
 *  - Group 16 (0F 18 /4../7 mem): reserved NOPs.
 *  - Two-byte map identities: cmovg, tzcnt, lzcnt, psubq.
 *  - SSE4A extrq/insertq operand forms and lengths (AMD APM Vol 4).
 *  - VEX space: vmovd/vmovq W selection, VEX3 vpand, FMA3 vfnmsub
 *    132/213/231 packed forms, vaesdeclast's destination write,
 *    vmpsadbw, FMA4 (AMD APM Vol 6), and the destination write of
 *    four-operand blendv forms.
 */

namespace di = Dyninst::InstructionAPI;

struct decoding_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

constexpr bool reads_memory = true;
constexpr bool writes_memory = true;

namespace {

const di::mem_test no_mem{!reads_memory, !writes_memory,
                          di::register_rw_test{Dyninst::register_set{}, Dyninst::register_set{}}};

bool run(Dyninst::Architecture arch, std::vector<decoding_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running decoding tests in " << sarch << " mode\n";

  // xbegin branches to its abort handler; nothing else covered here is
  // a control-flow instruction.
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

    if(insn.size() != t.bytes.size()) {
      std::cerr << "Length mismatch: expected " << t.bytes.size() << ", got " << insn.size()
                << '\n';
      failed = true;
    }
    if(!di::verify(insn, t.regs)) {
      failed = true;
    }
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
    if(t.opcode.opcode == e_xbegin) {
      if(insn.cft_begin() == insn.cft_end()) {
        std::cerr << "Expected a control-flow target (the abort handler)\n";
        failed = true;
      }
    } else if(!di::verify(insn, no_cft)) {
      failed = true;
    }
    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }

    std::clog << '\n';
  }
  return !failed;
}

// Encodings the SDM/APM leave reserved (or restrict to 64-bit mode) must
// not decode.
bool run_reserved_encodings() {
  using bytes = std::vector<unsigned char>;
  struct reserved_test {
    const char *what;
    bytes encoding;
    Dyninst::Architecture arch;
  };

  const std::vector<reserved_test> tests = {
    // Group 15 slots with a prefix the SDM does not define (Table A-6).
    {"F3 /0 memory form", {0xf3, 0x0f, 0xae, 0x07}, Dyninst::Arch_x86_64},
    {"66 /1 memory form", {0x66, 0x0f, 0xae, 0x0f}, Dyninst::Arch_x86_64},
    {"F3 /2 memory form", {0xf3, 0x0f, 0xae, 0x17}, Dyninst::Arch_x86_64},
    {"F3 /5 memory form", {0xf3, 0x0f, 0xae, 0x2f}, Dyninst::Arch_x86_64},
    {"F2 /6 memory form", {0xf2, 0x0f, 0xae, 0x37}, Dyninst::Arch_x86_64},
    {"F3 /7 memory form", {0xf3, 0x0f, 0xae, 0x3f}, Dyninst::Arch_x86_64},
    {"unprefixed /0 register form", {0x0f, 0xae, 0xc0}, Dyninst::Arch_x86_64},
    {"66 /0 register form", {0x66, 0x0f, 0xae, 0xc0}, Dyninst::Arch_x86_64},
    {"66 /5 register form (lfence slot)", {0x66, 0x0f, 0xae, 0xe8}, Dyninst::Arch_x86_64},
    {"F2 /5 register form (lfence slot)", {0xf2, 0x0f, 0xae, 0xe8}, Dyninst::Arch_x86_64},
    {"F3 /7 register form (sfence slot)", {0xf3, 0x0f, 0xae, 0xf8}, Dyninst::Arch_x86_64},
    {"66 /7 register form (sfence slot)", {0x66, 0x0f, 0xae, 0xf8}, Dyninst::Arch_x86_64},
    {"F2 /7 register form (sfence slot)", {0xf2, 0x0f, 0xae, 0xf8}, Dyninst::Arch_x86_64},
    // FSGSBASE is valid in 64-bit mode only (SDM Vol 2B RDFSBASE/
    // RDGSBASE, Vol 2C WRFSBASE/WRGSBASE), as is swapgs (Vol 2B).
    {"rdfsbase in 32-bit mode", {0xf3, 0x0f, 0xae, 0xc0}, Dyninst::Arch_x86},
    {"rdgsbase in 32-bit mode", {0xf3, 0x0f, 0xae, 0xc8}, Dyninst::Arch_x86},
    {"wrfsbase in 32-bit mode", {0xf3, 0x0f, 0xae, 0xd0}, Dyninst::Arch_x86},
    {"wrgsbase in 32-bit mode", {0xf3, 0x0f, 0xae, 0xd8}, Dyninst::Arch_x86},
    {"swapgs in 32-bit mode", {0x0f, 0x01, 0xf8}, Dyninst::Arch_x86},
    // Group 7 mod == 11 slots Table A-6 leaves reserved.
    {"group 7 reserved mod=11 slot (0F 01 FA)", {0x0f, 0x01, 0xfa}, Dyninst::Arch_x86_64},
    // Group 9 slots Table A-6 leaves reserved.
    {"group 9 F2 /6 memory form", {0xf2, 0x0f, 0xc7, 0x37}, Dyninst::Arch_x86_64},
    {"group 9 F3 /7 memory form", {0xf3, 0x0f, 0xc7, 0x3f}, Dyninst::Arch_x86_64},
    {"group 9 /1 register form (cmpxchg8b slot)", {0x0f, 0xc7, 0xc8}, Dyninst::Arch_x86_64},
    {"group 9 F2 /7 register form", {0xf2, 0x0f, 0xc7, 0xf8}, Dyninst::Arch_x86_64},
    // FMA4 requires a VEX prefix with pp = 66 (AMD APM Vol 6).
    {"legacy (non-VEX) FMA4 opcode", {0x66, 0x0f, 0x3a, 0x68, 0xca, 0x30}, Dyninst::Arch_x86_64},
    {"FMA4 with VEX.pp = F2", {0xc4, 0xe3, 0x73, 0x68, 0xca, 0x30}, Dyninst::Arch_x86_64},
  };

  bool failed = false;
  std::clog << "Running tests for reserved encodings\n";
  for(auto const &t : tests) {
    di::InstructionDecoder d(t.encoding.data(), t.encoding.size(), t.arch);
    auto insn = d.decode();
    if(insn.isValid()) {
      std::cerr << t.what << ": expected decode failure, got '" << insn.format() << "'\n";
      failed = true;
    }
  }
  return !failed;
}

// The 64-bit test table is assembled from chunks: the compiler
// materializes every temporary of a braced initializer list in the
// enclosing function's stack frame, so one function holding every test
// would exceed -Wframe-larger-than (see cmake/DyninstWarnings.cmake).
using test_list = std::vector<decoding_test>;

void append(test_list &tests, test_list chunk) {
  tests.insert(tests.end(), std::make_move_iterator(chunk.begin()),
               std::make_move_iterator(chunk.end()));
}

Dyninst::register_set status_flags64() {
  return Dyninst::register_set{Dyninst::x86_64::cf, Dyninst::x86_64::pf,
                               Dyninst::x86_64::af, Dyninst::x86_64::zf,
                               Dyninst::x86_64::sf, Dyninst::x86_64::of};
}

// CMOVcc reads OF/SF/ZF/PF/CF (SDM Vol 2A, CMOVcc).
Dyninst::register_set cc_flags64() {
  return Dyninst::register_set{Dyninst::x86_64::cf, Dyninst::x86_64::pf,
                               Dyninst::x86_64::zf, Dyninst::x86_64::sf,
                               Dyninst::x86_64::of};
}

// group 15 (0F AE), memory forms
test_list group15_memory_tests() {
  auto eax = Dyninst::x86_64::eax;
  auto edx = Dyninst::x86_64::edx;
  auto rdi = Dyninst::x86_64::rdi;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // xsaveopt mem (NP 0F AE /6; SDM Vol 2C, XSAVEOPT). The xsave
      // family reads the requested-feature bitmap in EDX:EAX
      // (SDM Vol 1, 13.7).
      {0x0f, 0xae, 0x37},
      di::opcode_test(e_xsaveopt, "xsaveopt (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
    { // clrssbsy m64 (F3 0F AE /6; SDM Vol 2A): reads the shadow-stack
      // token and clears its busy bit, so memory is both read and written.
      {0xf3, 0x0f, 0xae, 0x37},
      di::opcode_test(e_clrssbsy, "clrssbsy (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        reg_set{}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{rdi}
        }
      }
    },
    { // clwb m8 (66 0F AE /6; SDM Vol 2A).
      {0x66, 0x0f, 0xae, 0x37},
      di::opcode_test(e_clwb, "clwb (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
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
    { // clflush m8 (NP 0F AE /7; SDM Vol 2A).
      {0x0f, 0xae, 0x3f},
      di::opcode_test(e_clflush, "clflush (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
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
    { // clflushopt m8 (66 0F AE /7; SDM Vol 2A).
      {0x66, 0x0f, 0xae, 0x3f},
      di::opcode_test(e_clflushopt, "clflushopt (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
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
    { // xsave mem (NP 0F AE /4; SDM Vol 2C, XSAVE): reads EDX:EAX.
      {0x0f, 0xae, 0x27},
      di::opcode_test(e_xsave, "xsave (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
    { // xrstor mem (NP 0F AE /5; SDM Vol 2C, XRSTOR): reads EDX:EAX and
      // the XSAVE area.
      {0x0f, 0xae, 0x2f},
      di::opcode_test(e_xrstor, "xrstor (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
  };
  // clang-format on
}

// group 15 (0F AE), register forms: FSGSBASE and CET
test_list group15_register_tests() {
  auto rax = Dyninst::x86_64::rax;
  auto eax = Dyninst::x86_64::eax;
  auto rdi = Dyninst::x86_64::rdi;
  auto edi = Dyninst::x86_64::edi;
  auto r13 = Dyninst::x86_64::r13;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // rdfsbase r32 (F3 0F AE /0; SDM Vol 2B).
      {0xf3, 0x0f, 0xae, 0xc0},
      di::opcode_test(e_rdfsbase, "rdfsbase %eax"),
      di::register_rw_test{
        reg_set{},
        reg_set{eax}
      },
      no_mem
    },
    { // rdfsbase r64 (F3 REX.W 0F AE /0; SDM Vol 2B).
      {0xf3, 0x48, 0x0f, 0xae, 0xc0},
      di::opcode_test(e_rdfsbase, "rdfsbase %rax"),
      di::register_rw_test{
        reg_set{},
        reg_set{rax}
      },
      no_mem
    },
    { // rdgsbase r64 with REX.B (F3 0F AE /1; SDM Vol 2B).
      {0xf3, 0x49, 0x0f, 0xae, 0xcd},
      di::opcode_test(e_rdgsbase, "rdgsbase %r13"),
      di::register_rw_test{
        reg_set{},
        reg_set{r13}
      },
      no_mem
    },
    { // wrfsbase r32 (F3 0F AE /2; SDM Vol 2C).
      {0xf3, 0x0f, 0xae, 0xd7},
      di::opcode_test(e_wrfsbase, "wrfsbase %edi"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      no_mem
    },
    { // wrgsbase r32 (F3 0F AE /3; SDM Vol 2C).
      {0xf3, 0x0f, 0xae, 0xdf},
      di::opcode_test(e_wrgsbase, "wrgsbase %edi"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      no_mem
    },
    { // lfence (NP 0F AE /5, mod == 11; SDM Vol 2A). ModRM.rm is ignored.
      {0x0f, 0xae, 0xef},
      di::opcode_test(e_lfence, "lfence"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
    { // incsspd r32 (F3 0F AE /5, mod == 11; SDM Vol 2A): reads the
      // shift count. The SSP update itself is not representable as an
      // architectural register.
      {0xf3, 0x0f, 0xae, 0xe8},
      di::opcode_test(e_incsspd, "incsspd %eax"),
      di::register_rw_test{
        reg_set{eax},
        reg_set{}
      },
      no_mem
    },
    { // incsspq r64 (F3 REX.W 0F AE /5; SDM Vol 2A). The tables cannot
      // select a mnemonic on REX.W, so the ID stays e_incsspd; the
      // operand width follows REX.W and is correct.
      {0xf3, 0x48, 0x0f, 0xae, 0xef},
      di::opcode_test(e_incsspd, "incsspd %rdi"),
      di::register_rw_test{
        reg_set{rdi},
        reg_set{}
      },
      no_mem
    },
  };
  // clang-format on
}

// group 15 (0F AE), register forms: fences and WAITPKG
test_list group15_fence_tests() {
  auto eax = Dyninst::x86_64::eax;
  auto ax = Dyninst::x86_64::ax;
  auto edx = Dyninst::x86_64::edx;
  auto dx = Dyninst::x86_64::dx;
  auto edi = Dyninst::x86_64::edi;

  using reg_set = Dyninst::register_set;
  const reg_set status_flags = status_flags64();

  // clang-format off
  return {
    { // mfence (NP 0F AE /6, mod == 11; SDM Vol 2B).
      {0x0f, 0xae, 0xf0},
      di::opcode_test(e_mfence, "mfence"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
    { // umonitor r64 (F3 0F AE /6, mod == 11; SDM Vol 2B). The operand
      // register holds an address and follows the address size, which
      // the tables cannot express, so the default 64-bit form reports
      // the 32-bit sub-register; the register identity is what matters
      // for dependence analysis.
      {0xf3, 0x0f, 0xae, 0xf7},
      di::opcode_test(e_umonitor, "umonitor %edi"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      no_mem
    },
    { // tpause r32 (66 0F AE /6, mod == 11; SDM Vol 2B): also reads the
      // deadline from EDX:EAX and writes the status flags. The mandatory
      // 66 prefix doubles as the operand-size prefix in the prefix
      // parser, so the implicit pair reports as DX/AX; the full
      // registers are the true read set.
      {0x66, 0x0f, 0xae, 0xf7},
      di::opcode_test(e_tpause, "tpause %edi"),
      di::register_rw_test{
        reg_set{edi, dx, ax},
        status_flags
      },
      no_mem
    },
    { // umwait r32 (F2 0F AE /6, mod == 11; SDM Vol 2B): reads EDX:EAX
      // and writes the status flags.
      {0xf2, 0x0f, 0xae, 0xf7},
      di::opcode_test(e_umwait, "umwait %edi"),
      di::register_rw_test{
        reg_set{edi, edx, eax},
        status_flags
      },
      no_mem
    },
    { // sfence (NP 0F AE /7, mod == 11; SDM Vol 2B).
      {0x0f, 0xae, 0xf8},
      di::opcode_test(e_sfence, "sfence"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
  };
  // clang-format on
}

// group 9 (0F C7): compacted-xsave family and rdrand/rdseed/rdpid
test_list group9_xsave_rand_tests() {
  auto rax = Dyninst::x86_64::rax;
  auto eax = Dyninst::x86_64::eax;
  auto ecx = Dyninst::x86_64::ecx;
  auto edx = Dyninst::x86_64::edx;
  auto rdi = Dyninst::x86_64::rdi;

  using reg_set = Dyninst::register_set;
  const reg_set status_flags = status_flags64();

  // clang-format off
  return {
    { // xrstors mem (NP 0F C7 /3; SDM Vol 2C, XRSTORS).
      {0x0f, 0xc7, 0x1f},
      di::opcode_test(e_xrstors, "xrstors (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
    { // xsavec mem (NP 0F C7 /4; SDM Vol 2C, XSAVEC).
      {0x0f, 0xc7, 0x27},
      di::opcode_test(e_xsavec, "xsavec (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
    { // xsaves mem (NP 0F C7 /5; SDM Vol 2C, XSAVES).
      {0x0f, 0xc7, 0x2f},
      di::opcode_test(e_xsaves, "xsaves (%rdi)"),
      di::register_rw_test{
        reg_set{rdi, edx, eax},
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
    { // rdrand r32 (NP 0F C7 /6, mod == 11; SDM Vol 2B, RDRAND): CF
      // reports validity and the other status flags are cleared.
      {0x0f, 0xc7, 0xf0},
      di::opcode_test(e_rdrand, "rdrand %eax"),
      di::register_rw_test{
        reg_set{},
        reg_set{eax} | status_flags
      },
      no_mem
    },
    { // rdrand r64 (NP REX.W 0F C7 /6; SDM Vol 2B, RDRAND).
      {0x48, 0x0f, 0xc7, 0xf0},
      di::opcode_test(e_rdrand, "rdrand %rax"),
      di::register_rw_test{
        reg_set{},
        reg_set{rax} | status_flags
      },
      no_mem
    },
    { // rdrand r16 (66 0F C7 /6: the 66 prefix is the operand-size
      // variant; SDM Vol 2B, RDRAND).
      {0x66, 0x0f, 0xc7, 0xf0},
      di::opcode_test(e_rdrand, "rdrand %ax"),
      di::register_rw_test{
        reg_set{},
        reg_set{Dyninst::x86_64::ax} | status_flags
      },
      no_mem
    },
    { // rdseed r32 (NP 0F C7 /7, mod == 11; SDM Vol 2B, RDSEED).
      {0x0f, 0xc7, 0xf8},
      di::opcode_test(e_rdseed, "rdseed %eax"),
      di::register_rw_test{
        reg_set{},
        reg_set{eax} | status_flags
      },
      no_mem
    },
    { // rdpid (F3 0F C7 /7, mod == 11; SDM Vol 2B, RDPID). The operand
      // is always r64 in 64-bit mode, which the tables cannot express
      // without REX.W; the register identity is correct.
      {0xf3, 0x0f, 0xc7, 0xf9},
      di::opcode_test(e_rdpid, "rdpid %ecx"),
      di::register_rw_test{
        reg_set{},
        reg_set{ecx}
      },
      no_mem
    },
  };
  // clang-format on
}

// group 9 (0F C7): VMX pointer instructions
test_list group9_vmx_tests() {
  auto rdi = Dyninst::x86_64::rdi;

  using reg_set = Dyninst::register_set;
  const reg_set status_flags = status_flags64();

  // clang-format off
  return {
    { // vmptrld m64 (NP 0F C7 /6, mem; SDM Vol 2C): reads the pointer
      // and reports success through the status flags (SDM Vol 3C).
      {0x0f, 0xc7, 0x37},
      di::opcode_test(e_vmptrld, "vmptrld (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        status_flags
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // vmxon m64 (F3 0F C7 /6, mem; SDM Vol 2C).
      {0xf3, 0x0f, 0xc7, 0x37},
      di::opcode_test(e_vmxon, "vmxon (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        status_flags
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{rdi},
          reg_set{}
        }
      }
    },
    { // vmclear m64 (66 0F C7 /6, mem; SDM Vol 2C).
      {0x66, 0x0f, 0xc7, 0x37},
      di::opcode_test(e_vmclear, "vmclear (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        status_flags
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rdi}
        }
      }
    },
    { // vmptrst m64 (NP 0F C7 /7, mem; SDM Vol 2C).
      {0x0f, 0xc7, 0x3f},
      di::opcode_test(e_vmptrst, "vmptrst (%rdi)"),
      di::register_rw_test{
        reg_set{rdi},
        status_flags
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

// group 7 (0F 01) mod == 11, group 11 (C6/C7 F8), and group 16 (0F 18)
test_list group7_11_16_tests() {
  auto eax = Dyninst::x86_64::eax;
  auto ecx = Dyninst::x86_64::ecx;
  auto edx = Dyninst::x86_64::edx;
  auto rip = Dyninst::x86_64::rip;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // rdtscp (0F 01 F9; SDM Vol 2B): writes EDX:EAX and ECX.
      {0x0f, 0x01, 0xf9},
      di::opcode_test(e_rdtscp, "rdtscp"),
      di::register_rw_test{
        reg_set{},
        reg_set{edx, eax, ecx}
      },
      no_mem
    },
    { // swapgs (0F 01 F8; SDM Vol 2B), 64-bit mode only.
      {0x0f, 0x01, 0xf8},
      di::opcode_test(e_swapgs, "swapgs"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
    { // monitor (0F 01 C8; SDM Vol 2B): reads the address in rAX plus
      // the ECX/EDX hints.
      {0x0f, 0x01, 0xc8},
      di::opcode_test(e_monitor, "monitor"),
      di::register_rw_test{
        reg_set{eax, ecx, edx},
        reg_set{}
      },
      no_mem
    },
    { // mwait (0F 01 C9; SDM Vol 2B): reads EAX/ECX.
      {0x0f, 0x01, 0xc9},
      di::opcode_test(e_mwait, "mwait"),
      di::register_rw_test{
        reg_set{eax, ecx},
        reg_set{}
      },
      no_mem
    },
    { // xgetbv (0F 01 D0; SDM Vol 2C): writes EDX:EAX, reads ECX.
      {0x0f, 0x01, 0xd0},
      di::opcode_test(e_xgetbv, "xgetbv"),
      di::register_rw_test{
        reg_set{ecx},
        reg_set{eax, edx}
      },
      no_mem
    },
    // -------- group 11 (C6/C7 F8, TSX) --------
    { // xabort imm8 (C6 F8 ib; SDM Vol 2C).
      {0xc6, 0xf8, 0x11},
      di::opcode_test(e_xabort, "xabort $0x11"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
    { // xbegin rel32 (C7 F8 cd; SDM Vol 2C): the operand is a relative
      // branch target.
      {0xc7, 0xf8, 0x11, 0x22, 0x33, 0x44},
      di::opcode_test(e_xbegin, "xbegin 0x44332217(%rip)"),
      di::register_rw_test{
        reg_set{rip},
        reg_set{}
      },
      no_mem
    },
    // -------- group 16 (0F 18) --------
    { // 0F 18 /4../7 with a memory operand are reserved NOPs
      // (SDM Vol 2D, Table A-6, Group 16, and Vol 2B, NOP).
      {0x0f, 0x18, 0x20},
      di::opcode_test(e_nop, "nop"),
      di::register_rw_test{
        reg_set{},
        reg_set{}
      },
      no_mem
    },
  };
  // clang-format on
}

// two-byte map identities and SSE4A (AMD APM Vol 4)
test_list twobyte_sse4a_tests() {
  auto rax = Dyninst::x86_64::rax;
  auto eax = Dyninst::x86_64::eax;
  auto rcx = Dyninst::x86_64::rcx;
  auto ecx = Dyninst::x86_64::ecx;
  auto mm1 = Dyninst::x86_64::mm1;
  auto mm2 = Dyninst::x86_64::mm2;
  auto xmm1 = Dyninst::x86_64::xmm1;
  auto xmm2 = Dyninst::x86_64::xmm2;

  using reg_set = Dyninst::register_set;
  const reg_set status_flags = status_flags64();
  const reg_set cc_flags = cc_flags64();

  // clang-format off
  return {
    { // cmovg (0F 4F /r; SDM Vol 2A, CMOVcc, tttn = 1111).
      {0x0f, 0x4f, 0xc8},
      di::opcode_test(e_cmovg, "cmovg %eax,%ecx"),
      di::register_rw_test{
        reg_set{ecx, eax} | cc_flags,
        reg_set{ecx}
      },
      no_mem
    },
    { // tzcnt r32 (F3 0F BC /r; SDM Vol 2B): ZF/CF are defined results,
      // the other status flags are undefined and conservatively written.
      {0xf3, 0x0f, 0xbc, 0xc8},
      di::opcode_test(e_tzcnt, "tzcnt %eax,%ecx"),
      di::register_rw_test{
        reg_set{eax},
        reg_set{ecx} | status_flags
      },
      no_mem
    },
    { // lzcnt r64 (F3 REX.W 0F BD /r; SDM Vol 2B).
      {0xf3, 0x48, 0x0f, 0xbd, 0xc8},
      di::opcode_test(e_lzcnt, "lzcnt %rax,%rcx"),
      di::register_rw_test{
        reg_set{rax},
        reg_set{rcx} | status_flags
      },
      no_mem
    },
    { // psubq mm, mm/m64 (NP 0F FB /r; SDM Vol 2B, PSUBQ).
      {0x0f, 0xfb, 0xca},
      di::opcode_test(e_psubq, "psubq %mm2,%mm1"),
      di::register_rw_test{
        reg_set{mm1, mm2},
        reg_set{mm1}
      },
      no_mem
    },
    { // psubq xmm1, xmm2/m128 (66 0F FB /r; SDM Vol 2B, PSUBQ).
      {0x66, 0x0f, 0xfb, 0xca},
      di::opcode_test(e_psubq, "psubq %xmm2,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    // -------- SSE4A (AMD APM Vol 4) --------
    { // extrq xmm, imm8, imm8 (66 0F 78 /0 ib ib): the xmm is in
      // ModRM.rm; the length includes both immediates.
      {0x66, 0x0f, 0x78, 0xc1, 0x11, 0x22},
      di::opcode_test(e_extrq, "extrq $0x22,$0x11,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1},
        reg_set{xmm1}
      },
      no_mem
    },
    { // extrq xmm1, xmm2 (66 0F 79 /r): register pair, no immediates.
      {0x66, 0x0f, 0x79, 0xd1},
      di::opcode_test(e_extrq, "extrq %xmm1,%xmm2"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm2}
      },
      no_mem
    },
    { // insertq xmm1, xmm2 (F2 0F 79 /r): register pair, no immediates.
      {0xf2, 0x0f, 0x79, 0xd1},
      di::opcode_test(e_insertq, "insertq %xmm1,%xmm2"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm2}
      },
      no_mem
    },
  };
  // clang-format on
}

// VEX space: vmovd/vmovq, vpand, and the FMA3 vfnmsub forms
test_list vex_mov_fma_tests() {
  auto rcx = Dyninst::x86_64::rcx;
  auto ecx = Dyninst::x86_64::ecx;
  auto xmm1 = Dyninst::x86_64::xmm1;
  auto xmm2 = Dyninst::x86_64::xmm2;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // vmovq r64 -> xmm (VEX.66.0F.W1 6E; SDM Vol 2B, MOVD/MOVQ).
      {0xc4, 0xe1, 0xf9, 0x6e, 0xc9},
      di::opcode_test(e_vmovq, "vmovq %rcx,%xmm1"),
      di::register_rw_test{
        reg_set{rcx},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vmovd r32 -> xmm (VEX.66.0F.W0 6E).
      {0xc4, 0xe1, 0x79, 0x6e, 0xc9},
      di::opcode_test(e_vmovd, "vmovd %ecx,%xmm1"),
      di::register_rw_test{
        reg_set{ecx},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vmovq xmm -> r64 (VEX.66.0F.W1 7E).
      {0xc4, 0xe1, 0xf9, 0x7e, 0xc9},
      di::opcode_test(e_vmovq, "vmovq %xmm1,%rcx"),
      di::register_rw_test{
        reg_set{xmm1},
        reg_set{rcx}
      },
      no_mem
    },
    { // vmovd xmm -> r32 (VEX.66.0F.W0 7E).
      {0xc4, 0xe1, 0x79, 0x7e, 0xc9},
      di::opcode_test(e_vmovd, "vmovd %xmm1,%ecx"),
      di::register_rw_test{
        reg_set{xmm1},
        reg_set{ecx}
      },
      no_mem
    },
    { // vpand, VEX3-encoded (SDM Vol 2B, PAND: the VEX form is vpand
      // for every W; vpandd/vpandq are EVEX-only).
      {0xc4, 0xe1, 0x71, 0xdb, 0xca},
      di::opcode_test(e_vpand, "vpand %xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vfnmsub213ps (VEX.66.0F38.W0 AE /r; SDM Vol 2C).
      {0xc4, 0xe2, 0x71, 0xae, 0xca},
      di::opcode_test(e_vfnmsub213ps, "vfnmsub213ps %xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vfnmsub132pd (VEX.66.0F38.W1 9E /r; SDM Vol 2C).
      {0xc4, 0xe2, 0xf1, 0x9e, 0xca},
      di::opcode_test(e_vfnmsub132pd, "vfnmsub132pd %xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vfnmsub231ps (VEX.66.0F38.W0 BE /r; SDM Vol 2C).
      {0xc4, 0xe2, 0x71, 0xbe, 0xca},
      di::opcode_test(e_vfnmsub231ps, "vfnmsub231ps %xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
  };
  // clang-format on
}

// VEX space: AES, blendv write sets, vmpsadbw, and FMA4 (AMD APM Vol 6)
test_list vex_blend_fma4_tests() {
  auto xmm1 = Dyninst::x86_64::xmm1;
  auto xmm2 = Dyninst::x86_64::xmm2;
  auto xmm3 = Dyninst::x86_64::xmm3;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // vaesdeclast (VEX.128.66.0F38.WIG DF /r; SDM Vol 2A): the
      // destination register is written.
      {0xc4, 0xe2, 0x71, 0xdf, 0xca},
      di::opcode_test(e_vaesdeclast, "vaesdeclast %xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // blendvps (66 0F 38 14 /r; SDM Vol 2A): four-operand semantic
      // with the destination written.
      {0x66, 0x0f, 0x38, 0x14, 0xca},
      di::opcode_test(e_blendvps, "blendvps %xmm2,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vblendvps (VEX.128.66.0F3A.W0 4A /r /is4; SDM Vol 2A).
      {0xc4, 0xe3, 0x71, 0x4a, 0xca, 0x30},
      di::opcode_test(e_vblendvps, "vblendvps $0x30,%xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vmpsadbw (VEX.128.66.0F3A.WIG 42 /r ib; SDM Vol 2B, MPSADBW).
      {0xc4, 0xe3, 0x71, 0x42, 0xca, 0x07},
      di::opcode_test(e_vmpsadbw, "vmpsadbw $0x7,%xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vfmaddps (FMA4, VEX.66.0F3A 68 /r /is4; AMD APM Vol 6).
      {0xc4, 0xe3, 0x71, 0x68, 0xca, 0x30},
      di::opcode_test(e_vfmaddps, "vfmaddps %xmm3,%xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2, xmm3},
        reg_set{xmm1}
      },
      no_mem
    },
    { // vfmaddpd (FMA4, VEX.66.0F3A 69 /r /is4; AMD APM Vol 6).
      {0xc4, 0xe3, 0x71, 0x69, 0xca, 0x30},
      di::opcode_test(e_vfmaddpd, "vfmaddpd %xmm3,%xmm2,%xmm1,%xmm1"),
      di::register_rw_test{
        reg_set{xmm1, xmm2, xmm3},
        reg_set{xmm1}
      },
      no_mem
    },
  };
  // clang-format on
}

test_list make_tests64() {
  test_list tests;
  append(tests, group15_memory_tests());
  append(tests, group15_register_tests());
  append(tests, group15_fence_tests());
  append(tests, group9_xsave_rand_tests());
  append(tests, group9_vmx_tests());
  append(tests, group7_11_16_tests());
  append(tests, twobyte_sse4a_tests());
  append(tests, vex_mov_fma_tests());
  append(tests, vex_blend_fma4_tests());
  return tests;
}

std::vector<decoding_test> make_tests32() {
  auto eax = Dyninst::x86::eax;
  auto edx = Dyninst::x86::edx;
  auto edi = Dyninst::x86::edi;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // xsaveopt is valid in 32-bit mode (SDM Vol 2C, XSAVEOPT).
      {0x0f, 0xae, 0x37},
      di::opcode_test(e_xsaveopt, "xsaveopt (%edi)"),
      di::register_rw_test{
        reg_set{edi, edx, eax},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{edi}
        }
      }
    },
    { // clrssbsy is valid in 32-bit mode (SDM Vol 2A, CLRSSBSY).
      {0xf3, 0x0f, 0xae, 0x37},
      di::opcode_test(e_clrssbsy, "clrssbsy (%edi)"),
      di::register_rw_test{
        reg_set{edi},
        reg_set{}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{edi},
          reg_set{edi}
        }
      }
    },
    { // incsspd is valid in 32-bit mode (SDM Vol 2A, INCSSPD).
      {0xf3, 0x0f, 0xae, 0xe8},
      di::opcode_test(e_incsspd, "incsspd %eax"),
      di::register_rw_test{
        reg_set{eax},
        reg_set{}
      },
      no_mem
    },
  };
  // clang-format on
}

} // namespace

int main() {
  bool ok = run(Dyninst::Arch_x86_64, make_tests64());

  if(!run(Dyninst::Arch_x86, make_tests32())) {
    ok = false;
  }
  if(!run_reserved_encodings()) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}
