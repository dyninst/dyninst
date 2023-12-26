procstate.h
===========

.. cpp:namespace:: Dyninst::stackwalk

Class ProcDebug
~~~~~~~~~~~~~~~

Access to StackwalkerAPI’s debugger is through the ``ProcDebug`` class,
which inherits from the ``ProcessState`` interface. The easiest way to
get at a ``ProcDebug`` object is to cast the return value of
``Walker::getProcessState`` into a ``ProcDebug``. C++’s ``dynamic_cast``
operation can be used to test if a ``Walker`` uses the ``ProcDebug``
interface:

.. code-block:: cpp

   ProcDebug *debugger;
   debugger = dynamic_cast<ProcDebug*>(walker->getProcessState());
   if (debugger != NULL) {
       //3rd party
       ...
   } else {
       //1st party
       ...
   }

In addition to the handling of debug events, described in
Section `4.1 <#subsec:debugger>`__, the ``ProcDebug`` class provides a
process control interface; users can pause and resume process or
threads, detach from a process, and test for events such as process
death. As an implementation of the ``ProcessState`` class, ``ProcDebug``
also provides all of the functionality described in
Section `3.6.4 <#subsec:processstate>`__.

.. code-block:: cpp

    virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID)

This method pauses a process or thread. The paused object will not
resume execution until ``ProcDebug::resume`` is called. If the ``tid``
parameter is not ``NULL_THR_ID`` then StackwalkerAPI will pause the
thread specified by ``tid``. If ``tid`` is ``NULL_THR_ID`` then
StackwalkerAPI will pause every thread in the process.

When StackwalkerAPI collects a call stack from a running thread it first
pauses the thread, collects the stack walk, and then resumes the thread.
When collecting a call stack from a paused thread StackwalkerAPI will
collect the stack walk and leave the thread paused. This method is thus
useful for pausing threads before stack walks if the user needs to keep
the returned stack walk synchronized with the current state of the
thread.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID)

This method resumes execution on a paused process or thread. This method
only resumes threads that were paused by the ``ProcDebug::pause`` call,
using it on other threads is an error. If the ``tid`` parameter is not
``NULL_THR_ID`` then StackwalkerAPI will resume the thread specified by
``tid``. If ``tid`` is ``NULL_THR_ID`` then StackwalkerAPI will resume
all paused threads in the process.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool detach(bool leave_stopped = false)

This method detaches StackwalkerAPI from the target process.
StackwalkerAPI will no longer receive debug events on this target
process and will no longer be able to collect call stacks from it. This
method invalidates the associated ``Walker`` and ``ProcState`` objects,
they should be cleaned using C++’s ``delete`` operator after making this
call. It is an error to attempt to do operations on these objects after
a detach, and undefined behavior may result.

If the ``leave_stopped`` parameter is ``true`` StackwalkerAPI will
detach from the process but leave it in a paused state so that it does
resume progress. This is useful for attaching another debugger back to
the process for further analysis. The ``leave_stopped`` parameter is not
supported on the Linux platform and its value will have no affect on the
detach call.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool isTerminated()

This method returns ``true`` if the associated target process has
terminated and ``false`` otherwise. A target process may terminate
itself by calling exit, returning from main, or receiving an unhandled
signal. Attempting to collect stack walks or perform other operations on
a terminated process is illegal an will lead to undefined behavior.

A process termination will also be signaled through the notification FD.
Users should check processes for the isTerminated state after returning
from handleDebugEvent.

.. code-block:: cpp

    static int getNotificationFD()

This method returns StackwalkerAPI’s notification FD. The notification
FD is a file descriptor that StackwalkerAPI will write a byte to
whenever a debug event occurs that need. If the user code sees a byte on
this file descriptor it should call ``handleDebugEvent`` to let
StackwalkerAPI handle the debug event. Example code using
``getNotificationFD`` can be found in
Section `4.1 <#subsec:debugger>`__.

StackwalkerAPI will only create one notification FD, even if it is
attached to multiple 3rd party target processes.

.. code-block:: cpp

    static bool handleDebugEvent(bool block = false)

When this method is called StackwalkerAPI will receive and handle all
pending debug events from each 3rd party target process to which it is
attached. After handling debug events each target process will be
continued (unless it was explicitly stopped by the ProcDebug::pause
method) and any bytes on the notification FD will be cleared. It is
generally expected that users will call this method when a event is sent
to the notification FD, although it can be legally called at any time.

If the ``block`` parameter is ``true``, then ``handleDebugEvents`` will
block until it has handled at least one debug event. If the block
parameter is ``false``, then handleDebugEvents will handle any currently
pending debug events or immediately return if none are available.

StackwalkerAPI may receive process exit events for target processes
while handling debug events. The user should check for any exited
processes by calling ``ProcDebug::isTerminated`` after handling debug
events.

This method returns ``true`` if successful and ``false`` on error.