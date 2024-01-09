.. _`sec:resp.h`:

resp.h
######

.. cpp:type:: Resp* Resp_ptr
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::stat64_ptr> StatResp_t
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::int_eventAsyncFileRead> FileReadResp_t
.. cpp:type:: RespItem<Dyninst::ProcControlAPI::FileSet> FileSetResp_t
.. cpp:type:: RespItem<unsigned long> MemUsageResp_t


.. code:: cpp

  #define Resp_ptr_NULL NULL

.. cpp:class:: resp_process : virtual public int_process

  .. cpp:function:: Resp_ptr recvResp(unsigned int id, bool &is_complete)
  .. cpp:function:: resp_process(Dyninst::PID p, std::string e, std::vector<std::string> a,  std::vector<std::string> envp, std::map<int,int> f)
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

.. cpp:class:: template<class T> RespItem : public Resp

  .. cpp:member:: T *obj

  .. cpp:function:: RespItem(T *obj_, resp_process *proc_)
  .. cpp:function:: RespItem(T *obj_, resp_process *proc_, unsigned multi_size)
  .. cpp:function:: virtual ~RespItem()
  .. cpp:function:: T *get()
