/*
   liveness_dump — like the `disassemble` example, but at every instruction it
   also queries Dyninst's LivenessAnalyzer for the live-IN (Before) and live-OUT
   (After) register sets and prints them alongside the disassembly.

   Purpose: validate/debug AMDGPU (gfx908) register liveness at instruction
   granularity — the same information dyninst's instrumentation uses to decide
   which registers a spliced call must preserve. Any register reported dead here
   but genuinely live is a liveness bug that would corrupt a liveness-driven spill.

   Usage:  liveness_dump <code-object> [function-name]
           (default function: all functions)
*/
#include "CodeObject.h"
#include "CodeSource.h"
#include "CFG.h"
#include "Symtab.h"
#include "Instruction.h"
#include "liveness.h"
#include "Location.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

// Build the set of registers to probe for gfx908: s0..s103, vcc, v0..v63,
// exec. (s_i / v_i are consecutive MachRegister vals from the s0 / v0 base, the
// same construction the decoder uses via makeAmdgpuRegID.)
static vector<pair<string, MachRegister>> gfx908ProbeRegs() {
  vector<pair<string, MachRegister>> regs;
  for (int i = 0; i <= 103; i++)
    regs.push_back({"s" + to_string(i), MachRegister(amdgpu_gfx908::s0.val() + i)});
  regs.push_back({"vcc_lo", amdgpu_gfx908::vcc_lo});
  regs.push_back({"vcc_hi", amdgpu_gfx908::vcc_hi});
  for (int i = 0; i <= 63; i++)
    regs.push_back({"v" + to_string(i), MachRegister(amdgpu_gfx908::v0.val() + i)});
  regs.push_back({"exec_lo", amdgpu_gfx908::exec_lo});
  regs.push_back({"exec_hi", amdgpu_gfx908::exec_hi});
  return regs;
}

static string liveSet(LivenessAnalyzer &la, const Location &loc,
                      LivenessAnalyzer::Type when,
                      const vector<pair<string, MachRegister>> &regs) {
  // Whole-bitArray query (the per-register query asserts on regs not in the
  // index map); name bits via getIndex, skipping registers not in the map.
  bitArray live;
  if (!la.query(loc, when, live))
    return " <query-failed>";
  string out;
  for (auto &r : regs) {
    int idx = la.getIndex(r.second);
    if (idx >= 0 && idx < (int)live.size() && live.test(idx))
      out += " " + r.first;
  }
  return out;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <code-object> [function-name]\n", argv[0]);
    return -1;
  }
  const char *binaryPath = argv[1];
  const char *target = (argc > 2) ? argv[2] : nullptr;

  SymtabAPI::Symtab *symTab = nullptr;
  string binaryPathStr(binaryPath);
  if (!SymtabAPI::Symtab::openFile(symTab, binaryPathStr)) {
    cerr << "error: file can not be parsed\n";
    return -1;
  }

  SymtabCodeSource *sts = new SymtabCodeSource((char *)binaryPath);
  CodeObject *co = new CodeObject(sts);
  co->parse();

  const CodeObject::funclist &all = co->funcs();
  if (all.empty()) {
    cerr << "error: no functions in file\n";
    return -1;
  }

  auto regs = gfx908ProbeRegs();
  LivenessAnalyzer la(sts->getArch(), sts->getAddressWidth());

  // Diagnostic: for key registers show the index used by liveness and the
  // index of getBaseRegister() — a mismatch (or -1 base) means calcRWSets sets
  // a bit that queries never read back (silent liveness loss).
  if (getenv("DUMP_REG_INDEX")) {
    auto show = [&](const char *nm, MachRegister r) {
      MachRegister base = r.getBaseRegister();
      printf("  %-8s idx=%d  base=%s baseIdx=%d\n", nm, la.getIndex(r),
             base.name().c_str(), la.getIndex(base));
    };
    show("s4", MachRegister(amdgpu_gfx908::s0.val() + 4));
    show("vcc_lo", amdgpu_gfx908::vcc_lo);
    show("vcc_hi", amdgpu_gfx908::vcc_hi);
    show("v0", amdgpu_gfx908::v0);
    show("exec_lo", amdgpu_gfx908::exec_lo);
  }

  for (Function *f : all) {
    if (target && f->name() != target)
      continue;
    if (f->blocks().empty())
      continue;

    cout << "\n" << hex << setfill('0') << setw(2 * sts->getAddressWidth())
         << f->addr() << " <" << f->name() << ">:\n";

    la.analyze(f);

    // Blocks in address order for readable output.
    vector<Block *> blocks(f->blocks().begin(), f->blocks().end());
    sort(blocks.begin(), blocks.end(),
         [](Block *a, Block *b) { return a->start() < b->start(); });

    for (Block *b : blocks) {
      Block::Insns insns;
      b->getInsns(insns);
      for (auto &ip : insns) {
        Offset off = ip.first;
        Instruction insn = ip.second;
        Location loc(f, b, off, insn);
        string in = liveSet(la, loc, LivenessAnalyzer::Before, regs);
        string out = liveSet(la, loc, LivenessAnalyzer::After, regs);
        cout << "  " << hex << setfill(' ') << setw(8) << off << ":  "
             << setw(44) << left << insn.format() << right
             << "  IN:{" << in << " } OUT:{" << out << " }\n";
      }
    }
  }
  return 0;
}
