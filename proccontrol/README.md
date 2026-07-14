# ProcControlAPI

ProcControlAPI is the process-control layer: it launches/attaches to debuggees,
observes them via OS debug interfaces (ptrace on Linux, etc.), and delivers a
stream of typed **events** to callers, who instrument or inspect the target and
continue it.

This document describes the object model, the threads of control, the locking
discipline, and how events flow through the primitives.

---

## 1. Object model — two layers

Every process and thread is represented by a **pair** of objects:

| Public wrapper | Internal impl | Registry / container |
| -------------- | ------------- | -------------------- |
| `Process` (`Process::ptr`) | `int_process` | `ProcessPool` (singleton, `ProcPool()`) |
| `Thread` (`Thread::const_ptr`) | `int_thread` | `int_threadPool` (per process) / `ThreadPool` (its public face) |

**Wrappers** (`Process`, `Thread`) are the public, reference-counted
(`boost::shared_ptr`) handles. They are the exported ABI and the currency every
other subsystem holds.

**Impls** (`int_process`, `int_thread`) are the internal, platform-polymorphic
implementation (`linux_process`, `linux_thread`, …). They are *not* reference
counted; they are created and destroyed explicitly, and are reachable **only
through their wrapper**:

```
Process::llproc()  ->  int_process*     (NULL once the process is gone)
Thread::llthrd()   ->  int_thread*      (NULL once the thread is gone)
```

These two accessors are the *only* bridge between the layers. No other subsystem
stores a raw `int_process*`/`int_thread*` that can outlive the impl; long-lived
and cross-boundary references are held as `Process::ptr`/`Thread::ptr` and the
impl is dereferenced transiently at the point of use. If the impl has been
destroyed, `llproc()`/`llthrd()` return `NULL` — a checkable condition, never a
dangling pointer.

### Ownership and lifetime (a DAG)

```
ProcessPool ─strong─▶ Process ─owns─▶ int_threadPool ─holds Thread::ptr each─▶ Thread
                        │                   ▲                                     │
                        └─llproc_─▶ int_process ─(raw, non-owning cache)──────────┘
                                     └─int_thread reached via llthrd_
Event/response/RPC ─strong─▶ Process / Thread (the wrappers)
Thread ─proc_wrapper_ (strong)─▶ Process   [redundant up-ref; slated for removal]
```

- **`ProcessPool` owns the wrappers.** Its strong `Process::ptr`/`Thread::ptr`
  references are what keep a wrapper alive for the duration of the debugging
  session. Registration happens at birth; removal at exit.
- **The wrapper's lifetime is a superset of the impl's.** While an impl is
  alive its wrapper is too; after the impl is destroyed the wrapper lives on
  (holding the exit state) as long as a user handle or an in-flight event
  references it.
- **`Process` owns the `int_threadPool`** (the container of `Thread::ptr`), not
  `int_process`. `int_process` keeps a raw, non-owning cache of it for hot
  `threadPool()` access; since the wrapper outlives the impl, the pool (and
  `Process::threads()`) stays valid after the process exits. Note this is
  container ownership only — threads are still torn down deterministically at
  exit, so a dead thread still reads as `NULL llthrd()`.
- **The impls point only *downward or sideways*, never up.** `int_thread` knows
  its `int_process` (`proc_`). Helper objects owned by an impl (handler pool,
  library pool, register pool, breakpoints) hold a raw back-pointer to their
  owner — safe, because the owner outlives them by construction. No impl holds
  a reference *up* to a wrapper. (The one exception, `Thread::proc_wrapper_` — a
  strong `Process::ptr` cached for lock-free thread→process resolution — is a
  known wart that creates a `Process`↔threads cycle; it is slated for removal.)
- **Deletion is wrapper-centric and single-homed.** `ProcessPool::destroyProcess`
  / `destroyThread` are the only ways an impl is destroyed: they unregister,
  publish exit state into the wrapper, sever the `llproc_`/`llthread_` link,
  then `delete` the impl. Impl destructors never call back into the pool.

