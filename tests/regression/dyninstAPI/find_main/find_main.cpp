#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/find_main.h"
#include "Symtab.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
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
// must add 'main' to a file whose main symbol was stripped, and must place
// each symbol where the unstripped file has it.
bool test_missing_symbols(char const* filename, st::Symtab* reference) {
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
    const auto& name = s->getMangledName();
    std::cout << "  adds '" << name << "' at 0x" << std::hex << s->getOffset() << "\n";

    std::vector<st::Symbol*> existing;
    if(symtab->findSymbol(existing, name, st::Symbol::ST_UNKNOWN, st::mangledName)) {
      std::cerr << "'" << name << "' is already present\n";
      ok = false;
    }

    std::vector<st::Symbol*> expected;
    if(reference->findSymbol(expected, name, st::Symbol::ST_UNKNOWN, st::mangledName)) {
      auto same_offset = [s](st::Symbol* e) { return e->getOffset() == s->getOffset(); };
      if(std::none_of(expected.begin(), expected.end(), same_offset)) {
        std::cerr << "'" << name << "' added at 0x" << std::hex << s->getOffset() << ", expected 0x"
                  << expected[0]->getOffset() << "\n";
        ok = false;
      }
    }

    added_main = added_main || name == "main";
    delete s;
  }

  if(!had_main && !added_main) {
    std::cerr << "'main' was not added\n";
    ok = false;
  }

  return ok;
}

// Usage: find_main <file with symbols> <the same file stripped>
//        find_main --skip <reason>
// With --skip the test reports a skip (exit status 77) with the reason.
int main(int argc, char** argv) {
  if(argc == 3 && std::string{argv[1]} == "--skip") {
    std::cout << "skipping: " << argv[2] << "\n";
    return 77;
  }
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
  if(!test_missing_symbols(argv[1], symtab)) {
    return EXIT_FAILURE;
  }

  std::cout << "Testing get_missing_symbols without symbols...\n";
  if(!test_missing_symbols(argv[2], symtab)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
