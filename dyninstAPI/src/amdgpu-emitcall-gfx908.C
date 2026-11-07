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

// gfx908 inter-module call emitter: EmitterAmdgpuGfx908::emitCall (+ emitIndirectCall and
// clobberAllFuncCall) and the KernelMeta/symbol helpers it uses. OPEN-CODED: every instruction routes
// through the runtime-selected per-gen EmitterAmdgpu (getAmdgpuEmitter(gen) + raw*/toOpCode) via the
// thin emit* shims below; register spill goes through the gfx908 ScratchAbi. Each opcode-taking shim
// has two overloads: an AmdgpuOp form (gen-neutral, resolved via toOpCode) for named opcodes, and a
// raw-unsigned form (gfx908 opcode value, direct) for the handful of local raw-value opcodes the call
// path uses (e.g. the 64-bit exec moves). The validated body is otherwise preserved.

#include "codegen/emitters/amdgpu/gfx908/emit-amdgpu-gfx908.h"
#include "codegen/emitters/amdgpu/gfx940/emit-amdgpu-gfx940.h"
#include "codegen/emitters/amdgpu/emit-amdgpu.h"
#include "codegen/codegen.h"
#include "registerSpace/registerSpace.h"
#include "dyn_register.h"
#include "function_cache.h"
#include "arch-amdgpu.h"
#include "patching/instPoint.h"
#include "patching/function.h"
#include "mapped_object.h"
#include "addressSpace.h"
#include "Symtab.h"
#include "Symbol.h"
#include "AmdgpuKernelDescriptor.h"
#include "amdgpu-kernel-meta.h"
#include "amdgpu-scratch-abi.h"
#include "amdgpu-abi-sgpr.h"
#include "amdgpu-implicit-args.h"
#include "amdgpu-register-context.h"
#include "external/amdgpu/AMDGPUEFlags.h"
#include "ASTs/operandAST.h"
#include "ASTs/OperandType.h"
#include "ASTs/codeGenAST.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>

using namespace Dyninst;

namespace {

// Gen-neutral opcode identities for the named opcodes the call body passes to the emit* shims.
constexpr AmdgpuOp S_ADD_U32 = AmdgpuOp::S_ADD_U32;
constexpr AmdgpuOp S_ADDC_U32 = AmdgpuOp::S_ADDC_U32;
constexpr AmdgpuOp S_GETPC_B64 = AmdgpuOp::S_GETPC_B64;
constexpr AmdgpuOp S_GETREG_B32 = AmdgpuOp::S_GETREG_B32;
constexpr AmdgpuOp S_OR_B32 = AmdgpuOp::S_OR_B32;
constexpr AmdgpuOp S_SWAPPC_B64 = AmdgpuOp::S_SWAPPC_B64;
constexpr AmdgpuOp S_WAITCNT = AmdgpuOp::S_WAITCNT;
constexpr AmdgpuOp V_ADD_CO_U32 = AmdgpuOp::V_ADD_CO_U32;
constexpr AmdgpuOp V_ADDC_CO_U32 = AmdgpuOp::V_ADDC_CO_U32;
constexpr AmdgpuOp V_READLANE_B32 = AmdgpuOp::V_READLANE_B32;
constexpr AmdgpuOp V_WRITELANE_B32 = AmdgpuOp::V_WRITELANE_B32;
constexpr AmdgpuOp GLOBAL_LOAD_DWORD = AmdgpuOp::GLOBAL_LOAD_DWORD;
constexpr AmdgpuOp GLOBAL_STORE_DWORD = AmdgpuOp::GLOBAL_STORE_DWORD;

// Thin shims: forward the fork's call shape to the runtime-selected emitter's packers.
inline void emitSopP(AmdgpuOp op, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSopP(E->toOpCode(op), simm16, g);
}
inline void emitSopP(unsigned opcode, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSopP(opcode, simm16, g);
}
inline void emitSop1(AmdgpuOp op, Dyninst::Register dest, Dyninst::Register src0, bool hasLiteral, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop1(E->toOpCode(op), dest, src0, hasLiteral, literal, g);
}
inline void emitSop1(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0, bool hasLiteral, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop1(opcode, dest, src0, hasLiteral, literal, g);
}
inline void emitSop1Raw(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop1(E->toOpCode(op), sdst, ssrc0, false, 0, g);
}
inline void emitSop1Raw(unsigned opcode, uint32_t sdst, uint32_t ssrc0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop1(opcode, sdst, ssrc0, false, 0, g);
}
inline void emitSop2(AmdgpuOp op, Dyninst::Register dest, Dyninst::Register src0, Dyninst::Register src1, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2(E->toOpCode(op), dest, src0, src1, g);
}
inline void emitSop2(unsigned opcode, Dyninst::Register dest, Dyninst::Register src0, Dyninst::Register src1, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2(opcode, dest, src0, src1, g);
}
inline void emitSop2Raw(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, uint32_t ssrc1, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2(E->toOpCode(op), sdst, ssrc0, ssrc1, g);
}
inline void emitSop2Raw(unsigned opcode, uint32_t sdst, uint32_t ssrc0, uint32_t ssrc1, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2(opcode, sdst, ssrc0, ssrc1, g);
}
inline void emitSop2RawWithLiteral(AmdgpuOp op, uint32_t sdst, uint32_t ssrc0, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2WithSrc1Literal(E->toOpCode(op), sdst, ssrc0, literal, g);
}
inline void emitSop2RawWithLiteral(unsigned opcode, uint32_t sdst, uint32_t ssrc0, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSop2WithSrc1Literal(opcode, sdst, ssrc0, literal, g);
}
inline void emitSopK(AmdgpuOp op, Dyninst::Register dest, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSopK(E->toOpCode(op), dest, simm16, g);
}
inline void emitSopK(unsigned opcode, Dyninst::Register dest, int16_t simm16, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawSopK(opcode, dest, simm16, g);
}
inline void emitVop1Reg(AmdgpuOp op, uint32_t vdst, uint32_t src0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop1Reg(E->toOpCode(op), vdst, src0, g);
}
inline void emitVop1Reg(unsigned opcode, uint32_t vdst, uint32_t src0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop1Reg(opcode, vdst, src0, g);
}
inline void emitVop1Imm(AmdgpuOp op, uint32_t vdst, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop1Imm(E->toOpCode(op), vdst, literal, g);
}
inline void emitVop1Imm(unsigned opcode, uint32_t vdst, uint32_t literal, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop1Imm(opcode, vdst, literal, g);
}
inline void emitVop2(AmdgpuOp op, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop2(E->toOpCode(op), vdst, vsrc1, src0, g);
}
inline void emitVop2(unsigned opcode, uint32_t vdst, uint32_t vsrc1, uint32_t src0, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop2(opcode, vdst, vsrc1, src0, g);
}
inline void emitVop3a(AmdgpuOp op, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop3a(E->toOpCode(op), vdst, src0, src1, src2, g);
}
inline void emitVop3a(unsigned opcode, uint32_t vdst, uint32_t src0, uint32_t src1, uint32_t src2, codeGen &g) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawVop3a(opcode, vdst, src0, src1, src2, g);
}
inline void emitFlatGlobal(AmdgpuOp op, uint32_t vdst, uint32_t addr, uint32_t data, uint32_t saddr, int32_t offset, codeGen &g, bool glc, uint32_t seg) {
  EmitterAmdgpu *E = getAmdgpuEmitter(g); E->rawFlatGlobal(E->toOpCode(op), vdst, addr, data, saddr, offset, g, glc, seg);
}

}  // anonymous namespace

static uint32_t readCalleeMaxAbsSym(func_instance *callee, const char *key);

// Pillar B: the caller kernel's canonical KD metadata (parsed once, cached on the
// mapped_object, committed at ELF-emit). All KD reads/mutations below go through it —
// no site re-parses the ELF or eagerly writes the KD. Returns nullptr for a non-kernel
// caller (ordinary device function with no ".kd").
static Dyninst::DyninstAPI::KernelMeta *callerMeta(func_instance *caller) {
  if (!caller || !caller->obj())
    return nullptr;
  return caller->obj()->getAmdgpuKernelMeta(caller->symTabName());
}

// Scalars packed per pack-VGPR in the SGPR spill = wavefront width (wave64 = 64
// lanes; one v_writelane per lane). Overridable (clamped to <=64) purely to force
// numPacks>1 for testing the multi-pack path with a small callee; production = 64.
static uint32_t spillPackLanes() {
  const char *e = getenv("DYNINST_PACK_LANES");
  if (!e) return 64;
  uint32_t v = (uint32_t)atoi(e);
  return (v == 0 || v > 64) ? 64 : v;
}
// The caller kernel's ORIGINAL (pristine, pre-bump) granted VGPR count, captured once
// at parse in the KernelMeta. Using the pristine value (not the live, growing grant)
// keeps the per-call spill's scratch VGPRs at the same fixed registers across the
// multiple inserted calls that each raise the grant.
static uint32_t readCallerOriginalGrantedVgpr(func_instance *caller, codeGen & /*gen*/) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  return km ? km->originalGrantedVgpr : 0;
}

// The caller kernel's OWN scratch frame size [0,O). Our IACR/spill (SADDR=O) and the
// inserted callee frame (s32Base += O) sit above it so they don't alias it. 0 for a
// kernel with no frame of its own.
static uint32_t readCallerOriginalPrivate(func_instance *caller) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  return km ? km->originalPrivateSegment : 0;
}

// True when the caller kernel targets an architected/absolute-flat-scratch arch (gfx942/
// CDNA3). This changes the NON-LEAF call-ABI: gfx908 addresses the callee frame as
// buffer_* off a fabricated scratch V# descriptor s[0:3] + a PER-WAVEFRONT stack pointer
// s32 (soffset, swizzled /64); gfx942 addresses it as scratch_* off s32 alone (NO
// descriptor), with s32 a PER-LANE byte offset — exactly like our register spills. So on
// gfx942 setupCalleeStack sets only s32, the frame base is per-lane (no *64), and the KD
// must NOT enable flat_scratch_init. (Reverse-engineered from gfx942 hipcc: a callee with
// its own frame emits `scratch_store off, vN, s32 offset:M`.)
static bool callerArchitectedScratch(func_instance *caller) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  return km && km->kd.supportsArchitectedFlatScratch();
}

