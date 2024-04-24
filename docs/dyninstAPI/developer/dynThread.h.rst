.. _`sec:dynThread.h`:

dynThread.h
###########

.. cpp:class:: PCThread

  **Encapsulates a ProcControlAPI thread**

  This class is meant to be a replacement for the old dyn_thread class.

  .. cpp:function:: static PCThread *createPCThread(PCProcess *parent, ProcControlAPI::Thread::ptr thr)

  ......

  .. rubric::
    Stackwalking interface
  
  .. cpp:function:: bool walkStack(std::vector<Frame> &stackWalk)
  .. cpp:function:: Frame getActiveFrame()
  .. cpp:function:: bool getRegisters(ProcControlAPI::RegisterPool &regs, bool includeFP = false)
  .. cpp:function:: bool changePC(Address newPC)

  ......

  .. rubric::
    Field accessors
    
  .. cpp:function:: int getIndex() const
  .. cpp:function:: Dyninst::LWP getLWP() const
  .. cpp:function:: PCProcess *getProc() const
  .. cpp:function:: bool isLive() const
  .. cpp:function:: ProcControlAPI::Thread::ptr pcThr() const

  ......

  .. rubric::
    Thread info
    
  .. cpp:function:: dynthread_t getTid() const
  .. cpp:function:: func_instance *getStartFunc()
  .. cpp:function:: Address getStackAddr()
  .. cpp:function:: void clearStackwalk()
  .. cpp:function:: protected PCThread(PCProcess *parent, int ind, ProcControlAPI::Thread::ptr thr)
  .. cpp:function:: protected void markExited()
  .. cpp:function:: protected void setTid(dynthread_t tid)
  .. cpp:function:: protected bool continueThread()
  .. cpp:function:: protected bool isRunning()
  .. cpp:function:: protected bool postIRPC(inferiorRPCinProgress *newRPC)
  .. cpp:function:: protected void findStartFunc()
  .. cpp:function:: protected void findStackTop()
  .. cpp:function:: protected void findSingleThreadInfo()
  .. cpp:member:: protected PCProcess *proc_
  .. cpp:member:: protected ProcControlAPI::Thread::ptr pcThr_
  .. cpp:member:: protected int index_
  .. cpp:member:: protected Address stackAddr_
  .. cpp:member:: protected Address startFuncAddr_
  .. cpp:member:: protected func_instance *startFunc_
  .. cpp:member:: protected int_stackwalk cached_stackwalk_

    When we run an inferior RPC we cache the stackwalk of the process and return that if anyone asks for a stack walk

  .. cpp:member:: protected Dyninst::LWP savedLWP_
  .. cpp:member:: protected dynthread_t savedTid_
  .. cpp:member:: protected dynthread_t manuallySetTid_

    retrieved from the mutatee
