// noreturn_check <binary> <mangled-names-file> <demangled-names-file>
//
// Registers the non-returning functions listed in the two files with
// BPatch::addNonReturningMangledName and BPatch::addNonReturningDemangledName,
// parses <binary>, and prints how many functions came out non-returning. With
// NORETURN_LIST set in the environment it also lists them. See README.md for
// the file format.

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "CFG.h"
#include "name_file.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace pa = Dyninst::ParseAPI;

int main(int argc, char **argv) {
  if(argc != 4) {
    std::fprintf(stderr, "usage: %s <binary> <mangled-names-file> <demangled-names-file>\n", argv[0]);
    return EXIT_FAILURE;
  }
  BPatch bpatch;
  int const numMangled = noreturn_tools::readNames(
      argv[2], [&](std::string const &name) { return bpatch.addNonReturningMangledName(name); });
  int const numDemangled = noreturn_tools::readNames(
      argv[3], [&](std::string const &name) { return bpatch.addNonReturningDemangledName(name); });
  if(numMangled < 0 || numDemangled < 0) {
    return EXIT_FAILURE;
  }
  std::printf("registered %d mangled, %d demangled names\n", numMangled, numDemangled);

  BPatch_binaryEdit *binary = bpatch.openBinary(argv[1], false);
  if(!binary) {
    std::fprintf(stderr, "cannot open %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Parses the whole binary.
  std::vector<BPatch_function *> funcs;
  binary->getImage()->getProcedures(funcs, true);

  bool const list = std::getenv("NORETURN_LIST") != nullptr;
  int numNoreturn{0};
  for(auto *f : funcs) {
    pa::Function const *pf = pa::convert(f);
    if(pf && pf->retstatus() == pa::NORETURN) {
      ++numNoreturn;
      if(list) {
        std::printf("NORETURN %s\n", f->getMangledName().c_str());
      }
    }
  }
  std::printf("functions %zu  non-returning %d\n", funcs.size(), numNoreturn);
  return EXIT_SUCCESS;
}
