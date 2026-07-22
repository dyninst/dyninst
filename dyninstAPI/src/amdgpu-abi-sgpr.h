/*
 * amdgpu-abi-sgpr.h — compute the ABI SGPR layout from a kernel descriptor.
 *
 * A "pass" over the .kd that figures out which SGPRs the hardware/ABI loads at
 * wavefront start (user SGPRs + system SGPRs), and at which SGPR index each lands.
 * This is the metadata needed to decide which registers are LIVE at the kernel
 * entry and must not be clobbered / must be relocated by instrumentation (e.g.
 * the entry springboard, the scratch prologue).
 *
 * Ordering & sizes are from the AMDGPU ABI ("SGPR Register Set Up Order",
 * llvm.org/docs/AMDGPUUsage.html). USER SGPRs are packed first (set by CP), then
 * SYSTEM SGPRs (set by ADC/SPI). Registers are dense: a disabled entry consumes
 * no SGPR. The KD's user_sgpr_count is authoritative for where system SGPRs begin
 * (it also accounts for preloaded kernargs, which sit between the user SGPRs and
 * the work-group ids).
 *
 * USER SGPRs (in order):
 *   Private Segment Buffer (4)  enable_sgpr_private_segment_buffer
 *   Dispatch Ptr (2)            enable_sgpr_dispatch_ptr
 *   Queue Ptr (2)               enable_sgpr_queue_ptr
 *   Kernarg Segment Ptr (2)     enable_sgpr_kernarg_segment_ptr
 *   Dispatch Id (2)             enable_sgpr_dispatch_id
 *   Flat Scratch Init (2)       enable_sgpr_flat_scratch_init   (GFX6-GFX10 except GFX942)
 *   Private Segment Size (1)    enable_sgpr_private_segment_size
 *   [Preloaded Kernargs]        (folded into user_sgpr_count)
 * SYSTEM SGPRs (begin at user_sgpr_count, in order):
 *   Work-Group Id X (1)         enable_sgpr_workgroup_id_x
 *   Work-Group Id Y (1)         enable_sgpr_workgroup_id_y
 *   Work-Group Id Z (1)         enable_sgpr_workgroup_id_z
 *   Work-Group Info (1)         enable_sgpr_workgroup_info
 *   Scratch Wavefront Offset (1) enable_sgpr_private_segment_wavefront_offset (RSRC2 bit 0)
 *
 * NOTE: FLAT_SCRATCH = flat_scratch_init (per-QUEUE, in Flat Scratch Init SGPRs) +
 * scratch_wavefront_offset (per-WAVE, set by SPI). The wave offset is the ONLY
 * per-wave datum and is a GP system SGPR — so it must survive to the code that
 * builds the per-wave scratch base.
 */
#ifndef AMDGPU_ABI_SGPR_H
#define AMDGPU_ABI_SGPR_H

#include <cstdint>
#include "AmdgpuKernelDescriptor.h"

namespace Dyninst {
namespace DyninstAPI {

struct AbiSgprLayout {
  // -1 = not enabled (not present). Otherwise the first SGPR index of the field.
  int privSegBuf = -1;      // 4 SGPRs
  int dispatchPtr = -1;     // 2
  int queuePtr = -1;        // 2
  int kernargPtr = -1;      // 2
  int dispatchId = -1;      // 2
  int flatScratchInit = -1; // 2  (scratch base, per-queue)
  int privSegSize = -1;     // 1
  uint32_t userSgprCount = 0;   // system SGPRs begin here (from the KD, authoritative)

  int wgidX = -1;           // 1
  int wgidY = -1;           // 1
  int wgidZ = -1;           // 1
  int wgInfo = -1;          // 1
  int waveOffset = -1;      // 1  (scratch wavefront offset, per-wave)

  // One past the last live ABI SGPR. [0, liveSgprEnd) is the range of SGPRs that
  // hold live ABI inputs at kernel entry — instrumentation inserted before those
  // inputs are consumed (e.g. the entry springboard) must not clobber this range.
  uint32_t liveSgprEnd = 0;

  bool scratchEnabled() const { return flatScratchInit >= 0; }
};

inline AbiSgprLayout computeAbiSgprLayout(const Dyninst::AmdgpuKernelDescriptor &kd) {
  AbiSgprLayout L;
  uint32_t s = 0;
  auto place = [&](bool en, int n, int &out) { if (en) { out = (int)s; s += (uint32_t)n; } };

  // USER SGPRs (packed from s0, in ABI order).
  place(kd.getKernelCodeProperty_EnableSgprPrivateSegmentBuffer(), 4, L.privSegBuf);
  place(kd.getKernelCodeProperty_EnableSgprDispatchPtr(),          2, L.dispatchPtr);
  place(kd.getKernelCodeProperty_EnableSgprQueuePtr(),             2, L.queuePtr);
  place(kd.getKernelCodeProperty_EnableSgprKernargSegmentPtr(),    2, L.kernargPtr);
  place(kd.getKernelCodeProperty_EnableSgprDispatchId(),           2, L.dispatchId);
  place(kd.getKernelCodeProperty_EnableSgprFlatScratchInit(),      2, L.flatScratchInit);
  place(kd.getKernelCodeProperty_EnablePrivateSegmentSize(),       1, L.privSegSize);

  // System SGPRs begin at the KD's declared user_sgpr_count (covers preloaded
  // kernargs, which we don't itemize here).
  L.userSgprCount = kd.getCOMPUTE_PGM_RSRC2_UserSgprCount();
  s = L.userSgprCount;
  place(kd.getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX(),  1, L.wgidX);
  place(kd.getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY(),  1, L.wgidY);
  place(kd.getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ(),  1, L.wgidZ);
  place(kd.getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo(), 1, L.wgInfo);
  place(kd.getCOMPUTE_PGM_RSRC2_EnablePrivateSegment(),    1, L.waveOffset);  // wave offset
  L.liveSgprEnd = s;
  return L;
}

// If scratch (flat_scratch_init) is not already enabled, enabling it consumes 2
// additional USER SGPRs (Flat Scratch Init) and shifts every SYSTEM SGPR up by 2.
// Returns the shift amount (0 if scratch is already enabled).
inline uint32_t scratchEnableSgprShift(const Dyninst::AmdgpuKernelDescriptor &kd) {
  return kd.getKernelCodeProperty_EnableSgprFlatScratchInit() ? 0u : 2u;
}

}  // namespace DyninstAPI
}  // namespace Dyninst

#endif  // AMDGPU_ABI_SGPR_H
