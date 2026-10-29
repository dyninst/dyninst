// Walk both threads of undefined_ra_target and check that each walk ends at
// the outermost frame of its stack (issue #2378).
//
// worker: glibc's thread_start marks the return address undefined. The walk
//         must stop there; reading the RA as same-value instead repeats
//         thread_start's frame forever. thread_start and start_thread are
//         local symbols of libc, which distributions often strip, so the
//         bottom is checked by library: the two frames below worker()
//         (start_thread, thread_start) must both be in libc.
// main:   the walk must reach _start (in the target executable).
//
// Exit codes: 0 pass, 1 fail, 77 skip (prints "skipped: <reason>").

#include "dyninst_test_skip.h"
#include "frame.h"
#include "swk_errors.h"
#include "walker.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#if defined(__aarch64__) && defined(__GLIBC__)
#include <gnu/libc-version.h>
#endif

namespace sw = Dyninst::Stackwalker;

namespace {

// Only on AArch64 with glibc >= 2.34 does the worker's walk reach thread_start.
// Older glibc keeps start_thread in libpthread, where the bottom-of-stack
// stepper stops the walk first.
bool walk_reaches_thread_start() {
#if defined(__aarch64__) && defined(__GLIBC__)
    int major = 0, minor = 0;
    if (std::sscanf(gnu_get_libc_version(), "%d.%d", &major, &minor) != 2)
        return false;
    return major > 2 || (major == 2 && minor >= 34);
#else
    return false;
#endif
}

std::string frame_name(sw::Frame const &f) {
    std::string name;
    f.getName(name);
    return name;
}

bool in_libc(sw::Frame const &f) {
    std::string lib;
    Dyninst::Offset off = 0;
    void *symtab = nullptr;
    if (!f.getLibOffset(lib, off, symtab))
        return false;
    return lib.compare(lib.rfind('/') + 1, 7, "libc.so") == 0;
}

// The walk must succeed without repeating a frame. For the worker, exactly two
// frames below worker(), both in libc; for main, ending at _start.
bool check_walk(sw::Walker *walker, Dyninst::THR_ID tid, bool is_main) {
    std::vector<sw::Frame> frames;
    bool const walked = walker->walkStack(frames, tid);

    bool repeats = false;
    for (size_t i = 1; i < frames.size(); i++) {
        if (frames[i].getRA() == frames[i - 1].getRA() &&
            frames[i].getSP() == frames[i - 1].getSP() &&
            frames[i].getFP() == frames[i - 1].getFP())
            repeats = true;
    }

    std::fprintf(stderr, "%s thread %lu: walk %s, %zu frames%s\n", is_main ? "main" : "worker",
                 static_cast<unsigned long>(tid), walked ? "ok" : "FAILED", frames.size(),
                 repeats ? ", repeated frames" : "");
    for (sw::Frame const &f : frames)
        std::fprintf(stderr, "  '%s'%s\n", frame_name(f).c_str(), in_libc(f) ? " (libc)" : "");
    if (!walked)
        std::fprintf(stderr, "  error: %s\n", sw::getLastErrorMsg());
    if (!walked || repeats || frames.empty())
        return false;

    if (is_main)
        return frame_name(frames.back()) == "_start";

    size_t w = 0;
    while (w < frames.size() && frame_name(frames[w]) != "worker")
        w++;
    if (frames.size() - w != 3)
        return false;
    return in_libc(frames[w + 1]) && in_libc(frames[w + 2]);
}

}  // namespace

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <undefined_ra_target>\n", argv[0]);
        return 1;
    }
    if (!walk_reaches_thread_start()) {
        return skip_message("needs AArch64 with glibc >= 2.34");
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
                ok &= check_walk(walker, tid, tid == static_cast<Dyninst::THR_ID>(pid));
            }
        }
        delete walker;
    }

    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    std::printf("%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