// When we splice an inter-module call into a kernel, the callee (a separately
// compiled device function) may use more registers/scratch than the caller
// kernel's descriptor granted for its own code. The wave's register allocation
// is fixed by the caller's kernel descriptor, so the callee must fit inside it.
//
// SGPR is already handled: AmdgpuGfx908PointHandler::maximizeSgprAllocationIfKernel
// maxes the SGPR grant for every instrumented kernel. Here we bump the VGPR grant
// PRECISELY (to preserve occupancy) using the callee's exported register-usage
// symbols, and reserve scratch only if the callee actually uses it.
//
// The callee's footprint is read from SHN_ABS symbols ".dyninst.<callee>.num_vgpr"
// / ".dyninst.<callee>.private_seg_size" whose VALUE is the compiler's
// post-register-allocation count (injected by add_object_aliases.py --asm).
static void bumpCallerKdForCallee(func_instance *caller, func_instance *callee,
                                  codeGen &gen) {
  if (!caller || !callee)
    return;

  AddressSpace *aSpace = gen.addrSpace();
  mapped_object *callerObj = caller->obj();
  mapped_object *calleeObj = callee->obj();
  if (!aSpace || !callerObj || !calleeObj)
    return;

  // Only kernels have a descriptor "<mangled>.kd". If it's absent, the caller is
  // an ordinary device function and there is no descriptor to size. Route through the
  // canonical KernelMeta (mutate in memory; committed at ELF-emit) — no re-parse / no
  // eager write-back.
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  if (!km)
    return;

  SymtabAPI::Symtab *calleeSymtab = calleeObj->parse_img()->getObject();
  if (!calleeSymtab)
    return;

  const std::string prefix = ".dyninst." + callee->symTabName() + ".";
  auto readAbs = [&](const std::string &key, uint32_t &out) -> bool {
    std::vector<SymtabAPI::Symbol *> syms;
    if (!calleeSymtab->findSymbol(syms, prefix + key) || syms.empty())
      return false;
    out = static_cast<uint32_t>(syms[0]->getOffset());
    return true;
  };

  uint32_t calleeVgpr = 0;
  if (!readAbs("num_vgpr", calleeVgpr)) {
    fprintf(stderr,
            "[amdgpu] warning: no '.dyninst.%s.num_vgpr' symbol for callee; cannot "
            "size caller '%s' kernel-descriptor VGPR grant — leaving as-is\n",
            callee->symTabName().c_str(), caller->symTabName().c_str());
    return;
  }
  // Grant must cover the callee's TRANSITIVE footprint (wrapper + everything it
  // calls), so use the whole-object max — same set the VGPR save/restore in
  // emitCall preserves.
  { uint32_t m = readCalleeMaxAbsSym(callee, "num_vgpr"); if (m > calleeVgpr) calleeVgpr = m; }
  uint32_t calleeScratch = 0;
  readAbs("private_seg_size", calleeScratch);  // optional; absent => 0
  { uint32_t m = readCalleeMaxAbsSym(callee, "private_seg_size"); if (m > calleeScratch) calleeScratch = m; }

  // The caller's kernel descriptor — the canonical in-memory copy.
  Dyninst::AmdgpuKernelDescriptor &kd = km->kd;

  // 0.4: the SGPR spill packs {calleeSgpr numbered SGPRs, VCC_lo, VCC_hi} one-per-
  // lane across ceil((calleeSgpr+2)/64) pack VGPRs (must match numPacks in emitCall).
  // The spill ALWAYS runs (VCC alone needs a pack even if calleeSgpr==0).
  const uint32_t calleeSgpr = readCalleeMaxAbsSym(callee, "numbered_sgpr");
  const uint32_t packLanes  = spillPackLanes();   // must match emitCall
  const uint32_t numPacks   = (calleeSgpr + 2u + packLanes - 1u) / packLanes;

  // VGPR grant: granted = (granulated+1)*granule, granule=4 (gfx908) or 8 (gfx942/CDNA3); only raise.
  // PRESERVE-OCCUPANCY (pillar A): size the grant to the CALLEE FOOTPRINT ONLY —
  // max(caller_orig_vgpr, callee_nvgpr) — with NO extra growth for the trampoline temps.
  // The callee's compiled code clobbers v0..nvgpr-1 in the shared register file, so
  // grant >= nvgpr is a legitimate, required occupancy cost of calling a bigger function
  // (not what we're removing). The trampoline's vPack/vImplTmp temps do NOT need room
  // above that: emitCall ALLOCATES them from dead registers within this grant via the
  // point-specialized register space (their data lives in scratch across the call). ONLY
  // emitCall's small-callee last resort — when the allocator can't find room and the
  // footprint is too tight to host the temps below nvgpr — needs growth; this test
  // (vgprTempsFit) matches that fallback so the grant covers it when it triggers. In the
  // normal case both agree on max(orig, nvgpr) with no temp growth.
  {
    // Use the kernel's ORIGINAL granted VGPR count (cached), NOT the live grant read
    // from kdBytes — otherwise each inserted call reads the previously-raised grant and
    // ratchets the count upward.
    const uint32_t curVgpr = readCallerOriginalGrantedVgpr(caller, gen);
    const uint32_t vBase    = (curVgpr > calleeVgpr ? curVgpr : calleeVgpr);  // = max(orig, nvgpr)
    const bool vgprTempsFit = (calleeVgpr >= curVgpr + numPacks + 2u);
    // gfx942 (architected): emitCall places the spill temps ABOVE the callee clobber footprint
    // (at vgprGrant = vBase), because the empty point-liveness + under-reported grant make the
    // in-footprint allocation unsafe. So the grant MUST cover vBase + numPacks + 2 unconditionally.
    const uint32_t needVgpr = kd.supportsArchitectedFlatScratch()
                                  ? vBase + numPacks + 2u
                                  : (vgprTempsFit ? vBase              // temps fit in [orig,nvgpr): no growth
                                                  : vBase + numPacks + 2u);  // small-callee fallback: grow
    const uint32_t gran = kd.vgprAllocGranule();  // 4 (gfx908) or 8 (gfx942/CDNA3)
    const uint32_t neededGran = (needVgpr + gran - 1u) / gran - 1u;
    if (neededGran > kd.getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount())
      kd.setCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount(neededGran);

    // The caller kernel's OWN scratch frame occupies per-lane [0, origPriv). Our IACR +
    // spill region and (below) the callee frame are stacked ABOVE it: region + original +
    // callee. origPriv==0 (no caller frame) reduces to the old formula exactly.
    const uint32_t origPriv = km->originalPrivateSegment;
    const uint32_t implBase = Dyninst::DyninstAPI::ImplicitArgLayout::BYTES;
    // 0.3: size the scratch spill slot to the footprint = numPacks SGPR-pack dwords +
    // calleeVgpr VGPR dwords = 4*(numPacks+nvgpr) bytes/lane, PLUS the IACR reserved
    // below it, PLUS the caller's own frame it sits above. The hardcoded 256 in
    // enableScratchInKD overflows for large footprints -> scratch aperture violation.
    const uint32_t spillSlot = origPriv + implBase + 4u * (numPacks + calleeVgpr);
    if (spillSlot > kd.getPrivateSegmentFixedSize())
      kd.setPrivateSegmentFixedSize(spillSlot);
  }

  // Scratch: reserve only if the callee actually uses private/scratch memory.
  if (calleeScratch > 0) {   // non-leaf callee (auto-detected) -> size for its buffer frame
    // Frames STACK: [0,origPriv) caller's own frame, [origPriv,origPriv+spillR) our flat
    // IACR+spill (SADDR=origPriv), then the callee's buffer frame at s32 base (PER-WAVE) =
    // (origPriv+spillR)*64 aligned, so its per-lane base (s32/64) clears both. total
    // per-lane = (s32Base/64) + callee frame + margin. Must match the s32Base arithmetic
    // in emitCall.
    const uint32_t origPriv = km->originalPrivateSegment;
    const uint32_t iacr    = Dyninst::DyninstAPI::ImplicitArgLayout::BYTES;
    const uint32_t spillR  = iacr + 4u * (numPacks + calleeVgpr);   // our region (per-lane)
    // Size the per-lane private segment for the callee frame seated above our region. Units
    // must match emitCall's s32Base: gfx942 (architected) uses a PER-LANE s32 (no *64);
    // gfx908 uses a PER-WAVEFRONT s32 (soffset, swizzled /64).
    if (kd.supportsArchitectedFlatScratch()) {
      const uint32_t s32Base = ((origPriv + spillR) + 0xFu) & ~0xFu;      // per-lane, 16B-aligned
      const uint32_t need    = s32Base + calleeScratch + 256u;           // per-lane
      if (need > kd.getPrivateSegmentFixedSize())
        kd.setPrivateSegmentFixedSize(need);
      // Architected flat scratch: NO flat_scratch_init user SGPR. But the kernel must declare
      // it uses private/scratch (RSRC2.ENABLE_PRIVATE_SEGMENT) or ROCr won't back the scratch
      // aperture -> FLAT_SCRATCH=0 -> faults. Idempotent with enableScratchInKD.
      if (!kd.getCOMPUTE_PGM_RSRC2_EnablePrivateSegment())
        kd.setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(true);
    } else {
      // gfx908 buffer-scratch: s32 is a PER-WAVEFRONT soffset (swizzled /wave), so seat above our
      // per-lane region at spillRfull * waveLanes. waveLanes = 64 (wave64) — from the KD, so a
      // future wave32 non-architected arch (RDNA) would use 32 instead of a hardcoded 64.
      const uint32_t waveLanes = kd.isWave64() ? 64u : 32u;
      const uint32_t s32Base = (((origPriv + spillR) * waveLanes) + 0x3FFu) & ~0x3FFu;  // per-wave
      const uint32_t need    = (s32Base / waveLanes) + calleeScratch + 256u; // per-lane
      if (need > kd.getPrivateSegmentFixedSize())
        kd.setPrivateSegmentFixedSize(need);
      kd.setKernelCodeProperty_EnableSgprFlatScratchInit(true);
    }
  }

  // KD mutated in memory only; committed once at ELF-emit (BinaryEdit::writeFile).
  km->dirty = true;
}

// Largest ".dyninst.*.<key>" value across ALL functions in the callee's object.
//
// A dyninst-inserted call transfers into the callee wrapper, which may itself
// make ordinary (non-inlined) calls into other functions in the SAME library
// (e.g. hc_write -> gpu_fwrite). Each nested call clobbers its own register
// frame, so the wrapper's *own* ".dyninst.<callee>.num_vgpr" undercounts the
// registers actually destroyed by the whole call chain — leaving live caller
// registers above the wrapper's frame unsaved (observed: the kernel's v[4:5]
// address pair trashed by gpu_fwrite, faulting on the next load). The per-call
// save/restore set is independent per call site, so accumulation across other
// sites can't fix it — this one site must save everything it transitively
// clobbers. We don't do call-graph analysis; conservatively take the maximum
// footprint over EVERY function in the callee's address space, which upper-bounds
// any function the inserted call could transitively reach. Returns 0 if none.
static uint32_t readCalleeMaxAbsSym(func_instance *callee, const char *key) {
  if (!callee || !callee->obj() || !callee->obj()->parse_img())
    return 0;
  SymtabAPI::Symtab *st = callee->obj()->parse_img()->getObject();
  if (!st)
    return 0;
  std::vector<SymtabAPI::Symbol *> all;
  if (!st->getAllSymbols(all))
    return 0;
  const std::string prefix = ".dyninst.";
  const std::string suffix = std::string(".") + key;
  uint32_t maxVal = 0;
  for (SymtabAPI::Symbol *s : all) {
    if (!s) continue;
    const std::string &nm = s->getMangledName();
    if (nm.size() > prefix.size() + suffix.size() &&
        nm.compare(0, prefix.size(), prefix) == 0 &&
        nm.compare(nm.size() - suffix.size(), suffix.size(), suffix) == 0) {
      uint32_t v = static_cast<uint32_t>(s->getOffset());
      if (v > maxVal) maxVal = v;
    }
  }
  return maxVal;
}

