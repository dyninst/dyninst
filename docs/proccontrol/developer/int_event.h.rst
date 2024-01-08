.. _`sec:int_event.h`:

int_event.h
###########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: int_eventBreakpoint

  .. cpp:function:: int_eventBreakpoint(Address a, sw_breakpoint *i, int_thread *thr)
  .. cpp:function:: int_eventBreakpoint(hw_breakpoint *i, int_thread *thr)
  .. cpp:function:: ~int_eventBreakpoint()
  .. cpp:function:: bp_instance *lookupInstalledBreakpoint()
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: hw_breakpoint *hwbp
  .. cpp:member:: result_response::ptr pc_regset
  .. cpp:member:: int_thread *thrd
  .. cpp:member:: bool stopped_proc
  .. cpp:member:: std::set<Breakpoint::ptr> cb_bps


.. cpp:class:: int_eventBreakpointClear

  .. cpp:function:: int_eventBreakpointClear()
  .. cpp:function:: ~int_eventBreakpointClear()
  .. cpp:member:: std::set<response::ptr> bp_suspend
  .. cpp:member:: bool started_bp_suspends
  .. cpp:member:: bool cached_bp_sets
  .. cpp:member:: bool set_singlestep
  .. cpp:member:: bool stopped_proc
  .. cpp:member:: std::set<Thread::ptr> clearing_threads


.. cpp:class:: int_eventBreakpointRestore

  .. cpp:function:: int_eventBreakpointRestore(bp_instance *breakpoint_)
  .. cpp:function:: ~int_eventBreakpointRestore()
  .. cpp:member:: bool set_states
  .. cpp:member:: bool bp_resume_started
  .. cpp:member:: std::set<response::ptr> bp_resume
  .. cpp:member:: bp_instance *bp


.. cpp:class:: int_eventRPC

  .. cpp:function:: int_eventRPC()
  .. cpp:function:: ~int_eventRPC()
  .. cpp:member:: reg_response::ptr alloc_regresult
  .. cpp:member:: result_response::ptr memrestore_response
  .. cpp:member:: result_response::ptr regrestore_response
  .. cpp:function:: void getPendingAsyncs(std::set<response::ptr> &pending)


.. cpp:class:: int_eventAsync

  .. cpp:function:: int_eventAsync(response::ptr r)
  .. cpp:function:: ~int_eventAsync()
  .. cpp:function:: std::set<response::ptr> &getResponses()
  .. cpp:function:: void addResp(response::ptr r)


.. cpp:class:: int_eventNewUserThread

  .. cpp:function:: int_eventNewUserThread()
  .. cpp:function:: ~int_eventNewUserThread()
  .. cpp:member:: int_thread *thr
  .. cpp:member:: Dyninst::LWP lwp
  .. cpp:member:: void *raw_data
  .. cpp:member:: bool needs_update


.. cpp:class:: int_eventNewLWP

  .. cpp:function:: int_eventNewLWP()
  .. cpp:function:: ~int_eventNewLWP()
  .. cpp:member:: Dyninst::LWP lwp
  .. cpp:member:: int_thread::attach_status_t attach_status


.. cpp:class:: int_eventThreadDB

  .. cpp:function:: int_eventThreadDB()
  .. cpp:function:: ~int_eventThreadDB()
  .. cpp:member:: std::set<Event::ptr> new_evs
  .. cpp:member:: bool completed_new_evs
  .. cpp:member:: bool completed_getmsgs
  .. cpp:member:: std::vector<td_event_msg_t> msgs

      Only available when ``cap_thread_db`` is defined.

  .. cpp:member:: std::vector<td_thrhandle_t> handles

      Only available when ``cap_thread_db`` is defined.

.. cpp:class:: int_eventDetach

  .. cpp:function:: int_eventDetach()
  .. cpp:function:: ~int_eventDetach()
  .. cpp:member:: std::set<response::ptr> async_responses
  .. cpp:member:: result_response::ptr detach_response
  .. cpp:member:: bool temporary_detach
  .. cpp:member:: bool leave_stopped
  .. cpp:member:: bool removed_bps
  .. cpp:member:: bool done
  .. cpp:member:: bool had_error


.. cpp:class:: int_eventControlAuthority

  .. cpp:function:: int_eventControlAuthority(std::string toolname_, unsigned int toolid_, int priority_, EventControlAuthority::Trigger trigger_)
  .. cpp:function:: int_eventControlAuthority()
  .. cpp:function:: ~int_eventControlAuthority()
  .. cpp:member:: std::string toolname
  .. cpp:member:: unsigned int toolid
  .. cpp:member:: int priority
  .. cpp:member:: EventControlAuthority::Trigger trigger
  .. cpp:member:: bool control_lost
  .. cpp:member:: bool handled_bps
  .. cpp:member:: bool took_ca
  .. cpp:member:: bool did_desync
  .. cpp:member:: bool unset_desync
  .. cpp:member:: bool dont_delete
  .. cpp:member:: bool waiting_on_stop
  .. cpp:member:: std::set<response::ptr> async_responses
  .. cpp:member:: data_response::ptr dresp


.. cpp:class:: int_eventAsyncFileRead

  .. cpp:function:: int_eventAsyncFileRead()
  .. cpp:function:: ~int_eventAsyncFileRead()
  .. cpp:function:: bool isComplete()
  .. cpp:member:: void *data
  .. cpp:member:: size_t size
  .. cpp:member:: size_t orig_size
  .. cpp:member:: void *to_free
  .. cpp:member:: std::string filename
  .. cpp:member:: size_t offset
  .. cpp:member:: int errorcode
  .. cpp:member:: bool whole_file
  .. cpp:member:: Resp::ptr resp

.. cpp:class:: int_eventAsyncIO

  .. cpp:function:: int_eventAsyncIO(response::ptr resp_, asyncio_type)
  .. cpp:function:: ~int_eventAsyncIO()
  .. cpp:member:: response::ptr resp
  .. cpp:member:: void *local_memory
  .. cpp:member:: Address remote_addr
  .. cpp:member:: size_t size
  .. cpp:member:: void *opaque_value
  .. cpp:member:: asyncio_type iot
  .. cpp:member:: RegisterPool *rpool


.. cpp:enum:: int_eventAsyncIO::asyncio_type

  .. cpp:enumerator:: memread
  .. cpp:enumerator:: memwrite
  .. cpp:enumerator:: regallread
  .. cpp:enumerator:: regallwrite

