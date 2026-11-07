/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// gfx908 scratch ABI: the kernel scratch-entry prologue (implicit-arg capture region),
// the scratch spill store/load, and the callee-stack set-up for an inserted call.
//
// This is an OPEN-CODED emitter: it holds no per-gen namespace itself; every instruction is
// emitted through the runtime-selected EmitterAmdgpu for the instrumented object (resolved from
// the codeGen's bound address space via getAmdgpuEmitter), using that emitter's public raw*
// packers + toOpCode. The body is the validated gfx908 sequence; the thin emit* shims below give
// it the fork's call shape while forwarding to the base emitter.

#include "amdgpu-scratch-abi.h"
#include "amdgpu-abi-sgpr.h"
#include "amdgpu-implicit-args.h"
#include "AmdgpuKernelDescriptor.h"
#include "codegen/codegen.h"
#include "codegen/emitters/amdgpu/emit-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "dyn_register.h"
#include "patching/instPoint.h"
#include "addressSpace.h"

#include <cstdint>

using namespace Dyninst;

// Per-wave arena stride (bytes). BPatch_addressSpace::allocatePerWave publishes it; the scratch
// prologue reads it (the prologue's codeGen addrSpace can't carry it). Default 4096 keeps
// un-instrumented / non-perWaveVar kernels at the old fixed stride.
static uint32_t g_amdgpuPerWaveStride = 4096;

namespace {

// gfx908 special/inline operand encodings.
enum { GFX908_FLAT_SCRATCH_LO = 102, GFX908_FLAT_SCRATCH_HI = 103,
       GFX908_INLINE_0 = 128, GFX908_SADDR_BASE = 94 };

// Gen-neutral opcode identities used by the body (resolved to this object's gen via toOpCode).
constexpr AmdgpuOp S_MOV_B32            = AmdgpuOp::S_MOV_B32;
constexpr AmdgpuOp S_ADD_U32            = AmdgpuOp::S_ADD_U32;
constexpr AmdgpuOp S_ADDC_U32           = AmdgpuOp::S_ADDC_U32;
constexpr AmdgpuOp S_OR_B32             = AmdgpuOp::S_OR_B32;
constexpr AmdgpuOp S_AND_B32            = AmdgpuOp::S_AND_B32;
constexpr AmdgpuOp S_MUL_I32            = AmdgpuOp::S_MUL_I32;
constexpr AmdgpuOp S_LSHR_B32           = AmdgpuOp::S_LSHR_B32;
constexpr AmdgpuOp S_MULK_I32           = AmdgpuOp::S_MULK_I32;
constexpr AmdgpuOp S_WAITCNT            = AmdgpuOp::S_WAITCNT;
constexpr AmdgpuOp S_LOAD_DWORD         = AmdgpuOp::S_LOAD_DWORD;
constexpr AmdgpuOp S_LOAD_DWORDX2       = AmdgpuOp::S_LOAD_DWORDX2;
constexpr AmdgpuOp V_MOV_B32            = AmdgpuOp::V_MOV_B32;
constexpr AmdgpuOp V_READFIRSTLANE_B32  = AmdgpuOp::V_READFIRSTLANE_B32;
constexpr AmdgpuOp V_LSHL_OR_B32        = AmdgpuOp::V_LSHL_OR_B32;
constexpr AmdgpuOp GLOBAL_STORE_DWORD   = AmdgpuOp::GLOBAL_STORE_DWORD;
constexpr AmdgpuOp GLOBAL_LOAD_DWORD    = AmdgpuOp::GLOBAL_LOAD_DWORD;

// Thin shims: forward the fork's call shape to the runtime-selected emitter's public packers.
// gen is already bound to the instrumented object's address space (AmdgpuPrologue::generate), so
// getAmdgpuEmitter(gen) recovers that object's per-gen EmitterAmdgpu.
inline void emitSop1Raw(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSop1(E->toOpCode(op), sdst, ssrc0, /*hasLiteral=*/false, /*literal=*/0, g);
}
inline void emitSop2Raw(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, uint32_t ssrc1, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSop2(E->toOpCode(op), sdst, ssrc0, ssrc1, g);
}
inline void emitSop2RawWithLiteral(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSop2WithSrc1Literal(E->toOpCode(op), sdst, ssrc0, literal, g);
}
inline void emitSopP(AmdgpuOp op, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSopP(E->toOpCode(op), simm16, g);
}
inline void emitSopK(AmdgpuOp op, uint32_t dest, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSopK(E->toOpCode(op), dest, simm16, g);
}
inline void emitSmem(AmdgpuOp op, uint64_t sdata, uint64_t sbase, uint64_t offset, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawSmem(E->toOpCode(op), sdata, sbase, offset, g);
}
inline void emitVop1Reg(AmdgpuOp op, uint32_t vdst, uint32_t src0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawVop1Reg(E->toOpCode(op), vdst, src0, g);
}
inline void emitVop3a(AmdgpuOp op, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawVop3a(E->toOpCode(op), vdst, src0, src1, src2, g);
}
inline void emitFlatGlobal(AmdgpuOp op, uint32_t vdst, uint32_t addr, uint32_t data,
                           uint32_t saddr, int32_t offset, codeGen &g, bool glc, uint32_t seg) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g);
  E->rawFlatGlobal(E->toOpCode(op), vdst, addr, data, saddr, offset, g, glc, seg);
}