// Number of numbered SGPRs granted to the caller kernel (= one past the highest
// allocatable SGPR id). In scratch mode the reserved spill registers live at the TOP
// of this grant, so emitCall and generateBranch both derive the same reservedBase
// from it (maximizeSgprAllocationIfKernel sizes the grant for scratch). 0 if not a
// kernel.
static uint32_t readCallerGrantedSgprTop(func_instance *caller, codeGen & /*gen*/) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  return km ? km->grantedSgprTop() : 0;
}

// Whether the caller kernel provides a kernarg-segment pointer AND carries a COV5+
// implicit-args block (ORIGINAL kernarg_size >= 256) — i.e. whether the entry prologue
// captured the implicit-args pointer into the IACR, so an inserted call can forward it
// (blockDim/gridDim, Phase 3b). Returns the kernarg-ptr SGPR index, or -1. kernarg_ptr
// precedes flat_scratch_init in ABI order, so it is not shifted by scratch-enable.
static int readCallerKernargReg(func_instance *caller, codeGen & /*gen*/) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  if (!km)
    return -1;
  const Dyninst::DyninstAPI::AbiSgprLayout &L = km->layout();
  if (L.kernargPtr < 0 || km->originalKernargSize < 256)
    return -1;
  return L.kernargPtr;
}

// Byte offset from the kernarg-segment base to the COV5 implicit-args block (the fixed
// 256B tail): offset = originalKernargSize - 256. Uses the PRISTINE kernarg size cached
// in the KernelMeta — NOT the grown value in the KD — so the original-vs-grown ambiguity
// is gone (pillar B; formerly computed as grownSize-264 = originalSize-256). Added to the
// captured kernarg pointer to form the implicit-args pointer. -1 if unavailable.
static int readCallerImplicitOffset(func_instance *caller, codeGen & /*gen*/) {
  Dyninst::DyninstAPI::KernelMeta *km = callerMeta(caller);
  if (!km)
    return -1;
  const uint32_t ks = km->originalKernargSize;
  return (ks >= 256) ? (int)(ks - 256u) : -1;
}

// Shared gfx9-family inter-module call implementation. Both the gfx908 and gfx940 emitters delegate
// here (see the thin overrides at the end of this file). Takes the runtime-selected emitter by ref;
// routes every instruction through it, so each gen gets its own encodings (byte-identical across gfx9).
static void amdgpuEmitIndirectCall(EmitterAmdgpu &E, Dyninst::Address slotAddr, Dyninst::Register lrPair, codeGen &gen);

