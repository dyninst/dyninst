.. _`sec:syscallNotification.h`:

syscallNotification.h
#####################

.. code:: cpp

  // Non-NULL for platforms where we don't need the instMapping
  #define SYSCALL_INSTALLED ((instMapping *)1)


.. cpp:class:: syscallNotification

  .. cpp:member:: private instMapping *preForkInst

    If we use instrumentation to get notification of a syscall

  .. cpp:member:: private instMapping *postForkInst
  .. cpp:member:: private instMapping *preExecInst
  .. cpp:member:: private instMapping *postExecInst
  .. cpp:member:: private instMapping *preExitInst
  .. cpp:member:: private instMapping *preLwpExitInst
  .. cpp:member:: private PCProcess *proc

  ......

  .. rubric::
    Platform dependent

  .. cpp:function:: private const char *getForkFuncName()
  .. cpp:function:: private const char *getExecFuncName()
  .. cpp:function:: private const char *getExitFuncName()

  ......

  .. cpp:function:: syscallNotification()
  .. cpp:function:: syscallNotification(PCProcess *p)
  .. cpp:function:: syscallNotification(syscallNotification *parentSN, PCProcess *p)

    fork constructor

  .. cpp:function:: ~syscallNotification()
  .. cpp:function:: bool installPreFork()
  .. cpp:function:: bool installPostFork()
  .. cpp:function:: bool installPreExec()
  .. cpp:function:: bool installPostExec()
  .. cpp:function:: bool installPreExit()
  .. cpp:function:: bool installPreLwpExit()
  .. cpp:function:: bool removePreFork()
  .. cpp:function:: bool removePostFork()
  .. cpp:function:: bool removePreExec()
  .. cpp:function:: bool removePostExec()
  .. cpp:function:: bool removePreExit()
  .. cpp:function:: bool removePreLwpExit()
