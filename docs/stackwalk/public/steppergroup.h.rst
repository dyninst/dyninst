steppergroup.h
==============

.. cpp:namespace:: Dyninst::stackwalk

Class StepperGroup
~~~~~~~~~~~~~~~~~~

**Defined in:** ``steppergroup.h``

The ``StepperGroup`` class contains a collection of ``FrameStepper``
objects. The ``StepperGroup``\ ’s primary job is to decide which
``FrameStepper`` should be used to walk through a stack frame given a
return address. The default ``StepperGroup`` keeps a set of address
ranges for each ``FrameStepper``. If multiple ``FrameStepper`` objects
overlap an address, then the default ``StepperGroup`` will use a
priority system to decide.

``StepperGroup`` provides both an interface and a default implementation
of that interface. Users who want to customize the ``StepperGroup``
should inherit from this class and re-implement any of the below virtual
functions.

.. code-block:: cpp

    StepperGroup(Walker *walker)

This factory constructor creates a new ``StepperGroup`` object
associated with ``walker``.

.. code-block:: cpp

    virtual bool addStepper(FrameStepper *stepper)

This method adds a new ``FrameStepper`` to this ``StepperGroup``. The
newly added stepper will be tracked by this ``StepperGroup``, and it
will be considered for use when walking through stack frames.

This method returns ``true`` if it successfully added the
``FrameStepper``, and ``false`` on error.

.. code-block:: cpp

    virtual bool addStepper(FrameStepper *stepper, Address start, Address end) = 0;

Add the specified ``FrameStepper`` to the list of known steppers, and
register it to handle frames in the range [``start``, ``end``).

.. code-block:: cpp

    virtual void registerStepper(FrameStepper *stepper);

Add the specified ``FrameStepper`` to the list of known steppers and use
it over the entire address space.

.. code-block:: cpp

    virtual bool findStepperForAddr(Address addr, FrameStepper* &out, const FrameStepper *last_tried = NULL) = 0

Given an address that points into a function (or function-like object),
addr, this method decides which ``FrameStepper`` should be used to walk
through the stack frame created by the function at that address. A
pointer to the ``FrameStepper`` will be returned in parameter ``out``.

It may be possible that the ``FrameStepper`` this method decides on is
unable to walk through the stack frame (it returns ``gcf_not_me`` from
``FrameStepper::getCallerFrame``). In this case StackwalkerAPI will call
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

This method returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    typedef std::pair<std::string, Address> LibAddrPair; typedef enum
    library_load, library_unload lib_change_t; virtual void
    newLibraryNotification(LibAddrPair *libaddr, lib_change_t change);

Called by the StackwalkerAPI when a new library is loaded.

.. code-block:: cpp

    Walker *getWalker() const

This method returns the Walker object that associated with this
StepperGroup.

.. code-block:: cpp

    void getSteppers(std::set<FrameStepper *> &);

Fill in the provided set with all ``FrameSteppers`` registered in the
``StepperGroup``.