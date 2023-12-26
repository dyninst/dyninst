.. _`sec:proccontrolapi-intro`:

.. cpp:namespace:: Dyninst::ProcControlAPI

ProcControlAPI
##############

ProcControlAPI is a library for controlling processes. It runs as part of a *controller
process* and manages one or more *target processes*. It allows the
controller process to perform operations on target processes, such as
writing to memory, stopping and running threads, or receiving
notification when certain events occur. ProcControlAPI presents these
operations through a platform-independent API and high-level
abstractions. Users can describe what they want ProcControlAPI to do,
and ProcControlAPI handles the details.

An example use for ProcControlAPI would be as the underlying mechanism
for a debugger. A user writing a debugger could provide their own user
interface and debugging strategies, while using ProcControlAPI to
perform operations such as creating processes, running threads, and
handling breakpoints.

The interface for ProcControlAPI can be generally divided into two
parts: an interface for managing a process (e.g., reading and writing to
target process memory, stopping and running threads), and an interface
for monitoring a target process for certain events (e.g., watching the
target process for fork or thread creation events). The manager
interface represents a target process and its threads, libraries, registers,
and other interesting aspects.

Abstractions
************

This section focuses on some of the more important concepts in ProcControlAPI.

.. _`sec:proccontrol-intro-processes-threads`:

Processes and Threads
=====================

There are two central classes to ProcControlAPI, :cpp:class:`Dyninst::ProcControlAPI::Process`
and :cpp:class:`Dyninst::ProcControlAPI::Thread`.
Each class respectively represents a single target process or thread
running on the system. By performing operations on the Process and
Thread objects, a ProcControlAPI user is able to control the target
process and its threads.

Each Process is guaranteed to have at least one Thread associated with
it. A multi-threaded process may have a Process object with more than
one Thread. Each process has an address space associated with it, which
can be written or read through the Process object. Each thread has a set
of registers associated with it, which can be access through the Thread
object.

At any one time a Thread will be in either a *stopped state* or a
*running state*. A thread in a stopped state has had its execution
paused by ProcControlAPI - the OS will not schedule the thread to run. A
thread in a running state is allowed to execute as normal. A thread in a
running state may block for other reasons, e.g. blocking on IO calls,
but this does not affect ProcControlAPI’s view of the thread state. A
thread is only in the stopped state if ProcControlAPI has explicitly
stopped it.

A Process object is not considered to have a stopped or running
state—only its Thread objects are stopped or running. A stop operation
on a Process triggers a stop operation on each of its Threads, and
similarly a continue operation on a Process triggers continue operations
on each Thread.

.. _`sec:proccontrol-intro-breakpoints`:

Breakpoints
===========

A :cpp:class:`Dyninst::ProcControlAPI::Breakpoint` is a point in the code region of a target process that,
when executed, stops the process execution and notifies ProcControlAPI.
Upon being continued the process will resume execution at the point. A
Breakpoint object is a handle that can represents one or more
breakpoints in one or more processes. Upon receiving notification that a
breakpoint has executed, ProcControlAPI will deliver a callback with an
:cpp:class:`Dyninst::ProcControlAPI::EventBreakpoint`.

Some Breakpoint objects can be created as *control-transfer
breakpoints*. When a process is continued after executing a
control-transfer the process will resume at an alternate location,
rather than at the breakpoint’s installation point.

A single Breakpoint can be inserted into multiple locations within a
target process. This can be useful when a user has wants to perform a
single action at multiple locations in a target process. For example, if
a user wants to insert a breakpoint at the entry to function foo, and
foo has multiple instantiations in a process, then a single Breakpoint
can be inserted at each instance of foo.

A single Breakpoint object can be inserted into multiple target
processes at the same time. When a process does an operation that copies
an address space, such as fork on UNIX, the child process will receive
all Breakpoint objects that were installed in the parent process.

