#include "Architecture.h"
#include "cft_tests.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "opcode_tests.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/aarch64_regs.h"
#include "entryIDs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

struct math_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
};

static std::vector<math_test> make_tests();

int main() {
  bool failed = false;
  int test_id = 0;
  const auto arch = Dyninst::Arch_aarch64;
  for(auto const &t : make_tests()) {
    test_id++;
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }
    if(!di::verify(insn, t.regs)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::vector<math_test> make_tests() {
  auto nzcv = Dyninst::aarch64::nzcv;
  auto cntpct_el0 = Dyninst::aarch64::cntpct_el0;
  auto pmevcntr2_el0 = Dyninst::aarch64::pmevcntr2_el0;
  auto pmceid0_el0 = Dyninst::aarch64::pmceid0_el0;
  auto pmevtyper30_el0 = Dyninst::aarch64::pmevtyper30_el0;
  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  return {
    { // clrex #0
      {0x5f, 0x30, 0x03, 0xd5},
      di::opcode_test(aarch64_op_clrex, "clrex #0"),
      di::register_rw_test {
        reg_set{},
        reg_set{},
      },
    },
    { // dcps2 #2
      {0x42, 0x00, 0xa0, 0xd4},
      di::opcode_test(aarch64_op_dcps2, "dcps2 #2"),
      di::register_rw_test {
        reg_set{nzcv},
        reg_set{},
      },
    },
    { // dmb oshld
      {0xbf, 0x31, 0x03, 0xd5},
      di::opcode_test(aarch64_op_dmb, "dmb #1"),
      di::register_rw_test {
        reg_set{},
        reg_set{},
      },
    },
    { // dsb #4
      {0x9f, 0x34, 0x03, 0xd5},
      di::opcode_test(aarch64_op_dsb, "dsb #4"),
      di::register_rw_test {
        reg_set{},
        reg_set{},
      },
    },
    { // hint #5
      {0xbf, 0x20, 0x03, 0xd5},
      di::opcode_test(aarch64_op_hint, "hint #5"),
      di::register_rw_test {
        reg_set{},
        reg_set{},
      },
    },
    { // hlt #30
      {0xc0, 0x03, 0x40, 0xd4},
      di::opcode_test(aarch64_op_hlt, "hlt #1"),  // WRONG: should be 'hlt #30'
      di::register_rw_test {
        reg_set{nzcv},
        reg_set{},
      },
    },
    { // mrs x0, pmevcntr2_el0
      {0x40, 0xe8, 0x3b, 0xd5},
      di::opcode_test(aarch64_op_mrs, "mrs x0, pmevcntr2_el0"),
      di::register_rw_test {
        reg_set{pmevcntr2_el0},
        reg_set{x0},
      },
    },
    { // mrs x1, pmceid0_el0
      {0xc1, 0x9c, 0x3b, 0xd5},
      di::opcode_test(aarch64_op_mrs, "mrs x1, pmceid0_el0"),
      di::register_rw_test {
        reg_set{pmceid0_el0},
        reg_set{x1},
      },
    },
    { // msr daifset, #5
      {0xdf, 0x45, 0x03, 0xd5},
      di::opcode_test(aarch64_op_msr_imm, "msr #1, #5"),  // WRONG: should be 'msr daifset, #5'
      di::register_rw_test {
        reg_set{},
        reg_set{nzcv},
      },
    },
    { // msr cntpct_el0, x1
      {0x21, 0xe0, 0x1b, 0xd5},
      di::opcode_test(aarch64_op_msr_reg, "msr cntpct_el0, x1"),
      di::register_rw_test {
        reg_set{x1},
        reg_set{cntpct_el0},
      },
    },
    { // mrs pmevtyper30_el0, x0
      {0xc0, 0xef, 0x1b, 0xd5},
      di::opcode_test(aarch64_op_msr_reg, "msr pmevtyper30_el0, x0"),
      di::register_rw_test {
        reg_set{x0},
        reg_set{pmevtyper30_el0},
      },
    },
    { // smc #0
      {0x03, 0x00, 0x00, 0xd4},
      di::opcode_test(aarch64_op_smc, "smc #0"),
      di::register_rw_test {
        reg_set{nzcv},
        reg_set{},
      },
    },
    { // svc #32768
      {0x01, 0x00, 0x10, 0xd4},
      di::opcode_test(aarch64_op_svc, "svc #8000"), // WRONG: should be 'svc #32768' (hex->dec)
      di::register_rw_test {
        reg_set{nzcv},
        reg_set{},
      },
    },
    { // sys #1, c2, c3, #4, x0
      {0x80, 0x23, 0x09, 0xd5},
      di::opcode_test(aarch64_op_sys, "sys #1, #2, #3, #4, x0"),  // WRONG: should be 'sys #1, c2, c3, #4, x0'
      di::register_rw_test {
        reg_set{x0},
        reg_set{},
      },
    },
    { // sysl x30, #1, c2, c3, #4
      {0x9e, 0x23, 0x29, 0xd5},
      di::opcode_test(aarch64_op_sysl, "sysl #1, #2, #3, #4, x30"), // WRONG: should be 'sysl x30, #1, c2, c3, #4'
      di::register_rw_test {
        reg_set{},
        reg_set{x30},
      },
    },
  };
}
