// Checks that dyninstAPI names a PLT stub the way the linker labelled it.
//
// The symtabAPI and parseAPI tests over these same binaries read any
// architecture, because those layers only read.  This one opens the binary
// through BPatch, which needs an address space and a code generator for the
// target, so it runs only against the binary built for the architecture it is
// running on; the rest are registered and reported skipped.
//
// What it adds over the other two is the layer: a stub reaches BPatch as a
// PLTFunction made by DynCFGFactory::mkfunc from the relocation Symtab holds,
// by way of the linkage table, and nothing below here exercises that.
//
//   plt_stubs_image <binary> <linkage file>

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <binary> <linkage file>\n";
    return EXIT_FAILURE;
  }

  std::map<Dyninst::Address, std::string> expected;
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
      if (is >> addr >> name) {
        expected[std::stoull(addr, nullptr, 16)] = name;
      }
    }
  }
  if (expected.empty()) {
    std::cerr << "'" << argv[2] << "' names no stubs\n";
    return EXIT_FAILURE;
  }

  BPatch bpatch;
  BPatch_binaryEdit *be = bpatch.openBinary(argv[1]);
  if (!be) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *image = be->getImage();

  int failures = 0;
  for (auto const &want : expected) {
    BPatch_function *f = image->findFunction(want.first);
    if (!f) {
      std::cerr << "0x" << std::hex << want.first << std::dec << " ("
                << want.second << "): no function at the stub's address\n";
      failures++;
      continue;
    }
    // A stub whose relocation was not found keeps the name the parser invents
    // for code it reached without one, so the comparison catches that as well
    // as a wrong address.
    if (f->getName() != want.second) {
      std::cerr << "0x" << std::hex << want.first << std::dec << ": expected "
                << want.second << ", found " << f->getName() << '\n';
      failures++;
    }
  }

  if (failures) {
    std::cerr << argv[1] << ": " << failures << " of " << expected.size()
              << " stubs wrong\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
