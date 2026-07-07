// Divergent-EXEC mutatee — validates roadmap 0.2 (EXEC-safe spill).
//
// Same kernel NAME + SIGNATURE + RESULT as vectoradd_mutatee (kernel `vectoradd`,
// mangled _Z9vectoraddPfPKfS1_i, computes C[i] = A[i] + B[i] for EVERY lane), so
// the existing launcher (verifies C=A+B) and instrument.sh work UNCHANGED. The
// only difference is the BODY: it contains a real divergent branch, so when the
// mutator inserts hc_write before instructions inside that branch, those calls run
// under a NARROWED EXEC mask (only the odd lanes active). That is exactly the case
// 0.2 must handle: the trampoline must force EXEC=-1 around the register spill so
// all 64 lanes' registers are saved/restored — otherwise the SGPR pack (one scalar
// per lane in vPack) loses the inactive lanes and readlane restores garbage.
//
// The divergent region does net-zero reversible work (odd lanes add then subtract
// the same amount, with a DATA-DEPENDENT trip count so the compiler emits a real
// loop + s_and_saveexec/execz branch rather than predicating it away). Result is
// still exactly A+B, so verification is identical to vectoradd.
//
// Build contract MUST match hostcall_lib: --offload-arch=gfx908:sramecc+:xnack-
// -mcode-object-version=6.

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

// Divergent variant of vectoradd. C[i] = A[i] + B[i] for all lanes, but odd lanes
// take a divergent branch (narrowed EXEC) containing a data-dependent loop.
__global__ void vectoradd(float* C, const float* A, const float* B, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    float r = A[i] + B[i];
    if (threadIdx.x & 1) {                 // DIVERGENT: only odd lanes of the wave
        int reps = (i & 7) + 1;            // 1..8, data-dependent => real loop/branch
        for (int k = 0; k < reps; ++k) r += 3.0f;   // net-zero, but forces work
        for (int k = 0; k < reps; ++k) r -= 3.0f;   // under NARROWED EXEC
    }
    C[i] = r;                              // == A[i] + B[i] for every lane
}

int main() {
    HIP_CHECK(hipSetDevice(select_gfx908_device()));

    const int N = 1 << 20;
    const size_t bytes = (size_t)N * sizeof(float);

    float *hA = (float*)malloc(bytes), *hB = (float*)malloc(bytes), *hC = (float*)malloc(bytes);
    for (int i = 0; i < N; i++) { hA[i] = (float)i; hB[i] = (float)(2 * i); }

    float *dA, *dB, *dC;
    HIP_CHECK(hipMalloc(&dA, bytes));
    HIP_CHECK(hipMalloc(&dB, bytes));
    HIP_CHECK(hipMalloc(&dC, bytes));
    HIP_CHECK(hipMemcpy(dA, hA, bytes, hipMemcpyHostToDevice));
    HIP_CHECK(hipMemcpy(dB, hB, bytes, hipMemcpyHostToDevice));

    const int block = 256;
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
