# ProcControlAPI — object model and event flow

How the core classes interact: `MTManager`, `Process`/`Thread`,
`int_process`/`int_thread`, `Mailbox`, `Event`, `Generator`, `HandlerPool`,
`ProcessPool`, and the coordination primitives. Read alongside `README.md`
(ownership DAG) and `CLAUDE.md` (the active lock migration).

## The cast

| Class | Role |
|---|---|
| `Process` / `Thread` | Public, ref-counted **wrappers** — the API surface the user holds. Thin; take locks, deliver callbacks. |
| `int_process` / `int_thread` | The **implementation** — actual debuggee state (run-state machine, registers, breakpoints, ptrace). Reached only via the checked accessors. |
| `ProcessPool` (`ProcPool()`) | Registry: `pid → Process::ptr`. Owns wrappers until exit/detach; `findProcByPid`, `destroyProcess`. |
| `Event` / `EventType` | A decoded occurrence (stop, exit, fork, breakpoint, …), carrying a `Process`+`Thread` and a *sync type* (thread vs process scope). |
| `Mailbox` (`mbox()`) | The **global FIFO event queue** between the generator and the handler. |
| `Generator` | The thread that blocks in `waitpid`/`getEvent`, feeds raw `ArchEvent`s to decoders. |
| `Decoder` (`DecoderLinux`) | Turns one `ArchEvent` into one or more `Event`s, **stamping** each with its `Process`/`Thread` wrapper. |
| `HandlerPool` / `Handler` | The **state machine**: per-event dispatch to `Handler::handleEvent` implementations that mutate int-layer state. |
| `MTManager` (`mt()`) | The **Multi-Threading Manager**: owns the thread mode, the **handler thread** (`evhandler_main`), and the global **`work_lock`**. |
| `int_notify` (`notify()`) | A self-pipe used to wake a blocked `waitpid`/poll (edge between generator and the OS wait). |
| `response` / `getResponses()` | Side channel for **async operations** (memory/register I/O): the user thread parks on a `response`; the handler marks it ready. |
| `Counter`s | Global counters (`PendingStops`, `AsyncEvents`, `ClearingBPs`, …) that gate the handler's blocking decision. |
| `ProcStopEventManager` | Holds "procStopper" events until the whole process is stopped, then re-queues them. |

## Layer 1 — the object model (who owns whom)

```
   user code
      │ holds
      ▼
   Process::ptr ───────────► int_process ───► threadPool ──► int_thread(s)
   (wrapper, refcounted)     (impl)                              ▲
      ▲   owns impl (RAII backstop + destroyProcess)             │
      │                                                          │
   ProcessPool  ──registers pid→wrapper──►                Thread::ptr (wrapper)
                                                          └─ weak_ptr back to its Process
```

`Process`/`Thread` are the only things allowed to reach `int_process`/
`int_thread` (via `ProcImplRef`/`ThreadImplRef`, which resolve-and-lock).
`ProcessPool` keeps the wrapper alive; `~Process` tears down the impl.

## Layer 2 — the three threads (what `MTManager` arranges)

In the default `HandlerThreading` mode there are three kinds of thread:

1. **User thread(s)** — call the public API (`continueProc`, `readMemory`, …).
2. **Generator thread** — one, owned by `MTManager`; loops in `getEvent` (`waitpid`).
3. **Handler thread** — one, `MTManager::evhandler_main`; drains the mailbox.

(In `NoThreads`/`GeneratorThreading` modes the user thread does the handler's
work itself — same code, fewer threads.)

## The money shot — one event, end to end

The user stops a thread, and the inferior stops:

