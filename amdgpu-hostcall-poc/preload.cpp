#include <inttypes.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "hip/hip_runtime.h"
#include "hip/hip_runtime_api.h"
#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>
#include <link.h>
#include <vector>
#include <map>
#include <iostream>
#include <mutex>
#include <fstream>
using namespace std;

#define HIP_ASSERT(status)                                                                 \
    if (status != hipSuccess) {                                                          \
        std::cerr << "HIP Error: " << hipGetErrorString(status) << " at line " << __LINE__ << std::endl; \
        exit(1);                                                                         \
    }
#define hipCheck(status) HIP_ASSERT(status)

#define HSA_CHECK(s) do {                                                       \
    hsa_status_t _s = (s);                                                      \
    if (_s != HSA_STATUS_SUCCESS) {                                             \
        const char* _msg = nullptr;                                             \
        hsa_status_string(_s, &_msg);                                           \
        fprintf(stderr, "HSA Error: %s at line %d\n", _msg, __LINE__);         \
        exit(1);                                                                \
    }                                                                           \
} while(0)

int old_kernarg_size;
int old_kernarg_num;

FILE * fdata;

typedef struct kernel_launch_tracker{
  uint32_t * data;
  uint32_t * data_h;
  uint32_t avail_size;
  uint32_t data_size;
  uint32_t status;

  uint32_t kid;
  uint32_t num_records;
  uint32_t num_branches;
}kernel_launch_tracker;

std::vector<kernel_launch_tracker*> mempool;

void * fatbin_data;
#define ElfW(type)        _ElfW (Elf, __ELF_NATIVE_CLASS, type)
#define _ElfW(e,w,t)        _ElfW_1 (e, w, _##t)
#define _ElfW_1(e,w,t)        e##w##t

typedef void** (*registerFatBinary_t ) (
    const void * data);

registerFatBinary_t realRegisterFatBinary =0 ;
struct __CudaFatBinaryWrapper {
  unsigned int magic;
  unsigned int version;
  void*        binary;
  void*        dummy1;
};

FILE * fdebug;
extern "C" void** __hipRegisterFatBinary(void* data)
{
  if(realRegisterFatBinary == 0){
    realRegisterFatBinary = (registerFatBinary_t) dlsym(RTLD_NEXT,"__hipRegisterFatBinary");
  }

  __CudaFatBinaryWrapper* fbwrapper = reinterpret_cast<__CudaFatBinaryWrapper*>(data);
  printf("data %lx : %s %lx\n", (uint64_t) data, (char * ) data, (uint64_t) fbwrapper->binary);
  return realRegisterFatBinary( data );

}

typedef void (*registerFunc_t ) (
    void** modules,
    const void*  hostFunction,
    char*        deviceFunction,
    const char*  deviceName,
    unsigned int threadLimit,
    uint3*       tid,
    uint3*       bid,
    dim3*        blockDim,
    dim3*        gridDim,
    int*         wSize);

typedef struct config {
  uint32_t kernarg_size ;
  uint32_t kernarg_num ;
  uint32_t branch_count ;
  uint32_t kid;
  std::string name;
}config;
std::map<const void * , config* > FuncLookup;
std::map<std::string, config *> metaLookup;
std::map<int ,int > testMap;
registerFunc_t realRegisterFunction;
int g_kid = 0;

extern "C" void __hipRegisterFunction(
    void** modules,
    const void*  hostFunction,
    char*        deviceFunction,
    const char*  deviceName,
    unsigned int threadLimit,
    uint3*       tid,
    uint3*       bid,
    dim3*        blockDim,
    dim3*        gridDim,
    int*         wSize){
  printf("modules = %p, hostFunciton = %p, devceFunciton = %s, deviceName = %s\n",modules,hostFunction, deviceFunction, deviceName);

  if(realRegisterFunction == 0){
    realRegisterFunction = (registerFunc_t) dlsym(RTLD_NEXT,"__hipRegisterFunction");
  }
  realRegisterFunction(modules,hostFunction,deviceFunction,deviceName,threadLimit,tid,bid,blockDim,gridDim,wSize);
  return;
}

