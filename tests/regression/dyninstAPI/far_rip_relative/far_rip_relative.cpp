// Rewrite a program whose RIP-relative instructions end up more than 2 GiB
// from their targets once relocated, and run it.
//
// The binary rewriter puts relocated code after the end of the mutatee's
// .bss, which pad makes more than 2 GiB past far_value. Instrumenting leaf
// relocates its RIP-relative load of far_value there, and instrumenting
// far_caller relocates its RIP-relative indirect call through far_fn, which
// sits next to far_value. Neither can keep its 32-bit displacement:
//  - the load is rewritten to address far_value through a scratch register
//    saved on the stack, and leaf keeps a canary in the red zone that the
//    save must not overwrite;
//  - the call must reach the callee with the stack the caller set up, so its
//    seventh argument and 16-byte alignment are intact.
// The mutatee is linked above 4 GiB so that neither target can be addressed
// with a 32-bit absolute address either.
//
// Usage: far_rip_relative <mutatee> <output directory>
// DYNINSTAPI_RT_LIB names the runtime library; the rewritten mutatee is run
// with the runtime library's directory in LD_LIBRARY_PATH.

#include <cerrno>
#include <cstdint>
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
#include "Region.h"
#include "Symtab.h"

namespace {

  constexpr uint64_t max_disp32 = 0x7fffffff;
  constexpr uint64_t max_addr32 = 0xffffffff;

  std::string basename(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
  }

  std::string dirname(std::string const &path) {
    auto const slash = path.rfind('/');
    return slash == std::string::npos ? "." : path.substr(0, slash);
  }

  uint64_t address_of(BPatch_variableExpr *var) {
    return reinterpret_cast<uint64_t>(var->getBaseAddr());
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

  BPatch_variableExpr *far_value = image->findVariable("far_value");
  BPatch_variableExpr *entered = image->findVariable("entered");
  if (!far_value || !entered) {
    std::cerr << "Unable to find far_value and entered in the mutatee\n";
    return EXIT_FAILURE;
  }

  // The end of .bss is taken from the section table, not from pad's debug
  // information: Dyninst reports a 4-byte pad for clang's description of the
  // array (DW_AT_count 0xa0000000).
  Dyninst::SymtabAPI::Symtab *symtab = nullptr;
  Dyninst::SymtabAPI::Region *bss = nullptr;
  if (!Dyninst::SymtabAPI::Symtab::openFile(symtab, mutatee) ||
      !symtab->findRegion(bss, ".bss")) {
    std::cerr << "Unable to find .bss in " << mutatee << '\n';
    return EXIT_FAILURE;
  }
  uint64_t const bss_end = bss->getMemOffset() + bss->getMemSize();

  // far_fn is next to far_value, so it is out of reach whenever far_value is.
  // Without this, the test passes without exercising the rewritten code.
  if (address_of(far_value) <= max_addr32 || bss_end - address_of(far_value) <= max_disp32) {
    std::cerr << std::hex << "far_value (0x" << address_of(far_value) << ") is below 4 GiB or "
              << "within 2 GiB of the end of .bss (0x" << bss_end << "); the layout does not "
              << "exercise the rewritten code\n";
    return EXIT_FAILURE;
  }

  BPatch_arithExpr increment(BPatch_assign, *entered,
                             BPatch_arithExpr(BPatch_plus, *entered, BPatch_constExpr(1)));
  for (char const *name : {"leaf", "far_caller"}) {
    std::vector<BPatch_function *> funcs;
    image->findFunction(name, funcs);
    if (funcs.size() != 1) {
      std::cerr << "Expected one function " << name << ", found " << funcs.size() << '\n';
      return EXIT_FAILURE;
    }
    std::vector<BPatch_point *> *entry = funcs[0]->findPoint(BPatch_entry);
    if (!entry || entry->empty() || !edit->insertSnippet(increment, *entry)) {
      std::cerr << "Unable to instrument " << name << "'s entry\n";
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
