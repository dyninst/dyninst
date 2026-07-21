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
extern __attribute__((visibility("default"))) __device__ HostcallQueue mailbox;
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

// --- bounded MPMC ring helpers (one lane per wave) ---------------------------
// hc_acquire: take a wait-free ticket (the ONLY global atomic), pick this ticket's
// slot, and wait for the slot's generation gate (turn) to reach my ticket — i.e. the
// previous occupant of this slot has been serviced and released it. Returns the slot;
// the ticket is stashed in turn (turn==pos while we own it) so release needs no arg.
static __device__ inline HostcallSlot* hc_acquire() {
    uint32_t pos = atomicAdd(&mailbox.enqueue_pos, 1u);
    HostcallSlot* s = &mailbox.slots[pos & (HC_NSLOTS - 1)];
    while (*(volatile uint32_t*)&s->turn != pos)   // slot busy with an older generation
        __builtin_amdgcn_s_sleep(2);
    return s;
}
static __device__ inline void hc_call_and_wait(HostcallSlot* s, int op) {
    s->opcode = op;
    // Marshalling stores (path/mode/data) are wave-uniform, so the compiler emits
    // SCALAR stores (s_store_*) that sit in the r/w scalar cache and are NOT written
    // back by __threadfence_system (which flushes the vector L1/L2 path only). Without
    // this the host sees stale zeros. s_dcache_wb writes the scalar cache back; the
    // fence then does the system-scope release ordering it before the status signal.
    __builtin_amdgcn_s_dcache_wb();
    __threadfence_system();                 // publish args before signalling
    *(volatile int*)&s->status = 1;         // request ready
    while (*(volatile int*)&s->status != 2)
        __builtin_amdgcn_s_sleep(2);        // wait for CPU (services slots out of order)
    __builtin_amdgcn_s_dcache_inv();        // drop stale scalar-cached reads
    __threadfence_system();                 // acquire CPU's results
}
// Release: mark idle, then hand the slot to the next generation. turn still holds our
// ticket `pos`, so the next occupant is pos+HC_NSLOTS. atomicExch (an atomic write)
// publishes it to other waves the same way the old ticket_now atomicAdd did.
static __device__ inline void hc_release(HostcallSlot* s) {
    *(volatile int*)&s->status = 0;
    uint32_t pos = *(volatile uint32_t*)&s->turn;
    (void)atomicExch(&s->turn, pos + HC_NSLOTS);
}

// --- hostcall: fopen(filename, mode) -> handle (<0 on error) ----------------
extern "C" __device__ __noinline__ __attribute__((used))
int64_t gpu_fopen(const char* filename, const char* mode) {
    int64_t h = -1;
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        int i = 0;
        #pragma unroll 1
        for (; i < HC_PATH_SIZE - 1 && filename && filename[i]; i++) s->path[i] = filename[i];
        s->path[i] = '\0';
        int j = 0;
        #pragma unroll 1
        for (; j < HC_MODE_SIZE - 1 && mode && mode[j]; j++) s->mode[j] = mode[j];
        s->mode[j] = '\0';
        hc_call_and_wait(s, HC_OP_FOPEN);
        h = s->handle;
        hc_release(s);
    }
    // Broadcast the elected lane's handle to the whole wave so the ABI return value
    // (v0:v1) is UNIFORM across lanes. Only the elected lane ran the handshake and set
    // h; without this, other lanes return the stale -1. A uniform return lets an
    // inserted call SITE capture the handle by reading the return registers on any
    // lane (readfirstlane picks the first active lane = the elected lane, which holds h).
    unsigned lo = __builtin_amdgcn_readfirstlane((unsigned)(unsigned long long)h);
    unsigned hi = __builtin_amdgcn_readfirstlane((unsigned)((unsigned long long)h >> 32));
    return (int64_t)(((unsigned long long)hi << 32) | (unsigned long long)lo);
}