typedef hipError_t (*launch_t)(const void *hostFunction,
    dim3 gridDim,
    dim3 blockDim,
    void** args,
    size_t sharedMemBytes,
    hipStream_t stream);

launch_t realLaunch;

uint32_t launch_id = 0;
extern "C" hipError_t hipLaunchKernel(const void *hostFunction,
    dim3 gridDim,
    dim3 blockDim,
    void** args,
    size_t sharedMemBytes,
    hipStream_t stream)
{
  if(realLaunch ==0){
    realLaunch = (launch_t)  dlsym(RTLD_NEXT, "hipLaunchKernel");
  }

    printf("hip Launch Kernel Called, hostFunction = %p\n",hostFunction);
    printf("Grid(x,y,z)=(%u,%u,%u) Block(x,y,z)=(%u,%u,%u)\n", gridDim.x , gridDim.y , gridDim.z ,blockDim.x , blockDim.y , blockDim.z);

    realLaunch(hostFunction,gridDim,blockDim, args, sharedMemBytes, stream);
    HIP_ASSERT(hipStreamSynchronize(stream));
    printf("hip Launch Kernel Ended\n\n");
    return hipSuccess;
}

typedef  hipError_t (* malloc_t ) ( void ** ptr , size_t sizeBytes);
malloc_t realMalloc;
hipError_t hipMalloc(void** ptr, size_t sizeBytes){
  if(realMalloc == 0){
    realMalloc = (malloc_t) dlsym(RTLD_NEXT,"hipMalloc");
  }
  return realMalloc(ptr,sizeBytes);
}

typedef hipError_t (* memcpy_t) (void* dst, const void* src, size_t sizeBytes, hipMemcpyKind kind);
memcpy_t realMemcpy;

// ---------------------------------------------------------------------------
// HSA helpers
// ---------------------------------------------------------------------------

static hsa_executable_t g_hsa_exe;
static hsa_queue_t*     g_hsa_queue = nullptr;

struct AgentSearch { hsa_agent_t agent; bool found; };

