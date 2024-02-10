.. _`sec:dyninstAPI:frame.h`:

frame.h
#######


.. cpp:class:: Frame

  .. cpp:function:: Frame()
  .. cpp:function:: Frame(const Dyninst::Stackwalker::Frame &swf, PCProcess *proc, PCThread *thread, bool uppermost)
  .. cpp:function:: Frame(const Frame &f)
  .. cpp:function:: const Frame &operator=(const Frame &f)
  .. cpp:function:: bool operator==(const Frame &F) const
  .. cpp:function:: Address getPC() const
  .. cpp:function:: Address getUninstAddr() const

    Unwind instrumentation

  .. cpp:function:: Address getFP() const
  .. cpp:function:: Address getSP() const
  .. cpp:function:: PCProcess *getProc() const
  .. cpp:function:: PCThread *getThread() const
  .. cpp:function:: void setThread(PCThread *thrd)
  .. cpp:function:: bool isUppermost() const
  .. cpp:function:: instPoint *getPoint()
  .. cpp:function:: baseTramp *getBaseTramp()
  .. cpp:function:: func_instance *getFunc()
  .. cpp:function:: bool isSignalFrame()
  .. cpp:function:: bool isInstrumentation()
  .. cpp:function:: Address getPClocation()
  .. cpp:function:: bool setPC(Address newpc)
  .. cpp:function:: bool setRealReturnAddr(Address retaddr)

    We store the actual return addr in a word on the stack

  .. cpp:member:: private Dyninst::Stackwalker::Frame sw_frame_

      StackwalkerAPI frame

  .. cpp:member:: private PCProcess *proc_

      We're only valid for a single process anyway

  .. cpp:member:: private PCThread *thread_

      User-level thread

  .. cpp:member:: private bool uppermost_

.. cpp:class:: int_stackwalk

  .. cpp:function:: int_stackwalk()
  .. cpp:function:: bool isValid()
  .. cpp:function:: bool setStackwalk(std::vector<Frame> &new_stack)
  .. cpp:function:: bool clear()
  .. cpp:function:: std::vector<Frame> &getStackwalk()