Multiple Breakpoint objects can be inserted into the same location
with-in the same process. When this location is executed in the target
process a single callback will be delivered, and the EventBreakpoint
object will contain a reference to each Breakpoint inserted at the
location. At most one control-transfer breakpoint can be inserted at any
one point in a process.

Due to the many-to-many nature of Breakpoints and Processes, a single
installation of a Breakpoint can be identified by a Breakpoint, :cpp:class:`Dyninst::ProcControlAPI::Process`,
:cpp:type:`Dyninst::Address` triple. The functions for inserting and removing breakpoints
(:cpp:func:`Dyninst::ProcControlAPI::Process::addBreakpoint` and :cpp:func:`Dyninst::ProcControlAPI::Process::rmBreakpoint`) need all three pieces
of information.

A breakpoint can be a hardware breakpoint or a software breakpoint. A
hardware breakpoint is typically implemented by setting special debug
register in the process and can trigger on code execution, data reads or
data write. A software breakpoint is typically implemented by writing a
special instruction into a code sequence and can only be triggered by
code execution. There are typically a limited number of hardware
breakpoints available at the same time.

.. _`sec:proccontrol-intro-callbacks`:

Callbacks
=========

In addition to controlling a target process through the Process and
Thread objects, a ProcControlAPI user can also receive notification of
events that happen in that process. Examples of these events would be a
new thread being created, a breakpoint being executed, or a process
exiting.

The ProcControlAPI user receives notice of events through a callback
system. The user can register callback function that will be called by
ProcControlAPI whenever a particular type of event occurs. Details about
the event are passed to the callback function via an Event object.

.. _`sec:proccontrol-intro-callback-events`:

Events
------

Each event can be broken up into an EventType object and an Event
object. The EventType describes a type of event that can happen, and
Event describes a specific instance of an event happening. Each Event
will have one and only one EventType.

Each :cpp:class:`Dyninst::ProcControlAPI::EventType` has two primary fields:
its time and its code. The code
field of describes what type of event occurred, e.g. :cpp:enumerator:`EventType::Exit`
represents a target process exiting. The time field of an EventType
represents whether the EventType is happening before or after will have
code and will have a value of :cpp:enumerator:`EventType::Pre`,
:cpp:enumerator:`EventType::Post`, or :cpp:enumerator:`EventType::None`.

For example, an EventType with time and code of :cpp:enumerator:`EventType::Pre` and
:cpp:enumerator:`EventType::Exit` will occur just before a target process exits, and a
code of :cpp:enumerator:`EventType::Exec` with a time of :cpp:enumerator:`EventType::Post`
will occur after an exec system call occurs. In this document we will abbreviate
EventTypes such as these as pre-exit and post-exec. Some EventTypes do
not have a time associated with them, for example :cpp:enumerator:`EventType::Breakpoint`
does not have an associated time and thus has a time value of :cpp:enumerator:`EventType::none`.

An Event represents an instance of an EventType occurring. In addition
to an EventType, each Event also has pointer to the Process and Thread
that it occurred on. Certain events may also have event specific
information associated with them, which is represented in a sub-class of
Event. Each EventType is associated with a specific sub-class of Event.

For example, :cpp:enumerator:`EventType::Library` is used to signify a shared library
being loaded into the target process. When an ``EventType::Library`` occurs
ProcControlAPI will deliver an object of type EventLibrary, which is a
subclass of Event, to any registered callback functions. In addition to
the information inherited from Event, the EventLibrary will contain
extra information about the library that was loaded into the target
process.

Table 1 shows the Event subclass that is used for each EventType. Not
all EventTypes are available on every platform—a checkmark under the
specific OS column means that the EventType is available on that OS.

