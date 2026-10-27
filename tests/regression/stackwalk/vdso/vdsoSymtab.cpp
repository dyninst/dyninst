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
#include "Symbol.h"
#include "frame.h"
#include "swk_errors.h"
#include "walker.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <csignal>
#include <sys/auxv.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace sw = Dyninst::Stackwalker;
namespace st = Dyninst::SymtabAPI;

namespace {

  constexpr int skipped = 77;

  // Attach to the child and detach again, so that a host which refuses
  // ptrace fails with the reason rather than with the walker's generic error.
  bool probePtrace(pid_t pid) {
    if(ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) != 0) {
      int const err = errno;
      std::cerr << "PTRACE_ATTACH of " << pid << " failed: " << std::strerror(err) << "\n";
      if(err == EPERM) {
        std::ifstream scope("/proc/sys/kernel/yama/ptrace_scope");
        int level{};
        if(scope >> level) {
          std::cerr << "yama ptrace_scope is " << level << "\n";
        }
        std::cerr << "Configure with -DDYNINST_ENABLE_DYNAMIC_TESTS=OFF where ptrace "
                     "is not permitted\n";
      }
      return false;
    }
    int status{};
    if(waitpid(pid, &status, 0) != pid || !WIFSTOPPED(status) ||
       ptrace(PTRACE_DETACH, pid, nullptr, nullptr) != 0) {
      std::cerr << "Could not stop and detach from " << pid << "\n";
      return false;
    }
    return true;
  }

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

    // Older x86 kernels link the vDSO at a fixed address. Its symbol offsets
    // are then link-time addresses, while the walker's library offsets are
    // relative to where the vDSO is mapped, so the two cannot be compared.
    if(symtab->getLoadOffset() != 0) {
      std::cout << "The vDSO is linked at 0x" << std::hex << symtab->getLoadOffset()
                << ", not 0\n";
      return skipped;
    }

    // Every Linux vDSO this test runs on exports clock_gettime, under an
    // arch-specific name (__vdso_clock_gettime, __kernel_clock_gettime). The
    // hand-written ppc64 vDSO of older kernels gives it type NOTYPE, so it is
    // a Symbol but not a Function there.
    std::vector<st::Symbol*> syms;
    symtab->getAllSymbols(syms);
    st::Symbol* target{};
    for(auto* s : syms) {
      if(s->getPrettyName().find("clock_gettime") != std::string::npos) {
        target = s;
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
    if(offset != target->getOffset()) {
      std::cerr << "Offset 0x" << std::hex << offset << " is not "
                << target->getPrettyName() << "'s 0x" << target->getOffset() << "\n";
      return EXIT_FAILURE;
    }

    if(st::Function* func = target->getFunction()) {
      st::Function* found{};
      if(!symtab->getContainingFunction(offset, found) || found != func) {
        std::cerr << "Offset 0x" << std::hex << offset << " does not resolve to "
                  << func->getName() << "\n";
        return EXIT_FAILURE;
      }
    }
    return EXIT_SUCCESS;
  }

}

int main() {
  if(!DYNINST_ENABLE_DYNAMIC_TESTS) {
    std::cout << "Turned off by DYNINST_ENABLE_DYNAMIC_TESTS=OFF\n";
    return skipped;
  }

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

  int result = EXIT_FAILURE;
  if(probePtrace(child)) {
    if(sw::Walker* walker = sw::Walker::newWalker(child)) {
      result = check(walker, child, vdso_base);
      delete walker;
    } else {
      std::cerr << "Unable to attach a walker to " << child << ": "
                << sw::getLastErrorMsg() << "\n";
    }
  }

  kill(child, SIGKILL);
  waitpid(child, nullptr, 0);
  return result;
}
