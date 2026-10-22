// Walk both threads of undefined_ra_target and check that each walk ends at
// the outermost frame of its stack (issue #2378).
//
// worker: glibc's thread_start marks the return address undefined. The walk
//         must stop there; reading the RA as same-value instead repeats
//         thread_start's frame forever.
// main:   the walk must reach _start.
//
// Exit codes: 0 pass, 1 fail, 77 skip (ctest SKIP_RETURN_CODE).

#include "frame.h"
#include "swk_errors.h"
#include "walker.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <gnu/libc-version.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace sw = Dyninst::Stackwalker;

namespace {

constexpr int skip = 77;

// Only on AArch64 with glibc >= 2.34 does the worker's walk reach thread_start.
// Older glibc keeps start_thread in libpthread, where the bottom-of-stack
// stepper stops the walk first.
bool walk_reaches_thread_start() {
#if defined(__aarch64__)
    int major = 0, minor = 0;
    if (std::sscanf(gnu_get_libc_version(), "%d.%d", &major, &minor) != 2)
        return false;
    return major > 2 || (major == 2 && minor >= 34);
#else
    return false;
#endif
}

bool check_walk(sw::Walker *walker, Dyninst::THR_ID tid, char const *thread,
                char const *bottom) {
    std::vector<sw::Frame> frames;
    bool const walked = walker->walkStack(frames, tid);

    bool repeats = false;
    for (size_t i = 1; i < frames.size(); i++) {
        if (frames[i].getRA() == frames[i - 1].getRA() &&
            frames[i].getSP() == frames[i - 1].getSP() &&
            frames[i].getFP() == frames[i - 1].getFP())
            repeats = true;
    }

    std::string last;
    if (!frames.empty())
        frames.back().getName(last);

    std::printf("%s thread %lu: walk %s, %zu frames, last '%s'%s\n", thread,
                static_cast<unsigned long>(tid), walked ? "ok" : "FAILED",
                frames.size(), last.c_str(), repeats ? ", repeated frames" : "");
    if (!walked)
        std::printf("  error: %s\n", sw::getLastErrorMsg());
    return walked && !repeats && last == bottom;
}

}  // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <undefined_ra_target>\n", argv[0]);
        return 1;
    }
    if (!walk_reaches_thread_start()) {
        std::printf("SKIP: needs AArch64 with glibc >= 2.34\n");
        return skip;
    }

    // The target writes one byte once its worker thread is running.
    int ready[2];
    if (pipe(ready) != 0) {
        std::perror("pipe");
        return 1;
    }
    pid_t const pid = fork();
    if (pid < 0) {
        std::perror("fork");
        return 1;
    }
    if (pid == 0) {
        close(ready[0]);
        dup2(ready[1], STDOUT_FILENO);
        execl(argv[1], argv[1], static_cast<char *>(nullptr));
        _exit(127);
    }
    close(ready[1]);
    char c;
    ssize_t n;
    do {
        n = read(ready[0], &c, 1);
    } while (n < 0 && errno == EINTR);
    if (n != 1) {
        std::fprintf(stderr, "target did not start\n");
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
        return 1;
    }

    bool ok = false;
    sw::Walker *walker = sw::Walker::newWalker(static_cast<Dyninst::PID>(pid));
    if (!walker) {
        std::fprintf(stderr, "attach failed: %s\n", sw::getLastErrorMsg());
    } else {
        std::vector<Dyninst::THR_ID> threads;
        walker->getAvailableThreads(threads);
        if (threads.size() != 2) {
            std::fprintf(stderr, "expected 2 threads, found %zu\n", threads.size());
        } else {
            ok = true;
            for (Dyninst::THR_ID tid : threads) {
                bool const is_main = (tid == static_cast<Dyninst::THR_ID>(pid));
                ok &= check_walk(walker, tid, is_main ? "main" : "worker",
                                 is_main ? "_start" : "thread_start");
            }
        }
        delete walker;
    }

    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    std::printf("%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