.. csv-table:: EventType by OS
   :header: "EventType", "Event Subclass", "Linux", "FreeBSD", "Windows"
   :widths: 25, 25, 10, 10, 10

    EventType,EventSubclass,Linux,FreeBSD,Windows
    Stop,EventStop,X,,
    Breakpoint,EventBreakpoint,X,X,X
    Signal,EventSignal,X,X,X
    UserThreadCreate,EventNewUserThread,X,X,
    LWPCreate,EventNewLWP,X,,X
    Pre-UserThreadDestroy,EventUserThreadDestroy,X,X,
    Post-UserThreadDestroy,EventUserThreadDestroy,X,X,
    Pre-LWPDestroy,EventLWPDestroy,X,,X
    Post-LWPDestroy,EventLWPDestroy,X,,
    Pre-Fork,EventFork,,,
    Post-Fork,EventFork,X,,
    Pre-Exec,EventExec,,,
    Post-Exec,EventExec,X,X,
    RPC,EventRPC,X,X,X
    SingleStep,EventSingleStep,X,X,X
    Breakpoint,EventBreakpoint,X,X,X
    Library,EventLibrary,X,X,X
    Pre-Exit,EventExit,X,,
    Post-Exit,EventExit,X,X,X
    Crash,EventCrash,X,X,X
    ForceTerminate,EventForceTerminate,X,X,X

Callback Functions
------------------

Events are delivered via a callback function. A ProcControlAPI user can
register callback functions for an EventType using the
:cpp:func:`Process::registerEventCallback` function. All callback functions must be
declared using the signature:

In order to prevent a class of race conditions, ProcControlAPI does not
allow a callback function to perform any operation that would require
another callback to be recursively delivered. At most one callback
function can be running at a time.

To enforce this, the event that is passed to a callback function
contains only const pointers to the triggering Process and Thread
objects. Any member function that could trigger callbacks is not marked
const, thus triggering a compilation error if they are called on an
object passed to a callback. If the ProcControlAPI user uses const_cast
or global variables to get around the const restriction it will result
in a runtime error. API functions that cannot be used from a callback
are mentioned in the API entries.

Operations such as :cpp:func:`Process::stopProc`, :cpp:func:`Process::continueProc`,
:cpp:func:`Thread::stopThread`, and :cpp:func:`Thread::continueThread` are not safe to call from
a callback function, but it would still be useful to perform these
operations. ProcControlAPI allows the user to use the return value from
a callback function to specify whether process or thread that triggered
the event should be stopped or continued. More details on this can be
found in the :cpp:struct:`Process::cb_ret_t` section of the API reference.

Callback Delivery
-----------------

When ProcControlAPI needs to deliver a callback it must first gain
control of a user visible thread in the controller process. This thread
will be used to invoke the callback function. ProcControlAPI does not
use its internal threads for delivering callbacks, as this would expose
the ProcControlAPI user to race conditions.

Unfortunately, the user thread is not always accessible to
ProcControlAPI when it needs to invoke a callback function. For example,
the user visible thread may be performing network IO or waiting for
input from a GUI when an event occurs.

ProcControlAPI uses a notification system built around the EventNotify
class to alert the ProcControlAPI user that a callback is ready to be
delivered. Once the user is notified then they can call the
:cpp:func:`Process::handleEvents` function, under which ProcControlAPI will invoke
any pending callback functions.

The EventNotify class has two mechanisms for notifying the
ProcControlAPI user that a callback is pending: writing to a file
descriptor and a light-weight callback function. The :cpp:func:`EventNotify::getFD`
function returns a file descriptor that will have a byte written to it
when a callback is ready. This file descriptor can be added to a select
or poll to block a thread that handles ProcControlAPI events.
Alternatively, the ProcControlAPI user can register a light-weight
callback that is invoked when a callback is ready. This light-weight
callback provides no information about the Event and may occur on
another thread or from a signal handler—the ProcControlAPI user is
encouraged to keep this callback minimal.

It is important for a user to respond promptly to a callback
notification. A target process may remain blocked while a notification
is pending. If a target process is generating many events that need
callbacks, a long delay in notification could have a significant
performance impact.

