// A PLT relocation's symbol must be the dynamic symbol its relocation names by index, not the
// first dynamic symbol with the same name.  The fixture defines foo@V1 (foo_v1) and foo@@V2
// (foo_v2) and calls foo through its PLT stub; that jump slot names foo@@V2.
#include "Symtab.h"
#include "Symbol.h"
#include "relocationEntry.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace st = Dyninst::SymtabAPI;

namespace {

Dyninst::Offset offsetOf(st::Symtab *obj, char const *name) {
  std::vector<st::Symbol *> syms;
  if (!obj->findSymbol(syms, name, st::Symbol::ST_FUNCTION) || syms.empty())
    return Dyninst::ADDRESS_INVALID;
  return syms.front()->getOffset();
}

}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }
  st::Symtab *obj{nullptr};
  if (!st::Symtab::openFile(obj, argv[1])) {
    std::cerr << "cannot open " << argv[1] << '\n';
    return EXIT_FAILURE;
  }
  Dyninst::Offset const v1{offsetOf(obj, "foo_v1")};
  Dyninst::Offset const v2{offsetOf(obj, "foo_v2")};
  if (v1 == Dyninst::ADDRESS_INVALID || v2 == Dyninst::ADDRESS_INVALID || v1 == v2) {
    std::cerr << "fixture symbols foo_v1/foo_v2 not found\n";
    return EXIT_FAILURE;
  }

  std::vector<st::relocationEntry> fbt;
  obj->getFuncBindingTable(fbt);
  int found{0};
  int bad{0};
  for (st::relocationEntry const &r : fbt) {
    if (r.name() != "foo")
      continue;
    ++found;
    st::Symbol const *const sym{r.getDynSym()};
    if (sym == nullptr) {
      std::cerr << "PLT relocation for foo has no symbol\n";
      ++bad;
    } else if (sym->getOffset() != v2) {
      std::cerr << std::hex << "PLT relocation for foo has the symbol at 0x" << sym->getOffset()
                << ", expected foo@@V2 at 0x" << v2 << " (foo@V1 is at 0x" << v1 << ")\n";
      ++bad;
    }
  }
  if (found != 1) {
    std::cerr << "expected 1 PLT relocation for foo, found " << found << '\n';
    ++bad;
  }
  return bad == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
