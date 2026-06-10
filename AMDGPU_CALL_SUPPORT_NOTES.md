# AMDGPU emitCall / external-linkage session notes

Self-contained handoff so a fresh session can pick up without replaying the
discussion. Scope: enabling `emitCall` for AMDGPU when the callee lives in a
different mapped object (i.e. external/inter-module calls), via a GOT-slot +
S_SWAPPC_B64 pattern, plus the surrounding plumbing needed to make the path
reachable end-to-end.

Worktree: `/home/wuxx1279/dyninsts/worktree/claude`
Branch: `claude` (off `master`).

---

## Goal

Make `EmitterAmdgpuGfx908::emitCall` work for the inter-module case so that
when a snippet contains a call to a function defined in a different module:

1. Dyninst allocates a per-callee 8-byte slot in the rewritten binary and
   registers a dynamic relocation against the callee symbol (so the loader
   fills the slot at load time).
2. The emitted code loads the resolved callee address out of the slot via
   SMEM and jumps to it with `S_SWAPPC_B64` (AMDGPU's atomic call: PC swap
   plus return-address deposit into a destination SGPR pair).

This mirrors the GOT/PLT slot + indirect-call pattern x86/aarch64/ppc already
use through `Emitter::getInterModuleFuncAddr` (see
`dyninstAPI/src/codegen/emitters/x86/Emitterx86.C:332`,
`dyninstAPI/src/inst-aarch64.C:1128`, `dyninstAPI/src/inst-power.C` for
prior art).

---

## Background — codeGen / insnCodeGen / Emitter layering

Brief reminder of the architecture in `dyninstAPI/` so the changes make sense:

- **`codeGen`** (`dyninstAPI/src/codegen.h:85`, `codegen.C`) — arch-independent
  buffer + context (`buffer_`, `offset_`, plus pointers to `AddressSpace`,
  `registerSpace`, `regTracker_t`, `instPoint`, `func_instance`,
  `baseTramp`, target start `Address`, and `Emitter*`). The carrier threaded
  through every `emit*` call as `codeGen &gen`.
- **`insnCodeGen`** (`codegen-x86.h:52`, `codegen-power.h:43`,
  `codegen-aarch64.h:44`, `codegen-amdgpu.h:43`) — per-arch class of static
  helpers (`generateBranch`, `generateCall`, `generateNOOP`, `generateTrap`,
  `generateIllegal`, `modify*`, …) that encode concrete machine instructions
  into a `codeGen` buffer.
- **`Emitter`** (`dyninstAPI/src/emitter.h:48`) — abstract interface with
  `virtual` higher-level ops (`emitCall`, `emitLoad/Store`, `emitImm`,
  `emitR/V/A/Vload/Vstore`, …). Concrete subclasses
  (`Emitterx86`/`EmitterIA32`/`EmitterAARCH64`/`EmitterPOWER`/`EmitterAmdgpuGfx908`)
  delegate down to per-arch `insnCodeGen` to actually write bytes.