Once the ProcControlAPI user knows that a callback is ready to be
delivered they can call :cpp:func:Process::handleEvents`, which will invoke all
callback functions. Alternatively, if the ProcControlAPI user does not
need to handle events outside of ProcControlAPI, they can continue to
block in :cpp:func:`Process::handleEvents` without going through the notification
system.

.. _`sec:proccontrol-intro-irpcs`:

iRPCs
*****

An iRPC (Inferior Remote Procedure Call) is a mechanism for executing
code in a target process. Despite the name, an iRPC does not necessarily
have to involve a procedure call—any piece of code can be executed.

A ProcControlAPI user can invoke an iRPC by providing ProcControlAPI
with a buffer of machine code and specifying a Process or Thread on
which to run the machine code. ProcControlAPI will insert the machine
code into the address space, save the register set, run the machine
code, and then remove the machine code after execution completes. When
the iRPC completes (but before the registers and memory are cleaned)
ProcControlAPI will deliver an EventIRPC to any registered callback
function. The ProcControlAPI user may use this callback to collect any
results from the registers or memory used by the iRPC.

Note that ProcControlAPI will preserve the registers of the thread
running the iRPC, and it will preserve the memory used by the machine
code. Other memory or system state changed by the iRPC may remain
visible to the target process after the iRPC completes.

The machine code for each iRPC must contain at least one trap
instruction (e.g., a 0xCC instruction on x86 family or a 0x7D821008
instruction on the PPC family). ProcControlAPI will stop executing the
iRPC upon invocation of the trap. Note that the trap instruction must
fall within the original machine code for the iRPC. If the iRPC calls or
jumps to another piece of code that executes a trap instruction then
ProcControlAPI will not treat it as the end of the iRPC.

Before an iRPC can be run it must be posted to a process or thread using
:cpp:func:`Process::postIRPC` or :cpp:func:`Thread::postIRPC`.
``Process::postIRPC`` selects a thread to post the iRPC to.
Multiple iRPCs can be posted to the same thread, but only one iRPC will
run at a time—subsequent iRPCs will be queued and run after the
preceding iRPC completes. If multiple iRPCs are posted to different
threads in a multi-threaded process, then they may run in parallel.

An iRPC can be posted to a stopped or running thread. If posted to a
stopped thread, then the iRPC will run when the thread is continued. If
posted to a running thread, then the iRPC will run immediately or, if
posted from a callback function, when the callback function completes.

An iRPC may be blocking or non-blocking. If a blocking iRPC is posted to
any Process, then calls to :cpp:func:`Process::handleEvents` will block until the
iRPC is completed.

.. _`sec:proccontrol-intro-registers`:

Registers
*********

This section describes the MachRegister interface, which is used for
accessing registers in ProcControlAPI. The entire definition of
MachRegister contains more register names than are listed here; this
appendix only lists the registers that can be accessed through
ProcControlAPI.

An instance of :cpp:class:`Dyninst::MachRegister` is defined for each register ProcControlAPI
can name. These instances live inside a namespace that represents the
register’s architecture. For example, we can name a register from an
AMD64 machine with `Dyninst::x86_64::rax` or a register from a Power
machine with `Dyninst::ppc32::r1`.

The following tables describe :cpp:class:`MachRegister` that can be
passed to ProcControlAPI. These can be named by prepending the namespace
to the listed names, e.g., ``x86::eax``.

.. csv-table:: namespace x86
   :width: 5
   
    eax,ebx,ecx,edx,ebp,esp,esi,edi
    oeax,eip,flags,cs,ds,es,fs,gs
    ss,fsbase,gsbase,,,

.. csv-table:: namespace x86_64
   :width: 5

    rax,rbx,rcx,rdx,rbp,rsp,rsi,rdi
    r8,r9,r10,r11,r12,r13,r14,r15
    orax,rip,flags,cs,ds,es,fs,gs
    ss,fsbase,gsbase,,,

.. csv-table:: namespace ppc32
   :width: 5

    r0,r1,r2,r3,r4,r5,r6,r7
    r8,r9,r10,r11,r12,r13,r14,r15
    r16,r17,r18,r19,r20,r21,r22,r23
    r24,r25,r26,r27,r28,r29,r30,r31
    fpscw,lr,cr,xer,ctr,pc,msr

.. csv-table:: namespace ppc64
   :width: 5

    r0,r1,r2,r3,r4,r5,r6,r7
    r8,r9,r10,r11,r12,r13,r14,r15
    r16,r17,r18,r19,r20,r21,r22,r23
    r24,r25,r26,r27,r28,r29,r30,r31
    fpscw,lr,cr,xer,ctr,pcmsr,

.. csv-table:: namespace aarch64
   :width: 5

    x0,x1,x2,x3,x4,x5,x6,x7
    x8,x9,x10,x11,x12,x13,x14,x15
    x16,x17,x18,x19,x20,x21,x22,x23
    x24,x25,x26,x27,x28,x29,x30,q0
    q1,q2,q3,q4,q5,q6,q7,q8
    q9,q10,q11,q12,q13,q14,q15,q16
    q17,q18,q19,q20,q21,q22,q23,q24
    q25,q26,q27,q28,q29,q30,q31,sp
    pc,pstate,fpcr,fpsr,,

.. _`sec:proccontrol-intro-systemcalls`:

System Calls
************

The :cpp:class:`Dyninst::MachSyscall` class represents system calls
in a platform-independent manner. Currently, syscall events are only
supported on Linux.

.. _`sec:proccontrol-usage`:

Usage
*****

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/proccontrol/callback.cpp
   :language: cpp
   :linenos:

Example that creates a target process and prints a message whenever that
target process creates a new thread. Details on the API function used in
this example can be found in latter sections of this manual, but we will
provide a high level description of the operations here. Note that proper
error handling and checking have been left out for brevity.

1. We start by parsing the arguments passed to the controller process,
   turning them into arguments that will be passed to the new target
   process.

2. We ask ProcControlAPI to create a new Process using the given
   arguments. ProcControlAPI will spawn a new target process and leave
   it in a stopped state to prevent it from executing.

3. After creating the new target process we register a callback
   function. We ask ProcControlAPI to call our function,
   on_thread_create, when an event of type :ref:`eventtype-values`
   occurs in the target process.

4. The on_thread_create function takes a pointer to an object of type
   Event and returns a :cpp:struct:`Process::cb_ret_t`. The Event describes the target
   process event that triggered this callback. In this case, it provides
   information about the new thread in the target process. It is worth
   noting that :cpp:type:`Event::const_ptr` is a not a regular pointer, but a
   reference counted shared pointer. This means that we do not have to
   be concerned with cleaning the Event—it will be automatically cleaned
   when the last reference disappears. The :cpp:class:`Process::cb_ret_t` describes
   what action should be taken on the process in response to this event.

5. The Event class has several child classes, one of which is
   EventNewThread. We start by casting the Event into an EventNewThread
   and then extract information about the new thread from the
   EventNewThread.

6. In step 6, we’ve finished handling the new thread event and need to
   tell ProcControlAPI what to do in response to this event. For
   example, we could choose to stop the process from further execution
   by returning a value of :cpp:enumerator:`Process::cbProcStop`. Instead, we choose let
   ProcControlAPI take its default action for an EventNewThread by
   returning :cpp:enumerator:`Process::cbDefault`, which is to continue the process and
   its new thread (which were both stopped before delivery of the
   callback).

7. The registering of our callback in step 3 did not actually trigger
   any calls to the callback function—the target process was created in
   a stopped state and has not yet been able to create any threads. We
   tell ProcControlAPI to continue the target process in this step,
   which allows it to execute and possibly start generating new events.

8. In this step we wait for the target process to finish executing and
   terminate. Calling :cpp:func:`Process::handleEvents` blocks the controller
   process until an event occurs, allowing us to wait for events without
   needing to spin the controller process on the CPU.
