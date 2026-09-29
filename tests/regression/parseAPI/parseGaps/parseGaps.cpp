#include <cstdlib>
#include <iostream>

#include "CodeObject.h"
#include "Function.h"
#include "Instruction.h"
#include "Symtab.h"

namespace pa = Dyninst::ParseAPI;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }

  std::cout << "Parsing " << argv[1] << '\n';

  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};

  constexpr auto idiom = Dyninst::ParseAPI::GapParsingType::IdiomMatching;
  for (auto &reg : scs.regions()) {
    co.parseGaps(reg, idiom);
  }
}
