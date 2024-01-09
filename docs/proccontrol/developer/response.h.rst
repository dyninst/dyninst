.. _`sec:response.h`:

response.h
##########

.. cpp:function:: unsigned newResponseID()
.. cpp:function:: unsigned newResponseID(unsigned size);
.. cpp:function:: responses_pending &getResponses()


.. cpp:class:: response : public boost::enable_shared_from_this<response>

  .. cpp:member:: protected Dyninst::ProcControlAPI::Event::ptr event
  .. cpp:member:: protected unsigned int id
  .. cpp:member:: protected static unsigned int next_id
  .. cpp:member:: protected state_t state
  .. cpp:member:: protected mutable bool checked_ready
  .. cpp:member:: protected bool isSyncHandled
  .. cpp:member:: protected bool error
  .. cpp:member:: protected int errorcode
  .. cpp:member:: protected int_process *proc
  .. cpp:member:: protected int_eventAsyncIO *aio
  .. cpp:function:: protected response()
  .. cpp:member:: protected resp_type_t resp_type
  .. cpp:member:: protected ArchEvent *decoder_event
  .. cpp:member:: protected int multi_resp_size
  .. cpp:member:: protected int multi_resp_recvd
  .. cpp:type:: boost::shared_ptr<response> ptr
  .. cpp:type:: boost::shared_ptr<const response> const_ptr
  .. cpp:function:: virtual ~response()
  .. cpp:function:: unsigned int getID() const
  .. cpp:function:: boost::shared_ptr<result_response> getResultResponse()
  .. cpp:function:: boost::shared_ptr<mem_response> getMemResponse()
  .. cpp:function:: boost::shared_ptr<reg_response> getRegResponse()
  .. cpp:function:: boost::shared_ptr<allreg_response> getAllRegResponse()
  .. cpp:function:: boost::shared_ptr<stack_response> getStackResponse()
  .. cpp:function:: boost::shared_ptr<data_response> getDataResponse()
  .. cpp:function:: bool isReady() const
  .. cpp:function:: bool testReady() const
  .. cpp:function:: bool isPosted() const
  .. cpp:function:: bool hasError() const
  .. cpp:function:: int errorCode() const
  .. cpp:function:: void markPosted()
  .. cpp:function:: void markReady()
  .. cpp:function:: void markError(int code = 0)
  .. cpp:function:: void markSyncHandled()
  .. cpp:function:: void setEvent(Event::ptr ev)
  .. cpp:function:: Event::ptr getEvent() const
  .. cpp:function:: unsigned int markAsMultiResponse(int num_resps)
  .. cpp:function:: bool isMultiResponse()
  .. cpp:function:: unsigned int multiResponseSize()
  .. cpp:function:: bool isMultiResponseComplete()
  .. cpp:function:: void setDecoderEvent(ArchEvent *ae)
  .. cpp:function:: ArchEvent *getDecoderEvent()
  .. cpp:function:: void setProcess(int_process *p)
  .. cpp:function:: int_process *getProcess() const
  .. cpp:function:: int_eventAsyncIO *getAsyncIOEvent()
  .. cpp:function:: void setAsyncIOEvent(int_eventAsyncIO *aoi_)
  .. cpp:function:: std::string name() const


.. cpp:enum:: response::state_t

  .. cpp:enumerator:: unset
  .. cpp:enumerator:: posted
  .. cpp:enumerator:: ready

.. cpp:enum:: response::resp_type_t

  .. cpp:enumerator:: rt_result
  .. cpp:enumerator:: rt_reg
  .. cpp:enumerator:: rt_allreg
  .. cpp:enumerator:: rt_mem
  .. cpp:enumerator:: rt_stack
  .. cpp:enumerator:: rt_data
  .. cpp:enumerator:: rt_set


.. cpp:class:: responses_pending

  .. cpp:function:: response::ptr rmResponse(unsigned int id)
  .. cpp:function:: response::ptr getResponse(unsigned int id)
  .. cpp:function:: bool waitFor(response::ptr resp)
  .. cpp:function:: void addResponse(response::ptr r, int_process *proc)
  .. cpp:function:: void noteResponse()
  .. cpp:function:: bool hasAsyncPending(bool ev_only = true)
  .. cpp:function:: CondVar<Mutex <false> > &condvar()
  .. cpp:function:: void lock()
  .. cpp:function:: void unlock()
  .. cpp:function:: void signal()


