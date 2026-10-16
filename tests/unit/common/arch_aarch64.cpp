/*
 * The aarch64 instruction predicates that ProcControl's emulated single-step relies on:
 * exclusive loads and stores (the LL/SC sequence boundaries) and branch-register targets.
 * Encodings were produced by GNU as (-march=armv8.3-a) and read back with objdump, except the
 * unallocated op = 11 word, which no assembler emits.
 */
#include "common/src/arch-aarch64.h"
#include <cstdio>
#include <cstdlib>

using NS_aarch64::instruction;

namespace {

struct atomic_case {
  const char *text;
  unsigned raw;
  bool is_load_exclusive;
  bool is_store_exclusive;
};

// clang-format off
const atomic_case atomic_cases[] = {
  // exclusive register: the start and end of an LL/SC sequence
  {"ldxr w0, [x2]",            0x885f7c40, true,  false},
  {"ldaxr w0, [x2]",           0x885ffc40, true,  false},
  {"ldxr x0, [x2]",            0xc85f7c40, true,  false},
  {"ldaxr x0, [x2]",           0xc85ffc40, true,  false},
  {"ldxrb w0, [x2]",           0x085f7c40, true,  false},
  {"ldaxrh w0, [x2]",          0x485ffc40, true,  false},
  {"stxr w17, w1, [x2]",       0x88117c41, false, true},
  {"stlxr w17, w1, [x2]",      0x8811fc41, false, true},
  {"stxr w17, x1, [x2]",       0xc8117c41, false, true},
  {"stxrb w17, w1, [x2]",      0x08117c41, false, true},
  // exclusive pair
  {"ldxp x0, x1, [x2]",        0xc87f0440, true,  false},
  {"ldaxp w0, w1, [x2]",       0x887f8440, true,  false},
  {"stxp w17, x0, x1, [x2]",   0xc8310440, false, true},
  {"stlxp w17, w0, w1, [x2]",  0x88318440, false, true},
  // load/store ordered: acquire/release semantics but no exclusive monitor
  {"ldar w0, [x2]",            0x88dffc40, false, false},
  {"ldar x0, [x2]",            0xc8dffc40, false, false},
  {"ldlar w0, [x2]",           0x88df7c40, false, false},
  {"stlr w1, [x2]",            0x889ffc41, false, false},
  {"stlr x1, [x2]",            0xc89ffc41, false, false},
  // compare-and-swap (LSE): a single instruction, nothing to emulate
  {"casa w0, w1, [x2]",        0x88e07c41, false, false},
  {"cas w0, w1, [x2]",         0x88a07c41, false, false},
  {"casl x0, x1, [x2]",        0xc8a0fc41, false, false},
  {"casal w0, w1, [x2]",       0x88e0fc41, false, false},
  {"casp x0, x1, x2, x3, [x4]",   0x48207c82, false, false},
  {"caspa w0, w1, w2, w3, [x4]",  0x08607c82, false, false},
  // other loads, stores and LSE atomics
  {"ldr w0, [x2]",             0xb9400040, false, false},
  {"str w1, [x2]",             0xb9000041, false, false},
  {"swpa w0, w1, [x2]",        0xb8a08041, false, false},
  {"ldadd w0, w1, [x2]",       0xb8200041, false, false},
};

struct branch_case {
  const char *text;
  unsigned raw;
  bool is_branch_reg;
  unsigned target_reg;   // only meaningful when is_branch_reg
};

const branch_case branch_cases[] = {
  {"ret",      0xd65f03c0, true,  30},
  {"ret x14",  0xd65f01c0, true,  14},
  {"br x16",   0xd61f0200, true,  16},
  {"blr x17",  0xd63f0220, true,  17},
  {"br x3",    0xd61f0060, true,  3},
  {"br x30",   0xd61f03c0, true,  30},
  {"eret",     0xd69f03e0, false, 0},   // shares bits [31:25] with br/blr/ret; Rn field is 31
  {"drps",     0xd6bf03e0, false, 0},
  {"br xzr",   0xd61f03e0, false, 0},   // Rn = 31: a branch to 0, more likely literal-pool data
  {"ret xzr",  0xd65f03e0, false, 0},
  {"(op=11)",  0xd67f0000, false, 0},   // unallocated
  // pointer authentication (-mbranch-protection): the target is still Rn, or x30 for retaa/retab
  {"retaa",              0xd65f0bff, true,  30},
  {"retab",              0xd65f0fff, true,  30},
  {"braaz x16",          0xd61f0a1f, true,  16},
  {"braaz x0",           0xd61f081f, true,  0},
  {"brabz x3",           0xd61f0c7f, true,  3},
  {"blraaz x17",         0xd63f0a3f, true,  17},
  {"blrabz x30",         0xd63f0fdf, true,  30},
  {"braa x16, x17",      0xd71f0a11, true,  16},
  {"brab x3, sp",        0xd71f0c7f, true,  3},
  {"blraa x17, x16",     0xd73f0a30, true,  17},
  {"blrab x30, x1",      0xd73f0fc1, true,  30},
  {"braaz xzr",          0xd61f0bff, false, 0},
  {"eretaa",             0xd69f0bff, false, 0},
  {"eretab",             0xd69f0fff, false, 0},
  {"b .",      0x14000000, false, 0},
  {"bl .",     0x94000000, false, 0},
  {"cbz w0, .", 0x34000000, false, 0},
};
// clang-format on

int failures = 0;

void check(bool ok, const char *what, const char *text) {
  if (!ok) {
    std::printf("FAIL: %s for '%s'\n", what, text);
    ++failures;
  }
}

}  // namespace

int main() {
  for (const auto &c : atomic_cases) {
    instruction insn(c.raw);
    check(insn.isAtomicLoad() == c.is_load_exclusive, "isAtomicLoad", c.text);
    check(insn.isAtomicStore() == c.is_store_exclusive, "isAtomicStore", c.text);
  }
  for (const auto &c : branch_cases) {
    instruction insn(c.raw);
    check(insn.isBranchReg() == c.is_branch_reg, "isBranchReg", c.text);
    if (c.is_branch_reg) {
      check(insn.getBranchTargetReg() == c.target_reg, "getBranchTargetReg", c.text);
      check(insn.getTargetReg() == c.target_reg, "getTargetReg", c.text);
    }
  }
  if (failures) {
    std::printf("%d failure(s)\n", failures);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
