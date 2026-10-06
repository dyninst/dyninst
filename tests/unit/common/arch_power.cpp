/*
 * The Power instruction predicates that ProcControl's emulated single-step relies on: load-and-reserve and
 * store-conditional of every width (the LL/SC sequence boundaries), and the branch forms the scan follows.
 * Encodings were produced by GNU as (-a64 -mpower9 -mlittle) and read back with objdump.
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
// clang-format on

int failures = 0;
void check(bool ok, const char *what, const char *text) {
  if (!ok) { std::printf("FAIL: %s for '%s'\n", what, text); ++failures; }
}

}  // namespace

int main() {
  for (const auto &c : atomic_cases) {
    instruction insn(c.raw);
    check(insn.isAtomicLoad() == c.is_load_reserve, "isAtomicLoad", c.text);
    check(insn.isAtomicStore() == c.is_store_conditional, "isAtomicStore", c.text);
  }
  if (failures) { std::printf("%d failure(s)\n", failures); return EXIT_FAILURE; }
  return EXIT_SUCCESS;
}
