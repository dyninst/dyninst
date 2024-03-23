. _`sec:freebsd.h`:

freebsd.h
#########

.. cpp:class:: GeneratorFreeBSD : public GeneratorMT

  .. cpp:function:: GeneratorFreeBSD()
  .. cpp:function:: virtual ~GeneratorFreeBSD()
  .. cpp:function:: virtual bool initialize()
  .. cpp:function:: virtual bool canFastHandle()
  .. cpp:function:: virtual ArchEvent *getEvent(bool block)


.. cpp:class:: ArchEventFreeBSD : public ArchEvent

  .. cpp:member:: int status
  .. cpp:member:: pid_t pid
  .. cpp:member:: lwpid_t lwp
  .. cpp:member:: bool interrupted
  .. cpp:member:: int error
  .. cpp:function:: ArchEventFreeBSD(bool inter_)
  .. cpp:function:: ArchEventFreeBSD(pid_t p, lwpid_t l, int s)
  .. cpp:function:: ArchEventFreeBSD(int e)
  .. cpp:function:: virtual ~ArchEventFreeBSD()


.. cpp:class:: DecoderFreeBSD : public Decoder

  .. cpp:function:: DecoderFreeBSD()
  .. cpp:function:: virtual ~DecoderFreeBSD()
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events)
  .. cpp:function:: Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch)


.. cpp:class:: freebsd_process : virtual public sysv_process, virtual public unix_process, virtual public x86_process, virtual public thread_db_process, virtual public mmap_alloc_process, virtual public hybrid_lwp_control_process

  .. cpp:function:: freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: freebsd_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~freebsd_process()
  .. cpp:function:: virtual bool plat_create()
  .. cpp:function:: virtual bool plat_attach(bool allStopped, bool &)
  .. cpp:function:: virtual bool plat_forked()
  .. cpp:function:: virtual bool plat_execed()
  .. cpp:function:: virtual bool plat_detach(result_response::ptr resp, bool leave_stopped)
  .. cpp:function:: virtual bool plat_terminate(bool &needs_sync)
  .. cpp:function:: virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address remote, size_t size)
  .. cpp:function:: virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address remote, size_t size, bp_write_t bp_write)
  .. cpp:function:: virtual bool needIndividualThreadAttach()
  .. cpp:function:: virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()
  .. cpp:function:: virtual bool plat_individualRegAccess()
  .. cpp:function:: virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates)
  .. cpp:function:: virtual OSType getOS() const
  .. cpp:function:: virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual async_ret_t post_create(std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual int getEventQueue()
  .. cpp:function:: virtual bool initKQueueEvents()
  .. cpp:function:: virtual SymbolReaderFactory *plat_defaultSymReader()
  .. cpp:function:: virtual bool plat_threadOpsNeedProcStop()
  .. cpp:function:: virtual bool forked()
  .. cpp:function:: virtual bool isForking() const
  .. cpp:function:: virtual void setForking(bool b)
  .. cpp:function:: virtual bool post_forked()
  .. cpp:function:: virtual freebsd_process *getParent()
  .. cpp:function:: virtual const char *getThreadLibName(const char *symName)
  .. cpp:function:: virtual bool isSupportedThreadLib(string libName)
  .. cpp:function:: virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo)
  .. cpp:function:: virtual bool plat_suspendThread(int_thread *thr)
  .. cpp:function:: virtual bool plat_resumeThread(int_thread *thr)
  .. cpp:member:: protected string libThreadName
  .. cpp:member:: protected bool forking
  .. cpp:member:: protected freebsd_process *parent


.. cpp:class:: freebsd_thread : public thread_db_thread

  .. cpp:function:: freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: freebsd_thread()
  .. cpp:function:: virtual ~freebsd_thread()
  .. cpp:function:: virtual bool plat_cont()
  .. cpp:function:: virtual bool plat_stop()
  .. cpp:function:: virtual bool plat_getAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool plat_setAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
  .. cpp:function:: virtual bool attach()
  .. cpp:function:: virtual bool plat_suspend()
  .. cpp:function:: virtual bool plat_resume()
  .. cpp:function:: virtual bool plat_setStep()
  .. cpp:function:: void setBootstrapStop(bool b)
  .. cpp:function:: bool hasBootstrapStop() const
  .. cpp:function:: void setPCBugCondition(bool b)
  .. cpp:function:: bool hasPCBugCondition() const
  .. cpp:function:: void setPendingPCBugSignal(bool b)
  .. cpp:function:: bool hasPendingPCBugSignal() const
  .. cpp:function:: void setSignalStopped(bool b)
  .. cpp:function:: bool isSignalStopped() const
  .. cpp:function:: bool isAlive()
  .. cpp:member:: protected bool bootstrapStop
  .. cpp:member:: protected bool pcBugCondition
  .. cpp:member:: protected bool pendingPCBugSignal
  .. cpp:member:: protected bool signalStopped
  .. cpp:member:: protected bool is_pt_setstep
  .. cpp:member:: protected bool is_exited


