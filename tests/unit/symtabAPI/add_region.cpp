// Closing a Symtab after Symtab::addRegion deletes each added region exactly
// once.
#include "Region.h"
#include "Symtab.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

// This replaces the global operator delete for the whole process, symtabAPI
// included. While closeSymtab runs it counts the deletes of the added regions
// and keeps their memory, so a second delete is counted instead of crashing
// in free(). A region deleted twice is still destroyed twice. That second
// ~Region frees nothing here: the names below fit std::string's inline
// buffer, the regions have no relocations, and patchData is never called.
static std::vector<st::Region *> *watched = nullptr;
static int delete_count[3];

void operator delete(void *p) noexcept {
  if (watched)  {
    for (std::size_t i = 0; i < watched->size(); ++i)  {
      if ((*watched)[i] == p)  {
        ++delete_count[i];
        return;
      }
    }
  }
  std::free(p);
}

void operator delete(void *p, std::size_t) noexcept {
  operator delete(p);
}

int main(int argc, char **argv) {
  if (argc < 1)  {
    std::fprintf(stderr, "no program name\n");
    return 1;
  }
  st::Symtab *obj = nullptr;
  if (!st::Symtab::openFile(obj, argv[0]))  {
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
  if (!obj->getAllNewRegions(added) || added.size() != 3)  {
    std::fprintf(stderr, "expected 3 new regions, got %zu\n", added.size());
    return 1;
  }

  // ~Symtab frees the regions through getAllRegions' list only, so every
  // added region has to be in it.
  std::vector<st::Region *> all;
  if (!obj->getAllRegions(all))  {
    std::fprintf(stderr, "getAllRegions failed\n");
    return 1;
  }
  for (auto *r : added)  {
    if (std::find(all.begin(), all.end(), r) == all.end())  {
      std::fprintf(stderr, "added region %s is not in getAllRegions\n",
                   r->getRegionName().c_str());
      return 1;
    }
  }

  watched = &added;
  st::Symtab::closeSymtab(obj);
  watched = nullptr;
  for (std::size_t i = 0; i < added.size(); ++i)  {
    if (delete_count[i] != 1)  {
      std::fprintf(stderr, "added region %zu deleted %d times\n", i,
                   delete_count[i]);
      return 1;
    }
  }
  return 0;
}
