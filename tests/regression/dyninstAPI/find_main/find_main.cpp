#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/find_main.h"
#include "Symtab.h"

#include <array>
#include <iostream>
#include <vector>

namespace st = Dyninst::SymtabAPI;
namespace dd = Dyninst::DyninstAPI;

bool test_find_main(char const* filename, Dyninst::Address symtab_main_addr) {
  std::cout << "Finding 'main' for " << filename << "\n";

  st::Symtab* symtab{};
  if(!st::Symtab::openFile(symtab, filename)) {
    std::cerr << "Failed to open '" << filename << "'\n";
    return false;
  }

  auto main_addr = dd::find_main(symtab);

  std::cout << "find_main returned 0x" << std::hex << main_addr << "\n";

  if(main_addr != symtab_main_addr) {
    std::cerr << "Addresses don't match: Symtab=0x" << std::hex << symtab_main_addr << ", image=0x" << main_addr
              << "\n";
    return false;
  }

  return true;
}

// get_missing_symbols must only add symbols the file does not already have,
// and must add 'main' to a file whose main symbol was stripped.
bool test_missing_symbols(char const* filename, Dyninst::Address symtab_main_addr) {
  std::cout << "Checking missing symbols for " << filename << "\n";

  st::Symtab* symtab{};
  if(!st::Symtab::openFile(symtab, filename)) {
    std::cerr << "Failed to open '" << filename << "'\n";
    return false;
  }

  std::vector<st::Function*> funcs;
  const bool had_main = symtab->findFunctionsByName(funcs, "main");

  bool ok = true;
  bool added_main = false;
  for(auto* s : dd::get_missing_symbols(symtab, dd::find_main(symtab))) {
    std::cout << "  adds '" << s->getMangledName() << "' at 0x" << std::hex << s->getOffset() << "\n";

    std::vector<st::Symbol*> existing;
    if(symtab->findSymbol(existing, s->getMangledName(), st::Symbol::ST_UNKNOWN, st::mangledName)) {
      std::cerr << "'" << s->getMangledName() << "' is already present\n";
      ok = false;
    }
    if(s->getMangledName() == "main") {
      added_main = true;
      if(s->getOffset() != symtab_main_addr) {
        std::cerr << "'main' added at 0x" << std::hex << s->getOffset() << ", expected 0x" << symtab_main_addr
                  << "\n";
        ok = false;
      }
    }
    delete s;
  }

  if(!had_main && !added_main) {
    std::cerr << "'main' was not added\n";
    ok = false;
  }

  return ok;
}

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cerr << "Usage: " << argv[0] << " file_with_syms file_without_syms\n";
    return EXIT_FAILURE;
  }

  st::Symtab* symtab{};
  if(!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "Failed to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  std::vector<st::Function*> funcs;
  symtab->findFunctionsByName(funcs, "main");

  if(funcs.empty()) {
    std::cerr << "Didn't find 'main' in Symtab\n";
    return EXIT_FAILURE;
  }

  if(funcs.size() > 1UL) {
    std::cerr << "Found more than one 'main' in Symtab: ";
    for(auto* f : funcs) {
      std::cerr << f->getName() << ", ";
    }
    std::cerr << "\n";
    return EXIT_FAILURE;
  }

  const auto symtab_main_addr = funcs[0]->getFirstSymbol()->getOffset();

  std::cout << "Symtab returned 0x" << std::hex << symtab_main_addr << "\n";

  std::cout << "Testing find_main with symbols...\n";
  if(!test_find_main(argv[1], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  std::cout << "Testing find_main without symbols...\n";
  if(!test_find_main(argv[2], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  std::cout << "Testing get_missing_symbols with symbols...\n";
  if(!test_missing_symbols(argv[1], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  std::cout << "Testing get_missing_symbols without symbols...\n";
  if(!test_missing_symbols(argv[2], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
