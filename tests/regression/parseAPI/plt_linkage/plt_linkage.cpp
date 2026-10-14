// Checks the external linkage table ParseAPI builds -- what names a call that
// leaves the object -- against where the linker says such a call lands.
//
// See test_binaries/symtabAPI/plt_stubs, where each binary has a .linkage file
// listing, for every function it imports, the address objdump labels
// <name@plt>.  That is the stub a call reaches: the entry in .plt.sec where a
// two-level PLT puts one, and the .plt entry otherwise.  It is not always the
// entry the relocation belongs to, which is what the .expected files beside
// them hold and what the symtabAPI test checks.
//
//   plt_linkage <binary> <linkage file>

#include "CodeSource.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <binary> <linkage file>\n";
    return EXIT_FAILURE;
  }

  std::map<Address, std::string> expected;
  {
    std::ifstream in(argv[2]);
    if (!in) {
      std::cerr << "Unable to read '" << argv[2] << "'\n";
      return EXIT_FAILURE;
    }
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty() || line[0] == '#') {
        continue;
      }
      std::istringstream is(line);
      std::string addr, name;
      if (!(is >> addr >> name)) {
        std::cerr << "Malformed line in '" << argv[2] << "': " << line << '\n';
        return EXIT_FAILURE;
      }
      expected[std::stoull(addr, nullptr, 16)] = name;
    }
  }
  if (expected.empty()) {
    std::cerr << "'" << argv[2] << "' names no stubs\n";
    return EXIT_FAILURE;
  }

  SymtabCodeSource cs(argv[1]);
  auto const &linkage = cs.linkage();

  int failures = 0;
  for (auto const &want : expected) {
    auto found = linkage.find(want.first);
    if (found == linkage.end()) {
      // The address a call goes to, with nothing to say what it calls.
      std::cerr << "0x" << std::hex << want.first << std::dec << " ("
                << want.second << "): not in the linkage table\n";
      failures++;
    } else if (found->second != want.second) {
      std::cerr << "0x" << std::hex << want.first << std::dec << ": expected "
                << want.second << ", found " << found->second << '\n';
      failures++;
    }
  }

  if (failures) {
    std::cerr << argv[1] << ": " << failures << " of " << expected.size()
              << " stubs unnamed or misnamed\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
