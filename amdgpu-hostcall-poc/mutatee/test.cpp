#include "hip/hip_runtime.h"

__device__ void A(const float a){
    printf("A called: %f\n", a);
}

__device__ void B(const float b){
    printf("B called: %f\n", b);
}

__device__ void C(const float c){
    printf("C called: %f\n", c);
}

__device__ void (*fn_array[]) (const float) = {&A, &B, &B};

__global__ void load_fn_ptr(float* v)
{
    for(int i = 0; i < 3; ++i)
        v[i] = i;
    fn_array[2] = &C;
}

__global__ void call_fn(const float *v, const int i){
      fn_array[i](v[i]);
}

int main(int argc, char *argv[])
{
    float* v = nullptr;
    hipMalloc(&v,sizeof(float) * 3u);
    hipLaunchKernelGGL(load_fn_ptr, 1, 1, 0 ,0 , v);
    hipLaunchKernelGGL(call_fn, 1, 1, 0 ,0 , v, 2);
    hipDeviceSynchronize();
    return 0;
}
