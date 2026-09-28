------------------------- MODULE ProcControlLocks -------------------------
(***************************************************************************)
(* Phase C model of the proccontrol threading/locking design.  This       *)
(* extends the earlier phases to model the ACTUAL removal of work_lock     *)
(* (the S4/S5 concurrency flip) so the design can be proven deadlock- and  *)
(* race-free before it is implemented.                                     *)
(*                                                                        *)
(* S4/S5 design under test:                                               *)
(*   - work_lock is REMOVED entirely.  Boundary access serializes         *)
(*     per-process on proc_lock (the accessors' job).  Two API calls on    *)
(*     DIFFERENT processes now run truly concurrently, so we model TWO     *)
(*     processes P and Q (ascending "pid" order P < Q), each with its own  *)
(*     recursive proc_lock (owner + count).                               *)
(*   - The HANDLER holds proc_lock(proc) ACROSS its per-event handling of  *)
(*     proc, so a user path taking only proc_lock(proc) cannot race it     *)
(*     mid-handle.  deliverCallback RELEASES proc_lock around the callback *)
(*     invocation and re-acquires afterwards.                             *)
(*   - cb_lock gives one-callback-at-a-time globally.                      *)
(*   - in_callback is THREAD-LOCAL: thread A being inside a callback must  *)
(*     not make thread B's unrelated API reject.                          *)
(*                                                                        *)
(* Actors / threads:                                                      *)
(*   os        - a bounded event source (spawnBudget per process)         *)
(*   generator - waitpid -> registration-barrier lookup -> proc_lock       *)
(*               across decode -> statesync -> enqueue (either process)   *)
(*   handler   - dequeues an event for either process and holds THAT       *)
(*               process's proc_lock across handling; delivers the         *)
(*               callback (releasing proc_lock around the invocation)      *)
(*   usr       - operates on P: bootstrap, infMalloc, a proc_lock-guarded  *)
(*               modify, getRegister (leaf lock + plumbing), then a fork   *)
(*               that locks P then Q (ascending)                          *)
(*   usr2      - operates on Q: bootstrap, the client-side PCEventMuxer    *)
(*               dequeue -> getData path (clientLock then const API that   *)
(*               takes proc_lock(Q)), then a fork that also touches both   *)
(*               processes (ascending P then Q; BUG9 flips it)            *)
(*                                                                        *)
(* Boolean knobs (regressions + new S4/S5 bugs):                          *)
(*   BUG1_GenHoldsRegLockAcrossDecode - generator takes regLock then       *)
(*       blocks on decode's proc_lock while the handler takes regLock      *)
(*       inside its (proc_lock-holding) handle body: regLock/proc ABBA.    *)
(*   BUG2_NoLookupBarrier - decode of an unregistered pid is dropped;      *)
(*       the bootstrap event is lost and usr waits forever.               *)
(*   BUG3_MallocHoldsProcLock - usr holds proc_lock(P) across the malloc   *)
(*       park; caught by NoParkWhileHoldingProcLock, and deadlocks.        *)
(*   BUG5_PlumbingTakesProcLock - getRegister holds the leaf map_lock and  *)
(*       blocks on proc_lock(P); decode takes them the other way: ABBA.    *)
(*   BUG8_CallbackUnderWorkLock - now "handler holds proc_lock(proc)       *)
(*       ACROSS the callback invocation" (work_lock is gone).  Inverts     *)
(*       the client path's clientLock -> proc_lock order: ABBA vs usr2.    *)
(*       Caught by NoProcControlLockAcrossCallback, and deadlocks.        *)
(*   BUG9_ForkLockOrder - usr2's fork locks Q then P (descending) while    *)
(*       usr locks P then Q: the HandlePostFork/HandlePostForkCont ABBA.   *)
(*   BUG10_HandlerNoProcLock - the handler does NOT hold proc_lock(proc)   *)
(*       across handling, so it can mutate proc concurrently with a user   *)
(*       thread that holds proc_lock(proc): AtMostOneWriterPerProc.        *)
(*   BUG11_GlobalInCallback - in_callback is GLOBAL, so a user thread's    *)
(*       API is wrongly rejected while ANOTHER thread is in a callback:    *)
(*       SpuriousRejectFree (and/or a Termination hazard).                *)
(*                                                                        *)
(* Check with TLC: deadlock detection on; invariants LockInv,             *)
(* NoParkWhileHoldingProcLock, NoProcControlLockAcrossCallback,           *)
(* OneGlobalCallbackViaCbLock, AtMostOneWriterPerProc, SpuriousRejectFree; *)
(* liveness Termination.  Fairness: strong fairness per thread group (the  *)
(* threads wait on locks/flags that flicker under contention); the OS      *)
(* source is bounded and left unfair.                                     *)
(***************************************************************************)
EXTENDS Naturals

CONSTANTS
  BUG1_GenHoldsRegLockAcrossDecode,
  BUG2_NoLookupBarrier,
  BUG3_MallocHoldsProcLock,
  BUG5_PlumbingTakesProcLock,
  BUG8_CallbackUnderWorkLock,
  BUG9_ForkLockOrder,
  BUG10_HandlerNoProcLock,
  BUG11_GlobalInCallback

NoOne == "none"
Procs == {"P", "Q"}
Threads == {"usr", "usr2", "han"}

MaxInFlight     == 2   \* per-process bound on osEvents + mailbox
HandledCap      == 3   \* handled[p] saturates here (only 1 and 2 matter)
MaxSpawnPerProc == 1   \* bounded OS event source (keeps the space finite)

VARIABLES
  \* --- per-process recursive proc_lock: owner name or NoOne, plus count.
  procOwner, procCount,
  \* --- global locks (owner name or NoOne).
  regLock,     \* registration / ProcessPool lock
  leafLock,    \* the map_lock leaf (regpool)
  cbLock,      \* global callback-delivery lock (one callback at a time)
  clientLock,  \* the CLIENT's own mutex (its event mailbox)
  inCb,        \* [Threads -> BOOLEAN]: thread-local in_callback flag
  \* --- per-process protocol state
  registered, osEvents, mailbox, handled, bootstrapDone, mallocDone,
  spawnBudget,
  \* --- thread program counters (+ which process each is working on)
  genPC, genProc, hanPC, hanProc, usrPC, usr2PC

vars == << procOwner, procCount, regLock, leafLock, cbLock, clientLock, inCb,
           registered, osEvents, mailbox, handled, bootstrapDone, mallocDone,
           spawnBudget, genPC, genProc, hanPC, hanProc, usrPC, usr2PC >>

\* Grouped tuples for terse UNCHANGED (used only when a whole group is idle).
allLocks == << procOwner, procCount, regLock, leafLock, cbLock, clientLock,
               inCb >>
allProc  == << registered, osEvents, mailbox, handled, bootstrapDone,
               mallocDone, spawnBudget >>
allPC    == << genPC, genProc, hanPC, hanProc, usrPC, usr2PC >>

--------------------------------------------------------------------------
\* Per-process recursive lock helpers (assign procOwner and procCount).
AcqProc(t, p) ==
  \/ /\ procOwner[p] = NoOne
     /\ procOwner' = [procOwner EXCEPT ![p] = t]
     /\ procCount' = [procCount EXCEPT ![p] = 1]
  \/ /\ procOwner[p] = t
     /\ procOwner' = [procOwner EXCEPT ![p] = t]
     /\ procCount' = [procCount EXCEPT ![p] = procCount[p] + 1]

RelProc(t, p) ==
  /\ procOwner[p] = t
  /\ IF procCount[p] = 1
     THEN /\ procOwner' = [procOwner EXCEPT ![p] = NoOne]
          /\ procCount' = [procCount EXCEPT ![p] = 0]
     ELSE /\ procOwner' = [procOwner EXCEPT ![p] = t]
          /\ procCount' = [procCount EXCEPT ![p] = procCount[p] - 1]

ProcUnchanged == UNCHANGED << procOwner, procCount >>

\* Release both process locks at once (fork exit; caller holds both, count 1).
RelBoth(t) ==
  /\ procOwner["P"] = t /\ procOwner["Q"] = t
  /\ procOwner' = [procOwner EXCEPT !["P"] = NoOne, !["Q"] = NoOne]
  /\ procCount' = [procCount EXCEPT !["P"] = 0,     !["Q"] = 0]

\* API rejection: in_callback is thread-local (design) or global (BUG11).
ApiBlocked(t) == IF BUG11_GlobalInCallback
                 THEN \E s \in Threads : inCb[s]
                 ELSE inCb[t]
ApiAllowed(t) == ~ApiBlocked(t)

--------------------------------------------------------------------------
\* OS: bounded per-process event source, gated on a running debuggee.
OsSpawn ==
  /\ \E p \in Procs :
        /\ spawnBudget[p] > 0
        /\ registered[p] /\ bootstrapDone[p]
        /\ osEvents[p] + mailbox[p] < MaxInFlight
        /\ osEvents' = [osEvents EXCEPT ![p] = osEvents[p] + 1]
        /\ spawnBudget' = [spawnBudget EXCEPT ![p] = spawnBudget[p] - 1]
  /\ UNCHANGED allLocks
  /\ UNCHANGED << registered, mailbox, handled, bootstrapDone, mallocDone >>
  /\ UNCHANGED allPC

--------------------------------------------------------------------------
\* GENERATOR (works genProc): g0 wait -> g1 lookup(reg barrier) ->
\* [g2a take regLock under BUG1] -> g2b decode(proc_lock) -> g3 statesync
\* -> enqueue -> g0.
GenWait ==
  /\ genPC = "g0"
  /\ \E p \in Procs : osEvents[p] > 0 /\ genProc' = p
  /\ genPC' = "g1"
  /\ UNCHANGED allLocks /\ UNCHANGED allProc
  /\ UNCHANGED << hanPC, hanProc, usrPC, usr2PC >>

GenLookup ==
  /\ genPC = "g1"
  /\ IF BUG2_NoLookupBarrier
     THEN /\ regLock' = regLock                 \* no barrier taken
          /\ IF registered[genProc]
             THEN genPC' = "g2" /\ osEvents' = osEvents
             ELSE /\ genPC' = "g0"               \* DROPPED (stray/early pid)
                  /\ osEvents' = [osEvents EXCEPT ![genProc]=osEvents[genProc]-1]
     ELSE /\ regLock = NoOne /\ regLock' = NoOne \* take+release around lookup
          /\ IF registered[genProc]
             THEN genPC' = "g2" /\ osEvents' = osEvents
             ELSE /\ genPC' = "g0"
                  /\ osEvents' = [osEvents EXCEPT ![genProc]=osEvents[genProc]-1]
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << registered, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genProc, hanPC, hanProc, usrPC, usr2PC >>

GenDecodeRegAcq ==                    \* BUG1 only: grab regLock before decode
  /\ BUG1_GenHoldsRegLockAcrossDecode
  /\ genPC = "g2"
  /\ regLock = NoOne /\ regLock' = "gen"
  /\ genPC' = "g2a"
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genProc, hanPC, hanProc, usrPC, usr2PC >>

GenDecodeAcq ==                       \* block on proc_lock(genProc)
  /\ \/ (~BUG1_GenHoldsRegLockAcrossDecode /\ genPC = "g2")
     \/ (BUG1_GenHoldsRegLockAcrossDecode  /\ genPC = "g2a")
  /\ AcqProc("gen", genProc)
  /\ genPC' = "g2b"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genProc, hanPC, hanProc, usrPC, usr2PC >>

GenDecodeBody ==                      \* leaf under proc (decode order)
  /\ genPC = "g2b" /\ leafLock = NoOne
  /\ genPC' = "g3"
  /\ RelProc("gen", genProc)
  /\ IF BUG1_GenHoldsRegLockAcrossDecode THEN regLock' = NoOne
                                         ELSE regLock' = regLock
  /\ UNCHANGED << leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genProc, hanPC, hanProc, usrPC, usr2PC >>

GenStatesync ==                       \* brief locked accessor, then enqueue
  /\ genPC = "g3" /\ procOwner[genProc] = NoOne
  /\ genPC' = "g0"
  /\ osEvents' = [osEvents EXCEPT ![genProc] = osEvents[genProc] - 1]
  /\ mailbox'  = [mailbox  EXCEPT ![genProc] = mailbox[genProc] + 1]
  /\ UNCHANGED allLocks
  /\ UNCHANGED << registered, handled, bootstrapDone, mallocDone, spawnBudget >>
  /\ UNCHANGED << genProc, hanPC, hanProc, usrPC, usr2PC >>

--------------------------------------------------------------------------
\* HANDLER (works hanProc): holds proc_lock(hanProc) across handling; the
\* callback invocation releases it (design) and re-acquires afterwards.
HanPick ==
  /\ hanPC = "h0"
  /\ \E p \in Procs :
        /\ mailbox[p] > 0
        /\ hanProc' = p
        /\ IF BUG10_HandlerNoProcLock THEN ProcUnchanged ELSE AcqProc("han", p)
  /\ hanPC' = "h1"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, usrPC, usr2PC >>

HanHandle ==                          \* handle body: WRITER region for hanProc
  /\ hanPC = "h1"
  /\ leafLock = NoOne
  /\ IF BUG1_GenHoldsRegLockAcrossDecode THEN regLock = NoOne ELSE TRUE
  /\ hanPC' = "h2"
  /\ UNCHANGED allLocks /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanProc, usrPC, usr2PC >>

HanCbBegin ==                         \* deliverCallback: release proc, take cb
  /\ hanPC = "h2"
  /\ IF ~BUG8_CallbackUnderWorkLock /\ ~BUG10_HandlerNoProcLock
     THEN RelProc("han", hanProc)     \* clause 3: release around the callback
     ELSE ProcUnchanged               \* BUG8 keeps it held (BUG10 never held)
  /\ cbLock = NoOne /\ cbLock' = "han"
  /\ inCb' = [inCb EXCEPT !["han"] = TRUE]
  /\ hanPC' = "hcb"
  /\ UNCHANGED << regLock, leafLock, clientLock >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanProc, usrPC, usr2PC >>

HanCbClient ==                        \* the user fn enqueues via clientLock
  /\ hanPC = "hcb"
  /\ clientLock = NoOne /\ clientLock' = "han"
  /\ hanPC' = "hcb2"
  /\ UNCHANGED << procOwner, procCount, regLock, leafLock, cbLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanProc, usrPC, usr2PC >>

HanCbFinish ==
  /\ hanPC = "hcb2"
  /\ clientLock' = NoOne
  /\ inCb' = [inCb EXCEPT !["han"] = FALSE]
  /\ cbLock' = NoOne
  /\ hanPC' = "h3"
  /\ UNCHANGED << procOwner, procCount, regLock, leafLock >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanProc, usrPC, usr2PC >>

HanFinish ==                          \* re-acquire proc, publish, release
  /\ hanPC = "h3"
  /\ IF BUG10_HandlerNoProcLock
     THEN ProcUnchanged                          \* never held it
     ELSE IF BUG8_CallbackUnderWorkLock
          THEN RelProc("han", hanProc)            \* held through cb; drop now
          ELSE /\ procOwner[hanProc] = NoOne      \* design: re-acquire+release
               /\ ProcUnchanged
  /\ mailbox' = [mailbox EXCEPT ![hanProc] = mailbox[hanProc] - 1]
  /\ handled' = [handled EXCEPT ![hanProc] =
                   IF handled[hanProc] < HandledCap THEN handled[hanProc] + 1
                                                    ELSE handled[hanProc]]
  /\ bootstrapDone' = [bootstrapDone EXCEPT ![hanProc] = TRUE]
  /\ mallocDone' = [mallocDone EXCEPT ![hanProc] =
                      mallocDone[hanProc] \/ (handled[hanProc] + 1 >= 2)]
  /\ hanPC' = "h0"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << registered, osEvents, spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanProc, usrPC, usr2PC >>

--------------------------------------------------------------------------
\* USER usr (operates on P).
UsrBootstrapBegin ==
  /\ usrPC = "u0" /\ regLock = NoOne
  /\ regLock' = "usr"
  /\ osEvents' = [osEvents EXCEPT !["P"] = osEvents["P"] + 1]
  /\ usrPC' = "u0b"
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << registered, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrBootstrapRegister ==
  /\ usrPC = "u0b"
  /\ registered' = [registered EXCEPT !["P"] = TRUE]
  /\ regLock' = NoOne
  /\ usrPC' = "u1"
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << osEvents, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrBootstrapPark ==
  /\ usrPC = "u1" /\ bootstrapDone["P"]
  /\ usrPC' = "u2"
  /\ UNCHANGED allLocks /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrMallocPost ==                      \* API entry: guarded by ApiAllowed
  /\ usrPC = "u2" /\ ApiAllowed("usr")
  /\ IF BUG3_MallocHoldsProcLock THEN AcqProc("usr", "P") ELSE ProcUnchanged
  /\ osEvents' = [osEvents EXCEPT !["P"] = osEvents["P"] + 1]
  /\ usrPC' = "u3"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << registered, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrMallocPark ==
  /\ usrPC = "u3" /\ mallocDone["P"]
  /\ IF BUG3_MallocHoldsProcLock THEN RelProc("usr", "P") ELSE ProcUnchanged
  /\ usrPC' = "um0"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrModifyAcq ==                       \* take proc_lock(P) for a mutation
  /\ usrPC = "um0"
  /\ AcqProc("usr", "P")
  /\ usrPC' = "um1"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrModifyRel ==                       \* um1 is a WRITER region for P
  /\ usrPC = "um1"
  /\ RelProc("usr", "P")
  /\ usrPC' = "u4"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrGetRegAcqLeaf ==
  /\ usrPC = "u4" /\ leafLock = NoOne
  /\ leafLock' = "usr"
  /\ usrPC' = "u4b"
  /\ UNCHANGED << procOwner, procCount, regLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrGetRegPlumbing ==                  \* BUG5: proc_lock INSIDE the leaf lock
  /\ usrPC = "u4b"
  /\ IF BUG5_PlumbingTakesProcLock THEN AcqProc("usr", "P") ELSE ProcUnchanged
  /\ usrPC' = "u4c"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrGetRegRelease ==
  /\ usrPC = "u4c"
  /\ IF BUG5_PlumbingTakesProcLock THEN RelProc("usr", "P") ELSE ProcUnchanged
  /\ leafLock' = NoOne
  /\ usrPC' = "uf0"
  /\ UNCHANGED << regLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrForkAcqP ==                        \* fork: ascending P then Q
  /\ usrPC = "uf0"
  /\ AcqProc("usr", "P")
  /\ usrPC' = "uf1"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrForkAcqQ ==
  /\ usrPC = "uf1"
  /\ AcqProc("usr", "Q")
  /\ usrPC' = "uf2"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

UsrForkRel ==
  /\ usrPC = "uf2"
  /\ RelBoth("usr")
  /\ usrPC' = "uDone"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usr2PC >>

--------------------------------------------------------------------------
\* CLIENT usr2 (operates on Q): bootstrap, PCEventMuxer client path, fork.
Usr2BootstrapBegin ==
  /\ usr2PC = "v0" /\ regLock = NoOne
  /\ regLock' = "usr2"
  /\ osEvents' = [osEvents EXCEPT !["Q"] = osEvents["Q"] + 1]
  /\ usr2PC' = "v0b"
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << registered, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2BootstrapRegister ==
  /\ usr2PC = "v0b"
  /\ registered' = [registered EXCEPT !["Q"] = TRUE]
  /\ regLock' = NoOne
  /\ usr2PC' = "v1"
  /\ UNCHANGED << procOwner, procCount, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED << osEvents, mailbox, handled, bootstrapDone, mallocDone,
                  spawnBudget >>
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2BootstrapPark ==
  /\ usr2PC = "v1" /\ bootstrapDone["Q"]
  /\ usr2PC' = "v2"
  /\ UNCHANGED allLocks /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2ClientAcq ==                      \* take the client's own mutex
  /\ usr2PC = "v2"
  /\ clientLock = NoOne /\ clientLock' = "usr2"
  /\ usr2PC' = "v3"
  /\ UNCHANGED << procOwner, procCount, regLock, leafLock, cbLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2ConstAPI ==                       \* getData: const API takes proc_lock(Q)
  /\ usr2PC = "v3" /\ ApiAllowed("usr2")   \* API entry guard
  /\ procOwner["Q"] = NoOne             \* take+release proc_lock(Q) (blocking)
  /\ usr2PC' = "v4"
  /\ UNCHANGED allLocks /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2ClientRel ==
  /\ usr2PC = "v4"
  /\ clientLock' = NoOne
  /\ usr2PC' = "vf0"
  /\ UNCHANGED << procOwner, procCount, regLock, leafLock, cbLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

\* Fork: ascending (P then Q) by design; BUG9 flips usr2 to (Q then P).
Usr2ForkAcqA ==
  /\ usr2PC = "vf0"
  /\ AcqProc("usr2", IF BUG9_ForkLockOrder THEN "Q" ELSE "P")
  /\ usr2PC' = "vf1"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2ForkAcqB ==
  /\ usr2PC = "vf1"
  /\ AcqProc("usr2", IF BUG9_ForkLockOrder THEN "P" ELSE "Q")
  /\ usr2PC' = "vf2"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

Usr2ForkRel ==
  /\ usr2PC = "vf2"
  /\ RelBoth("usr2")
  /\ usr2PC' = "vDone"
  /\ UNCHANGED << regLock, leafLock, cbLock, clientLock, inCb >>
  /\ UNCHANGED allProc
  /\ UNCHANGED << genPC, genProc, hanPC, hanProc, usrPC >>

--------------------------------------------------------------------------
Init ==
  /\ procOwner = [p \in Procs |-> NoOne] /\ procCount = [p \in Procs |-> 0]
  /\ regLock = NoOne /\ leafLock = NoOne /\ cbLock = NoOne /\ clientLock = NoOne
  /\ inCb = [t \in Threads |-> FALSE]
  /\ registered = [p \in Procs |-> FALSE]
  /\ osEvents = [p \in Procs |-> 0] /\ mailbox = [p \in Procs |-> 0]
  /\ handled = [p \in Procs |-> 0]
  /\ bootstrapDone = [p \in Procs |-> FALSE]
  /\ mallocDone = [p \in Procs |-> FALSE]
  /\ spawnBudget = [p \in Procs |-> MaxSpawnPerProc]
  /\ genPC = "g0" /\ genProc = "P"
  /\ hanPC = "h0" /\ hanProc = "P"
  /\ usrPC = "u0" /\ usr2PC = "v0"

GenNext ==
  \/ GenWait \/ GenLookup \/ GenDecodeRegAcq \/ GenDecodeAcq
  \/ GenDecodeBody \/ GenStatesync

HanNext ==
  \/ HanPick \/ HanHandle \/ HanCbBegin \/ HanCbClient
  \/ HanCbFinish \/ HanFinish

UsrNext ==
  \/ UsrBootstrapBegin \/ UsrBootstrapRegister \/ UsrBootstrapPark
  \/ UsrMallocPost \/ UsrMallocPark \/ UsrModifyAcq \/ UsrModifyRel
  \/ UsrGetRegAcqLeaf \/ UsrGetRegPlumbing \/ UsrGetRegRelease
  \/ UsrForkAcqP \/ UsrForkAcqQ \/ UsrForkRel

Usr2Next ==
  \/ Usr2BootstrapBegin \/ Usr2BootstrapRegister \/ Usr2BootstrapPark
  \/ Usr2ClientAcq \/ Usr2ConstAPI \/ Usr2ClientRel
  \/ Usr2ForkAcqA \/ Usr2ForkAcqB \/ Usr2ForkRel

Done == usrPC = "uDone" /\ usr2PC = "vDone" /\ UNCHANGED vars

Next ==
  \/ OsSpawn \/ GenNext \/ HanNext \/ UsrNext \/ Usr2Next
  \/ Done

\* Strong fairness per thread group: each waits on locks/flags that flicker
\* under contention, so WF could permit spurious starvation; SF forces
\* progress only when the group is enabled infinitely often (so it does NOT
\* mask a genuinely disabled thread, e.g. usr under BUG2).  OsSpawn is a
\* bounded environment source and is left unfair.
Spec == Init /\ [][Next]_vars
        /\ SF_vars(GenNext) /\ SF_vars(HanNext)
        /\ SF_vars(UsrNext) /\ SF_vars(Usr2Next)

--------------------------------------------------------------------------
\* Liveness: both user threads' scenarios complete.  (Each awaits events
\* that only get handled if the handler drains its process, so this also
\* captures "an enqueued event is eventually handled".)
Termination == <>(usrPC = "uDone") /\ <>(usr2PC = "vDone")

\* Safety: recursive proc_lock consistency, per process.
LockInv == \A p \in Procs : (procOwner[p] = NoOne) <=> (procCount[p] = 0)

\* The entry-point-lock rule: never park for an event outcome while holding
\* proc_lock(P) (usr's parks are u1 and u3).
NoParkWhileHoldingProcLock ==
  (usrPC \in {"u1", "u3"}) => (procOwner["P"] /= "usr")

\* Callback invocation region (design path) for the delivering thread.
HanInCb == hanPC \in {"hcb", "hcb2"}

\* No proccontrol lock (proc_lock; work_lock is gone) may be held across a
\* user callback invocation.  BUG8 makes the handler hold proc_lock here.
NoProcControlLockAcrossCallback ==
  HanInCb => (procOwner["P"] /= "han" /\ procOwner["Q"] /= "han")

\* The documented contract: whoever is in a callback owns cb_lock (so
\* cb_lock enforces one-callback-at-a-time globally).
OneGlobalCallbackViaCbLock == \A t \in Threads : inCb[t] => (cbLock = t)

\* Data-race rule: for each process at most one thread is in a "mutating"
\* region unless serialized by proc_lock.  The handler's handle body (h1)
\* and usr's modify body (um1, on P) are the mutators; the design keeps
\* proc_lock held throughout both, so they exclude.  BUG10 drops the
\* handler's proc_lock, allowing a concurrent mutation of P.
AtMostOneWriterPerProc ==
  ~( (hanPC = "h1" /\ hanProc = "P") /\ (usrPC = "um1") )

\* in_callback is thread-local: a thread's API may be blocked ONLY by its
\* own callback, never by another thread's.  BUG11 (global flag) breaks it.
SpuriousRejectFree ==
  /\ (usrPC  = "u2" /\ ApiBlocked("usr"))  => inCb["usr"]
  /\ (usr2PC = "v3" /\ ApiBlocked("usr2")) => inCb["usr2"]
=============================================================================
