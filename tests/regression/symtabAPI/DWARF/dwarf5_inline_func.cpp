#include "Symtab.h"

#include <cstdlib>
#include <iostream>

// See test_binaries/symtabAPI/DWARF/dwarf5_inline_func

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

  constexpr auto offset_of_main = 0x1130;
  auto inlined_target = "inl";

  st::FunctionBase *f{};
  symtab->getContainingInlinedFunction(offset_of_main, f);
  
  if (!f) {
    std::cerr << "No inlined function found in 'main' (0x: " << std::hex
              << offset_of_main << "\n";
    return EXIT_FAILURE;
  }

  if (f->getName() != inlined_target) {
    std::cerr << "Expected '" << inlined_target << "', but found '"
              << f->getName() << "'\n";
    return EXIT_FAILURE;
  }
}
