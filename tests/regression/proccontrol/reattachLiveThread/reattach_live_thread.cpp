/*
 * Detach from a created process with Process::temporaryDetach() while a worker
 * thread is alive, then attach again with Process::reAttach(). A thread
 * ProcControl saw created keeps its "created attached" status across the
 * detach, so reAttach() did not ptrace-attach it. The thread then never
 * reported a stop, the process never left startup, and reAttach() blocked
 * forever.
 *
 * Passes when reAttach() returns, both threads are back under ProcControl's
 * control (their registers can be read once the process is stopped), and the
 * process can be terminated. A watchdog turns a hang into a failure.
 */
#include "Event.h"
#include "PCProcess.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace pc = Dyninst::ProcControlAPI;

namespace {

bool exited = false;
volatile pid_t mutatee_pid = -1;
const char *phase = "setup";

pc::Process::cb_ret_t on_exit(pc::Event::const_ptr) {
  exited = true;
  return pc::Process::cbDefault;
}

void on_alarm(int) {
  char msg[160];
  int n = snprintf(msg, sizeof msg, "FAIL: no progress in %s\n", phase);
  if (n > 0) {
    ssize_t r = write(STDOUT_FILENO, msg, static_cast<size_t>(n));
    (void)r;
  }
  if (mutatee_pid > 0) {
    kill(mutatee_pid, SIGKILL);
  }
  _exit(EXIT_FAILURE);
}

// Handle events without blocking until `done` holds or `secs` pass.
template <typename Done>
bool poll_events(Done done, int secs) {
  time_t t0 = time(nullptr);
  while (!done()) {
    if (time(nullptr) - t0 >= secs) {
      return false;
    }
    if (!pc::Process::handleEvents(false)) {
      usleep(10000);
    }
  }
  return true;
}

int fail(pc::Process::ptr const &proc, char const *what) {
  std::printf("FAIL: %s\n", what);
  if (proc && !proc->isTerminated()) {
    proc->terminate();
  }
  return EXIT_FAILURE;
}

}  // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    std::fprintf(stderr, "Usage: %s mutatee\n", argv[0]);
    return EXIT_FAILURE;
  }
  std::string path = argv[1];
  std::signal(SIGALRM, on_alarm);
  alarm(60);

  pc::Process::registerEventCallback(pc::EventType(pc::EventType::Post, pc::EventType::Exit),
                                     on_exit);

  pc::Process::ptr proc = pc::Process::createProcess(path, {path});
  if (!proc) {
    std::printf("FAIL: createProcess %s\n", path.c_str());
    return EXIT_FAILURE;
  }
  mutatee_pid = proc->getPid();

  phase = "starting the worker thread";
  if (!proc->continueProc()) {
    return fail(proc, "continueProc");
  }
  if (!poll_events([&] { return exited || proc->threads().size() >= 2; }, 30) || exited) {
    return fail(proc, "the worker thread was not created");
  }

  phase = "temporaryDetach";
  if (!proc->temporaryDetach()) {
    return fail(proc, "temporaryDetach");
  }
  usleep(100000);

  // Before the fix, this call never returned.
  phase = "reAttach";
  alarm(30);
  if (!proc->reAttach()) {
    return fail(proc, "reAttach");
  }

  phase = "checking the threads after reAttach";
  if (!proc->stopProc()) {
    return fail(proc, "stopProc after reAttach");
  }
  if (proc->threads().size() != 2) {
    std::printf("FAIL: %zu threads after reAttach, expected 2\n", proc->threads().size());
    proc->terminate();
    return EXIT_FAILURE;
  }
  Dyninst::MachRegister pc_reg = Dyninst::MachRegister::getPC(proc->getArchitecture());
  for (pc::ThreadPool::iterator i = proc->threads().begin(); i != proc->threads().end(); ++i) {
    pc::Thread::ptr thr = *i;
    Dyninst::MachRegisterVal val = 0;
    if (!thr->isLive() || !thr->isStopped() || !thr->getRegister(pc_reg, val)) {
      std::printf("FAIL: thread %d is not attached and stopped after reAttach\n",
                  static_cast<int>(thr->getLWP()));
      proc->terminate();
      return EXIT_FAILURE;
    }
  }

  phase = "terminate";
  if (!proc->terminate()) {
    std::printf("FAIL: terminate\n");
    return EXIT_FAILURE;
  }
  std::printf("PASS\n");
  return EXIT_SUCCESS;
}
