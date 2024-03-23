.. _`sec:local_var.h`:

local_var.h
###########

StackwalkerAPI can be used to access local variables found in the frames of a call stack.
The StackwalkerAPI interface for accessing the values of local variables is closely tied
to the SymtabAPI interface for collecting information about local variables - SymtabAPI
handles for functions, local variables, and types are part of this interface.

A local variable only has a limited scope with-in a target process’ execution. StackwalkerAPI
cannot guarantee that it can collect the correct return value of a local variable from a call
stack if the target process is continued after the call stack is collected.

Finding and collecting the values of local variables is dependent on debugging information
being present in a target process’ binary. Not all binaries contain debugging information,
and in some cases, such as for binaries built with high compiler optimization levels, that
debugging information may be incorrect.


.. cpp:function:: static Symtab *getSymtabForName(std::string name)


.. cpp:class:: LVReader : public Dyninst::SymtabAPI::MemRegReader

  .. cpp:function:: LVReader(ProcessState *p, int f, std::vector<Dyninst::Stackwalker::Frame> *s, Dyninst::THR_ID t)
  .. cpp:function:: virtual bool ReadMem(Dyninst::Address addr, void *buffer, unsigned size)
  .. cpp:function:: virtual bool GetReg(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool start()
  .. cpp:function:: virtual bool done()


.. cpp:function:: static Dyninst::SymtabAPI::Function *getFunctionForFrame(Dyninst::Stackwalker::Frame f)

  Returns a :ref:`sec:symtab-intro` function handle for the function that created the
  call stack frame ``f``.


.. cpp:var:: static int glvv_Success = 0

  A value was successfully read from a variable.

.. cpp:var:: static int glvv_EParam = -1

  An error occurred, an incorrect parameter was specified (frame was
  larger than ``swalk.size()``, or var was not a variable in the
  function specified by frame).

.. cpp:var:: static int glvv_EOutOfScope = -2

  An error occurred. The variable exists in the function but isn’t live
  at the current execution point.

.. cpp:var:: static int glvv_EBufferSize = -3

  An error occurred. The variable’s value does not fit in the requested destination.

.. cpp:var:: static int glvv_EUnknown = -4

  An unknown error occurred. It is most likely that the local variable
  was optimized away or debugging information about the variable was
  incorrect.


.. cpp:function:: static int getLocalVariableValue(Dyninst::SymtabAPI::localVar *var, std::vector<Dyninst::Stackwalker::Frame> &swalk, \
                                                   unsigned frame, void *out_buffer, unsigned out_buffer_size)

  Given a local variable and a stack frame from a call stack, returns the value of the variable in that frame.

  The local variable is specified by ``var``. ``swalk`` is a call stack that was collected via StackwalkerAPI, and
  ``frame`` specifies an index into that call stack that contains the local variable. The value of the variable is
  stored in ``out_buffer`` and the size of ``out_buffer`` should be specified in ``out_buffer_size``.
