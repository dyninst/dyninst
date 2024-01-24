.. _`sec:steppergroup.h`:

steppergroup.h
##############

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: StepperGroup

  **A collection of frame steppers criteria for their use**

  The ``StepperGroup``\ ’s primary job is to decide which :cpp:class:`FrameStepper`
  should be used to walk through a stack frame given a return address. The default
  ``StepperGroup`` keeps a set of address ranges for each ``FrameStepper``. If multiple
  ``FrameStepper`` objects overlap an address, then the default ``StepperGroup`` will use a
  priority system to decide.

  .. cpp:member:: protected Walker *walker
  .. cpp:member:: protected std::set<FrameStepper *> steppers

  .. cpp:function:: StepperGroup(Walker *new_walker)

      Creates a group associated with ``new_walker``.

  .. cpp:function:: virtual bool addStepper(FrameStepper *stepper, Address start, Address end) = 0

      Adds the frame stepper ``stepper`` to the group and registers it to handle frames in the
      range ``[start, end)``.

  .. cpp:function:: virtual bool findStepperForAddr(Dyninst::Address addr, FrameStepper* &out, \
                                                    const FrameStepper *last_tried = NULL) = 0

      Returns in ``out`` the stepper to use for the stack frame created by a function at
      the address ``addr``.

      It may be possible that the ``FrameStepper`` this method decides on is
      unable to walk through the stack frame (it returns :cpp:enumerator:`gcframe_ret_t::gcf_not_me` from
      :cpp:func:`FrameStepper::getCallerFrame`). In this case StackwalkerAPI will call
      findStepperForAddr again with the last_tried parameter set to the failed
      ``FrameStepper``. findStepperForAddr should then find another
      ``FrameStepper`` to use. Parameter ``last_tried`` will be set to NULL
      the first time getStepperToUse is called for a stack frame.

      The default version of this method uses address ranges to decide which
      ``FrameStepper`` to use. The address ranges are contained within the
      process’ code space, and map a piece of the code space to a
      ``FrameStepper`` that can walk through stack frames created in that code
      range. If multiple ``FrameStepper`` objects share the same range, then
      the one with the highest priority will be tried first.

      Returns ``false`` on failure.

  .. cpp:function:: virtual Walker *getWalker() const

      Returns the walker associated with this group.

  .. cpp:function:: virtual void registerStepper(FrameStepper *stepper)

      Adds ``stepper`` to this group to be used over the entire address space.

  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *libaddr, lib_change_t change)

      Called by the StackwalkerAPI when a new library is loaded.

  .. cpp:function:: void getSteppers(std::set<FrameStepper *> &steppers)

      Returns in ``steppers`` all frame steppers in this group.


.. cpp:class:: AddrRangeGroup : public StepperGroup

  .. cpp:member:: protected AddrRangeGroupImpl *impl
  .. cpp:function:: AddrRangeGroup(Walker *new_walker)
  .. cpp:function:: virtual bool addStepper(FrameStepper *stepper, Address start, Address end)
  .. cpp:function:: virtual bool findStepperForAddr(Dyninst::Address addr, FrameStepper* &out, const FrameStepper *last_tried = NULL)

