# ProcControl re-architecture — working notes (branch `prototype-pool-owns-wrapper`)

Read `README.md` first for the object model, ownership DAG, and event flow.
This file covers the *active* work: the lock migration, its verification
harness, and the rules changes must follow.

## Design goal

Retire the global `work_lock` in favor of per-process parallelism:

- `int_process`/`int_thread` are impl details reachable only through the
  refcounted wrappers (`Process::ptr`/`Thread::ptr`). **Enforced by the
  compiler**: `llproc()`/`llthrd()` are private; boundary code crosses via the
  checked accessors `ProcImplRef`/`ThreadImplRef` (int_process.h).
- The accessors take the per-process `proc_lock` by default (impl resolved
  under the lock, serialized against teardown). `work_lock` still sits on top;
  these locks become load-bearing as `MTLock` shrinks.
- The old ProcPool condvar survives ONLY as the **registration lock**:
  bootstrap holds it across [plat_create/attach .. registration]; the decoder
  takes it briefly around `findThreadAndProc` (a per-process lock cannot
  exclude decode of a pid that is not yet registered/findable).
- Lock order: `work_lock > registration lock (lookup only) | proc_lock > map_lock`.
  The generator never holds the registration lock while acquiring a proc_lock.
- End state (Phase B, not started): `work_lock` dissolves into a global
  `cb_lock` (one-callback-at-a-time, per the API doc) + `proc_lock`.

### The proc_lock discipline (every clause is a found deadlock)

