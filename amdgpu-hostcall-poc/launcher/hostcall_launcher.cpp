// hostcall_launcher — loads the dyninst-instrumented vectoradd code object and
// the device hostcall library into one HSA executable, resolves the cross-object
// relocations (the inserted .dyninst.hc_* calls + the shared `mailbox`), starts a
// CPU service thread for the GPU->CPU hostcalls, and dispatches the kernel.
//
// This is the runtime counterpart to the dyninst static-rewrite flow:
//   instrument.sh -> vectoradd.inst.co   (kernel with hc_open/hc_write/hc_close)
//   this launcher -> loads it + hostcall_lib.aliased.elf, freezes, runs, services
//
// Run with ROCR_VISIBLE_DEVICES=1 so the gfx908 MI100 is the only agent.

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include <atomic>
#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <unistd.h>

#include "hostcalls.h"   // shared mailbox ABI

#define HSA_CHECK(s) do {                                                      \
    hsa_status_t _s = (s);                                                     \
    if (_s != HSA_STATUS_SUCCESS) {                                            \
        const char* _m = nullptr; hsa_status_string(_s, &_m);                 \
        fprintf(stderr, "HSA error: %s at %s:%d\n", _m, __FILE__, __LINE__);   \
        exit(1);                                                               \
    }                                                                          \
} while (0)

// ---- agent / pool discovery -------------------------------------------------
struct AgentSearch { hsa_agent_t agent; bool found; };
static hsa_status_t find_gpu(hsa_agent_t a, void* d) {
    hsa_device_type_t t; hsa_agent_get_info(a, HSA_AGENT_INFO_DEVICE, &t);
    if (t != HSA_DEVICE_TYPE_GPU) return HSA_STATUS_SUCCESS;
    char n[64] = {}; hsa_agent_get_info(a, HSA_AGENT_INFO_NAME, n);
    if (strstr(n, "gfx908")) { auto* s=(AgentSearch*)d; s->agent=a; s->found=true; return HSA_STATUS_INFO_BREAK; }
    return HSA_STATUS_SUCCESS;
}
static hsa_status_t find_cpu(hsa_agent_t a, void* d) {
    hsa_device_type_t t; hsa_agent_get_info(a, HSA_AGENT_INFO_DEVICE, &t);
    if (t == HSA_DEVICE_TYPE_CPU) { auto* s=(AgentSearch*)d; s->agent=a; s->found=true; return HSA_STATUS_INFO_BREAK; }
    return HSA_STATUS_SUCCESS;
}
struct PoolSearch { hsa_amd_memory_pool_t pool; bool found; };
static hsa_status_t find_fine_grained(hsa_amd_memory_pool_t pool, void* d) {
    hsa_amd_segment_t seg;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &seg);
    if (seg != HSA_AMD_SEGMENT_GLOBAL) return HSA_STATUS_SUCCESS;
    uint32_t flags;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_FINE_GRAINED) {
        auto* s=(PoolSearch*)d; s->pool=pool; s->found=true; return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}
// Coarse-grained, ALLOCATABLE device-local (VRAM) pool — iterate the GPU agent.
// Register-spill scratch must live here, NOT in fine-grained host-coherent
// memory: the latter's coherent MTYPE means scalar (SMEM) accesses don't cache
// in K$ normally, so the trampoline's SGPR save->load races with the callee's
// mandatory s_dcache_inv. Device-local memory uses ordinary K$/L2 caching that
// is self-coherent within a wave. Match a GLOBAL, non-fine-grained pool that
// actually permits runtime allocation (skip read-only / non-allocatable pools).
static hsa_status_t find_coarse_grained(hsa_amd_memory_pool_t pool, void* d) {
    hsa_amd_segment_t seg;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &seg);
    if (seg != HSA_AMD_SEGMENT_GLOBAL) return HSA_STATUS_SUCCESS;
    uint32_t flags;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_FINE_GRAINED) return HSA_STATUS_SUCCESS;
    bool alloc_ok = false;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED, &alloc_ok);
    if (!alloc_ok) return HSA_STATUS_SUCCESS;
    auto* s=(PoolSearch*)d; s->pool=pool; s->found=true; return HSA_STATUS_INFO_BREAK;
}