A long 2026 refactor by Tim Haines (PRs #2207–#2251) has been pulling
arch-specific `emit*` from free functions in `inst.h`/`inst-*.C` into
`Emitter` virtuals; see `git log --grep=Emitter` for the trajectory.

For AMDGPU specifically, the relevant SOP1 opcodes (`amdgpu-gfx908-details.h:94-96`):

```
S_GETPC_B64  = 28   D[sdst:sdst+1] <- PC + 4
S_SETPC_B64  = 29   PC             <- S[ssrc0:ssrc0+1]
S_SWAPPC_B64 = 30   D[sdst:sdst+1] <- PC + 4
                    PC             <- S[ssrc0:ssrc0+1]
```

`emitSop1(opcode, dest, src0, hasLiteral, literal, gen)` already handles any
SOP1, so `S_SWAPPC_B64` works with no new encoder wiring.

Free SGPR/VGPR allocation in `registerSpace`:
- `gen.rs()` carries the `registerSpace*`.
- `rs->allocateGprBlock(RegKind::SCALAR, numRegs, NS_amdgpu::PAIR_ALIGNMENT)`
  returns a pair-aligned consecutive block.
- `rs->freeGprBlock(block)` releases it.
- See `registerSpace-amdgpu.C:57` for the AMDGPU implementation, and
  `codegen-amdgpu.C:60-83` for the existing usage inside
  `insnCodeGen::generateBranch` (the 4-reg allocation that `emitLongJump`
  expects).

Note on `registerSpace` membership for AMDGPU: both SGPRs and VGPRs are
stored in `GPRs_` (see `registerSpace.C:170-181`, where the `SGPR`/`VGPR`
cases fall through into the `GPR` case). `numFPRs()` returns 0.

---

## Changes made in this session

Five files touched (one new). All edits already applied to the working tree.

### 1. `dyninstAPI/src/inst-amdgpu.C` — implement `getInterModuleFuncAddr`

Added includes: `Architecture.h`, `addressSpace.h`, `binaryEdit.h`,
`function.h`, `Symbol.h`, plus `<cstdlib>`, `<cstring>`.

Replaced the `assert(!"Not implemented")` stub with the slot + relocation
implementation, mirroring `Emitterx86::getInterModuleFuncAddr`
(`codegen/emitters/x86/Emitterx86.C:332`):

- `gen.addrSpace()->edit()` to get `BinaryEdit*`; **asserts** if null
  (live-process indirect call is not supported yet — deliberate scope cap).
- `func->getRelocSymbol()` for the symbol the dynamic linker will resolve.
- `binEdit->getDependentRelocationAddr(referring)` to reuse an existing
  slot for the same symbol (one slot per external callee, shared across
  call sites).
- Otherwise `inferiorMalloc(jump_slot_size)` (8 bytes on AMDGPU per
  `getArchAddressWidth(Arch_amdgpu_gfx908)`), zero-initialise via
  `writeDataSpace`, then `addDependentRelocation(slot, referring)`.

Result: returns the absolute address of the new (or reused) slot.

### 2. `dyninstAPI/src/emit-amdgpu.h` — declarations

Added:
- `#include "dyninstAPI/src/function_cache.h"` (for the new
  `clobbered_functions` member).
- Public method declaration `emitIndirectCall(Address slotAddr, Register lrPair, codeGen &gen)`
  next to `emitLongJump`, with comment explaining that the caller owns
  `lrPair`.
- Private member
  `Dyninst::DyninstAPI::function_cache clobbered_functions;` mirroring
  `emit-x86.h:147`, `emit-aarch64.h:204`, etc.

### 3. `dyninstAPI/src/emit-amdgpu.C` — implementations

Added `#include "function.h"`.

**`emitIndirectCall`** (defined next to `emitLongJump`):
- Allocates a 4-SGPR pair-aligned scratch block via `gen.rs()->allocateGprBlock`.
- Slices the block into `slotAddrPair` (reg+0, reg+1) and `tgtPair`
  (reg+2, reg+3) by constructing `Register(OperandRegId(...), RegKind::SCALAR, BlockSize(2))`.
- Sequence:
  1. Compute the slot's runtime address **PC-relative** (the `.dyninstInst`
     region is position-independent, so an absolute immediate would be wrong
     after the loader relocates the code object — same getpc+add idiom as
     `emitLongJump`):
     - `getpcAddr = gen.currAddr()`; `diff = slotAddr - (getpcAddr + 4)`
       (`S_GETPC_B64` returns the address of the *next* instruction).
     - `emitSop1(S_GETPC_B64, dest=slotAddrPair, ...)`.
     - `emitLoadConst(tgtPair, diff)` then `S_ADD_U32` / `S_ADDC_U32` to add
       the 64-bit `diff` into `slotAddrPair`.
  2. `emitLoadRelative(tgtPair, /*offset=*/0, slotAddrPair, /*size=*/2, gen)`
     — `S_LOAD_DWORDX2` (size is in dwords, 2 = 8 bytes). `emitLoadRelative`
     emits the required `S_WAITCNT` itself.
  3. `emitSop1(S_SWAPPC_B64, dest=lrPair.getId(), src0=tgtPair.getId(), false, 0, gen)`.
- Frees the scratch block.
- **History:** originally materialised the slot's *absolute* address via
  `emitLoadConst(slotAddrPair, slotAddr)`. That was wrong for PIC code objects
  — fixed to the PC-relative getpc+add sequence above.

