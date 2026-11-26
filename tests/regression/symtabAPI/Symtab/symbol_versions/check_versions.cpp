/*
 * Symtab::addSymbol(newSym, referringSymbol) gives newSym the first version of
 * referringSymbol, as the rewriter does for each reference to a symbol in
 * another library. Check that newSym still has that version after addSymbol
 * returns.
 *
 * This program is its own input: it defines dyninst_versioned_function, which
 * versioned.map gives the version DYNINST_TEST_1.0, and exports it.
 */
#include "Symbol.h"
#include "Symtab.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

extern "C" int dyninst_versioned_function(int x) { return x + 1; }

namespace st = Dyninst::SymtabAPI;

namespace {

char const *const versioned_name = "dyninst_versioned_function";
std::string const expected_version = "DYNINST_TEST_1.0";

// Fill the stack below the caller with 0xa5.
//
// A broken addSymbol leaves newSym pointing at a vector that lived in
// addSymbol's own stack frame. That memory is dead once addSymbol returns, but
// until something else uses it the old bytes are still there, so reading
// through the pointer can still find the right version and the bug goes
// unseen. Calling this right after addSymbol reuses that part of the stack and
// overwrites it. A dangling pointer then reads garbage (the vector's begin and
// end both become 0xa5a5..., an empty list) instead of the old contents.
__attribute__((noinline)) void clobber_stack() {
  volatile char junk[16 * 1024];
  for (auto &c : junk) {
    c = static_cast<char>(0xa5);
  }
}

}

int main(int, char **argv) {
  st::Symtab *self = nullptr;
  if (!st::Symtab::openFile(self, argv[0])) {
    std::fprintf(stderr, "cannot open %s\n", argv[0]);
    return EXIT_FAILURE;
  }

  std::vector<st::Symbol *> candidates;
  self->findSymbol(candidates, versioned_name, st::Symbol::ST_UNKNOWN,
                   st::mangledName);
  st::Symbol *referring = nullptr;
  for (auto *s : candidates) {
    if (s->isInDynSymtab()) {
      referring = s;
      break;
    }
  }
  if (!referring) {
    std::fprintf(stderr, "no dynamic %s in %s\n", versioned_name, argv[0]);
    return EXIT_FAILURE;
  }
  // The input must be what versioned.map made it, or the check below means
  // nothing.
  if (referring->getVersions() != std::vector<std::string>{expected_version}) {
    std::fprintf(stderr, "%s does not have exactly the version %s\n",
                 versioned_name, expected_version.c_str());
    return EXIT_FAILURE;
  }

  // The new symbol gets a name of its own, so that it is not merged with the
  // referring symbol in this same symbol table.
  auto *sym = new st::Symbol(std::string(versioned_name) + "_ref",
                             referring->getType(), st::Symbol::SL_GLOBAL,
                             st::Symbol::SV_DEFAULT, 0, self->getDefaultModule(),
                             nullptr, referring->getSize(), true, false);
  if (!self->addSymbol(sym, referring)) {
    std::fprintf(stderr, "addSymbol failed\n");
    return EXIT_FAILURE;
  }
  clobber_stack();

  auto const &versions = sym->getVersions();
  if (versions.size() != 1 || versions[0] != expected_version) {
    std::fprintf(stderr, "expected version %s, got %zu entries\n",
                 expected_version.c_str(), versions.size());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
