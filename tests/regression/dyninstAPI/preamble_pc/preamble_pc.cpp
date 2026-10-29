#include "BPatch.h"
#include "BPatch_frame.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_point.h"
#include "BPatch_process.h"
#include "BPatch_snippet.h"
#include "BPatch_thread.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// In create mode the mutatee stops at main's global entry point (main + 0).
// Instrumenting main relocates it, and the relocated copy skips the TOC
// preamble (the first two instructions, which set r2 from r12). The stopped
// thread's PC was moved into the relocated code past that preamble, so main
// ran with the caller's r2 and crashed on its first TOC-relative access.
// The PC is only moved after the mutator has walked the stopped thread's
// stack, as parseThat does, so the test walks it first.

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
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <mutatee> <RT library directory>\n";
    return EXIT_FAILURE;
  }

  // BPatch preloads the RT library by searching LD_LIBRARY_PATH. Prepend the
  // build's RT directory to the inherited value instead of replacing it.
  std::string ld_path = argv[2];
  if (char const *inherited = getenv("LD_LIBRARY_PATH")) {
    if (*inherited) ld_path += std::string(":") + inherited;
  }
  setenv("LD_LIBRARY_PATH", ld_path.c_str(), 1);
  std::cout << "LD_LIBRARY_PATH=" << ld_path << "\n";

  if (!ptrace_allowed()) {
    std::cout << "skipping: ptrace is not permitted here\n";
    return 77;
  }

  BPatch bpatch;
  char const *mutatee_argv[] = {argv[1], nullptr};
  BPatch_process *proc = bpatch.processCreate(argv[1], mutatee_argv);
  if (!proc) {
    std::cerr << "Unable to create '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  BPatch_image *image = proc->getImage();
  std::vector<BPatch_function *> mains;
  image->findFunction("main", mains);
  BPatch_variableExpr *entered = image->findVariable("entered");
  if (mains.size() != 1 || !entered) {
    std::cerr << "Unable to find main and 'entered' in the mutatee\n";
    proc->terminateExecution();
    return EXIT_FAILURE;
  }
  std::vector<BPatch_point *> *entry = mains[0]->findPoint(BPatch_entry);
  if (!entry || entry->empty()) {
    std::cerr << "Unable to find main's entry point\n";
    proc->terminateExecution();
    return EXIT_FAILURE;
  }

  // Walk the stack and check that the thread is stopped at main's global entry.
  std::vector<BPatch_thread *> threads;
  proc->getThreads(threads);
  std::vector<BPatch_frame> stack;
  if (threads.size() != 1 || !threads[0]->getCallStack(stack) || stack.empty()) {
    std::cerr << "Unable to walk the stack of the stopped mutatee\n";
    proc->terminateExecution();
    return EXIT_FAILURE;
  }
  if (stack[0].getPC() != mains[0]->getBaseAddr()) {
    std::cerr << "Expected the mutatee to be stopped at main (" << mains[0]->getBaseAddr()
              << "), not at " << stack[0].getPC() << "\n";
    proc->terminateExecution();
    return EXIT_FAILURE;
  }

  BPatch_arithExpr increment(BPatch_assign, *entered,
                             BPatch_arithExpr(BPatch_plus, *entered, BPatch_constExpr(1)));
  if (!proc->insertSnippet(increment, *entry)) {
    std::cerr << "insertSnippet failed\n";
    proc->terminateExecution();
    return EXIT_FAILURE;
  }

  if (!proc->continueExecution()) {
    std::cerr << "continueExecution failed\n";
    return EXIT_FAILURE;
  }
  while (!proc->isTerminated()) {
    bpatch.waitForStatusChange();
  }

  if (proc->terminationStatus() != ExitedNormally || proc->getExitCode() != 0) {
    std::cerr << "Expected the mutatee to exit normally with 0; termination "
              << "status " << proc->terminationStatus() << ", exit code "
              << proc->getExitCode() << ", signal " << proc->getExitSignal() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
