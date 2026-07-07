#include <hip/hip_runtime.h>
// Force the compiler to use PRIVATE/SCRATCH memory: a local array with a
// runtime-dependent index cannot be kept in registers, so it spills to scratch.
extern "C" __global__ void scratch_probe(int* out, const int* in, int n, int sel) {
    int t = threadIdx.x + blockIdx.x * blockDim.x;
    volatile int buf[96];                 // large -> can't live in registers
    for (int i = 0; i < 96; i++) buf[i] = in[t] + i * n;
    int acc = 0;
    for (int i = 0; i < 96; i++) acc += buf[(i + sel) & 95];   // dynamic index
    out[t] = acc;
}