static hsa_status_t find_gfx908_agent(hsa_agent_t agent, void* data) {
    hsa_device_type_t type;
    hsa_agent_get_info(agent, HSA_AGENT_INFO_DEVICE, &type);
    if (type != HSA_DEVICE_TYPE_GPU) return HSA_STATUS_SUCCESS;
    char name[64] = {};
    hsa_agent_get_info(agent, HSA_AGENT_INFO_NAME, name);
    if (strstr(name, "gfx908")) {
        auto* s = (AgentSearch*)data;
        s->agent = agent;
        s->found = true;
        return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}

static hsa_status_t find_kernarg_region(hsa_region_t region, void* data) {
    hsa_region_segment_t seg;
    hsa_region_get_info(region, HSA_REGION_INFO_SEGMENT, &seg);
    if (seg != HSA_REGION_SEGMENT_GLOBAL) return HSA_STATUS_SUCCESS;
    hsa_region_global_flag_t flags;
    hsa_region_get_info(region, HSA_REGION_INFO_GLOBAL_FLAGS, &flags);
    if (flags & HSA_REGION_GLOBAL_FLAG_KERNARG) {
        *(hsa_region_t*)data = region;
        return HSA_STATUS_INFO_BREAK;
    }
    return HSA_STATUS_SUCCESS;
}

// Submit a 1x1x1 kernel dispatch and wait for completion.
// Build the packet in a local, copy it into the queue slot, THEN advance
// write_index and ring the doorbell — otherwise the GPU can observe a
// half-initialized slot and mis-handle scratch / other fields.
static void dispatch_1x1(hsa_queue_t* queue, uint64_t kern_obj,
                          void* kernargs,
                          uint32_t private_seg_size, uint32_t group_seg_size) {
    hsa_signal_t signal;
    HSA_CHECK(hsa_signal_create(1, 0, nullptr, &signal));

    hsa_kernel_dispatch_packet_t local_pkt;
    memset(&local_pkt, 0, sizeof(local_pkt));
    local_pkt.setup              = 1 << HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS;
    local_pkt.workgroup_size_x   = 1;
    local_pkt.workgroup_size_y   = 1;
    local_pkt.workgroup_size_z   = 1;
    local_pkt.grid_size_x        = 1;
    local_pkt.grid_size_y        = 1;
    local_pkt.grid_size_z        = 1;
    local_pkt.private_segment_size = private_seg_size;
    local_pkt.group_segment_size   = group_seg_size;
    local_pkt.kernel_object        = kern_obj;
    local_pkt.kernarg_address      = kernargs;
    local_pkt.completion_signal    = signal;

    uint16_t header =
        (HSA_PACKET_TYPE_KERNEL_DISPATCH << HSA_PACKET_HEADER_TYPE) |
        (1                               << HSA_PACKET_HEADER_BARRIER) |
        (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE) |
        (HSA_FENCE_SCOPE_SYSTEM << HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE);
    __atomic_store_n(&local_pkt.header, header, __ATOMIC_RELEASE);

    uint64_t ridx = hsa_queue_load_write_index_relaxed(queue);
    const uint32_t queueMask = queue->size - 1;
    ((hsa_kernel_dispatch_packet_t*)(queue->base_address))[ridx & queueMask] = local_pkt;
    hsa_queue_store_write_index_relaxed(queue, ridx + 1);

    hsa_signal_store_screlease(queue->doorbell_signal, ridx);
    hsa_signal_wait_scacquire(signal, HSA_SIGNAL_CONDITION_LT, 1,
                              UINT64_MAX, HSA_WAIT_STATE_BLOCKED);
    hsa_signal_destroy(signal);
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

int gct=0;
std::mutex g_mtx;

__attribute__((constructor)) static void setup(void){
  realLaunch = 0;
  realRegisterFunction = 0;
  realMalloc = 0;
  realMemcpy = 0;

  fprintf(stderr, "In setup of ld preload\n");

  // Select the gfx908 HIP device so hipMalloc targets the right GPU.
  int dev_count = 0;
  hipCheck(hipGetDeviceCount(&dev_count));
  int target_dev = -1;
  for (int i = 0; i < dev_count; i++) {
    hipDeviceProp_t prop;
    hipCheck(hipGetDeviceProperties(&prop, i));
    if (strstr(prop.gcnArchName, "gfx908")) { target_dev = i; break; }
  }
  if (target_dev < 0) { fprintf(stderr, "preload: no gfx908 device\n"); exit(1); }
  fprintf(stderr, "preload: using HIP device %d (gfx908)\n", target_dev);
  hipCheck(hipSetDevice(target_dev));

  // Find the matching HSA agent for dispatching via HSA APIs.
  // hsa_iterate_agents returns HSA_STATUS_INFO_BREAK when the callback stops early — that's expected.
  AgentSearch as = {};
  hsa_status_t iter_st = hsa_iterate_agents(find_gfx908_agent, &as);
  if (iter_st != HSA_STATUS_SUCCESS && iter_st != HSA_STATUS_INFO_BREAK) {
      const char* msg = nullptr; hsa_status_string(iter_st, &msg);
      fprintf(stderr, "HSA Error (iterate_agents): %s\n", msg); exit(1);
  }
  if (!as.found) { fprintf(stderr, "preload: no gfx908 HSA agent\n"); exit(1); }
  hsa_agent_t gpu = as.agent;

  // Find kernarg memory region.
  hsa_region_t kernarg_region = {0};
  hsa_status_t reg_st = hsa_agent_iterate_regions(gpu, find_kernarg_region, &kernarg_region);
  if (reg_st != HSA_STATUS_SUCCESS && reg_st != HSA_STATUS_INFO_BREAK) {
      const char* msg = nullptr; hsa_status_string(reg_st, &msg);
      fprintf(stderr, "HSA Error (iterate_regions): %s\n", msg); exit(1);
  }

  // Create a single HSA executable and load both code objects into it.
  // hsa_executable_freeze() will resolve the funptr1 relocation across objects.
  HSA_CHECK(hsa_executable_create_alt(HSA_PROFILE_FULL,
                                      HSA_DEFAULT_FLOAT_ROUNDING_MODE_DEFAULT,
                                      "", &g_hsa_exe));

  hsa_code_object_reader_t reader_funptr1, reader_mutatee;

  int fd_funptr1 = open("funptr1_gfx908.hsaco", O_RDONLY);
  if (fd_funptr1 < 0) { perror("open funptr1_gfx908.hsaco"); exit(1); }
  HSA_CHECK(hsa_code_object_reader_create_from_file(fd_funptr1, &reader_funptr1));
  fprintf(stderr, "preload: loading funptr1_gfx908.hsaco\n");
  HSA_CHECK(hsa_executable_load_agent_code_object(g_hsa_exe, gpu,
                                                  reader_funptr1, "", nullptr));
  fprintf(stderr, "preload: funptr1_gfx908.hsaco loaded OK\n");

  int fd_mutatee = open("mutatee/code-objects/1-hipv4-amdgcn-amd-amdhsa--gfx908", O_RDONLY);
  if (fd_mutatee < 0) { perror("open mutatee code object"); exit(1); }
  HSA_CHECK(hsa_code_object_reader_create_from_file(fd_mutatee, &reader_mutatee));
  fprintf(stderr, "preload: loading mutatee code object\n");
  HSA_CHECK(hsa_executable_load_agent_code_object(g_hsa_exe, gpu,
                                                  reader_mutatee, "", nullptr));
  fprintf(stderr, "preload: mutatee loaded OK\n");

  // Freeze: resolves all relocations, including funptr1 across the two objects.
  HSA_CHECK(hsa_executable_freeze(g_hsa_exe, ""));
  fprintf(stderr, "preload: HSA executable frozen — relocations resolved\n");

  // Find fn_array's host address and funptr1_gfx908's load base.
  // funptr1 is a device function — no VARIABLE_ADDRESS API.
  // Instead: look up __hip_cuid_db78d7753046f6bf (a GLOBAL OBJECT in funptr1_gfx908.hsaco)
  // whose VAddr is 0x3398. funptr1 is at VAddr 0x1320 in the same code object.
  // => funptr1_host_addr = cuid_host_addr - 0x3398 + 0x1320
  uint64_t* fnarray_host = nullptr;
  uint64_t  funptr1_host = 0;
  {
    hsa_executable_symbol_t sym_fnarray, sym_cuid;

    HSA_CHECK(hsa_executable_get_symbol_by_name(g_hsa_exe,
        "fn_array", &gpu, &sym_fnarray));
    uint64_t fnarray_addr = 0;
    HSA_CHECK(hsa_executable_symbol_get_info(sym_fnarray,
        HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, &fnarray_addr));
    fnarray_host = (uint64_t*)fnarray_addr;
    fprintf(stderr, "preload: fn_array  @ %#lx  [0]=%#lx [1]=%#lx [2]=%#lx\n",
            fnarray_addr, fnarray_host[0], fnarray_host[1], fnarray_host[2]);

    HSA_CHECK(hsa_executable_get_symbol_by_name(g_hsa_exe,
        "__hip_cuid_db78d7753046f6bf", &gpu, &sym_cuid));
    uint64_t cuid_addr = 0;
    HSA_CHECK(hsa_executable_symbol_get_info(sym_cuid,
        HSA_EXECUTABLE_SYMBOL_INFO_VARIABLE_ADDRESS, &cuid_addr));
    funptr1_host = cuid_addr - 0x3398 + 0x1320;
    fprintf(stderr, "preload: cuid      @ %#lx  => funptr1 @ %#lx\n",
            cuid_addr, funptr1_host);

    // Manually redirect fn_array[0] to the instrumentation library's funptr1.
    fnarray_host[0] = funptr1_host;
    fprintf(stderr, "preload: fn_array[0] patched to funptr1 @ %#lx\n", funptr1_host);
  }

  // Look up kernel descriptor symbols.
  hsa_executable_symbol_t sym_load, sym_call;
  HSA_CHECK(hsa_executable_get_symbol_by_name(g_hsa_exe,
      "_Z11load_fn_ptrPf.kd", &gpu, &sym_load));
  HSA_CHECK(hsa_executable_get_symbol_by_name(g_hsa_exe,
      "_Z7call_fnPKfi.kd", &gpu, &sym_call));

  uint64_t kern_load, kern_call;
  uint32_t ksize_load, ksize_call;
  uint32_t psize_load, psize_call;
  uint32_t gsize_load, gsize_call;

  HSA_CHECK(hsa_executable_symbol_get_info(sym_load,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kern_load));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_load,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &ksize_load));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_load,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &psize_load));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_load,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &gsize_load));

  HSA_CHECK(hsa_executable_symbol_get_info(sym_call,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_OBJECT, &kern_call));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_call,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_KERNARG_SEGMENT_SIZE, &ksize_call));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_call,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_PRIVATE_SEGMENT_SIZE, &psize_call));
  HSA_CHECK(hsa_executable_symbol_get_info(sym_call,
      HSA_EXECUTABLE_SYMBOL_INFO_KERNEL_GROUP_SEGMENT_SIZE, &gsize_call));

  // The kernel descriptor reports psize=0 for call_fn, but the indirect call
  // through fn_array into funptr1 (a separately-compiled code object) needs
  // scratch. Force 4 KiB so ROCR doesn't hit HandleInsufficientScratch.
  psize_call = 0x1000;

  fprintf(stderr, "preload: kern_load=0x%lx ksize=%u psize=%u gsize=%u\n",
         kern_load, ksize_load, psize_load, gsize_load);
  fprintf(stderr, "preload: kern_call=0x%lx ksize=%u psize=%u gsize=%u\n",
         kern_call, ksize_call, psize_call, gsize_call);

  // Create an HSA queue for dispatch.
  fprintf(stderr, "preload: creating HSA queue\n");
  HSA_CHECK(hsa_queue_create(gpu, 64, HSA_QUEUE_TYPE_SINGLE,
                             nullptr, nullptr, UINT32_MAX, UINT32_MAX, &g_hsa_queue));
  fprintf(stderr, "preload: queue created base=%p doorbell=%lu\n",
         g_hsa_queue->base_address, g_hsa_queue->doorbell_signal.handle);

  // Allocate device memory for the float v[3] array (via HIP, fine-grained).
  float* d_v = nullptr;
  hipCheck(hipMalloc((void**)&d_v, sizeof(float) * 3));
  fprintf(stderr, "preload: d_v=%p\n", d_v);

  // Skip load_fn_ptr — it would overwrite fn_array[0] which we already patched above.

  // --- Dispatch call_fn(const float* v, int i) ---
  // Kernarg layout: [ptr:8 bytes][i:4 bytes + 4 pad]
  void* ka_call = nullptr;
  uint32_t ka_call_sz = ksize_call < 64 ? 64 : ksize_call;
  HSA_CHECK(hsa_memory_allocate(kernarg_region, ka_call_sz, &ka_call));
  memset(ka_call, 0, ka_call_sz);
  *(const float**)ka_call = d_v;
  *(int*)((char*)ka_call + 8) = 0;  // i=0 → calls fn_array[0] (relocated to funptr1)
  fprintf(stderr, "preload: launching call_fn(v, 0)\n");
  dispatch_1x1(g_hsa_queue, kern_call, ka_call, psize_call, gsize_call);
  fprintf(stderr, "preload: call_fn done\n");

  hsa_memory_free(ka_call);
  hipFree(d_v);
}

__attribute__((destructor)) static void fini(void){
  if (g_hsa_queue) hsa_queue_destroy(g_hsa_queue);
  hsa_executable_destroy(g_hsa_exe);
}
