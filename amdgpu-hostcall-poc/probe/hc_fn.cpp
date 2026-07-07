#include <hip/hip_runtime.h>
#include <stdint.h>
struct HostcallMailbox { uint32_t a,b; int opcode; int status; char buffer[256]; void* fh; };
extern "C" __attribute__((visibility("default"),used)) __device__ HostcallMailbox mailbox;
extern "C" __device__ __attribute__((used,noinline)) void gpu_hostcall(int op, const char* in) {
    if ((threadIdx.x & 63) == 0) {
        for (int i=0;i<32 && in && in[i];i++) mailbox.buffer[i]=in[i];
        mailbox.opcode=op;
        __threadfence_system();
        *(volatile int*)&mailbox.status=1;
        while (*(volatile int*)&mailbox.status!=2) __builtin_amdgcn_s_sleep(64);
        *(volatile int*)&mailbox.status=0;
    }
    __builtin_amdgcn_wave_barrier();
}
