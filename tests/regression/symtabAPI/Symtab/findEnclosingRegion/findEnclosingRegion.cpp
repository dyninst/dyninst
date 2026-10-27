#include "Region.h"
#include "Symtab.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <memory>
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

namespace st = Dyninst::SymtabAPI;

namespace {

  bool check(st::Symtab *symtab, Dyninst::Offset addr, char const *name) {
    st::Region *reg = symtab->findEnclosingRegion(addr);
    std::string const found = reg ? reg->getRegionName() : "<none>";
    if (found == name) {
      return true;
    }
    std::cerr << "findEnclosingRegion(0x" << std::hex << addr << ") gave '" << found
              << "', expected '" << name << "'\n";
    return false;
  }

  struct region_spec {
    Dyninst::Offset addr;
    unsigned int size;
    char const *name;
    bool tbss;
  };

  // A Symtab holding only the given regions, so the binary search's path is
  // fixed: each layout below makes it land on the .tbss region first.
  std::unique_ptr<st::Symtab> make_symtab(std::initializer_list<region_spec> regions) {
    static char bytes[0x200];
    std::unique_ptr<st::Symtab> symtab(new st::Symtab());
    for (auto const &r : regions) {
      symtab->addRegion(r.addr, bytes, r.size, r.name,
                        r.tbss ? st::Region::RT_BSS : st::Region::RT_DATA, true, 8, r.tbss);
    }
    return symtab;
  }

  // The sections of stat.nopie.stripped that .tbss overlaps.
  bool check_binary(char const *path) {
    st::Symtab *symtab;
    if (!st::Symtab::openFile(symtab, path)) {
      std::cerr << "Unable to open '" << path << "'\n";
      return false;
    }
    bool ok = true;
    ok &= check(symtab, 0x100ec8e8, ".tdata");
    ok &= check(symtab, 0x100ec908, ".ctors");
    ok &= check(symtab, 0x100ec918, ".dtors");
    ok &= check(symtab, 0x100ec928, ".data.rel.ro");
    st::Symtab::closeSymtab(symtab);
    return ok;
  }

  // Layouts that linker output does not produce but addRegion can.
  bool check_add_region() {
    bool ok = true;

    // A region that starts before .tbss and extends into it.
    ok &= check(make_symtab({{0x1000, 0x180, "before", false},
                             {0x1100, 0x100, ".tbss", true},
                             {0x3000, 0x10, "after", false}}).get(),
                0x1140, "before");

    // Several regions inside .tbss: the smallest one wins, not the first one
    // in address order.
    ok &= check(make_symtab({{0x800, 0x10, "before", false},
                             {0x1100, 0x100, ".tbss", true},
                             {0x1100, 0x200, "outer", false},
                             {0x1180, 0x10, "inner", false}}).get(),
                0x1184, "inner");
    return ok;
  }

}

// With no arguments, check the addRegion layouts. With --binary, check
// stat.nopie.stripped at the given path, or report a skip when there is none
// (the test binaries are only available with DYNINST_ENABLE_TESTS=ALL).
int main(int argc, char **argv) {
  if (argc == 1) {
    return check_add_region() ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (std::string(argv[1]) == "--binary" && argc <= 3) {
    if (argc == 2) {
      std::cout << "skipping: needs the test binaries (DYNINST_ENABLE_TESTS=ALL)\n";
      return EXIT_SUCCESS;
    }
    return check_binary(argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  std::cerr << "Usage: " << argv[0] << " [--binary [<binary path>]]\n";
  return EXIT_FAILURE;
}
