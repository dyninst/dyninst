// Rewrite the C library a program uses, then run the program with it.
//
// The runtime library depends on the C library, so a DT_NEEDED entry on the
// runtime library in the rewritten C library makes a cycle. With glibc 2.28
// (RHEL 8) the loader then relocates libdl before libc and the program dies
// before main:
//
//   Relink `/lib64/libdl.so.2' with `.../libc.so.6' for IFUNC symbol `strcpy'
//   Segmentation fault
//
// The rewritten C library must have no DT_NEEDED entry on the runtime library
// and no empty one, and the program must run.
//
// Usage: rewrite_libc <mutatee> <output directory>
// DYNINSTAPI_RT_LIB names the runtime library; the rewritten mutatee is run
// with its directory and the output directory in LD_LIBRARY_PATH.

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

  std::string dirname(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
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

  std::vector<BPatch_object *> objects;
  edit->getImage()->getObjects(objects);
  BPatch_object *libc = nullptr;
  for (auto *o : objects) {
    if (o->name().rfind("libc.so", 0) == 0) {
      libc = o;
    }
  }
  if (!libc) {
    std::cerr << "The mutatee does not use libc.so\n";
    return EXIT_FAILURE;
  }

  // Any change to the C library makes it written; relocate the function main calls.
  std::vector<BPatch_function *> puts_funcs;
  libc->findFunction("puts", puts_funcs);
  if (puts_funcs.empty()) {
    std::cerr << "No puts in " << libc->pathName() << '\n';
    return EXIT_FAILURE;
  }
  for (auto *f : puts_funcs) {
    f->relocateFunction();
  }

  // Dependencies are written to the current directory.
  if (mkdir(outdir.c_str(), 0755) != 0 && errno != EEXIST) {
    std::cerr << "mkdir " << outdir << ": " << std::strerror(errno) << '\n';
    return EXIT_FAILURE;
  }
  if (chdir(outdir.c_str()) != 0) {
    std::cerr << "chdir " << outdir << ": " << std::strerror(errno) << '\n';
    return EXIT_FAILURE;
  }
  std::string const rewritten = outdir + "/rewrite_libc_mutatee";
  if (!edit->writeFile(rewritten.c_str())) {
    std::cerr << "Cannot write " << rewritten << '\n';
    return EXIT_FAILURE;
  }

  std::string const rewritten_libc = outdir + "/" + libc->name();
  st::Symtab *obj = nullptr;
  if (!st::Symtab::openFile(obj, rewritten_libc)) {
    std::cerr << "Cannot open " << rewritten_libc << '\n';
    return EXIT_FAILURE;
  }
  bool ok = true;
  for (auto const &dep : obj->getDependencies()) {
    std::cout << libc->name() << " needs '" << dep << "'\n";
    if (dep.empty()) {
      std::cerr << "Empty DT_NEEDED entry in " << rewritten_libc << '\n';
      ok = false;
    } else if (dep.find("libdyninstAPI_RT") != std::string::npos) {
      std::cerr << "DT_NEEDED entry on the runtime library in " << rewritten_libc << '\n';
      ok = false;
    }
  }

  if (!run(rewritten, outdir + ":" + dirname(rt))) {
    ok = false;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
