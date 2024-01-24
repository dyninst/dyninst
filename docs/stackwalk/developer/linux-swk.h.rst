.. _`sec:linux-swk.h`:

linux-swk.h
###########

.. cpp:namespace:: Dyninst::Stackwalker

.. code:: cpp

  #define START_THREAD_FUNC_NAME "start_thread"
  #define CLONE_FUNC_NAME "__clone"
  #define START_FUNC_NAME "_start"

.. cpp:struct:: vsys_info

  .. cpp:member:: void *vsys_mem
  .. cpp:member:: Dyninst::Address start
  .. cpp:member:: Dyninst::Address end
  .. cpp:member:: Dyninst::SymReader *syms
  .. cpp:member:: std::string name
  .. cpp:function:: vsys_info()
  .. cpp:function:: ~vsys_info()

.. cpp:function:: vsys_info *getVsysInfo(ProcessState *ps)
