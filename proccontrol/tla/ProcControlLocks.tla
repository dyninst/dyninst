------------------------- MODULE ProcControlLocks -------------------------
(***************************************************************************)
(* Phase A+B model of the proccontrol threading/locking design on the     *)
(* prototype-pool-owns-wrapper branch (option (ii): condvar retired to a  *)
(* registration lock; per-process proc_lock via ImplRef accessors), plus  *)
(* Phase B: callback delivery under a new global cb_lock.                 *)
(*                                                                        *)
(* Actors:                                                                *)
(*   os         - once the debuggee is registered and running (bootstrap  *)
(*                handled), it keeps generating events (threads, signals) *)
(*                bounded by MaxInFlight so the state space is finite     *)
(*   generator  - waitpid -> registration-barrier lookup -> proc_lock     *)
(*                across decode -> statesync -> enqueue                   *)
(*   handler    - work_lock -> dequeue -> locked prologue -> handle       *)
(*   user (usr) - bootstrap (registration lock across create+register,   *)
(*                then park for the bootstrap event), then a loop (at     *)
(*                most MaxUsrIter iterations) of: an infMalloc-style op   *)
(*                (post request, park for response) followed by a         *)
(*                getRegister-style op (leaf lock + response plumbing).   *)
(*                Each time a park is resolved, the thread DELIVERS the   *)
(*                corresponding user CALLBACK before proceeding.          *)
(*   client (usr2) - the client-side path (PCEventMuxer::dequeue ->       *)
(*                getData): loop (bounded MaxUsrIter): take its own       *)
(*                clientLock (its event-mailbox mutex), call a const      *)
(*                proccontrol API (blocks until workLock free,            *)
(*                take+release), release clientLock, then deliver one     *)
(*                callback per iteration under the same delivery rules.   *)
(*                                                                        *)
(* Callback delivery (HandleCallbacks::deliverCallback):                  *)
(*   design (BUG8 off): assert workLock/procLock NOT held by the caller,  *)
(*     take cbLock, enter in_cb, acquire+release clientLock (the client's *)
(*     callback code enqueues into its mailbox), leave in_cb, release     *)
(*     cbLock.  cbLock also enforces the documented one-callback-at-a-    *)
(*     time contract.                                                     *)
(*   BUG8_CallbackUnderWorkLock: the OLD code - deliver while HOLDING     *)
(*     workLock, acquire clientLock inside.  Inverts the client path's    *)
(*     clientLock -> workLock order: ABBA against usr2.  (The old code    *)
(*     had no cbLock / in_cb notion, so those states are not marked       *)
(*     in_cb; the new-design invariants range over design-path states.)   *)
(*                                                                        *)
(* Lock acquisitions are in hold-and-wait form: an actor first acquires   *)
(* the outer lock (its own PC state), then a SEPARATE action blocks       *)
(* until the inner lock is free.  This makes ABBA inversions              *)
(* representable as real deadlocks.                                       *)
(*                                                                        *)
(* Each historical failure is a Boolean knob; TLC must find the bug when  *)
(* the knob is on, and prove the current design (all knobs off) sound:    *)
(*   BUG1_GenHoldsRegLockAcrossDecode - attempt 1's ABBA (generator takes *)
(*        regLock, THEN blocks on decode's proc_lock while handlers take  *)
(*        regLock transitively inside their locked prologue).             *)
(*   BUG2_NoLookupBarrier - attempt 2's attach hang (decode of an         *)
(*        unregistered pid is dropped; bootstrap waits forever; a         *)
(*        never-started debuggee produces no further events).             *)
(*   BUG3_MallocHoldsProcLock - attempt 3 (Codegen held proc_lock across  *)
(*        infMalloc's blocking wait; generator needs it to decode).       *)
(*        Also caught statically by NoParkWhileHoldingProcLock.           *)
(*   BUG5_PlumbingTakesProcLock - the TSan-found regpool inversion        *)
(*        (leaf lock held, response plumbing blocks on proc_lock; decode  *)
(*        takes them in the opposite order).                              *)
(*   BUG8_CallbackUnderWorkLock - callbacks delivered under workLock      *)
(*        (see above): workLock/clientLock ABBA.                          *)
(*                                                                        *)
(* Check with TLC: deadlock detection on; invariants LockInv,             *)
(* NoParkWhileHoldingProcLock, NoProcControlLockAcrossCallback,           *)
(* OneCallbackAtATime; liveness property Termination.  Fairness: weak     *)
(* fairness per thread for gen/han/usr, STRONG fairness for usr2 (its     *)
(* const-API wait sees workLock flicker as the handler loops forever on   *)
(* spawned events, so WF alone would let usr2 starve and spuriously       *)
(* violate Termination).  OsSpawn is environment and stays unfair.        *)
(***************************************************************************)
EXTENDS Naturals, Sequences, TLC

CONSTANTS
  BUG1_GenHoldsRegLockAcrossDecode,
  BUG2_NoLookupBarrier,
  BUG3_MallocHoldsProcLock,
  BUG5_PlumbingTakesProcLock,
  BUG8_CallbackUnderWorkLock

NoOne == "none"
P == "p1"    \* one target process suffices for every known failure class

MaxInFlight == 3   \* OS spawner bound on osEvents + mailbox
HandledCap  == 3   \* handled saturates here (only thresholds 1 and 2 matter)
MaxUsrIter  == 2   \* loop bound for BOTH user threads

VARIABLES
  \* --- locks: owner name, or NoOne.  procLock is recursive: (owner,count).
  workLock, regLock, leafLock,
  procOwner, procCount,
  cbLock,          \* Phase B: global callback-delivery lock
  clientLock,      \* Phase B: the CLIENT's own mutex (its event mailbox)
  \* --- protocol state
  registered,      \* BOOLEAN: P is in the ProcessPool registry
  osEvents,        \* Nat: pending OS events for P (waitpid supply)
  mailbox,         \* Nat: decoded events awaiting the handler
  handled,         \* Nat: events fully handled (saturates at HandledCap)
  bootstrapDone,   \* BOOLEAN: bootstrap event handled
  mallocDone,      \* BOOLEAN: infMalloc response event handled
  \* --- thread program counters
  genPC, hanPC,
  usrPC, usrIter,
  usrRet,          \* where usr resumes after a callback delivery
  usr2PC, usr2Iter

vars == << workLock, regLock, leafLock, procOwner, procCount,
           cbLock, clientLock,
           registered, osEvents, mailbox, handled, bootstrapDone, mallocDone,
           genPC, hanPC, usrPC, usrIter, usrRet, usr2PC, usr2Iter >>

\* Grouped tuples for UNCHANGED terseness.
protoVars == << registered, osEvents, mailbox, handled,
                bootstrapDone, mallocDone >>
cbVars    == << cbLock, clientLock >>
usrVars   == << usrPC, usrIter, usrRet >>
usr2Vars  == << usr2PC, usr2Iter >>

--------------------------------------------------------------------------
\* Lock helpers.  Acquire actions are enabled only when the lock is free
\* (or, for the recursive procLock, already owned by the acquirer).
AcqProc(t)  == \/ /\ procOwner = NoOne /\ procOwner' = t /\ procCount' = 1
               \/ /\ procOwner = t     /\ procOwner' = t /\ procCount' = procCount + 1
RelProc(t)  == /\ procOwner = t
               /\ IF procCount = 1
                  THEN procOwner' = NoOne /\ procCount' = 0
                  ELSE procOwner' = t /\ procCount' = procCount - 1

--------------------------------------------------------------------------
\* OS: a registered, running debuggee keeps generating events.  Gated on
\* bootstrapDone: a debuggee whose bootstrap event was dropped never got
\* continued, so it produces nothing further (preserves BUG2's hang).
\* Bounded by MaxInFlight to keep the state space finite.
OsSpawn ==
  /\ registered /\ bootstrapDone
  /\ osEvents + mailbox < MaxInFlight
  /\ osEvents' = osEvents + 1
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  registered, mailbox, handled, bootstrapDone, mallocDone,
                  genPC, hanPC, cbVars, usrVars, usr2Vars >>

--------------------------------------------------------------------------
\* GENERATOR: g0 wait -> g1 lookup(reg barrier) -> [g2a take regLock under
\*            BUG1] -> g2b decode(proc_lock) -> g3 statesync(proc_lock)
\*            -> enqueue -> g0
GenWait ==
  /\ genPC = "g0" /\ osEvents > 0
  /\ genPC' = "g1"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, hanPC, cbVars, usrVars, usr2Vars >>

\* Lookup: under the registration barrier unless BUG2 disables it.
\* Unregistered pid -> event dropped (current code's decode behavior).
GenLookup ==
  /\ genPC = "g1"
  /\ IF BUG2_NoLookupBarrier
     THEN /\ IF registered
             THEN genPC' = "g2" /\ UNCHANGED osEvents
             ELSE genPC' = "g0" /\ osEvents' = osEvents - 1  \* DROPPED
          /\ UNCHANGED << regLock >>
     ELSE /\ regLock = NoOne              \* blocks while bootstrap holds it
          /\ regLock' = NoOne             \* take + release around the lookup
          /\ IF registered
             THEN genPC' = "g2" /\ UNCHANGED osEvents
             ELSE genPC' = "g0" /\ osEvents' = osEvents - 1  \* stray pid
  /\ UNCHANGED << workLock, leafLock, procOwner, procCount, registered,
                  mailbox, handled, bootstrapDone, mallocDone,
                  hanPC, cbVars, usrVars, usr2Vars >>

\* BUG1 only: the generator FIRST takes regLock (the old condvar held
\* across decode), entering hold-and-wait before it blocks on proc_lock.
GenDecodeRegAcq ==
  /\ BUG1_GenHoldsRegLockAcrossDecode
  /\ genPC = "g2"
  /\ regLock = NoOne /\ regLock' = "gen"
  /\ genPC' = "g2a"
  /\ UNCHANGED << workLock, leafLock, procOwner, procCount,
                  protoVars, hanPC, cbVars, usrVars, usr2Vars >>

\* Decode: blocks on proc_lock.  Under BUG1 the generator is already
\* holding regLock (from g2a) while it waits here - the ABBA half.
GenDecodeAcq ==
  /\ \/ (~BUG1_GenHoldsRegLockAcrossDecode /\ genPC = "g2")
     \/ (BUG1_GenHoldsRegLockAcrossDecode /\ genPC = "g2a")
  /\ AcqProc("gen")
  /\ genPC' = "g2b"
  /\ UNCHANGED << workLock, regLock, leafLock,
                  protoVars, hanPC, cbVars, usrVars, usr2Vars >>

\* Decode body reads registers: leaf lock inside proc_lock (decode order).
GenDecodeBody ==
  /\ genPC = "g2b" /\ leafLock = NoOne
  /\ genPC' = "g3"
  /\ RelProc("gen")
  /\ IF BUG1_GenHoldsRegLockAcrossDecode
     THEN regLock' = NoOne ELSE UNCHANGED regLock
  /\ UNCHANGED << workLock, leafLock,
                  protoVars, hanPC, cbVars, usrVars, usr2Vars >>

\* Statesync: brief proc_lock (the locked accessor), then enqueue.
GenStatesync ==
  /\ genPC = "g3"
  /\ procOwner = NoOne \* acquire+release modeled atomically (no nesting here)
  /\ genPC' = "g0"
  /\ osEvents' = osEvents - 1
  /\ mailbox' = mailbox + 1
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  registered, handled, bootstrapDone, mallocDone,
                  hanPC, cbVars, usrVars, usr2Vars >>

--------------------------------------------------------------------------
\* HANDLER: h0 take work_lock+dequeue -> h1 locked prologue -> h2 handle
\*          (BUG1: blocks on regLock inside; models intCont's old condvar)
\*          -> h3 release, publish outcome
HanDequeue ==
  /\ hanPC = "h0" /\ mailbox > 0 /\ workLock = NoOne
  /\ workLock' = "han" /\ hanPC' = "h1"
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, cbVars, usrVars, usr2Vars >>

HanPrologue ==
  /\ hanPC = "h1"
  /\ AcqProc("han")
  /\ hanPC' = "h2"
  /\ UNCHANGED << workLock, regLock, leafLock,
                  protoVars, genPC, cbVars, usrVars, usr2Vars >>

\* Handle body: register reads (leaf under proc - decode order), and under
\* BUG1 an intCont-style regLock acquisition while holding proc_lock.
HanHandle ==
  /\ hanPC = "h2"
  /\ leafLock = NoOne
  /\ IF BUG1_GenHoldsRegLockAcrossDecode
     THEN regLock = NoOne   \* blocks if generator holds it: the ABBA
     ELSE TRUE
  /\ hanPC' = "h3"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, cbVars, usrVars, usr2Vars >>

HanFinish ==
  /\ hanPC = "h3"
  /\ RelProc("han")
  /\ workLock' = NoOne
  /\ mailbox' = mailbox - 1
  /\ handled' = IF handled < HandledCap THEN handled + 1 ELSE handled
  /\ hanPC' = "h0"
  \* handled events resolve whichever wait the user is parked on:
  \* bootstrapDone once >= 1 handled in total, mallocDone once >= 2.
  /\ bootstrapDone' = TRUE
  /\ mallocDone' = (handled + 1 >= 2)
  /\ UNCHANGED << regLock, leafLock, registered, osEvents,
                  genPC, cbVars, usrVars, usr2Vars >>

--------------------------------------------------------------------------
\* USER (usr): u0 bootstrap (regLock held across create+register)
\*       u1 park for bootstrap event (holding NOTHING)
\*       u2 infMalloc (BUG3: holds proc_lock across the park)
\*       u3 park for malloc response
\*       u4 getRegister: leaf lock; BUG5: plumbing blocks on proc_lock
\*          while still holding the leaf (hold-and-wait)
\*       u5 iteration done; loop back to u2 up to MaxUsrIter times
\* Each park-resolution routes through the callback delivery states
\* (cb0 -> ... -> usrRet); see the header for the knob-dependent shape.
UsrBootstrapBegin ==
  /\ usrPC = "u0" /\ regLock = NoOne
  /\ regLock' = "usr"
  /\ osEvents' = osEvents + 1      \* plat_create: OS can now deliver events
  /\ usrPC' = "u0b"
  /\ UNCHANGED << workLock, leafLock, procOwner, procCount, registered,
                  mailbox, handled, bootstrapDone, mallocDone,
                  genPC, hanPC, cbVars, usrIter, usrRet, usr2Vars >>

UsrBootstrapRegister ==
  /\ usrPC = "u0b"
  /\ registered' = TRUE
  /\ regLock' = NoOne              \* release before any wait
  /\ usrPC' = "u1"
  /\ UNCHANGED << workLock, leafLock, procOwner, procCount, osEvents,
                  mailbox, handled, bootstrapDone, mallocDone,
                  genPC, hanPC, cbVars, usrIter, usrRet, usr2Vars >>

UsrBootstrapPark ==                 \* waitfor_startup: holds nothing.
  /\ usrPC = "u1" /\ bootstrapDone  \* wakes -> deliver the bootstrap cb
  /\ usrPC' = "cb0" /\ usrRet' = "u2"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrIter, usr2Vars >>

UsrMallocPost ==
  /\ usrPC = "u2"
  /\ IF BUG3_MallocHoldsProcLock
     THEN AcqProc("usr")
     ELSE UNCHANGED << procOwner, procCount >>
  /\ osEvents' = osEvents + 1      \* the RPC completion arrives as an event
  /\ usrPC' = "u3"
  /\ UNCHANGED << workLock, regLock, leafLock, registered, mailbox,
                  handled, bootstrapDone, mallocDone,
                  genPC, hanPC, cbVars, usrIter, usrRet, usr2Vars >>

UsrMallocPark ==                    \* waitAndHandleEvents for the response;
  /\ usrPC = "u3" /\ mallocDone     \* wakes -> deliver the response cb
  /\ IF BUG3_MallocHoldsProcLock
     THEN RelProc("usr")
     ELSE UNCHANGED << procOwner, procCount >>
  /\ usrPC' = "cb0" /\ usrRet' = "u4"
  /\ UNCHANGED << workLock, regLock, leafLock, registered, osEvents,
                  mailbox, handled, bootstrapDone, mallocDone,
                  genPC, hanPC, cbVars, usrIter, usr2Vars >>

\* --- usr callback delivery, design path (BUG8 off): caller must hold no
\* proccontrol lock; cbLock brackets the in_cb section (states cb1, cb2).
UsrCbAcq ==
  /\ usrPC = "cb0" /\ ~BUG8_CallbackUnderWorkLock
  /\ workLock /= "usr" /\ procOwner /= "usr"   \* the design's assertion
  /\ cbLock = NoOne /\ cbLock' = "usr"
  /\ usrPC' = "cb1"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, clientLock,
                  usrIter, usrRet, usr2Vars >>

UsrCbClientAcq ==                   \* the user fn takes the client's mutex
  /\ usrPC = "cb1"
  /\ clientLock = NoOne /\ clientLock' = "usr"
  /\ usrPC' = "cb2"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock,
                  usrIter, usrRet, usr2Vars >>

UsrCbFinish ==
  /\ usrPC = "cb2"
  /\ clientLock' = NoOne /\ cbLock' = NoOne
  /\ usrPC' = usrRet
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, usrIter, usrRet, usr2Vars >>

\* --- usr callback delivery, BUG8 path (the OLD code): deliver while
\* HOLDING workLock; the user fn takes the client's mutex inside.
UsrCbWlAcq ==
  /\ usrPC = "cb0" /\ BUG8_CallbackUnderWorkLock
  /\ workLock = NoOne /\ workLock' = "usr"
  /\ usrPC' = "wb1"
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars,
                  usrIter, usrRet, usr2Vars >>

UsrCbWlClientAcq ==                 \* ABBA half: clientLock inside workLock
  /\ usrPC = "wb1"
  /\ clientLock = NoOne /\ clientLock' = "usr"
  /\ usrPC' = "wb2"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock,
                  usrIter, usrRet, usr2Vars >>

UsrCbWlFinish ==
  /\ usrPC = "wb2"
  /\ clientLock' = NoOne /\ workLock' = NoOne
  /\ usrPC' = usrRet
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock,
                  usrIter, usrRet, usr2Vars >>

\* getRegister: leaf regpool_lock, then response plumbing.  BUG5 makes the
\* plumbing block on proc_lock INSIDE the leaf lock - inverting decode's
\* order (the leaf was taken in the previous action: hold-and-wait).
UsrGetRegAcqLeaf ==
  /\ usrPC = "u4" /\ leafLock = NoOne
  /\ leafLock' = "usr"
  /\ usrPC' = "u4b"
  /\ UNCHANGED << workLock, regLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars,
                  usrIter, usrRet, usr2Vars >>

UsrGetRegPlumbing ==
  /\ usrPC = "u4b"
  /\ IF BUG5_PlumbingTakesProcLock
     THEN AcqProc("usr")            \* blocks if generator holds proc_lock
     ELSE UNCHANGED << procOwner, procCount >>
  /\ usrPC' = "u4c"
  /\ UNCHANGED << workLock, regLock, leafLock,
                  protoVars, genPC, hanPC, cbVars,
                  usrIter, usrRet, usr2Vars >>

UsrGetRegRelease ==
  /\ usrPC = "u4c"
  /\ IF BUG5_PlumbingTakesProcLock
     THEN RelProc("usr")
     ELSE UNCHANGED << procOwner, procCount >>
  /\ leafLock' = NoOne
  /\ usrPC' = "u5"
  /\ UNCHANGED << workLock, regLock,
                  protoVars, genPC, hanPC, cbVars,
                  usrIter, usrRet, usr2Vars >>

UsrLoop ==                          \* re-run malloc+getRegister (bounded)
  /\ usrPC = "u5" /\ usrIter < MaxUsrIter
  /\ usrIter' = usrIter + 1
  /\ usrPC' = "u2"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrRet, usr2Vars >>

--------------------------------------------------------------------------
\* CLIENT (usr2): the dyninstAPI-side loop, bounded MaxUsrIter iterations:
\*   v0 take clientLock -> v1 const proccontrol API (getData: blocks until
\*   workLock free, take+release) -> v2 release clientLock -> v3 deliver
\*   one callback (same knob-dependent rules; in_cb states v4, v5 on the
\*   design path, x4, x5 on the BUG8 path) -> v6 loop or done (vD).
Usr2ClientAcq ==
  /\ usr2PC = "v0"
  /\ clientLock = NoOne /\ clientLock' = "usr2"
  /\ usr2PC' = "v1"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock, usrVars, usr2Iter >>

Usr2ConstAPI ==                     \* getData: workLock inside clientLock
  /\ usr2PC = "v1"
  /\ workLock = NoOne /\ workLock' = NoOne  \* take+release, atomically
  /\ usr2PC' = "v2"
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrVars, usr2Iter >>

Usr2ClientRel ==
  /\ usr2PC = "v2"
  /\ clientLock' = NoOne
  /\ usr2PC' = "v3"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock, usrVars, usr2Iter >>

\* --- usr2 callback delivery, design path (states v4, v5 are in_cb).
Usr2CbAcq ==
  /\ usr2PC = "v3" /\ ~BUG8_CallbackUnderWorkLock
  /\ workLock /= "usr2" /\ procOwner /= "usr2" \* the design's assertion
  /\ cbLock = NoOne /\ cbLock' = "usr2"
  /\ usr2PC' = "v4"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, clientLock, usrVars, usr2Iter >>

Usr2CbClientAcq ==
  /\ usr2PC = "v4"
  /\ clientLock = NoOne /\ clientLock' = "usr2"
  /\ usr2PC' = "v5"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock, usrVars, usr2Iter >>

Usr2CbFinish ==
  /\ usr2PC = "v5"
  /\ clientLock' = NoOne /\ cbLock' = NoOne
  /\ usr2PC' = "v6"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, usrVars, usr2Iter >>

\* --- usr2 callback delivery, BUG8 path (old code, under workLock).
Usr2CbWlAcq ==
  /\ usr2PC = "v3" /\ BUG8_CallbackUnderWorkLock
  /\ workLock = NoOne /\ workLock' = "usr2"
  /\ usr2PC' = "x4"
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrVars, usr2Iter >>

Usr2CbWlClientAcq ==
  /\ usr2PC = "x4"
  /\ clientLock = NoOne /\ clientLock' = "usr2"
  /\ usr2PC' = "x5"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock, usrVars, usr2Iter >>

Usr2CbWlFinish ==
  /\ usr2PC = "x5"
  /\ clientLock' = NoOne /\ workLock' = NoOne
  /\ usr2PC' = "v6"
  /\ UNCHANGED << regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbLock, usrVars, usr2Iter >>

Usr2Loop ==
  /\ usr2PC = "v6" /\ usr2Iter < MaxUsrIter
  /\ usr2Iter' = usr2Iter + 1
  /\ usr2PC' = "v0"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrVars >>

Usr2Done ==
  /\ usr2PC = "v6" /\ usr2Iter = MaxUsrIter
  /\ usr2PC' = "vD"
  /\ UNCHANGED << workLock, regLock, leafLock, procOwner, procCount,
                  protoVars, genPC, hanPC, cbVars, usrVars, usr2Iter >>

--------------------------------------------------------------------------
Init ==
  /\ workLock = NoOne /\ regLock = NoOne /\ leafLock = NoOne
  /\ procOwner = NoOne /\ procCount = 0
  /\ cbLock = NoOne /\ clientLock = NoOne
  /\ registered = FALSE /\ osEvents = 0 /\ mailbox = 0 /\ handled = 0
  /\ bootstrapDone = FALSE /\ mallocDone = FALSE
  /\ genPC = "g0" /\ hanPC = "h0"
  /\ usrPC = "u0" /\ usrIter = 1 /\ usrRet = "u2"
  /\ usr2PC = "v0" /\ usr2Iter = 1

GenNext  == GenWait \/ GenLookup \/ GenDecodeRegAcq \/ GenDecodeAcq
            \/ GenDecodeBody \/ GenStatesync
HanNext  == HanDequeue \/ HanPrologue \/ HanHandle \/ HanFinish
UsrNext  == UsrBootstrapBegin \/ UsrBootstrapRegister \/ UsrBootstrapPark
            \/ UsrMallocPost \/ UsrMallocPark
            \/ UsrCbAcq \/ UsrCbClientAcq \/ UsrCbFinish
            \/ UsrCbWlAcq \/ UsrCbWlClientAcq \/ UsrCbWlFinish
            \/ UsrGetRegAcqLeaf \/ UsrGetRegPlumbing \/ UsrGetRegRelease
            \/ UsrLoop
Usr2Next == Usr2ClientAcq \/ Usr2ConstAPI \/ Usr2ClientRel
            \/ Usr2CbAcq \/ Usr2CbClientAcq \/ Usr2CbFinish
            \/ Usr2CbWlAcq \/ Usr2CbWlClientAcq \/ Usr2CbWlFinish
            \/ Usr2Loop \/ Usr2Done

Next ==
  \/ OsSpawn
  \/ GenNext \/ HanNext \/ UsrNext \/ Usr2Next
  \/ (usrPC = "u5" /\ usrIter = MaxUsrIter /\ usr2PC = "vD"
      /\ UNCHANGED vars)
     \* stutter when both user threads are fully done (TLC deadlock)

\* Per-thread fairness: WF for gen/han/usr; SF for usr2 because its
\* const-API wait sees workLock flicker under the handler's endless
\* spawner-fed loop (WF alone would let usr2 starve).  OsSpawn unfair.
Spec == Init /\ [][Next]_vars
        /\ WF_vars(GenNext) /\ WF_vars(HanNext) /\ WF_vars(UsrNext)
        /\ SF_vars(Usr2Next)

\* Liveness: both user threads' whole scenarios complete.
Termination == <>(usrPC = "u5") /\ <>(usr2PC = "vD")

\* Safety: recursive procLock consistency.
LockInv == (procOwner = NoOne) <=> (procCount = 0)

\* Safety: the entry-point-lock design rule - never park waiting for an
\* event outcome while holding the per-process proc_lock.
NoParkWhileHoldingProcLock == (usrPC \in {"u1", "u3"}) => (procOwner /= "usr")

\* Phase B design rules.  in_cb = the design-path delivery states (the old
\* BUG8 code predates the cbLock/in_cb notion, so wb*/x* do not count).
InCbUsr  == usrPC  \in {"cb1", "cb2"}
InCbUsr2 == usr2PC \in {"v4", "v5"}

\* No proccontrol lock may be held across a user callback.
NoProcControlLockAcrossCallback ==
  /\ InCbUsr  => (workLock /= "usr"  /\ procOwner /= "usr")
  /\ InCbUsr2 => (workLock /= "usr2" /\ procOwner /= "usr2")

\* The documented contract: at most one callback in flight at a time.
OneCallbackAtATime == ~(InCbUsr /\ InCbUsr2)
=============================================================================
