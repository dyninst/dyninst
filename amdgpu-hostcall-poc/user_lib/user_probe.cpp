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
    ((volatile int*)slice)[0] = 0xABCD1234;          // marker at this wave's slice[0]
}
