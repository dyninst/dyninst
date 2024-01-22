.. _`sec-dev:frame.h`:

frame.h
#######

.. cpp:namespace:: Dyninst::Stackwalker::dev

.. cpp:class:: Frame : public AnnotatableDense

  .. cpp:member:: protected Dyninst::MachRegisterVal ra
  .. cpp:member:: protected Dyninst::MachRegisterVal fp
  .. cpp:member:: protected Dyninst::MachRegisterVal sp
  .. cpp:member:: protected location_t ra_loc
  .. cpp:member:: protected location_t fp_loc
  .. cpp:member:: protected location_t sp_loc
  .. cpp:member:: protected mutable std::string sym_name
  .. cpp:member:: protected mutable void *sym_value
  .. cpp:member:: protected mutable @val_set_t name_val_set
  .. cpp:member:: protected bool top_frame
  .. cpp:member:: protected bool bottom_frame
  .. cpp:member:: protected bool frame_complete
  .. cpp:member:: protected bool non_call_frame
  .. cpp:member:: protected const Frame *prev_frame
  .. cpp:member:: protected FrameStepper *stepper
  .. cpp:member:: protected FrameStepper *next_stepper
  .. cpp:member:: protected Walker *walker
  .. cpp:member:: protected THR_ID originating_thread
  .. cpp:function:: protected void setStepper(FrameStepper *newstep)
  .. cpp:function:: protected void setWalker(Walker *newwalk)
  .. cpp:function:: protected void markTopFrame()
  .. cpp:function:: protected void markBottomFrame()
  .. cpp:function:: protected void setNameValue() const

  .. cpp:function:: bool nonCall() const

      Dyninst instrumentation constructs "synthetic" frames that don't correspond
      to calls. We really need to know about these. Also, signal handlers.

.. cpp:enum:: Frame::@val_set_t

  .. cpp:enumerator:: nv_unset
  .. cpp:enumerator:: nv_set
  .. cpp:enumerator:: nv_err

.. cpp:class:: FrameNode

  .. cpp:member:: @frame_type_t frame_type


.. cpp:enum:: FrameNode::@frame_type_t

  .. cpp:enumerator:: FTFrame
  .. cpp:enumerator:: FTThread
  .. cpp:enumerator:: FTString
  .. cpp:enumerator:: FTHead

