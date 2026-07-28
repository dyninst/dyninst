/*
 * amdgpu-kernel-meta.C — mapped_object accessors for the canonical per-kernel KD
 * metadata (pillar B). See amdgpu-kernel-meta.h.
 *
 * getAmdgpuKernelMeta() sources the KD ONCE from the ".kd" symbol (the symtab) and
 * caches a KernelMeta on the owning mapped_object. flushAmdgpuKernelMeta() commits
 * every dirty KD back to the code object's data space; it is called from
 * BinaryEdit::writeFile — dyninst's normal parse-once / commit-at-emit lifecycle —
 * so KD changes ride the same data-region write-back that persists ".kd" today. It
 * does NOT emit new symbols (writeFile can't add arbitrary AMDGPU symbols; only the
 * existing ".kd" data modification is persisted).
 */

#include "mapped_object.h"
#include "addressSpace.h"
#include "amdgpu-kernel-meta.h"
#include "external/amdgpu/AMDGPUEFlags.h"

#include <cstring>

using Dyninst::DyninstAPI::KernelMeta;

KernelMeta *mapped_object::getAmdgpuKernelMeta(const std::string &kernelName) {
  auto it = amdgpuKernelMeta_.find(kernelName);
  if (it != amdgpuKernelMeta_.end())
    return it->second;

  // Parse ONCE from the ".kd" symbol. Absent => not a kernel (ordinary device fn).
  int_symbol kdSym;
  if (!getSymbolInfo(kernelName + ".kd", kdSym)) {
    amdgpuKernelMeta_[kernelName] = nullptr;   // negative-cache so we don't re-look-up
    return nullptr;
  }

  AddressSpace *as = proc();
  const size_t kdSize = sizeof(llvm::amdhsa::kernel_descriptor_t);
  uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
  if (!as || !as->readDataSpace(reinterpret_cast<const void *>(kdSym.getAddr()),
                                static_cast<u_int>(kdSize), kdBytes, true)) {
    amdgpuKernelMeta_[kernelName] = nullptr;
    return nullptr;
  }

  KernelMeta *km = new KernelMeta(kernelName, kdSym.getAddr(), kdBytes,
                                  static_cast<uint32_t>(kdSize),
                                  EF_AMDGPU_MACH_AMDGCN_GFX908);
  amdgpuKernelMeta_[kernelName] = km;
  return km;
}

void mapped_object::flushAmdgpuKernelMeta() {
  AddressSpace *as = proc();
  for (auto &kv : amdgpuKernelMeta_) {
    KernelMeta *km = kv.second;
    if (!km)
      continue;
    if (km->dirty && as) {
      uint8_t kdBytes[sizeof(llvm::amdhsa::kernel_descriptor_t)];
      km->kd.writeToMemory(kdBytes);
      if (!as->writeDataSpace(reinterpret_cast<void *>(km->kdAddr),
                              static_cast<u_int>(km->kdSize), kdBytes)) {
        fprintf(stderr, "[amdgpu] warning: failed to commit KD for '%s'\n",
                km->name.c_str());
      }
      km->dirty = false;
    }
    delete km;
    kv.second = nullptr;
  }
  amdgpuKernelMeta_.clear();
}
