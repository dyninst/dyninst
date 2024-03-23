.. _`sec:resp.h`:

resp.h
######

.. cpp:class:: resp_process : virtual public int_process

  A second implementation of the response class. On async systems, it tracks command
  messages we've sent to the OS, and associates them with the responses we receive
  from the OS.

  Resp has several improvements over response:

    - Integrated support for multi-responses, where you get multiple responses from one message.
    - Replacement of response's global lock with a  per-process condition variable, which should
      have significantly less contention
    - Templated parameters to hold command specific data.

  Over the long-term, it would be nice to completely replace response with Resp, but for the
  moment Resp will just be used for new code.

  .. cpp:function:: private void addResp(Resp_ptr resp, unsigned id_start, unsigned id_end)
  .. cpp:function:: private void markRespPosted(Resp_ptr resp)
  .. cpp:function:: private void rmResponse(Resp_ptr resp, bool lock_held = false)
  .. cpp:function:: private void markRespDone(Resp_ptr resp)
  .. cpp:member:: private std::map<int, Resp_ptr> active_resps
  .. cpp:member:: private CondVar<Mutex<false>> active_resps_lock
  .. cpp:function:: Resp_ptr recvResp(unsigned int id, bool &is_complete)
  .. cpp:function:: resp_process(Dyninst::PID p, std::string e, std::vector<std::string> a,\
                                 std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: resp_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: ~resp_process()
  .. cpp:function:: void waitForEvent(Resp_ptr resp)

.. cpp:class:: Resp

  .. cpp:member:: protected unsigned int id_start
  .. cpp:member:: protected unsigned int id_end
  .. cpp:member:: protected unsigned int num_recvd
  .. cpp:member:: protected bool isWaitedOn
  .. cpp:member:: protected bool isCleaned
  .. cpp:member:: protected resp_process *proc
  .. cpp:member:: protected Dyninst::ProcControlAPI::Event::ptr event
  .. cpp:function:: protected void init()
  .. cpp:type:: Resp_ptr ptr
  .. cpp:function:: Resp(resp_process *proc_)
  .. cpp:function:: Resp(unsigned int multi_size, resp_process *proc_)
  .. cpp:function:: virtual ~Resp()
  .. cpp:function:: resp_process *getProc()
  .. cpp:function:: unsigned int getID()
  .. cpp:function:: unsigned int getIDEnd()
  .. cpp:function:: void post()
  .. cpp:function:: void done()
  .. cpp:function:: bool hadError() const


.. cpp:enum:: Resp::state

  .. cpp:enumerator:: Setup
  .. cpp:enumerator:: Posted
  .. cpp:enumerator:: Received
  .. cpp:enumerator:: Done
  .. cpp:enumerator:: Error


.. cpp:type:: Resp* Resp_ptr
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::stat64_ptr> StatResp_t
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::int_eventAsyncFileRead> FileReadResp_t
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::FileSet> FileSetResp_t
.. cpp:type:: RespItem<unsigned long> MemUsageResp_t


.. code:: cpp

  #define Resp_ptr_NULL NULL
