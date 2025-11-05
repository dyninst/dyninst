#include "Architecture.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/riscv64_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  The RISC-V Instruction Set Manual Volume I: Unprivileged Architecture
 *  Chapter 27: "C" Extension for Compressed Instructions, Version 2.0
 */

namespace di = Dyninst::InstructionAPI;

struct rv64i_tests {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64i_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64i_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64i_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64i' in " << sarch << " mode\n";
  for (auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.opcode.data(), t.opcode.size(), arch);
    auto insn = d.decode();
    if (!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if (!di::verify(insn, t.regs)) {
      failed = true;
    }
    if (!di::verify(insn, t.mem)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<rv64i_tests> make_tests64() {
  auto ra = Dyninst::riscv64::ra;
  auto sp = Dyninst::riscv64::sp;
  auto t0 = Dyninst::riscv64::t0;
  auto t1 = Dyninst::riscv64::t1;
  auto t2 = Dyninst::riscv64::t2;
  auto s0 = Dyninst::riscv64::s0;
  auto s1 = Dyninst::riscv64::s1;
  auto a0 = Dyninst::riscv64::a0;
  auto a1 = Dyninst::riscv64::a1;
  auto a2 = Dyninst::riscv64::a2;
  auto a3 = Dyninst::riscv64::a3;
  auto a4 = Dyninst::riscv64::a4;
  auto a5 = Dyninst::riscv64::a5;
  auto s2 = Dyninst::riscv64::s2;
  auto t3 = Dyninst::riscv64::t3;
  auto pc = Dyninst::riscv64::pc;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // c.addi a0, 8
      {0x21,0x05},
      di::register_rw_test{ reg_set{a0}, reg_set{a0} },
      di::mem_test{}
    },
    { // c.nop
      {0x01,0x00},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    { // c.li t0, 5
      {0x95,0x42},
      di::register_rw_test{ reg_set{}, reg_set{t0} },
      di::mem_test{}
    },
    { // c.lui s0, 12
      {0x31,0x64},
      di::register_rw_test{ reg_set{}, reg_set{s0} },
      di::mem_test{}
    },
    { // c.addi16sp sp, 64
      {0x21,0x61},
      di::register_rw_test{ reg_set{sp}, reg_set{sp} },
      di::mem_test{}
    },
    { // c.slli t1, 3
      {0x0e,0x03},
      di::register_rw_test{ reg_set{t1}, reg_set{t1} },
      di::mem_test{}
    },
    { // c.add s1, s2
      {0xca,0x94},
      di::register_rw_test{ reg_set{s1, s2}, reg_set{s1} },
      di::mem_test{}
    },
    { // c.mv a1, a2
      {0xb2,0x85},
      di::register_rw_test{ reg_set{a2}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.j 256
      {0x01,0xa2},
      di::register_rw_test{ reg_set{pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.beqz s0, 16
      {0x01,0xc8},
      di::register_rw_test{ reg_set{s0, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.bnez s1, -8
      {0xe5,0xfc},
      di::register_rw_test{ reg_set{s1, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.lwsp a0, 8(sp)
      {0x22,0x45},
      di::register_rw_test{ reg_set{sp}, reg_set{a0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.swsp a1, 12(sp)
      {0x2e,0xc6},
      di::register_rw_test{ reg_set{a1, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.lw a2, 4(s1)
      {0xd0,0x40},
      di::register_rw_test{ reg_set{s1}, reg_set{a2} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // c.sw a3, 8(a0)
      {0x14,0xc5},
      di::register_rw_test{ reg_set{a3, a0}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a0} } }
    },
    { // c.ld a4, 0(a2)
      {0x18,0x62},
      di::register_rw_test{ reg_set{a2}, reg_set{a4} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a2}, reg_set{} } }
    },
    { // c.sd a5, 8(a4)
      {0x1c,0xe7},
      di::register_rw_test{ reg_set{a5, a4}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a4} } }
    },
    { // c.mv t2, t3
      {0xf2,0x83},
      di::register_rw_test{ reg_set{t3}, reg_set{t2} },
      di::mem_test{}
    },
    { // c.xor a2, a3
      {0x35,0x8e},
      di::register_rw_test{ reg_set{a2, a3}, reg_set{a2} },
      di::mem_test{}
    },
    { // c.jr ra
      {0x82,0x80},
      di::register_rw_test{ reg_set{ra}, reg_set{pc} },
      di::mem_test{},
    },
    { // c.jalr a0
      {0x02,0x95},
      di::register_rw_test{ reg_set{a0}, reg_set{ra, pc} },
      di::mem_test{},
    },
    { // c.ebreak
      {0x02,0x90},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    { // c.addi4spn a3, sp, 32
      {0x14,0x10},
      di::register_rw_test{ reg_set{sp}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.addi16sp sp, 16
      {0x41,0x61},
      di::register_rw_test{ reg_set{sp}, reg_set{sp} },
      di::mem_test{}
    },
    { // c.srli a0, 2
      {0x09,0x81},
      di::register_rw_test{ reg_set{a0}, reg_set{a0} },
      di::mem_test{}
    },
    { // c.srai a1, 12
      {0xb1,0x85},
      di::register_rw_test{ reg_set{a1}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.slli a2, 40
      {0x22,0x16},
      di::register_rw_test{ reg_set{a2}, reg_set{a2} },
      di::mem_test{}
    }
  };
}
