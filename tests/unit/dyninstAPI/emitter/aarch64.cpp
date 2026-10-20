/*
 * insnCodeGen::modifyData for AArch64 ADR, ADRP and LDR (literal).
 *
 * Each case relocates an ADR, ADRP or LDR (literal) to a codeGen placed at a given address
 * and runs what modifyData emitted through a small simulator of the instructions it may emit
 * (ADR, ADRP, ADD (immediate), MOVZ, MOVK, LDR (literal)).  The register must end up holding
 * what the original instruction computed: the target itself for ADR, the target's 4 KiB page
 * for ADRP.  For LDR (literal) the simulator has no memory to read, so it records the address
 * the load reads from instead, which must be the literal.  The code is placed above 4 GiB, so
 * a page computation that drops the upper 32 bits shows.
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
  uint32_t encode_adr(bool is_adrp, unsigned rd) {
    return (is_adrp ? 1u << 31 : 0u) | (0x10u << 24) | rd;
  }

  // LDR <Xt>, <label>:  01 011 0 00 imm19(23..5) Rt(4..0)
  uint32_t encode_ldr_literal(unsigned rt) {
    return 0x58000000u | rt;
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
      } else if((w & 0xff000000u) == 0x58000000u) {
        // LDR (literal), 64-bit: Xt = [pc + imm19 * 4]; record the address it reads
        x[d] = pc + static_cast<uint64_t>(sign_extend(bits(w, 23, 5), 19) * 4);
      } else {
        std::printf("    unexpected instruction 0x%08" PRIx32 "\n", w);
        return false;
      }
    }
    value = x[rd];
    return true;
  }

  enum kind_t { adr, adrp, ldr };

  struct test_case {
    char const *name;
    kind_t kind;
    Address at;      // where the relocated instruction is placed
    Address target;  // what the original instruction computed
    unsigned insns;  // instructions modifyData must emit; 0 = not checked
  };

  constexpr Address four_gib = Address{1} << 32;
  constexpr Address mib = Address{1} << 20;
  constexpr Address page = 4096;
  constexpr Address base = four_gib + 0x10;  // just above 4 GiB

  test_case const cases[] = {
    // ADRP, in range
    {"ADRP across 4 GiB, one page back", adrp, base, four_gib - page, 1},
    {"ADRP 2^20 - 1 pages forward", adrp, base, base + (mib - 1) * page, 1},
    {"ADRP exactly 2^20 pages back", adrp, base, base - mib * page, 1},
    // ADRP, out of range
    {"ADRP exactly 2^20 pages forward", adrp, base, base + mib * page, 0},
    {"ADRP 8 GiB forward", adrp, base, base + 2 * four_gib, 0},
    // ADR, in range
    {"ADR 0x1000 bytes forward", adr, base, base + 0x1000, 1},
    {"ADR exactly 2^20 bytes back", adr, base, base - mib, 1},
    // ADR, out of range
    {"ADR exactly 2^20 bytes forward", adr, base, base + mib, 2},
    {"ADR 2 MiB forward, page offset 0xf123", adr, base, base + 2 * mib + 0xf123 - 0x10, 2},
    {"ADR 2 MiB forward, page aligned", adr, base, (base & ~(page - 1)) + 2 * mib, 1},
    {"ADR 6 GiB forward", adr, base, base + (Address{6} << 30), 0},
    // LDR (literal), in range: the literal is pc - 2^20 .. pc + 2^20 - 4.  A literal out of
    // range is loaded through a scratch register, which needs a register space this codeGen
    // does not have, so that path is not tested here.
    {"LDR literal 0x100 forward", ldr, base, base + 0x100, 1},
    {"LDR literal 0x100 back", ldr, base, base - 0x100, 1},
    {"LDR literal 2^20 - 4 forward", ldr, base, base + mib - 4, 1},
    {"LDR literal exactly 2^20 back", ldr, base, base - mib, 1},
  };

}

int main() {
  bool failed = false;
  unsigned const rd = 7;

  for(auto const &c : cases) {
    codeGen gen(128);
    gen.setAddr(c.at);

    NS_aarch64::instruction insn(c.kind == ldr ? encode_ldr_literal(rd)
                                               : encode_adr(c.kind == adrp, rd));
    insnCodeGen::modifyData(c.target, insn, gen);

    uint64_t const expected = c.kind == adrp ? (c.target & ~Address{0xfff}) : c.target;
    uint64_t got = 0;
    unsigned const count = gen.used() / 4;
    bool ok = simulate(static_cast<uint32_t const *>(gen.start_ptr()), count, c.at, rd, got);
    ok = ok && got == expected && (c.insns == 0 || count == c.insns);
    std::printf("%-40s %u insn(s)  x%u = 0x%" PRIx64 "  expected 0x%" PRIx64 "  %s\n", c.name,
                count, rd, got, expected, ok ? "OK" : "FAILED");
    failed |= !ok;
  }
  return failed ? 1 : 0;
}
