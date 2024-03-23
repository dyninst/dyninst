.. _`sec:walker.h`:

walker.h
########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: Walker

  **Walks call stacks and queries basic information about threads in a target process**
  
  Users should create a walker for each process from which they are walking call
  stacks. Each walker is associated with one process, but may walk call stacks on
  multiple threads within that process. This class allows users to query for the
  threads available for walking, and it allows you to specify a particular thread
  whose call stack should be walked.

  Every walker contains a :cpp:class:`ProcessState`, a :cpp:class:`StepperGroup`,
  and a :cpp:class:`SymbolLookup`. These are part of the Callback Interface and can
  be used to customize StackwalkerAPI. The ``ProcessState`` tells the walker how to
  access data in the target process and determines whether this walker collects first
  party or third party stackwalks. ``Walker`` will pick an appropriate default
  ``ProcessState`` based on which factory method the users calls. The ``StepperGroup``
  is used to customize how the walker steps through stack frames. The ``SymbolLookup``
  is used to customize how StackwalkerAPI looks up symbolic names of the function or
  object that created a stack frame.

  .. cpp:function:: static Walker *newWalker(std::string exec_name = std::string(""))

      Creates a walker by launching a process for the executable ``exec_name``.

      If ``exec_name`` is omitted, the current process is used.

  .. cpp:function:: static Walker *newWalker(Dyninst::PID pid, std::string executable)

      Creates a third-party walker attached to the process with id ``pid`` and gives it the name ``executable``. 

      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: static Walker *newWalker(Dyninst::PID pid)
  
      Creates a third-party walker attached to the process with id ``pid``.
      
      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: static Walker *newWalker(Dyninst::ProcControlAPI::Process::ptr proc)

      Creates a third-party walker attached to the process ``proc``.

      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: static bool newWalker(const std::vector<Dyninst::PID> &pids, std::vector<Walker *> &walkers_out, std::string executable)

      For each id in ``pids``, returns in ``walkers_out`` a new third-party walker associated with that process and given the
      name ``executable``.
      
      Returns ``false`` on error.
      
      .. danger:: The user is responsible for deallocating the returned values. 

  .. cpp:function:: static bool newWalker(const std::vector<Dyninst::PID> &pids, std::vector<Walker *> &walkers_out)

      For each id in ``pids``, returns in ``walkers_out`` a new third-party walker associated with that process.
      
      Returns ``false`` on error.
      
      .. danger:: The user is responsible for deallocating the returned values.

  .. cpp:function:: static Walker *newWalker(std::string exec_name, const std::vector<std::string> &argv)

      Creates a third-party walker by launching a process for the executable ``exec_name`` with arguments ``argv``.

      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: static Walker *newWalker(ProcessState *proc, StepperGroup *grp = NULL, SymbolLookup *lookup = NULL,\
                                             bool default_steppers = true)

      Creates a walker associated with a running process with state ``proc``, group ``grp``, and symbol lookup ``lookup``.
      
      Return either a first-party and third-party walker, depending on the configuration of ``proc``.

      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: virtual ~Walker()

  .. cpp:function:: static SymbolReaderFactory *getSymbolReader()

      Returns a factory for creating process-specific symbol readers.

  .. cpp:function:: static void setSymbolReader(SymbolReaderFactory *srf)

      Sets the symbol reader factory to ``srf``.

  .. cpp:function:: bool walkStack(std::vector<Frame> &stackwalk, Dyninst::THR_ID thread = NULL_THR_ID)

      Returns in ``stackwalk`` the call stack in either the associated process.
      
      The top of the stack is the first element returned. The bottom of the stack is
      the last element.
      
      If ``thread`` is provided, then only the thread with that id is walked. If it is
      omitted, then a default thread will be chosen. When doing a third-party stackwalk, the
      default thread will be the process’ initial thread. When doing a first-party stackwalk,
      the default thread will be the thread that called this function. The default
      ``StepperGroup`` provided to a Walker will support collecting call stacks from almost
      all types of functions, including signal handlers and optimized, frameless functions.

      Returns ``false`` on failure.

  .. cpp:function:: bool walkStackFromFrame(std::vector<Frame> &stackwalk, const Frame &frame)

      Returns in ``stackwalk`` the call stack starting from the stack frame, ``frame``.

      Returns ``false`` on failure.

  .. cpp:function:: bool walkSingleFrame(const Frame &in, Frame &out)

      Walks the single frame ``in``. ``out`` is set to ``in``\ ’s caller frame.

      Returns ``false`` on failure.

  .. cpp:function:: bool getInitialFrame(Frame &frame, Dyninst::THR_ID thread = NULL_THR_ID)

      Returns in ``frame`` the top of the stack for thread ``thread``.
      
      If ``thread`` is omitted, then a default thread is chosen. When doing a third-party walk,
      the default thread is the process’ initial thread. For a first-party walk, the thread that
      called this function is used.

      Returns ``false`` on error.

  .. cpp:function:: bool getAvailableThreads(std::vector<Dyninst::THR_ID> &threads) const

      Returns in ``threads`` the threads in the target process that can be walked.
      
      The set of threads may be a subset of the actual threads in the process. For example,
      when walking call stacks on the current process, it is only legal to walk the call stack on the
      currently running thread. In this case, this function returns only the current thread.

      Returns ``false`` on error.

  .. cpp:function:: ProcessState *getProcessState() const

      Returns the ProcessState associated with this walker.

  .. cpp:function:: SymbolLookup *getSymbolLookup() const

      Returns the SymbolLookup associated with this walker.

  .. cpp:function:: StepperGroup *getStepperGroup() const

      Returns the StepperGroup associated with this walker.

  .. cpp:function:: bool addStepper(FrameStepper *stepper)

      Adds ``stepper`` to the stepper group for this walker.
