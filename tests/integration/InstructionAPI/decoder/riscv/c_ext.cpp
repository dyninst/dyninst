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
  // General purpose registers
  auto zero = Dyninst::riscv64::zero;
  auto ra = Dyninst::riscv64::ra;
  auto sp = Dyninst::riscv64::sp;
  auto gp = Dyninst::riscv64::gp;
  auto tp = Dyninst::riscv64::tp;
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
  auto a6 = Dyninst::riscv64::a6;
  auto a7 = Dyninst::riscv64::a7;
  auto s2 = Dyninst::riscv64::s2;
  auto s3 = Dyninst::riscv64::s3;
  auto s4 = Dyninst::riscv64::s4;
  auto s5 = Dyninst::riscv64::s5;
  auto s6 = Dyninst::riscv64::s6;
  auto s7 = Dyninst::riscv64::s7;
  auto s8 = Dyninst::riscv64::s8;
  auto s9 = Dyninst::riscv64::s9;
  auto s10 = Dyninst::riscv64::s10;
  auto s11 = Dyninst::riscv64::s11;
  auto t3 = Dyninst::riscv64::t3;
  auto t4 = Dyninst::riscv64::t4;
  auto t5 = Dyninst::riscv64::t5;
  auto t6 = Dyninst::riscv64::t6;

  // Floating point registers
  // Only f8 ~ f15 is used in compressed instructions
  auto f8 = Dyninst::riscv64::f8;
  auto f9 = Dyninst::riscv64::f9;
  auto f10 = Dyninst::riscv64::f10;
  auto f11 = Dyninst::riscv64::f11;
  auto f12 = Dyninst::riscv64::f12;
  auto f13 = Dyninst::riscv64::f13;
  auto f14 = Dyninst::riscv64::f14;
  auto f15 = Dyninst::riscv64::f15;

  auto pc = Dyninst::riscv64::pc;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // c.addi tp, 8
      {0x21,0x02},
      di::register_rw_test{ reg_set{tp}, reg_set{tp} },
      di::mem_test{}
    },
    { // c.addi s4, -27
      {0x15,0x1a},
      di::register_rw_test{ reg_set{s4}, reg_set{s4} },
      di::mem_test{}
    },
    { // c.nop
      {0x01,0x00},
      di::register_rw_test{ reg_set{zero}, reg_set{zero} },
      di::mem_test{}
    },
    { // c.li t0, 5
      {0x95,0x42},
      di::register_rw_test{ reg_set{zero}, reg_set{t0} },
      di::mem_test{}
    },
    { // c.li s9, 26
      {0xe9,0x4c},
      di::register_rw_test{ reg_set{zero}, reg_set{s9} },
      di::mem_test{}
    },
    { // c.lui s6, 22
      {0x59,0x6b},
      di::register_rw_test{ reg_set{}, reg_set{s6} },
      di::mem_test{}
    },
    { // c.lui s11, 30
      {0xf9,0x6d},
      di::register_rw_test{ reg_set{}, reg_set{s11} },
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
    { // c.slli t4, 19
      {0xce,0x0e},
      di::register_rw_test{ reg_set{t4}, reg_set{t4} },
      di::mem_test{}
    },
    { // c.add s7, s8
      {0xe2,0x9b},
      di::register_rw_test{ reg_set{s7, s8}, reg_set{s7} },
      di::mem_test{}
    },
    { // c.sub a1, a4
      {0x99,0x8d},
      di::register_rw_test{ reg_set{a1, a4}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.xor a3, s1
      {0xa5,0x8e},
      di::register_rw_test{ reg_set{a3, s1}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.or s0, a5
      {0x5d,0x8c},
      di::register_rw_test{ reg_set{s0, a5}, reg_set{s0} },
      di::mem_test{}
    },
    { // c.and a1, a0
      {0xe9,0x8d},
      di::register_rw_test{ reg_set{a1, a0}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.add s5, t5
      {0xfa,0x9a},
      di::register_rw_test{ reg_set{s5, t5}, reg_set{s5} },
      di::mem_test{}
    },
    { // c.mv s2, s10
      {0x6a,0x89},
      di::register_rw_test{ reg_set{s10, zero}, reg_set{s2} },
      di::mem_test{}
    },
    { // c.mv s3, t5
      {0xfa,0x89},
      di::register_rw_test{ reg_set{t5, zero}, reg_set{s3} },
      di::mem_test{}
    },
    { // c.j 256
      {0x01,0xa2},
      di::register_rw_test{ reg_set{pc}, reg_set{pc, zero} },
      di::mem_test{}
    },
    { // c.beqz s0, 16
      {0x01,0xc8},
      di::register_rw_test{ reg_set{s0, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.bnez s1, -8
      {0xe5,0xfc},
      di::register_rw_test{ reg_set{s1, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.lwsp gp, 8(sp)
      {0xa2,0x41},
      di::register_rw_test{ reg_set{sp}, reg_set{gp} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.swsp a7, 12(sp)
      {0x46,0xc6},
      di::register_rw_test{ reg_set{a7, sp}, reg_set{} },
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
    { // c.mv t2, t3
      {0xf2,0x83},
      di::register_rw_test{ reg_set{t3, zero}, reg_set{t2} },
      di::mem_test{}
    },
    { // c.xor s0, a3
      {0x35,0x8c},
      di::register_rw_test{ reg_set{s0, a3}, reg_set{s0} },
      di::mem_test{}
    },
    { // c.jr ra
      {0x82,0x80},
      di::register_rw_test{ reg_set{ra}, reg_set{zero, pc} },
      di::mem_test{},
    },
    { // c.jr a6
      {0x02,0x88},
      di::register_rw_test{ reg_set{a6}, reg_set{zero, pc} },
      di::mem_test{},
    },
    { // c.jalr a0
      {0x02,0x95},
      di::register_rw_test{ reg_set{a0}, reg_set{ra, pc} },
      di::mem_test{},
    },
    { // c.jalr t6
      {0x82,0x9f},
      di::register_rw_test{ reg_set{t6}, reg_set{ra, pc} },
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
    { // c.andi a3, 15
      {0xbd,0x8a},
      di::register_rw_test{ reg_set{a3}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.slli a2, 40
      {0x22,0x16},
      di::register_rw_test{ reg_set{a2}, reg_set{a2} },
      di::mem_test{}
    },
    // The following are RV64C only instructions
    { // c.addiw ra, 0xc
      {0xb1,0x20},
      di::register_rw_test{ reg_set{ra}, reg_set{ra} },
      di::mem_test{}
    },
    { // c.addw a3, s1
      {0xa5,0x9e},
      di::register_rw_test{ reg_set{a3, s1}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.subw a4, a2
      {0x11,0x9f},
      di::register_rw_test{ reg_set{a4, a2}, reg_set{a4} },
      di::mem_test{}
    },
    { // c.ldsp a0, 24(sp)
      {0x62,0x65},
      di::register_rw_test{ reg_set{sp}, reg_set{a0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.sdsp a6, 12(sp)
      {0x42,0xec},
      di::register_rw_test{ reg_set{a6, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.fldsp f8, 16(sp)
      {0x42,0x24},
      di::register_rw_test{ reg_set{sp}, reg_set{f8} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.fldsp f9, 304(sp)
      {0xd2,0x34},
      di::register_rw_test{ reg_set{sp}, reg_set{f9} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.fsdsp f10, 192(sp)
      {0xaa,0xa1},
      di::register_rw_test{ reg_set{f10, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.fsdsp f11, 88(sp)
      {0xae,0xac},
      di::register_rw_test{ reg_set{f11, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
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
    { // c.fld f12, 56(a1)
      {0x90,0x3d},
      di::register_rw_test{ reg_set{a1}, reg_set{f12} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a1}, reg_set{} } }
    },
    { // c.fld f13, 184(a3)
      {0xd4,0x3e},
      di::register_rw_test{ reg_set{a3}, reg_set{f13} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a3}, reg_set{} } }
    },
    { // c.fsd f14, 200(s1)
      {0xf8,0xa4},
      di::register_rw_test{ reg_set{s1, f14}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s1} } }
    },
    { // c.fsd f15, 56(a4)
      {0x1c,0xbf},
      di::register_rw_test{ reg_set{a4, f15}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a4} } }
    },
  };
}
