// Probe device library — compiled to a STANDALONE code object via `hipcc --genco`.
//
// The load-bearing question: `mailbox` is declared here as an UNDEFINED external
// global. It is NOT defined in this code object. The host harness must resolve it
// at load time via hsa_executable_agent_global_variable_define(...) to a buffer it
// allocated in an HSA fine-grained (host-coherent) pool — the same mechanism the
// real instrumentation library would use instead of HIP's __managed__ plumbing.
//
// If the loader resolves the reference AND the CPU can coherently poll the buffer
// while this kernel spins, the separated-device-library hostcall design is viable.

#include <hip/hip_runtime.h>
#include <stdint.h>

struct HostcallMailbox {
    uint32_t ticket_next;
    uint32_t ticket_now;
    int      opcode;
    int      status;       // 0 idle, 1 GPU-request, 2 CPU-done, 3 GPU-ack
    char     buffer[256];
    void*    file_handle;
};

// Undefined external — resolved by the host at load time.
// default visibility: HIP gives device globals "protected" visibility by default,
// and a protected symbol may not be left undefined. We need it undefined so the
// loader resolves it, so force default visibility. `used` keeps the reference from
// being optimized out.
extern "C" __attribute__((visibility("default"), used)) __device__ HostcallMailbox mailbox;

extern "C" __global__ void probe_kernel() {
    if (threadIdx.x == 0) {
        // GPU writes a request into the shared buffer.
        const char msg[] = "Hello from GPU";
        #pragma unroll
        for (int i = 0; i < (int)sizeof(msg); i++) mailbox.buffer[i] = msg[i];
        mailbox.opcode = 42;

        // Publish the request, then signal the host.
        __threadfence_system();
        *(volatile int*)&mailbox.status = 1;

        // Spin until the CPU acknowledges (status == 2).
        while (*(volatile int*)&mailbox.status != 2) {
            __builtin_amdgcn_s_sleep(64);
        }

        // Confirm we saw the host's response, then mark fully done.
        mailbox.opcode = 99;
        __threadfence_system();
        *(volatile int*)&mailbox.status = 3;
    }
    __builtin_amdgcn_wave_barrier();
}