.. cpp:class:: result_response : public response

  .. cpp:type:: boost::shared_ptr<result_response> ptr
  .. cpp:type:: boost::shared_ptr<const result_response> const_ptr
  .. cpp:function:: static result_response::ptr createResultResponse()
  .. cpp:function:: virtual ~result_response()
  .. cpp:function:: void setResponse(bool b)
  .. cpp:function:: void postResponse(bool b)
  .. cpp:function:: bool getResult() const


.. cpp:class:: reg_response : public response

  .. cpp:type:: boost::shared_ptr<reg_response> ptr
  .. cpp:type:: boost::shared_ptr<const reg_response> const_ptr
  .. cpp:function:: static reg_response::ptr createRegResponse()
  .. cpp:function:: virtual ~reg_response()
  .. cpp:function:: void setRegThread(Dyninst::MachRegister r, int_thread *t)
  .. cpp:function:: void setResponse(Dyninst::MachRegisterVal v)
  .. cpp:function:: void postResponse(Dyninst::MachRegisterVal v)
  .. cpp:function:: Dyninst::MachRegisterVal getResult() const


.. cpp:class:: allreg_response : public response

  .. cpp:type:: boost::shared_ptr<allreg_response> ptr
  .. cpp:type:: boost::shared_ptr<const allreg_response> const_ptr
  .. cpp:function:: static allreg_response::ptr createAllRegResponse(int_registerPool *regpool)
  .. cpp:function:: static allreg_response::ptr createAllRegResponse()
  .. cpp:function:: virtual ~allreg_response()
  .. cpp:function:: void setThread(int_thread *t)
  .. cpp:function:: void setRegPool(int_registerPool *p)
  .. cpp:function:: void setResponse()
  .. cpp:function:: void postResponse()
  .. cpp:function:: void setIndividualRegAccess(reg_response::ptr iacc, Dyninst::MachRegister ireg)
  .. cpp:function:: Dyninst::MachRegister getIndividualReg()
  .. cpp:function:: reg_response::ptr getIndividualAcc()
  .. cpp:function:: int_registerPool *getRegPool() const


.. cpp:class:: mem_response : public response

  .. cpp:type:: boost::shared_ptr<mem_response> ptr
  .. cpp:type:: boost::shared_ptr<const mem_response> const_ptr
  .. cpp:function:: static mem_response::ptr createMemResponse()
  .. cpp:function:: static mem_response::ptr createMemResponse(char *targ, unsigned targ_size)
  .. cpp:function:: virtual ~mem_response()
  .. cpp:function:: char *getBuffer() const
  .. cpp:function:: unsigned getSize() const
  .. cpp:function:: void setBuffer(char *targ, unsigned targ_size)
  .. cpp:function:: void setResponse(char *src, unsigned src_size)
  .. cpp:function:: void setResponse()
  .. cpp:function:: void postResponse(char *src, unsigned src_size, Address src_addr = 0)
  .. cpp:function:: void postResponse()
  .. cpp:function:: void setLastBase(Address a)
  .. cpp:function:: Address lastBase()


.. cpp:class:: stack_response : public response

  .. cpp:type:: boost::shared_ptr<stack_response> ptr
  .. cpp:type:: boost::shared_ptr<const stack_response> const_ptr
  .. cpp:function:: static stack_response::ptr createStackResponse(int_thread *t)
  .. cpp:function:: virtual ~stack_response()
  .. cpp:function:: void *getData()
  .. cpp:function:: int_thread *getThread()
  .. cpp:function:: void postResponse(void *d)


.. cpp:class:: data_response : public response

  .. cpp:type:: boost::shared_ptr<data_response> ptr
  .. cpp:type:: boost::shared_ptr<const data_response> const_ptr
  .. cpp:function:: static data_response::ptr createDataResponse()
  .. cpp:function:: virtual ~data_response()
  .. cpp:function:: void *getData()
  .. cpp:function:: void postResponse(void *d)


.. cpp:class:: ResponseSet

  .. cpp:function:: ResponseSet()
  .. cpp:function:: void addID(unsigned resp_id, unsigned index)
  .. cpp:function:: unsigned getID() const
  .. cpp:function:: unsigned getIDByIndex(unsigned int index, bool &found) const
  .. cpp:function:: static ResponseSet *getResponseSetByID(unsigned)

