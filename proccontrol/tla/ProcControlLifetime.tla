------------------------ MODULE ProcControlLifetime ------------------------
(***************************************************************************)
(* Lifecycle model for the pool-owns-wrapper design: the object-lifetime   *)
(* rules that the lock model (ProcControlLocks) takes for granted.  One    *)
(* process, post-registration, driven to teardown by an exit event, with a *)
(* straggler signal event that may be decoded before or after teardown.    *)
(*                                                                        *)
(* Objects:                                                               *)
(*   impl (int_process)  - implAlloc: allocated until the exit handler     *)
(*                         frees it                                        *)
(*   wrapper (Process)   - wrapperAlloc + refs (refcount) + llproc (the    *)
(*                         wrapper's impl pointer) + exitstate + the       *)
(*                         proc_lock itself (procLock lives IN the         *)
(*                         wrapper: LockInWrapper)                         *)
(*   pool entry          - poolEntry: the ProcPool map slot; holds one     *)
(*                         wrapper ref while present                       *)
(*                                                                        *)
(* Design rules being checked (each is an invariant):                     *)
(*   - NoUAF: every impl deref happens with the impl allocated.  The       *)
(*     design achieves this with the checked-accessor protocol: pin the    *)
(*     wrapper (a ref), take proc_lock, re-check llproc under the lock,    *)
(*     and only then deref -- teardown severs+frees under the same lock.   *)
(*   - SeverBeforeFree / PublishBeforeSever: teardown order is publish     *)
(*     exitstate -> sever llproc -> free impl, all under proc_lock.        *)
(*   - ExitReadSafe: a query that finds llproc null reads exitstate,       *)
(*     which must already be published.                                    *)
(*   - WrapperLastOut: the wrapper is reclaimed only after the impl is     *)
(*     freed, the pool entry is gone and every ref is dropped.             *)
(*   - Reclaimed (liveness): everything is eventually reclaimed (no leak). *)
(*                                                                        *)
(* Threads: gen (decode: lookup -> pin -> proc_lock -> checked deref ->    *)
(* enqueue), han (handle sig / teardown on exit), usr (bounded API calls,  *)
(* then drops its ref).  The registration lock is NOT modeled: its         *)
(* critical sections here are single atomic steps (lookup+pin,            *)
(* deregister), which gives the same exclusion; regLock ORDERING bugs      *)
(* live in ProcControlLocks.                                               *)
(*                                                                        *)
(* Knobs (TLC must catch each):                                           *)
(*   BUGL1_UnlockedUserDeref - the master-native UAF class closed by       *)
(*       921064e/the getPid cache: a public reader checks llproc and       *)
(*       derefs the impl with NO proc_lock; teardown's free interleaves    *)
(*       between check and use.  -> NoUAF.                                 *)
(*   BUGL2_FreeBeforeSever - teardown frees the impl first (outside the    *)
(*       lock), then locks/publishes/severs: a correctly-locked reader     *)
(*       sees llproc still set and derefs freed memory.                    *)
(*       -> SeverBeforeFree (and NoUAF).                                   *)
(*   BUGL3_PoolRefLeak - the detach-leak class: teardown removes the pool  *)
(*       entry but never drops the pool's wrapper ref; refs never reach 0  *)
(*       and the wrapper is never reclaimed.  -> deadlock (system can      *)
(*       never reach the reclaimed terminal state).                        *)
(*   BUGL4_GenDecodeNoProcLock - the generator decodes with the wrapper    *)
(*       pinned but WITHOUT proc_lock: teardown (which holds proc_lock)    *)
(*       frees the impl between the generator's llproc check and its       *)
(*       deref.  -> NoUAF.                                                 *)
(***************************************************************************)
EXTENDS Naturals

CONSTANTS
  BUGL1_UnlockedUserDeref,
  BUGL2_FreeBeforeSever,
  BUGL3_PoolRefLeak,
  BUGL4_GenDecodeNoProcLock

NoOne == "none"

VARIABLES
  genPC, hanPC, usrPC,
  genEv, hanEv,            \* event currently held by gen / han
  sigPending, exitPending, \* OS events awaiting decode
  mbSig, mbExit,           \* decoded events queued for the handler
  procLock,                \* the wrapper's proc_lock: thread name or NoOne
  implAlloc,               \* int_process storage is allocated
  llproc,                  \* wrapper's llproc_ pointer is set
  exitstate,               \* wrapper's exitstate_ is published
  poolEntry,               \* ProcPool map entry present
  refs,                    \* wrapper refcount: pool + usr + pinned events
  wrapperAlloc,            \* Process wrapper storage is allocated
  usrIters                 \* remaining user API calls

vars == << genPC, hanPC, usrPC, genEv, hanEv, sigPending, exitPending,
           mbSig, mbExit, procLock, implAlloc, llproc, exitstate,
           poolEntry, refs, wrapperAlloc, usrIters >>

objs == << implAlloc, llproc, exitstate, poolEntry, refs, wrapperAlloc >>

Init ==
  /\ genPC = "g0" /\ hanPC = "h0" /\ usrPC = "u0"
  /\ genEv = NoOne /\ hanEv = NoOne
  /\ sigPending = TRUE /\ exitPending = TRUE
  /\ mbSig = FALSE /\ mbExit = FALSE
  /\ procLock = NoOne
  /\ implAlloc = TRUE /\ llproc = TRUE
  /\ exitstate = FALSE /\ poolEntry = TRUE
  /\ refs = 2                 \* pool's ref + usr's ref
  /\ wrapperAlloc = TRUE
  /\ usrIters = 2

--------------------------------------------------------------------------
\* Generator: pick a pending event, atomically {lookup, pin} under the
\* (unmodeled-as-atomic) registration lock, take proc_lock, re-check llproc
\* under the lock (the checked accessor), deref for decode, enqueue.

GenPick ==
  /\ genPC = "g0" /\ genEv = NoOne
  /\ \/ (sigPending  /\ genEv' = "sig"  /\ sigPending' = FALSE
                     /\ UNCHANGED exitPending)
     \/ (exitPending /\ genEv' = "exit" /\ exitPending' = FALSE
                     /\ UNCHANGED sigPending)
  /\ genPC' = "gLook"
  /\ UNCHANGED << hanPC, usrPC, hanEv, mbSig, mbExit, procLock, objs,
                  usrIters >>

\* Atomic registration-lock critical section: if the pid is still findable,
\* pin the wrapper (take a ref); otherwise drop the event.
GenLookup ==
  /\ genPC = "gLook"
  /\ \/ (poolEntry  /\ refs' = refs + 1 /\ genPC' = "gAcq")
     \/ (~poolEntry /\ refs' = refs     /\ genPC' = "g0")
  /\ genEv' = (IF poolEntry THEN genEv ELSE NoOne)
  /\ UNCHANGED << hanPC, usrPC, hanEv, sigPending, exitPending, mbSig,
                  mbExit, procLock, implAlloc, llproc, exitstate, poolEntry,
                  wrapperAlloc, usrIters >>

\* Decode takes proc_lock (BUGL4 skips it).
GenAcquire ==
  /\ genPC = "gAcq"
  /\ IF BUGL4_GenDecodeNoProcLock
       THEN procLock' = procLock
       ELSE procLock = NoOne /\ procLock' = "gen"
  /\ genPC' = "gChk"
  /\ UNCHANGED << hanPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, objs, usrIters >>

\* The checked accessor: llproc re-checked before deref; a severed process
\* means the event is dropped (unpin, unlock).
GenCheck ==
  /\ genPC = "gChk"
  /\ \/ (llproc  /\ genPC' = "gUse" /\ refs' = refs
                 /\ procLock' = procLock /\ genEv' = genEv)
     \/ (~llproc /\ genPC' = "g0"   /\ refs' = refs - 1
                 /\ procLock' = (IF procLock = "gen" THEN NoOne ELSE procLock)
                 /\ genEv' = NoOne)
  /\ UNCHANGED << hanPC, usrPC, hanEv, sigPending, exitPending, mbSig,
                  mbExit, implAlloc, llproc, exitstate, poolEntry,
                  wrapperAlloc, usrIters >>

\* gUse is the impl-deref window (NoUAF checks it).  Enqueue transfers the
\* pinned ref to the mailbox entry and releases proc_lock.
GenEnqueue ==
  /\ genPC = "gUse"
  /\ mbSig'  = (IF genEv = "sig"  THEN TRUE ELSE mbSig)
  /\ mbExit' = (IF genEv = "exit" THEN TRUE ELSE mbExit)
  /\ procLock' = (IF procLock = "gen" THEN NoOne ELSE procLock)
  /\ genEv' = NoOne
  /\ genPC' = "g0"
  /\ UNCHANGED << hanPC, usrPC, hanEv, sigPending, exitPending, objs,
                  usrIters >>

GenNext == GenPick \/ GenLookup \/ GenAcquire \/ GenCheck \/ GenEnqueue

--------------------------------------------------------------------------
\* Handler.  A sig event: proc_lock, checked deref, drop the event's ref.
\* The exit event: teardown -- publish exitstate, sever llproc, free the
\* impl (all under proc_lock), then deregister the pool entry and drop both
\* the event's ref and the pool's ref.

HanPick ==
  /\ hanPC = "h0" /\ hanEv = NoOne
  /\ \/ (mbSig  /\ hanEv' = "sig"  /\ mbSig' = FALSE /\ UNCHANGED mbExit
                /\ hanPC' = "hSAcq")
     \/ (mbExit /\ hanEv' = "exit" /\ mbExit' = FALSE /\ UNCHANGED mbSig
                /\ hanPC' = (IF BUGL2_FreeBeforeSever THEN "hTFree0" ELSE "hTAcq"))
  /\ UNCHANGED << genPC, usrPC, genEv, sigPending, exitPending, procLock,
                  objs, usrIters >>

\* --- sig handling ---
HanSigAcquire ==
  /\ hanPC = "hSAcq" /\ procLock = NoOne /\ procLock' = "han"
  /\ hanPC' = "hSChk"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, implAlloc, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanSigCheck ==
  /\ hanPC = "hSChk"
  /\ \/ (llproc  /\ hanPC' = "hSUse")
     \/ (~llproc /\ hanPC' = "hSRel")    \* late event vs exited: drop
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs, usrIters >>

\* hSUse is an impl-deref window.
HanSigUse ==
  /\ hanPC = "hSUse" /\ hanPC' = "hSRel"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs, usrIters >>

HanSigRelease ==
  /\ hanPC = "hSRel"
  /\ procLock' = NoOne /\ refs' = refs - 1 /\ hanEv' = NoOne
  /\ hanPC' = "h0"
  /\ UNCHANGED << genPC, usrPC, genEv, sigPending, exitPending, mbSig,
                  mbExit, implAlloc, llproc, exitstate, poolEntry,
                  wrapperAlloc, usrIters >>

\* --- exit handling (teardown) ---
\* BUGL2: free FIRST, outside any lock, then run the rest of the sequence.
HanTearFree0 ==
  /\ hanPC = "hTFree0"
  /\ implAlloc' = FALSE
  /\ hanPC' = "hTAcq"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanTearAcquire ==
  /\ hanPC = "hTAcq" /\ procLock = NoOne /\ procLock' = "han"
  /\ hanPC' = "hTPub"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, implAlloc, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanTearPublish ==
  /\ hanPC = "hTPub" /\ exitstate' = TRUE /\ hanPC' = "hTSev"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, implAlloc, llproc, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanTearSever ==
  /\ hanPC = "hTSev" /\ llproc' = FALSE
  /\ hanPC' = (IF BUGL2_FreeBeforeSever THEN "hTRel" ELSE "hTFree")
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, implAlloc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanTearFree ==
  /\ hanPC = "hTFree" /\ implAlloc' = FALSE /\ hanPC' = "hTRel"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

HanTearRelease ==
  /\ hanPC = "hTRel" /\ procLock' = NoOne /\ hanPC' = "hTDereg"
  /\ UNCHANGED << genPC, usrPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, implAlloc, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc, usrIters >>

\* Atomic registration-lock critical section: remove the pool entry.
\* Then drop the event's ref + the pool's ref (BUGL3 leaks the pool's).
HanTearDeregister ==
  /\ hanPC = "hTDereg" /\ poolEntry' = FALSE
  /\ refs' = (IF BUGL3_PoolRefLeak THEN refs - 1 ELSE refs - 2)
  /\ hanEv' = NoOne
  /\ hanPC' = "h0"
  /\ UNCHANGED << genPC, usrPC, genEv, sigPending, exitPending, mbSig,
                  mbExit, procLock, implAlloc, llproc, exitstate,
                  wrapperAlloc, usrIters >>

HanNext == HanPick \/ HanSigAcquire \/ HanSigCheck \/ HanSigUse
           \/ HanSigRelease \/ HanTearFree0 \/ HanTearAcquire
           \/ HanTearPublish \/ HanTearSever \/ HanTearFree
           \/ HanTearRelease \/ HanTearDeregister

--------------------------------------------------------------------------
\* User: bounded API calls.  Design path = the public-helper discipline
\* after the D-3 flip: proc_lock, check llproc under the lock, deref (or
\* read exitstate on the exited branch), release.  BUGL1 = the old
\* master-native readers: check + deref with no lock at all.

UsrBegin ==
  /\ usrPC = "u0" /\ usrIters > 0
  /\ IF BUGL1_UnlockedUserDeref
       THEN usrPC' = "uChkNL" /\ procLock' = procLock
       ELSE procLock = NoOne /\ procLock' = "usr" /\ usrPC' = "uChk"
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, objs, usrIters >>

UsrCheck ==
  /\ usrPC = "uChk"
  /\ usrPC' = (IF llproc THEN "uUse" ELSE "uExitRead")
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs, usrIters >>

\* uUse / uExitRead are the deref / exitstate-read windows.
UsrUse ==
  /\ usrPC \in {"uUse", "uExitRead"}
  /\ usrPC' = "uRel"
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs, usrIters >>

UsrRelease ==
  /\ usrPC = "uRel"
  /\ procLock' = NoOne /\ usrIters' = usrIters - 1 /\ usrPC' = "u0"
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, implAlloc, llproc, exitstate, poolEntry,
                  refs, wrapperAlloc >>

\* BUGL1 path: same shape, no lock.
UsrCheckNL ==
  /\ usrPC = "uChkNL"
  /\ usrPC' = (IF llproc THEN "uUseNL" ELSE "uExitReadNL")
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs, usrIters >>

UsrUseNL ==
  /\ usrPC \in {"uUseNL", "uExitReadNL"}
  /\ usrIters' = usrIters - 1 /\ usrPC' = "u0"
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, objs >>

UsrUnref ==
  /\ usrPC = "u0" /\ usrIters = 0
  /\ refs' = refs - 1 /\ usrPC' = "uDone"
  /\ UNCHANGED << genPC, hanPC, genEv, hanEv, sigPending, exitPending,
                  mbSig, mbExit, procLock, implAlloc, llproc, exitstate,
                  poolEntry, wrapperAlloc, usrIters >>

UsrNext == UsrBegin \/ UsrCheck \/ UsrUse \/ UsrRelease
           \/ UsrCheckNL \/ UsrUseNL \/ UsrUnref

--------------------------------------------------------------------------
\* Reclaim: the last unref frees the wrapper.
Reclaim ==
  /\ refs = 0 /\ wrapperAlloc
  /\ wrapperAlloc' = FALSE
  /\ UNCHANGED << genPC, hanPC, usrPC, genEv, hanEv, sigPending,
                  exitPending, mbSig, mbExit, procLock, implAlloc, llproc,
                  exitstate, poolEntry, refs, usrIters >>

Done == ~wrapperAlloc /\ UNCHANGED vars

Next == GenNext \/ HanNext \/ UsrNext \/ Reclaim \/ Done

Spec == Init /\ [][Next]_vars
        /\ SF_vars(GenNext) /\ SF_vars(HanNext) /\ SF_vars(UsrNext)
        /\ SF_vars(Reclaim)

--------------------------------------------------------------------------
\* Impl-deref windows, across all threads.
UsingImpl ==
  \/ genPC = "gUse"
  \/ hanPC = "hSUse"
  \/ usrPC \in {"uUse", "uUseNL"}

\* Every deref sees an allocated impl.
NoUAF == UsingImpl => implAlloc

\* Teardown order: free only after sever ...
SeverBeforeFree == ~implAlloc => ~llproc

\* ... and sever only after exitstate is published.
PublishBeforeSever == ~llproc => exitstate

\* The exited branch of a query reads a published exitstate.
ExitReadSafe == (usrPC \in {"uExitRead", "uExitReadNL"}) => exitstate

\* The wrapper outlives the impl, the pool entry and every ref.
WrapperLastOut ==
  ~wrapperAlloc => (~implAlloc /\ refs = 0 /\ ~poolEntry)

\* proc_lock lives in the wrapper: it is only ever held while the wrapper
\* is allocated.
LockInWrapper == (procLock /= NoOne) => wrapperAlloc

RefsBound == refs \in 0..4

\* Liveness: everything is eventually reclaimed (no leak).
Reclaimed == <>(~wrapperAlloc)
=============================================================================
