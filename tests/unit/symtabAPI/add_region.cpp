// Closing a Symtab after Symtab::addRegion deletes each added region once.
#include "Region.h"
#include "Symtab.h"

#include <cstdio>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

int main(int argc, char **argv) {
  if (argc < 1) {
    std::fprintf(stderr, "no program name\n");
    return 1;
  }
  st::Symtab *obj = nullptr;
  if (!st::Symtab::openFile(obj, argv[0])) {
    std::fprintf(stderr, "cannot open %s\n", argv[0]);
    return 1;
  }

  static char loadable_data[16];
  static char other_data[16];
  obj->addRegion(0x10000000, loadable_data, sizeof loadable_data, ".added_text",
                 st::Region::RT_TEXT, true);
  obj->addRegion(0x10001000, loadable_data, sizeof loadable_data, ".added_data",
                 st::Region::RT_DATA, true);
  obj->addRegion(0, other_data, sizeof other_data, ".added_note",
                 st::Region::RT_OTHER, false);

  std::vector<st::Region *> added;
  if (!obj->getAllNewRegions(added) || added.size() != 3) {
    std::fprintf(stderr, "expected 3 new regions, got %zu\n", added.size());
    return 1;
  }

  st::Symtab::closeSymtab(obj);
  return 0;
}
