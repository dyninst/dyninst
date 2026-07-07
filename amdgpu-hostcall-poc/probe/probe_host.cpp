// Probe host harness — the load-bearing feasibility test for turning the
// hostcall example into a SEPARATED device library.
//
// It answers two questions at once:
//   (1) Can the HSA loader resolve an UNDEFINED data symbol (`mailbox`) in a
//       standalone code object to a host-provided address, via
//       hsa_executable_agent_global_variable_define()? (replacing HIP __managed__)
//   (2) Once resolved to an HSA fine-grained (host-coherent) buffer, can the CPU
//       coherently poll the mailbox while the GPU kernel spins on it — i.e. does
//       the GPU->CPU hostcall handshake work without HIP's managed-memory plumbing?
//
// If both pass on gfx908/MI100, the separated-library hostcall design is viable.

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

#define HSA_CHECK(s) do {                                                   \
    hsa_status_t _s = (s);                                                  \
    if (_s != HSA_STATUS_SUCCESS) {                                         \
        const char* _m = nullptr; hsa_status_string(_s, &_m);              \
        fprintf(stderr, "HSA error: %s at %s:%d\n", _m, __FILE__, __LINE__);\
        exit(1);                                                            \
    }                                                                       \
} while (0)

// Must match probe_dev.cpp exactly.
struct HostcallMailbox {
    uint32_t ticket_next;
    uint32_t ticket_now;
    int      opcode;
    int      status;       // 0 idle, 1 GPU-request, 2 CPU-done, 3 GPU-ack
    char     buffer[256];
    void*    file_handle;
};

// --- agent / pool discovery ---------------------------------------------------
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
// Find a host-coherent, fine-grained GLOBAL pool on the CPU agent.
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
static hsa_status_t find_kernarg_pool(hsa_amd_memory_pool_t pool, void* d) {
    hsa_amd_segment_t seg;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &seg);
    if (seg != HSA_AMD_SEGMENT_GLOBAL) return HSA_STATUS_SUCCESS;
    uint32_t flags;
    hsa_amd_memory_pool_get_info(pool, HSA_AMD_MEMORY_POOL_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_AMD_MEMORY_POOL_GLOBAL_FLAG_KERNARG_INIT) {
        auto* s=(PoolSearch*)d; s->pool=pool; s->found=true; return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}

