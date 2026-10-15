#include "Region.h"
#include "Symtab.h"

#include <cstdlib>
#include <iostream>
#include <string>

// See test_binaries/dyninstAPI/findMain/ppc64le/stat.nopie.stripped
//
// The TLS NOBITS section .tbss occupies no address space in the image, and
// the linker places other sections inside its address range:
//
//   .tdata        0x100ec8e8  0x20
//   .tbss         0x100ec908  0x48  (NOBITS, TLS)
//   .ctors        0x100ec908  0x10  same start as .tbss, sorts before it
//   .dtors        0x100ec918  0x10  starts inside .tbss
//   .data.rel.ro  0x100ec928        starts inside .tbss
//
// findEnclosingRegion must return the section that holds the bytes at each
// address, never .tbss. findMain reads _start's start_addresses structure at
// 0x100ec928; resolving it to .tbss dereferenced a null raw-data pointer.

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <binary path>\n";
    return EXIT_FAILURE;
  }

  namespace st = Dyninst::SymtabAPI;
  st::Symtab *symtab;
  if (!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  struct {
    Dyninst::Offset addr;
    char const *name;
  } const expected[] = {
      {0x100ec8e8, ".tdata"},
      {0x100ec908, ".ctors"},
      {0x100ec918, ".dtors"},
      {0x100ec928, ".data.rel.ro"},
  };

  bool ok = true;
  for (auto const &e : expected) {
    st::Region *reg = symtab->findEnclosingRegion(e.addr);
    std::string const found = reg ? reg->getRegionName() : "<none>";
    if (found != e.name) {
      std::cerr << "findEnclosingRegion(0x" << std::hex << e.addr << ") gave '"
                << found << "', expected '" << e.name << "'\n";
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
