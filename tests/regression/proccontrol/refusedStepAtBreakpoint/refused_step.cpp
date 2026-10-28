/*
 * Single-step a thread from a breakpoint on a load-reserve whose sequence the emulated
 * single step refuses. The continue that should step over the breakpoint fails, and it
 * used to leave the step-over half set up: the thread's ClearingBPs count stayed at 1,
 * so every later Process::handleEvents(), even a non-blocking one, blocked forever.
 *
 * Passes when the event loop keeps returning, the thread is reported stopped and is still
 * on the breakpoint, and the mutatee runs to a normal exit once the breakpoint and
 * single-step are off.  On any other host, where the code generator does not target the
 * host, or with DYNINST_ENABLE_DYNAMIC_TESTS=OFF, it reports itself skipped (exit 77).
 */
#include "Event.h"
#include "PCProcess.h"
#include "Symbol.h"
#include "Symtab.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unistd.h>
#include <vector>

namespace pc = Dyninst::ProcControlAPI;
namespace st = Dyninst::SymtabAPI;

namespace {

bool at_bp = false, stepped = false, exited = false;
int exit_code = -1;

pc::Process::cb_ret_t on_bp(pc::Event::const_ptr) {
  at_bp = true;
  return pc::Process::cbThreadStop;
}

pc::Process::cb_ret_t on_step(pc::Event::const_ptr) {
  stepped = true;
  return pc::Process::cbThreadStop;
}

pc::Process::cb_ret_t on_exit(pc::Event::const_ptr ev) {
  exited = true;
  exit_code = ev->getEventExit()->getExitCode();
  return pc::Process::cbDefault;
}

const char *phase = "setup";

void on_alarm(int) {
  // A blocked handleEvents() never returns; report which phase hung.
  char msg[160];
  int n = snprintf(msg, sizeof msg, "FAIL: no progress in %s (handleEvents blocked)\n", phase);
  if (n > 0) {
    ssize_t r = write(STDOUT_FILENO, msg, static_cast<size_t>(n));
    (void)r;
  }
  _exit(EXIT_FAILURE);
}

// Poll the event loop without blocking for up to `secs` or until `flag` is set.
void poll_events(bool &flag, int secs) {
  time_t t0 = time(nullptr);
  while (!flag && !exited && time(nullptr) - t0 < secs) {
    if (!pc::Process::handleEvents(false)) {
      usleep(10000);
    }
  }
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::fprintf(stderr, "Usage: %s mutatee\n", argv[0]);
    return EXIT_FAILURE;
  }
  std::string path = argv[1];
  if (!DYNINST_ENABLE_DYNAMIC_TESTS) {
    std::printf(
        "skipping: drives a live process, which DYNINST_ENABLE_DYNAMIC_TESTS=OFF turns off\n");
    return 77;
  }
#if !REFUSED_STEP_LLSC_HOST
  std::printf("skipping: emulated single steps through load-reserve/store-conditional sequences "
              "exist only on ppc64le and aarch64 hosts whose code generator targets the host\n");
  return 77;
#endif
  std::signal(SIGALRM, on_alarm);
  alarm(60);

  st::Symtab *symtab = nullptr;
  std::vector<st::Symbol *> syms;
  if (!st::Symtab::openFile(symtab, path) ||
      !symtab->findSymbol(syms, "refused_seq_start", st::Symbol::ST_UNKNOWN) || syms.empty()) {
    std::printf("FAIL: no refused_seq_start in %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  Dyninst::Offset off = syms[0]->getOffset();

  pc::Process::registerEventCallback(pc::EventType::Breakpoint, on_bp);
  pc::Process::registerEventCallback(pc::EventType::SingleStep, on_step);
  pc::Process::registerEventCallback(pc::EventType(pc::EventType::Post, pc::EventType::Exit),
                                     on_exit);

  pc::Process::ptr proc = pc::Process::createProcess(path, {path});
  if (!proc) {
    std::printf("FAIL: createProcess %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  Dyninst::Address addr = proc->libraries().getExecutable()->getLoadAddress() + off;
  pc::Breakpoint::ptr bp = pc::Breakpoint::newBreakpoint();
  if (!proc->addBreakpoint(addr, bp) || !proc->continueProc()) {
    std::printf("FAIL: could not set the breakpoint at 0x%lx\n", (unsigned long)addr);
    return EXIT_FAILURE;
  }

  phase = "running to the breakpoint";
  poll_events(at_bp, 30);
  if (!at_bp) {
    std::printf("FAIL: never reached the breakpoint at 0x%lx\n", (unsigned long)addr);
    return EXIT_FAILURE;
  }
  pc::Thread::ptr thr = proc->threads().getInitialThread();
  Dyninst::MachRegister pc_reg = Dyninst::MachRegister::getPC(proc->getArchitecture());

  // The step the platform refuses.
  phase = "the refused single step";
  thr->setSingleStepMode(true);
  thr->continueThread();
  poll_events(stepped, 3);

  if (stepped || exited) {
    std::printf("FAIL: the refused step ran (stepped=%d exited=%d)\n", stepped, exited);
    return EXIT_FAILURE;
  }

  // The continue did not happen, so the thread is still stopped on the breakpoint.
  if (!thr->isStopped()) {
    std::printf("FAIL: the thread is not reported stopped after the refused step\n");
    return EXIT_FAILURE;
  }
  Dyninst::MachRegisterVal at = 0;
  if (!thr->getRegister(pc_reg, at) || at != addr) {
    std::printf("FAIL: the thread is not on the breakpoint (pc=0x%lx, breakpoint 0x%lx)\n",
                (unsigned long)at, (unsigned long)addr);
    return EXIT_FAILURE;
  }

  // The process is still usable: without the breakpoint and single-step it runs to exit.
  phase = "running to exit";
  thr->setSingleStepMode(false);
  if (!proc->rmBreakpoint(addr, bp)) {
    std::printf("FAIL: rmBreakpoint\n");
    return EXIT_FAILURE;
  }
  thr->continueThread();
  poll_events(exited, 30);
  if (!exited || exit_code != 0) {
    std::printf("FAIL: the mutatee did not exit normally (exited=%d code=%d)\n", exited, exit_code);
    return EXIT_FAILURE;
  }
  std::printf("PASS\n");
  return EXIT_SUCCESS;
}