// ---- CPU-side hostcall service loop -----------------------------------------
static std::atomic<bool> g_run{true};

static void service_loop(HostcallMailbox* mb) {
    // Files are keyed by HANDLE so different waves can open different files: FOPEN
    // opens the requested path (deduped by name) and returns its FILE* as the handle;
    // FWRITE/FCLOSE act on (FILE*)mb->handle. Files stay open until teardown. A
    // `primary` (the first file opened) preserves the legacy single-file WRITE_ID
    // path used by the hc_write_id-based examples, which don't thread a handle.
    std::map<std::string, FILE*> files;
    FILE* primary = nullptr;
    long opens = 0, closes = 0, writes = 0;
    while (g_run.load(std::memory_order_acquire)) {
        if (__atomic_load_n(&mb->status, __ATOMIC_ACQUIRE) != 1) {
            std::this_thread::yield();
            continue;
        }
        switch (mb->opcode) {
        case HC_OP_FOPEN: {
            std::string name(mb->path);
            FILE*& f = files[name];                      // open each distinct name once
            if (!f) {
                f = fopen(mb->path, mb->mode);
                if (!primary) primary = f;
                fprintf(stderr, "[host] fopen('%s','%s') -> %p\n", mb->path, mb->mode, (void*)f);
            }
            opens++;
            mb->handle = (int64_t)(uintptr_t)f;          // return the handle to the device
            mb->retval = f ? 0 : -1;
            break; }
        case HC_OP_FWRITE: {
            FILE* f = (FILE*)(uintptr_t)mb->handle;       // per-wave file (from gpu_fopen)
            int n = f ? (int)fwrite(mb->data, 1, mb->size, f) : -1;
            if (f) fflush(f);
            writes++;
            mb->retval = n;
            break; }
        case HC_OP_FREAD: {
            FILE* f = (FILE*)(uintptr_t)mb->handle;
            int n = f ? (int)fread(mb->data, 1, mb->size, f) : -1;
            mb->retval = n;
            break; }
        case HC_OP_FCLOSE: {
            FILE* f = (FILE*)(uintptr_t)mb->handle;
            if (f) fflush(f);                             // defer real close to teardown
            else if (primary) fflush(primary);
            closes++;
            mb->retval = 0;
            break; }
        case HC_OP_WRITE_ID:                             // legacy per-site scalar id -> primary file
            if (primary) { fprintf(primary, "[gpu] site %d\n", mb->arg); fflush(primary); }
            writes++;
            mb->retval = 0;
            break;
        default:
            mb->retval = -1;
            break;
        }
        __atomic_thread_fence(__ATOMIC_RELEASE);
        __atomic_store_n(&mb->status, 2, __ATOMIC_RELEASE);   // done -> release GPU
    }
    for (auto& kv : files) if (kv.second) fclose(kv.second);  // close every opened file
    fprintf(stderr, "[host] serviced %ld fopen / %ld fwrite / %ld fclose (%zu distinct files)\n",
            opens, writes, closes, files.size());
    fprintf(stderr, "[host] final ticket_next=%u ticket_now=%u status=%d (should be equal / 0)\n",
            mb->ticket_next, mb->ticket_now, mb->status);
}

