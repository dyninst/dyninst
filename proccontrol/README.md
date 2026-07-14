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
ProcessPool ──strong──▶ Process ──owns──▶ int_process ──owns──▶ int_threadPool
                          ▲                                            │
Event/response/RPC ─strong┤                              holds a Thread::ptr each
                          │                                            │
Thread  ◀──────────────── stored in int_threadPool ◀─────────────────┘
  └─caches its owning Process::ptr (proc_wrapper_)
```

- **`ProcessPool` owns the wrappers.** Its strong `Process::ptr`/`Thread::ptr`
  references are what keep a wrapper alive for the duration of the debugging
  session. Registration happens at birth; removal at exit.
- **The wrapper's lifetime is a superset of the impl's.** While an impl is
  alive its wrapper is too; after the impl is destroyed the wrapper lives on
  (holding the exit state) as long as a user handle or an in-flight event
  references it.
- **The impls point only *downward or sideways*, never up.** `int_thread` knows
  its `int_process` (`proc_`); an `int_process` owns its `int_threadPool`.
  Helper objects owned by an impl (handler pool, library pool, register pool,
  breakpoints) hold a raw back-pointer to their owner — safe, because the owner
  outlives them by construction. No impl holds a reference *up* to a wrapper.
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

There are three locks, with a strict order (outermost first):

```
work_lock  (MTManager)   >   ProcPool condvar (var)   >   map_lock (ProcessPool)
```

- **`work_lock`** — held for the length of an API call (via `MTLock`);
  serializes user threads against the handler thread.
- **ProcPool condvar (`var`)** — a *condition variable*: the generator `wait()`s
  on it for "a process changed state / a live process exists," handlers
  `broadcast()`. Callers also bracket it around multi-step lifecycle sequences
  (create/attach bootstrap). It is held across blocking waits. It is **not** the
  container guardian.
- **`map_lock`** — the sole guardian of `ProcessPool`'s registry containers
  (`procs`, `lwps`, `deadThreads`). A **leaf** lock: held only for the duration
  of a container operation, and **never** across a wait, a user callback, an
  impl `delete`, or the acquisition of an outer lock. Public pool methods take
  it once and delegate to `_nolock` helpers, so it can be a plain
  (non-recursive) mutex. Because it is a leaf, it cannot participate in a cycle.

Impl→wrapper resolution (`ProcessPool::wrapperFor`) is the fallback path that
takes `map_lock`; on the steady-state paths it is not called at all — wrappers
travel with the control flow (events carry them, `int_thread` caches its
`Process::ptr`), so the lock stays off the hot paths.

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
