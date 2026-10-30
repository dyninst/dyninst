#include "Module.h"
#include "Symtab.h"
#include "Type.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// See test_binaries/symtabAPI/DWARF/parallel_parse/README.md
//
// Usage: parallel_parse <binary> <threads> <runs>
//
// Parses the types of <binary> once with one OpenMP thread, then <runs> more
// times with <threads>, and fails unless every parse finds the same types.
// Each parse runs in a fresh process: libdw keeps state between parses within
// a process, and only a first parse is a fair test of the parallel walk. The
// thread count goes in OMP_NUM_THREADS, which the OpenMP runtime reads when it
// starts, so the test needs no OpenMP of its own.
//
// A parse sharing one libdw handle across workers loses types, and with dwz
// partial units can crash in dwarf_getlocations (#2410).

namespace st = Dyninst::SymtabAPI;

extern char **environ;

namespace {

// Child: parse once and print "<types> <fields>".
int parse(char const *file) {
  st::Symtab *symtab{};
  if (!st::Symtab::openFile(symtab, file)) {
    std::cerr << "FAIL: unable to open '" << file << "'\n";
    return EXIT_FAILURE;
  }
  symtab->parseTypesNow();

  std::vector<st::Module *> modules;
  symtab->getAllModules(modules);
  size_t types = 0, fields = 0;
  for (auto *m : modules) {
    std::vector<boost::shared_ptr<st::Type>> all;
    m->getAllTypes(all);
    types += all.size();
    for (auto &t : all)
      if (t->isFieldListType()) fields += t->asFieldListType().getComponents()->size();
  }
  std::cout << types << " " << fields << "\n";
  return EXIT_SUCCESS;
}

// Parent: run this program as a child parsing <file> with <threads> threads.
// Returns its output, or an empty string if it failed.
std::string run_child(char const *self, char const *file, char const *threads) {
  int out[2];
  if (pipe(out) != 0) return {};

  std::vector<std::string> env_strings;
  for (char **e = environ; *e; ++e)
    if (std::strncmp(*e, "OMP_NUM_THREADS=", 16) != 0) env_strings.emplace_back(*e);
  env_strings.push_back(std::string{"OMP_NUM_THREADS="} + threads);
  std::vector<char *> env;
  for (auto &s : env_strings) env.push_back(&s[0]);
  env.push_back(nullptr);

  posix_spawn_file_actions_t actions;
  posix_spawn_file_actions_init(&actions);
  posix_spawn_file_actions_adddup2(&actions, out[1], STDOUT_FILENO);
  posix_spawn_file_actions_addclose(&actions, out[0]);

  char const *argv[] = {self, "--parse", file, nullptr};
  pid_t pid{};
  int const err = posix_spawn(&pid, self, &actions, nullptr, const_cast<char **>(argv), env.data());
  posix_spawn_file_actions_destroy(&actions);
  close(out[1]);
  if (err != 0) {
    close(out[0]);
    std::cerr << "FAIL: unable to start '" << self << "': " << std::strerror(err) << "\n";
    return {};
  }

  std::string text;
  char buf[256];
  ssize_t n;
  while ((n = read(out[0], buf, sizeof buf)) > 0) text.append(buf, n);
  close(out[0]);

  int status{};
  waitpid(pid, &status, 0);
  if (WIFSIGNALED(status)) {
    std::cerr << "FAIL: parse with " << threads << " threads died with signal "
              << WTERMSIG(status) << " (" << strsignal(WTERMSIG(status)) << ")\n";
    return {};
  }
  if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
    std::cerr << "FAIL: parse with " << threads << " threads failed\n";
    return {};
  }
  if (!text.empty() && text.back() == '\n') text.pop_back();
  return text;
}

}

int main(int argc, char **argv) {
  if (argc == 3 && std::string{argv[1]} == "--parse") return parse(argv[2]);

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <binary> <threads> <runs>\n";
    return EXIT_FAILURE;
  }
  char const *file = argv[1];
  char const *threads = argv[2];
  int const runs = std::atoi(argv[3]);

  // posix_spawn needs a path; argv[0] may not be one.
  char self[4096];
  ssize_t const len = readlink("/proc/self/exe", self, sizeof self - 1);
  if (len <= 0) {
    std::cerr << "FAIL: unable to find this executable\n";
    return EXIT_FAILURE;
  }
  self[len] = '\0';

  std::string const expected = run_child(self, file, "1");
  if (expected.empty()) return EXIT_FAILURE;
  std::cout << "1 thread: types fields " << expected << "\n";

  bool failed = false;
  for (int i = 0; i < runs; i++) {
    std::string const got = run_child(self, file, threads);
    if (got.empty()) {
      failed = true;
      continue;
    }
    std::cout << threads << " threads: types fields " << got << "\n";
    if (got != expected) {
      std::cerr << "FAIL: parse " << i + 1 << " with " << threads << " threads found '" << got
                << "', the serial parse '" << expected << "'\n";
      failed = true;
    }
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
