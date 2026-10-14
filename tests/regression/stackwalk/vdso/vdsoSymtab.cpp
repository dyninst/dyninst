/*
 * A frame inside the vDSO must resolve to a Symtab.
 *
 * StackwalkerAPI names the vDSO "[vsyscall-<pid>]", which is not a file, so
 * Frame::getLibOffset can only find its Symtab if the walker registered the one
 * it parsed from memory. Tools such as STAT pass that Symtab straight to
 * Symtab::getContainingFunction, so a NULL here is a crash there.
 *
 * The test attaches to a forked child, as STAT does, and builds Frames whose
 * return address points into the vDSO, so no thread has to be caught inside it.
 * A fork child keeps its parent's vDSO address.
 */
#include "Symtab.h"

#include "Function.h"
#include "frame.h"
#include "swk_errors.h"
#include "walker.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <csignal>
#include <sys/auxv.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace sw = Dyninst::Stackwalker;
namespace st = Dyninst::SymtabAPI;

namespace {

  constexpr int skipped = 77;

  bool symtabAt(sw::Walker* walker, pid_t pid, Dyninst::Address addr, st::Symtab*& symtab,
                Dyninst::Offset& offset) {
    sw::Frame frame(walker);
    frame.setRA(addr);

    std::string lib;
    void* symtab_v{};
    if(!frame.getLibOffset(lib, offset, symtab_v)) {
      std::cerr << "getLibOffset failed for 0x" << std::hex << addr << "\n";
      return false;
    }

    std::ostringstream expected;
    expected << "[vsyscall-" << pid << "]";
    if(lib != expected.str()) {
      std::cerr << "0x" << std::hex << addr << " is in '" << lib << "', expected '"
                << expected.str() << "'\n";
      return false;
    }

    symtab = static_cast<st::Symtab*>(symtab_v);
    if(!symtab) {
      std::cerr << "No Symtab for " << lib << "\n";
      return false;
    }
    return true;
  }

  int check(sw::Walker* walker, pid_t pid, Dyninst::Address vdso_base) {
    st::Symtab* symtab{};
    Dyninst::Offset offset{};
    if(!symtabAt(walker, pid, vdso_base, symtab, offset)) {
      return EXIT_FAILURE;
    }

    // Every Linux vDSO this test runs on exports clock_gettime, under an
    // arch-specific name (__vdso_clock_gettime, __kernel_clock_gettime).
    std::vector<st::Function*> funcs;
    symtab->getAllFunctions(funcs);
    st::Function* target{};
    for(auto* f : funcs) {
      for(auto it = f->pretty_names_begin(); !target && it != f->pretty_names_end(); ++it) {
        if(it->find("clock_gettime") != std::string::npos) {
          target = f;
        }
      }
      if(target) {
        break;
      }
    }
    if(!target) {
      std::cerr << "The vDSO's Symtab has no clock_gettime\n";
      return EXIT_FAILURE;
    }

    if(!symtabAt(walker, pid, vdso_base + target->getOffset(), symtab, offset)) {
      return EXIT_FAILURE;
    }

    st::Function* found{};
    if(!symtab->getContainingFunction(offset, found) || found != target) {
      std::cerr << "Offset 0x" << std::hex << offset << " does not resolve to "
                << target->getName() << "\n";
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }

}

int main() {
  auto const vdso_base = static_cast<Dyninst::Address>(getauxval(AT_SYSINFO_EHDR));
  if(vdso_base == Dyninst::ADDR_NULL) {
    std::cout << "No vDSO in this process\n";
    return skipped;
  }

  pid_t const parent = getpid();
  pid_t const child = fork();
  if(child < 0) {
    std::cerr << "fork failed\n";
    return EXIT_FAILURE;
  }
  if(child == 0) {
    // Don't outlive a parent that crashes before it can kill us.
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    if(getppid() != parent) {
      _exit(EXIT_FAILURE);
    }
    for(;;) {
      pause();
    }
  }

  // Attaching needs ptrace, which yama or a container may forbid.
  int result = skipped;
  if(sw::Walker* walker = sw::Walker::newWalker(child)) {
    result = check(walker, child, vdso_base);
    delete walker;
  } else {
    std::cout << "Unable to attach a walker to " << child << ": "
              << sw::getLastErrorMsg() << "\n";
  }

  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  return result;
}
