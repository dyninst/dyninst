.. _`sec:irpc.h`:

irpc.h
######

.. cpp:class:: iRPCAllocation

  .. cpp:type:: boost::shared_ptr<iRPCAllocation> ptr
  .. cpp:function:: iRPCAllocation()

    ``needs_datasave`` is set to ``false`` by default so that we do need a data save. If we've just
    allocated space, why save the data?

  .. cpp:function:: ~iRPCAllocation()
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: unsigned long size
  .. cpp:member:: unsigned start_offset
  .. cpp:member:: void *orig_data
  .. cpp:member:: bool needs_datasave
  .. cpp:member:: bool have_saved_regs
  .. cpp:member:: int ref_count
  .. cpp:member:: boost::weak_ptr<int_iRPC> creation_irpc

    ``NULL`` if the user handed us memory to run the iRPC in.

  .. cpp:member:: boost::weak_ptr<int_iRPC> deletion_irpc

    ``NULL`` if the user handed us memory to run the iRPC in.


.. cpp:class:: int_iRPC : public boost::enable_shared_from_this<int_iRPC>

  .. cpp:type:: boost::shared_ptr<int_iRPC> ptr
  .. cpp:function:: int_iRPC(void *binary_blob_, unsigned long binary_size_, bool async_, bool alreadyAllocated = false, Dyninst::Address addr = 0)
  .. cpp:function:: ~int_iRPC()
  .. cpp:function:: unsigned long id() const
  .. cpp:function:: State getState() const
  .. cpp:function:: Type getType() const
  .. cpp:function:: const char *getStrType() const
  .. cpp:function:: const char *getStrState() const
  .. cpp:function:: void *binaryBlob() const
  .. cpp:function:: unsigned long binarySize() const
  .. cpp:function:: unsigned long startOffset() const
  .. cpp:function:: bool isAsync() const
  .. cpp:function:: int_thread *thread() const
  .. cpp:function:: IRPC::weak_ptr getIRPC() const
  .. cpp:function:: iRPCAllocation::ptr allocation() const
  .. cpp:function:: iRPCAllocation::ptr targetAllocation() const
  .. cpp:function:: bool isProcStopRPC() const
  .. cpp:function:: bool isInternalRPC() const
  .. cpp:function:: unsigned long allocSize() const
  .. cpp:function:: Dyninst::Address addr() const
  .. cpp:function:: bool hasSavedRegs() const
  .. cpp:function:: bool userAllocated() const
  .. cpp:function:: bool shouldSaveData() const
  .. cpp:function:: bool isMemManagementRPC() const
  .. cpp:function:: int_iRPC::ptr allocationRPC() const
  .. cpp:function:: int_iRPC::ptr deletionRPC() const
  .. cpp:function:: bool isRPCPrepped()
  .. cpp:function:: bool needsToRestoreInternal() const
  .. cpp:function:: void setRestoreInternal(bool b)
  .. cpp:function:: bool saveRPCState()
  .. cpp:function:: bool checkRPCFinishedSave()
  .. cpp:function:: bool writeToProc()
  .. cpp:function:: bool checkRPCFinishedWrite()
  .. cpp:function:: bool runIRPC()
  .. cpp:function:: void setState(State s)
  .. cpp:function:: void setType(Type t)
  .. cpp:function:: void setBinaryBlob(void *b)
  .. cpp:function:: void setBinarySize(unsigned long s)
  .. cpp:function:: void copyBinaryBlob(void *b, unsigned long s)
  .. cpp:function:: void setStartOffset(unsigned long o)
  .. cpp:function:: void setAsync(bool a)
  .. cpp:function:: void setThread(int_thread *t)
  .. cpp:function:: void setIRPC(IRPC::weak_ptr o)
  .. cpp:function:: void setAllocation(iRPCAllocation::ptr i)
  .. cpp:function:: void setTargetAllocation(iRPCAllocation::ptr i)
  .. cpp:function:: void setAllocSize(unsigned long size)
  .. cpp:function:: void setShouldSaveData(bool b)
  .. cpp:function:: bool fillInAllocation()
  .. cpp:function:: bool countedSync()
  .. cpp:function:: void setDirectFree(bool s)
  .. cpp:function:: bool directFree() const
  .. cpp:function:: void getPendingResponses(std::set<response::ptr> &resps)
  .. cpp:function:: void syncAsyncResponses(bool is_sync)
  .. cpp:function:: int_iRPC::ptr newAllocationRPC()
  .. cpp:function:: int_iRPC::ptr newDeallocationRPC()
  .. cpp:function:: Dyninst::Address infMallocResult()
  .. cpp:function:: void setMallocResult(Dyninst::Address addr)
  .. cpp:function:: rpc_wrapper *getWrapperForDecode()
  .. cpp:function:: Address getInfFreeTarget()
  .. cpp:function:: int_thread::State getRestoreToState() const
  .. cpp:function:: void setRestoreToState(int_thread::State s)