**`emitCall`** (replaces stub at the original location):
- Asserts `op == callOp` and `callee != nullptr`.
- Asserts `operands.empty()` — no calling convention pinned down for
  argument passing; explicit TODO marker.
- External-call detection: `caller = gen.func(); isExternal = caller && (caller->obj() != callee->obj());`.
  This is the same idiom as `EmitterIA32Stat::emitCallInstruction`
  (`codegen/emitters/x86/IA32/EmitterIA32Stat.C:34`).
- If **not external**: `assert(!"AMDGPU emitCall: intra-module direct call not implemented yet")`.
  Future work would route through `insnCodeGen::generateBranch` /
  `emitLongJump` for a PC-relative direct call.
- If **external**:
  - Allocate a 2-SGPR pair-aligned link-register pair via `allocateGprBlock`.
  - `Address slot = getInterModuleFuncAddr(callee, gen);`
  - `emitIndirectCall(slot, lrPair, gen);`
  - `rs->freeGprBlock(lrPair);`
  - Returns `Null_Register` (no return-value convention pinned).

**`clobberAllFuncCall`** (replaces stub at `emit-amdgpu.C:576`):
- Null-check, memoize via `clobbered_functions`.
- Conservative: marks every `rs->GPRs()[i]->beenUsed = true` (covers both
  SGPRs and VGPRs because of the registerSpace layout note above).
- Uses the same inverted-cache idiom every other emitter uses — see
  "Pre-existing bugs" below.

### 4. `symtabAPI/src/relocationEntry-elf-amdgpu.C` — **new file**

ELF arch-specific specialisation. Stock `<elf.h>` from glibc does **not**
define `R_AMDGPU_*` (confirmed: `grep R_AMDGPU /usr/include/elf.h` returns
nothing); `EM_AMDGPU` does exist and is already used at
`dwarfHandle.C:206`, `Object-elf.C:280`, `Elf_X.C:1850`.

Provides:
- `R_AMDGPU_*` constants (gated by `#ifndef R_AMDGPU_NONE` so they don't
  collide once libc catches up). Values are the LLVM/AMDGPU-ABI canonical
  numbers (0..13).
- `relType2Str` covering all of them.
- `getGlobalRelType(addressWidth, sym)` returning `R_AMDGPU_ABS64` (absolute
  64-bit reloc — what the loader needs for the slot).
- `getCategory(addressWidth)` mapping `R_AMDGPU_RELATIVE64 -> relative`,
  everything else -> `absolute`.

### 5. `symtabAPI/CMakeLists.txt` — wire the new file in

Inserted an `elseif(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)` branch ahead of the
`else()` stub fallback (line ~85). Pulls in
`src/relocationEntry-elf-amdgpu.C`, keeps `src/emitElfStatic-stub.C` for
the static-emission piece (no AMDGPU `emitElfStatic` exists yet — known gap).

### 6. `symtabAPI/src/Symtab.C` — retarget the external reference for AMDGPU

`Symtab::addExternalSymbolReference` (`Symtab.C:2111`) builds the placeholder
symbol the relocation resolves against. This evolved across the session as the
runtime behaviour was understood (see the "MAJOR FINDING" bullet under Design
choices). **Final form for the three AMDGPU arches:**

- **Type → `ST_NOTYPE`** (→ `STT_NOTYPE`, `emitElf.C:130`). The ROCr loader
  only does a name-based cross-module lookup (`agent_symbols_`) for
  `STT_NOTYPE` relocation symbols; `STT_FUNC`/`STT_OBJECT` resolve *locally* to
  `segment_base + st_value` (an `UND` placeholder → a segment base, the wrong
  value originally observed).
- **Name → `.dyninst.<mangled>`** (e.g. `funptr1` → `.dyninst.funptr1`). The
  loader never registers `STT_FUNC` symbols, so the bare callee name is never
  resolvable. A separate build step exports, per callee, an `STT_OBJECT`
  `.dyninst.<callee>` aliasing the entry address; the relocation is pointed at
  that name so the loader's `agent_symbols_` lookup hits.
- Applied to **all three** of: the `.dynsym` placeholder (`symRef`), the
  `.symtab` mirror (`symtabRef`), and `localRel.setName(refName)` — the last so
  `emitElf::createRelocationSections` resolves the reloc's `r_info` via
  `dynSymNameMapping[".dyninst.<callee>"]` (`emitElf.C:1863`). Other platforms
  keep `externalSym->getType()` / bare mangled name (gated by
  `switch (getArchitecture())`).

