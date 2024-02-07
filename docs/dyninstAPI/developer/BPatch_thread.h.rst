.. _`sec-dev:BPatch_thread.h`:

BPatch_thread.h
###############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_thread
   
  **A thread of execution running in a process**

  .. cpp:member:: private bool madeExitCallback_

    Sometimes we get per-thread exit notifications, sometimes we  just get whole-process. So keep track of whether we've notified the user of an exit so we don't duplicate when the process exits.

  .. cpp:function:: protected BPatch_thread(BPatch_process *parent, PCThread *thr)
  .. cpp:function:: protected static BPatch_thread *createNewThread(BPatch_process *proc, PCThread *thr)

  .. cpp:function:: protected void updateThread(PCThread *newThr)

    Currently only used on an exec to replace the underlying PCThread

  .. cpp:function:: protected bool madeExitCallback()
  .. cpp:function:: protected void setMadeExitCallback()

