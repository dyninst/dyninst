# Displaced single-step in ProcControlAPI

Developer notes on how ProcControlAPI emulates a single step across an
LL/SC (load-exclusive / store-conditional) sequence by running a copy of the
sequence out of line. The code lives in `proccontrol/src/process.C`
(`int_thread::setupDisplacedSingleStep`, `int_thread::cancelDisplacedSingleStep`,
`int_thread::translateDisplacedPC`, the `int_process::*DisplacedSlot*` pool,
`emulated_singlestep`), `proccontrol/src/handler.C`
(`HandleEmulatedSingleStep`, `HandleDisplacedStepCancel`), and the per-platform
hooks in `arm_process.C` and `ppc_process.C`.

## 1. The problem

On aarch64 and Power a hardware single step cannot get through an LL/SC
loop: the trap after each instruction clears the exclusive monitor (the
reservation), so the store-conditional always fails and the loop retries
forever. ProcControl therefore *emulates* the step when a stepping thread is
stopped at an exclusive load: it finds the end of the sequence (the first
store-conditional) and the targets of branches that leave it, plants a
thread-specific, one-time breakpoint at each, runs the thread without
stepping, and on the hit removes the breakpoints and restores stepping
(`int_thread::handleSingleStepContinue`, `emulated_singlestep`).

The breakpoints of that *in-place* emulation sit in the text of the sequence,
which on glibc is a shared outline-atomics helper (`__aarch64_cas4_acq`,
`__aarch64_swp4_rel`, ...) that every thread of the process executes. Three
things follow, all seen in the testsuite's `pc_singlestep` on aarch64:

1. **Foreign hits.** Another thread hits the breakpoint. ProcControl must
   treat it as a breakpoint hit by a thread that is not stepping: it holds
   every other thread, removes the breakpoint, single-steps the hitter over
   it and restores it. That is a process-wide stop for every foreign hit.
2. **Late traps.** The owner's handler removes the breakpoint while a foreign
   thread has already executed it but its SIGTRAP has not been decoded yet.
   The decoder finds no breakpoint at the PC and no step flag, and the trap is
   delivered to the mutatee as a real SIGTRAP, which kills it.
3. **Stale marks.** `HandleBreakpoint` marks the hitter as "stopped on the
   breakpoint at X" so the next continue steps it over X. The emulation
   removes X without a step-over, the mark stays, and when another thread's
   emulation puts a breakpoint back at X the stale mark triggers a step-over
   of the first thread from wherever it is by then. If that one stepped
   instruction is a futex wait on a lock a held thread owns, the process
   deadlocks.

