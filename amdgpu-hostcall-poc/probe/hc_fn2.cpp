#include <hip/hip_runtime.h>
#include <stdint.h>
struct HostcallMailbox { uint32_t a,b; int opcode; int status; char buffer[256]; void* fh; };
extern "C" __attribute__((visibility("default"),used)) __device__ HostcallMailbox mailbox;

// SAME kind of hostcall, but with a dynamically-indexed LOCAL staging array.
// Still a "real call", still a leaf — but now it has a stack-resident object.
extern "C" __device__ __attribute__((used,noinline)) void gpu_hostcall_staged(int op, const char* in) {
    if ((threadIdx.x & 63) == 0) {
        char staging[128];                          // <-- forced onto the stack
        for (int i=0;i<128;i++) staging[i] = in ? in[i] : 0;
        for (int i=0;i<128;i++) mailbox.buffer[i] = staging[(i*7)&127]; // dynamic index defeats regs
        mailbox.opcode=op;
        __threadfence_system();
        *(volatile int*)&mailbox.status=1;
        while (*(volatile int*)&mailbox.status!=2) __builtin_amdgcn_s_sleep(64);
    }
    __builtin_amdgcn_wave_barrier();
}