```
USER THREAD                         GENERATOR THREAD              HANDLER THREAD (evhandler_main)
───────────                         ────────────────              ──────────────────────────────
Process::continueProc/stopProc
  MTLock (work_lock)  ┐
  proc_lock(P)        │ set int_thread target-state
  throwNopEvent       │ (a Nop event to kick the loop)
  release proc_lock   ┘
  waitAndHandleForProc ····park····
                                    getEvent()  ← waitpid: OS
                                      reports SIGSTOP
                                    decoder->decode(archEvent)
                                      → Event(EventStop)
                                      stamp Process P + Thread T
                                      (ProcPool lookup)
                                    proc->updateSyncState(ev)
                                    mbox()->enqueue(ev)
                                    fire generator CBs ──────────► MTManager::eventqueue_cb
                                                                     signals pending_event_lock
                                                                   evhandler_main wakes:
                                                                   startWork()  ← work_lock
                                                                   waitAndHandleEvents(false):
                                                                     should_block? (Counters:
                                                                        PendingStops, AsyncEvents…)
                                                                     ev = mbox()->dequeue()
                                                                     ┌ ProcScopeLock handle_plock(P)
                                                                     │ ProcStopMgr::prepEvent(ev)
                                                                     │ HandlerPool::handleEvent(ev)
                                                                     │   → HandleThreadStop, …
                                                                     │     mutate int_thread state,
                                                                     │     clear pending_stop
                                                                     │ syncRunState()  ← decides
                                                                     │   next continues/stops,
                                                                     │   may plat_stop()/plat_cont()
                                                                     └ (release handle_plock)
                                                                   HandleCallbacks::deliverCallback:
                                                                     suspend proc_lock + take cb slot
                                                                     → user's registered callback()
                                                                   endWork()  ← release work_lock
  ····wakes when target reached····
  returns
```

Key points:

- **The mailbox is the seam.** Generator *produces* (enqueue), handler
  *consumes* (dequeue). One global FIFO, so events across all processes are
  serialized through it.
- **`work_lock` makes one handler *drain-batch* mutually exclusive with each
  user op** — the global serialization being migrated off of. `startWork`/
  `endWork` bracket the whole `waitAndHandleEvents` drain; user API entry
  points take the same lock via `MTLock`.
- **`syncRunState`** is the heart of the state machine: after a handler updates
  state it reconciles "what should be running vs stopped" and issues the ptrace
  continues/stops (`plat_cont`/`plat_stop`) — which *generates the next OS
  event*, closing the loop.
- **`updateSyncState` / `Counter`s** are how the handler decides whether to
  block for more events (e.g. `PendingStops>0` = "a stop I asked for hasn't
  arrived — keep waiting").

## The async side channel (`response`)

Memory/register I/O does not go through a user callback — it uses `response`:

```
USER: readMemory → build mem_response → llproc_->readMem(req) → int_process::waitForAsyncEvent(resp)
        → getResponses().waitFor(resp)  ····parks····
GENERATOR/HANDLER: the reply arrives as an event → handler fills the response → resp->markReady()
USER: waitFor wakes, reads the data
```

This is why `readMemory` takes `proc_lock` only for the *issue* and releases
before the wait — completion is signaled through the response's own condvar,
independent of `work_lock`.

## Coordination primitives (the leaves)

- **`pending_event_lock`** (in `MTManager`) — condvar the handler thread sleeps
  on; signaled by the generator's enqueue callback.
- **`int_notify` pipe** — wakes a blocked `waitpid`/poll when something needs
  the generator's attention (`wakeGenerator()`).
- **`mailbox` `message_cond`** — guards the queue itself.
- **`gen_wait_cv`** — generator's own wait state.
- **`Counter::locks[]`** — guard the global counters.

## Event thread/process wiring (a gotcha)

`Event` stores `thread` and `proc` as **independent** members. The constructor
links them (`proc = thread ? thread->getProcess() : null`), but `setThread`/
`setProcess` each touch only their own field, so after construction they are
decoupled. The decoder stamps both at the end of decode (`event->setThread`,
`event->setProcess`). Runtime rule for consumers: **a dequeued event always has
a non-null process** (`waitAndHandleEvents` derefs `getProcess()->llproc()`
without a null check), but **`getThread()` may be null** for process-scoped
events (process stop, exit, fork/exec, library) — always guard it.

## How this ties to the lock migration

The migration replaces the one **`work_lock`** (global serializer between user
ops and the handler batch) with per-`Process` **`proc_lock`**, so ops on
*different* processes run concurrently. The handler already brackets each event
with `proc_lock(P)` (`handle_plock`); the user-side state-machine methods now do
too. The remaining flip removes `work_lock`, relying on: per-process `proc_lock`
for exclusion, the **kernel round-trip** for cross-thread ordering
(arm→SIGSTOP→decode→handle), and the two-form split (internal lock-free
accessors for the generator/leaf contexts that cannot take `proc_lock`).
