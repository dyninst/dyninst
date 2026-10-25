/*
 * Turning single-step mode off while an emulated single step is in flight must stick.
 *
 * Power and AArch64 single-step a load-reserve/store-conditional sequence by letting the
 * thread run to breakpoints at the sequence's exits.  The thread is single-stepped into a
 * sequence that it cannot leave while a flag is 0, so the emulated step stays in flight.  The
 * process is stopped, the thread's single-step mode is turned off and the flag is set.  When
 * the thread reaches the exit breakpoint, the emulated step used to restore the single-step mode
 * it had saved when it started, and the thread went on single-stepping.
 *
 * Passes when no SingleStep event arrives after single-step mode was turned off and the
 * mutatee exits normally.  A SIGALRM watchdog turns a blocked event loop into a failure.
 */
#include "Event.h"
#include "PCProcess.h"
#include "Symbol.h"
#include "Symtab.h"

#include <chrono>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

namespace pc = Dyninst::ProcControlAPI;
namespace st = Dyninst::SymtabAPI;

namespace {

Dyninst::Address seq_start = 0, seq_end = 0;
bool at_bp = false, reached_seq = false, step_off = false, exited = false;
unsigned steps = 0, stray_steps = 0;
int exit_code = -1;
pc::Thread::ptr thr;
pc::Process::ptr proc;
volatile pid_t mutatee_pid = 0;

pc::Process::cb_ret_t on_bp(pc::Event::const_ptr) {
  at_bp = true;
  return pc::Process::cbProcStop;
}

pc::Process::cb_ret_t on_step(pc::Event::const_ptr ev) {
  steps++;
  if (step_off) {
    // The mode came back.  Turn it off again so that the mutatee can still finish.
    stray_steps++;
    thr->setSingleStepMode(false);
    return pc::Process::cbThreadContinue;
  }
  Dyninst::MachRegisterVal pc_val = 0;
  if (ev->getThread()->getRegister(
          Dyninst::MachRegister::getPC(ev->getProcess()->getArchitecture()), pc_val) &&
      pc_val == seq_start) {
    reached_seq = true;
  }
  return pc::Process::cbThreadContinue;
}

pc::Process::cb_ret_t on_exit(pc::Event::const_ptr ev) {
  exited = true;
  exit_code = ev->getEventExit()->getExitCode();
  return pc::Process::cbDefault;
}

const char *phase = "setup";

void on_alarm(int) {
  char msg[160];
  int n = snprintf(msg, sizeof msg, "FAIL: no progress in %s (handleEvents blocked)\n", phase);
  if (n > 0) {
    ssize_t r = write(STDOUT_FILENO, msg, static_cast<size_t>(n));
    (void)r;
  }
  if (mutatee_pid > 0) {
    kill(mutatee_pid, SIGKILL);
  }
  _exit(EXIT_FAILURE);
}

// Poll the event loop without blocking for up to `secs` or until `flag` is set.
void poll_events(const bool &flag, int secs) {
  auto end = std::chrono::steady_clock::now() + std::chrono::seconds(secs);
  while (!flag && !exited && std::chrono::steady_clock::now() < end) {
    if (!pc::Process::handleEvents(false)) {
      usleep(10000);
    }
  }
}

// Report a failure, with ProcControl's last error, and do not leave the mutatee behind.
__attribute__((format(printf, 1, 2))) int fail(const char *fmt, ...) {
  std::printf("FAIL: ");
  va_list ap;
  va_start(ap, fmt);
  std::vprintf(fmt, ap);
  va_end(ap);
  if (proc && proc->getLastError() != pc::err_none) {
    std::printf(" (ProcControl: %s)", proc->getLastErrorMsg());
  }
  std::printf("\n");
  if (proc && !proc->isTerminated()) {
    proc->terminate();
  }
  return EXIT_FAILURE;
}

bool lookup(st::Symtab *symtab, const char *name, Dyninst::Offset &off) {
  std::vector<st::Symbol *> syms;
  if (!symtab->findSymbol(syms, name, st::Symbol::ST_UNKNOWN) || syms.empty()) {
    std::printf("FAIL: no %s in the mutatee\n", name);
    return false;
  }
  off = syms[0]->getOffset();
  return true;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::fprintf(stderr, "Usage: %s mutatee\n", argv[0]);
    return EXIT_FAILURE;
  }
  std::string path = argv[1];
  std::signal(SIGALRM, on_alarm);
  alarm(90);