int main() {
    HSA_CHECK(hsa_init());

    AgentSearch gpu={}, cpu={};
    hsa_iterate_agents(find_gpu, &gpu);
    hsa_iterate_agents(find_cpu, &cpu);
    if (!gpu.found) { fprintf(stderr, "no gfx908 GPU agent\n"); return 1; }
    if (!cpu.found) { fprintf(stderr, "no CPU agent\n"); return 1; }
    printf("[host] found gfx908 GPU agent and CPU agent\n");

    // Fine-grained, host-coherent pool (system memory) — the __managed__ replacement.
    // We use it both for the mailbox and (after allow_access) for kernargs.
    PoolSearch fg={};
    hsa_amd_agent_iterate_memory_pools(cpu.agent, find_fine_grained, &fg);
    if (!fg.found) { fprintf(stderr, "no fine-grained CPU pool\n"); return 1; }

    // Allocate the mailbox in fine-grained memory and grant the GPU access.
    HostcallMailbox* mbox = nullptr;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, sizeof(HostcallMailbox), 0, (void**)&mbox));
    HSA_CHECK(hsa_amd_agents_allow_access(1, &gpu.agent, nullptr, mbox));
    memset(mbox, 0, sizeof(*mbox));
    printf("[host] mailbox allocated in fine-grained pool @ %p\n", (void*)mbox);

    // Build executable, DEFINE the external `mailbox` to our buffer, then load+freeze.
    hsa_executable_t exe;
    HSA_CHECK(hsa_executable_create_alt(HSA_PROFILE_FULL,
              HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT, "", &exe));
    HSA_CHECK(hsa_executable_agent_global_variable_define(exe, gpu.agent, "mailbox", mbox));
    printf("[host] defined external symbol 'mailbox' -> %p\n", (void*)mbox);

    int fd = open("probe_dev.elf", O_RDONLY);
    if (fd < 0) { perror("open probe_dev.elf"); return 1; }
    hsa_code_object_reader_t reader;
    HSA_CHECK(hsa_code_object_reader_create_from_file(fd, &reader));
    HSA_CHECK(hsa_executable_load_agent_code_object(exe, gpu.agent, reader, "", nullptr));
    HSA_CHECK(hsa_executable_freeze(exe, ""));
    printf("[host] executable frozen\n");

    // Verify the loader resolved `mailbox` to our buffer.
    hsa_executable_symbol_t sym_mbox;
    HSA_CHECK(hsa_executable_get_symbol_by_name(exe, "mailbox", &gpu.agent, &sym_mbox));
    uint64_t resolved = 0;
    HSA_CHECK(hsa_executable_symbol_get_info(sym_mbox,
              HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, &resolved));
    printf("[host] loader resolved 'mailbox' -> %#lx  (expected %p) %s\n",
           resolved, (void*)mbox,
           (resolved == (uint64_t)mbox) ? "*** MATCH ***" : "*** MISMATCH ***");

    // Kernel descriptor + resources.
    hsa_executable_symbol_t sym_k;
    HSA_CHECK(hsa_executable_get_symbol_by_name(exe, "probe_kernel.kd", &gpu.agent, &sym_k));
    uint64_t kobj; uint32_t ksize, psize, gsize;
    HSA_CHECK(hsa_executable_symbol_get_info(sym_k, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kobj));
    HSA_CHECK(hsa_executable_symbol_get_info(sym_k, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &ksize));
    HSA_CHECK(hsa_executable_symbol_get_info(sym_k, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &psize));
    HSA_CHECK(hsa_executable_symbol_get_info(sym_k, HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &gsize));
    printf("[host] probe_kernel: kobj=%#lx ksize=%u psize=%u gsize=%u\n", kobj, ksize, psize, gsize);

    // Queue + kernarg.
    hsa_queue_t* queue = nullptr;
    HSA_CHECK(hsa_queue_create(gpu.agent, 64, HSA_QUEUE_TYPE_SINGLE, nullptr, nullptr,
                               UINT32_MAX, UINT32_MAX, &queue));
    void* kernargs = nullptr;
    HSA_CHECK(hsa_amd_memory_pool_allocate(fg.pool, ksize < 64 ? 64 : ksize, 0, &kernargs));
    HSA_CHECK(hsa_amd_agents_allow_access(1, &gpu.agent, nullptr, kernargs));

    // Completion signal.
    hsa_signal_t done;
    HSA_CHECK(hsa_signal_create(1, 0, nullptr, &done));

    // Build + enqueue the dispatch packet (NON-blocking: we must run the CPU
    // service loop while the kernel spins).
    uint64_t idx = hsa_queue_load_write_index_relaxed(queue);
    const uint32_t mask = queue->size - 1;
    hsa_kernel_dispatch_packet_t* slot =
        &((hsa_kernel_dispatch_packet_t*)queue->base_address)[idx & mask];
    memset((void*)((uintptr_t)slot + 4), 0, sizeof(*slot) - 4); // zero all but header word
    slot->setup = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    slot->workgroup_size_x = 1; slot->workgroup_size_y = 1; slot->workgroup_size_z = 1;
    slot->grid_size_x = 1; slot->grid_size_y = 1; slot->grid_size_z = 1;
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
    printf("[host] kernel dispatched; entering CPU service loop\n");

    // --- CPU service loop: the hostcall handler ---
    bool serviced = false;
    for (int spins = 0; spins < 200000000; spins++) {
        int st = __atomic_load_n(&mbox->status, __ATOMIC_ACQUIRE);
        if (st == 1) {
            printf("[CPU] saw GPU request: opcode=%d buffer='%s'\n", mbox->opcode, mbox->buffer);
            __atomic_store_n(&mbox->status, 2, __ATOMIC_RELEASE);  // acknowledge
            serviced = true;
            break;
        }
    }
    if (!serviced) {
        printf("[CPU] *** TIMEOUT *** never saw GPU request (status=%d) — coherency FAILED\n",
               __atomic_load_n(&mbox->status, __ATOMIC_ACQUIRE));
    }

    // Wait for the kernel to finish its side of the handshake.
    hsa_signal_value_t v = hsa_signal_wait_scacquire(done, HSA_SIGNAL_CONDITION_LT, 1,
                                                     (uint64_t)5e9, HSA_WAIT_STATE_BLOCKED);
    printf("[host] kernel completion signal = %ld\n", (long)v);
    printf("[host] final mailbox: status=%d opcode=%d  %s\n",
           mbox->status, mbox->opcode,
           (mbox->status == 3 && mbox->opcode == 99)
               ? "*** HANDSHAKE COMPLETE ***" : "*** INCOMPLETE ***");

    hsa_signal_destroy(done);
    hsa_queue_destroy(queue);
    hsa_executable_destroy(exe);
    hsa_amd_memory_pool_free(kernargs);
    hsa_amd_memory_pool_free(mbox);
    hsa_shut_down();
    return 0;
}
