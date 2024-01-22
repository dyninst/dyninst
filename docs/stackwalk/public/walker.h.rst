walker.h
========

.. cpp:namespace:: Dyninst::stackwalk

Class Walker
~~~~~~~~~~~~

**Defined in:** ``walker.h``

The ``Walker`` class returns a call stack as a vector of ``Frame``
objects. As described in Section `3.1.1 <#subsec:definitions>`__, each
Frame object represents a stack frame, and contains a return address
(RA), stack pointer (SP) and frame pointer (FP). For each of these
values, optionally, it stores the location where the values were found.
Each Frame object may also be augmented with symbol information giving a
function name (or a symbolic name, in the case of non-functions) for the
object that created the stack frame.

The ``Walker`` class allows users to walk call stacks and query basic
information about threads in a target process. The user should create a
``Walker`` object for each process from which they are walking call
stacks. Each ``Walker`` object is associated with one process, but may
walk call stacks on multiple threads within that process. The ``Walker``
class allows users to query for the threads available for walking, and
it allows you to specify a particular thread whose call stack should be
walked. Stackwalks are returned as a vector of Frame objects.

Each Walker object contains three objects:

-  ProcessState

-  StepperGroup

-  SymbolLookup

These objects are part of the Callback Interface and can be used to
customize StackwalkerAPI. The ``ProcessState`` object tells ``Walker``
how to access data in the target process, and it determines whether this
``Walker`` collects first party or third party stackwalks. ``Walker``
will pick an appropriate default ``ProcessState`` object based on which
factory method the users calls. The ``StepperGroup`` object is used to
customize how the ``Walker`` steps through stack frames. The
``SymbolLookup`` object is used to customize how StackwalkerAPI looks up
symbolic names of the function or object that created a stack frame.

.. code-block:: cpp

    static Walker *newWalker() static Walker *newWalker(Dyninst::PID pid)
    static Walker *newWalker(Dyninst::PID pid, std::string executable)
    static Walker *newWalker(Dyninst::ProcControlAPI::Process::ptr proc);
    static Walker *newWalker(std::string executable, const
    std::vector<std::string> &argv) static Walker *newWalker(ProcessState *proc, StepperGroup *steppergroup = NULL , SymbolLookup *lookup = NULL)

These factory methods return new Walker objects:

-  The first takes no arguments and returns a first-party stackwalker.

-  The second takes a PID representing a running process and returns a
   third-party stackwalker on that process.

-  The third takes the name of the executing binary in addition to the
   PID and also returns a third-party stackwalker on that process.

-  The fourth takes a ProcControlAPI process object and returns a
   third-party stackwalker.

-  The fifth takes the name of an executable and its arguments, creates
   the process, and returns a third-party stackwalker.

-  The sixth takes a ProcessState pointer representing a running process
   as well as user-defined StepperGroup and SymbolLookup pointers. It
   can return both first-party and third-party Walkers, depending on the
   ProcessState parameter.

Unless overriden with the sixth variant, the new Walker object uses the
default StepperGroup and SymbolLookup callbacks for the current
platform. First-party walkers use the ProcSelf callback for its
ProcessState object. Third-party walkers use ProcDebug instead. See
Section 3.5.1 for more information about defaults in the Callback
Interface.

This method returns NULL if it was unable to create a new Walker object.
The new Walker object was created with the new operator, and should be
deallocated with the delete operator when it is no longer needed.

.. code-block:: cpp

    static bool newWalker(const std::vector<Dyninst::PID> &pids,
    std::vector<Walker *> &walkers_out) static bool newWalker(const
    std::vector<Dyninst::PID> &pids, std::vector<Walker *> &walkers_out,
    std::string executable)

This method attaches to a group of processes and returns a vector of
Walker objects that perform third-party stackwalks. As above, the first
variant takes a list of PIDs and attaches to those processes; the second
variant also specifies the executable binary.

.. code-block:: cpp

    bool walkStack(std::vector<Frame> &stackwalk, Dyninst::THR_ID thread = NULL_THR_ID)

This method walks a call stack in the process associated with this
``Walker``. The call stack is returned as a vector of ``Frame`` objects
in stackwalk. The top of the stack is returned in index 0 of stackwalk,
and the bottom of the stack is returned in index ``stackwalk.size()-1``.

A stackwalk can be taken on a specific thread by passing a value in the
thread parameter. If ``thread`` has the value ``NULL_THR_ID``, then a
default thread will be chosen. When doing a third party stackwalk, the
default thread will be the process’ initial thread. When doing a first
party stackwalk, the default thread will be the thread that called
``walkStack``. The default StepperGroup provided to a Walker will
support collecting call stacks from almost all types of functions,
including signal handlers and optimized, frameless functions.

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool walkStackFromFrame(std::vector<Frame> &stackwalk, const Frame &frame)

This method walks a call stack starting from the given stack frame,
``frame``. The call stack will be output in the ``stackwalk`` vector,
with frame stored in index 0 of ``stackwalk`` and the bottom of the
stack stored in index ``stackwalk.size()-1``.

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool walkSingleFrame(const Frame &in, Frame &out)

This methods walks through single frame, ``in``. Parameter ``out`` will
be set to ``in``\ ’s caller frame.

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool getInitialFrame(Frame &frame, Dyninst::THR_ID thread = NULL_THR_ID)

This method returns the ``Frame`` object on the top of the stack in
parameter frame. Under ``walkStack``, ``frame`` would be the one
returned in index 0 of the ``stackwalk`` vector. A stack frame can be
found on a specific thread by passing a value in the thread parameter.
If ``thread`` has the value ``NULL_THR_ID``, then a default thread will
be chosen. When doing a third party stackwalk, the default thread will
be the process’ initial thread. When doing a first party stackwalk, the
default thread will be the thread that called ``getInitialFrame``.

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    bool getAvailableThreads(std::vector<Dyninst::THR_ID> &threads)

This method returns a vector of threads in the target process upon which
StackwalkerAPI can walk call stacks. The threads are returned in output
parameter ``threads``. Note that this method may return a subset of the
actual threads in the process. For example, when walking call stacks on
the current process, it is only legal to walk the call stack on the
currently running thread. In this case, ``getAvailableThreads`` returns
a vector containing only the current thread.

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    ProcessState *getProcessState() const

This method returns the ``ProcessState`` object associated with this
``Walker``.

.. code-block:: cpp

    StepperGroup *getStepperGroup() const

This method returns the ``StepperGroup`` object associated with this
``Walker``.

.. code-block:: cpp

    SymbolLookup *getSymbolLookup() const

This method returns the ``SymbolLookup`` object associated with this
``Walker``.

.. code-block:: cpp

    bool addStepper(FrameStepper *stepper)

This method adds a provided FrameStepper to those used by the Walker.

.. code-block:: cpp

    static SymbolReaderFactory *getSymbolReader()

This method returns a factory for creating process-specific symbol
readers. Unlike the above methods it is global across all Walkers and is
thus defined static.

.. code-block:: cpp

    static void setSymbolReader(SymbolReaderFactory *);

Set the symbol reader factory used when creating ``Walker`` objects.

.. code-block:: cpp

    static void version(int &major, int &minor, int &maintenance)

This method returns version information (e.g., 8, 0, 0 for the 8.0
release).