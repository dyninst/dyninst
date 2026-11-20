// Rewrite a program with a snippet at a function exit, with liveness analysis
// off, and run it.
//
// With liveness analysis off every register is live, so the aarch64 base tramp
// saves the system registers (nzcv, fpcr, fpsr) as well as the GPRs. The
// snippet calls record(<return value>, kMarker): it reads the saved x0 back
// from the tramp frame, and x1, which holds kMarker, is still allocated when
// the tramp guard is released after the call, so it is pushed around that
// call. The mutatee checks that record saw answer's return value and the
// marker, and that FPCR, which it set to a non-default value, is unchanged.
// kMarker's upper half is a different FPCR rounding mode, so a push that
// lands on the saved fpcr changes FPCR.
//
// Usage: tramp_spr <mutatee> <output directory>
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

  // Must match MARKER in mutatee.c.
  long const kMarker = 0x0040000000000007L;

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
  bpatch.setLivenessAnalysis(false);

  BPatch_binaryEdit *edit = bpatch.openBinary(mutatee.c_str(), false);
  if (!edit) {
    std::cerr << "Cannot open " << mutatee << '\n';
    return EXIT_FAILURE;
  }
  BPatch_image *image = edit->getImage();

  BPatch_function *answer = find_function(image, "answer");
  BPatch_function *record = find_function(image, "record");
  if (!answer || !record) {
    return EXIT_FAILURE;
  }

  std::vector<BPatch_point *> *exits = answer->findPoint(BPatch_exit);
  if (!exits || exits->empty()) {
    std::cerr << "No exit points in answer\n";
    return EXIT_FAILURE;
  }

  std::vector<BPatch_snippet *> args{new BPatch_retExpr(), new BPatch_constExpr(kMarker)};
  BPatch_funcCallExpr call(*record, args);
  if (!edit->insertSnippet(call, *exits)) {
    std::cerr << "Cannot insert the snippet at answer's exit\n";
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
