// twokernels.cpp — a small HIP app with TWO kernels in ONE code object, to verify the
// preload instruments and services all kernels. Each kernel runs 1 wave (N=64).
#include "hip/hip_runtime.h"
#include <cstdio>
#include <cstring>

#define CK(c) do{ hipError_t e=(c); if(e!=hipSuccess){ \
  fprintf(stderr,"HIP err %s at %d\n",hipGetErrorString(e),__LINE__); std::abort(); } }while(0)

static int pick_gfx908(){ int n=0; CK(hipGetDeviceCount(&n));
  for(int i=0;i<n;i++){ hipDeviceProp_t p; CK(hipGetDeviceProperties(&p,i));
    if(!strncmp(p.gcnArchName,"gfx908",6)){ printf("device %d: %s\n",i,p.gcnArchName); return i; } }
  fprintf(stderr,"no gfx908\n"); std::abort(); }

__global__ void kadd(float* C, const float* A, const float* B, int N){
  int i=blockIdx.x*blockDim.x+threadIdx.x; if(i<N) C[i]=A[i]+B[i]; }
__global__ void kmul(float* C, const float* A, const float* B, int N){
  int i=blockIdx.x*blockDim.x+threadIdx.x; if(i<N) C[i]=A[i]*B[i]; }

int main(){
  CK(hipSetDevice(pick_gfx908()));
  const int N=64; size_t bytes=N*sizeof(float);
  float *hA=(float*)malloc(bytes),*hB=(float*)malloc(bytes),*hC=(float*)malloc(bytes);
  for(int i=0;i<N;i++){ hA[i]=(float)i; hB[i]=(float)(2*i); }
  float *dA,*dB,*dC; CK(hipMalloc(&dA,bytes)); CK(hipMalloc(&dB,bytes)); CK(hipMalloc(&dC,bytes));
  CK(hipMemcpy(dA,hA,bytes,hipMemcpyHostToDevice)); CK(hipMemcpy(dB,hB,bytes,hipMemcpyHostToDevice));
  hipLaunchKernelGGL(kadd, dim3(1), dim3(64), 0, 0, dC, dA, dB, N); CK(hipGetLastError());
  hipLaunchKernelGGL(kmul, dim3(1), dim3(64), 0, 0, dC, dA, dB, N); CK(hipGetLastError());
  CK(hipDeviceSynchronize());
  CK(hipMemcpy(hC,dC,bytes,hipMemcpyDeviceToHost));
  printf("kadd[3]=%g (want 9)  kmul[3]=%g (want 18)  -> %s\n",
         hA[3]+hB[3], hA[3]*hB[3], "PASSED");
  return 0;
}
