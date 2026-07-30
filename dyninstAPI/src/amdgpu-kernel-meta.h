/*
 * amdgpu-kernel-meta.h — the single, canonical, in-memory per-kernel descriptor
 * metadata (pillar B).
 *
 * The AMDGPU kernel descriptor (".kd") used to be re-parsed from the ELF/data-space
 * at ~8 independent sites and mutated-then-written-back EAGERLY, which left the
 * original-vs-grown kernarg_segment_size ambiguous (the source of a 2D offset bug).
 *
 * This replaces all of that with ONE KernelMeta per kernel, sourced ONCE from the
 * kernel's ".kd" symbol (the symtab IS the source of truth), cached on the owning
 * mapped_object, mutated IN MEMORY, and committed back to the code object ONCE at
 * ELF-emit (mapped_object::flushAmdgpuKernelMeta, invoked from BinaryEdit::writeFile).
 * No site re-parses the ELF; no site eagerly writes the KD.
 *
 * It holds the parsed AmdgpuKernelDescriptor (the working copy = source of truth
 * until flush), the AbiSgprLayout (computed once, refreshed on a structural change),
 * and — kept DISTINCT — the ORIGINAL (pristine) kernarg_segment_size / granted
 * VGPR count / flat-scratch state vs the current/grown KD, plus the per-wave arena
 * stride and a dirty flag.
 */
#ifndef AMDGPU_KERNEL_META_H
#define AMDGPU_KERNEL_META_H

#include <cstdint>
#include <string>

#include "dyntypes.h"                  // Dyninst::Address
#include "AmdgpuKernelDescriptor.h"
#include "amdgpu-abi-sgpr.h"

namespace Dyninst { namespace DyninstAPI {

struct KernelMeta {
  std::string name;                    // mangled kernel name (WITHOUT ".kd")
  Dyninst::Address kdAddr = 0;         // address of the ".kd" object in data space
  uint32_t kdSize = 0;                 // sizeof(kernel_descriptor_t) == 64

  // THE working copy: the source of truth for the KD from parse until flush. Every
  // read and every mutation goes through this; nothing else touches the ELF ".kd".
  Dyninst::AmdgpuKernelDescriptor kd;

  // Facts captured at first parse (BEFORE any instrumentation mutation), kept DISTINCT
  // from the grown/current KD so the original-vs-grown kernarg ambiguity is explicit:
  //   originalKernargSize  — pristine kernarg_segment_size. The launcher appends the
  //                          per-wave buffer pointer at THIS offset; the entry prologue
  //                          dereferences it here. dyninst later GROWS kd's kernarg by 8,
  //                          so kd.getKernargSize() != originalKernargSize after finalize.
  //   COV5 implicit-args block sits at (originalKernargSize - 256).
  uint32_t originalKernargSize = 0;
  uint32_t originalGrantedVgpr = 0;    // pristine granted VGPR count = (gran+1)*4
  bool     originalScratchEnabled = false;
  // Pristine private_segment_fixed_size = the caller kernel's OWN scratch frame [0,O).
  // Our IACR/spill region and the inserted callee's frame are seated ABOVE it (SADDR=O,
  // s32Base += O) so they don't alias the caller's live frame; 0 for a kernel with no
  // frame of its own (every leaf/getpc-free case so far).
  uint32_t originalPrivateSegment = 0;

  uint32_t perWaveStride = 4096;       // per-wave arena stride (bump-allocator)
  bool     dirty = false;              // KD mutated since parse -> committed at flush

  KernelMeta(const std::string &n, Dyninst::Address addr, const uint8_t *bytes,
             uint32_t size, unsigned mach)
      : name(n), kdAddr(addr), kdSize(size),
        kd(const_cast<uint8_t *>(bytes), size, mach) {
    originalKernargSize    = kd.getKernargSize();
    originalGrantedVgpr    = (kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() + 1) * 4;
    originalScratchEnabled = kd.getKernelCodeProperty_EnableSgprFlatScratchInit();
    originalPrivateSegment = kd.getPrivateSegmentFixedSize();
    layoutCache_           = computeAbiSgprLayout(kd);
  }

  // ABI SGPR layout of the CURRENT KD (cached, "computed once"). Enabling flat-scratch
  // shifts the system SGPRs, so refreshLayout() must be called after that structural
  // change; ordinary field mutations (grant size, kernarg growth) don't move SGPRs.
  const AbiSgprLayout &layout() const { return layoutCache_; }
  void refreshLayout() { layoutCache_ = computeAbiSgprLayout(kd); }

  // One past the highest allocatable numbered SGPR in the CURRENT (grown) grant.
  uint32_t grantedSgprTop() const {
    return ((kd.getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() / 2) + 1) * 16;
  }
  // CURRENT granted VGPR count (grows as bumpCallerKdForCallee raises it).
  uint32_t grantedVgpr() const {
    return (kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() + 1) * 4;
  }

private:
  AbiSgprLayout layoutCache_;
};

}}  // namespace Dyninst::DyninstAPI

#endif  // AMDGPU_KERNEL_META_H
