/*
 * amdgpu-register-context.h — a single register-boundary description used for BOTH
 * a callee (its entry contract) and a mutatee instrumentation point (what values are
 * available / what is constrained there).
 *
 * Design note (why one type, not "ABI" + "liveness" split): caller/callee-saved is
 * an ABI fact that is ALSO a liveness input — a caller-saved register the callee
 * clobbers must be preserved by the caller IFF it is live; a callee-saved one need
 * not be. So the save-convention is the bridge between the calling convention and
 * the spill decision (spill = caller_saved(callee) ∩ live(point)); splitting them
 * into two types just re-joins them in the marshaller. Hence one RegisterContext,
 * instanced per side, related by a lowering pass.
 *
 * FIRST MIGRATION SCOPE: this header currently models the pieces the implicit-arg
 * path needs — the callee's implicit INPUT slots and the point's ability to supply
 * each implicit source (from the Implicit-Arg Capture Region). Explicit args, the
 * caller/callee-saved spill set, the register budget, and the return value join the
 * same RegisterContext / lowering as those paths migrate (see fields marked TODO).
 */
#ifndef AMDGPU_REGISTER_CONTEXT_H
#define AMDGPU_REGISTER_CONTEXT_H

#include <cstdint>
#include <functional>
#include <vector>

namespace Dyninst {
namespace DyninstAPI {

// Implicit ABI inputs a device function may read — supplied by the caller/ABI, not
// the parameter list. Each maps to a fixed register on gfx908 (see the callee
// contract built by the lowering) and to a capture slot (amdgpu-implicit-args.h).
enum class ImplicitSource : uint8_t {
  None = 0, WorkitemId, WgidX, WgidY, WgidZ, ImplicitArgPtr, KernargPtr, DispatchPtr,
  Count
};

enum class RegClass : uint8_t { SGPR, VGPR };

// Role of a register at a boundary. Encodes the ABI save-convention, which is the
// bridge to liveness (see the design note above).
enum class Role : uint8_t {
  None = 0,
  InputArg,       // explicit tool-provided argument
  InputImplicit,  // implicit ABI input (see RegInfo::implicit)
  Output,         // return value
  CallerSaved,    // clobbered across the call (preserve iff live at the point)
  CalleeSaved,    // preserved by the callee (no spill needed even if live)
  Scratch
};

enum class Live : uint8_t { Unknown = 0, Dead, Live };

// One register's role + status at a boundary.
struct RegInfo {
  RegClass class_ = RegClass::SGPR;
  uint16_t index  = 0;                 // s<index> or v<index>; base of a multi-dword value
  uint8_t  dwords = 1;                 // 1, or 2 for a pointer pair
  bool     uniform = true;             // wave-invariant (SGPR / readfirstlane) vs per-lane (VGPR)
  Role     role   = Role::None;
  Live     live   = Live::Unknown;
  ImplicitSource implicit = ImplicitSource::None;  // when role == InputImplicit
};

// How the mutatee point can MATERIALIZE an implicit source: where it was captured
// (a per-lane byte offset in the Implicit-Arg Capture Region) and any fix-up.
struct SourceLoc {
  bool    available  = false;
  int32_t iacrOffset = 0;    // IACR slot (per-lane byte offset from FLAT_SCRATCH)
  uint8_t dwords     = 1;
  bool    uniform    = true; // uniform -> readfirstlane into an SGPR; per-lane -> straight VGPR load
  int32_t addend     = 0;    // added after load, e.g. ImplicitArgPtr = KernargPtr capture + implicit-block offset
};

// Callee side: the bulk of caller-saved registers the call clobbers, in compact form
// (the named RegInfo slots cover the handful of ABI inputs; this covers the many
// numbered registers). "The call clobbers s0..sgprCount-1, v0..vgprCount-1, vcc, plus
// extraSgpr" — extraSgpr is for caller-saved registers outside the [0,count) footprint,
// e.g. s32 (SP) which the non-leaf call ABI overwrites.
struct ClobberSet {
  uint32_t sgprCount = 0;
  uint32_t vgprCount = 0;
  bool     vcc = false;                 // clobbers vcc_lo/hi (s106/107)
  std::vector<uint16_t> extraSgpr;      // caller-saved SGPRs outside [0,sgprCount)
};

// A single register-boundary description. Instanced for the callee AND the point.
struct RegisterContext {
  // Callee side: its register contract — the named implicit/explicit input slots, and
  // the bulk caller-saved footprint (`clobber`). Explicit args / return / callee-saved
  // join here as those paths migrate.
  std::vector<RegInfo> regs;
  ClobberSet clobber;

