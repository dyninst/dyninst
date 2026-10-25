// Rewrite a program and run it; a library initialized before the runtime
// library calls into the program's relocated code.
//
// Relocating early_trap_target, which is too short for a jump, leaves a trap
// at its original address; early_trap_ctor's constructor calls that address.
// The constructor runs before the runtime library's, so the trap handler
// must already be installed by then, or the program dies before main:
//
//   Trace/breakpoint trap
//
// Usage: rewrite_early_trap <mutatee> <output directory>
// DYNINSTAPI_RT_LIB names the runtime library; the rewritten mutatee is run
// with its directory and the output directory in LD_LIBRARY_PATH.
// Exits with 77 (skipped) if relocation left no trap at early_trap_target.

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
#include "BPatch_object.h"
#include "Symtab.h"

namespace st = Dyninst::SymtabAPI;

namespace {

  std::string basename(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
  }

  std::string dirname(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
  }

  // True if the first byte of 'name' in 'file' is int3.
  bool starts_with_trap(std::string const &file, std::string const &name) {
    st::Symtab *obj = nullptr;
    if (!st::Symtab::openFile(obj, file)) {
      std::cerr << "Cannot open " << file << '\n';
      return false;
    }
    std::vector<st::Symbol *> syms;
    if (!obj->findSymbol(syms, name, st::Symbol::ST_FUNCTION) || syms.empty()) {
      std::cerr << "No " << name << " in " << file << '\n';
      return false;
    }
    Dyninst::Offset const off = syms[0]->getOffset();
    st::Region *reg = obj->findEnclosingRegion(off);
    if (!reg || !reg->getPtrToRawData()) {
      std::cerr << "No data for " << name << " in " << file << '\n';
      return false;
    }
    auto const *bytes = static_cast<unsigned char const *>(reg->getPtrToRawData());
    return bytes[off - reg->getMemOffset()] == 0xcc;
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
  BPatch_binaryEdit *edit = bpatch.openBinary(mutatee.c_str(), true);
  if (!edit) {
    std::cerr << "Cannot open " << mutatee << '\n';
    return EXIT_FAILURE;
  }

  // Relocate the mutatee's own functions only: its libraries stay as they are.
  int relocated = 0;
  for (auto *f : *edit->getImage()->getProcedures()) {
    if (basename(f->getModule()->getObject()->pathName()) != basename(mutatee)) {
      continue;
    }
    f->relocateFunction();
    ++relocated;
  }
  if (relocated == 0) {
    std::cerr << "No functions in " << mutatee << '\n';
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

  if (!starts_with_trap(rewritten, "early_trap_target")) {
    std::cerr << "No trap at early_trap_target; nothing to test\n";
    return 77;
  }

  return run(rewritten, outdir + ":" + dirname(rt)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