static Register amdgpuEmitCall(EmitterAmdgpu &E, opCode op, codeGen &gen,
                                       const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands,
                                       func_instance *callee) {
  assert(op == callOp);
  assert(callee && "emitCall: callee is null");

  // Immediate call arguments (if any) are materialized into the AMDGPU function-CC
  // argument registers just before the call — see below, after the register spill.

  // Determine whether the callee lives in a different mapped object (i.e.
  // a different module / shared object). The static-rewriting flow has us
  // here only when there is a current function; matches the x86 idiom in
  // EmitterIA32Stat::emitCallInstruction.
  func_instance *caller = gen.func();
  const bool isExternal = caller && (caller->obj() != callee->obj());

  if (!isExternal) {
    // Intra-module direct call would be a PC-relative branch via
    // insnCodeGen::generateBranch. Leaving that path for future work so this
    // commit stays scoped to the inter-module case.
    assert(!"AMDGPU emitCall: intra-module direct call not implemented yet");
    return Null_Register;
  }

  // The link-register pair receives the return address from S_SWAPPC_B64.
  // It MUST be s[30:31]: the AMDGPU non-kernel-function ABI returns via
  // `s_setpc_b64 s[30:31]`, so a separately-compiled callee reads its return
  // address from s[30:31]. Using an arbitrary allocated pair here leaves
  // s[30:31] untouched and the callee returns to garbage (jump to ~0 / fault).
  // s30/s31 is the ABI return-address register (caller-clobbered by a call), so
  // it is fixed, not taken from the allocator.
  registerSpace *rs = gen.rs();
  assert(rs && "AMDGPU emitCall: codeGen has no registerSpace");
  Register lrPair(30);

  // ===========================================================================
  // gfx942 ARCHITECTED-SCRATCH caller-save model. Measured on MI300A (rocgdb +
  // DYNINST_DUMP_LIVE): at these relocated / SIMT-split instrumentation points Dyninst's point
  // liveness returns EMPTY (nothing reported live) — so the liveness-DRIVEN spill reduction
  // (reduceSgpr/reduceVgpr, below) would preserve NOTHING, and the callee (bb_inc/bb_flush_pw)
  // clobbers every kernel register the body still needs (the kernarg s[0:1] read late for args,
  // the packed work-item id v0, and every computed value / per-lane datum under a partial EXEC
  // mask -> wild addresses -> faults). The correct, principled fallback — the SAME one the code
  // already uses when there is no point liveness (see the FULL-footprint spill note below) — is
  // to preserve the ENTIRE callee-clobber footprint (all of s[0..nsgpr), v[0..nvgpr), vcc, s32)
  // saved/restored under forced EXEC=-1 so ALL lanes (active and inactive) survive the call.
  // This is not a per-register hack: it is conservative caller-save, correct whenever liveness
  // is unavailable, and it SUBSUMES the earlier s[0:1]/v0 force-preserves. Arch-gated: gfx908
  // keeps its (working) liveness-driven reduction and is byte-identical.
  const bool architected = callerArchitectedScratch(caller);

  // ABI live-in: a caller kernel that uses its OWN buffer scratch keeps the scratch V#
  // descriptor LIVE in the Private-Segment-Buffer SGPRs (s[0:3]) for the whole body
  // (buffer_load/store off s[0:3] address its frame). It is an ABI register the
  // trampoline must not hand out — otherwise the inter-module GOT idiom (getpc s[0:1];
  // s_load s[2:3],s[0:1]; swappc ,s[2:3]) gets allocated s[0:1]/s[2:3] == the live
  // descriptor and the call jumps to the descriptor base (rocgdb: faulting PC == s[0:1]).
  // Reserve it here so allocateGprBlock (GOT idiom, arg lowering) skips it. Only when the
  // kernel actually uses scratch (O>0); O==0 leaves s[0:3] free exactly as before, so
  // every leaf/getpc-free case is byte-identical (no regression).
  std::vector<Register> abiScratchReserved;
  if (readCallerOriginalPrivate(caller) > 0) {
    Dyninst::DyninstAPI::KernelMeta *ckm = callerMeta(caller);
    int psb = ckm ? ckm->layout().privSegBuf : -1;   // Private Segment Buffer = s[psb:psb+3]
    if (psb >= 0)
      for (uint32_t r = (uint32_t)psb; r < (uint32_t)psb + 4u; r++) {
        Register rr = Register(r);
        if (rs->allocateSpecificRegister(gen, rr))
          abiScratchReserved.push_back(rr);
      }
  }

  // Preserve the caller's registers across the call. The AMDGPU function ABI
  // lets a callee clobber any caller-saved SGPR (e.g. hc_open uses s[4:5] for
  // its exec-mask save, but the kernel holds its kernarg pointer there). So
  // spill the SGPRs the callee will clobber to this wave's hardware-scratch slot (via
  // FLAT_SCRATCH, set up by the entry prologue) before the call, and reload them after.
  // The clobber count is the callee's TRANSITIVE "numbered_sgpr" — the whole-
  // object max, not just this wrapper's frame, because the wrapper may call
  // other functions in its library that clobber more SGPRs (see
  // readCalleeMaxAbsSym).
  // Register footprint the callee (transitively) clobbers — whole-object max, so
  // it covers everything the wrapper reaches (e.g. hc_write -> gpu_fwrite).
  const uint32_t nsgpr = readCalleeMaxAbsSym(callee, "numbered_sgpr");
  const uint32_t nvgpr = readCalleeMaxAbsSym(callee, "num_vgpr");

  // --- roadmap 1.0/1.1: liveness probe -------------------------------------
  // AMDGPU register liveness IS wired in this dyninst (ABI.C dispatches the
  // gfx908 register->bitArray map; the decoder marks per-operand read/written;
  // registerSpace::actualRegSpace(point) -> instPoint::liveRegisters() populates
  // liveState). This diagnostic dumps, at each insertion point, which of the
  // callee-clobbered registers s0..s(nsgpr-1)/vcc/v0..v(nvgpr-1) are actually
  // LIVE in the caller — the set the efficient spill (1.1) may restrict itself
  // to. Env-gated so it costs nothing by default.
  if (getenv("DYNINST_DUMP_LIVE") && gen.point()) {
    registerSpace *lrs = registerSpace::actualRegSpace(gen.point());
    if (lrs) {
      auto isLive = [&](Register r) -> bool {
        registerSlot *sl = (*lrs)[r];
        return sl && sl->liveState == registerSlot::live;
      };
      std::string liveS, liveV;
      for (uint32_t i = 0; i < nsgpr; i++)
        if (isLive(Register(i)))
          liveS += " s" + std::to_string(i);
      if (isLive(Register(106))) liveS += " vcc_lo";
      if (isLive(Register(107))) liveS += " vcc_hi";
      for (uint32_t i = 0; i < nvgpr; i++)
        if (isLive(Register(NS_amdgpu::VGPR_OPERAND_BASE + i)))
          liveV += " v" + std::to_string(i);
      fprintf(stderr,
              "[amdgpu][live] @0x%lx call %s: callee clobbers s0..s%u,vcc,v0..v%u; "
              "LIVE-at-point SGPR:{%s } VGPR:{%s }\n",
              (unsigned long)gen.point()->insnAddr(),
              callee ? callee->symTabName().c_str() : "?", nsgpr ? nsgpr - 1 : 0,
              nvgpr ? nvgpr - 1 : 0, liveS.c_str(), liveV.c_str());
    }
  }

  // Register spill uses hardware SCRATCH via the ScratchAbi seam: the per-wave base
  // lives in FLAT_SCRATCH (s[102:103], a SPECIAL register — not from the allocatable
  // pool, so 0 GP cost) and the per-lane swizzle is hardware-provided, so there is no
  // lane-offset math and each register needs only 4 bytes of the slot. gfx908 scratch
  // still needs a SADDR *register* (both-operands-off is illegal), but it's just a
  // constant 0 in a reserved SGPR (set per-trampoline). (An older global-buffer spill
  // backend existed behind DYNINST_SPILL_SCRATCH; it has been removed.)
  Dyninst::DyninstAPI::ScratchAbi &sabi = Dyninst::DyninstAPI::gfx908ScratchAbi();

  // Implicit-Arg Capture Region (IACR): when implicit-arg forwarding is enabled, a
  // fixed, write-once-at-entry / read-only region occupies per-lane scratch offsets
  // [0, IACR_BYTES); the caller-save spill and the callee frame shift UP by this so
  // they can't overwrite it. Retrieved (below) into the callee's ABI registers. See
  // amdgpu-implicit-args.h and the entry-capture in emitScratchEntryPrologue.
  // On by default: an inserted callee may be ordinary HIP device code reading
  // blockIdx/blockDim/threadIdx. (A future optimization could gate this per-callee
  // on whether the callee actually reads any implicit ABI register.)
  const uint32_t implBase = Dyninst::DyninstAPI::ImplicitArgLayout::BYTES;

  // Non-leaf call-ABI: the setupCalleeStack path (below) reconstructs s[0:3] and
  // OVERWRITES s32 (SP = s32Base). Per the AMDGPU ABI the callee restores s32 to
  // its INCOMING value (our s32Base), NOT the kernel's — so across the call s32 is
  // effectively clobbered. s32 lies OUTSIDE the callee register footprint [0,nsgpr)
  // that the caller-save spill covers, so it must be added to the save set
  // explicitly ("liveness for the ABI"): a kernel that uses s32 (vectoradd
  // allocates s0..s39) would otherwise read garbage after the call -> wild pointer.
  // Auto-detected: a callee is non-leaf (needs its own stack frame set up) iff it
  // declares private/scratch use. No env gate — the callee's metadata decides.
  const bool nonLeafCallAbi =
      readCalleeMaxAbsSym(callee, "private_seg_size") > 0;

  // Scratch VGPRs for the spill machinery:
  //   vPack    = holds SGPRs packed one-per-lane for the SGPR spill (below)
  //   vImplTmp = a pair for implicit-arg marshalling / per-wave-buf loads / captureRet
  // The SGPR spill packs {nsgpr numbered SGPRs, vcc_lo, vcc_hi} one-per-lane; each pack
  // VGPR holds 64 lanes -> ceil((nsgpr+2)/64) pack VGPRs (numPacks==1 for typical callees).
  const uint32_t kernelVgpr = readCallerOriginalGrantedVgpr(caller, gen);  // caller's ORIGINAL grant
  const uint32_t packLanes = spillPackLanes();   // 64 (wave64); overridable for testing
  const uint32_t numPacks = (nsgpr + 2u + packLanes - 1u) / packLanes;

  // Pillar A (final): allocate the trampoline's VGPR temps (vPack = the SGPR-pack VGPRs,
  // vImplTmp = a consecutive PAIR for implicit-arg marshalling / per-wave-buf loads /
  // captureRet) from the REAL register allocator on the point-specialized register space
  // (gen.rs() = actualRegSpace(gen.point()) — the same space used for the SGPR block and
  // the spill set). KEY: these temps do NOT need to survive the call — vPack's data lives
  // in hardware scratch (S_SGPR_SLOT) across it and vImplTmp is transient within one
  // pre-/post-call step — so they only need to be DEAD AT THE POINT, not above the callee
  // footprint. The caller-dead tail [kernelVgpr, nvgpr) (and any dead low VGPR) reads as
  // dead in the point liveness, so allocateGprBlock hands it out with NO grant growth
  // (grant stays max(caller_orig, callee_nvgpr)). We mark OFF-LIMITS only what the
  // trampoline must keep intact:
  //   * everything at/above the grant (those VGPRs don't physically exist in the wave),
  //   * v31 = the workitem-id ABI reg the implicit marshalling writes — reserving it
  //     STRUCTURALLY removes the old "vImplTmp+1 == v31, safe only by ordering" landmine,
  //   * the per-arg VGPRs the call materializes into v0.. and v0:v1 (the return value
  //     captureReturn reads) — so a temp never lands on an arg or the return.
  // Caller-live VGPRs are already live in the space, so the allocator stays off them.
  // Freed after the post-call restore so the arg/return codegen can reuse the registers.
  using DA_IAL = Dyninst::DyninstAPI::ImplicitArgLayout;
  const uint32_t vgprGrant = (kernelVgpr > nvgpr ? kernelVgpr : nvgpr);   // = max(orig, nvgpr)
  // Bound the per-arg VGPRs (materialized into v0..argDwords-1) + the v0:v1 return, by
  // pre-scanning operands with the SAME advance rule as the materialization loop below.
  uint32_t argDwords = 0; bool hasCaptureRet = false;
  for (const auto &opnd : operands) {
    if (!opnd) continue;
    if (opnd->getoType() == operandType::Constant) { argDwords += 1; continue; }
    if (opnd->getoType() == operandType::GpuValue) {
      const long k = (long)(reinterpret_cast<uintptr_t>(opnd->getOValue()) & 0xFF);
      if (k == (long)GpuValueKind::PerWaveBuf || k == (long)GpuValueKind::PerWaveVal) argDwords += 2;
      else if (k == (long)GpuValueKind::CaptureRet) hasCaptureRet = true;
      else argDwords += 1;
    } else argDwords += 1;
  }
  const uint32_t argVEnd = (hasCaptureRet && argDwords < 2u) ? 2u : argDwords;  // cover v0:v1 return

  uint32_t vPack, vImplTmp;
  Register vTmpBlk; bool vTmpOk = false;
  // gfx942: point liveness is EMPTY (see the caller-save note above) AND the KD's granulated
  // VGPR count UNDER-reports the kernel's real usage (measured on MI300A: reports 8 for a kernel
  // that uses 12 — CDNA3 granule / AGPR accounting differs). So allocateGprBlock lands a spill
  // temp (vPack/vImplTmp) on a kernel-live VGPR (e.g. v3 holding the `acc` cndmask input) and the
  // post-call unpack overwrites it -> the kernel stores per-lane-shifted garbage (out[i] wrong
  // while counts are correct). Robust placement: put the temps ABOVE the CALLEE clobber footprint
  // (vgprGrant = max(orig,nvgpr)); the kernel (below its grant) and the callee (below nvgpr) never
  // touch them, and their transient data lives in scratch across the call anyway. bumpCallerKd-
  // ForCallee grows the grant to fit. gfx908 keeps the real-liveness allocation (byte-identical).
  if (architected) {
    vPack = vgprGrant; vImplTmp = vgprGrant + numPacks;   // v[vgprGrant .. vgprGrant+numPacks+1]
  } else {
    auto liveV = [&](uint32_t id) {
      Register r = Register(NS_amdgpu::VGPR_OPERAND_BASE + id);
      if (registerSlot *s = (*rs)[r]) s->liveState = registerSlot::live;
    };
    for (uint32_t i = vgprGrant; i <= NS_amdgpu::MAX_VGPR_ID; i++) liveV(i);  // beyond the grant
    liveV(DA_IAL::ABI_WITEMID_VGPR);                                          // v31 (workitem ABI)
    for (uint32_t i = 0; i < argVEnd; i++) liveV(i);                          // per-arg + return v0:v1
    vTmpBlk = rs->allocateRegister(gen, registerSlot::VGPR, numPacks + 2u, 1);
    vTmpOk = !(vTmpBlk == Null_Register);
  }
  if (architected) {
    /* vPack/vImplTmp set above */
  } else if (vTmpOk) {
    vPack    = vTmpBlk - NS_amdgpu::VGPR_OPERAND_BASE;          // numPacks SGPR-pack VGPRs
    vImplTmp = vPack + numPacks;         // + the marshalling pair (vImplTmp, vImplTmp+1)
  } else {
    // Last resort: no dead block of numPacks+2 fits within the grant (extreme VGPR
    // pressure). Place in the caller-dead footprint tail by arithmetic if it fits (still
    // NO grant bump); only the truly-tight small-callee case grows via bumpCallerKdForCallee
    // (its independent vgprTempsFit test agrees). Not reached by these kernels; a spill-reuse
    // borrow (of an already-scratch-saved VGPR) would preserve occupancy here too.
    if (nvgpr >= kernelVgpr + numPacks + 2u) { vImplTmp = nvgpr - 2u; vPack = nvgpr - 2u - numPacks; }
    else { vPack = vgprGrant; vImplTmp = vgprGrant + numPacks; }
  }

  // Reserved SGPRs for the trampoline: a pair-aligned 4-SGPR block — EXEC save (pair),
  // scratch SADDR (=0), spare — that must SURVIVE the call.
  //
  // Pillar A: ALLOCATE this block from the register allocator (gen.rs(), already
  // specialized to this insertion point's liveness) instead of hardcoding grantTop-10.
  // The block must avoid registers that are live or that the call destroys, so we mark
  // those OFF-LIMITS by holding them across the allocation: the callee clobber range
  // s0..nsgpr-1 (caller-saved), the ABI link pair s30/31, the SP s32 for a non-leaf
  // call, the forwarded implicit-arg ABI SGPRs (s8 / s12 / s13 / s14 — written after,
  // so the block must not land on them), and everything from grantTop-6 up (the
  // special-reg-accounting slots per LLVM getNumExtraSGPRs, plus the SGPRs above the
  // wave's actual grant, which don't physically exist). FLAT_SCRATCH / VCC / EXEC /
  // XNACK are already off-limits in the register pool. The pinned regs held here are
  // released at the end. FALLBACK to the old fixed grantTop-10 if the allocator can't
  // satisfy the request — the grant is NEVER bumped just to make room for this block.
  const uint32_t sgprTop = readCallerGrantedSgprTop(caller, gen);
  std::vector<Register> pinnedSgpr;
  auto holdSgpr = [&](uint32_t id) {
    Register r = Register(id);
    if (rs->allocateSpecificRegister(gen, r)) pinnedSgpr.push_back(r);
  };
  for (uint32_t i = 0; i < nsgpr; i++) holdSgpr(i);        // callee clobber range (caller-saved)
  holdSgpr(30); holdSgpr(31);                              // ABI link pair s[30:31] (pinned)
  if (nonLeafCallAbi) holdSgpr(32);                        // SP (non-leaf ABI)
  holdSgpr(8); holdSgpr(12); holdSgpr(13); holdSgpr(14);   // forwarded implicit-arg ABI SGPRs
  for (uint32_t i = (sgprTop >= 6 ? sgprTop - 6 : 0);      // special-accounting slots + beyond-grant
       i <= NS_amdgpu::MAX_ALLOCATABLE_SGPR_ID; i++) holdSgpr(i);
  Register resBlock = rs->allocateRegister(gen, registerSlot::SGPR, 4, 2);
  const bool resBlockOk = !(resBlock == Null_Register);
  const uint32_t reservedBase =
      resBlockOk ? resBlock : (sgprTop >= 10 ? sgprTop - 10 : 0);
  // The pinned regs only STEERED the block allocation; release them now (the block
  // itself stays held to protect EXEC-save/SADDR/spare during arg lowering and the
  // call). In the fallback (no block), the later argReserved reservation guards the
  // fixed block, exactly as before.
  for (Register &r : pinnedSgpr) rs->freeRegister(r);
  const uint32_t EXEC_SAVE_REG = reservedBase;          // EXEC save pair
  const uint32_t SADDR_REG = reservedBase + 2;          // scratch SADDR (set to 0 per-trampoline)
  const uint32_t S_SGPR_SLOT = implBase;                // scratch: SGPR pack dword (per-lane, 4B), above the IACR
  const uint32_t S_VGPR_SLOT = implBase + numPacks * 4; // scratch: VGPR spill area, after the packs

  // SGPR (+VCC) preservation the way LLVM does it — NEVER through SMEM/K$. A
  // scalar-cache spill is corrupted by the callee's s_dcache_inv (issued for the
  // hostcall mailbox): observed as C-base reverting to a stale value -> +4 GiB
  // fault, timing-dependent and immune to wb/inv. Instead move each SGPR into a
  // lane of vPack with v_writelane, spill vPack through the VECTOR pipe
  // (global_store), and reverse with global_load + v_readlane. The vector pipe
  // (vmcnt / L1-L2) is untouched by s_dcache_inv — the same path the VGPR spill
  // already uses successfully. Lanes: s0..s(nsave-1) in lanes 0.., VCC_LO/HI next.
  // The per-lane pack store/load run under a forced EXEC=-1 (see the EXEC save/force
  // below, ~L1340), so inactive lanes at a narrowed-EXEC insertion point are covered.
  // Spill goes through hardware SCRATCH via the ScratchAbi (scratch_store/scratch_load off
  // FLAT_SCRATCH): the per-wave base and per-lane swizzle are hardware-provided, so there is
  // no explicit base register and no lane-offset math — SADDR is a reserved SGPR held at 0.
  // Pack the callee-clobbered scalars {s0..s(nsgpr-1), vcc_lo, vcc_hi} one-per-lane
  // across numPacks VGPRs (64 lanes each): scalar index i -> s_i (i<nsgpr), else
  // vcc_lo (i==nsgpr) / vcc_hi (i==nsgpr+1). Each pack VGPR spills to its own dword
  // slot, so there is no 62-SGPR limit. VCC(106/107) and s_i are fixed operand
  // encodings the HW maps to the wave's physical SGPRs regardless of the grant.
  // roadmap 1.1: restrict the spill to registers actually LIVE at the insertion
  // point (intersect the callee clobber footprint with actualRegSpace liveness).
  // With no point liveness it builds the FULL clobber list -> byte-identical to the
  // proven full-footprint spill; otherwise liveScalars/liveVgprs hold only the live
  // subset, so a point with nothing live (e.g. kernel exit) emits ZERO spill traffic.
  // The LAYOUT/reservation sizing above (numPacks, S_VGPR_SLOT, reservedBase, the KD
  // grant) stays conservative — computed from nsgpr/nvgpr — so all the delicate grant
  // and reserved-block logic is untouched; only the emitted save/restore ops shrink.
  // liveScalars holds SGPR operand ids (0..nsgpr-1, then 106/107 for vcc_lo/hi);
  // liveVgprs holds VGPR ids. See [[dyninst-amdgpu-liveness-works]].
  // DYNINST_LIVE_SPILL: reduce the register spill to the callee clobber footprint
  // INTERSECTED with the registers LIVE at the insertion point. Both SGPR and VGPR
  // liveness are reliable now that the aliased special-register liveness bug is
  // fixed (dataflowAPI/src/liveness.C: getBaseRegister maps vcc_lo/hi -> vcc,
  // exec_lo/hi -> exec, flat_scratch_lo/hi -> flat_scratch_all, none of which are
  // in the liveness index map, so their reads/writes were silently dropped and VCC
  // was reported dead across a live carry chain -> a reduced SGPR spill clobbered
  // VCC -> address fault). Validated: SGPR+VGPR reduction PASSES N=64/256/1024.
  // DYNINST_FULL_SGPR / DYNINST_FULL_VGPR force-spill that class in full (escape
  // hatches for A/B testing). See [[dyninst-amdgpu-liveness-works]].
  const bool liveSpill = true;   // liveness-driven spill reduction is always correct + a win
  // gfx942: point liveness is empty at these relocated/split points (see the caller-save note
  // above), so the reduction would drop the whole live kernel context -> save the FULL footprint.
  const bool reduceSgpr = liveSpill && !getenv("DYNINST_FULL_SGPR") && !architected;
  const bool reduceVgpr = liveSpill && !getenv("DYNINST_FULL_VGPR") && !architected;
  std::vector<uint32_t> liveScalars, liveVgprs;
  {
    // The caller-save spill set = caller-saved(callee) ∩ live(point), expressed via
    // RegisterContext. Callee clobber footprint: s0..nsgpr-1, v0..nvgpr-1, vcc, and —
    // "liveness for the ABI" — s32 (SP), which the non-leaf call ABI overwrites and
    // the callee restores to OUR s32Base (not the kernel's), so it must be preserved
    // for a non-leaf call even though it's outside [0,nsgpr).
    namespace DA = Dyninst::DyninstAPI;
    registerSpace *lrs = (liveSpill && gen.point())
                             ? registerSpace::actualRegSpace(gen.point())
                             : nullptr;
    DA::RegisterContext spillCallee, spillPoint;
    spillCallee.clobber.sgprCount = nsgpr;
    spillCallee.clobber.vgprCount = nvgpr;
    spillCallee.clobber.vcc = true;               // the SGPR pack always covers vcc_lo/hi
    if (nonLeafCallAbi) spillCallee.clobber.extraSgpr.push_back(32u);
    spillPoint.reduceSgpr = reduceSgpr;
    spillPoint.reduceVgpr = reduceVgpr;
    spillPoint.live = [&](DA::RegClass cls, uint32_t i) -> bool {
      if (!lrs) return true;                      // no liveness -> conservative: save all
      Register r = (cls == DA::RegClass::SGPR)
                       ? Register(i)
                       : Register(NS_amdgpu::VGPR_OPERAND_BASE + i);
      registerSlot *sl = (*lrs)[r];
      return sl && sl->liveState == registerSlot::live;
    };
    DA::SpillPlan plan = DA::lowerSpill(spillCallee, spillPoint);
    liveScalars = std::move(plan.sgprs);
    liveVgprs   = std::move(plan.vgprs);
  }
  const uint32_t nScalars = (uint32_t)liveScalars.size();
  // Packs actually needed for the LIVE scalars (<= numPacks reserved above), so the
  // pack VGPRs vPack..vPack+numPacksLive-1 and scratch slots p*4 stay a subset of
  // the conservatively reserved region.
  const uint32_t numPacksLive = (nScalars + packLanes - 1u) / packLanes;
  auto packSrc = [&](uint32_t idx) -> uint32_t { return liveScalars[idx]; };
  auto spillFill = [&](bool save) {
    if (nScalars == 0) return;             // nothing live to preserve (e.g. exit)
    if (save) {
      for (uint32_t p = 0; p < numPacksLive; p++)
        for (uint32_t l = 0; l < packLanes; l++) {
          const uint32_t idx = p * packLanes + l;
          if (idx >= nScalars) break;
          emitVop3a(V_WRITELANE_B32, /*vdst=*/vPack + p, /*src0=*/packSrc(idx),
                    /*src1=*/128 + l, /*src2=*/0, gen);
        }
      for (uint32_t p = 0; p < numPacksLive; p++)
        sabi.emitScratchStore(vPack + p, S_SGPR_SLOT + p * 4, SADDR_REG, gen);
      emitSopP(S_WAITCNT, /*simm16=*/0, gen);
    } else {
      for (uint32_t p = 0; p < numPacksLive; p++)
        sabi.emitScratchLoad(vPack + p, S_SGPR_SLOT + p * 4, SADDR_REG, gen);
      emitSopP(S_WAITCNT, /*simm16=*/0, gen);
      for (uint32_t p = 0; p < numPacksLive; p++)
        for (uint32_t l = 0; l < packLanes; l++) {
          const uint32_t idx = p * packLanes + l;
          if (idx >= nScalars) break;
          emitVop3a(V_READLANE_B32, /*vdst=*/packSrc(idx), /*src0=*/256 + (vPack + p),
                    /*src1=*/128 + l, /*src2=*/0, gen);
        }
    }
  };

  // Preserve the callee-clobbered VGPRs (e.g. v[4:5] address pair). Per-lane, via
  // hardware scratch. TRANSITIVE footprint (whole-object max): the wrapper's own
  // num_vgpr undercounts what it clobbers via nested calls.
  auto vgprSpill = [&](bool save) {
    if (liveVgprs.empty())                 // roadmap 1.1: only live VGPRs
      return;
    for (uint32_t i : liveVgprs) {
      if (i >= 256) continue;
      if (save) sabi.emitScratchStore(/*vreg=*/i, S_VGPR_SLOT + i * 4, SADDR_REG, gen);
      else      sabi.emitScratchLoad(/*vreg=*/i, S_VGPR_SLOT + i * 4, SADDR_REG, gen);
    }
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);   // let the VMEM group complete
  };

  // Drain ALL in-flight memory before saving registers. dyninst can splice this
  // call between an async load (s_load/global_load into a register) and the
  // compiler's s_waitcnt that guards that register's first use — so a register we
  // are about to spill may still be the target of an outstanding load. Without
  // this wait the save reads the STALE pre-load value (observed: A/B/C base
  // pointers saved as garbage while an s_load_dwordx4 from kernarg was in flight
  // -> nil/wild-address fault; deterministic, hidden by any halt that lets the
  // load retire). vmcnt+lgkmcnt=0 covers both VMEM and scalar/LDS loads.
  emitSopP(S_WAITCNT, /*simm16=*/0, gen);

  // Preserve EXEC across the call and force ALL lanes on for the register spill.
  // The trampoline never saved EXEC before; the callee (hostcall wrapper) narrows
  // EXEC for lane election and restores it, but the caller's live EXEC must be
  // guaranteed intact across the whole trampoline — AND the SGPR pack's per-lane
  // store/load must cover every lane (writelane is EXEC-independent, but the
  // store/load are not). Save the caller EXEC in a reserved pair (s[92:93], above
  // the callee's clobber range), run the spill under EXEC=-1, restore the caller
  // EXEC for the call and again for the post-call continuation. (No-op at full
  // EXEC, e.g. single-wave N<=64.)  S_MOV_B64=1; EXEC_LO=126; inline -1=193.
  // EXEC_SAVE_REG is the reserved pair: s[92:93] (global, maxed grant) or the top
  // of the tight scratch grant (reservedBase).
  const uint32_t S_MOV_B64_OP = 1, EXEC_LO = 126, INLINE_NEG1 = 193;

  // Scratch SADDR = the caller kernel's own private-frame size O, so our IACR+spill sit
  // ABOVE the caller's frame [0,O) (matches emitScratchEntryPrologue's SADDR and the
  // s32Base seating below). O==0 emits the exact same s_mov SADDR,0 as before. Set in the
  // trampoline (reservedBase+2, above the callee clobber range so it survives the call).
  const uint32_t origPriv = readCallerOriginalPrivate(caller);
  if (origPriv == 0)
    emitSop1Raw(/*S_MOV_B32=*/0, SADDR_REG, /*inline 0=*/128, gen);
  else
    emitSop2RawWithLiteral(S_OR_B32, SADDR_REG, /*inline 0=*/128, origPriv, gen);

  emitSop1Raw(S_MOV_B64_OP, EXEC_SAVE_REG, EXEC_LO,    gen);  // save exec
  emitSop1Raw(S_MOV_B64_OP, EXEC_LO,   INLINE_NEG1, gen); // exec = -1 (all lanes)

  spillFill(/*save=*/true);
  vgprSpill(/*save=*/true);

  emitSop1Raw(S_MOV_B64_OP, EXEC_LO, EXEC_SAVE_REG, gen);     // exec = caller (for call)

  // Materialize call arguments into the AMDGPU function-CC arg registers. Per the
  // ABI, VGPR args are consecutive from v0 (a plain `int` lands in v0); HIP can't
  // express inreg/SGPR params, so every arg ends in a VGPR. Two lowering paths:
  //   * Constant  -> uniform immediate straight into v(i): v_mov_b32 v(i), imm.
  //   * otherwise -> evaluate the operand AST with generateCode_phase2 (the same
  //     path the other emitters use) into a register, then move it into v(i). AMDGPU
  //     AST codegen is SGPR-based, so a computed arg is uniform-per-wave and is
  //     broadcast (v_mov_b32 v(i), s(res)); a VGPR result is copied per-lane.
  // v0.. are saved/restored by the spill above, so overwriting them here (after
  // save, before the call, under caller EXEC) is safe. generateCode_phase2 allocates
  // scratch registers from gen.rs(), which does NOT know about the trampoline's
  // hardcoded spill SGPRs (EXEC save at reservedBase / SADDR base) that the post-call
  // restore reads — so reserve that block across argument lowering to keep the
  // allocator off it.
  std::vector<Register> argReserved;
  if (!operands.empty()) {
    const uint32_t rb = reservedBase;
    for (uint32_t r = rb; r < rb + 4u; r++) {
      Register rr = Register(r);
      if (rs->allocateSpecificRegister(gen, rr))
        argReserved.push_back(rr);
    }
  }
  // Each explicit arg's DESTINATION register comes from the callee's arg CC — an
  // InputArg slot in the RegisterContext model (same abstraction as the implicit
  // inputs and the spill set). Default (HIP) CC: every explicit arg in a VGPR, arg i
  // -> v(i). An SGPR/inreg arg would be RegClass::SGPR here; that lands once the
  // callee's per-arg register assignment is derived (deferred data-population, like
  // the saturation verdict) — the destination model below already routes on class_.
  auto argDest = [&](uint32_t i) -> Dyninst::DyninstAPI::RegInfo {
    Dyninst::DyninstAPI::RegInfo d;
    d.role = Dyninst::DyninstAPI::Role::InputArg;
    d.class_ = Dyninst::DyninstAPI::RegClass::VGPR;   // TODO: SGPR for inreg args (arg-CC derivation)
    d.index = (uint16_t)i; d.dwords = 1; d.uniform = false;
    return d;
  };
  // VGPR cursor: HIP passes args in consecutive VGPRs from v0; a 32-bit arg takes
  // one VGPR, a 64-bit pointer arg (per-wave buffer) takes a pair. Advance by the
  // arg's dword width so a following arg lands in the right register.
  uint32_t vreg = 0;
  bool     captureReturnToPerWave = false; // set by a CaptureRet marker operand; handled post-call
  uint32_t captureOff = 0;                  // slice offset for the captured return
  for (uint32_t i = 0; i < operands.size(); i++) {
    const auto &arg = operands[i];
    assert(arg && "AMDGPU emitCall: null call argument");
    const Dyninst::DyninstAPI::RegInfo dst = argDest(i);
    assert(dst.class_ == Dyninst::DyninstAPI::RegClass::VGPR &&
           "AMDGPU emitCall: SGPR/inreg explicit args not yet lowered "
           "(needs callee arg-CC derivation to populate the InputArg slot)");
    if (arg->getoType() == operandType::Constant) {
      const uint32_t imm = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(arg->getOValue()));
      emitVop1Imm(/*V_MOV_B32=*/1u, /*vdst=*/vreg, imm, gen);
      vreg += 1;
    } else if (arg->getoType() == operandType::GpuValue) {
      const uintptr_t packed = reinterpret_cast<uintptr_t>(arg->getOValue());
      const long     kind  = (long)(packed & 0xFF);
      const uint32_t pwOff = (uint32_t)(packed >> 8);   // per-wave var byte offset in the slice
      if (kind == (long)GpuValueKind::PerWaveBuf) {
        // This wave's per-wave buffer slice: a 64-bit pointer captured at entry into
        // the IACR (OFF_PWBASE). Load both dwords straight into the pointer arg's VGPR
        // pair v(vreg):v(vreg+1). Uniform per wave (every lane's IACR slot holds the
        // same base), so the per-lane scratch_load gives every lane the same pointer.
        using IAL = Dyninst::DyninstAPI::ImplicitArgLayout;
        sabi.emitScratchLoad(vreg,     IAL::OFF_PWBASE,     SADDR_REG, gen);
        sabi.emitScratchLoad(vreg + 1, IAL::OFF_PWBASE + 4, SADDR_REG, gen);
        emitSopP(S_WAITCNT, /*simm16=*/0, gen);
        if (pwOff) {
          // Multi-var: this variable sits at slice+pwOff, so the pointer is base+pwOff.
          // 64-bit add-with-carry: VCC is dead here (the spill already saved it) and
          // vImplTmp is a free scratch VGPR. Offset -> VGPR (handles any 32-bit offset).
          emitVop1Imm(/*V_MOV_B32=*/1u, /*vdst=*/vImplTmp, pwOff, gen);
          emitVop2(V_ADD_CO_U32,  /*vdst=*/vreg,     /*vsrc1=*/vreg,     /*src0=*/256u + vImplTmp, gen); // lo += off, carry->vcc
          emitVop2(V_ADDC_CO_U32, /*vdst=*/vreg + 1, /*vsrc1=*/vreg + 1, /*src0=*/128u /*inline 0*/,   gen); // hi += carry
        }
        vreg += 2;
      } else if (kind == (long)GpuValueKind::PerWaveVal) {
        // This wave's stored 64-bit VALUE (slice[0], e.g. a captured file handle):
        // load the per-wave base (IACR) into a VGPR PAIR and global-load the value into
        // the arg VGPR pair via NULL-saddr addressing (address = v[vImplTmp:vImplTmp+1]).
        // Do NOT readfirstlane the base into an SGPR pair for a SADDR form: on gfx9 an
        // SGPR written by v_readfirstlane can't be safely read by the very next global's
        // SADDR (a wait-state hazard) — at full speed the store/load used a STALE base
        // and faulted (single-stepping hid it; rocgdb-diagnosed). The VGPR path is
        // vmcnt-ordered by the s_waitcnt, so no such hazard. (SGPR grant is 48 and would
        // have held the pair fine — the bug was the hazard, not an overstep.)
        using IAL = Dyninst::DyninstAPI::ImplicitArgLayout;
        sabi.emitScratchLoad(vImplTmp,     IAL::OFF_PWBASE,     SADDR_REG, gen);
        sabi.emitScratchLoad(vImplTmp + 1, IAL::OFF_PWBASE + 4, SADDR_REG, gen);
        emitSopP(S_WAITCNT, /*simm16=*/0, gen);
        // Multi-var: value lives at slice+pwOff — fold pwOff into the load immediate
        // (13-bit signed => pwOff must be <= 4091; ample for value/capture vars).
        emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/vreg,
                                     /*addr=*/vImplTmp, /*data=*/0, /*saddr=*/0x7fu /*NULL*/,
                                     /*offset=*/(int)pwOff, gen, /*glc=*/true, /*seg=*/2u);
        emitFlatGlobal(GLOBAL_LOAD_DWORD, /*vdst=*/vreg + 1,
                                     /*addr=*/vImplTmp, /*data=*/0, /*saddr=*/0x7fu /*NULL*/,
                                     /*offset=*/(int)pwOff + 4, gen, /*glc=*/true, /*seg=*/2u);
        emitSopP(S_WAITCNT, /*simm16=*/0, gen);
        vreg += 2;
      } else if (kind == (long)GpuValueKind::CaptureRet) {
        captureReturnToPerWave = true;   // MARKER: capture the return post-call, not an arg
        captureOff = pwOff;              // ...into slice[pwOff]
      } else {
        // A GPU hardware/execution value read at the site: emit its recipe into the
        // reserved arg-scratch SGPR, then broadcast that uniform value into the arg VGPR.
        // Recipe scratch = the SPARE reserved SGPR (reservedBase+3); [reservedBase..+3]
        // is EXEC-save (rb,rb+1), SADDR (rb+2), spare (rb+3); rb+3 is held by argReserved.
        const uint32_t sc = reservedBase + 3;
        if (kind == (long)GpuValueKind::ExecMask)
          emitSop1Raw(/*S_MOV_B32=*/0u, sc, /*exec_lo=*/126u, gen);
        else /* HwWaveId */
          emitSopK(S_GETREG_B32, sc, /*hwreg(HW_ID)=*/(int16_t)0xF804, gen);
        emitVop1Reg(/*V_MOV_B32=*/1u, /*vdst=*/vreg, /*src0(SGPR)=*/sc, gen);
        vreg += 1;
      }
    } else {
      Dyninst::Address unusedAddr = 0;
      Register res = Dyninst::Null_Register;
      if (!arg->generateCode_phase2(gen, unusedAddr, res) || res == Dyninst::Null_Register)
        assert(!"AMDGPU emitCall: could not lower non-constant call argument");
      // src0 encoding: SGPR n -> n, VGPR n -> 256+n.
      const uint32_t src0 = res;
      emitVop1Reg(/*V_MOV_B32=*/1u, /*vdst=*/vreg, src0, gen);
      rs->freeRegister(res);
      vreg += 1;
    }
  }
  for (Register &rr : argReserved)
    rs->freeRegister(rr);

  // --- Implicit ABI args, via a RegisterContext relation -----------------------
  // An inserted call sets up none of the callee's implicit ABI context, so we build
  // the callee's implicit-input CONTRACT and the mutatee point's AVAILABILITY as two
  // RegisterContexts and let lowerImplicitArgs() relate them into marshalling steps.
  // Each value comes from the IACR (captured at kernel entry) — NOT the live ABI
  // register, which the kernel reuses mid-kernel (rocgdb-proven). SGPR (uniform)
  // destinations are RESERVED so emitIndirectCall's call-target block avoids them,
  // then materialized here and kept live to the swappc (freed below). Emitted BEFORE
  // setupCalleeStack (touches neither s0..3 nor s32). SADDR_REG is already 0.
  std::vector<Register> abiReserved;
  {
    namespace DA = Dyninst::DyninstAPI;
    using IAL = DA::ImplicitArgLayout;

    // Callee contract — the implicit inputs this callee may read and their ABI regs.
    // (Full set for now = forward-always; per-callee need-detection is a later
    // refinement the RegisterContext enables.)
    DA::RegisterContext calleeCtx;
    auto want = [&](DA::RegClass c, uint16_t idx, uint8_t dw, bool uni, DA::ImplicitSource s) {
      DA::RegInfo ri; ri.class_ = c; ri.index = idx; ri.dwords = dw; ri.uniform = uni;
      ri.role = DA::Role::InputImplicit; ri.implicit = s; calleeCtx.regs.push_back(ri);
    };
    want(DA::RegClass::VGPR, IAL::ABI_WITEMID_VGPR, 1, /*uniform=*/false, DA::ImplicitSource::WorkitemId);
    want(DA::RegClass::SGPR, IAL::ABI_BLOCKIDX_X,   1, /*uniform=*/true,  DA::ImplicitSource::WgidX);
    want(DA::RegClass::SGPR, IAL::ABI_BLOCKIDX_Y,   1, /*uniform=*/true,  DA::ImplicitSource::WgidY);
    want(DA::RegClass::SGPR, IAL::ABI_BLOCKIDX_Z,   1, /*uniform=*/true,  DA::ImplicitSource::WgidZ);
    want(DA::RegClass::SGPR, /*s8*/8u,              2, /*uniform=*/true,  DA::ImplicitSource::ImplicitArgPtr);

    // Point availability — what this mutatee kernel can supply, and from which IACR
    // slot (captured by emitScratchEntryPrologue). blockIdx components only if the
    // kernel enables them; the implicit-args pointer = captured kernarg ptr + the
    // COV5 implicit-block offset.
    DA::RegisterContext point;
    auto provide = [&](DA::ImplicitSource s, int32_t off, uint8_t dw, bool uni, int32_t add) {
      DA::SourceLoc &sl = point.source(s);
      sl.available = true; sl.iacrOffset = off; sl.dwords = dw; sl.uniform = uni; sl.addend = add;
    };
    provide(DA::ImplicitSource::WorkitemId, IAL::OFF_WITEMID, 1, /*uniform=*/false, 0);
    // blockIdx.{x,y,z}: ALWAYS forwarded from the IACR. The entry prologue writes EVERY
    // slot (the real relocated wgid where the kernel enables it, 0 otherwise), so a
    // callee's general global_wavefront_id() sees by=bz=0 on a lower-rank launch instead
    // of a garbage register. (Previously only kernel-provided components were forwarded,
    // leaving blockIdx.y/z unset — fine when probes read only .x, wrong for 2D/3D.)
    provide(DA::ImplicitSource::WgidX, IAL::OFF_WGID_X, 1, /*uniform=*/true, 0);
    provide(DA::ImplicitSource::WgidY, IAL::OFF_WGID_Y, 1, /*uniform=*/true, 0);
    provide(DA::ImplicitSource::WgidZ, IAL::OFF_WGID_Z, 1, /*uniform=*/true, 0);
    const int implOff = readCallerImplicitOffset(caller, gen);
    if (readCallerKernargReg(caller, gen) >= 0 && implOff >= 0)
      provide(DA::ImplicitSource::ImplicitArgPtr, IAL::OFF_KERNARG, 2, /*uniform=*/true, implOff);

    // Lower the callee contract against point availability -> ordered steps, and emit.
    auto reserve = [&](uint32_t r) {
      Register rr = Register(r);
      if (rs->allocateSpecificRegister(gen, rr)) abiReserved.push_back(rr);
    };
    for (const DA::MarshalStep &st : DA::lowerImplicitArgs(calleeCtx, point)) {
      if (st.uniform) {   // SGPR dest via readfirstlane; reserve so the call target avoids it
        for (uint8_t d = 0; d < st.dwords; d++) reserve(st.dstIndex + d);
        for (uint8_t d = 0; d < st.dwords; d++) {
          sabi.emitScratchLoad(vImplTmp, st.iacrOffset + 4 * d, SADDR_REG, gen);
          emitSopP(S_WAITCNT, /*simm16=*/0, gen);
          emitVop1Reg(/*V_READFIRSTLANE_B32=*/2u, /*sdst=*/st.dstIndex + d,
                                    /*vsrc=*/256u + vImplTmp, gen);
        }
        if (st.addend) {  // pointer fix-up (kernarg -> implicit-args ptr): dst += addend
          emitSop2RawWithLiteral(S_ADD_U32, st.dstIndex, st.dstIndex,
                                               (uint32_t)st.addend, gen);
          emitSop2Raw(S_ADDC_U32, st.dstIndex + 1, st.dstIndex + 1,
                                    /*inline 0=*/128u, gen);
        }
      } else {            // per-lane VGPR dest: straight scratch_load (no readfirstlane / reserve)
        sabi.emitScratchLoad(st.dstIndex, st.iacrOffset, SADDR_REG, gen);
        emitSopP(S_WAITCNT, /*simm16=*/0, gen);
      }
    }
  }

  // --- Non-leaf call-ABI (auto-detected from callee private_seg_size > 0) -------
  // A hipcc-compiled NON-LEAF callee addresses its own stack frame as buffer_* off
  // the scratch V# descriptor s[0:3] + stack pointer s32. Our flat-scratch spill
  // occupies flat offsets [0,R); reconstruct s[0:3] from FLAT_SCRATCH (kept live)
  // and place the callee's stack base s32 = R (buffer soffset == flat offset,
  // validated 1:1) so the callee's frame sits ABOVE our spill and can't collide.
  // Only when the callee is non-leaf (its private_seg_size > 0); leaf callees need
  // no stack. Runs AFTER the caller-save spill (which preserved s0..s3 if live) and
  // arg materialization, and BEFORE the call; s0..s3 are restored afterward.
  {
    const uint32_t calleeScratch = readCalleeMaxAbsSym(callee, "private_seg_size");
    if (calleeScratch > 0) {   // non-leaf callee -> set up its buffer-scratch frame
      // Frames STACK: [0,O) caller's own frame, [O,O+spillR) our IACR+spill (SADDR=O),
      // then the callee frame above. spillRfull = O + spillR is our region's per-lane top.
      const uint32_t spillR  = implBase + 4u * (numPacks + nvgpr);   // IACR + spill (per-lane)
      // origPriv (caller's own private-frame size O) is the outer decl from the SADDR
      // setup above — same readCallerOriginalPrivate(caller) value, reused here.
      const uint32_t spillRfull = origPriv + spillR;
      // (architected computed once at emitCall top)
      // s32 (stack pointer) units differ by arch:
      //  * gfx908 (buffer-scratch): PER-WAVEFRONT bytes — the reconstructed HIP scratch
      //    descriptor swizzles per lane, so the callee's per-lane frame offset = s32/64
      //    (wave64). Seat above our per-lane region [0,spillRfull): s32Base >= spillRfull*64.
      //  * gfx942 (architected scratch_* off s32): PER-LANE bytes — s32 is added directly
      //    to the per-lane swizzled base (same model as our spill SADDR), so s32Base =
      //    spillRfull (no *64). Must match bumpCallerKdForCallee's sizing.
      // gfx908 per-wave soffset uses the wavefront lane count (64 for wave64) — read from the
      // caller KD so it isn't hardcoded for a hypothetical wave32 non-architected arch.
      Dyninst::DyninstAPI::KernelMeta *wkm = callerMeta(caller);
      const uint32_t waveLanes = (wkm && !wkm->kd.isWave64()) ? 32u : 64u;
      const uint32_t s32Base = architected
          ? ((spillRfull + 0xFu) & ~0xFu)                       // per-lane, 16B-aligned (frame align)
          : (((spillRfull * waveLanes) + 0x3FFu) & ~0x3FFu);    // per-wave, 0x400-aligned
      if (!architected && origPriv > 0) {
        // Caller uses its OWN buffer scratch: the scratch V# descriptor is an ABI live-in
        // captured in IACR[OFF_VDESC] at entry. Reload the REAL descriptor into s[0:3] and
        // pass it to the callee (readfirstlane, same as implicit-arg retrieval) — do NOT
        // fabricate one (wrong num_records; clobbers the caller's live descriptor).
        const int32_t vdescOff = Dyninst::DyninstAPI::ImplicitArgLayout::OFF_VDESC;
        for (uint32_t d = 0; d < 4u; d++) {
          sabi.emitScratchLoad(vImplTmp, vdescOff + 4 * (int32_t)d, SADDR_REG, gen);
          emitSopP(S_WAITCNT, /*simm16=*/0, gen);
          emitVop1Reg(/*V_READFIRSTLANE_B32=*/2u, /*sdst=s(d)*/d, 256u + vImplTmp, gen);
        }
        sabi.setupCalleeStack(s32Base, /*reconstructDescriptor=*/false, /*architected=*/false, gen);  // s[0:3] loaded; set s32 only
      } else {
        // gfx942 architected: set s32 only (no descriptor). gfx908 leaf caller: fabricate s[0:3]+s32.
        sabi.setupCalleeStack(s32Base, /*reconstructDescriptor=*/!architected, architected, gen);
      }
    }
  }

  Address slot = E.getInterModuleFuncAddr(callee, gen);
  // Hold the reserved trampoline block (EXEC-save pair + SADDR, at reservedBase..+3)
  // across the call so emitIndirectCall's GOT idiom block CANNOT land on it. gen.rs()
  // does not track resBlock (see the reservedBase comment above — it is re-specialized
  // to the point liveness during arg lowering), and argReserved, which re-held this
  // block on the current rs, was already released after arg lowering. Without this
  // re-hold the GOT idiom's target pair aliased SADDR (both s[reservedBase+2]) and the
  // s_swappc clobbered the spill SADDR, faulting the post-call restore. Same live-across-
  // the-call pattern as abiReserved; freed immediately after emitIndirectCall.
  std::vector<Register> resHold;
  for (uint32_t r = reservedBase; r < reservedBase + 4u; r++) {
    Register rr = Register(r);
    if (rs->allocateSpecificRegister(gen, rr)) resHold.push_back(rr);
  }
  amdgpuEmitIndirectCall(E, slot, lrPair, gen);
  for (Register &rr : resHold)     rs->freeRegister(rr);   // reserved block survived the call
  for (Register &rr : abiReserved)   // release the forwarded ABI regs now the call is emitted
    rs->freeRegister(rr);

  // Capture the callee's ABI return value (v0:v1) into this wave's slice[0]. Done HERE
  // — after the call, while EXEC is still the caller's and v0:v1 hold the return — and
  // BEFORE the caller-save restore below (vgprSpill(false) reloads v0:v1). The wrapper
  // broadcast the value so v0:v1 are uniform across lanes; store via the per-wave base
  // (IACR -> SGPR pair rb+4:+5) with a zero VGPR offset. Enables: pw = gpu_fopen(...).
  if (captureReturnToPerWave) {
    // Store v0:v1 to this wave's slice[0] via a VGPR-PAIR address (NULL saddr). The
    // base is loaded into v[vImplTmp:vImplTmp+1] and used directly as the address — NOT
    // readfirstlane'd into an SGPR SADDR, which hits a gfx9 v_readfirstlane->global-SADDR
    // wait-state hazard (store used a STALE base at full speed; rocgdb single-step showed
    // the same store succeeding once the write settled). The scratch loads are
    // vmcnt-ordered by the s_waitcnt, so the address is committed before the store.
    using IAL = Dyninst::DyninstAPI::ImplicitArgLayout;
    sabi.emitScratchLoad(vImplTmp,     IAL::OFF_PWBASE,     SADDR_REG, gen);
    sabi.emitScratchLoad(vImplTmp + 1, IAL::OFF_PWBASE + 4, SADDR_REG, gen);
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);
    emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0,
                                 /*addr=*/vImplTmp, /*data=*/0 /*v0*/, /*saddr=*/0x7fu /*NULL*/,
                                 /*offset=*/(int)captureOff, gen, /*glc=*/true, /*seg=*/2u);
    emitFlatGlobal(GLOBAL_STORE_DWORD, /*vdst=*/0,
                                 /*addr=*/vImplTmp, /*data=*/1 /*v1*/, /*saddr=*/0x7fu /*NULL*/,
                                 /*offset=*/(int)captureOff + 4, gen, /*glc=*/true, /*seg=*/2u);
    emitSopP(S_WAITCNT, /*simm16=*/0, gen);
  }

  emitSop1Raw(S_MOV_B64_OP, EXEC_LO, INLINE_NEG1, gen);   // exec = -1 (for restore)

  vgprSpill(/*save=*/false);
  spillFill(/*save=*/false);

  emitSop1Raw(S_MOV_B64_OP, EXEC_LO, EXEC_SAVE_REG, gen);     // exec = caller (continue)

  // Grow the caller kernel's register/scratch grant to fit the callee.
  bumpCallerKdForCallee(caller, callee, gen);

  // Release the liveness-allocated trampoline scratch (pillar A): the reserved SGPR block
  // and the VGPR spill temps (vPack/vImplTmp).
  if (resBlockOk) rs->freeRegister(resBlock, 4);
  if (vTmpOk)     rs->freeRegister(vTmpBlk, numPacks + 2u);

  // No return-value convention pinned down yet for AMDGPU calls.
  return Null_Register;
}