`.symtab` mirror: `isInSymtab()` / `isInDynSymtab()` are mutually exclusive on
one `isDynamic_` flag (`Symbol.h:164-165`), so a single Symbol can't be in both
tables — hence the separate static (`isDynamic=false`) object. The reloc itself
resolves against `.dynsym`; the mirror is a hedge against the loader's
registration pass (`AmdHsaCode::PullElf`) reading a different table — see
open-item below.

**Verify on a real build:**
- `readelf --dyn-syms`: `.dyninst.funptr1` present as `NOTYPE GLOBAL UND`;
  `--relocations`: `R_AMDGPU_ABS64 … .dyninst.funptr1 + 0`.
- Slot value at load time = `funptr1`'s real entry (no longer a segment base).
  Requires the provider's `.dyninst.funptr1` `st_value` to alias the function
  entry; if it instead holds the *address of a pointer*, `emitIndirectCall`
  needs one extra load.
- Which table `PullElf::getSymbolTable()` reads (`.dynsym` vs `.symtab`) —
  decides whether the `.symtab` mirror is load-bearing or removable. **Not yet
  traced.**
- Provider + consumer must load into the **same `hsa_executable_t`**
  (`agent_symbols_` is per-executable); the consumer's `DT_NEEDED` is not
  auto-followed by the HSA loader.

---

## Design choices and rationale

- **`S_SWAPPC_B64` over `S_GETPC_B64` + arithmetic + `S_SETPC_B64`.**
  One instruction does the atomic PC swap and link-register write, removing
  a race window and the offset-arithmetic sync with the load sequence size.
  Was an option from the start; the user surfaced it after we initially
  sketched the GETPC/SETPC path.

