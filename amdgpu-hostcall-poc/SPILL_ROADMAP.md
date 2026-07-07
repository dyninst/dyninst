# Register-Spill Generalization Roadmap

Plan to take the dyninst AMDGPU register-spill/trampoline implementation
(`dyninst/dyninstAPI/src/emit-amdgpu.C`, `EmitterAmdgpuGfx908::emitCall`) from the
**current PoC envelope** to a **general, practically-implementable** state.

## Current state (baseline, 2026-07)
Correct and verified end-to-end (vectoradd `PASSED`, 11/11 deterministic), but only
within the PoC envelope:
- **1 wave** (grid ≤ 64), **full EXEC** at the insertion point, **leaf/nullary**
  callees, **small** register footprints.

Mechanism in place (the right foundation — harden, don't replace):
- Per-wave scratch slot, base in `s[94:95]` (prologue-loaded from kernarg+(ksize-8)).
- SGPR+VCC spill via the **vector pipe** (`v_writelane` pack → `global_store` GLC →
  `global_load` → `v_readlane`) — immune to the callee's `s_dcache_inv`.
- VGPR spill per-lane via `global_store`/`global_load` (GLC).
- `s_waitcnt 0` **before** the save (drains in-flight loads — the blocker fix).
- KD register/scratch grant bump (idempotent; cached original grant).
- Whole-object-max callee footprint (`readCalleeMaxAbsSym`).

"General" = lift each PoC assumption. Ordered so anything that would **corrupt/fault
on a realistic kernel** comes first, then feature-generality, then optimization.

---

## Phase 0 — Correctness on real kernels (MUST-DO)
These break TODAY on ordinary inputs.

### 0.1 Per-wave address via HARDWARE SCRATCH  [M/L]  **(highest priority)**
**Supersedes the old "atomic counter" idea; folds in former 2.2 (non-leaf).**
- **The real requirement:** a per-wave (and per-lane) address we can rely on — NOT
  "avoid corruption via manual slotting." Multiple waves sharing one global buffer is
  a reliability/occupancy problem, and the right primitive already exists in hardware.
- **Insight:** AMDGPU **scratch** (private memory) is hardware-swizzled so lane L of
  wave W at private offset X hits a unique physical address — per-wave AND per-lane for
  free. Cost is occupancy (VRAM reserved per wave), a tradeoff, not corruption.
- **Fix:** ENABLE scratch on kernels that lack it and spill to it via `scratch_store`/
  `scratch_load`. Use **FLAT scratch** (not buffer+V#): `flat_scratch_init` is user-SGPR
  #6 (after kernarg #4) so it can be APPENDED without shifting the kernel's existing
  preloads; `private_seg_buffer` is #1 and would shift kernarg. This deletes the mbcnt
  lane-offset math, the manual `s[94:95]` base, and the atomic-counter plan.
- **Bonus:** the same scratch/ABI setup unlocks non-leaf/scratch-using callees (old
  2.2) — just also set `s32` (stack ptr) before such a call.
- **Steps (see detail below): A** empirical anchor (dump a scratch-using kernel's KD +
  prologue), **B** KD rewrite (enable flat_scratch_init, user_sgpr_count+=2,
  private_segment_fixed_size), **C** prologue codegen (FLAT_SCRATCH init), **D** spill
  via `scratch_*`, **E** `s32` for non-leaf, **F** launcher sets dispatch
  private_segment_size.
- **Still need alongside:** EXEC handling (0.2) — scratch stores from inactive lanes are
  masked, so force EXEC=-1 around the spill to save all lanes. Slot size (0.3) matters
  more (smaller scratch = better occupancy).
- **Risks:** appending flat_scratch_init must not shift kernarg (verify via KD dump);
  ROCr must honor KD scratch flags on an originally-non-scratch object; exact
  FLAT_SCRATCH operands + `scratch_*`/`FLAT_SCRATCH_LO/HI` encodings from Step A.
- **Depends on:** nothing (Step A first).

### 0.2 EXEC-safe spill  [M]
- **Breaks when:** call inserted in a divergent region (narrowed EXEC). Common —
  `hc_write` is inserted every Nth instruction, so it lands in `execz` regions. The
  packed per-lane store/load drops inactive lanes → restore garbage.
- **Fix:** reserve a high SGPR pair (e.g. `s[92:93]`, above the callee clobber range)
  for saved EXEC. Around each save/restore block: `s_mov exec,-1` → spill/fill all 64
  lanes → `s_mov exec, s[92:93]`. Run the call under the chosen EXEC policy (default:
  restore caller EXEC so the probe fires for originally-active lanes).
- **Depends on:** reserve `s[92:93]` (coordinate with
  `maximizeSgprAllocationIfKernel`).

### 0.3 Slot size derived from footprint  [S]
- **Breaks when:** large kernel VGPR count — VGPR area reaches `512 + nvgpr*256`;
  overflows the hardcoded 4 KiB `SLOT_SIZE`.
- **Fix:** mutator computes required slot size from `max(nsgpr,nvgpr)` and emits it
  (symbol/section, e.g. `.dyninst.instslotsize`); launcher reads it instead of
  hardcoding 4096.
- **Depends on:** nothing.

### 0.4 SGPR pack beyond 62  [S]
- **Breaks when:** kernel uses >62 numbered SGPRs (max ~102). One VGPR = 64 lanes;
  VCC takes 2, leaving 62.
- **Fix:** use `ceil((nsgpr+2)/64)` pack VGPRs, loop writelane/readlane; reserve the
  extra scratch VGPR(s).
- **Depends on:** grant bump already reserves scratch VGPRs — extend count.

---

## Phase 1 — Efficiency (stop saving everything)
Every call currently saves the FULL footprint (~34 writelane + 10 VGPR stores, ×2).
Correct but bad for occupancy/perf.

### 1.0 ⚠️ Investigation: is AMDGPU register liveness wired in dyninst?  [S, GATING]
- `arch-amdgpu.h` says "we will use liveness … to allocate them" — may be aspirational.
- Determine whether `LivenessAnalyzer` yields valid live-in/out for AMDGPU registers.
  Decides whether 1.1 is config or a compiler-analysis project. Do BEFORE Phase 1.

### 1.1 Liveness + ABI-based save-set reduction  [M if liveness exists, else L]
- Add caller-saved regmask from the AMDGPU function ABI: caller-saved `s0–s31,
  v0–v39, VCC, s[30:31]`; callee-saved `~s34–s105, v40–v255`.
- Save only `clobbered ∩ caller-saved ∩ live-across-point`. The **ABI half alone**
  (drop callee-saved) is safe with NO liveness and already shrinks the set.
- **Depends on:** 1.0.

### 1.2 Precise per-callee footprint (drop whole-object max)  [M]
- Replace `readCalleeMaxAbsSym` with a real call-graph transitive-closure walk from
  the entry wrapper, so a `gpu_fread`-heavy library doesn't inflate an `hc_write`
  insertion. Exact for our leaf wrappers once done.
- **Depends on:** nothing (independent of liveness).

### 1.3 Refactor + dedup  [S]
- `emitLaneByteOffset()` (3 instrs) recomputed 4× per call; `vAddr` is
  loop-invariant → compute once per trampoline.
- Introduce a **SpillContext** (reserved regs, computed layout, helper emitters) that
  makes 0.2 / 0.4 / 1.1 cleaner to land. Do this FIRST as scaffolding for the rest.

---

## Phase 2 — Feature generality (lift device-lib constraints)
Expand what a hook can be. High value, high cost.

### 2.1 Parameter passing  [M→L]
- **Lifts:** nullary constraint (`emitCall` asserts `operands.empty()`).
- **Fix (incremental):** start with immediate/scalar args — materialize site id / PC /
  buffer pointer into CC arg registers (`s0.., v0..` per `CC_AMDGPU_Func`) after the
  save, before the call. Enough for real instrumentation without full arbitrary-expr
  arg lowering.
- **Depends on:** arg regs already treated as clobbered (caller-saved).

### 2.2 Non-leaf / scratch-using callees  [FOLDED INTO 0.1]
- Now handled by the scratch enablement in 0.1: once FLAT_SCRATCH + the scratch ABI are
  set up, a non-leaf callee works by additionally setting `s32` (stack pointer = wave
  scratch offset) before the call, and sizing `private_segment_fixed_size` to include the
  callee's scratch. See 0.1 Step E.

### 2.3 Return values  [M] — DEFER
- `emitGetRetVal` unimplemented; mailbox covers result return. Lowest ROI.

---

## Recommended sequencing
1. **0.1 multi-wave + 0.2 EXEC** — the two that make it genuinely general; both
   correctness-critical and common. **This is the "make it real" milestone.**
2. **0.3 slot size + 0.4 >62 SGPR** — cheap, close remaining silent bounds.
3. **1.3 refactor + dedup** — scaffolding for the rest (can also land before step 1).
4. **1.0 liveness probe → 1.1 / 1.2** — efficiency once correctness is general.
5. **2.1 args → 2.2 non-leaf** — feature expansion.

## Step A findings (2026-07) — approach pivot to BUFFER scratch
Compared vectoradd (non-scratch) vs a scratch-forcing probe (scratch_probe/):
- vectoradd KD: user_sgpr_count=6; PRIV_SEG_BUFFER=1 (s[0:3]=scratch V# desc),
  KERNARG=1 (s[4:5]), wgid_x=1 (s6); wave_offset(RSRC2 b0)=0; priv_seg=0.
- scratch KD: adds FLAT_SCRATCH_INIT (s[6:7]) + wave_offset (RSRC2 b0 → s9);
  user_sgpr_count=8; wgid_x shifts to s8. Prologue:
  `FLAT_SCRATCH = flat_scratch_init(s[6:7]) + wave_offset(s9)`.
- **KEY:** enabling flat_scratch_init bumps user_sgpr_count 6→8 → SHIFTS the system
  SGPRs (wgid_x s6→s8) → breaks the kernel's wgid reads. So the naive "append
  flat_scratch_init / FLAT scratch" plan is NOT viable without rewriting wgid refs.
- **RESCUE (revised approach = BUFFER scratch, zero SGPR shift):**
  1. The scratch V# descriptor is ALREADY in s[0:3] (HIP enables PRIV_SEG_BUFFER by
     default) — no shift needed to obtain it.
  2. Enabling only private_segment_wave_offset (RSRC2 b0) APPENDS a system SGPR AFTER
     wgid_x (at s7 for vectoradd) — confirmed by scratch kernel's wgid_x=s8 <
     wave_offset=s9 ordering — so it shifts nothing the kernel uses.
- **Revised Steps B–D:**
  - B (KD): set `private_segment_fixed_size` = slot size; set RSRC2 wave_offset bit;
    leave user_sgpr_count / props / flat_scratch_init UNTOUCHED. (RSRC1 grants already
    handled.)
  - C (prologue): at entry, before the kernel clobbers s[0:3], capture the scratch
    descriptor s[0:3] and wave offset s7 into reserved high SGPRs (same trick as
    s[94:95]).
  - D (spill): `buffer_store_dword/buffer_load_dword` with captured descriptor +
    wave-offset soffset → hardware per-wave (soffset) + per-lane (desc swizzle) +
    per-reg (offset). Deletes mbcnt lane math + global buffer.
- **Verify next:** ROCr provides a VALID s[0:3] scratch descriptor once we set
  private_segment_fixed_size on an originally-non-scratch object.
- Probe kept at scratch_probe/ (scratch_probe.cpp/.co) as the anchor.

### Step A refinement — CHOSEN PATH: FLAT_SCRATCH + prologue SGPR-relocation
The compiler's scratch mechanism on gfx908/COV6 is FLAT ops (flat_/scratch_) backed by
FLAT_SCRATCH = flat_scratch_init(s[6:7]) + wave_offset(s9). Enabling flat_scratch_init
shifts system SGPRs (wgid_x s6→s8). CHOSEN approach (lowest-risk; uses ROCr's real
flat_scratch_init, no descriptor/swizzle guess):
- **Accept the shift; the prologue RECORDS/UNDOES the register update.** After the
  hardware preloads with the new layout, the prologue:
  1. `s_add_u32 flat_scratch_lo, s[fsi_lo], s[waveoff]` / `s_addc_u32 flat_scratch_hi,
     s[fsi_hi], 0`  (compiler-exact FLAT_SCRATCH).
  2. For each enabled system SGPR (wgid_x/y/z/info): `s_mov s[old_pos+j], s[new_pos+j]`
     where new_pos = old_pos + (new_uc - old_uc) = old_pos + 2. Relocates them back to
     where the un-shifted kernel expects them.
- KD deltas: props FLAT_SCRATCH_INIT(b5)=1, RSRC2 wave_offset(b0)=1, user_sgpr_count
  += 2, private_segment_fixed_size = slot size. (register grants already handled.)
- Spill via scratch_store/scratch_load off FLAT_SCRATCH (per-lane automatic).
- Encoders needed: scratch_store/load (FLAT enc 0x37, SEG=1), s_add/s_addc to
  flat_scratch_lo/hi (0x66/0x67 — above MAX_SGPR_ID, needs raw SOP2 like the VCC case).
  Verify all by objdump round-trip vs the probe's bytes (80660906 / 82678007).
- Risk retired: ROCr always provides flat_scratch_init for a scratch-requesting KD.
  Remaining verify: ROCr honors hand-set scratch bits on an originally-non-scratch obj.

## Step B–F IMPLEMENTED (2026-07) — scratch spill behind ScratchAbi seam, env-gated
Status: **scratch register-spill path works end-to-end at N=64 (`PASSED`), behind
`DYNINST_SPILL_SCRATCH=1`; global-buffer path remains the default.**
- Seam: `amdgpu-scratch-abi.h` (abstract `ScratchAbi`) + `Gfx908ScratchAbi` in
  emit-amdgpu.C (`waveSize`, `enableScratchInKD`, `emitScratchEntryPrologue`,
  `emitScratchStore/Load`). Later arches (gfx940/gfx11 architected/absolute flat
  scratch) slot in as simpler subclasses.
- Encoders: `scratch_*` = `emitFlatGlobal(..., seg=1)` (verified vs llvm-mc);
  raw `emitSop1Raw/emitSop2Raw` for flat_scratch_lo/hi (102/103) writes.
- KD rewrite (`enableScratchInKD`): flat_scratch_init + wave_offset +
  user_sgpr_count+=2 + private_segment_fixed_size. Verified on vectoradd.inst.co:
  priv_seg=256, user_sgpr_count=8, wave_off=1, FLAT_SCR_INIT=1.
- Prologue (`emitScratchEntryPrologue`): FLAT_SCRATCH = flat_scratch_init +
  wave_offset, then relocate shifted system SGPRs (wgid_x s8->s6) — the "record the
  register update" step. SADDR base s94=0.
- Spill: `scratch_store/load` (SEG=1), per-lane swizzle automatic → dropped mbcnt
  lane math + global buffer + s[94:95] base. vPack still used for SGPR packing.
- Launcher: NO change needed — it already forwards the KD's private_segment_size to
  the dispatch packet, so ROCr backs scratch automatically.
- **Per-wave CONFIRMED**: wave_offset (s9) differs per wave (0x2580000 / 0x2588000 /
  0x258c000, spaced 0x4000 = 256B/lane*64) → FLAT_SCRATCH per-wave. Resolves 0.1.
- Retired risks: ROCr honors hand-set scratch KD; FLAT_SCRATCH setup; SGPR
  relocation; scratch encoding — all validated on-GPU.

### Multi-wave bug ROOT-CAUSED (2026-07) — springboard clobbers the wave_offset SGPR
Symptom: N>64 (multi-wave) fails, 1 wave correct / rest C=0, no fault, deterministic,
IDENTICAL for global and scratch spill. NOT the hostcall (no-op callee still fails),
NOT VCC/EXEC/spill-mechanism. Diagnosis chain (rocgdb, per-lane `info registers v2`):
- wave 1's per-thread INDEX is 0..63 (should be 64..127) → its workitem-id v0 is
  wave 0's → the spill of v0 is SHARED across waves, not per-wave.
- The spill base s[94:95] is the SAME for all waves at the spill. It's built in the
  prologue as `s[94:95] = flat_scratch_init(s[6:7]) + wave_offset(s9)`.
- wave_offset s9 at kernel ENTRY differs per wave (0x2580000 / 0x2584000, stride
  0x4000 = private_seg 256 * 64 lanes) — CORRECT. But at the PROLOGUE s9 = 0x7fff
  for ALL waves — CLOBBERED between entry and prologue.
- **ROOT CAUSE:** dyninst's entry SPRINGBOARD (`s_getpc/s_setpc` long jump) uses a
  4-SGPR block `s[8:11]` chosen by `regSpace->allocateGprBlock` in
  `codegen/codegen-amdgpu.C` generateBranch. For the ORIGINAL kernel s8+ were free,
  but ENABLING SCRATCH shifted the system SGPRs up so `s8=wgid_x`, `s9=wave_offset`.
  The springboard's `s_getpc s[8:9]` overwrites s9 (wave_offset) with PC-high (0x7fff)
  before the prologue reads it → every wave gets the same base → shared spill.
- Special-register note: FLAT_SCRATCH (s102/103) survives the springboard but gfx908
  does NOT pre-init it (compiler always computes it), and the wave_offset source (s9)
  is the only per-wave datum — it's a GP SGPR, hence clobbered.

**Chosen fix = Option B:** stop the springboard from clobbering the live system SGPRs.
`codegen-amdgpu.C` generateBranch must allocate the long-jump block AVOIDING the
scratch-shifted system SGPRs (s8/s9 = wgid/wave_offset) — i.e. reserve them in the
regSpace (entry-specific, scratch-specific) so `allocateGprBlock` picks a higher block
(s[10:13]+). Alternative Option A (rejected): don't enable flat_scratch_init (no shift);
use the private-seg-buffer base s[0:1] (mask s1&0xffff) + wave_offset at s7 (below
s8, so springboard-safe) — needs descriptor masking.
Current code state: prologue builds s[94:95]=fsi+wave_offset via s_add (correct once
s9 survives); spill is unified on the global path (s[94:95]+vAddr). Only the springboard
reg collision remains.

### Remaining for the scratch path
- 0.2 EXEC-force still applies (scratch stores from inactive lanes are masked).
- Make scratch the default once multi-wave validates (post hostcall fix).
- gfx940/gfx11 subclasses (architected/absolute flat scratch — no shift/relocation).

## Validation strategy
Reuse this session's tooling:
- rocgdb per-lane effective-address checker (flags any VMEM access outside known
  buffers; walks the def-use back to the bad register).
- "run ×N, expect deterministic PASSED" harness as the regression gate.
Add:
- **Multi-wave test** (N=256, 4 waves): verify distinct per-wave slots + PASSED.
- **Divergent-EXEC test**: insert inside an `if (tid < K)` region + PASSED.
