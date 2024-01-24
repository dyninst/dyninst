.. _`sec:linuxbsd-swk.h`:

linuxbsd-swk.h
##############

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: SigHandlerStepperImpl : public FrameStepper

  .. cpp:function:: SigHandlerStepperImpl(Walker *w, SigHandlerStepper *parent)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change)
  .. cpp:function:: virtual const char *getName() const
  .. cpp:function:: virtual ~SigHandlerStepperImpl()
