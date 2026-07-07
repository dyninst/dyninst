# Hostcall Instrumentation — Session Handoff

End-to-end status of the dyninst→hostcall→host-I/O pipeline. **WORKING END-TO-END
(2026-07):** we statically instrument a GPU code object with dyninst, load it + the
device hostcall library via HSA, run the kernel, GPU→CPU hostcalls reach a host
service thread, `fopen("dyninst_trace.txt")`, all trace lines are written, AND
**vectoradd computes correct results (`PASSED`, 11/11 deterministic runs)** with no
fault. The register-preservation blocker is fixed — see "RESOLVED" below.

## RESOLVED — the register-preservation fault (root cause + fixes)
The kernel faulted mid-run on a wild/nil base pointer. Root cause was NOT the spill
mechanism (nor cache coherence, nor a race — it's a single wave, so nothing races):
dyninst splices the trampoline **between an async `s_load` that fills a register and
the compiler's `s_waitcnt` that guards that register's first use**, so the register
save read STALE, not-yet-loaded values (A/B/C base pointers saved as garbage while an
`s_load_dwordx4` from kernarg was still in flight). Deterministic; hidden by any
debugger halt (the halt lets the load retire). Fixes applied in `emit-amdgpu.C`
`EmitterAmdgpuGfx908::emitCall` (+ helpers):
1. **`s_waitcnt 0` before the register save** — drain in-flight VMEM+scalar loads so
   the save captures settled register values. **This was the actual blocker.**
2. **VGPR transitive under-count** → `readCalleeMaxAbsSym` uses the whole-object max
   (`num_vgpr`/`numbered_sgpr`), covering nested callees (hc_write→gpu_fwrite→…).
3. **VCC preservation** across the call (kernel may hold a live carry: v_add_co …
   <call> … v_addc_co). VCC is s[106:107]; saved/restored with the SGPRs.
4. **Idempotent VGPR grant bump** — cache the caller's ORIGINAL granted VGPR count
   (`g_origGrantedVgpr`) so the grant doesn't ratchet to gran 31/128; now gran 2/12.
5. **SGPR spill via the VECTOR path** (compiler-style: `v_writelane`→`global_store`
   [GLC]→`global_load`→`v_readlane`), not SMEM/K$ — robust vs. the callee's
   `s_dcache_inv`. (Fix #1 alone resolves the fault; this makes the spill sound.)
6. Launcher: instrumentation scratch is now **device-local (coarse-grained) VRAM**
   (`find_coarse_grained`), zeroed via `hsa_amd_memory_fill` — proper spill scratch.
Known limitation: the SGPR pack assumes EXEC is full at the insertion point (true
here); a narrowed EXEC would need EXEC forced to -1 around the packed store/load.

## CONSTRAINT — insertion targets must be LEAF functions (private_seg_size=0)
The inserted wrappers (`hc_open`/`hc_write`/`hc_close`) are **leaf functions**: all
helpers (`hc_first_active_lane`, `hc_acquire`, `hc_call_and_wait`, `hc_release`) are
`inline`, so the wrappers contain NO `s_swappc` — they make no real calls, use only
caller-saved registers (`num_vgpr` small, `numbered_sgpr=32`), and report
**`private_seg_size=0`**. That is *why* the trampoline works without any scratch/stack
ABI setup.

**A non-leaf inserted callee (one that makes a genuine, non-inlined call) is NOT yet
supported.** Per the AMDGPU function ABI, caller-saved = `s0–s31`, `v0–v39`, `VCC`,
and `s[30:31]` (return address); callee-saved = ~`s34–s105`, `v40–v255`. A function
that makes a call must preserve any caller-saved value live across it — and it ALWAYS
has at least its own return address `s[30:31]` live across an inner `s_swappc`. It
preserves these by spilling to its **private stack frame (scratch)** (either directly,
or by first spilling the callee-saved regs it repurposes). Hence any non-leaf function
reports `private_seg_size > 0` and expects a valid stack frame at runtime.

To support such a callee, the trampoline (or prologue) would have to emit the full
**call-ABI setup** that the compiler normally provides — currently NOT emitted
(`bumpCallerKdForCallee` only bumps the KD `private_seg_size` + sets
`EnableSgprFlatScratchInit`, then warns; see `emit-amdgpu.C` ~L604):
- **Scratch descriptor** in `s[0:3]` (private-segment buffer V# — base/stride/num-records/flags),
- **Wave scratch offset / stack pointer** in `s32` (the per-wave scratch base offset),
- **FLAT_SCRATCH** (`flat_scratch_lo/hi`) initialized from the dispatch scratch base + wave offset,
- KD flags: `EnableSgprPrivateSegmentWaveByteOffset` / flat-scratch-init already partly set.
The dispatch packet's `private_segment_size` and the ROCR-provided scratch base feed
these. Until that's implemented, **keep every insertion target leaf (force-inline its
helpers)** — the moment a target needs a stack frame it will read/write unmapped
scratch and fault.

---
(Historical notes below predate the fix.)

## The goal / architecture
Instrument a HIP kernel at the code-object level to inject GPU→CPU **hostcalls** into a
separately-compiled device library, then run it via a custom HSA launcher with a CPU
service thread that services the hostcall mailbox. Flow:

```
instrument.sh (dyninst) : vectoradd.co + hostcall_lib.aliased.elf -> vectoradd.inst.co
launcher (HSA)          : loads both into one hsa_executable, defines `mailbox`, freezes
                          (resolves .dyninst.hc_* cross-object calls), starts CPU service
                          thread, dispatches the kernel; GPU hostcalls hit the mailbox.
```

## Components (all under preload/)
- **hostcall_lib/** — device hostcall library.
  - `hostcalls.h` — shared mailbox ABI (POD; device+host). Fields: ticket_next/now,
    opcode, status, handle, size, retval, path[128]@32, mode[8]@160, data[512]@168.
    Opcodes: HC_OP_FOPEN=1, FREAD=2, FWRITE=3, FCLOSE=4.
  - `hostcall_lib.cpp` — `gpu_fopen/fwrite/fread` + **nullary** wrappers `hc_open/hc_write/
    hc_close` (dyninst can't pass args, so insertion targets must be nullary). Lane election
    is **exec-mask mbcnt** (`hc_first_active_lane`), NOT threadIdx (v31 isn't set up by
    inserted calls). `hc_call_and_wait` has `s_dcache_wb`/`s_dcache_inv` (scalar-cache flush).
  - `Makefile` — genco (`--offload-arch=gfx908 -mcode-object-version=6`, `-Xoffload-linker
    --unresolved-symbols=ignore-all`) → unbundle → `add_object_aliases.py --tables symtab`.
    Produces **hostcall_lib.aliased.elf** (ABI ver 4, flags 0xE30).
- **vectoradd_mutatee/** — `vectoradd.cpp`, `Makefile` (same ABI: `gfx908:sramecc+:xnack-`
  COV6 → 0xE30), extracts `vectoradd.co` via `--genco`. `decode_kd.py` decodes a KD's
  granulated VGPR/SGPR/scratch. `vectoradd.inst.co` = the instrumented output.
- **examples/test_amdgpu_instrument/** — the dyninst mutator (`test_amdgpu_instrument.C`).
  Inserts `hc_open`@kernel-entry, `hc_close`@exit, `hc_write` before every 4th instruction
  (cap 8). `findFunction(..., incUninstrumentable=true)` for call targets. Built via the
  examples CMake project; binary copied to `test_amdgpu_instrument.bin`.
- **launcher/** — `hostcall_launcher.cpp`: HSA loader (defines `mailbox` via
  `hsa_executable_agent_global_variable_define`, loads lib then mutatee, freezes), CPU
  service thread (idempotent open-once/close-at-teardown; dumps FOPEN mailbox bytes for
  debug), allocates the **instrumentation scratch buffer** (HEADER 64 + n_waves*SLOT 4096)
  and writes its pointer to **kernarg+(ksize-8)** (=0x120) which the prologue loads into
  `s[94:95]`. Build: `g++ -O2 -std=c++17 -I../hostcall_lib -I/opt/rocm-7.0.2/include
  -L/opt/rocm-7.0.2/lib -lhsa-runtime64 -pthread`.
- **add_object_aliases.py** — extended: `--asm <file.s>` (inject `.dyninst.<fn>.<key>` ABS
  reg-usage symbols), `preserve_eflags`, `fix_load_segment`, and it's run with `--tables symtab`.
- **instrument.sh** — full instrument+verify (KD bump 8→…, relocs).
- **dyninst/** — patched Dyninst (install prefix **/home/wuxx1279/bin/dynamd**, lib64).

## Build / install / run
```sh
# 1. build+install patched dyninst
cd dyninst/build && make dyninstAPI && make install      # -> /home/wuxx1279/bin/dynamd/lib64

# 2. build the device hostcall library
make -C hostcall_lib                                     # -> hostcall_lib.aliased.elf

# 3. instrument the mutatee
./instrument.sh                                          # -> vectoradd_mutatee/vectoradd.inst.co
# (mutator binary: examples/test_amdgpu_instrument/test_amdgpu_instrument.bin; rebuild via
#  cd examples && cmake . -B <bld> -DDyninst_DIR=/home/wuxx1279/bin/dynamd/lib64/cmake/Dyninst
#  && cmake --build <bld> --target test_amdgpu_instrument )

# 4. run the launcher
cd launcher
/bin/cp ../vectoradd_mutatee/vectoradd.inst.co .         # NOTE: /bin/cp — `cp` is aliased to -i
/bin/cp ../hostcall_lib/hostcall_lib.aliased.elf .
g++ -O2 -std=c++17 -o hostcall_launcher hostcall_launcher.cpp -I../hostcall_lib \
    -I/opt/rocm-7.0.2/include -L/opt/rocm-7.0.2/lib -lhsa-runtime64 -pthread
ROCR_VISIBLE_DEVICES=1 ./hostcall_launcher vectoradd.inst.co hostcall_lib.aliased.elf
cat dyninst_trace.txt

# debug on-GPU:
ROCR_VISIBLE_DEVICES=1 /opt/rocm-7.0.2/bin/rocgdb -batch -ex "set pagination off" \
  -ex run -ex "printf \"pc=%#lx\n\", $pc" -ex "x/4i $pc-8" -ex "p/x $v0" \
  --args ./hostcall_launcher vectoradd.inst.co hostcall_lib.aliased.elf
```

## WORKING (validated on-GPU via rocgdb)
1. Cross-object linking + trampoline branch (`.dyninst.hc_*` resolved from the co-loaded lib).
2. Lane election via exec-mask mbcnt (NOT threadIdx/v31).
3. Return-address ABI: inserted call uses **s[30:31]** (`emitCall` in emit-amdgpu.C).
4. `BPatch_callBefore` at exit enabled for AMDGPU (BPatch_point.C, arch-gated) → `hc_close`.
5. KD register-grant bump from the callee's exported footprint (`bumpCallerKdForCallee`).
6. **SGPR** save/restore around the call (kernarg `s[4:5]` preserved) — via SMEM to `[s94:95]`.
7. **Hostcall marshaling**: `hc_open` → host `fopen('dyninst_trace.txt','w')` succeeds.
8. **VGPR** per-lane save/restore encoders (VOP2/VOP3A/FLAT `global_store/load`) built +
   emitted; `v0..v3` round-trip through `[s94:95+512+i*256+lane*4]`.
9. `add_object_aliases.py` produces valid objects (`.rodata` loads, e_flags 0xE30).
Result: **dyninst_trace.txt gets the fopen + 6 `[gpu] function entered` lines** before the fault.

## REMAINING BUG (the one blocker)
The kernel faults after ~6 writes on a vectoradd body load `global_load_dword v4, v[4:5]`.
At the fault, the A-address high half **v5 = 0x7fff**, which equals **s95** (the scratch-base
high, since `s[94:95]=0x7ffff7e98000`). So a kernel address register is contaminated with the
scratch-base value across the interleaved hostcalls → wild address → page fault.
Also: the VGPR **grant overshoots to 128 (gran 31)** — the per-call bump accumulates because
`vAddr` is derived from the *growing* granted count.

### Root cause (hypothesis, needs confirming)
- The save set is the **callee's** clobber count (`v0..v(num_vgpr-1)`=v0..v3; SGPR s0..s(numbered_sgpr)),
  but what must be preserved is the **caller kernel's LIVE registers across the call**. A live
  *address* register (or the SGPR base pointer feeding it) isn't in the saved set, OR is being
  aliased with the scratch base. `v5 == s95` strongly implicates an SGPR base-pointer register
  getting the scratch-base value — i.e. an SGPR-spill/restore or register-numbering mixup, not
  the VGPR spill mechanics (the emitted mbcnt/global_store use v88 correctly).
- The `vAddr` scratch reg is computed from `readCallerGrantedVgpr` (the *mutable* grant), so the
  grant bump is not idempotent → accumulates to 128.

### Next steps to fix
1. **Idempotent grant bump / stable vAddr.** In `bumpCallerKdForCallee` (emit-amdgpu.C ~L555)
   and `emitCall` (~L681), `vAddr = max(kernelVgpr, nvgpr)` where kernelVgpr = current granted
   VGPR. Because the bump raises the grant, the next call reads a bigger grant → vAddr grows →
   grant grows (→128). Fix: derive vAddr from the kernel's **original** VGPR usage (read once /
   cache per function), and make the bump set an absolute target, not `current+1`.