.. cpp:class:: FreeBSDPollLWPDeathHandler : public Handler

  .. cpp:function:: FreeBSDPollLWPDeathHandler()
  .. cpp:function:: virtual ~FreeBSDPollLWPDeathHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: FreeBSDPostThreadDeathBreakpointHandler : public Handler

  .. cpp:function:: FreeBSDPostThreadDeathBreakpointHandler()
  .. cpp:function:: virtual ~FreeBSDPostThreadDeathBreakpointHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDPreStopHandler : public Handler

  Only available when ``bug_freebsd_mt_suspend`` is defined.

  .. cpp:function:: FreeBSDPreStopHandler()
  .. cpp:function:: virtual ~FreeBSDPreStopHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: FreeBSDBootstrapHandler : public Handler

  Only available when ``bug_freebsd_mt_suspend`` is defined.

  .. cpp:function:: FreeBSDBootstrapHandler()
  .. cpp:function:: virtual ~FreeBSDBootstrapHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDChangePCHandler : public Handler

  Only available when ``bug_freebsd_change_pc`` is defined.

  .. cpp:function:: FreeBSDChangePCHandler()
  .. cpp:function:: virtual ~FreeBSDChangePCHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDPreForkHandler : public Handler

  .. cpp:function:: FreeBSDPreForkHandler()
  .. cpp:function:: ~FreeBSDPreForkHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


FreeBSD Bug Descriptions
************************

bug_freebsd_missing_sigstop
---------------------------

This bug is documented in FreeBSD problem report kern/142757 -- if we yield to the scheduler, we may avoid the race condition
described in this bug report.

The basic idea of the workaround it is to yield to other processes
(and the debuggee) to avoid the race condition described in the bug report. A call to sched_yield
attempts to do this.

Specifically, the race condition results from the following sequence of
events, where P = parent process, C = child process:

``P-SIGSTOP C-STOP P-WAITPID P-CONT P-SIGSTOP C-RUN``

Because the child doesn't run before the parent sends the second stop signal, the child
doesn't receive the second SIGSTOP.

A workaround for this problem would guarantee that the C-RUN
event always occurs before the second P-SIGSTOP.

bug_freebsd_mt_suspend
----------------------

Here is the scenario:

  1) The user requests an attach to a multithreaded process
  2) ProcControl issues the attach and waits for the stop
  3) On receiving the stop, ProcControl suspends each thread in the process.
  4) User continues a single thread but other threads which should remain stopped continue to run

The workaround for this bug is to issue SIGSTOPs to all the threads (except the thread that
received the SIGSTOP from the initial attach). Once the stops are handled, the suspend state will be
honored when continuing a whole process; that is, when all threads are suspended, a continue of one
thread will not continue another thread.

See the Stop, Bootstrap event handlers and the
post_attach function for more detailed comments.

bug_freebsd_change_pc
---------------------

Here is the scenario:

  1) A signal is received on a thread in a multithreaded process Let T = a thread that did
     not receive the signal and is blocked in the kernel.

  2) This signal generates a ProcControl event

  3) In the handler/callback for this event, the PC of T is changed. 4) When T is continued, it
     remains blocked in the kernel and does not run at the new PC.

This bug manifests itself in the iRPC tests in the testsuite. The debuggee's threads are blocked in the kernel on a mutex and the
iRPC tests change the PCs of these threads to run the RPCs. Specifically, this bug is encountered
when RPCs are posted from callbacks because the thread hasn't been stopped directly by a signal
rather by another thread in the process receiving a signal.

The workaround is to send a signal to a thread after it's PC has been changed in a callback or due to prepping an RPC. The handler for
this thread only unsets a flag in the thread. This required a special event, ChangePCStop, which is
only defined on platforms this bug affects.

See the ChangePCHandler as well for more info.

bug_freebsd_lost_signal
-----------------------

This bug is documented in FreeBSD problem report kern/150138 -- it
also includes a patch that fixes the problem.

Here is the scenario:

  1) A signal (such as a SIGTRAP) is delivered to a specific thread, which stops the whole process.

  2) The user (or handler) thread wishes to make a specific thread stop. Not knowing that the process has already been
     stopped, a SIGSTOP is sent to the thread. The user (or handler) thread then waits for the stop event.

  3) The generator observes the SIGTRAP event (via waitpid) and generates the event for it.
  4) The user (or handler) thread handle the SIGTRAP event but continues to wait on the pending stop.

At this point, the user (or handler) thread could wait indefinitely for the pending stop, if
the debuggee is blocked in the kernel. The problem is that when the process is continued after the
SIGSTOP is sent to the stopped  process, the thread blocked in the kernel does not wakeup to handle
the new signal that was delivered to it while it was stopped.

Unfortunately, I was not able to
come up with a solid workaround for this bug because there wasn't a race-free way to determine if
the process was stopped before sending it a signal (i.e., after checking whether the process was
stopped, the OS could deschedule the ProcControl process and the debuggee could hit a breakpoint or
finish an iRPC, resulting in ProcControl's model of the debuggee being inconsistent).

bug_freebsd_attach_stop
-----------------------

When attaching to a stopped process on FreeBSD, the stopped process is
resumed on attach. This isn't the expected behavior. The best we can do is send a signal to the
debuggee after attach to allow us to regain control of the debuggee.
