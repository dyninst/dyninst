#include "hip/hip_runtime.h"
#include <cstdio>
#include <cstring>
#define CK(c) do{ hipError_t e=(c); if(e!=hipSuccess){fprintf(stderr,"err %d\n",e);__builtin_trap();} }while(0)
__global__ void kA(float* C, const float* A, const float* B, int N){
  int i=blockIdx.x*blockDim.x+threadIdx.x; if(i<N) C[i]=A[i]+B[i]; }
extern "C" void run_kB(float*, const float*, const float*, int);
int main(){
  int n=0; CK(hipGetDeviceCount(&n)); int dev=0;
  for(int i=0;i<n;i++){hipDeviceProp_t p;CK(hipGetDeviceProperties(&p,i));if(!strncmp(p.gcnArchName,"gfx908",6)){dev=i;break;}}
  CK(hipSetDevice(dev));
  const int N=64; size_t b=N*sizeof(float);
  float*A,*B,*C; CK(hipMalloc(&A,b));CK(hipMalloc(&B,b));CK(hipMalloc(&C,b));
  float hA[64],hB[64],hC[64]; for(int i=0;i<N;i++){hA[i]=i;hB[i]=2*i;}
  CK(hipMemcpy(A,hA,b,hipMemcpyHostToDevice));CK(hipMemcpy(B,hB,b,hipMemcpyHostToDevice));
  hipLaunchKernelGGL(kA,dim3(1),dim3(64),0,0,C,A,B,N); CK(hipGetLastError());   // kA: exe's co
  CK(hipMemcpy(hC,C,b,hipMemcpyDeviceToHost)); float add=hC[3];
  run_kB(C,A,B,N);                                                              // kB: .so's co
  CK(hipDeviceSynchronize()); CK(hipMemcpy(hC,C,b,hipMemcpyDeviceToHost));
  printf("kA add[3]=%g (want 9)  kB sub[3]=%g (want -3)  -> %s\n", add, hC[3],
         (add==9 && hC[3]==-3)?"PASSED":"FAILED");
  return 0;
}
