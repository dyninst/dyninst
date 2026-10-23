// Symtab::addSymbol(newSym, referringSymbol) gives newSym the first version
// name of the referring symbol, as the rewriter does for every reference to
// a symbol in another library. The name must survive addSymbol returning.
#include "dyninst_test_skip.h"

#include "Symbol.h"
#include "Symtab.h"

#include <link.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

namespace {

// The path of the loaded libc, or "" if there is none.
std::string libc_path() {
  std::string path;
  dl_iterate_phdr(
      [](dl_phdr_info *info, size_t, void *data) {
        std::string const name = info->dlpi_name ? info->dlpi_name : "";
        auto const slash = name.rfind('/');
        auto const base = slash == std::string::npos ? 0 : slash + 1;
        // libc.so.6, or libc-2.28.so where glibc-hwcaps loads the real file
        bool const is_libc =
            name.compare(base, 7, "libc.so") == 0 ||
            (name.compare(base, 5, "libc-") == 0 && base + 5 < name.size() &&
             std::isdigit(static_cast<unsigned char>(name[base + 5])));
        if (!is_libc) {
          return 0;
        }
        *static_cast<std::string *>(data) = name;
        return 1;
      },
      &path);
  return path;
}

// Overwrite the stack below the caller, where addSymbol's frame was.
__attribute__((noinline)) void clobber_stack() {
  volatile char junk[16 * 1024];
  for (auto &c : junk) {
    c = static_cast<char>(0xa5);
  }
}

}

int main(int argc, char **argv) {
  if (argc < 1) {
    std::fprintf(stderr, "no program name\n");
    return EXIT_FAILURE;
  }
  std::string const libc_file = libc_path();
  if (libc_file.empty()) {
    return skip_message("libc is not loaded");
  }

  st::Symtab *exe = nullptr;
  st::Symtab *libc = nullptr;
  if (!st::Symtab::openFile(exe, argv[0]) ||
      !st::Symtab::openFile(libc, libc_file)) {
    std::fprintf(stderr, "cannot open %s or %s\n", argv[0], libc_file.c_str());
    return EXIT_FAILURE;
  }

  std::vector<st::Symbol *> candidates;
  libc->findSymbol(candidates, "getpid", st::Symbol::ST_UNKNOWN, st::mangledName);
  st::Symbol *referring = nullptr;
  std::vector<std::string> *referring_versions = nullptr;
  for (auto *s : candidates) {
    if (s->isInDynSymtab() && s->getVersions(referring_versions) &&
        referring_versions && !referring_versions->empty()) {
      referring = s;
      break;
    }
  }
  if (!referring) {
    return skip_message("no versioned getpid in " + libc_file);
  }
  std::string const expected = (*referring_versions)[0];

  auto *sym = new st::Symbol(referring->getMangledName(), referring->getType(),
                             st::Symbol::SL_GLOBAL, st::Symbol::SV_DEFAULT, 0,
                             exe->getDefaultModule(), nullptr,
                             referring->getSize(), true, false);
  if (!exe->addSymbol(sym, referring)) {
    std::fprintf(stderr, "addSymbol failed\n");
    return EXIT_FAILURE;
  }
  clobber_stack();

  std::vector<std::string> *versions = nullptr;
  if (!sym->getVersions(versions) || !versions) {
    std::fprintf(stderr, "the new symbol has no versions\n");
    return EXIT_FAILURE;
  }
  if (versions->size() != 1 || (*versions)[0] != expected) {
    std::fprintf(stderr, "expected version %s, got %zu entries\n",
                 expected.c_str(), versions->size());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
