#include "Symtab.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " file\n";
    return EXIT_FAILURE;
  }

  namespace st = Dyninst::SymtabAPI;
  st::Symtab* s{};
  if(!st::Symtab::openFile(s, argv[1])) {
    std::cerr << "Unable to open file '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  if(!s->isExecutable()) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
