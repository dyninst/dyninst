// Device-only hostcall library: three GPU->CPU hostcalls (fopen/fread/fwrite).
//
// Compiled to a standalone code object with `hipcc --genco`. `mailbox` is an
// UNDEFINED external data symbol (default visibility) — it is NOT defined here;
// it is resolved at load/link time (by dyninst relocation, or by the HSA
// loader's variable-define) to a host-coherent fine-grained buffer.
//
// Each hostcall runs on lane 0 of the wavefront, guarded by a ticket lock so
// concurrent wavefronts serialize their access to the single shared mailbox.
//
// Build contract (MUST match the mutatee it is linked into):
//   --offload-arch=gfx908 -mcode-object-version=6   => ABI ver 4, flags 0x530
// Use the GENERIC gfx908 target-id (NOT :sramecc+:xnack-, which yields 0xE30 and
// would not match). scratch mode / xnack / code-object version must all be
// identical across the caller kernel and this library.

#include "hip/hip_runtime.h"
#include "hostcalls.h"

// Undefined external — resolved at link/load time. default visibility so the
// device linker leaves it UND (see -Xoffload-linker --unresolved-symbols=ignore-all).
// MUST be a pure `extern` DECLARATION, not a tentative definition. If this TU
// *defines* `mailbox` (i.e. drop the `extern` storage class), the compiler treats
// it as a local zero-initialized global whose contents it fully knows, and then
// CONSTANT-FOLDS the ticket/status handshake: ticket_next starts 0 -> atomicAdd
// returns 0 -> ticket_now starts 0 -> the acquire spin `ticket_now != 0` is dead,
// and the whole critical section collapses to a single `opcode = HC_OP_FOPEN`
// store (verified via rocgdb single-step: hc_open shrank to an 88-byte stub that
// issued no hostcall -> CPU serviced 0). Leaving it `extern` (undefined here,
// resolved at load time by hsa_executable_agent_global_variable_define) makes it
// opaque, so real atomics/loads/stores/spins are emitted. See [[hostcall-separated-lib-feasible]].
extern "C" {
extern __attribute__((visibility("default"))) __device__ HostcallMailbox mailbox;
}

// Elect exactly ONE lane of the wavefront to perform the hostcall.
//
// We MUST NOT use threadIdx.x here: threadIdx maps to the workitem-id ABI
// register (v31), which is only set up by the compiler when a *kernel* calls a
// function. A dyninst-INSERTED call emits a raw S_SWAPPC and does NOT set v31,
// so threadIdx.x is garbage and `(threadIdx.x & 63)==0` passes for many lanes ->
// the intra-wave ticket lock deadlocks. Instead pick the first ACTIVE lane from
// the exec mask via mbcnt, which is always valid regardless of ABI setup.
static __device__ inline bool hc_first_active_lane() {
    unsigned long long ex = __builtin_amdgcn_read_exec();
    unsigned lo  = __builtin_amdgcn_mbcnt_lo((unsigned)ex, 0u);
    unsigned pos = __builtin_amdgcn_mbcnt_hi((unsigned)(ex >> 32), lo);
    return pos == 0;
}

// --- ticket-lock critical-section helpers (one lane per wave) ----------------
static __device__ inline void hc_acquire() {
    uint32_t my = atomicAdd(&mailbox.ticket_next, 1);
    while (*(volatile uint32_t*)&mailbox.ticket_now != my)
        __builtin_amdgcn_s_sleep(2);
}
static __device__ inline void hc_call_and_wait(int op) {
    mailbox.opcode = op;
    // Marshalling stores (path/mode/data) are wave-uniform, so the compiler
    // emits SCALAR stores (s_store_*), which sit in the read/write scalar cache
    // and are NOT written back to system memory by __threadfence_system (that
    // flushes the vector L1/L2 path only). Without this the host sees stale
    // zeros for the path. s_dcache_wb writes the scalar cache back; the fence
    // then does the system-scope release ordering it before the status signal.
    __builtin_amdgcn_s_dcache_wb();
    __threadfence_system();                 // publish args before signalling
    *(volatile int*)&mailbox.status = 1;    // request ready
    while (*(volatile int*)&mailbox.status != 2)
        __builtin_amdgcn_s_sleep(2);        // wait for CPU
    __builtin_amdgcn_s_dcache_inv();        // drop stale scalar-cached reads
    __threadfence_system();                 // acquire CPU's results
}
static __device__ inline void hc_release() {
    *(volatile int*)&mailbox.status = 0;
    atomicAdd(&mailbox.ticket_now, 1);      // let next waiter in
}

