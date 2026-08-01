// eh_hotcold: a try/catch inside a function the compiler hot/cold-splits, so the
// throwing call-site and the landing pad can land in different (non-contiguous)
// code fragments -- the .text.unlikely cold partition vs. the hot body. This
// exercises the generator's per-run FDE emission and its cross-run LSDA /
// landing-pad handling for a *single function* (distinct from eh_align, where the
// split coincides with a stack realign). Built at both -O2 and -O3 because the
// block-partitioning heuristics -- and thus the split shape -- differ by level.
//
// Prints "CAUGHT-6 ..." only on full success (right handler + destructor ran).

#include <cstdio>
#include <stdexcept>

// Marked cold + noinline: GCC pushes the call to this into its callers' cold
// (.text.unlikely) partition.
static __attribute__((noinline, cold)) void cold_throw(const char* w) {
    throw std::runtime_error(w);
}

struct Cleanup {
    bool* ran;
    ~Cleanup() { *ran = true; }   // must run while unwinding out of the cold path
};

static __attribute__((noinline)) int worker(int n) {
    bool cleaned = false;
    try {
        Cleanup c{&cleaned};
        long acc = 0;
        for (int i = 1; i <= n; ++i) acc += (long)i * i;   // hot loop
        // acc > 0 is ALWAYS true here (n >= 1), but predicting it false makes the
        // compiler compile the throwing edge as cold -- so the cold_throw call
        // site is in the split-out fragment while the try body / landing pad are
        // in the hot body. The edge is nonetheless taken at runtime, so it throws.
        if (__builtin_expect(acc > 0, 0))
            cold_throw("hotcold");
        std::printf("MISSED-6\n");                          // unreachable at runtime
        return 1;
    } catch (const std::exception& e) {
        if (!cleaned) { std::printf("NO-CLEANUP-6\n"); return 2; }
        std::printf("CAUGHT-6 %s\n", e.what());
        return 0;
    }
}

int main(int argc, char**) {
    return worker(argc + 5);
}
