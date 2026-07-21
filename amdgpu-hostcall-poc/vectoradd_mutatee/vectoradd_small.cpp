// Simple vectoradd mutatee — the target application dyninst will instrument by
// injecting hostcalls (from hostcall_lib) into the kernel.
//
// Build contract (MUST match hostcall_lib so cross-object linking works):
//   --offload-arch=gfx908 -mcode-object-version=6   => ABI ver 4, flags 0x530
//
// NOTE on instrumentability: this kernel makes no device calls of its own, so
// the compiler will NOT emit a call-ABI prologue (stack pointer s32, FLAT_SCRATCH,
// scratch descriptor). If dyninst injects a call to a hostcall that touches
// scratch, the injector must establish that state. See build note / the
// call-ready variant if injection needs a pre-set-up frame.

#include "hip/hip_runtime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define HIP_CHECK(cmd) do {                                                  \
    hipError_t _e = (cmd);                                                    \
    if (_e != hipSuccess) {                                                   \
        fprintf(stderr, "HIP error %s (%d) at %s:%d -- %s\n",                 \
                hipGetErrorString(_e), _e, __FILE__, __LINE__, #cmd);         \
        std::abort();                                                         \
    }                                                                         \
} while (0)

// Pick the gfx908 device (device 0 here is gfx900; the MI100 is device 1).
static int select_gfx908_device() {
    int n = 0; HIP_CHECK(hipGetDeviceCount(&n));
    for (int i = 0; i < n; i++) {
        hipDeviceProp_t p; HIP_CHECK(hipGetDeviceProperties(&p, i));
        if (strncmp(p.gcnArchName, "gfx908", 6) == 0) {
            printf("Selecting device %d: %s (%s)\n", i, p.name, p.gcnArchName);
            return i;
        }
    }
    fprintf(stderr, "No gfx908 device found.\n"); std::abort();
}

__global__ void vectoradd(float* C, const float* A, const float* B, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) C[i] = A[i] + B[i];
}

int main() {
    HIP_CHECK(hipSetDevice(select_gfx908_device()));

    const int N = 64;
    const size_t bytes = (size_t)N * sizeof(float);

    float *hA = (float*)malloc(bytes), *hB = (float*)malloc(bytes), *hC = (float*)malloc(bytes);
    for (int i = 0; i < N; i++) { hA[i] = (float)i; hB[i] = (float)(2 * i); }

    float *dA, *dB, *dC;
    HIP_CHECK(hipMalloc(&dA, bytes));
    HIP_CHECK(hipMalloc(&dB, bytes));
    HIP_CHECK(hipMalloc(&dC, bytes));
    HIP_CHECK(hipMemcpy(dA, hA, bytes, hipMemcpyHostToDevice));
    HIP_CHECK(hipMemcpy(dB, hB, bytes, hipMemcpyHostToDevice));

    const int block = 64;
    const int grid  = (N + block - 1) / block;
    hipLaunchKernelGGL(vectoradd, dim3(grid), dim3(block), 0, 0, dC, dA, dB, N);
    HIP_CHECK(hipGetLastError());
    HIP_CHECK(hipDeviceSynchronize());

    HIP_CHECK(hipMemcpy(hC, dC, bytes, hipMemcpyDeviceToHost));

    int errors = 0;
    for (int i = 0; i < N; i++) if (hC[i] != hA[i] + hB[i]) { if (errors < 5)
        fprintf(stderr, "mismatch at %d: %f != %f\n", i, hC[i], hA[i] + hB[i]); errors++; }
    printf(errors ? "FAILED with %d errors\n" : "PASSED (%d elements)\n", errors ? errors : N);

    hipFree(dA); hipFree(dB); hipFree(dC);
    free(hA); free(hB); free(hC);
    return errors ? 1 : 0;
}