// --- hostcall: fopen(filename, mode) -> handle (<0 on error) ----------------
extern "C" __device__ __noinline__ __attribute__((used))
int64_t gpu_fopen(const char* filename, const char* mode) {
    int64_t h = -1;
    if (hc_first_active_lane()) {
        hc_acquire();
        int i = 0;
        #pragma unroll 1
        for (; i < HC_PATH_SIZE - 1 && filename && filename[i]; i++) mailbox.path[i] = filename[i];
        mailbox.path[i] = '\0';
        int j = 0;
        #pragma unroll 1
        for (; j < HC_MODE_SIZE - 1 && mode && mode[j]; j++) mailbox.mode[j] = mode[j];
        mailbox.mode[j] = '\0';
        hc_call_and_wait(HC_OP_FOPEN);
        h = mailbox.handle;
        hc_release();
    }
    return h;
}

// --- hostcall: fwrite(handle, data, size) -> bytes written ------------------
extern "C" __device__ __noinline__ __attribute__((used))
int gpu_fwrite(int64_t handle, const char* data, int size) {
    int n = 0;
    if (hc_first_active_lane()) {
        hc_acquire();
        if (size > HC_DATA_SIZE) size = HC_DATA_SIZE;
        #pragma unroll 1
        for (int i = 0; i < size; i++) mailbox.data[i] = data[i];
        mailbox.handle = handle;
        mailbox.size   = size;
        hc_call_and_wait(HC_OP_FWRITE);
        n = mailbox.retval;
        hc_release();
    }
    return n;
}

// --- hostcall: fread(handle, out, size) -> bytes read -----------------------
extern "C" __device__ __noinline__ __attribute__((used))
int gpu_fread(int64_t handle, char* out, int size) {
    int n = 0;
    if (hc_first_active_lane()) {
        hc_acquire();
        if (size > HC_DATA_SIZE) size = HC_DATA_SIZE;
        mailbox.handle = handle;
        mailbox.size   = size;
        hc_call_and_wait(HC_OP_FREAD);
        n = mailbox.retval;
        #pragma unroll 1
        for (int i = 0; i < n && i < size; i++) out[i] = mailbox.data[i];
        hc_release();
    }
    return n;
}

// --- nullary trace wrappers -------------------------------------------------
// dyninst's AMDGPU emitCall cannot pass arguments yet, so anything it INSERTS
// must be a no-argument function. These wrap the fopen/fwrite/fclose hostcalls
// with fixed values; the host server keeps a single trace FILE* across the
// open -> write... -> close sequence (fopen sets it, fwrite/fclose use it).

extern "C" __device__ __noinline__ __attribute__((used))
void hc_open() {
    if (hc_first_active_lane()) {
        hc_acquire();
        const char fn[] = "dyninst_trace.txt";
        #pragma unroll 1
        for (int i = 0; i < (int)sizeof(fn); i++) mailbox.path[i] = fn[i];
        mailbox.mode[0] = 'w';
        mailbox.mode[1] = '\0';
        hc_call_and_wait(HC_OP_FOPEN);
        hc_release();
    }
}

extern "C" __device__ __noinline__ __attribute__((used))
void hc_write() {
    if (hc_first_active_lane()) {
        hc_acquire();
        const char msg[] = "[gpu] function entered\n";
        int n = 0;
        #pragma unroll 1
        for (; n < (int)sizeof(msg) - 1; n++) mailbox.data[n] = msg[n];
        mailbox.size = n;
        hc_call_and_wait(HC_OP_FWRITE);
        hc_release();
    }
}

// Like hc_write, but takes a per-site scalar id as a CALL ARGUMENT (demonstrates
// dyninst AMDGPU argument passing). `id` arrives in the CC arg register v0; it is
// uniform per wave (the trampoline materialises the same immediate into every
// lane). readfirstlane makes that explicit (a true scalar) before marshalling.
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_id(int id) {
    if (hc_first_active_lane()) {
        hc_acquire();
        mailbox.arg = __builtin_amdgcn_readfirstlane(id);
        hc_call_and_wait(HC_OP_WRITE_ID);
        hc_release();
    }
}

extern "C" __device__ __noinline__ __attribute__((used))
void hc_close() {
    if (hc_first_active_lane()) {
        hc_acquire();
        hc_call_and_wait(HC_OP_FCLOSE);
        hc_release();
    }
}