`proc_lock` is an **entry-point lock**. It must NEVER be:
1. acquired in plumbing reachable with leaf locks held
   (deadlock #5: `reg_response::postResponse` under `regpool_lock`);
2. held across a blocking park — `waitAndHandleEvents/ForProc`,
   `infMalloc`/`infFree`, `preTerminate` — the generator needs it to decode
   the very event being awaited (deadlocks #3 Codegen, #6 terminate);
3. held across user-callback delivery — callbacks are foreign code that takes
   client (dyninstAPI) locks (deadlocks #7 proc_lock, #8 work_lock; TSan-found).
   deliverCallback drops BOTH work_lock and proc_lock and holds only cb_lock
   (MTManager::suspendForCallback/resumeAfterCallback) for the invocation.

Scopes matching these use `ProcImplRef(x, implref_nolock)` with a comment
naming the reason. Danger pattern: waits/locks hidden in *callees* — all
seven deadlocks were transitive escapes that body-level audits missed.

### Partial work_lock retirement (option b) — rules for converting methods

We are dropping `work_lock` **per method**, only for **ordering-free
per-process ops** (reads first: `readMemory`, `getRegister`,
`getAllRegisters`). A converted method holds `proc_lock` *instead of*
`work_lock`. Rules, each a real hazard:

1. **Every `proc_lock` acquisition must go through `Process::lockImpl`/
   `unlockImpl`** (via `ProcScopeLock` or the ImplRef accessors), never
   `procLock()->lock()` directly — otherwise `proc_lock_depth_` is wrong and
   `suspendImplLock` (callback delivery) under-unwinds → clause-3 violation or
   a wedged lock.
2. **Inside a converted method's `proc_lock` scope, no re-entry into a public
   API method** (which takes `MTLock`) — that is `proc_lock → work_lock`,
   inverting against the whole rest of the codebase (`work_lock → proc_lock`).
   Resolve everything needed, then release before any such call.
3. **A reader may only drop `work_lock` once its paired *mutators* also take
   `proc_lock`.** A `proc_lock`-only reader vs. a `work_lock`-only mutator
   share no lock → race. E.g. converting `getRegister` required
   `setRegister`/`setAllRegisters` (+ async) to take `proc_lock` too; reads
   and writes of the same per-process state must share `proc_lock`.
4. **Only ordering-free ops qualify.** Anything that validates state → waits →
   acts on that state (continue/stop/breakpoint/iRPC) carries a cross-wait
   invariant that `work_lock`'s global serialization provided and per-process
   `proc_lock` does not (this is what broke the full-removal attempt, S5).
   Those stay under `work_lock` pending an event-state-machine redesign.

Validation caveat: the suite is largely single-user-thread, so the *new*
interleavings (reader-on-P vs handler-on-P, two user threads) are lightly
exercised — a green gate understates coverage. TSan flags orders from any run.

## Status

- Landed: encapsulation E1–E5 (accessors + private bridge), 2a/2b deletion
  model, weak thread→process cache, detach-leak fix. Each validated 1534/1534.
- **Option (b) — per-method work_lock retirement (landed, each gated
  1534/1534 + 0 TSan inversions):**
  - `401cf2149` correction: depth-routed proc_lock (lockImpl/unlockImpl),
    mutators take proc_lock, 13 manual brackets routed through lockImpl.
  - Per-process data I/O family now proc_lock-only (work_lock dropped),
    all four corners {sync,async}×{read,write}: `6ba158e8a` readMemory,
    `7ddb09d27` register reads, `52b4ce295` writes, `aa748bd39` async reads.
  - `921064ee3` memory-perm/region family (getMemoryPageSize, findFreeMemory,
    getMemoryAccessRights/setMemoryAccessRights, findAllocatedRegionAround) →
    proc_lock; this also closed a pre-existing (master-native) llproc_-vs-
    teardown UAF on the four lock-free readers.
  - Option-(b) surface for ordering-free ops is exhausted.
- **State-machine redesign (D-series) — diagnosis + D-2 groundwork landed:**
  - S5 post-mortem, corrected: the `⚠(b)` handler asserts (HandleThreadStop
    pending-stop, HandleBreakpointClear all-stopped, the desync/restore count
    chains, iRPC state asserts) are NOT a fundamental cross-thread ordering
    problem — the arm→SIGSTOP→stop→decode→handle causality is provided by the
    kernel round-trip.  What was missing is mutual exclusion + memory
    visibility between the user-side state-machine mutator and the handler's
    syncRunState.  The handler already holds proc_lock (S4 handle_plock); the
    user state-machine methods held only work_lock.  Fix = give them proc_lock
    too (closes the rule-3 gap).
  - **D-2 (proc_lock the state-machine mutators, keep work_lock; no-op until
    removal):** `34873317f` stop/continueThread; `9016d2b1d` setSingleStep/
    SyscallMode + postIRPC; `0d6199fcd` addBreakpoint/rmBreakpoint/runIRPCAsync.
    (ProcessSet stop/continue paths were already proc_lock-covered via default-
    locking ProcImplRef.)  Each gated native 1534/1534 + targeted TSan (the
    on-target pc_* / test1_13-create sweep — NOT the slow test1_* sweep, which
    times out before reaching the changed paths; see verify notes).
  - **D-3 plan (remove work_lock) — the KEY design decision:** convert the
    remaining PUBLIC work_lock helper methods (getPid, getLWP, isTerminated,
    is{Running,Stopped,Exited,Detached,Crashed}, hasRunningThread,
    setLastError/getLastError/clearLastError, ...) to take **proc_lock**, NOT
    route their call sites to internal overloads.  Rationale: proc_lock is
    recursive, so a handler/state-machine method already holding proc_lock that
    calls one of these just re-locks (no proc_lock→work_lock edge); the internal
    int_process/int_thread overloads stay lock-free for leaf/generator callers
    (the two-form split is preserved).  Method-conversion is fewer changes and
    null-safe (proc_lock is on the WRAPPER, valid even when llproc_==NULL —
    matters for setLastError's exited branch).  setLastError specifically needs
    NO lock beyond proc_lock: globalSetLastError already writes plain statics
    lock-free (last-write-wins, pcerrors.C:40/155) and the local write is
    last-write-wins under proc_lock.
  - **Gate before D-3:** a "no PUBLIC helper is ever called under a leaf lock
    or on the generator/decode path" audit (else proc_lock there = clause-1
    deadlock).  Sweep PART 3 indicates leaf/generator uniformly use the
    lock-free internal overloads; a confirming audit is in flight.

## Verification — run these before any commit

All scripts in `~/testsuite/verify/`. **Never run two runTests concurrently**
(the attach tests share a server socket; concurrent suites deadlock falsely).

- `validate_pw.sh` — the standard gate: test_thread_1 stress ×30 (hang
  detector, timeout 90/run) then full `runTests -all`. Pass = STRESS clean AND
  `PASSED=1534 FAILED=0 CRASHED=0 CORES=0 TRIPWIRES=0`. Any `PROTOTYPE:` line
  on stderr is a tripwire regression.
- `stress_tt1.sh [N]` — stress loop only.
- `catch_t12.sh` — hang catcher: runs a test group, gdb-dumps all threads of
  a test_driver stuck >80s. Edit the `-test` list to target a hang.
- Builds: `make pcontrol -j && make install -j` in `~/dynmaster/build`
  (target is `pcontrol`, install takes >2 min — background it).
- Lock-attempt logging: set `DYNINST_DEBUG_PROCCONTROL=1`; accessors print
  `proc_lock: acquiring/acquired/released for <pid>`.

### ThreadSanitizer

Separate trees: `~/dynmaster/build-tsan` → `~/bin/dynmaster-tsan`, and
`~/testsuite/build-tsan` → `~/testsuite/install-tsan`
(`-fsanitize=thread -fno-omit-frame-pointer -g -O1`, mirrors build-asan).

- `tsan_target.sh` — targeted sweep (edit the `-test` list);
  `tsan_sweep.sh` — attach + pc_* sweep. Both set
  `TSAN_OPTIONS="detect_deadlocks=1 second_deadlock_stack=1 ..."` and
  `RUNTESTS_TIMEOUT=7200` (env override added to runTests-utils.C; TSan is
  5–15× slower than native — never run TSan under the default 600 s reaper).
- After changing proccontrol: `make pcontrol install -j` in build-tsan first.
- Verdict: grep the `log_path` report files for
  `lock-order-inversion` — must be zero. Data-race warnings are a separate
  triage pile and do not gate commits.
- pc_library and pc_tls are EXCLUDED from the TSan sweep: under TSan the
  mutatee is itself sanitizer-instrumented, and Dyninst instrumenting a
  TSan binary's dlopen/TLS paths segfaults the debuggee (native-clean).
  Attach mode is likewise unusable under TSan (ptrace-attach vs TSan).
- All verify scripts serialize on /tmp/runtests.lock (flock): the attach
  tests share a server socket and concurrent runTests deadlock falsely.
- TSan reports inversions from *passing* runs (both stacks) — run it on any
  new locking change even when tests are green.

### TLA+ model checking

`tla/ProcControlLocks.tla` + `.cfg` — the lock/protocol design model.
Generator, handler, user threads; registration barrier; parks; recurring
event source. Every historical failure class is a constant knob
(`BUG1/2/3/5_...`); the design rule is a checked invariant
(`NoParkWhileHoldingProcLock`).

Run (needs Java 11+; a portable Temurin JRE works, no root):

    cd ~/dynmaster/proccontrol/tla
    java -XX:+UseParallelGC -cp tla2tools.jar tlc2.TLC -workers 4 ProcControlLocks.tla

(`tla2tools.jar` from https://github.com/tlaplus/tlaplus/releases; deadlock
checking is on by default — do NOT pass `-deadlock`, it disables it.)

Expected: all knobs FALSE → clean (~4.6k distinct states); each knob TRUE →
deadlock (BUG1/2/5) or invariant violation (BUG3). Changing the lock design?
Update the model FIRST and keep all five configs behaving — the model is the
regression suite for design decisions. Phase B additions queued: callback
clause (BUG7 knob + `NoProcLockAcrossCallback`), one-callback-at-a-time
invariant, a separate `ProcControlLifetime` module for the pool/impl
lifecycle.

## Change rules

1. Any locking change: `validate_pw.sh` AND a TSan targeted sweep, green in
   the same iteration, before committing.
2. New boundary scope: locked accessor by default; `implref_nolock` only with
   a comment citing which discipline clause applies — and check the *callees*
   for hidden waits/locks/condvars.
3. Windows/FreeBSD don't build here: they are friended wholesale in
   PCProcess.h and unconverted — expect a small fixup pass on platform CI.