class Gfx908ScratchAbi : public Dyninst::DyninstAPI::ScratchAbi {
public:
  uint32_t waveSize() const override { return 64; }

  bool enableScratchInKD(Dyninst::AmdgpuKernelDescriptor &kd, uint32_t slotBytes) override {
    if (slotBytes > kd.getPrivateSegmentFixedSize())
      kd.setPrivateSegmentFixedSize(slotBytes);
    // ARCHITECTED/ABSOLUTE flat scratch (gfx942/CDNA3): the hardware provides FLAT_SCRATCH
    // per wave with NO flat_scratch_init user SGPR and NO wavefront-offset system SGPR. But
    // the kernel must still DECLARE it uses private/scratch memory or ROCr/HW won't provision
    // the scratch aperture — FLAT_SCRATCH stays 0 and every scratch_store faults (verified on
    // MI300A: native gfx942 scratch kernels set COMPUTE_PGM_RSRC2.ENABLE_PRIVATE_SEGMENT=1,
    // RSRC2=0x85 vs our 0x84). So set that bit here (idempotent). No user SGPR is appended, so
    // the system SGPRs are NOT shifted — return false (no relocation prologue needed). This is
    // the "simpler subclass" the seam was designed for.
    if (kd.supportsArchitectedFlatScratch()) {
      if (!kd.getCOMPUTE_PGM_RSRC2_EnablePrivateSegment())
        kd.setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(true);
      return false;
    }
    // Already scratch-enabled (compiler set flat_scratch_init up): reuse its
    // FLAT_SCRATCH, only grew the size, no new (user-SGPR) shift. BUT a kernel that
    // originally used 0 private scratch (e.g. a non-leaf kernel whose call spilled
    // nothing) can have flat_scratch_init enabled yet the per-wave scratch WAVEFRONT
    // OFFSET SGPR disabled (EnablePrivateSegment=0) — its own FLAT_SCRATCH setup was
    // dead. Now that instrumentation spills to FLAT_SCRATCH we need ROCr to actually
    // provide the per-wave offset, so enable it if absent. This appends ONE system
    // SGPR (the wave offset, after the work-group IDs) and does NOT shift the user
    // SGPRs, so the relocated body's register expectations are unchanged.
    if (kd.getKernelCodeProperty_EnableSgprFlatScratchInit()) {
      if (!kd.getCOMPUTE_PGM_RSRC2_EnablePrivateSegment())
        kd.setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(true);
      return false;
    }
    const uint32_t uc = kd.getCOMPUTE_PGM_RSRC2_UserSgprCount();
    kd.setKernelCodeProperty_EnableSgprFlatScratchInit(true);  // +2 user SGPRs at s[uc:uc+1]
    kd.setCOMPUTE_PGM_RSRC2_UserSgprCount(uc + 2);
    kd.setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(true);        // wave-offset system SGPR
    return true;                                               // system SGPRs shifted by +2
  }

  void emitScratchEntryPrologue(const Dyninst::AmdgpuKernelDescriptor &kd,
                                uint32_t originalKernargSize,
                                uint32_t systemSgprShift,
                                uint32_t originalPrivateSegment,
                                codeGen &gen) override {
    // Architected/absolute flat scratch (gfx942/CDNA3, gfx11 RDNA3): the HARDWARE sets
    // up FLAT_SCRATCH per wave — there is NO flat_scratch_init user SGPR, NO scratch
    // wavefront-offset system SGPR, and hence NO system-SGPR shift and NO manual
    // FLAT_SCRATCH add here. Detected from the kernel's own ELF mach (the KD carries it),
    // so a single emitter body serves gfx908 (manual) and gfx942 (architected). The two
    // ISA deltas that force a branch (docs/reference/amdgpu-lowlevel.md §1) both key off
    // this predicate: (1) manual-vs-architected FLAT_SCRATCH. See amdgpu-scratch-abi.h.
    const bool architected = kd.usesArchitectedFlatScratch();
    // (2) separate-vs-packed work-item id at entry is a DISTINCT ISA fact (CDNA3), gated on its
    // own predicate so a future architected-but-non-packed arch is not mis-handled.
    const bool packedWid = kd.usesPackedWorkitemId();
    // Read the ABI SGPR layout from the (already scratch-enabled) KD: exact indices
    // of flat_scratch_init (per-queue base) and the scratch wavefront offset.
    Dyninst::DyninstAPI::AbiSgprLayout L = Dyninst::DyninstAPI::computeAbiSgprLayout(kd);
    const uint32_t fsi     = (uint32_t)L.flatScratchInit;   // e.g. s[6:7] (gfx908 only)
    const uint32_t waveoff = (uint32_t)L.waveOffset;        // e.g. s9     (gfx908 only)

    // Set FLAT_SCRATCH (s[102:103]) = flat_scratch_init + wave_offset — the exact
    // per-wave base the compiler computes (verified vs scratch_probe:
    // `s_add_u32 flat_scratch_lo, s6, s9`). gfx908 does NOT pre-init FLAT_SCRATCH, so
    // we compute it here. It's a SPECIAL register (above MAX_SGPR_ID), NOT from the
    // allocatable pool, so it costs zero GP SGPRs. wave_offset (s9) is CONSUMED here
    // and dead afterward — the scratch_* spills read the base implicitly from
    // FLAT_SCRATCH, so no s[94:95] pair and no per-lane vAddr are needed.
    // MUST be an ADD (base + per-wave offset), not a mov, or every wave aliases.
    // ARCHITECTED (gfx942): the hardware already loaded FLAT_SCRATCH with the per-wave
    // base, so skip the manual add entirely (there is no flat_scratch_init/wave_offset).
    if (!architected) {
      emitSop2Raw(S_ADD_U32,  GFX908_FLAT_SCRATCH_LO, fsi,     waveoff,         gen); // FS_LO = fsi_lo + wave_off
      emitSop2Raw(S_ADDC_U32, GFX908_FLAT_SCRATCH_HI, fsi + 1, GFX908_INLINE_0, gen); // FS_HI = fsi_hi + carry
    }
    // NOTE: the scratch SADDR register (=0) is NOT set here — it lives at the top of
    // the tight grant (reservedBase+2, kernel-relative) and is zeroed per-trampoline
    // in emitCall, so the prologue stays decoupled from reservedBase (FLAT_SCRATCH is
    // a fixed special reg). This is what lets the grant be sized down to the actual
    // requirement without the prologue needing to know it.

    // Enabling flat_scratch_init shifted the SYSTEM SGPRs up by 2 (Flat Scratch
    // Init occupies 2 user SGPRs). Move the ones the un-shifted kernel body reads
    // (wgid/info) back down by 2. Do AFTER the add (which consumes s[fsi],
    // s[waveoff]); ascending order is collision-free.
    // 0 for a non-leaf kernel (compiler already had flat_scratch_init: no shift; the
    // movs would otherwise relocate wgid onto the flat_scratch_init SGPRs and desync the
    // relocated body). 2 for the leaf case enableScratchInKD just shifted.
    const uint32_t SHIFT = systemSgprShift;
    const int sysRegs[4] = { L.wgidX, L.wgidY, L.wgidZ, L.wgInfo };
    for (int p : sysRegs)
      if (p >= 0 && SHIFT > 0)
        emitSop1Raw(S_MOV_B32, (uint32_t)p - SHIFT, (uint32_t)p, gen);
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);

