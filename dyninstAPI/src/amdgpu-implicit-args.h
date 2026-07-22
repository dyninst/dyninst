/*
 * amdgpu-implicit-args.h — layout of the Implicit-Arg Capture Region (IACR).
 *
 * Phase 3 of the AMDGPU hostcall roadmap lets an inserted callee be ordinary HIP
 * device code that reads IMPLICIT ABI inputs (blockIdx / threadIdx / blockDim /
 * gridDim / kernargs). Those inputs are only valid in their ABI registers at
 * KERNEL ENTRY; the kernel body freely reuses those registers afterward (proven
 * by rocgdb: wgid_x sits in its entry SGPR only at early insertion sites, then the
 * kernel overwrites it with blockIdx*blockDim, then array pointers). So an inserted
 * call cannot read the "live" ABI register at the call site — it must read a value
 * CAPTURED at entry.
 *
 * The IACR is that capture area: a small, WRITE-ONCE-AT-ENTRY / READ-ONLY region
 * reserved at the BOTTOM of the instrumentation scratch area (per-lane byte offsets
 * from FLAT_SCRATCH, matching the caller-save spill's addressing). Everything else
 * that touches scratch — the kernel's own private segment, our per-call spill, and
 * callee frames — lives ABOVE it (offset >= IACR_BYTES), so nothing overwrites it:
 * the kernel may trash any register it deems dead, but never this region.
 *
 * Per-lane scratch layout when implicit-arg forwarding is enabled:
 *   [0,           IACR_BYTES)          IACR (captured implicit args, RO after entry)
 *   [IACR_BYTES,  IACR_BYTES + R)      per-call caller-save spill (reused each call)
 *   [ .. above .. ]                    callee buffer-scratch frames
 *      s32Base(per-wave) = roundup((IACR_BYTES + R) * waveSize, 0x400)
 * The IACR offset is FIXED (independent of the per-call callee footprint R) so the
 * entry-capture and the per-call retrieve agree on where each datum lives.
 *
 * Datum kinds:
 *   uniform  — same value across the wave (from an SGPR at entry). Captured by
 *              broadcasting to a VGPR then scratch_store; retrieved by scratch_load
 *              then v_readfirstlane into the callee's ABI SGPR.
 *   per-lane — differs per lane (workitem-id, packed in v0 at entry). Captured by
 *              scratch_store v0 (hardware per-lane swizzle); retrieved by
 *              scratch_load straight into the callee's per-lane ABI VGPR (v31).
 */
#ifndef AMDGPU_IMPLICIT_ARGS_H
#define AMDGPU_IMPLICIT_ARGS_H

#include <cstdint>

namespace Dyninst {
namespace DyninstAPI {

struct ImplicitArgLayout {
  // Per-lane byte offsets of each captured datum within the IACR.
  enum Offset : int32_t {
    OFF_WGID_X   = 0,   // uniform  : blockIdx.x  (wgid_x SGPR at entry)      4B  [3a]
    OFF_WGID_Y   = 4,   // uniform  : blockIdx.y                              4B  [3a]
    OFF_WGID_Z   = 8,   // uniform  : blockIdx.z                              4B  [3a]
    OFF_WITEMID  = 12,  // per-lane : packed workitem-id (v0 at entry)        4B  [3c]
    OFF_DISPATCH = 16,  // uniform  : dispatch ptr (lo@16, hi@20)             8B  [3b]
    OFF_KERNARG  = 24,  // uniform  : kernarg ptr  (lo@24, hi@28)             8B  [3d]
    OFF_PWBASE   = 32,  // uniform  : per-wave buffer base ptr (lo@32,hi@36)  8B  [per-wave var]
  };
  // Total reserved size (per lane). The spill region and callee frames shift up by
  // this amount when implicit-arg forwarding is enabled. Keep 4-byte aligned; a
  // little headroom above the highest slot leaves room to grow.
  static const uint32_t BYTES = 40;

  // Callee ABI registers the retrieved values are forwarded INTO (gfx908 device-fn
  // calling convention, empirically pinned): blockIdx x/y/z -> s12/s13/s14; packed
  // workitem-id -> v31; the kernarg/implicit-args ptr (3b) -> the callee's
  // metadata-dependent SGPR pair, from which the callee derives blockDim/gridDim.
  enum { ABI_BLOCKIDX_X = 12, ABI_BLOCKIDX_Y = 13, ABI_BLOCKIDX_Z = 14,
         ABI_WITEMID_VGPR = 31 };
};

}  // namespace DyninstAPI
}  // namespace Dyninst

#endif  // AMDGPU_IMPLICIT_ARGS_H
