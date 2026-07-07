/*
 * amdgpu-scratch-abi.h — per-arch seam for giving an instrumented kernel a
 * reliable per-wave + per-lane scratch region to spill registers into.
 *
 * Rationale (see SPILL_ROADMAP.md): register spill needs a per-wave/per-lane
 * address. Hardware SCRATCH (private memory) provides exactly that — hardware
 * swizzles per lane and separates per wave. But the mechanism to enable/address
 * scratch differs by arch:
 *
 *   gfx908 / gfx90a (GCN/CDNA1-2): manual FLAT_SCRATCH = flat_scratch_init +
 *     wave_offset. Enabling flat_scratch_init adds a user SGPR, which SHIFTS the
 *     system SGPRs (wgid_x, ...) the kernel already reads → the entry prologue
 *     must "record the register update" and move them back. (Worst case.)
 *   gfx940 (CDNA3) / gfx11 (RDNA3): architected / absolute flat scratch — the
 *     hardware sets up FLAT_SCRATCH (no init add, no shift, no relocation), and
 *     scratch_* can take an absolute SGPR base. These become SIMPLER subclasses.
 *   gfx10/11 (RDNA): wave32 default (affects the SGPR pack) + different FLAT
 *     instruction encodings.
 *
 * This interface is the seam: gfx908 is the messy first implementation; later
 * arches slot in behind the same calls.
 */
#ifndef AMDGPU_SCRATCH_ABI_H
#define AMDGPU_SCRATCH_ABI_H

#include <cstdint>

class codeGen;   // global class (matches emit-amdgpu.h / emitter.h forward decls)

namespace Dyninst {
class AmdgpuKernelDescriptor;

namespace DyninstAPI {

class ScratchAbi {
public:
  virtual ~ScratchAbi() {}

  // Wave width in lanes: 64 (GCN/CDNA), 32 (RDNA default). The SGPR pack packs
  // one SGPR per lane, so this bounds how many fit in one packing VGPR.
  virtual uint32_t waveSize() const = 0;

  // Rewrite the caller kernel's descriptor to allocate `slotBytes` of private
  // scratch and enable the SGPRs needed to address it (flat_scratch_init +
  // wave_offset + user_sgpr_count bump + private_segment_fixed_size on gfx908).
  // Idempotent: only grows private_segment_fixed_size and only sets bits once.
  // Returns true if this shifted the system SGPRs (=> the entry prologue must
  // emit the relocation via emitScratchEntryPrologue).
  virtual bool enableScratchInKD(AmdgpuKernelDescriptor &kd, uint32_t slotBytes) = 0;

  // Emit, at kernel entry, the code that makes scratch usable for the rest of
  // the kernel: set up the scratch base (e.g. FLAT_SCRATCH) and undo any system-
  // SGPR shift caused by enableScratchInKD. Reads the (already-rewritten) KD to
  // recover the layout.
  virtual void emitScratchEntryPrologue(const AmdgpuKernelDescriptor &kd,
                                        codeGen &gen) = 0;

  // Per-lane spill of a single VGPR to/from private byte `offset`. Hardware
  // swizzles per lane; the per-wave base was established by the prologue. With
  // per-lane swizzle each register occupies only 4 bytes of the slot.
  virtual void emitScratchStore(uint32_t vreg, int32_t offset, codeGen &gen) = 0;
  virtual void emitScratchLoad(uint32_t vreg, int32_t offset, codeGen &gen) = 0;
};

// gfx908 (CDNA1) implementation (defined in emit-amdgpu.C).
ScratchAbi &gfx908ScratchAbi();

}  // namespace DyninstAPI
}  // namespace Dyninst

#endif  // AMDGPU_SCRATCH_ABI_H
