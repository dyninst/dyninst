/*
 * insnCodeGen::modifyData for AArch64 ADR and ADRP.
 *
 * Each case relocates an ADR or ADRP to a codeGen placed at a given address and runs what
 * modifyData emitted through a small simulator of the instructions it may emit (ADR, ADRP,
 * ADD (immediate), MOVZ, MOVK).  The register must end up holding what the original
 * instruction computed: the target itself for ADR, the target's 4 KiB page for ADRP.
 * The code is placed above 4 GiB, so a page computation that drops the upper 32 bits shows.
 */
#include "codegen/codegen.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>

namespace {

  using Dyninst::Address;

  // The bits hi..lo of insn.
  uint64_t bits(uint32_t insn, unsigned hi, unsigned lo) {
    return (insn >> lo) & ((uint64_t{1} << (hi - lo + 1)) - 1);
  }

  int64_t sign_extend(uint64_t value, unsigned width) {
    if(value & (uint64_t{1} << (width - 1))) {
      return static_cast<int64_t>(value) - (int64_t{1} << width);
    }
    return static_cast<int64_t>(value);
  }

  // ADR <Xd>, <label>:   0 immlo(30..29) 10000 immhi(23..5) Rd(4..0)
  // ADRP <Xd>, <label>:  1 immlo(30..29) 10000 immhi(23..5) Rd(4..0)
  uint32_t encode_adr(bool adrp, unsigned rd) {
    return (adrp ? 1u << 31 : 0u) | (0x10u << 24) | rd;
  }

  // Run the emitted code, starting at pc, and return the value left in register rd.
  // Returns false if it emitted something the simulator does not know.
  bool simulate(uint32_t const *code, unsigned count, Address pc, unsigned rd, uint64_t &value) {
    uint64_t x[32] = {};
    for(unsigned i = 0; i < count; i++, pc += 4) {
      uint32_t const w = code[i];
      unsigned const d = bits(w, 4, 0);
      if((w & 0x1f000000u) == 0x10000000u) {
        // ADR / ADRP: imm = immhi:immlo, signed 21 bits
        int64_t const imm = sign_extend((bits(w, 23, 5) << 2) | bits(w, 30, 29), 21);
        if(bits(w, 31, 31)) {
          x[d] = (pc & ~Address{0xfff}) + static_cast<uint64_t>(imm * 4096);
        } else {
          x[d] = pc + static_cast<uint64_t>(imm);
        }
      } else if((w & 0xff800000u) == 0x91000000u) {
        // ADD (immediate), 64-bit: Xd = Xn + imm12 << (sh ? 12 : 0)
        uint64_t const imm = bits(w, 21, 10) << (bits(w, 22, 22) ? 12 : 0);
        x[d] = x[bits(w, 9, 5)] + imm;
      } else if((w & 0xff800000u) == 0xd2800000u) {
        // MOVZ, 64-bit: Xd = imm16 << (hw * 16)
        x[d] = bits(w, 20, 5) << (16 * bits(w, 22, 21));
      } else if((w & 0xff800000u) == 0xf2800000u) {
        // MOVK, 64-bit: replace the 16 bits at hw * 16
        unsigned const sh = 16 * bits(w, 22, 21);
        x[d] = (x[d] & ~(uint64_t{0xffff} << sh)) | (bits(w, 20, 5) << sh);
      } else {
        std::printf("    unexpected instruction 0x%08" PRIx32 "\n", w);
        return false;
      }
    }
    value = x[rd];
    return true;
  }

  struct test_case {
    char const *name;
    bool adrp;
    Address at;      // where the relocated instruction is placed
    Address target;  // what the original instruction computed
  };

  constexpr Address four_gib = Address{1} << 32;
  constexpr Address mib = Address{1} << 20;
  constexpr Address page = 4096;
  constexpr Address base = four_gib + 0x10;  // just above 4 GiB

  test_case const cases[] = {
    // ADRP, in range
    {"ADRP across 4 GiB, one page back", true, base, four_gib - page},
    {"ADRP 2^20 - 1 pages forward", true, base, base + (mib - 1) * page},
    {"ADRP exactly 2^20 pages back", true, base, base - mib * page},
    // ADRP, out of range
    {"ADRP exactly 2^20 pages forward", true, base, base + mib * page},
    {"ADRP 8 GiB forward", true, base, base + 2 * four_gib},
    // ADR, in range
    {"ADR 0x1000 bytes forward", false, base, base + 0x1000},
    {"ADR exactly 2^20 bytes back", false, base, base - mib},
    // ADR, out of range
    {"ADR exactly 2^20 bytes forward", false, base, base + mib},
    {"ADR 2 MiB forward, page offset 0xf123", false, base, base + 2 * mib + 0xf123 - 0x10},
    {"ADR 6 GiB forward", false, base, base + (Address{6} << 30)},
  };

}

int main() {
  bool failed = false;
  unsigned const rd = 7;

  for(auto const &c : cases) {
    codeGen gen(128);
    gen.setAddr(c.at);

    NS_aarch64::instruction insn(encode_adr(c.adrp, rd));
    insnCodeGen::modifyData(c.target, insn, gen);

    uint64_t const expected = c.adrp ? (c.target & ~Address{0xfff}) : c.target;
    uint64_t got = 0;
    unsigned const count = gen.used() / 4;
    bool ok = simulate(static_cast<uint32_t const *>(gen.start_ptr()), count, c.at, rd, got);
    ok = ok && got == expected;
    std::printf("%-40s %u insn(s)  x%u = 0x%" PRIx64 "  expected 0x%" PRIx64 "  %s\n", c.name,
                count, rd, got, expected, ok ? "OK" : "FAILED");
    failed |= !ok;
  }
  return failed ? 1 : 0;
}
