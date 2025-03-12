#include "BPatch_enums.h"
#include "dyninstAPI/src/debug.h"
#include "image.h"
#include "Symtab.h"

#include <array>
#include <iostream>
#include <vector>

bool test_findMain(const char* filename, Dyninst::Address symtab_main_addr) {
  std::cout << "Parsing image for " << filename << "\n";
  fileDescriptor desc{filename, 0, 0};
  constexpr auto analysisMode = BPatch_normalMode;
  constexpr bool parseGaps = false;
  image* img = image::parseImage(desc, analysisMode, parseGaps);

  if(!img) {
    std::cerr << "failed to parse image for '" << filename << "\n";
    return false;
  }

  const auto image_main_addr = img->getAddressOfMain();

  std::cout << "findMain returned 0x" << std::hex << image_main_addr << "\n";

  if(image_main_addr != symtab_main_addr) {
    std::cerr << "Addresses don't match: Symtab=0x" << std::hex << symtab_main_addr << ", image=0x" << image_main_addr
              << "\n";
    return false;
  }

  return true;
}

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cerr << "Usage: " << argv[0] << " file_with_syms file_without_syms\n";
    return EXIT_FAILURE;
  }

  namespace st = Dyninst::SymtabAPI;

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

  std::cout << "Testing findMain with symbols...\n";
  if(!test_findMain(argv[1], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  std::cout << "Testing findMain without symbols...\n";
  if(!test_findMain(argv[2], symtab_main_addr)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
