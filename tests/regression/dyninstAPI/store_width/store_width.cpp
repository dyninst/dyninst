
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

namespace {

  struct global {
    char const *name;
    char const *neighbour;
    unsigned size;
  };

  global const globals[] = {
      {"g_char", "g_char_next", 1},
      {"g_short", "g_short_next", 2},
      {"g_long", nullptr, 8},
  };

  std::string basename(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
  }

  std::string dirname(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
  }

  bool run(std::string const &exe, std::string const &libpath) {
    std::string ldpath = libpath;
    if (char const *old = std::getenv("LD_LIBRARY_PATH")) {
      if (*old) {
        ldpath += ":" + std::string(old);
      }
    }
    pid_t const pid = fork();
    if (pid < 0) {
      std::cerr << "fork: " << std::strerror(errno) << '\n';
      return false;
    }
    if (pid == 0) {
      setenv("LD_LIBRARY_PATH", ldpath.c_str(), 1);
      execl(exe.c_str(), exe.c_str(), static_cast<char *>(nullptr));
      _exit(127);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) {
      std::cerr << "waitpid: " << std::strerror(errno) << '\n';
      return false;
    }
    if (WIFSIGNALED(status)) {
      std::cerr << "rewritten mutatee killed by signal " << WTERMSIG(status) << '\n';
      return false;
    }
    if (WEXITSTATUS(status) != 0) {
      std::cerr << "rewritten mutatee exited with " << WEXITSTATUS(status) << '\n';
      return false;
    }
    return true;
  }

}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage " << argv[0] << ": mutatee outdir\n";
    return EXIT_FAILURE;
  }
  std::string const mutatee = argv[1];
  std::string const outdir = argv[2];

  char const *rt = std::getenv("DYNINSTAPI_RT_LIB");
  if (!rt || !*rt) {
    std::cerr << "DYNINSTAPI_RT_LIB is not set\n";
    return EXIT_FAILURE;
  }

  BPatch bpatch;
  BPatch_binaryEdit *edit = bpatch.openBinary(mutatee.c_str(), false);
  if (!edit) {
    std::cerr << "Cannot open " << mutatee << '\n';
    return EXIT_FAILURE;
  }
  BPatch_image *image = edit->getImage();

  std::vector<BPatch_function *> mains;
  image->findFunction("main", mains);
  if (mains.size() != 1) {
    std::cerr << "Expected one function main, found " << mains.size() << '\n';
    return EXIT_FAILURE;
  }
  std::vector<BPatch_point *> *entry = mains[0]->findPoint(BPatch_entry);
  if (!entry || entry->empty()) {
    std::cerr << "No entry point in main\n";
    return EXIT_FAILURE;
  }

  std::vector<BPatch_snippet *> stores;
  for (auto const &g : globals) {
    BPatch_variableExpr *var = image->findVariable(g.name);
    if (!var) {
      std::cerr << "Cannot find " << g.name << '\n';
      return EXIT_FAILURE;
    }
    if (var->getSize() != g.size) {
      std::cerr << g.name << ": expected size " << g.size << ", found " << var->getSize() << '\n';
      return EXIT_FAILURE;
    }
    if (g.neighbour) {
      BPatch_variableExpr *next = image->findVariable(g.neighbour);
      auto const addr = reinterpret_cast<unsigned long>(var->getBaseAddr());
      if (!next || reinterpret_cast<unsigned long>(next->getBaseAddr()) != addr + g.size) {
        std::cerr << g.neighbour << " does not follow " << g.name << '\n';
        return EXIT_FAILURE;
      }
    }
    stores.push_back(new BPatch_arithExpr(BPatch_assign, *var, BPatch_constExpr(0)));
  }
  if (!edit->insertSnippet(BPatch_sequence(stores), *entry)) {
    std::cerr << "Cannot insert the stores at main's entry\n";
    return EXIT_FAILURE;
  }

  if (mkdir(outdir.c_str(), 0755) != 0 && errno != EEXIST) {
    std::cerr << "mkdir " << outdir << ": " << std::strerror(errno) << '\n';
    return EXIT_FAILURE;
  }
  std::string const rewritten = outdir + "/" + basename(mutatee);
  if (!edit->writeFile(rewritten.c_str())) {
    std::cerr << "Cannot write " << rewritten << '\n';
    return EXIT_FAILURE;
  }

  return run(rewritten, dirname(rt)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
