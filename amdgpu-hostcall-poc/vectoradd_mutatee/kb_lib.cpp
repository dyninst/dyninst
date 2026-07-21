#include "hip/hip_runtime.h"
__global__ void kB(float* C, const float* A, const float* B, int N){
  int i=blockIdx.x*blockDim.x+threadIdx.x; if(i<N) C[i]=A[i]-B[i]; }
extern "C" void run_kB(float* C, const float* A, const float* B, int N){
  hipLaunchKernelGGL(kB, dim3(1), dim3(64), 0, 0, C, A, B, N); }
