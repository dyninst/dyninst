.. _`sec:pcEventMuxer.h`:

pcEventMuxer.h
##############

ProcControl events have two items of interest: the event type (which tells us what happened)
and the event process (which tells us who it happened to). ProcControl internally muxes a
callback based on the type, as you can register a different callback for each event type.
However, it does not mux based on process. Therefore, we have to do the muxing here in a
multiprocess situation.

To make matters worse, we want to handle some events internally, without waiting for
the user to explicitly wait() for an event. For example, traps corresponding to
tramp jumps; such traps should be handled as quickly as possible.

This leads to the following design::

  pcEventMuxer - unique class (static and globally accessible).
    - Registers callbacks with ProcControl
    - Contains a list of all active pcEventHandlers
    - Loops in ProcControl::wait(), waiting for events to be received
    - When an event is received, decode and determine if it is "fast" or "slow"
    - Fast events are handled immediately
    - Slow events are entered on a queue to be handled when the user calls
      pcEventMuxer::wait.
  pcEventHandler - one per active process and a friend of pcProcess
    - Is called by pcEventMuxer to handle fast and slow process events.

  So the flow of events is twofold:
    pcEM::main() -> wait() -> decode() -> handle (fast) or enqueue (slow)
    pcEH::wait -> dequeue -> handle


**SYSCALL HANDLING**

The logic for handling system call entry/exit is complicated by the
fact that ProcControlAPI doesn't provide events for all system call
entry/exit events we are interested in on all platforms (it doesn't
provide them due to lacking OS debug interfaces). Additionally,
for some events we need to alert ProcControl that they have occurred
to allow it to update its Process/Thread structures (e.g., Post-Fork
on FreeBSD).

The following approach is used to manage this situation in a
sane, clean way.

There are two events that are used to indicate syscall entry/exit::

  1) A Dyninst breakpoint via the RT library
  2) A ProcControlAPI event for the syscall

There are three cases we need to handle related to these events::

|  case 1: Event 2 is provided by ProcControl -> 1 is not necessary,
|  the BPatch-level event is reported when 2 is received.
|
|  case 2: Event 2 is not provided by ProcControl -> 1 is necessary,
|  2 is reported to ProcControl at 1, the BPatch-level event is
|  reported when 2 is received via ProcControl (after it updates its
|  internal data structures)
|
|  case 3: Event 2 is not provided by ProcControl -> 1 is necessary,
|  the BPatch-level event is reported at 1

These cases translate to the following in terms of registering
ProcControlAPI callbacks and inserting breakpoints via the
syscallNotification class.

|  case 1: register the callback, don't insert the breakpoint
|  case 2: register the callback, insert the breakpoint
|  case 3: don't register the callback, insert the breakpoint



.. cpp:class:: PCEventMailbox

  .. cpp:function:: PCEventMailbox()
  .. cpp:function:: ~PCEventMailbox()
  .. cpp:function:: void enqueue(Dyninst::ProcControlAPI::Event::const_ptr ev)
  .. cpp:function:: Dyninst::ProcControlAPI::Event::const_ptr dequeue(bool block)
  .. cpp:function:: unsigned int size()
  .. cpp:function:: bool find(PCProcess *proc)
  .. cpp:member:: protected std::map<int, int> procCount
  .. cpp:member:: protected std::queue<Dyninst::ProcControlAPI::Event::const_ptr> eventQueue
  .. cpp:member:: protected CondVar<> queueCond


.. cpp:class:: PCEventMuxer

  .. cpp:type:: Dyninst::ProcControlAPI::Process::cb_ret_t cb_ret_t
  .. cpp:type:: Dyninst::ProcControlAPI::Event::const_ptr EventPtr
  .. cpp:function:: static PCEventMuxer &muxer()
  .. cpp:function:: static bool start()
  .. cpp:function:: static WaitResult wait(bool block)
  .. cpp:function:: bool hasPendingEvents(PCProcess *proc)
  .. cpp:function:: static bool handle(PCProcess *proc = NULL)
  .. cpp:function:: static cb_ret_t defaultCallback(EventPtr)

    Commented out callbacks are handled by the default instead

  .. cpp:function:: static cb_ret_t exitCallback(EventPtr)
  .. cpp:function:: static cb_ret_t crashCallback(EventPtr)
  .. cpp:function:: static cb_ret_t signalCallback(EventPtr)
  .. cpp:function:: static cb_ret_t breakpointCallback(EventPtr)
  .. cpp:function:: static cb_ret_t RPCCallback(EventPtr)
  .. cpp:function:: static cb_ret_t threadCreateCallback(EventPtr)
  .. cpp:function:: static cb_ret_t threadDestroyCallback(EventPtr)
  .. cpp:function:: static cb_ret_t forkCallback(EventPtr)
  .. cpp:function:: static cb_ret_t execCallback(EventPtr)
  .. cpp:function:: static cb_ret_t SingleStepCallback(EventPtr)
  .. cpp:function:: static bool useCallback(Dyninst::ProcControlAPI::EventType et)
  .. cpp:function:: static bool useBreakpoint(Dyninst::ProcControlAPI::EventType et)
  .. cpp:function:: private static DThread::dthread_ret_t main(void *)

    We need to use a separate thread so that we can fast-handle certain events, such as trapping for PC changes

  .. cpp:function:: private bool registerCallbacks()
  .. cpp:function:: private WaitResult wait_internal(bool block)
  .. cpp:function:: private bool handle_internal(PCProcess *proc)
  .. cpp:function:: private PCEventMuxer()
  .. cpp:member:: private bool callbacksRegistered_
  .. cpp:member:: private bool started_
  .. cpp:member:: private DThread thrd_
  .. cpp:member:: private static PCEventMuxer muxer_
  .. cpp:function:: private void enqueue(EventPtr)
  .. cpp:function:: private EventPtr dequeue(bool block)
  .. cpp:function:: private bool handle(EventPtr)
  .. cpp:member:: private static Dyninst::ProcControlAPI::Process::cb_ret_t ret_stopped
  .. cpp:member:: private static Dyninst::ProcControlAPI::Process::cb_ret_t ret_continue
  .. cpp:member:: private static Dyninst::ProcControlAPI::Process::cb_ret_t ret_default
  .. cpp:member:: private PCEventMailbox mailbox_


.. cpp:enum:: PCEventMuxer::WaitResult

  .. cpp:enumerator:: EventsReceived
  .. cpp:enumerator:: NoEvents
  .. cpp:enumerator:: Error
