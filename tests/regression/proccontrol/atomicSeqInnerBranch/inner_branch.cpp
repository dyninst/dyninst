/*
 * Single-step a thread through a load-reserve/store-conditional sequence that branches
 * back to the reserve before the store-conditional. The emulated single step used to
 * take that branch for an exit from the sequence and put a breakpoint on its target, the
 * reserve, which is the instruction the thread is about to execute: every continue
 * trapped at once, and the thread never got past the reserve.
 *
 * The breakpoint is on the instruction before the reserve, so the step from the reserve is
 * an ordinary emulated step rather than the step over a breakpoint.
 *
 * Passes when the step from the reserve ends on the instruction after the store-conditional,
 * and the mutatee runs to a normal exit once single-step is off. Skips (exit 77) on
 * architectures that do not emulate single steps through such a sequence, without the test
 * binaries that hold the mutatee (or with a checkout of them that predates it), and with
 * DYNINST_ENABLE_DYNAMIC_TESTS=OFF.
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

Dyninst::Address reserve_addr = 0, after_store_addr = 0;
bool at_bp = false, exited = false;
int exit_code = -1;

// Where single steps ended, in order
std::vector<Dyninst::Address> steps;
// Enough steps to tell a thread that is not making progress from one that is
const std::size_t max_steps = 50;

pc::Process::cb_ret_t on_bp(pc::Event::const_ptr) {
  at_bp = true;
  return pc::Process::cbThreadStop;
}

bool step_done() {
  return !steps.empty() && (steps.back() == after_store_addr || steps.size() >= max_steps);
}

pc::Process::cb_ret_t on_step(pc::Event::const_ptr ev) {
  Dyninst::MachRegisterVal at = 0;
  pc::Thread::const_ptr thr = ev->getThread();
  thr->getRegister(Dyninst::MachRegister::getPC(ev->getProcess()->getArchitecture()), at);
  steps.push_back(at);
  return step_done() ? pc::Process::cbThreadStop : pc::Process::cbThreadContinue;
}

pc::Process::cb_ret_t on_exit(pc::Event::const_ptr ev) {
  exited = true;
  exit_code = ev->getEventExit()->getExitCode();
  return pc::Process::cbDefault;
}

const char *phase = "setup";

void on_alarm(int) {
  char msg[160];
  int n = snprintf(msg, sizeof msg, "FAIL: no progress in %s\n", phase);
  if (n > 0) {
    ssize_t r = write(STDOUT_FILENO, msg, static_cast<size_t>(n));
    (void)r;
  }
  _exit(EXIT_FAILURE);
}

// Poll the event loop without blocking for up to `secs` or until `done` returns true.
template <typename Done>
void poll_events(Done done, int secs) {
  time_t t0 = time(nullptr);
  while (!done() && !exited && time(nullptr) - t0 < secs) {
    if (!pc::Process::handleEvents(false)) {
      usleep(10000);
    }
  }
}

bool find_offset(st::Symtab *symtab, const char *name, Dyninst::Offset &off) {
  std::vector<st::Symbol *> syms;
  if (!symtab->findSymbol(syms, name, st::Symbol::ST_UNKNOWN) || syms.empty()) {
    return false;
  }
  off = syms[0]->getOffset();
  return true;
}

int skip(const char *reason) {
  std::printf("skipping: %s\n", reason);
  return 77;
}

}  // namespace

// The mutatee comes from the test binaries; without them no path is passed and the test skips.
int main(int argc, char **argv) {
  if (argc > 2) {
    std::fprintf(stderr, "Usage: %s [mutatee]\n", argv[0]);
    return EXIT_FAILURE;
  }
#if !defined(__aarch64__) && !defined(__powerpc64__)
  // Only Power and AArch64 emulate single steps through load-reserve/store-conditional sequences
  return skip("no emulated single step through load-reserve/store-conditional on this "
              "architecture (only ppc64le and aarch64)");
#endif
  if (!DYNINST_ENABLE_DYNAMIC_TESTS) {
    return skip("runs a live process, which DYNINST_ENABLE_DYNAMIC_TESTS=OFF turns off");
  }
  if (argc == 1) {
    return skip("needs the test binaries (DYNINST_ENABLE_TESTS=ALL)");
  }
  std::string path = argv[1];
  if (access(path.c_str(), F_OK) != 0) {
    // A test_binaries clone is made once per build directory, so it can predate the mutatee
    std::string reason =
        path + " is missing; the test_binaries checkout predates proccontrol/atomicSeqInnerBranch";
    return skip(reason.c_str());
  }
  std::signal(SIGALRM, on_alarm);
  alarm(60);

  st::Symtab *symtab = nullptr;
  Dyninst::Offset bp_off = 0, reserve_off = 0, after_store_off = 0;
  if (!st::Symtab::openFile(symtab, path) || !find_offset(symtab, "inner_seq", bp_off) ||
      !find_offset(symtab, "inner_seq_reserve", reserve_off) ||
      !find_offset(symtab, "inner_seq_after_store", after_store_off)) {
    std::printf("FAIL: no inner_seq symbols in %s\n", path.c_str());
    return EXIT_FAILURE;
  }

  pc::Process::registerEventCallback(pc::EventType::Breakpoint, on_bp);
  pc::Process::registerEventCallback(pc::EventType::SingleStep, on_step);
  pc::Process::registerEventCallback(pc::EventType(pc::EventType::Post, pc::EventType::Exit),
                                     on_exit);

  pc::Process::ptr proc = pc::Process::createProcess(path, {path});
  if (!proc) {
    std::printf("FAIL: createProcess %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  Dyninst::Address base = proc->libraries().getExecutable()->getLoadAddress();
  Dyninst::Address bp_addr = base + bp_off;
  reserve_addr = base + reserve_off;
  after_store_addr = base + after_store_off;
  pc::Breakpoint::ptr bp = pc::Breakpoint::newBreakpoint();
  if (!proc->addBreakpoint(bp_addr, bp) || !proc->continueProc()) {
    std::printf("FAIL: could not set the breakpoint at 0x%lx\n", (unsigned long)bp_addr);
    return EXIT_FAILURE;
  }

  phase = "running to the breakpoint";
  poll_events([] { return at_bp; }, 30);
  if (!at_bp) {
    std::printf("FAIL: never reached the breakpoint at 0x%lx\n", (unsigned long)bp_addr);
    return EXIT_FAILURE;
  }
  pc::Thread::ptr thr = proc->threads().getInitialThread();
  if (!proc->rmBreakpoint(bp_addr, bp)) {
    std::printf("FAIL: rmBreakpoint\n");
    return EXIT_FAILURE;
  }

  // One step onto the reserve, then one emulated step over the whole sequence
  phase = "single-stepping the sequence";
  thr->setSingleStepMode(true);
  thr->continueThread();
  poll_events(step_done, 10);

  std::size_t at_reserve = 0;
  for (Dyninst::Address a : steps) {
    if (a == reserve_addr) {
      at_reserve++;
    }
  }
  bool ok = steps.size() == 2 && steps[0] == reserve_addr && steps[1] == after_store_addr;
  if (!ok) {
    std::printf("FAIL: %zu step(s), %zu on the reserve 0x%lx, last at 0x%lx; expected the "
                "reserve, then 0x%lx after the store-conditional\n",
                steps.size(), at_reserve, (unsigned long)reserve_addr,
                (unsigned long)(steps.empty() ? 0 : steps.back()),
                (unsigned long)after_store_addr);
    proc->terminate();
    return EXIT_FAILURE;
  }

  // Without single-step the mutatee runs to exit
  phase = "running to exit";
  thr->setSingleStepMode(false);
  thr->continueThread();
  poll_events([] { return false; }, 30);
  if (!exited || exit_code != 0) {
    std::printf("FAIL: the mutatee did not exit normally (exited=%d code=%d)\n", exited, exit_code);
    return EXIT_FAILURE;
  }
  std::printf("PASS\n");
  return EXIT_SUCCESS;
}