int main(int argc, char** argv) {
    const char* mutatee = (argc > 1) ? argv[1] : "vectoradd.inst.co";
    const char* instlib = (argc > 2) ? argv[2] : "hostcall_lib.aliased.elf";
    const char* kd_name = (argc > 3) ? argv[3] : "_Z9vectoraddPfPKfS1_i.kd";
    const int   N       = getenv("HOSTCALL_N") ? atoi(getenv("HOSTCALL_N")) : 64;    // 1 wave (wave64). N=256 (4 waves) exposes the
                                 // separate multi-wave hostcall/mailbox bug.

    HSA_CHECK(hsa_init());
    AgentSearch gpu={}, cpu={};
    hsa_iterate_agents(find_gpu, &gpu);
    hsa_iterate_agents(find_cpu, &cpu);
    if (!gpu.found || !cpu.found) { fprintf(stderr, "no gfx908 GPU / CPU agent\n"); return 1; }
    PoolSearch fg={};
    hsa_amd_agent_iterate_memory_pools(cpu.agent, find_fine_grained, &fg);
    if (!fg.found) { fprintf(stderr, "no fine-grained pool\n"); return 1; }
    PoolSearch cg={};
    hsa_amd_agent_iterate_memory_pools(gpu.agent, find_coarse_grained, &cg);
    if (!cg.found) { fprintf(stderr, "no coarse-grained device-local pool\n"); return 1; }
    printf("[host] found gfx908 agent + fine-grained + device-local pools\n");

    // Mailbox in host-coherent fine-grained memory; grant the GPU access.
    HostcallMailbox* mbox = nullptr;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, sizeof(HostcallMailbox), 0, (void**)&mbox));
    HSA_CHECK(hsa_amd_agents_allow_access(1, &gpu.agent, nullptr, mbox));
    memset(mbox, 0, sizeof(*mbox));

    // Build the executable: define `mailbox`, load BOTH objects, freeze.
    hsa_executable_t exe;
    HSA_CHECK(hsa_executable_create_alt(HSA_PROFILE_FULL,
              HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, "", &exe));
    HSA_CHECK(hsa_executable_agent_global_variable_define(exe, gpu.agent, "mailbox", mbox));
    printf("[host] defined 'mailbox' -> %p\n", (void*)mbox);

    auto load = [&](const char* path) {
        int fd = open(path, O_RDONLY);
        if (fd < 0) { perror(path); exit(1); }
        hsa_code_object_reader_t r;
        HSA_CHECK(hsa_code_object_reader_create_from_file(fd, &r));
        HSA_CHECK(hsa_executable_load_agent_code_object(exe, gpu.agent, r, "", nullptr));
        close(fd);
        printf("[host] loaded %s\n", path);
    };
    load(instlib);    // provides hc_* + .dyninst.hc_* OBJECT aliases; references mailbox
    load(mutatee);    // references .dyninst.hc_* (the inserted calls)

    HSA_CHECK(hsa_executable_freeze(exe, ""));
    printf("[host] executable frozen (cross-object relocs resolved)\n");

    // Kernel descriptor + resources.
    hsa_executable_symbol_t ksym;
    HSA_CHECK(hsa_executable_get_symbol_by_name(exe, kd_name, &gpu.agent, &ksym));
    uint64_t kobj; uint32_t ksize, psize, gsize;
    HSA_CHECK(hsa_executable_symbol_get_info(ksym, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kobj));
    HSA_CHECK(hsa_executable_symbol_get_info(ksym, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &ksize));
    HSA_CHECK(hsa_executable_symbol_get_info(ksym, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &psize));
    HSA_CHECK(hsa_executable_symbol_get_info(ksym, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &gsize));
    printf("[host] kernel: kobj=%#lx ksize=%u psize=%u gsize=%u\n", kobj, ksize, psize, gsize);

    // Data buffers (fine-grained, GPU-accessible). vectoradd(C, A, B, N).
    float *A, *B, *C;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, N*sizeof(float), 0, (void**)&A));
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, N*sizeof(float), 0, (void**)&B));
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, N*sizeof(float), 0, (void**)&C));
    hsa_agent_t agents[1] = { gpu.agent };
    HSA_CHECK(hsa_amd_agents_allow_access(1, agents, nullptr, A));
    HSA_CHECK(hsa_amd_agents_allow_access(1, agents, nullptr, B));
    HSA_CHECK(hsa_amd_agents_allow_access(1, agents, nullptr, C));
    for (int i=0;i<N;i++){ A[i]=(float)i; B[i]=(float)(2*i); C[i]=0; }
    printf("[host] bufs A=%p B=%p C=%p mailbox=%p  (each %zu bytes)\n",
           (void*)A, (void*)B, (void*)C, (void*)mbox, N*sizeof(float));

    // Per-wave user buffer (per-wave variable). dyninst captures this base from
    // kernarg[ksize-8] at kernel entry into the IACR; an inserted call passes THIS
    // wave's slice (base + wid*stride) as a void* arg to the user probe. Sized to one
    // SLOT per grid wave. FINE-GRAINED (host-coherent) so the host can read what the
    // probes wrote after the kernel finishes. [M1a: probes write the base (offset 0);
    // per-wave wid*stride slicing lands in M1b.]
    const uint32_t INST_SLOT_SIZE = 4096;                  // bytes per wave (stride; matches emitter)
    const uint32_t INST_HEADER    = 0;                     // logical wid => slot at wid*SLOT (no counter)
    uint32_t n_waves = (uint32_t)((N + 63) / 64);          // wave64
    size_t   instbuf_sz = INST_HEADER + (size_t)n_waves * INST_SLOT_SIZE;
    void*    instbuf = nullptr;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, instbuf_sz, 0, &instbuf));
    HSA_CHECK(hsa_amd_agents_allow_access(1, agents, nullptr, instbuf));
    memset(instbuf, 0, instbuf_sz);                        // fine-grained: CPU-zeroable
    printf("[host] per-wave buffer @ %p  (%u waves x %u + %u hdr = %zu bytes)\n",
           instbuf, n_waves, INST_SLOT_SIZE, INST_HEADER, instbuf_sz);

    // Workgroup size: default = N (single workgroup, historical behavior).
    // HOSTCALL_WG=<n> splits the grid into ceil(N/n) workgroups so blockIdx.x
    // varies — needed to validate implicit-arg (blockIdx) forwarding.
    uint32_t wg = (uint32_t)N;
    if (const char* e = getenv("HOSTCALL_WG")) { uint32_t v = (uint32_t)atoi(e); if (v) wg = v; }
    const uint32_t n_blocks = ((uint32_t)N + wg - 1) / wg;

    // Kernarg: C, A, B, N  (explicit args at the front of the segment).
    void* kernargs = nullptr;
    uint32_t ka_sz = ksize < 64 ? 64 : ksize;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, ka_sz, 0, &kernargs));
    HSA_CHECK(hsa_amd_agents_allow_access(1, agents, nullptr, kernargs));
    memset(kernargs, 0, ka_sz);
    *(float**)((char*)kernargs + 0)  = C;
    *(const float**)((char*)kernargs + 8)  = A;
    *(const float**)((char*)kernargs + 16) = B;
    *(int*)((char*)kernargs + 24) = N;
    // Hidden (implicit) kernargs the HIP runtime normally fills — COV5/6 reads
    // blockDim/gridDim from here, NOT the dispatch packet. Layout from the kernel
    // metadata (llvm-readelf --notes): block_count_x @32 (u32), group_size_x @44
    // (u16 = blockDim.x), remainder_x @50 (u16). Without these, blockDim.x reads 0
    // and a multi-block kernel (i = blockIdx*blockDim + threadIdx) is wrong for
    // every block but block 0. (Harmless at 1 workgroup since blockIdx==0.)
    if (ka_sz >= 52) {
        *(uint32_t*)((char*)kernargs + 32) = n_blocks;          // hidden_block_count_x
        *(uint16_t*)((char*)kernargs + 44) = (uint16_t)wg;      // hidden_group_size_x = blockDim.x
        *(uint16_t*)((char*)kernargs + 50) = (uint16_t)((uint32_t)N % wg); // hidden_remainder_x
    }
    // The instrumentation prologue loads its scratch base from the 8 bytes that
    // maximizeSgprAllocationIfKernel appended at the end of the kernarg segment.
    if (ksize >= 8)
        *(void**)((char*)kernargs + (ksize - 8)) = instbuf;

    // Queue + completion signal.
    hsa_queue_t* queue = nullptr;
    HSA_CHECK(hsa_queue_create(gpu.agent, 256, HSA_QUEUE_TYPE_SINGLE, nullptr, nullptr,
                               UINT32_MAX, UINT32_MAX, &queue));
    hsa_signal_t done;
    HSA_CHECK(hsa_signal_create(1, 0, nullptr, &done));

    // Start the CPU hostcall service thread BEFORE dispatch.
    std::thread svc(service_loop, mbox);
    printf("[host] service thread started; dispatching kernel (N=%d, wg=%u, %u workgroup(s))\n",
           N, wg, (unsigned)n_blocks);

    // Enqueue the dispatch: grid = N threads, workgroup = wg threads.
    uint64_t idx = hsa_queue_load_write_index_relaxed(queue);
    const uint32_t mask = queue->size - 1;
    hsa_kernel_dispatch_packet_t* slot =
        &((hsa_kernel_dispatch_packet_t*)queue->base_address)[idx & mask];
    memset((void*)((uintptr_t)slot + 4), 0, sizeof(*slot) - 4);
    slot->setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    slot->workgroup_size_x = (uint16_t)wg; slot->workgroup_size_y = 1; slot->workgroup_size_z = 1;
    slot->grid_size_x = N; slot->grid_size_y = 1; slot->grid_size_z = 1;
    slot->private_segment_size = psize;
    slot->group_segment_size = gsize;
    slot->kernel_object = kobj;
    slot->kernarg_address = kernargs;
    slot->completion_signal = done;
    uint16_t header =
        (HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE) |
        (1 << HSA_PACKET_HEADER_BARRIER) |
        (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE) |
        (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE);
    __atomic_store_n(&slot->header, header, __ATOMIC_RELEASE);
    hsa_queue_store_write_index_relaxed(queue, idx + 1);
    hsa_signal_store_screlease(queue->doorbell_signal, idx);

    // Wait for the kernel (it makes hostcalls the service thread handles).
    hsa_signal_value_t v = hsa_signal_wait_scacquire(done, HSA_SIGNAL_CONDITION_LT, 1,
                                                     (uint64_t)10e9, HSA_WAIT_STATE_BLOCKED);
    printf("[host] kernel completion = %ld\n", (long)v);

    // Stop the service thread.
    g_run.store(false, std::memory_order_release);
    svc.join();

    // Per-wave buffer dump (per-wave variable). M1a: probes wrote through the base
    // pointer (offset 0), so word[0] holds the marker. M1b: each wave writes its own
    // slot at base + HEADER + wid*SLOT, so distinct slots become non-zero.
    {
        const uint32_t* pw = (const uint32_t*)instbuf;
        int nonzero = 0;
        printf("[host] per-wave buffer (one slice per wave):\n");
        for (uint32_t w = 0; w < n_waves; w++) {
            size_t o = (INST_HEADER + (size_t)w * INST_SLOT_SIZE) / 4;
            printf("[host]   wave %2u slice[0]=%d (probe hits)  slice[1]=%d (active lanes)\n",
                   w, (int)pw[o], (int)pw[o + 1]);
            if (pw[o]) nonzero++;
        }
        printf("[host] per-wave slots written: %d / %u\n", nonzero, n_waves);
    }

    // Verify vectoradd result.
    int errors = 0;
    for (int i=0;i<N;i++) if (C[i] != A[i]+B[i]) { if (errors<5)
        fprintf(stderr, "  mismatch @%d: %f != %f\n", i, C[i], A[i]+B[i]); errors++; }
    printf(errors ? "[host] vectoradd FAILED (%d errors)\n" : "[host] vectoradd PASSED (%d elems)\n",
           errors ? errors : N);

    hsa_signal_destroy(done);
    hsa_queue_destroy(queue);
    hsa_executable_destroy(exe);
    hsa_shut_down();
    printf("[host] done. trace file: dyninst_trace.txt\n");
    return errors ? 1 : 0;
}
