/*
 * The Power instruction predicates that ProcControl's emulated single-step relies on: load-and-reserve and
 * store-conditional of every width (the LL/SC sequence boundaries), and the branches to LR, CTR and TAR
 * (exits whose target is not in the instruction).
 * Instruction words were produced by GNU as (-a64 -mpower9) and read back with objdump.
 */
#include "common/src/arch-power.h"
#include <cstdio>
#include <cstdlib>

using NS_power::instruction;

namespace {

struct atomic_case { const char *text; unsigned raw; bool is_load_reserve; bool is_store_conditional; };

// clang-format off
const atomic_case atomic_cases[] = {
  {"lwarx 3,0,4",      0x7c602028, true,  false},
  {"lwarx 3,5,4",      0x7c652028, true,  false},
  {"lwarx 3,0,4,1",    0x7c602029, true,  false},   // EH bit
  {"ldarx 3,0,4",      0x7c6020a8, true,  false},
  {"ldarx 3,0,4,1",    0x7c6020a9, true,  false},
  {"lbarx 3,0,4",      0x7c602068, true,  false},
  {"lharx 3,0,4",      0x7c6020e8, true,  false},
  {"lqarx 6,0,4",      0x7cc02228, true,  false},
  {"stwcx. 3,0,4",     0x7c60212d, false, true},
  {"stwcx. 3,5,4",     0x7c65212d, false, true},
  {"stdcx. 3,0,4",     0x7c6021ad, false, true},
  {"stbcx. 3,0,4",     0x7c60256d, false, true},
  {"sthcx. 3,0,4",     0x7c6025ad, false, true},
  {"stqcx. 6,0,4",     0x7cc0216d, false, true},
  // plain loads and stores, X-form included, are neither
  {"lwz 3,0(4)",       0x80640000, false, false},
  {"stw 3,0(4)",       0x90640000, false, false},
  {"ld 3,0(4)",        0xe8640000, false, false},
  {"std 3,0(4)",       0xf8640000, false, false},
  {"lwzx 3,0,4",       0x7c60202e, false, false},
  {"stwx 3,0,4",       0x7c60212e, false, false},
  {"lwax 3,0,4",       0x7c6022aa, false, false},
  {"ldx 3,5,4",        0x7c65202a, false, false},
  {"sync",             0x7c0004ac, false, false},
  {"lwsync",           0x7c2004ac, false, false},
  {"isync",            0x4c00012c, false, false},
  {"cmpw 3,4",         0x7c032000, false, false},
  {"addi 3,3,1",       0x38630001, false, false},
};

struct branch_case { const char *text; unsigned raw; bool to_lr; bool to_ctr; bool to_tar; };

const branch_case branch_cases[] = {
  {"blr",              0x4e800020, true,  false, false},
  {"blrl",             0x4e800021, true,  false, false},   // LK bit
  {"beqlr",            0x4d820020, true,  false, false},
  {"bnelr",            0x4c820020, true,  false, false},
  {"bdnzlr",           0x4e000020, true,  false, false},
  {"bctr",             0x4e800420, false, true,  false},
  {"bctrl",            0x4e800421, false, true,  false},
  {"bnectr",           0x4c820420, false, true,  false},
  {"bctar 20,0",       0x4e800460, false, false, true},
  {"bctarl 20,0",      0x4e800461, false, false, true},
  // branches with the target in the instruction, moves to/from LR and CTR, and other
  // opcode-19 instructions are none of them
  {"b .+8",            0x48000008, false, false, false},
  {"bl .+8",           0x48000009, false, false, false},
  {"beq .+8",          0x41820008, false, false, false},
  {"mtlr 3",           0x7c6803a6, false, false, false},
  {"mfctr 3",          0x7c6902a6, false, false, false},
  {"isync",            0x4c00012c, false, false, false},
  {"crclr 0",          0x4c000182, false, false, false},
  {"rfid",             0x4c000024, false, false, false},
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
    check(insn.isAtomicLoad() == c.is_load_reserve, "isAtomicLoad", c.text);
    check(insn.isAtomicStore() == c.is_store_conditional, "isAtomicStore", c.text);
  }
  for (const auto &c : branch_cases) {
    instruction insn(c.raw);
    check(insn.isBranchToLR() == c.to_lr, "isBranchToLR", c.text);
    check(insn.isBranchToCTR() == c.to_ctr, "isBranchToCTR", c.text);
    check(insn.isBranchToTAR() == c.to_tar, "isBranchToTAR", c.text);
  }
  if (failures) {
    std::printf("%d failure(s)\n", failures);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
