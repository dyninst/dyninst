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

#include <cstdint>
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
    // Per-wave slice layout (bytes): [0:7]=file handle  [8]=hits  [12]=lanes  [16..]=name/text.
    if (pos == 0) {
        char* b = (char*)slice;
        *(volatile int*)(b + 8) += 1;                 // hits: # probe sites this wave hit
        *(volatile int*)(b + 12) = __builtin_popcountll(ex);  // active lanes at last hit
    }
}

// Our fopen/fwrite runtime (defined in hostcall_lib.cpp, same code object). Match
// its exact signatures (int64_t handle) so the single-TU reg-usage compile agrees.
extern "C" __device__ int64_t gpu_fopen(const char* filename, const char* mode);
extern "C" __device__ int     gpu_fwrite(int64_t handle, const char* data, int size);

static __device__ int put_str(char* b, int i, const char* s) { while (*s) b[i++] = *s++; return i; }
static __device__ int put_uint(char* b, int i, unsigned v) {
    char t[10]; int n = 0;
    if (!v) t[n++] = '0';
    while (v) { t[n++] = char('0' + v % 10); v /= 10; }
    while (n) b[i++] = t[--n];
    return i;
}

// PER-WAVE OPEN: each wave opens its OWN file "wave_<wid>.txt" and stashes the
// returned handle in its slice, so later writes go to that file. gpu_fopen returns
// the handle only on its elected lane; we do the format+open+store under a single
// elected lane so the stored handle is the real one (not the -1 other lanes see).
// The filename is built in the (global) slice — gpu_fopen reads its path via flat
// loads, so a private/stack buffer would marshal garbage. Inserted at kernel ENTRY.
extern "C" __device__ __noinline__ __attribute__((used))
void pw_open(void* slice) {
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    if (pos != 0) return;
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;
    char* b = (char*)slice;
    int i = 16;                                        // name buffer at slice+16
    i = put_str(b, i, "wave_"); i = put_uint(b, i, wid); i = put_str(b, i, ".txt");
    b[i] = '\0';
    long long h = gpu_fopen(b + 16, "w");              // this wave's file
    *(volatile long long*)(b + 0) = h;                 // stash handle for pw_flush
    *(volatile int*)(b + 8)  = 0;                      // reset hits
    *(volatile int*)(b + 12) = 0;                      // reset lanes
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
    char* b = (char*)slice;
    long long h  = *(volatile long long*)(b + 0);     // this wave's file handle (from pw_open)
    int hits     = *(volatile int*)(b + 8);
    int lanes    = *(volatile int*)(b + 12);          // read stats BEFORE reformatting
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;
    // Format into the (global) slice text area; gpu_fwrite reads its data via flat
    // loads, so it must be global, not a private/stack buffer.
    int i = 16;
    i = put_str(b, i, "wave ");    i = put_uint(b, i, wid);
    i = put_str(b, i, ": hits=");  i = put_uint(b, i, (unsigned)hits);
    i = put_str(b, i, " lanes=");  i = put_uint(b, i, (unsigned)lanes);
    b[i++] = '\n';
    gpu_fwrite(h, b + 16, i - 16);                    // -> this wave's own file
}

// ---- COMPOSABLE model: per-wave variable HOLDS a call's return value ----------
// These demonstrate the general "per-wave variable holds the return value, later
// passed into fwrite as its first parameter" idea WITHOUT a hand-written wrapper doing
// the store: dyninst inserts `hv = pw_openfile()` (the call SITE captures the ABI
// return into the per-wave var) and later `pw_writeln(hv.value())` (the var's held
// value passed as an argument). The wrappers just follow the ABI return contract.

// Returns a fixed sentinel in the ABI return registers (uniform) — isolates call-site
// return capture from any file/string dependency, for validating the mechanism.
extern "C" __device__ __noinline__ __attribute__((used))
int64_t pw_magic() {
    return 0xABCDE;
}

// Returns a file handle in the ABI return registers (gpu_fopen broadcasts it uniform).
extern "C" __device__ __noinline__ __attribute__((used))
int64_t pw_openfile() {
    return gpu_fopen("captured.txt", "w");
}

// Writes one line to the file identified by `handle` (supplied from a per-wave var's
// value()). Shows the held handle used as fwrite's first parameter.
extern "C" __device__ __noinline__ __attribute__((used))
void pw_writeln(int64_t handle) {
    gpu_fwrite(handle, "hello from a wave\n", 18);
}

// ---- GLOBAL-BASE per-wave model (for the LD_PRELOAD path) ---------------------
// The dyninst per-wave variable delivers this wave's slice via an appended KERNARG
// slot that a host launcher fills. Under LD_PRELOAD the HIP runtime builds the kernarg
// itself and won't fill an appended slot, so instead we resolve a GLOBAL per-wave
// buffer base the same way as `mailbox` (host variable-define) and let each wave index
// its own slice by its logical flattened wave id. That makes these probes NULLARY —
// dyninst inserts no-arg calls (like hc_open/hc_close) — with no kernarg growth.
extern "C" { extern __attribute__((visibility("default"))) __device__ char g_pw_base[]; }
#define PWG_STRIDE 4096u   // bytes per wave slice (must match the host allocation stride)

static __device__ char* pwg_slice() {
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;   // logical flattened wid
    return g_pw_base + (unsigned long)wid * PWG_STRIDE;
}

// ENTRY: this wave opens its OWN file "wave_<wid>.txt" and stashes the handle in its
// slice. One elected lane does the open (gpu_fopen returns the real handle only there).
extern "C" __device__ __noinline__ __attribute__((used))
void pwg_open() {
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    if (pos != 0) return;
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;
    char* b = pwg_slice();
    int i = 16;                                        // name buffer at slice+16
    i = put_str(b, i, "wave_"); i = put_uint(b, i, wid); i = put_str(b, i, ".txt");
    b[i] = '\0';
    long long h = gpu_fopen(b + 16, "w");              // this wave's file
    *(volatile long long*)(b + 0) = h;                 // stash handle for pwg_flush
}

// EXIT: this wave writes one line to its own file and closes it.
extern "C" __device__ __noinline__ __attribute__((used))
void pwg_flush() {
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    if (pos != 0) return;
    unsigned wid = (blockIdx.x * blockDim.x + threadIdx.x) / 64u;
    char* b = pwg_slice();
    long long h = *(volatile long long*)(b + 0);       // this wave's handle (from pwg_open)
    int i = 16;
    i = put_str(b, i, "wave "); i = put_uint(b, i, wid); i = put_str(b, i, " reporting in\n");
    gpu_fwrite(h, b + 16, i - 16);                     // -> this wave's own file
}
