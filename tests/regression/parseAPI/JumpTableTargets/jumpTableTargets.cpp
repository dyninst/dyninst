// Parses a binary and checks the targets of one indirect jump: the indirect edges out of
// the block that ends at the given address must be exactly the given addresses.
//
// Usage: jumpTableTargets <file> <address of the jump> <target>...  (addresses in hex)

#include <cstdlib>
#include <iostream>
#include <set>

#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"

namespace pa = Dyninst::ParseAPI;

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Usage " << argv[0] << ": file jump-address target...\n";
    return EXIT_FAILURE;
  }
  Dyninst::Address const jump = std::strtoull(argv[2], nullptr, 16);
  std::set<Dyninst::Address> expected;
  for (int i = 3; i < argc; ++i) {
    expected.insert(std::strtoull(argv[i], nullptr, 16));
  }

  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};
  co.parse();

  bool found = false;
  bool ok = true;
  for (auto *f : co.funcs()) {
    for (auto *b : f->blocks()) {
      if (b->last() != jump) {
        continue;
      }
      found = true;
      std::set<Dyninst::Address> got;
      for (auto *e : b->targets()) {
        if (e->type() == pa::INDIRECT && !e->sinkEdge()) {
          got.insert(e->trg()->start());
        }
      }
      if (got != expected) {
        ok = false;
        std::cerr << "jump at 0x" << std::hex << jump << " in " << f->name() << ": " << std::dec << got.size()
                  << " targets, expected " << expected.size() << std::hex << '\n';
        for (auto a : expected) {
          if (!got.count(a)) std::cerr << "  missing 0x" << a << '\n';
        }
        for (auto a : got) {
          if (!expected.count(a)) std::cerr << "  extra 0x" << a << '\n';
        }
      }
    }
  }
  if (!found) {
    std::cerr << "no block ends at 0x" << std::hex << jump << '\n';
    return EXIT_FAILURE;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