.. cpp:enum:: int_iRPC::State

  .. cpp:enumerator:: Unassigned = 0
  .. cpp:enumerator:: Posted = 1

    RPC is in queue to run

  .. cpp:enumerator:: Prepping = 2

    Thread/Process is being stopped to setup RPC

  .. cpp:enumerator:: Prepped = 3

    Thread/Process has been stopped to setup RPC

  .. cpp:enumerator:: Saving = 4

    Process state is being saved

  .. cpp:enumerator:: Saved = 5

    Process state has been saved

  .. cpp:enumerator:: Writing = 6

    RPC is being written into the process

  .. cpp:enumerator:: Ready = 7

    RPC is setup on thread and needs continue

  .. cpp:enumerator:: Running = 8

    RPC is running

  .. cpp:enumerator:: Cleaning = 9

    RPC is complete and is being remove

  .. cpp:enumerator:: Finished = 10

    RPC ran

.. cpp:enum:: int_iRPC::Type

  .. cpp:enumerator:: NoType
  .. cpp:enumerator:: Allocation
  .. cpp:enumerator:: Deallocation
  .. cpp:enumerator:: User
  .. cpp:enumerator:: InfMalloc
  .. cpp:enumerator:: InfFree


.. cpp:class:: iRPCMgr

  Singleton class, only one of these across all processes.

  .. cpp:function:: iRPCMgr()
  .. cpp:function:: ~iRPCMgr()
  .. cpp:function:: unsigned numActiveRPCs(int_thread *thr)
  .. cpp:function:: iRPCAllocation::ptr findAllocationForRPC(int_thread *thread, int_iRPC::ptr rpc)
  .. cpp:function:: bool postRPCToProc(int_process *proc, int_iRPC::ptr rpc)
  .. cpp:function:: bool postRPCToThread(int_thread *thread, int_iRPC::ptr rpc)
  .. cpp:function:: int_thread* createThreadForRPC(int_process* proc, int_thread* best_candidate)
  .. cpp:function:: int_iRPC::ptr createInfMallocRPC(int_process *proc, unsigned long size, bool use_addr, Dyninst::Address addr)
  .. cpp:function:: int_iRPC::ptr createInfFreeRPC(int_process *proc, unsigned long size, Dyninst::Address addr)
  .. cpp:function:: bool isRPCTrap(int_thread *thr, Dyninst::Address addr)

.. cpp:function:: iRPCMgr *rpcMgr()

.. cpp:class:: iRPCHandler : public Handler

  Runs after user callback

  .. cpp:function:: iRPCHandler()
  .. cpp:function:: virtual ~iRPCHandler()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: iRPCPreCallbackHandler : public Handler

  .. cpp:function:: iRPCPreCallbackHandler()
  .. cpp:function:: virtual ~iRPCPreCallbackHandler()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: iRPCLaunchHandler : public Handler

  .. cpp:function:: iRPCLaunchHandler()
  .. cpp:function:: virtual ~iRPCLaunchHandler()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: rpc_wrapper

  Wraps an int_iRPC::ptr so that the user level class IRPC doesn't
  need to directly maintain a shared pointer into internal code.

  .. cpp:function:: rpc_wrapper(int_iRPC::ptr rpc_)
  .. cpp:function:: rpc_wrapper(rpc_wrapper *w)
  .. cpp:function:: ~rpc_wrapper()
  .. cpp:member:: int_iRPC::ptr rpc