### Creation

Creation goes through the wrapper layer:

- `Process::makeProcess(...)` / `Thread::makeThread(...)` mint the wrapper +
  impl and initialize the pair. (`int_process::createProcess` /
  `int_thread::createThreadPlat` remain only as the platform impl-construction
  primitives these call.)
- Bootstrap orchestration lives on the lifecycle owner:
  `ProcessPool::createProcs` / `attachProcs`.

So a caller never assembles a process/thread out of raw impl pieces, and the
factory never hands a raw impl pointer back across a boundary.

---

## 2. Threads of control

ProcControlAPI is multithreaded. In the default **HandlerThreading** mode three
kinds of thread touch these objects:

1. **User / API threads.** Whoever calls the public API (`Process::continueProc`,
   `writeMemory`, `ProcessSet::…`, etc.). Every public entry point takes an
   `MTLock`, which in HandlerThreading mode acquires the MTManager **`work_lock`**
   for the duration of the call.

2. **The generator thread** (`generator.C`). Blocks on the OS debug interface,
   decodes raw OS notifications into `ArchEvent`s and then `Event`s (see
   `DecoderLinux::decode`), and enqueues them. It resolves the OS id (pid/lwp)
   to a wrapper via `ProcPool()->findThread`/`findProcByPid` — the one place a
   pid must be translated to a live object.

3. **The handler thread** (`MTManager::evhandler_main`, `evhandler_thread`).
   Runs the `HandlerPool` over queued events, mutating process/thread state and
   ultimately deciding what reaches the user as a callback.

The user thread and the handler thread are **mutually exclusive** on `work_lock`
— only one runs "work" at a time. This is why, during create/attach bootstrap,
the calling user thread drives event handling itself and process/thread
deletion is synchronous with (not concurrent to) the bootstrap loops.

---

## 3. Locking

> **In flux.** The lock model is mid-migration toward per-process parallelism
> (retiring the global `work_lock` in favor of a per-process `proc_lock`).
> This section describes the current, transitional state.

The locks, outermost first:

```
work_lock (MTManager)  >  ProcPool condvar (var)  >  proc_lock  >  map_lock
                          gen_wait_cv (generator signaling; leaf, off to the side)
```

- **`work_lock`** — held for the length of an API call (via `MTLock`);
  serializes user threads against the handler thread. The migration is
  shrinking its scope; the end goal is to remove it.
- **`proc_lock`** (per-`Process`, recursive) — the per-process mutual-exclusion
  lock, held on the `Process` wrapper (stable lifetime, so it can guard the
  impl it protects). Taken by the generator across decode and by
  `destroyProcess`/`destroyThread`. As `work_lock` shrinks, this becomes the
  primary serializer; operations that touch only `proc_lock` (memory, registers,
  breakpoints) will run in parallel across processes.
- **ProcPool condvar (`var`)** — now a plain **mutex** (its signaling role was
  split out). Callers bracket it around multi-step lifecycle sequences. Not the
  container guardian.
- **`gen_wait_cv`** (generator.C) — the dedicated condition variable the
  generator idles on; `wakeGenerator()` notifies it. Split off the ProcPool
  condvar so no code waits on a lock others hold recursively.
- **`map_lock`** — the sole guardian of `ProcessPool`'s registry containers
  (`procs`, `lwps`, `deadThreads`). A **leaf** lock: held only for a container
  operation, never across a wait, callback, impl `delete`, or an outer lock.
  Public pool methods take it once and delegate to `_nolock` helpers, so it is a
  plain (non-recursive) mutex and cannot participate in a cycle.

Impl→wrapper resolution (`ProcessPool::wrapperFor`) is a guarded fallback that
takes `map_lock`; on steady-state paths it is called **zero** times — wrappers
travel with the control flow (events carry them; a `Thread` caches its
`Process::ptr`). That cache (`proc_wrapper_`) is itself slated for removal (see
§1), after which thread→process resolution moves fully top-down / to pool
lookups.

---

## 4. Events