- **⚠️ MAJOR FINDING (runtime debug): the ROCr loader does not resolve
  cross-code-object FUNCTION symbols — the whole slot+reloc-against-funptr1
  design cannot work as-is.** Verified against ROCr source in
  `~/sources/aomp-git/aomp21.0/rocr-runtime/runtime/hsa-runtime`. Symptom:
  the slot was filled with `0x6fffe8c00000` (a segment base) instead of
  `funptr1`'s address `0x7ffff7ec1320`.

  Evidence chain:
  1. `ExecutableImpl::ApplyDynamicRelocation` (`loader/executable.cpp:1844`)
     branches on `rel->symbol()->type()`:
     - `STT_FUNC`/`STT_OBJECT`/`STT_AMDGPU_HSA_KERNEL`: address is computed
       *locally* as `VirtualAddressSegment(st_value)->Address(st_value)`. For
       our `UND` `funptr1` (`st_value==0`) that's `segment(VA 0).base + 0` =
       the original code object's first LOAD segment base
       (`0x6fffe8c00000`). The `.dyninstInst` segment loads at a *different*
       base (`0x7fffe8c00000`) — hence the bit-40 difference. `ABS64` then
       writes this non-zero value with **no error** → silent wrong slot.
     - `STT_NOTYPE`: the **only** name-based path —
       `agent_symbols_.find({name, agent})`.
  2. `AmdHsaCode::PullElfV2` (`libamdhsacode/amd_hsa_code.cpp:1799`, same in
     V1 @325) classifies symbols: only `STT_AMDGPU_HSA_KERNEL`, `STT_OBJECT`,
     `STT_COMMON` are kept; **`STT_FUNC`/`STT_NOTYPE` are skipped**
     (`default: break;`). So a defined `STT_FUNC funptr1` in the provider
     `.hsaco` is never registered into `agent_symbols_`/`program_symbols_`
     (`LoadDefinitionSymbol`, `executable.cpp:1442`).

  Consequence: the loader tracks only kernels + variables for cross-module
  resolution, and only resolves `STT_NOTYPE` relocation symbols by name.
  A function symbol is neither registered (provider) nor name-resolved
  (consumer). Our reloc is `R_AMDGPU_ABS64` against `STT_FUNC funptr1`
  (`getGlobalRelType` returns `R_AMDGPU_ABS64`, and the placeholder inherits
  `externalSym->getType()` = function).

  **Viable redesign** — route through the variable mechanism, the one cross-
  module path that exists:
  - Provider (`funptr1_gfx908.hsaco`): export an `STT_OBJECT` agent-allocated
    variable holding `funptr1`'s entry address (so it lands in
    `agent_symbols_` when loaded).
  - Consumer (rewritten object): emit the slot reloc as `R_AMDGPU_ABS64`
    against an `STT_NOTYPE` *undefined* symbol of that variable's name. The
    `STT_NOTYPE` branch then resolves it by name. (If unregistered, `symAddr`
    stays 0 → `ABS64` hits the `if(!symAddr)` guard →
    `HSA_STATUS_ERROR_VARIABLE_UNDEFINED`: a loud, diagnosable error rather
    than today's silent wrong value.)
  - **Implemented** in `addExternalSymbolReference` — see §6 above: placeholder
    + reloc forced to `ST_NOTYPE` and retargeted to `.dyninst.<callee>` to match
    the provider's exported `STT_OBJECT` alias.

- **Slot + dynamic reloc over an absolute immediate.**
  Same reasoning as PLT/GOT on host ELF: the rewritten binary can't bake
  in absolute target addresses for symbols defined in other modules — the
  loader is the only authority. `inferiorMalloc` reserves space in the new
  `.dyninstInst` region; `addDependentRelocation` records the
  `slot → symbol` pair; `BinaryEdit::finalize` (`binaryEdit.C:562-571`)
  walks dependentRelocations and calls
  `symObj->addExternalSymbolReference(referring, newSec, localRel)` with
  `relocationEntry::getGlobalRelType(addressWidth, referring)`. That's why
  the symtab-side change is necessary — without it AMDGPU falls through to
  the stub `getGlobalRelType` which returns `relocationEntry::dynrel` (= 2),
  not a real ELF reloc type.

- **Conservative `clobberAllFuncCall`.** Without an AMDGPU calling
  convention pinned down, the only safe choice is "callee may write any
  caller-saved SGPR/VGPR." That's the same shape as the AArch64 non-leaf
  branch (`inst-aarch64.C:422-426`). Narrowing this requires defining the
  convention first.

- **Inter-module path only, intra-module deferred.** Keeps the change
  focused on the slot+relocation+SWAPPC mechanism. The intra-module case
  collapses to `insnCodeGen::generateBranch` already; wiring `emitCall` to
  it is a separate concern.

- **Asserts over silent fallbacks.** Live-process path
  (`AddressSpace::edit() == nullptr`), intra-module call,
  argument passing, return-value convention — all explicit asserts. We
  prefer "fail loudly at the right spot" over compiling but producing wrong
  code. Each assert is the next bite for a future commit.

---

## Open items / known limitations

Things to verify or extend the next time someone actually builds and runs
this on an AMDGPU-configured tree:

1. **`emitElfStatic-stub` may not honour `addExternalSymbolReference`
   end-to-end for AMDGPU.** `BinaryEdit::finalize` only walks the dependent
   relocations when `mobj == getAOut()` (`binaryEdit.C:560`). If the stub
   skips the relocation section entirely on AMDGPU, the slot in the
   output binary stays zero and the indirect call dereferences null. **First
   place to check if the chain silently fails.** Likely needs an
   `emitElfStatic-amdgpu.C` mirror of `emitElfStatic-aarch64.C`.

2. **`.dyninstInst` region reachability under the AMDGPU loader.** The slot
   lives in a `RT_TEXTDATA` region (`binaryEdit.C:550-554`). Need to confirm
   that the AMD code-object loader actually maps that region and that SMEM
   loads can read it. For HSA code objects the data layout may diverge from
   host ELF; if it does, a separate writable region for slots would be the
   workaround.

3. **`getRelocSymbol` failure mode.** Asserts at `function.C:632` if the
   callee has no symbols. External callees always do, but if the path gets
   exercised with a callee Dyninst synthesised internally (e.g. a wrapper
   it created), this will fire. Currently we let it; consider a clearer
   error if it shows up in practice.

4. **gfx90a / gfx940 not wired.** `Architecture.h` calls these "future
   support". This commit only handles `DYNINST_CODEGEN_ARCH_AMDGPU_GFX908`,
   matching the existing scope in `dyninstAPI/CMakeLists.txt:311`. Same
   gating in the new symtab CMake branch.

5. **Calling convention not pinned down.** `emitCall` asserts on non-empty
   `operands` and returns `Null_Register`. To actually pass arguments and
   return values:
   - Decide caller-saved vs callee-saved SGPR/VGPR split.
   - Decide which SGPR pair is the link register (currently chosen
     dynamically from the free pool; needs to be ABI-stable so the callee
     can find it).
   - Decide argument SGPRs/VGPRs.
   - Tighten `clobberAllFuncCall` to mark only the actual caller-saved set.

6. **Intra-module direct call not implemented.** Should be a thin wrapper
   over `insnCodeGen::generateBranch(gen, from, to, /*link=*/true)`, which
   in turn uses `emitLongJump` when the displacement doesn't fit. Probably
   should route through `S_SWAPPC_B64` too (with a saved PC) if you want
   the call/return convention to be uniform; or stick with the GETPC/SETPC
   pattern `emitLongJump` already uses if you treat it as a tail-call.

7. **`function_cache::contains` is inverted (pre-existing).**
   `function_cache.h:46-52`:
   ```cpp
   bool contains(func_instance *f) const {
     auto cmp = ...;
     return std::find_if(funcs.begin(), funcs.end(), cmp) == funcs.end();
   }
   ```
   That's "returns true when not found." Every existing `clobberAllFuncCall`
   uses it as `if (contains(callee)) return true;` which under that body
   actually means "skip when not yet cached, do the work when cached" —
   exactly backwards. AMDGPU mirrors the same usage so the eventual fix is
   a one-place change in `function_cache.h`. **Don't fix it in this commit
   — fix it once across all five emitters.**

8. **Recursion guard requires `emitCall` to return a register (blocks any
   func-call snippet).** Symptom observed: instrumenting a snippet that calls
   `funptr1` (callee in a separate code object) fails with
   `[operatorAST.C: 273] ERROR: returned register invalid`.

   Chain: a snippet that `containsFuncCall()` makes `baseTramp::generateBT`
   wrap the whole instrumentation in the recursion guard
   (`baseTramp.C:376-380`):
   ```
   If( DYNINST_lock_tramp_guard(),        // loperand = condition
       <set guard; call funptr1; clear guard> )
   ```
   The `If` condition is a `functionCallAST`. `operatorAST`'s `ifOp` case does
   `REGISTER_CHECK(src1)` on the condition's result (`operatorAST.C:273`).
   `functionCallAST::generateCode_phase2` returns `true` but leaves
   `retReg = Null_Register` whenever `emitCall` returns `Null_Register`
   (`functionCallAST.C:92-98`) — and AMDGPU `emitCall` always does
   (`emit-amdgpu.C:492`, see item #5). So the guard condition has no register
   to test and `REGISTER_CHECK` bails. This is **not** a fault in the
   inter-module slot/SWAPPC mechanism; it's the guard needing a value-returning
   call. (Compounding: `DYNINST_lock_tramp_guard` /
   `DYNINST_unlock_tramp_guard` live in `libdyninstAPI_RT`, which has no GPU
   presence, so the guard can't resolve on AMDGPU even if it generated.)

   - **Immediate unblock (no code change):** `bpatch.setTrampRecursive(true)`
     before inserting. That calls `instance->disableRecursiveGuard()`
     (`BPatch_addressSpace.C:965-966`), so `guarded()` returns false and
     `generateBT` takes the `else` branch (`baseTramp.C:382-385`) — no `If`
     wrapper, the call is emitted in sequence and its null return is harmless.
   - **Recommended code fix (#1):** suppress guards for AMDGPU unconditionally
     — one arch gate in `baseTramp::guarded()` (`baseTramp.C:539`, alongside
     the existing `if (suppressGuards) return false;`). Lowest-risk, unblocks
     every func-call snippet, and is correct because the guard runtime doesn't
     exist on GPU. Chosen over fix #2 for now.
   - **Real long-term fix (#2):** give `emitCall` a return register (move the
     ABI return SGPR into an allocated reg after `S_SWAPPC_B64` and return it).
     Blocked on pinning the calling convention — see item #5.

---

## Diagnostic notes from the session (instrumentability)

We also discussed how to chase down "function X reported uninstrumentable"
(motivated by a function called `funptr1`). Captured here because it's
likely to come up again while wiring up calls.

`isInstrumentable()` is **derived on every call**, not flagged. Decision
chain:

```
BPatch_function::isInstrumentable           BPatch_function.C:972
  -> func_instance::isInstrumentable        function.C:499
    -> Instrumenter::isInstrumentable
      -> DynInstrumenter::isInstrumentable  Relocation/DynInstrumenter.C:181
        -> parse_func::isInstrumentable     parse_func.C:133
```

`parse_func::isInstrumentable()` returns false if any of:

1. `isInstrumentableByFunctionName()` rejects — hardcoded blocklist of
   `gethrvtime`, `_divdi3`, `GetProcessTimes` (`parse_func.C:44`).
2. `img()->isUnlinkedObjectFile()` — true for `.o` files; rejects *every*
   function in the image.
3. PLT — `obj()->cs()->linkage().find(addr())` hit (`parse_func.C:138`).
4. `hasUnresolvedCF()` — `parse_func.C:107`. Walks every block's edges;
   triggers on any `edge->sinkEdge() && !edge->interproc() && (type == INDIRECT || type == DIRECT)`.

For a function named `funptr1`, the leading cause is **#4** — the function
likely contains an indirect tail call (`jmp *fp`) that ParseAPI couldn't
resolve, leaving a non-interprocedural sink edge of type `INDIRECT`.

Distinctions worth keeping straight:
- Indirect **call** (`call *rax`) is typed `CALL` with `interproc()=true`
  and by itself does **not** trigger #4.
- Indirect **jump / tail-call** is typed `INDIRECT`, non-interprocedural,
  and **does** trigger.
- `hasWeirdInsns_` (`parse_func.h:206`, set via
  `DynParseCallback::foundWeirdInsns` from `Parser.C:1896`) is **not** fed
  into `isInstrumentable()` — it's a ParseAPI-internal re-parse gate. Easy
  to mistake for an instrumentability flag.

If you want a real `setUninstrumentable(bool)` hook, add a member to
`parse_func` and an `&&!uninstrumentable_` clause in
`parse_func::isInstrumentable()`. Nothing comparable exists today.

---

## Reference: the original four-way external-call recipe

For sanity if any of the edits get lost. Inside an `emitCall` for a
known-external callee, the slot+SWAPPC sequence is:

```cpp
// 1. Get/reserve a slot, relocation registered against the callee symbol.
Address slot = getInterModuleFuncAddr(callee, gen);

// 2. Allocate an SGPR pair for the link register (S_SWAPPC writes PC+4 here).
Register lrPair = gen.rs()->allocateGprBlock(
    RegKind::SCALAR, /*numRegs=*/2, NS_amdgpu::PAIR_ALIGNMENT);

// 3. Inside emitIndirectCall: allocate a 4-SGPR pair-aligned scratch
//    block, split into slotAddrPair (lo) and tgtPair (hi).
//    Compute slot address PC-relative (S_GETPC_B64 + add diff), dereference
//    for 8 bytes, S_SWAPPC.
emitIndirectCall(slot, lrPair, gen);

gen.rs()->freeGprBlock(lrPair);
```

`emitIndirectCall` is the helper this session added — see
`emit-amdgpu.C` next to `emitLongJump`. It owns the scratch allocation and
the actual instruction emission.

---

## Quick-start for next session

1. `cd /home/wuxx1279/dyninsts/worktree/claude && git status` to see
   uncommitted state.
2. The five files above carry the changes; nothing committed yet.
3. Build on an AMDGPU-configured tree:
   `cmake -DDYNINST_CODEGEN_ARCH=amdgpu_gfx908 ...` (see
   `cmake/DyninstPlatform.cmake:90-91`).
4. The first thing likely to break is item #1 under "Open items" — the
   stub `emitElfStatic` not emitting the dynamic relocation that
   `addExternalSymbolReference` registers. If the slot in the output binary
   reads zero at load time, that's the cause; look at
   `symtabAPI/src/emitElfStatic-stub.C` and `emitElfStatic-aarch64.C` for
   the missing implementation pattern.
