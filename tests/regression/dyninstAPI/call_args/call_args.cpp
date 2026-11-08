// Rewrite a program with calls that pass four arguments, and run it.
//
// At the entry of each of first, second and third, the mutator inserts two
// calls check(id, id + 0x100, id + 0x200, id + 0x300), with a different id
// for each call. The parameters of the instrumented functions are live at
// their entries, and every call's arguments must reach the callee in the
// argument registers no matter how many calls came before it. The mutatee
// checks the arguments of every call and how many calls ran.
//
// Usage: call_args <mutatee> <output directory>
// DYNINSTAPI_RT_LIB names the runtime library; the rewritten mutatee is run
// with the runtime library's directory in LD_LIBRARY_PATH.

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

  std::string basename(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
  }

  std::string dirname(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
  }

  BPatch_function *find_function(BPatch_image *image, char const *name) {
    std::vector<BPatch_function *> funcs;
    image->findFunction(name, funcs);
    if (funcs.size() != 1) {
      std::cerr << "Expected one function " << name << ", found " << funcs.size() << '\n';
      return nullptr;
    }
    return funcs[0];
  }

  // check(id, id + 0x100, id + 0x200, id + 0x300)
  BPatch_snippet *make_call(BPatch_function &check, long id) {
    std::vector<BPatch_snippet *> args{
        new BPatch_constExpr(id), new BPatch_constExpr(id + 0x100),
        new BPatch_constExpr(id + 0x200), new BPatch_constExpr(id + 0x300)};
    return new BPatch_funcCallExpr(check, args);
  }

  // Run 'exe' with 'libpath' prepended to LD_LIBRARY_PATH; true if it exits with 0.
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

  BPatch_function *check = find_function(image, "check");
  if (!check) {
    return EXIT_FAILURE;
  }

  long id = 1;
  for (char const *name : {"first", "second", "third"}) {
    BPatch_function *func = find_function(image, name);
    if (!func) {
      return EXIT_FAILURE;
    }
    std::vector<BPatch_point *> *entries = func->findPoint(BPatch_entry);
    if (!entries || entries->empty()) {
      std::cerr << "No entry point in " << name << '\n';
      return EXIT_FAILURE;
    }
    std::vector<BPatch_snippet *> calls{make_call(*check, id), make_call(*check, id + 1)};
    id += 2;
    if (!edit->insertSnippet(BPatch_sequence(calls), *entries)) {
      std::cerr << "Cannot insert the calls at " << name << "'s entry\n";
      return EXIT_FAILURE;
    }
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