  // Point side: how the mutatee supplies each implicit source, and its liveness (for
  // the caller-saved ∩ live spill decision). `live` empty => unknown (save-all).
  SourceLoc sources[(int)ImplicitSource::Count] = {};
  std::function<bool(RegClass, uint32_t)> live;   // is register (class,index) live here?
  bool reduceSgpr = true;   // honor liveness for SGPRs (false = save-all; debug hatch)
  bool reduceVgpr = true;   // honor liveness for VGPRs

  // TODO (join as paths migrate): register/scratch budget headroom (saturation
  // verdict), EXEC state (per-lane correctness), frame info (leaf / private_seg).

  SourceLoc &source(ImplicitSource s) { return sources[(int)s]; }
  const SourceLoc &source(ImplicitSource s) const { return sources[(int)s]; }
};

// The registers that must be preserved across the call = caller-saved(callee) ∩
// live(point). This IS the calling-convention-meets-liveness relation the whole
// abstraction is built around. Order matches the historical spill: s0..sgprCount-1,
// then vcc_lo/hi, then the extra caller-saved SGPRs (deduped).
struct SpillPlan {
  std::vector<uint32_t> sgprs;   // operand ids: 0..n, 106/107 for vcc_lo/hi
  std::vector<uint32_t> vgprs;
};

inline SpillPlan lowerSpill(const RegisterContext &callee, const RegisterContext &point) {
  const ClobberSet &c = callee.clobber;
  auto saved = [&](RegClass cls, uint32_t i, bool reduce) -> bool {
    return !reduce || !point.live || point.live(cls, i);   // no liveness => save-all
  };
  SpillPlan p;
  for (uint32_t i = 0; i < c.sgprCount; i++)
    if (saved(RegClass::SGPR, i, point.reduceSgpr)) p.sgprs.push_back(i);
  if (c.vcc) {
    if (saved(RegClass::SGPR, 106, point.reduceSgpr)) p.sgprs.push_back(106);
    if (saved(RegClass::SGPR, 107, point.reduceSgpr)) p.sgprs.push_back(107);
  }
  for (uint32_t i = 0; i < c.vgprCount; i++)
    if (saved(RegClass::VGPR, i, point.reduceVgpr)) p.vgprs.push_back(i);
  for (uint16_t e : c.extraSgpr) {
    bool have = false;
    for (uint32_t s : p.sgprs) if (s == e) { have = true; break; }
    if (!have) p.sgprs.push_back(e);
  }
  return p;
}

// One marshalling action produced by lowering: place an implicit source into the
// callee's expected register. (Explicit-arg / spill / return steps join as those
// paths migrate.)
struct MarshalStep {
  RegClass dstClass  = RegClass::SGPR;
  uint16_t dstIndex  = 0;
  uint8_t  dwords    = 1;
  bool     uniform   = true;   // SGPR via readfirstlane vs per-lane VGPR load
  int32_t  iacrOffset = 0;     // source slot
  int32_t  addend    = 0;      // pointer adjust after load
};

// Relate a callee contract to a mutatee point: for each implicit input the callee
// requires that the point can supply, emit a step. Skips inputs the point cannot
// provide (e.g. blockDim when the kernel exposes no kernarg pointer) — the caller
// may treat an unmet REQUIRED input as a diagnostic once callees declare need.
inline std::vector<MarshalStep>
lowerImplicitArgs(const RegisterContext &callee, const RegisterContext &point) {
  std::vector<MarshalStep> steps;
  for (const RegInfo &r : callee.regs) {
    if (r.role != Role::InputImplicit)
      continue;
    const SourceLoc &src = point.source(r.implicit);
    if (!src.available)
      continue;
    MarshalStep st;
    st.dstClass   = r.class_;
    st.dstIndex   = r.index;
    st.dwords     = r.dwords;
    st.uniform    = r.uniform;
    st.iacrOffset = src.iacrOffset;
    st.addend     = src.addend;
    steps.push_back(st);
  }
  return steps;
}

}  // namespace DyninstAPI
}  // namespace Dyninst

#endif  // AMDGPU_REGISTER_CONTEXT_H