2. **Preserve the kernel's live set, not the callee's clobber count.** Determine which regs are
   live across each insertion point (dyninst liveness) OR conservatively save the kernel's full
   footprint (vgpr_count + numbered_sgpr). The vectoradd address regs (v[2:3], v[4:5]) and the
   base-pointer SGPRs must survive. Confirm exactly which register holds A_base_high and trace
   where `0x7fff` (=s95) enters it: disassemble the trampoline around the faulting load
   (`disassemble $pc-0x40,$pc+8` in rocgdb on vectoradd.inst.co) and follow the def-use of v5.
3. **SGPR spill via vector path (robustness).** Per the scalar-cache finding, SGPR spill via
   SMEM (`emitStoreRelative`) "works" only by K$ retention; LLVM spills SGPRs through the vector
   path (v_writelane/v_readfirstlane + global_store). The `v5==s95` symptom may be an SMEM SGPR
   spill coherency/aliasing issue — reworking SGPR spill to the vector path (now that FLAT/VOP
   encoders exist) may fix it directly.
4. **Per-wave offset (deferred).** Single-wave only right now. For multi-wave, add the prologue
   per-wave offset to `s[94:95]` (atomic counter in the buffer header; see the design discussion
   — HW_ID is unsafe under CWSR preemption, use a logical/atomic id, base=buffer+HEADER+id*SLOT).