static bool amdgpuClobberAllFuncCall(registerSpace *rs, Dyninst::DyninstAPI::function_cache &clob, func_instance *callee) {
  if (!callee) {
    return true;
  }
  if (clob.contains(callee)) {
    return true;
  }
  clob.insert(callee);

  // No AMDGPU calling convention is pinned down in dyninst yet, so assume the
  // callee may write any allocatable SGPR or VGPR. Both kinds live in
  // registerSpace::GPRs_ on AMDGPU (registerSpace.C:170-181, SGPR/VGPR
  // fall through into the GPR case), so a single loop covers them.
  // numFPRs() is 0 on AMDGPU.
  for (int i = 0; i < rs->numGPRs(); i++) {
  //  rs->GPRs()[i]->beenUsed = true;
  }
  return true;
}

static void amdgpuEmitIndirectCall(EmitterAmdgpu &E, Address slotAddr, Register lrPair, codeGen &gen) {

  // Need two scratch SGPR pairs adjacent and pair-aligned:
  //   pair0 (reg, reg+1)   -- slot address (PC-relative), then SMEM base
  //   pair1 (reg+2, reg+3) -- PC-relative addend, then the loaded callee entry
  registerSpace *rs = gen.rs();
  assert(rs && "emitIndirectCall: codeGen has no registerSpace");

  // A caller kernel that uses its OWN buffer scratch keeps the scratch V# descriptor in
  // s[0:3]; emitCall has just reloaded the callee's descriptor there (from IACR[OFF_VDESC]).
  // s[0:3] is NOT in the register-space's allocated set (we wrote it via readfirstlane, not
  // via allocation), so at a site where the caller's s[0:3] is dead (e.g. the exit call) the
  // block allocator below would grab s[0:3] and clobber that descriptor before the swappc.
  // Reserve s[0:3] across THIS allocation so the GOT idiom's pair lands elsewhere; then free.
  std::vector<Register> vdescReserved;
  if (func_instance *caller = gen.func())
    if (readCallerOriginalPrivate(caller) > 0)
      for (uint32_t r = 0; r < 4u; r++) {
        Register rr = Register(r);
        if (rs->allocateSpecificRegister(gen, rr)) vdescReserved.push_back(rr);
      }

  // The GOT idiom's 4-SGPR block MUST lie within the caller kernel's SGPR grant.
  // allocateGprBlock draws from the architectural SGPR file (102 numbered SGPRs),
  // NOT from the wave's grant, so with no steering it can land the block ABOVE
  // grantedSgprTop — those SGPRs don't physically exist for the wave and read
  // garbage, so the s_load_dwordx2 below reads a wild address (scratch-aperture
  // violation). At a HIGH-PRESSURE mid-block point the caller's live registers fill
  // the low SGPRs, making the beyond-grant landing the common case. Hold everything
  // from grantTop-6 up (mirrors emitCall's reserved-block steering at ~L1409) so the
  // block lands WITHIN the grant. emitCall's reserved block (resBlock: EXEC-save +
  // SADDR) is still held here, so the GOT block also lands BELOW it — without this,
  // the GOT idiom's target pair aliased SADDR (both ~s66) and the call clobbered the
  // spill SADDR. Freed right after the allocation (the block stays held to the swappc).
  std::vector<Register> beyondGrant;
  if (func_instance *caller = gen.func()) {
    const uint32_t sgprTop = readCallerGrantedSgprTop(caller, gen);
    if (sgprTop > 0)
      for (uint32_t i = (sgprTop >= 6 ? sgprTop - 6 : 0);
           i <= NS_amdgpu::MAX_ALLOCATABLE_SGPR_ID; i++) {
        Register rr = Register(i);
        if (rs->allocateSpecificRegister(gen, rr)) beyondGrant.push_back(rr);
      }
  }

  Register block = rs->allocateRegister(gen, registerSlot::SGPR,
                                        /*numRegs=*/4,
                                        2);
  assert(block != Null_Register && "emitIndirectCall: failed to allocate scratch SGPR block");
  for (Register &rr : beyondGrant)   rs->freeRegister(rr);   // grant limit steered the block
  for (Register &rr : vdescReserved) rs->freeRegister(rr);   // block chosen; descriptor stays put

  Register slotAddrPair = block;      // s[block:block+1]
  Register tgtPair      = block + 2;   // s[block+2:block+3]

  // 1. Compute the slot's runtime address PC-relative. The slot lives in the
  //    position-independent .dyninstInst region, so an absolute immediate would
  //    be wrong once the loader relocates the code object. Same getpc+add idiom
  //    as emitLongJump: S_GETPC_B64 returns (addr-of-getpc + 4), so the addend
  //    is slotAddr - (currAddr + 4).
  int64_t getpcAddr = (int64_t)gen.currAddr();
  int64_t diff = (int64_t)slotAddr - (getpcAddr + 4);

  emitSop1(S_GETPC_B64, /*dest=*/slotAddrPair, /*src0=*/0,
           /*hasLiteral=*/false, /*literal=*/0, gen);

  // diff -> tgtPair (reg+2, reg+3), then 64-bit add into slotAddrPair.
  E.emitLoadConst(tgtPair, (uint64_t)diff, gen);
  emitSop2(S_ADD_U32,  slotAddrPair,     slotAddrPair,     tgtPair,     gen);
  emitSop2(S_ADDC_U32, slotAddrPair + 1, slotAddrPair + 1, tgtPair + 1, gen);

  // 2. Load the 64-bit callee entry from *slotAddr. size=2 means
  //    S_LOAD_DWORDX2 (2 dwords = 8 bytes); emitLoadRelative emits the
  //    required S_WAITCNT for us.
  E.emitLoadRelative(tgtPair, /*offset=*/0, slotAddrPair, /*size=*/2, gen);

  // 3. Indirect call: PC <- tgtPair, lrPair <- return address (PC+4).
  emitSop1(S_SWAPPC_B64,
           /*dest=*/lrPair,
           /*src0=*/tgtPair,
           /*hasLiteral=*/false, /*literal=*/0, gen);

  rs->freeRegister(block, 4);
}


// ---- per-gen entry points: both gfx9 emitters delegate to the shared implementation above ----
// The call logic is gfx9-family (gfx908 manual flat scratch + gfx942 architected scratch, branched at
// runtime on the KD); GFX9 SALU/SMEM/VOP/FLAT encodings are byte-identical, so routing through either
// emitter is equivalent. gfx942 objects select EmitterAmdgpuGfx940 at runtime, so it needs these too.
Register EmitterAmdgpuGfx908::emitCall(opCode op, codeGen &gen,
    const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands, func_instance *callee) {
  return amdgpuEmitCall(*this, op, gen, operands, callee);
}
Register EmitterAmdgpuGfx940::emitCall(opCode op, codeGen &gen,
    const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands, func_instance *callee) {
  return amdgpuEmitCall(*this, op, gen, operands, callee);
}
bool EmitterAmdgpuGfx908::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
  return amdgpuClobberAllFuncCall(rs, clobbered_functions, callee);
}
bool EmitterAmdgpuGfx940::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
  return amdgpuClobberAllFuncCall(rs, clobbered_functions, callee);
}
