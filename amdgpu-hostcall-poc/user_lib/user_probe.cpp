// user_probe.cpp — a USER-WRITTEN device instrumentation library.
//
// Demonstrates the intended "CPU-like" Dyninst-on-AMDGPU workflow: the user
// writes ordinary device code (the probe/analysis body) that calls into the
// hostcall RUNTIME we provide (hc_write_id — declared here, defined in
// hostcall_lib.cpp). user_probe.cpp is compiled TOGETHER with hostcall_lib.cpp
// into ONE code object, so the user->runtime call is resolved by the device
// linker. Dyninst then instruments a mutatee to call `user_probe`; the user
// never touches the mailbox ABI, the trampoline, or any dyninst-facing detail.
//
// What a probe must satisfy to be a dyninst AMDGPU insertion target:
//   * nullary — dyninst inserts a no-argument call;
//   * __noinline__ + used — so it survives as a real, named, address-taken callee.
// The probe MAY be non-leaf: calling our wrappers makes it non-leaf, and the
// inserted-call ABI sets up the scratch frame + return-address register for it.

#include "hip/hip_runtime.h"

// The runtime we provide. DEFINED in hostcall_lib.cpp, which is compiled into the
// same code object; the device linker binds this call at --genco time.
extern "C" __device__ void hc_write_id(int id);

// USER analysis: count the active lanes at this program point (population count of
// the EXEC mask) and report it through the runtime. __builtin_amdgcn_read_exec is
// valid regardless of ABI setup, so this probe needs no implicit-arg forwarding.
extern "C" __device__ __noinline__ __attribute__((used))
void user_probe() {
    unsigned long long exec = __builtin_amdgcn_read_exec();
    int active_lanes = __builtin_popcountll(exec);   // user's metric
    hc_write_id(active_lanes);                        // our runtime wrapper
}

// PER-WAVE-BUFFER probe: dyninst passes THIS wave's slice of a per-wave buffer as
// the `slice` pointer (a BPatch_perWaveVar). The probe keeps per-wave state there —
// here it just writes a marker to prove the 64-bit pointer arg arrives and points at
// writable, per-wave-distinct memory the host can read back. A real probe would
// accumulate (e.g. a counter/histogram) and periodically flush via gpu_fwrite.
extern "C" __device__ __noinline__ __attribute__((used))
void pw_probe(void* slice) {
    // Elect the first active lane so exactly ONE lane per wave updates the slice
    // (each wave owns its slice, so a plain RMW is race-free — no atomics needed).
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    if (pos == 0) {
        volatile int* s = (volatile int*)slice;
        s[0] += 1;                                    // [0] = # probe sites this wave hit
        s[1]  = __builtin_popcountll(ex);             // [1] = active lanes at the last hit
    }
}

// Our fwrite runtime (defined in hostcall_lib.cpp, same code object).
extern "C" __device__ int gpu_fwrite(long long handle, const char* data, int size);

static __device__ int put_str(char* b, int i, const char* s) { while (*s) b[i++] = *s++; return i; }
static __device__ int put_uint(char* b, int i, unsigned v) {
    char t[10]; int n = 0;
    if (!v) t[n++] = '0';
    while (v) { t[n++] = char('0' + v % 10); v /= 10; }
    while (n) b[i++] = t[--n];
    return i;
}

// PER-WAVE FLUSH: read this wave's accumulated slice, format a human-readable line,
// and stream it to the trace file via gpu_fwrite (the fopen/fwrite hostcall path).
// Inserted at kernel EXIT, so each wave emits exactly one grouped line; the mailbox
// ticket lock serializes waves, so lines don't interleave. wid is the wave's logical
// flattened id, computed from the forwarded implicit ABI args (blockIdx/blockDim/
// threadIdx). Non-leaf (calls gpu_fwrite, has a local buffer) — inserted-call ABI.
extern "C" __device__ __noinline__ __attribute__((used))
void pw_flush(void* slice) {
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    if (pos != 0) return;                             // one lane per wave writes the line
    volatile int* s = (volatile int*)slice;
    int hits = s[0], lanes = s[1];                    // read BEFORE reformatting the slice
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;
    // Format into the SLICE itself (global device memory). gpu_fwrite reads its data
    // pointer via flat loads; a private/stack buffer would read back as garbage, but
    // the per-wave slice is global and per-wave-exclusive, so it's a safe scratch line.
    char* b = (char*)slice; int i = 0;
    i = put_str(b, i, "wave ");    i = put_uint(b, i, wid);
    i = put_str(b, i, ": hits=");  i = put_uint(b, i, (unsigned)hits);
    i = put_str(b, i, " lanes=");  i = put_uint(b, i, (unsigned)lanes);
    b[i++] = '\n';
    gpu_fwrite(0, b, i);                              // -> dyninst_trace.txt
}