  st::Symtab *symtab = nullptr;
  Dyninst::Offset bp_off = 0, seq_off = 0, end_off = 0, flag_off = 0;
  if (!st::Symtab::openFile(symtab, path)) {
    std::printf("FAIL: cannot open %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  if (!lookup(symtab, "before_window", bp_off) || !lookup(symtab, "window_seq", seq_off) ||
      !lookup(symtab, "window_end", end_off) || !lookup(symtab, "flag", flag_off)) {
    return EXIT_FAILURE;
  }

  pc::Process::registerEventCallback(pc::EventType::Breakpoint, on_bp);
  pc::Process::registerEventCallback(pc::EventType::SingleStep, on_step);
  pc::Process::registerEventCallback(pc::EventType(pc::EventType::Post, pc::EventType::Exit),
                                     on_exit);

  proc = pc::Process::createProcess(path, {path});
  if (!proc) {
    std::printf("FAIL: createProcess %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  Dyninst::Address base = proc->libraries().getExecutable()->getLoadAddress();
  Dyninst::Address bp_addr = base + bp_off;
  seq_start = base + seq_off;
  seq_end = base + end_off;
  Dyninst::Address flag_addr = base + flag_off;
  mutatee_pid = proc->getPid();
  thr = proc->threads().getInitialThread();

  pc::Breakpoint::ptr bp = pc::Breakpoint::newBreakpoint();
  if (!proc->addBreakpoint(bp_addr, bp) || !proc->continueProc()) {
    return fail("could not set the breakpoint at 0x%lx", (unsigned long)bp_addr);
  }
  phase = "running to the breakpoint";
  poll_events(at_bp, 30);
  if (!at_bp) {
    return fail("never reached before_window()");
  }

  // Single-step into the sequence.  The step from the load-reserve is emulated: the thread runs
  // to the exit breakpoints and waits inside the sequence, so no further SingleStep arrives.
  phase = "stepping into the sequence";
  if (!proc->rmBreakpoint(bp_addr, bp) || !thr->setSingleStepMode(true) ||
      !thr->continueThread()) {
    return fail("could not start single-stepping");
  }
  poll_events(reached_seq, 30);
  if (!reached_seq) {
    return fail("the thread never stepped onto the sequence (%u steps)", steps);
  }
  bool never = false;
  poll_events(never, 1);

  phase = "stopping the process";
  if (!proc->stopProc()) {
    return fail("stopProc");
  }
  Dyninst::MachRegisterVal at = 0;
  if (!thr->getRegister(Dyninst::MachRegister::getPC(proc->getArchitecture()), at) ||
      at < seq_start || at >= seq_end) {
    return fail("the thread is not inside the sequence (pc=0x%lx, sequence 0x%lx-0x%lx)",
                (unsigned long)at, (unsigned long)seq_start, (unsigned long)seq_end);
  }

  // Turn single-stepping off with the emulated step in flight, set the flag, continue.
  phase = "running to exit with single-step off";
  int one = 1;
  step_off = true;
  if (!thr->setSingleStepMode(false) || !proc->writeMemory(flag_addr, &one, sizeof one) ||
      !proc->continueProc()) {
    return fail("could not turn single-step off, set the flag and continue");
  }
  poll_events(exited, 30);
  if (stray_steps) {
    return fail("%u SingleStep events after single-step mode was turned off", stray_steps);
  }
  if (!exited || exit_code != 0) {
    return fail("the mutatee did not exit normally (exited=%d code=%d)", exited, exit_code);
  }
  std::printf("PASS\n");
  return EXIT_SUCCESS;
}