Events are the output of ProcControlAPI. Each is an `Event` subclass carrying a
`Process::ptr` and (usually) a `Thread::const_ptr` identifying who it happened
to, plus type-specific payload.

### Taxonomy (by what they report)

| Group | Events | Meaning |
| ----- | ------ | ------- |
| **Lifecycle** | `EventBootstrap`, `EventPreBootstrap`, `EventExit`, `EventCrash`, `EventFork`, `EventExec`, `EventTerminate`, `EventForceTerminate`, `EventDetach` | process came up / went away / forked / exec'd |
| **Threads** | `EventNewThread`, `EventNewUserThread`, `EventNewLWP`, `EventThreadDestroy`, `EventUserThreadDestroy`, `EventLWPDestroy`, `EventThreadDB` | threads/LWPs appearing and disappearing |
| **Execution control** | `EventBreakpoint`, `EventBreakpointClear`, `EventSingleStep`, `EventStop`, `EventChangePCStop`, `EventSignal` | traps, stepping, stops, signals |
| **Syscalls** | `EventSyscall`, `EventPreSyscall`, `EventPostSyscall` | syscall entry/exit stops |
| **RPC** | `EventRPC`, `EventRPCLaunch` | inferior-RPC progress |
| **Async I/O** | `EventAsyncRead`, `EventAsyncWrite`, `EventAsyncReadAllRegs`, `EventAsyncSetAllRegs`, `EventAsyncFileRead`, `EventAsyncIO` | completions of async memory/register/file ops |
| **Library** | `EventLibrary` | shared object load/unload |
| **Control/internal** | `EventControlAuthority`, `EventNop`, `EventIntBootstrap`, `EventWinStopThreadDestroy` | tool-coordination and internal plumbing |

Each event also has a **time** (`EventType::Pre` / `Post` / `None`) and a
**sync type** (`sync_process`, `sync_thread`, `async`) that governs how much of
the process is stopped while it is handled.

### How an event flows, and where the primitives come in

```
OS notification
   │  generator thread
   ▼
DecoderLinux::decode(ArchEvent)          pid/lwp ──▶ ProcPool()->findThread/findProcByPid
   │                                                 (the one impl-lookup by OS id)
   ▼
Event  (stamped with Process::ptr + Thread::const_ptr from the wrapper just found)
   │  enqueued to the mailbox
   ▼
HandlerPool::handleEvent   (handler thread, or the user thread during bootstrap)
   │   - mutates int_process / int_thread state via the wrapper's llproc()/llthrd()
   │   - may create threads (Thread::makeThread), post RPCs, install breakpoints,
   │     or destroy the process/thread (ProcessPool::destroyProcess/destroyThread)
   ▼
HandleCallbacks  ──▶  user callback  (receives the Process::ptr / Thread::ptr)
```

Two properties make this safe:

- **Events carry wrappers, not impls.** An event can outlive the thread that
  triggered it (it sits queued while the thread exits). Because it holds a
  `Thread::const_ptr`, the wrapper survives; `getThread()->llthrd()` simply
  returns `NULL` if the thread is gone. The same holds for `response` objects
  and in-flight `int_iRPC`s.
- **The wrapper is reachable through the whole handling window.** A process's
  exit is decoded, handled, and delivered to the user callback across several
  handler steps. The pool entry (and thus the wrapper) is kept until
  `destroyProcess` runs at the *end* of that window, so callback-time code such
  as `Thread::getProcess()` always resolves.

### Handlers and callbacks

`Handler`s are registered per `EventType` in a `HandlerPool` (one per process).
A single decoded event may fan out into several handler actions and *late*
events (e.g. a breakpoint hit → single-step over → re-arm). Only after the
internal handlers finish does `HandleCallbacks` decide whether to invoke the
user's registered callback (respecting suppression and process "silent" mode).
User callbacks receive wrappers and run outside the pool's `map_lock`.

---

## Branch states

| Branch  | Status | Notes           |
| ------- |:------:|:---------------:|
| master  | stable | No open issues  |
