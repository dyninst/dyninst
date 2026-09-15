/*
 * Parse an unlinked object file (ET_REL). The displacement bytes of direct
 * calls and branches that carry an unapplied relocation are placeholders and
 * must not be treated as concrete control-flow targets. The harness only
 * parses; with DYNINST_DEBUG_PARSING=1 ParseAPI reports each rejected target
 * in the parsing log, which the ctest driver checks for.
 */
#include "CodeObject.h"
#include "CodeSource.h"
#include "Symtab.h"

#include <cstdio>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int main(int argc, char **argv) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <object file>\n", argv[0]);
    return 1;
  }

  SymtabAPI::Symtab *symtab = nullptr;
  if(!SymtabAPI::Symtab::openFile(symtab, argv[1])) {
    fprintf(stderr, "Unable to open '%s'\n", argv[1]);
    return 2;
  }

  SymtabCodeSource source(symtab);
  CodeObject co(&source);
  co.parse();

  // Mirror image::analyzeImage(): gap-parse each text region.
  for(auto *region : source.regions()) {
    auto *scr = static_cast<SymtabCodeRegion *>(region);
    if(scr->symRegion()->isText()) {
      co.parseGaps(scr);
    }
  }

  printf("parsed %zu functions\n", co.funcs().size());
  return 0;
}