    // Implicit-Arg Capture Region (IACR): capture the implicit ABI inputs HERE, at
    // entry, where they are still valid, into a fixed read-only scratch region the
    // kernel body never touches (offsets [0,IACR_BYTES); spill/callee frames sit
    // above it). An inserted call retrieves them at the call site (emitCall) —
    // reading the LIVE wgid register there is unreliable because the kernel reuses
    // it (rocgdb-proven). Only wgid (3a) for now; 3b/3c/3d add more slots.
    // SADDR (=reservedBase+2, matches emitCall's derivation) is set to 0 here; a
    // temp VGPR above the kernel's own usage broadcasts each scalar wgid before the
    // per-lane scratch_store (uniform value written to every lane's slot).
    {   // implicit-arg capture is on by default (see emitCall's implBase note)
      using IAL = Dyninst::DyninstAPI::ImplicitArgLayout;

      // --- Pillar A: allocate the prologue's scratch temps by LIVENESS -------------
      // The entry codeGen carries a POINT-SPECIALIZED registerSpace (AmdgpuPrologue::
      // generate). At kernel entry only the ABI-preloaded regs are live; we mark those
      // (and the workitem-id VGPRs v0/v1/v2) off-limits — the prologue itself READS the
      // kernarg pointer (SMEM base/dim loads), the relocated workgroup-id SGPRs
      // (blockIdx), FLAT_SCRATCH/wave_offset (already consumed), and v0/v1/v2 (threadIdx),
      // so they must not be handed out as scratch — then allocateGprBlock the temps:
      //   saddr  1 SGPR  (scratch SADDR, held at 0 for every IACR store)
      //   pwr    2 SGPR  (per-wave slice base pair; even/PAIR-aligned for S_LOAD_DWORDX2)
      //   w[0..] up to 11 SGPR working temps (the 2D/3D block-linear wid math)
      //   vtmp   1 VGPR  (broadcast scalar->lane + the v0|v1<<10|v2<<20 packing)
      // No grant bump is needed: the allocator picks dead-at-entry regs already inside
      // the wave's grant, low-first (the kernel body recomputes them). FALLBACK to
      // computed placement (a temp window above the ABI live-in ceiling) when there is
      // no point / liveness is off / the allocator can't satisfy — the grant is never
      // bumped just for these.
      registerSpace *prs = gen.rs();
      instPoint *ppt = gen.point();
      uint32_t saddr = 0, vtmp = 0, pwr = 0, w[11] = {0};
      Register bSaddr, bPwr, bWork, bVtmp;
      bool okS = false, okP = false, okW = false, okV = false, allocated = false;
      if (prs && ppt) {
        auto liveS = [&](uint32_t id) {
          Register r = Register(id);
          if (registerSlot *s = (*prs)[r]) s->liveState = registerSlot::live;
        };
        auto liveV = [&](uint32_t id) {
          Register r = Register(NS_amdgpu::VGPR_OPERAND_BASE + id);
          if (registerSlot *s = (*prs)[r]) s->liveState = registerSlot::live;
        };
        // ABI user+system SGPRs (covers kernarg, flat_scratch_init, wave_offset, and the
        // wgid/wginfo both pre- and post-relocation, since L.liveSgprEnd > relocated pos).
        for (uint32_t i = 0; i < L.liveSgprEnd; i++) liveS(i);
        liveV(0); liveV(1); liveV(2);              // workitem-id VGPRs (threadIdx sources)
        bVtmp  = prs->allocateRegister(gen, registerSlot::VGPR, 1,  1); okV = !(bVtmp  == Null_Register);
        bSaddr = prs->allocateRegister(gen, registerSlot::SGPR, 1,  1); okS = !(bSaddr == Null_Register);
        bPwr   = prs->allocateRegister(gen, registerSlot::SGPR, 2,  2); okP = !(bPwr == Null_Register);
        bWork  = prs->allocateRegister(gen, registerSlot::SGPR, 11, 1); okW = !(bWork  == Null_Register);
        if (okV && okS && okP && okW) {
          allocated = true;
          vtmp = bVtmp - NS_amdgpu::VGPR_OPERAND_BASE; saddr = bSaddr; pwr = bPwr;
          for (uint32_t i = 0; i < 11; i++) w[i] = bWork + i;
        } else {                                    // partial failure -> release + fall back
          if (okV) prs->freeRegister(bVtmp, 1);
          if (okS) prs->freeRegister(bSaddr, 1);
          if (okP) prs->freeRegister(bPwr, 2);
          if (okW) prs->freeRegister(bWork, 11);
        }
      }
      if (!allocated) {
        // Computed fallback: a temp window just above the ABI live-in ceiling (kernarg +
        // relocated wgid), even-aligned base; vtmp above the kernel's own VGPRs.
        int liveInCeil = 0;
        auto bumpCeil = [&](int r) { if (r + 1 > liveInCeil) liveInCeil = r + 1; };
        if (L.kernargPtr >= 0) bumpCeil(L.kernargPtr + 1);
        const int relWg[4] = { L.wgidX, L.wgidY, L.wgidZ, L.wgInfo };
        for (int p : relWg) if (p >= 0) bumpCeil(p - (int)SHIFT);
        uint32_t base = ((uint32_t)liveInCeil + 1u) & ~1u;   // even
        saddr = base;
        pwr   = base + 2;                                    // even pair for S_LOAD_DWORDX2
        for (uint32_t i = 0; i < 11; i++) w[i] = base + 4 + i;
        vtmp = (kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() + 1) * kd.vgprAllocGranule();
      }

      // SADDR = originalPrivateSegment: our IACR/spill region is seated ABOVE the caller
      // kernel's own scratch frame [0,O) so it can't alias it. O==0 (no caller frame)
      // emits the exact same s_mov SADDR,0 as before (strict no-op for those kernels).
      if (originalPrivateSegment == 0)
        emitSop1Raw(S_MOV_B32, saddr, GFX908_INLINE_0, gen);          // SADDR = 0
      else
        emitSop2RawWithLiteral(S_OR_B32, saddr, GFX908_INLINE_0, originalPrivateSegment, gen); // SADDR = O

      // Packed workitem-id (3c: threadIdx). CRITICAL gfx908 fact: gfx908 (CDNA1) has NO
      // "Packed work-item IDs" target property, so the kernel receives the work-item id
      // in SEPARATE VGPRs at entry: v0=id_x, v1=id_y, v2=id_z (only the dims that
      // ENABLE_VGPR_WORKITEM_ID enables exist: 0=>x, 1=>x,y, 2=>x,y,z). A separately-
      // compiled device-fn callee, however, reads threadIdx.{x,y,z} from a PACKED v31
      // (x[9:0]/y[19:10]/z[29:20]). So BUILD the packed value per-lane (v0 | v1<<10 |
      // v2<<20 via V_LSHL_OR_B32 — the 10-bit fields don't overlap) and capture THAT into
      // the IACR. Do it FIRST, before anything here touches v0/v1/v2. Scratch is hardware-
      // swizzled per lane, so each lane stores its own packed id. Retrieved into v31 at
      // the call site. [On gfx942/MI300 v0 is already packed — read v0 directly there.]
      const uint32_t witid = kd.getCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId();
      if (packedWid) {
        // gfx942/MI300: v0 is ALREADY the packed work-item id (x[9:0]/y[19:10]/z[29:20]),
        // exactly the layout a device-fn callee reads from v31 — capture it verbatim.
        emitVop1Reg(V_MOV_B32, vtmp, /*v0=*/256u + 0u, gen);         // vtmp = packed id
      } else {
        emitVop1Reg(V_MOV_B32, vtmp, /*v0=*/256u + 0u, gen);         // vtmp = id_x
        if (witid >= 1)   // vtmp |= id_y << 10
          emitVop3a(V_LSHL_OR_B32, vtmp, /*src0=v1*/256u + 1u, /*src1=10*/128u + 10u,
                    /*src2=vtmp*/256u + vtmp, gen);
        if (witid >= 2)   // vtmp |= id_z << 20
          emitVop3a(V_LSHL_OR_B32, vtmp, /*src0=v2*/256u + 2u, /*src1=20*/128u + 20u,
                    /*src2=vtmp*/256u + vtmp, gen);
      }
      emitScratchStore(vtmp, IAL::OFF_WITEMID, saddr, gen);                // IACR[+12] = packed id

      // Capture blockIdx.{x,y,z}. Write EVERY slot: the real relocated wgid for a dim the
      // kernel enables, else 0. Zeroing disabled dims makes blockIdx.y/z a valid 0 on a
      // lower-rank launch, so a forwarded call's general global_wavefront_id() collapses
      // correctly (the emitter is the only place that can tell — a disabled wgid SGPR
      // would hold garbage).
      const int     wsrc[3] = { L.wgidX, L.wgidY, L.wgidZ };  // relocated to p-SHIFT above
      const int32_t woff[3] = { IAL::OFF_WGID_X, IAL::OFF_WGID_Y, IAL::OFF_WGID_Z };
      for (int i = 0; i < 3; i++) {
        if (wsrc[i] >= 0)
          emitVop1Reg(V_MOV_B32, vtmp, (uint32_t)wsrc[i] - SHIFT, gen);  // real wgid
        else
          emitVop1Reg(V_MOV_B32, vtmp, GFX908_INLINE_0, gen);            // disabled -> 0
        emitScratchStore(vtmp, woff[i], saddr, gen);                           // IACR[woff]
      }

      // Capture the scratch V# descriptor s[0:3] (Private Segment Buffer) — the ABI
      // input a buffer-scratch callee (buffer_load/store off s[0:3]) expects. Only when
      // the caller has its OWN scratch frame (originalPrivateSegment>0): then setupCalleeStack
      // reloads THIS captured descriptor at the call site instead of fabricating one from
      // FLAT_SCRATCH. A leaf caller (O==0) keeps the fabricated path (unchanged, no
      // regression). s[0:3] is still pristine here — the prologue's liveness allocator holds
      // [0,liveSgprEnd) (which covers privSegBuf) off-limits, so nothing above clobbered it.
      if (originalPrivateSegment > 0 && L.privSegBuf >= 0) {
        for (uint32_t d = 0; d < 4u; d++) {
          emitVop1Reg(V_MOV_B32, vtmp, (uint32_t)L.privSegBuf + d, gen);   // s(psb+d) -> vtmp
          emitScratchStore(vtmp, IAL::OFF_VDESC + 4 * (int32_t)d, saddr, gen);    // IACR[+VDESC+4d]
        }
      }

      // Kernarg-segment pointer (3b: blockDim/gridDim source). A device function reads
      // block/grid dims by dereferencing the implicit-args pointer (offsets 0/12/18 =
      // block_count/group_size/remainder). In COV5 the implicit args are a fixed
      // 256-byte block at the END of the kernarg segment: implicitarg_ptr =
      // kernarg_ptr + (kernarg_size - 256). kernarg_ptr precedes flat_scratch_init in
      // ABI order so it is NOT shifted by scratch-enable. We store the RAW kernarg
      // pointer here (no temp SGPRs — a scalar add in the prologue would need scratch
      // regs that collide with the relocated system SGPRs); the +offset is applied at
      // retrieve time in emitCall using the reserved s[8:9].
      if (L.kernargPtr >= 0 && originalKernargSize >= 256) {
        const uint32_t kp = (uint32_t)L.kernargPtr;                   // s[kp:kp+1] = kernarg ptr
        emitVop1Reg(V_MOV_B32, vtmp, kp, gen);
        emitScratchStore(vtmp, IAL::OFF_KERNARG,     saddr, gen);     // IACR[+24] = kernarg lo
        emitVop1Reg(V_MOV_B32, vtmp, kp + 1, gen);
        emitScratchStore(vtmp, IAL::OFF_KERNARG + 4, saddr, gen);     // IACR[+28] = kernarg hi
      }

      // Per-wave variable base. The launcher appends a device buffer pointer at the
      // END of the kernarg segment (repurposing the vestigial instrumentation ptr);
      // getKernargSize() is the ORIGINAL (pre-growth) size == that appended offset.
      // Dereference it here, add THIS wave's slice offset, and capture the result into
      // the IACR so an inserted call passes it as a 64-bit pointer arg (emitCall).
      //
      // Slice offset = wid * STRIDE, where wid is the LOGICAL flattened wave id
      // (blockIdx.x*blockDim.x + threadIdx.x)/64 — uniform per wave (a wavefront is 64
      // consecutive flattened thread ids). blockIdx.x = wgid_x entry SGPR (relocated to
      // p-SHIFT); blockDim.x = hidden group_size_x (u16 @ kernarg + (ksize-256) + 12,
      // COV5 implicit block); threadIdx.x = readfirstlane(v0) & 0x3ff (v0 is the packed
      // workitem id at entry, still intact here). STRIDE is the per-wave arena stride (the
      // EXACT 8-aligned arena size from the bump-allocator; 4096 if none), applied with a
      // scalar multiply (S_MULK_I32) — no power-of-two constraint, so no wasted slack.
      // Scratch SGPRs are the free upper half of the reserved block.
      //
      // DETECTION: pick the wid form from the kernel's own configuration. A kernel that
      // enables workgroup_id_y (2D/3D) or a higher workitem-id dim, launched WITH a COV5
      // implicit-args block, takes the GENERAL block-linear form; otherwise the cheap 1D
      // form (byte-identical to the validated 1D path). Both compute the SAME flattened
      // wave id as the probe's global_wavefront_id() and the host's nwaves.
      const bool hasY    = (L.wgidY >= 0);
      const bool hasZ    = (L.wgidZ >= 0);
      const bool general = (hasY || witid >= 1) && (originalKernargSize >= 256);

      if (L.kernargPtr >= 0 && originalKernargSize >= 8 && !general) {
        // ---- 1D wid : wid = (blockIdx.x*blockDim.x + threadIdx.x) >> 6 --------------
        const uint32_t kp   = (uint32_t)L.kernargPtr;                 // s[kp:kp+1] = kernarg ptr
        const uint32_t sBd  = w[0], sBi = w[1];                       // blockDim.x, blockIdx.x
        const uint32_t sTid = w[2], sWid = w[3];                      // threadIdx.x, wid/scratch
        emitSmem(S_LOAD_DWORDX2, /*sdata=*/pwr, /*sbase=*/kp >> 1,
                 /*offset=*/originalKernargSize, gen);              // base = *(kernarg + orig ksize)
        // blockDim.x (only if the COV5 implicit block is present; else single-wg => 0)
        if (originalKernargSize >= 256) {
          emitSmem(S_LOAD_DWORD, /*sdata=*/sBd, /*sbase=*/kp >> 1,
                   /*offset=*/(originalKernargSize - 256) + 12, gen);
          emitSopP(S_WAITCNT, /*simm16=*/0, gen);
          emitSop2RawWithLiteral(S_AND_B32, sBd, sBd, 0xFFFFu, gen);  // low 16 = blockDim.x
        } else {
          emitSopP(S_WAITCNT, /*simm16=*/0, gen);
          emitSop1Raw(S_MOV_B32, sBd, GFX908_INLINE_0, gen);    // 0
        }
        // blockIdx.x (uniform) from its entry SGPR, else 0 (single workgroup)
        if (L.wgidX >= 0) emitSop1Raw(S_MOV_B32, sBi, (uint32_t)L.wgidX - SHIFT, gen);
        else              emitSop1Raw(S_MOV_B32, sBi, GFX908_INLINE_0, gen);
        // threadIdx.x = readfirstlane(v0) & 0x3ff  (uniform: first active lane's id)
        emitVop1Reg(V_READFIRSTLANE_B32, sTid, /*v0=*/256u + 0u, gen);
        emitSop2RawWithLiteral(S_AND_B32, sTid, sTid, 0x3FFu, gen);
        // wid = (blockIdx*blockDim + threadIdx) >> 6 ; off = wid*STRIDE
        emitSop2Raw(S_MUL_I32, sWid, sBi, sBd, gen);
        emitSop2Raw(S_ADD_U32, sWid, sWid, sTid, gen);
        emitSop2RawWithLiteral(S_LSHR_B32, sWid, sWid, 6u, gen);      // /64 -> logical wave id
        emitSopK(S_MULK_I32, sWid, (int16_t)g_amdgpuPerWaveStride, gen); // wid *= STRIDE (scalar mul)
        emitSop2Raw(S_ADD_U32,  pwr,     pwr,     sWid,            gen);  // base_lo += off
        emitSop2Raw(S_ADDC_U32, pwr + 1, pwr + 1, GFX908_INLINE_0, gen); // carry
        // capture this wave's slice base into the IACR (uniform: broadcast then store)
        emitVop1Reg(V_MOV_B32, vtmp, pwr,     gen);
        emitScratchStore(vtmp, IAL::OFF_PWBASE,     saddr, gen);      // IACR[+32] = slice lo
        emitVop1Reg(V_MOV_B32, vtmp, pwr + 1, gen);
        emitScratchStore(vtmp, IAL::OFF_PWBASE + 4, saddr, gen);      // IACR[+36] = slice hi
      } else if (L.kernargPtr >= 0 && general) {
        // ---- GENERAL (2D/3D) block-linear wid --------------------------------------
        //   wid          = block_linear * wpb + (local_tid >> 6)
        //   block_linear = (bz*GDy + by)*GDx + bx        local_tid = (tz*BDy + ty)*BDx + tx
        //   wpb          = ceil(BDx*BDy*BDz / 64)         (waves per block)
        // Matches the probe's global_wavefront_id() and the host's
        // nwaves = numBlocks * ceil(blockDim/64). z/y terms collapse when the kernel is
        // 2D/1D (blockIdx.{y,z}=0, threadIdx.{y,z}=0, no BDz factor).
        // blockIdx.{x,y,z} = the relocated wgid SGPRs; blockDim = group_size (u16 @
        // io+12/14/16); gridDim = block_count (u32 @ io+0/4); threadIdx = SEPARATE v0/v1/v2
        // first-lane (gfx908, NOT packed). io = COV5 implicit-args offset = originalKernarg
        // Size-256.
        //
        // TEMP SGPRs come from the liveness allocation above: pwr (even pair, kept to the
        // final PWBASE store) + w[0..10] working regs. All are dead-at-entry (the kernel
        // body recomputes them) and inside the wave's grant.
        const uint32_t kp  = (uint32_t)L.kernargPtr;
        const uint32_t io  = originalKernargSize - 256;            // COV5 implicit-args offset
        const uint32_t sBDx = w[0], sBDy = w[1], sBDz = w[2];      // blockDim x/y/z
        const uint32_t sGDx = w[3], sGDy = w[4];                   // gridDim x/y (block_count)
        const uint32_t sTx  = w[5], sTy  = w[6], sTz  = w[7];      // threadIdx x/y/z (1st lane)
        const uint32_t sWpb = w[8], sWib = w[9], sAcc = w[10];     // waves/blk, wave-in-blk, acc
        emitSmem(S_LOAD_DWORDX2, pwr,  kp >> 1, originalKernargSize, gen);  // slice base ptr
        emitSmem(S_LOAD_DWORD,   sBDx, kp >> 1, io + 12, gen);      // group_size x|y (u16 pair)
        if (hasZ) emitSmem(S_LOAD_DWORD, sBDz, kp >> 1, io + 16, gen);  // group_size z (low16)
        emitSmem(S_LOAD_DWORD,   sGDx, kp >> 1, io + 0, gen);       // block_count x
        emitSmem(S_LOAD_DWORD,   sGDy, kp >> 1, io + 4, gen);       // block_count y
        // threadIdx first active lane. PACKED (gfx942/CDNA3): the work-item id is PACKED in
        // a single v0 (x[9:0]/y[19:10]/z[29:20]) — read v0 once, then extract each 10-bit
        // field by shift+mask. gfx908: SEPARATE v0/v1/v2. Absent dim -> 0.
        if (packedWid) {
          emitVop1Reg(V_READFIRSTLANE_B32, sTx, /*v0=*/256u + 0u, gen);  // packed id (1st lane)
          emitSop2RawWithLiteral(S_LSHR_B32, sTy, sTx, 10u, gen);               // ty' = packed >> 10
          if (witid >= 2) emitSop2RawWithLiteral(S_LSHR_B32, sTz, sTx, 20u, gen); // tz' = packed >> 20
          else            emitSop1Raw(S_MOV_B32, sTz, GFX908_INLINE_0, gen);
          emitSop2RawWithLiteral(S_AND_B32, sTx, sTx, 0x3FFu, gen);             // tx = packed & 0x3ff
          if (witid >= 1) emitSop2RawWithLiteral(S_AND_B32, sTy, sTy, 0x3FFu, gen); // ty = (packed>>10)&0x3ff
          else            emitSop1Raw(S_MOV_B32, sTy, GFX908_INLINE_0, gen);
          if (witid >= 2) emitSop2RawWithLiteral(S_AND_B32, sTz, sTz, 0x3FFu, gen); // tz = (packed>>20)&0x3ff
        } else {
          emitVop1Reg(V_READFIRSTLANE_B32, sTx, /*v0=*/256u + 0u, gen);
          if (witid >= 1) emitVop1Reg(V_READFIRSTLANE_B32, sTy, /*v1=*/256u + 1u, gen);
          else            emitSop1Raw(S_MOV_B32, sTy, GFX908_INLINE_0, gen);
          if (witid >= 2) emitVop1Reg(V_READFIRSTLANE_B32, sTz, /*v2=*/256u + 2u, gen);
          else            emitSop1Raw(S_MOV_B32, sTz, GFX908_INLINE_0, gen);
        }
        emitSopP(S_WAITCNT, /*simm16=*/0, gen);                     // SMEM dims ready
        emitSop2RawWithLiteral(S_LSHR_B32, sBDy, sBDx, 16u, gen);      // BDy = high16
        emitSop2RawWithLiteral(S_AND_B32,  sBDx, sBDx, 0xFFFFu, gen);  // BDx = low16
        if (hasZ) emitSop2RawWithLiteral(S_AND_B32, sBDz, sBDz, 0xFFFFu, gen);  // BDz = low16
        // wpb = ceil(BDx*BDy*BDz / 64)   (z factor only when the kernel is 3D)
        emitSop2Raw(S_MUL_I32, sWpb, sBDx, sBDy, gen);
        if (hasZ) emitSop2Raw(S_MUL_I32, sWpb, sWpb, sBDz, gen);
        emitSop2RawWithLiteral(S_ADD_U32,  sWpb, sWpb, 63u, gen);
        emitSop2RawWithLiteral(S_LSHR_B32, sWpb, sWpb, 6u, gen);
        // local_tid = (tz*BDy + ty)*BDx + tx ; wave-in-block = local_tid >> 6
        emitSop2Raw(S_MUL_I32, sAcc, sTz,  sBDy, gen);   // tz*BDy  (0 if no z)
        emitSop2Raw(S_ADD_U32, sAcc, sAcc, sTy,  gen);   // + ty    (0 if no y)
        emitSop2Raw(S_MUL_I32, sAcc, sAcc, sBDx, gen);   // * BDx
        emitSop2Raw(S_ADD_U32, sAcc, sAcc, sTx,  gen);   // + tx  (= local_tid)
        emitSop2RawWithLiteral(S_LSHR_B32, sWib, sAcc, 6u, gen);       // wave-in-block
        // block_linear = (bz*GDy + by)*GDx + bx  (bx/by/bz = relocated wgid; absent -> 0)
        if (hasZ) {
          emitSop1Raw(S_MOV_B32, sAcc, (uint32_t)L.wgidZ - SHIFT, gen);  // bz
          emitSop2Raw(S_MUL_I32, sAcc, sAcc, sGDy, gen);                       // bz*GDy
        } else {
          emitSop1Raw(S_MOV_B32, sAcc, GFX908_INLINE_0, gen);           // 0
        }
        if (hasY) emitSop2Raw(S_ADD_U32, sAcc, sAcc, (uint32_t)L.wgidY - SHIFT, gen);  // + by
        emitSop2Raw(S_MUL_I32, sAcc, sAcc, sGDx, gen);                                 // * GDx
        if (L.wgidX >= 0) emitSop2Raw(S_ADD_U32, sAcc, sAcc, (uint32_t)L.wgidX - SHIFT, gen);  // + bx
        // wid = block_linear*wpb + wave-in-block ; slice offset = wid*STRIDE ; base += off
        emitSop2Raw(S_MUL_I32, sAcc, sAcc, sWpb, gen);
        emitSop2Raw(S_ADD_U32, sAcc, sAcc, sWib, gen);
        emitSopK(S_MULK_I32, sAcc, (int16_t)g_amdgpuPerWaveStride, gen);
        emitSop2Raw(S_ADD_U32,  pwr,     pwr,     sAcc,            gen);  // base_lo += off
        emitSop2Raw(S_ADDC_U32, pwr + 1, pwr + 1, GFX908_INLINE_0, gen); // carry
        // capture this wave's slice base into the IACR (uniform: broadcast then store)
        emitVop1Reg(V_MOV_B32, vtmp, pwr,     gen);
        emitScratchStore(vtmp, IAL::OFF_PWBASE,     saddr, gen);      // IACR[+32] = slice lo
        emitVop1Reg(V_MOV_B32, vtmp, pwr + 1, gen);
        emitScratchStore(vtmp, IAL::OFF_PWBASE + 4, saddr, gen);      // IACR[+36] = slice hi
      }
      emitSopP(S_WAITCNT, /*simm16=*/0, gen);

      // Release the liveness-allocated prologue temps (pillar A). No-op in the fallback.
      if (allocated) {
        prs->freeRegister(bVtmp, 1);
        prs->freeRegister(bSaddr, 1);
        prs->freeRegister(bPwr, 2);
        prs->freeRegister(bWork, 11);
      }
    }
  }

  void emitScratchStore(uint32_t vreg, int32_t offset, uint32_t saddr, codeGen &gen) override {
    // SADDR = a reserved SGPR the trampoline sets to 0; the per-wave 64-bit base
    // comes implicitly from FLAT_SCRATCH; per-lane swizzle is automatic. addr(VADDR)
    // is ignored in SADDR mode (SADDR != 0x7f). glc for L2-coherence across the call.
    emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0, /*addr=*/0, /*data=*/vreg,
                   saddr, offset, gen, /*glc=*/true, /*seg=*/1);
  }
  void emitScratchLoad(uint32_t vreg, int32_t offset, uint32_t saddr, codeGen &gen) override {
    emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/vreg, /*addr=*/0, /*data=*/0,
                   saddr, offset, gen, /*glc=*/true, /*seg=*/1);
  }

  void setupCalleeStack(uint32_t s32Base, bool reconstructDescriptor,
                        bool architected, codeGen &gen) override {
    // ARCHITECTED (gfx942/CDNA3): a hipcc-compiled callee addresses its OWN frame via
    // `scratch_store/load off, vN, s32 offset:M` — s32 is the stack pointer (a per-lane
    // byte offset into architected flat scratch), and there is NO buffer V# descriptor
    // (no s[0:3]) — HW provides the per-wave/per-lane scratch aperture. So the ONLY caller
    // set-up is s32. (Verified: gfx942 hipcc emits exactly this for a callee with a frame.)
    if (architected) {
      emitSop2RawWithLiteral(S_OR_B32, /*s32=*/32u, GFX908_INLINE_0, s32Base, gen);
      return;
    }
    // gfx908 (buffer-scratch): the scratch V# descriptor s[0:3] the non-leaf callee needs.
    //  * reconstructDescriptor=true (LEAF caller, no real descriptor of its own):
    //    fabricate one from FLAT_SCRATCH + the constant fields measured on gfx908/
    //    ROCm7.0.2. FLAT_SCRATCH already includes wave_offset so s0 is per-wave-correct.
    //  * reconstructDescriptor=false (caller that USES buffer scratch): emitCall has
    //    already reloaded the caller's REAL descriptor (captured in IACR[OFF_VDESC] at
    //    entry) into s[0:3] — do NOT overwrite it with a fabricated one (wrong
    //    num_records, and it would clobber the caller's live descriptor).
    if (reconstructDescriptor) {
      emitSop1Raw(S_MOV_B32, /*s0=*/0, GFX908_FLAT_SCRATCH_LO, gen);              // s0 = base_lo = FLAT_lo
      emitSop2RawWithLiteral(S_OR_B32, /*s1=*/1, GFX908_FLAT_SCRATCH_HI, 0x80000000u, gen); // s1 = base_hi | swizzle_enable
      emitSop2RawWithLiteral(S_OR_B32, /*s2=*/2, GFX908_INLINE_0, 0x00400000u, gen);        // s2 = num_records
      emitSop2RawWithLiteral(S_OR_B32, /*s3=*/3, GFX908_INLINE_0, 0x00ea4facu, gen);        // s3 = format/swizzle flags
    }
    // s32 = stack pointer base, placed ABOVE the caller frame + our flat-scratch spill
    // region so the callee's frame (buffer soffset >= that) can't collide.
    emitSop2RawWithLiteral(S_OR_B32, /*s32=*/32u, GFX908_INLINE_0, s32Base, gen);
  }
};

}  // anonymous namespace

namespace Dyninst { namespace DyninstAPI {
ScratchAbi &gfx908ScratchAbi() { static Gfx908ScratchAbi abi; return abi; }
}}

// Publish the per-wave arena stride the scratch prologue reads (set by allocatePerWave).
void AddressSpace::publishPerWaveStrideToEmitter(unsigned bytes) {
  g_amdgpuPerWaveStride = bytes ? bytes : 1u;
}
