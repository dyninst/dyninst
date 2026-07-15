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

## Status

- Landed: encapsulation E1–E5 (accessors + private bridge), 2a/2b deletion
  model, weak thread→process cache, detach-leak fix. Each validated 1534/1534.
- **Uncommitted working tree**: option-(ii) condvar retirement + default-ON
  accessor locking + lock-attempt logging + the nolock tags + `tla/` model.
  Gate: full native validation AND zero TSan lock-order-inversions in the
  same iteration.

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
