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

  // Seed the KernelMeta's KD with the object's REAL AMDGPU machine, not a hardcoded
  // GFX908. Everything downstream (enableScratchInKD, the entry-prologue scratch idiom,
  // packed-vs-separate work-item id) branches on AmdgpuKernelDescriptor::
  // supportsArchitectedFlatScratch()/isGfx9() which read this mach — a wrong default makes
  // a gfx942 kernel emit the gfx908 manual FLAT_SCRATCH setup (garbage FLAT_SCRATCH ->
  // scratch aperture violation). getArchitecture() folds gfx940/gfx942 (both CDNA3
  // architected flat scratch) onto Arch_amdgpu_gfx940; map that to GFX942 so architected
  // detection fires.
  unsigned amdgpuMach = EF_AMDGPU_MACH_AMDGCN_GFX908;
  if (image *pimg = parse_img()) {
    if (SymtabAPI::Symtab *st = pimg->getObject()) {
      switch (st->getArchitecture()) {
      case Dyninst::Arch_amdgpu_gfx908: amdgpuMach = EF_AMDGPU_MACH_AMDGCN_GFX908; break;
      case Dyninst::Arch_amdgpu_gfx90a: amdgpuMach = EF_AMDGPU_MACH_AMDGCN_GFX90A; break;
      case Dyninst::Arch_amdgpu_gfx940: amdgpuMach = EF_AMDGPU_MACH_AMDGCN_GFX942; break;
      case Dyninst::Arch_amdgpu_gfx950: amdgpuMach = EF_AMDGPU_MACH_AMDGCN_GFX950; break;
      default: break;
      }
    }
  }
  KernelMeta *km = new KernelMeta(kernelName, kdSym.getAddr(), kdBytes,
                                  static_cast<uint32_t>(kdSize), amdgpuMach);
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
