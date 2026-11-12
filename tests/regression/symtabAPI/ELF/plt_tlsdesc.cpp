// Checks an aarch64 library whose .rela.plt holds a TLSDESC relocation beside
// an ordinary JUMP_SLOT (test_binaries/symtabAPI/plt_stubs/aarch64/
// libtlsdesc.so).
//
// The TLSDESC relocation has no PLT stub.  SymtabAPI must leave it out of the
// function binding table, but the rewriter must still write it back: a
// rewritten library that lost it failed to load (test1_22 on aarch64).
//
//   plt_tlsdesc <library> <stub name> <stub address> <no-stub name> <output>
//
// The library is read and rewritten, never run, so any host can check it.

#include "Region.h"
#include "Symtab.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

namespace {

// The bytes of .rela.plt.  Summed over every region of that name, since the
// rewriter may write the relocations in more than one section.
unsigned long rela_plt_size(st::Symtab *symtab)
{
  std::vector<st::Region *> regions;
  symtab->getAllRegions(regions);
  unsigned long size{0};
  for (auto *r : regions) {
    if (r->getRegionName() == ".rela.plt") {
      size += r->getDiskSize();
    }
  }
  return size;
}

// Where .plt starts, or 0 if there is none.
Dyninst::Offset plt_addr(st::Symtab *symtab)
{
  st::Region *plt{};
  if (!symtab->findRegion(plt, ".plt")) {
    return 0;
  }
  return plt->getMemOffset();
}

// The number of failures in the binding table: the stub must be there at its
// address, and the relocation without a stub must not be there at all.
int check_bindings(st::Symtab *symtab, std::string const &what,
                   std::string const &stub, Dyninst::Offset stub_addr,
                   std::string const &no_stub)
{
  std::vector<st::relocationEntry> fbt;
  symtab->getFuncBindingTable(fbt);
  int failures{0};
  bool found{false};
  for (auto const &r : fbt) {
    if (r.name() == stub) {
      found = true;
      if (r.target_addr() != stub_addr) {
        std::cerr << what << ": " << stub << " at 0x" << std::hex
                  << r.target_addr() << ", expected 0x" << stub_addr
                  << std::dec << '\n';
        failures++;
      }
    }
    if (r.name() == no_stub) {
      std::cerr << what << ": " << no_stub << " is in the binding table at 0x"
                << std::hex << r.target_addr() << std::dec
                << "; it has no stub\n";
      failures++;
    }
  }
  if (!found) {
    std::cerr << what << ": " << stub << " is not in the binding table\n";
    failures++;
  }
  return failures;
}

}

int main(int argc, char **argv)
{
  if (argc != 6) {
    std::cerr << "Usage: " << argv[0]
              << " <library> <stub name> <stub address> <no-stub name>"
                 " <output>\n";
    return EXIT_FAILURE;
  }
  std::string const stub{argv[2]};
  Dyninst::Offset const stub_addr{std::strtoul(argv[3], nullptr, 0)};
  std::string const no_stub{argv[4]};

  st::Symtab *in{};
  if (!st::Symtab::openFile(in, argv[1])) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  int failures{check_bindings(in, argv[1], stub, stub_addr, no_stub)};

  unsigned long const in_size{rela_plt_size(in)};
  if (in_size == 0) {
    std::cerr << argv[1] << ": no .rela.plt\n";
    return EXIT_FAILURE;
  }

  if (!in->emit(argv[5])) {
    std::cerr << "Unable to write '" << argv[5] << "'\n";
    return EXIT_FAILURE;
  }
  st::Symtab *out{};
  if (!st::Symtab::openFile(out, argv[5])) {
    std::cerr << "Unable to open the rewritten '" << argv[5] << "'\n";
    return EXIT_FAILURE;
  }

  // Every relocation must survive the rewrite, the one without a stub too.
  unsigned long const out_size{rela_plt_size(out)};
  if (out_size != in_size) {
    std::cerr << argv[5] << ": .rela.plt is " << out_size
              << " bytes, the original's " << in_size << '\n';
    failures++;
  }
  // The rewrite may move .plt; the stub stays at the same place in it.
  Dyninst::Offset const out_stub_addr{stub_addr - plt_addr(in) + plt_addr(out)};
  failures += check_bindings(out, argv[5], stub, out_stub_addr, no_stub);

  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
