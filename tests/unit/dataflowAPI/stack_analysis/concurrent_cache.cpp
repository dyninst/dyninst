// StackAnalysis caches its results on the ParseAPI::Function. Analyses of
// different functions run concurrently during parallel parsing, so the cache
// must be safe from several threads. Analyze every function of libc from
// several threads at once, then read each function's results back from the
// cache in one thread and check that they match what was computed.
#include "dyninst_test_skip.h"

#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "stackanalysis.h"

#include <link.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <thread>
#include <vector>

namespace pa = Dyninst::ParseAPI;

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

// The stack height at the last instruction of each block of f. StackAnalysis
// throws for code it cannot analyze and on architectures it does not
// implement; that is recorded instead.
std::vector<std::string> heights(pa::Function *f) {
  std::vector<std::string> out;
  try {
    Dyninst::StackAnalysis sa(f);
    for (auto *b : f->blocks()) {
      out.push_back(sa.findSP(b, b->last()).format());
    }
  } catch (std::exception const &e) {
    out.assign(1, std::string("exception: ") + e.what());
  }
  return out;
}

}

int main() {
  std::string const libc_file = libc_path();
  if (libc_file.empty()) {
    return skip_message("libc is not loaded");
  }
  auto *source = new pa::SymtabCodeSource(libc_file.c_str());
  auto *code = new pa::CodeObject(source);
  code->parse();

  std::vector<pa::Function *> funcs(code->funcs().begin(), code->funcs().end());
  if (funcs.empty()) {
    std::fprintf(stderr, "no functions parsed in %s\n", libc_file.c_str());
    return EXIT_FAILURE;
  }

  unsigned const nthreads = 8;
  std::vector<std::vector<std::string>> first(funcs.size());
  std::vector<std::thread> threads;
  for (unsigned t = 0; t < nthreads; ++t) {
    threads.emplace_back([&, t] {
      for (size_t i = t; i < funcs.size(); i += nthreads) {
        first[i] = heights(funcs[i]);
      }
    });
  }
  for (auto &th : threads) {
    th.join();
  }

  size_t analyzed = 0;
  for (auto const &h : first) {
    if (h.empty() || h[0].compare(0, 10, "exception:") != 0) {
      ++analyzed;
    }
  }
  if (analyzed == 0) {
    return skip_message("StackAnalysis threw for every function (" +
                        first[0][0] + ")");
  }

  // A new StackAnalysis finds the cached results, so this pass reads them
  // back rather than analyzing again. A function that threw has nothing
  // cached; it is analyzed again and throws again.
  size_t mismatches = 0;
  for (size_t i = 0; i < funcs.size(); ++i) {
    if (heights(funcs[i]) != first[i]) {
      if (++mismatches <= 5) {
        std::fprintf(stderr, "%s: stack heights differ between passes\n",
                     funcs[i]->name().c_str());
      }
    }
  }
  std::printf("%zu functions, %zu analyzed, %zu mismatches\n", funcs.size(),
              analyzed, mismatches);

  delete code;
  delete source;
  return mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