// --- hostcall: fwrite(handle, data, size) -> bytes written ------------------
extern "C" __device__ __noinline__ __attribute__((used))
int gpu_fwrite(int64_t handle, const char* data, int size) {
    int n = 0;
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        if (size > HC_DATA_SIZE) size = HC_DATA_SIZE;
        #pragma unroll 1
        for (int i = 0; i < size; i++) s->data[i] = data[i];
        s->handle = handle;
        s->size   = size;
        hc_call_and_wait(s, HC_OP_FWRITE);
        n = s->retval;
        hc_release(s);
    }
    return n;
}

// --- hostcall: fread(handle, out, size) -> bytes read -----------------------
extern "C" __device__ __noinline__ __attribute__((used))
int gpu_fread(int64_t handle, char* out, int size) {
    int n = 0;
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        if (size > HC_DATA_SIZE) size = HC_DATA_SIZE;
        s->handle = handle;
        s->size   = size;
        hc_call_and_wait(s, HC_OP_FREAD);
        n = s->retval;
        #pragma unroll 1
        for (int i = 0; i < n && i < size; i++) out[i] = s->data[i];
        hc_release(s);
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
        HostcallSlot* s = hc_acquire();
        const char fn[] = "dyninst_trace.txt";
        #pragma unroll 1
        for (int i = 0; i < (int)sizeof(fn); i++) s->path[i] = fn[i];
        s->mode[0] = 'w';
        s->mode[1] = '\0';
        hc_call_and_wait(s, HC_OP_FOPEN);
        hc_release(s);
    }
}

extern "C" __device__ __noinline__ __attribute__((used))
void hc_write() {
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        const char msg[] = "[gpu] function entered\n";
        int n = 0;
        #pragma unroll 1
        for (; n < (int)sizeof(msg) - 1; n++) s->data[n] = msg[n];
        s->size = n;
        hc_call_and_wait(s, HC_OP_FWRITE);
        hc_release(s);
    }
}

// Like hc_write, but takes a per-site scalar id as a CALL ARGUMENT (demonstrates
// dyninst AMDGPU argument passing). `id` arrives in the CC arg register v0; it is
// uniform per wave (the trampoline materialises the same immediate into every
// lane). readfirstlane makes that explicit (a true scalar) before marshalling.
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_id(int id) {
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane(id);
        hc_call_and_wait(s, HC_OP_WRITE_ID);
        hc_release(s);
    }
}

extern "C" __device__ __noinline__ __attribute__((used))
void hc_close() {
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        hc_call_and_wait(s, HC_OP_FCLOSE);
        hc_release(s);
    }
}

// --- NON-LEAF wrapper (call-ABI test target) --------------------------------
// hc_nl_helper is a genuinely non-inlined device function with its OWN stack
// frame (local array forced to memory), so hc_write_nl becomes NON-LEAF: it must
// set up an s[0:3]+s32 buffer-scratch frame and s_swappc into the helper. This is
// the target for validating dyninst's inserted-call ABI: an inserted call to
// hc_write_nl must provide the scratch descriptor (s[0:3]) and stack pointer (s32)
// so the helper's frame works. hc_nl_helper runs under the full (caller) EXEC so
// multiple lanes exercise the frame; lane 0 then logs the computed result.
// Expected hc_nl_helper(id) for id=0..7: 21,37,53,69,85,45,61,77.
extern "C" __device__ __noinline__ __attribute__((used))
int hc_nl_helper(int x) {
    volatile int a[8];
    #pragma unroll 1
    for (int i = 0; i < 8; i++) a[i] = x + i * 7;
    return a[x & 7] + a[(x + 3) & 7];
}

extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_nl(int id) {
    int v = hc_nl_helper(id);                       // non-inlined call => hc_write_nl is NON-LEAF
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane(v);
        hc_call_and_wait(s, HC_OP_WRITE_ID);           // reuse WRITE_ID op: log the computed value
        hc_release(s);
    }
}