Displaced stepping removes the shared-text breakpoints, and with them 1 and
2 entirely. 3 is removed for emulation breakpoints by clearing the mark where
the breakpoint is removed, unless a user breakpoint at the same address is
still installed (then the thread really is stopped on a breakpoint and the
step-over must happen); the generic continue-time check added separately
(dyninst/dyninst#2424) covers the remaining paths.

## 2. The design in one paragraph

When a stepping thread is at an exclusive load, the sequence (through the
store-conditional) is copied into a per-thread *slot* of a scratch page in the
mutatee. Branches that stay inside the sequence keep their offsets, because
the copy preserves the layout. Branches that leave it are retargeted to *stub*
words placed after the copy, and one more stub follows the copy for the
fall-through. The stubs are installed as ordinary thread-specific one-time
breakpoints through the existing `emulated_singlestep`, the thread's PC is
moved to the slot, and the thread runs. Only this thread can execute the
slot. On the stub hit the PC is moved to the original address the stub stands
for (exit target or the instruction after the sequence), stepping is restored
and a `SingleStep` event is delivered, exactly as the in-place emulation does.

## 3. Life of a displaced step

`int_thread::intCont` → `handleSingleStepContinue` runs for every continue of
a stepping thread. For each thread that needs emulation (`plat_needsEmulatedSingleStep`
found a sequence at the PC and returned the exit targets plus the address after
the sequence), it first tries `setupDisplacedSingleStep`:

1. **Platform check.** `plat_supportsDisplacedSingleStep()` is false by default;
   `arm_process` and `ppc_process` return true. x86 never needs the emulation.
2. **Scratch pool.** If the process has no pool yet, an internal inferior-malloc
   RPC for one page is posted to *this* thread (`startDisplacedSlotPoolAlloc`)
   and the thread is **held** (see §5). The RPC's completion
   (`displacedSlotPoolRPCFinished`, called from `iRPCHandler`) records the page
   and splits it into 256-byte slots. If the RPC fails, the process falls back to
   in-place emulation for good (`dstep_pool_failed`).
3. **Read and classify.** The words `[pc, end)` are read. For each one the platform
   hook `plat_classifyInsnForDisplacedStep` says `plain` (position independent),
   `rel_branch` (PC-relative branch with its target) or `unrelocatable`. Any
   `unrelocatable` instruction, or a breakpoint installed anywhere inside the
   sequence, makes the thread fall back to in-place emulation for this step, which
   runs under a process stop (§5a).
4. **Slot.** `acquireDisplacedSlot()`; if every slot is busy the thread is held
   until one is released.
5. **Layout.** `copy = sequence words; stub(fall-through → end); one stub per
   distinct exit target`. Exit branches are rewritten by
   `plat_retargetBranchForDisplacedStep` to point at their stub. Stub words hold
   the platform's breakpoint instruction even before the breakpoint is installed.
6. **Write and arm.** The copy is written to the slot, an `emulated_singlestep`
   is created (its constructor turns the thread's step flags off and remembers
   them), `setDisplaced(slot, end-of-copy, sequence start, stub→resume map)`
   records the geometry, each stub becomes a breakpoint via
   `emulated_singlestep::add`, and the PC is set to the slot.
7. **Run.** The thread is continued normally (no single step). It executes the
   copy; the retry edge loops inside the copy; the LL/SC completes because
   nothing traps in between.
8. **Stub hit.** The decoder finds an installed breakpoint at the stub address and
   produces an `EventBreakpoint`. `HandleBreakpoint` runs first (marks the thread
   stopped-on-bp, no user callback: the breakpoint has no user `Breakpoint`).
   `HandleEmulatedSingleStep` (PostPlatformPriority) sees the address is one of
   its breakpoints, removes all of them, moves the PC to the stub's resume
   address, clears the stopped-on-bp mark (the breakpoint and the PC it referred
   to are both gone), restores the step flags, deletes the `emulated_singlestep`
   (which releases the slot) and throws a late `EventSingleStep`. The user's
   callback sees the thread at the original address, single-step mode on.

## 4. Events and how each is handled

ProcControl dispatches handlers by exact event type, so every event that can
catch a thread inside its copy has to be listed. `HandleDisplacedStepCancel`
runs at PrePlatformPriority (before the type's own handler) and calls
`int_thread::cancelDisplacedSingleStep` on the event's thread, and on every
thread only for a detach.

Why only a detach: a process-synchronous event (exit, crash, exec, and the
thread_db thread events) makes ProcControl mark *every* thread's handler state
stopped as bookkeeping, whether or not that thread has actually stopped. A
cancel issued on that mark can run while another thread sits on a stub BRK
whose trap the generator has not decoded yet: the cancel moves its PC and
removes the stub, and the trap is then decoded as a plain SIGTRAP and delivered
to the mutatee (found by pc_atomic_step's run of pc_singlestep: core with
`si_code = TRAP_BRKPT`, `si_addr` in the scratch page, PC already at the resume
address). Detach is safe because `EventDetach` is a proc-stopper, so each
thread really is stopped when it is handled.

| event | who | what happens |
|---|---|---|
| Breakpoint at a stub | `HandleEmulatedSingleStep` | completion, §3 step 8 |
| Breakpoint elsewhere while in the copy (a hardware watchpoint, say) | `HandleEmulatedSingleStep` | cancel: PC translated back, stubs removed, slot freed, stepping restored |
| Signal | `HandleDisplacedStepCancel` | cancel before `HandleSignal`; the signal is then delivered at the original address |
| Stop (our SIGSTOP: user `stopProc`/`stopThread`, or an internal process stop for another thread's breakpoint step-over or an iRPC setup) | `HandleDisplacedStepCancel` | cancel; on resume the thread steps in the original text, the store-conditional fails under the step, the loop retries into a fresh displaced step |
| Crash, Exit (pre) | `HandleDisplacedStepCancel`, the event's thread only | cancel, so no scratch PC is ever reported; the other threads are not really stopped (see above) |
| Detach | `HandleDisplacedStepCancel`, every thread (a proc-stopper, so all really stopped) | cancel **before** `HandleDetach` removes every breakpoint, including the stubs; otherwise the thread would run off the end of its copy after detach. The scratch page stays mapped in the detachee (one page) |
| LWPDestroy, UserThreadDestroy (pre and post) | `HandleDisplacedStepCancel`; also `~int_thread` | cancel; if the thread object dies with a live emulation anyway, `~int_thread` removes the stub breakpoints from the breakpoint table before deleting it (a reused slot address must not find a dangling `int_breakpoint`), the slot is released, and if the thread was running the pool RPC the in-flight RPC is forgotten so the next stepper posts a new one |
| RPC completion of the pool RPC | `iRPCHandler` → `displacedSlotPoolRPCFinished` | pool recorded, the RPC thread's step flags restored; held threads resume at the next `syncRunState` |
| EmulatedSingleStepStart (internal, proc-stopper) | `HandleEmulatedSingleStepStart` | in-place fallback: once every thread is stopped, install the text breakpoints and release only the stepping thread (§5a) |
| Exec | `int_process::execed` → `resetDisplacedSlotPool` | the page is gone with the image; the next sequence allocates a new one |
| Fork | nothing | the child's `int_process` has no pool and allocates its own; the parent's page is inherited unused |
| user `Thread::setRegister`/`setAllRegisters`/`setAllRegistersAsync` | public API | cancel first, then write; moving a PC out from under a displaced step would leave the stubs armed and stepping off |
| user `Thread::getRegister`/`getAllRegisters` | public API | an in-copy PC is translated back (safety net; after the cancel handlers above it should never be observed) |
| `EventSignal::getAddress()` | Linux decoder | translated when the event is built, so the reported address is the original one |

### PC translation, and why the sequence is never restarted

`translateDisplacedPC` maps a body address 1:1 (`sequence_start + (pc - slot)`,
valid because the copy preserves the layout) and a stub address to the address
the stub stands for. The sequence is *not* restarted at the exclusive load on a
cancel: the PC can already sit on a stub with the store-conditional committed
and the BRK trap not yet taken (a SIGSTOP that lands between the two), and
re-executing the read-modify-write would corrupt the lock word.

A cancel on an internal stop is correct but wasteful: the thread lands
mid-sequence in the original text with stepping on, the store-conditional
fails under the step, and the loop retries into a new displaced step.

## 5. Holding a thread, and the one exemption

Two situations have no slot to give: the pool RPC is still in flight, or every
slot is busy. In both, `setupDisplacedSingleStep` returns `aret_async` and
`handleSingleStepContinue` propagates it. `intCont` already treats that return
as "postpone this continue" (the path exists for asynchronous platforms), so the
thread simply stays stopped with its target state running. `syncRunState` runs
after every handled event and retries the continue of every such thread, so the
hold ends as soon as the RPC completion, or the stub hit that frees a slot, has
been handled. No shared-text breakpoint is ever planted for a relocatable
sequence. The RPC thread's own step flags are turned off for the duration of the
RPC so the allocation snippet is not stepped, and restored on completion.

**Exemption: a pending stop.** When ProcControl has sent a thread a SIGSTOP and
the thread stops on something else first (its own stub, a step), the SIGSTOP is
still queued in the kernel. ProcControl continues the thread once more *only to
take delivery of the stop*; the thread executes nothing before it stops again
(`int_thread::hasPendingStop()`, the PendingStop state). `handleSingleStepContinue`
skips emulation setup for such a thread. Holding that continue instead deadlocks:
the stop is never delivered, so whatever asked for the process to stop (an iRPC
setup, say) never proceeds, so if that iRPC is the pool allocation the pool never
arrives and the held threads never run. This was found on aarch64 with
`pc_singlestep` and is the one hard interaction between the hold and the rest of
the state machine.

## 5a. The in-place fallback runs under a process stop

When a sequence cannot be displaced (an `unrelocatable` instruction, a user
breakpoint inside it, or no scratch pool because its allocation failed), the
emulation breakpoints have to go into the shared text after all. To keep the
"no thread ever traps on a breakpoint it did not install" property, the step is
run the way a breakpoint step-over runs:

1. `handleSingleStepContinue` does not continue the thread. It desyncs every
   thread's BreakpointState to stopped (`desyncStateProc`), throws an
   `EventEmulatedSingleStepStart` for the thread, and postpones the continue.
2. The event is a proc-stopper: `ProcStopEventManager` holds it until every
   thread of the process is stopped on that state.
3. `HandleEmulatedSingleStepStart` then installs the breakpoints
   (`emulated_singlestep::add`), desyncs the BreakpointResume state of every
   *other* thread to stopped (threads created meanwhile start stopped), marks
   the emulation as holding the process, and restores the BreakpointState so
   the stepping thread, and only it, runs. The stepping thread is deliberately
   not pinned to running at that level, unlike a breakpoint step-over: the
   BreakpointResume level outranks the user's, and a sequence can take a while
   (a loop that spins inside the exclusive window until another thread writes
   something), so a `stopProc` during it must really stop the thread. The hold
   on the others stays in force across such a stop and the continue after it;
   `handleSingleStepContinue` sees the emulation in flight and does not set up
   another.
4. `HandleEmulatedSingleStep` (the breakpoint hit) removes the breakpoints and
   releases the hold (`releaseProcess`, which restores BreakpointResume for
   the held threads), then proceeds as usual. If the thread dies first,
   `~emulated_singlestep` releases the hold.

Which addresses get a breakpoint is decided by `plat_needsEmulatedSingleStep`:
the instruction after the store-conditional and the target of every branch
that leaves the sequence. A branch whose target is *inside* the sequence (a
retry edge back to the exclusive load, as in a loop that spins inside the
exclusive window) is not an exit and gets none; a breakpoint there would sit
on the instruction the thread is about to execute, so the step would complete
without progress, or reach the mutatee as SIGTRAP if the trap were decoded
after the breakpoint was removed. The scans used to treat such targets as
exits; this series fixes that ahead of the displaced-step commits, since it
affects the pre-existing in-place emulation on its own.

Nothing inside an LL/SC loop waits on another thread, so the hold lasts for
one pass of the sequence. The cost is a process-wide stop on a path the
testsuite never takes (every sequence in glibc's helpers is relocatable).

Only one such step runs at a time per process (`inplaceSingleStepOwner`). The
StateTracker levels do not nest: a second BreakpointResume desync on top of a
live one would be undone by the first release and leave every thread stopped.
A second thread that needs the fallback while one is in progress (typically
one whose target state was computed in the same `syncRunState` pass, before
the first request stopped the process) simply stays stopped and is retried
after the release.

## 6. Several threads and the same sequence

The point of the design is that threads never share emulation breakpoints:

- **Two threads at the same exclusive load.** Each gets its own slot and its own
  copy; the two copies carry independent thread-specific one-time breakpoints.
  Neither thread can hit the other's stubs, because no thread ever executes
  another thread's slot. The original text is untouched, so threads that are
  *not* stepping run through the helper at full speed and see no breakpoint.
- **A thread reaching a breakpoint "already hit" by another thread.** For stubs
  this cannot happen, and not for the in-place fallback's text breakpoints
  either, because every other thread is held while they exist (§5a). For
  *user* breakpoints the standard machinery applies, with
  two displaced-step specifics: a user breakpoint anywhere inside a sequence makes
  the stepping thread fall back to in-place emulation (the copy would otherwise
  skip the breakpoint, and the user would miss the hit); and when another
  thread's hit on a user breakpoint triggers a process-wide step-over, the
  resulting Stop on a thread that is mid-copy cancels its displaced step (§4).
- **The stale stopped-on-breakpoint mark.** With stubs the mark would point into
  the thread's own slot, which the same thread reuses for its next displaced
  step, so a stale mark would fire on every reuse. `HandleEmulatedSingleStep`
  clears the mark at completion, for displaced and in-place emulation alike,
  unless a breakpoint is still installed at that address: with in-place
  emulation a user breakpoint right after the sequence shares the installed
  breakpoint with the emulation's, and the thread, which is stopped on it, must
  still be stepped over it. `cancelDisplacedSingleStep` clears the mark when it
  points into the copy.
- **Slot exhaustion.** A page of 4 KiB holds 16 slots. The 17th concurrent
  displaced step is held until a slot frees; a running owner always frees its
  slot (it traps at a stub or gets cancelled on its next stop), so the hold
  cannot last.

## 7. Platform hooks

Adding an architecture means implementing three virtuals on its `int_process`
subclass:

- `plat_supportsDisplacedSingleStep()` → true.
- `plat_classifyInsnForDisplacedStep(raw, addr, info)`: fill `info.kind` with
  `plain`, `rel_branch` (and `info.target`), or `unrelocatable`. Anything whose
  meaning depends on its address must be `unrelocatable`: PC-relative data
  access, register-indirect branches, calls (the link register would point into
  the slot), system calls and trap instructions.
- `plat_retargetBranchForDisplacedStep(raw, copy_addr, new_target)`: rewrite the
  branch's displacement so that, placed at `copy_addr`, it reaches `new_target`.
  Targets are inside the same slot, so the displacement is always small.

| arch | relocatable branches | unrelocatable |
|---|---|---|
| aarch64 | B, B.cond, CBZ/CBNZ, TBZ/TBNZ | BR/BLR/RET, BL, ADR/ADRP, literal loads (LDR/LDRSW/PRFM), SVC/HVC/SMC/BRK/HLT |
| Power | B, BC with AA=0 and LK=0 | absolute or linking branches, `sc`, bclr/bcctr/bctar, addpcis |

Instruction width is taken from `plat_breakpointSize()`; the design assumes a
fixed-width instruction set with a breakpoint that does not advance the PC,
which is what every LL/SC architecture ProcControl supports looks like.

## 8. Memory and lifetime

- The pool is one page from `createInfMallocRPC` (the same mechanism as
  `Process::mallocMemory`), owned by the process, never freed; it is recorded in
  `mem_state::inf_malloced_memory`. Exec forgets it. Detach leaves it mapped.
- The allocation RPC runs its mmap snippet from borrowed executable memory
  (`int_process::mallocExecMemory`), like every inferior malloc. If a platform's
  first executable mapping starts with live code (a PLT under `-z separate-code`)
  that borrowing is a pre-existing hazard; aarch64 and ppc64le Linux binaries
  start their first executable mapping with the ELF header.
- A slot is owned by the thread's `emulated_singlestep` from `setDisplaced` until
  the object is deleted (completion, cancel, or `~int_thread`).
- Memory writes to a slot go through `int_process::writeMem`, so the kernel's
  ptrace write path handles instruction-cache maintenance exactly as it does for
  breakpoints.

## 9. Debugging

With `DYNINST_DEBUG_PROCCONTROL=1` the relevant lines are:

- `Displaced single step on P/T: sequence A-B copied to S, N stubs`
- `Displaced single step done on P/T, stub S -> resuming at R`
- `Holding P/T until displaced single-step scratch is available`
- `Posted displaced-step pool RPC ...` / `Displaced-step pool on P at A, N slots`
- `Cancelling displaced single step on P/T, PC X -> Y`
- `Translating PC X inside displaced copy ...` (the safety net fired: a stop
  path is missing from §4)
- `Instruction ... cannot be displaced`, `Breakpoint at ... inside atomic
  sequence, not displacing`, `Could not retarget branch ...` (fallbacks)
- `Installing emulated single-step breakpoint for P/T at X` (the in-place
  emulation; for relocatable sequences on a supporting platform this should not
  appear)

`DYNINST_DISABLE_DISPLACED_SINGLESTEP=1` forces every emulated step down the
in-place fallback (§5a), which is the only way to exercise that path with the
testsuite's mutatees; its marker is `Stopping process P for an in-place
emulated single step of thread T`.

Note that the debug log slows the mutatee by about two orders of magnitude and
hides the races the design removes; use it to count paths, not to measure
pass/fail rates.

## 10. Known limitations

- During an in-place fallback every other thread is held. If a signal handler
  runs on the stepping thread in that window and blocks on a lock a held thread
  owns, the process deadlocks until the user stops it. A breakpoint step-over
  has the same exposure today.

- `Thread::getRegisterAsync` and `getAllRegistersAsync` deliver their values
  through asynchronous events and do not translate an in-copy PC (the
  synchronous getters do; the cancel handlers make the case unreachable in
  practice).
- A SIGSTOP that lands with the PC on a stub whose BRK trap is also pending moves
  the PC to the resume address; the later SIGTRAP is then decoded as an ordinary
  single step there, so the user may get no step event for the instruction after
  the sequence.
- Asynchronous platforms (none built today) would need async handling in setup
  and cancel.
- Power's sequence detector recognises only `lwarx`/`stwcx.`; `ldarx`/`stdcx.`
  loops are not emulated at all (pre-existing). The aarch64 `ATOMIC` masks also
  match LDAR/STLR/CAS (pre-existing); displaced copies of such "sequences" are
  harmless.
