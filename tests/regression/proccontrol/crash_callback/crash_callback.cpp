#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_thread.h"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

// A mutator that polls (pollForStatusChange, as parseThat does) must get its
// exit callback when the mutatee crashes. ProcControl processes the crash on
// its handler thread and defers the callback to the user thread; that
// deferred Crash event used to be dropped because the process had already
// exited, so the exit callback never ran and the process was never marked
// as having exited via a signal.

namespace {
  // Whether this process may trace a child at all; containers often refuse.
  bool ptrace_allowed() {
    pid_t pid = fork();
    if (pid == 0) {
      _exit(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) == 0 ? 0 : 1);
    }
    int status = 0;
    return pid > 0 && waitpid(pid, &status, 0) == pid && WIFEXITED(status)
        && WEXITSTATUS(status) == 0;
  }

  bool exit_seen = false;
  BPatch_exitType exit_type = NoExit;

  void on_exit(BPatch_thread *, BPatch_exitType type) {
    exit_seen = true;
    exit_type = type;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <mutatee>\n";
    return EXIT_FAILURE;
  }

  if (!ptrace_allowed()) {
    std::cerr << "ptrace is not permitted here; skipping\n";
    return 77;
  }

  BPatch bpatch;
  bpatch.registerExitCallback(on_exit);
  char const *mutatee_argv[] = {argv[1], nullptr};
  BPatch_process *proc = bpatch.processCreate(argv[1], mutatee_argv);
  if (!proc) {
    std::cerr << "Unable to create '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  if (!proc->continueExecution()) {
    std::cerr << "continueExecution failed\n";
    return EXIT_FAILURE;
  }

  auto const deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);
  while (!exit_seen) {
    if (std::chrono::steady_clock::now() > deadline) {
      std::cerr << "No exit callback within 60 s after the mutatee crashed "
                << "(isTerminated " << proc->isTerminated() << ", termination status "
                << proc->terminationStatus() << ")\n";
      return EXIT_FAILURE;
    }
    bpatch.pollForStatusChange();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (exit_type != ExitedViaSignal || proc->getExitSignal() != SIGSEGV) {
    std::cerr << "Expected the mutatee to exit via SIGSEGV; exit type " << exit_type
              << ", signal " << proc->getExitSignal() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