// --- IMPLICIT-ARG wrapper (Phase 3a: blockIdx forwarding test target) --------
// Reads blockIdx.x (workgroup-id x) — an IMPLICIT ABI argument the compiler passes
// to a device function in a fixed SGPR (empirically s12 on gfx908). A dyninst-
// inserted s_swappc sets up NONE of the ABI context, so without implicit-arg
// forwarding this SGPR holds an arbitrary kernel value -> wrong (or 0). With
// DYNINST_IMPLICIT_ARGS the trampoline forwards the kernel's workgroup-id into the
// callee's blockIdx register before the call, so this logs the correct per-block
// index. Validated with multiple workgroups (HOSTCALL_WG): block b logs b.
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_bid(int /*site (unused): we log blockIdx.x instead)*/) {
    int bx = (int)__builtin_amdgcn_workgroup_id_x();
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane(bx);
        hc_call_and_wait(s, HC_OP_WRITE_ID);           // reuse WRITE_ID op: log blockIdx.x
        hc_release(s);
    }
}

// --- IMPLICIT-ARG wrapper (Phase 3b: blockDim/gridDim forwarding test target) --
// Reads blockDim.x (workgroup size) — an implicit ABI input a device function
// obtains by dereferencing an implicit POINTER (dispatch ptr or kernarg ptr,
// depending on COV/toolchain), which a dyninst-inserted call doesn't set up. With
// 3b forwarding the trampoline supplies that pointer so this logs the true block
// dimension. Logs blockDim.x (expect the launch's workgroup size, e.g. 64).
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_bdim(int /*site (unused): we log blockDim.x instead)*/) {
    int bd = (int)blockDim.x;
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane(bd);
        hc_call_and_wait(s, HC_OP_WRITE_ID);           // reuse WRITE_ID op: log blockDim.x
        hc_release(s);
    }
}

// --- IMPLICIT-ARG wrapper (Phase 3c: threadIdx forwarding test target) --------
// Reads threadIdx.x (workitem-id x) — a PER-LANE implicit ABI input packed in v31
// (x=[9:0]) at a device function's entry, set up by the caller only at kernel
// entry (v0) and clobbered mid-kernel. With 3c forwarding the trampoline captures
// v0 at entry and restores it to v31 per-lane before the call. We log the elected
// (first active) lane's threadIdx.x, so per wave it should equal 64*wave_index
// (0,64,128,... — proving the value is PER-LANE, not a broadcast constant).
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_tid(int /*site (unused): we log threadIdx.x instead)*/) {
    int tx = (int)threadIdx.x;                      // per-lane; reads v31 & 0x3ff
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane(tx);
        hc_call_and_wait(s, HC_OP_WRITE_ID);           // reuse WRITE_ID op: log threadIdx.x
        hc_release(s);
    }
}

// --- ARBITRARY HIP HOOK using forwarded ABI context (milestone 2: flattened ids) --
// Computes the flattened WAVE id from the flattened global thread id (ftid). This is
// ordinary HIP device code combining THREE implicit ABI inputs — blockIdx.x (s12, via
// 3a), blockDim.x (implicit-args ptr, via 3b), and threadIdx.x (v31, via 3c) — all of
// which the inserted-call ABI forwards. No new dyninst codegen: the hook just uses the
// context Phase 3 supplies. ftid = blockIdx.x*blockDim.x + threadIdx.x (1D grid);
// flattened wave id = ftid/64 (a wavefront is 64 consecutive ftids, uniform per wave).
extern "C" __device__ __noinline__ __attribute__((used))
void hc_write_ftid(int /*site (unused)*/) {
    unsigned ftid = blockIdx.x * blockDim.x + threadIdx.x;   // flattened global thread id
    unsigned wid  = ftid / 64u;                              // flattened wave id (uniform/wave)
    if (hc_first_active_lane()) {
        HostcallSlot* s = hc_acquire();
        s->arg = __builtin_amdgcn_readfirstlane((int)wid);
        hc_call_and_wait(s, HC_OP_WRITE_ID);           // reuse WRITE_ID op: log flattened wave id
        hc_release(s);
    }
}