## Hard-won gotchas (don't re-discover these)
- **Device: `ROCR_VISIBLE_DEVICES=1`** — MI100/gfx908 is device 1 (device 0 is gfx900); a
  gfx908-only fatbin fails to launch (`invalid device function` / "no code objects for gfx900")
  even after hipSetDevice(1).
- **ABI must match exactly** across mutatee + lib: `--offload-arch=gfx908:sramecc+:xnack-`
  (explicit; generic `gfx908` drifts) + `-mcode-object-version=6` → ELF ABI ver 4, flags **0xE30**.
- **`add_object_aliases.py` yaml round-trip corrupts the ELF** (LLVM 15 obj2yaml/yaml2obj):
  drops sramecc e_flag (→`preserve_eflags`) and orphans `.rodata` from PT_LOAD (→`fix_load_segment`
  + **`--tables symtab`** so `.dynsym` doesn't grow and RO sections stay congruent). Needs
  `import struct`. Symptom of the .rodata bug: GPU reads string constants as zeros (empty fopen path).
- **Loader resolves `.symtab` (STT_OBJECT), ignores `.dynsym`** → `--tables symtab` suffices.
- **`cp` is aliased to `-i`** → use `/bin/cp` when staging into launcher/.
- **Lane election**: use exec-mask mbcnt, not `threadIdx.x` (v31 workitem-id reg is NOT set up by
  a dyninst-inserted call).
- **Return address** for the inserted `S_SWAPPC` must be **s[30:31]** (the ABI RA reg).
- **Device functions strip their `.num_vgpr`/`.numbered_sgpr` symbols**; we re-inject them via
  `add_object_aliases.py --asm hostcall_lib.s`. dyninst reads them via `readCalleeAbsSym`.
- **Scalar cache**: wave-uniform stores go through K$ and aren't flushed by `__threadfence_system`;
  `s_dcache_wb` is needed (added to hc_call_and_wait). (Turned out the empty-path was the .rodata
  bug, not this — but the scalar-cache issue is real and relevant to the SGPR-spill rework.)

## Key patched-dyninst locations
- `emit-amdgpu.C`: `emitCall` (link reg s30:31, SGPR spillFill, VGPR vgprSpill),
  `bumpCallerKdForCallee`, `readCalleeAbsSym`, `readCallerGrantedVgpr`, `emitIndirectCall`.
- `amdgpu-gfx908-details.{h,C}`: `emitVop2`/`emitVop3a`/`emitFlatGlobal` + opcode enums.
  Encodings (ISA-cited): FLAT enc 0x37, SEG=2, GLOBAL_LOAD_DWORD=0x14, GLOBAL_STORE_DWORD=0x1C;
  VOP2 V_LSHLREV_B32=0x12; VOP3A enc 0x34, V_MBCNT_LO=0x28C, V_MBCNT_HI=0x28D; VOP1 V_MOV_B32=1.
  Operand codes: VGPR src=256+n, inline 0=128, -1=193, 2=130; VDST/FLAT addr/data are plain VGPR#.
- `BPatch/BPatch_point.C`: `BPatchToInternalArgs` — AMDGPU-gated allow of callBefore@exit.
- `AmdgpuPointHandler.C`: `maximizeSgprAllocationIfKernel` (maxes SGPR grant), the prologue
  (`s_load_dwordx2 s[94:95], s[4:5], 0x120`).
</content>
