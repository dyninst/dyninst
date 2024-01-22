local_var.h
===========

.. cpp:namespace:: Dyninst::stackwalk

**Defined in:** ``local_var.h``

StackwalkerAPI can be used to access local variables found in the frames
of a call stack. The StackwalkerAPI interface for accessing the values
of local variables is closely tied to the SymtabAPI interface for
collecting information about local variables–SymtabAPI handles for
functions, local variables, and types are part of this interface.

Given an initial handle to a SymtabAPI Function object, SymtabAPI can
look up local variables contained in that function and the types of
those local variables. See the SymtabAPI Programmer’s Guide for more
information.

.. code-block:: cpp

    static Dyninst::SymtabAPI::Function *getFunctionForFrame(Frame f)

This method returns a SymtabAPI function handle for the function that
created the call stack frame, f.

.. code-block:: cpp

    static int glvv_Success = 0; static int glvv_EParam = -1; static int
    glvv_EOutOfScope = -2; static int glvv_EBufferSize = -3; static int
    glvv_EUnknown = -4;

    static int getLocalVariableValue(Dyninst::SymtabAPI::localVar *var,
    std::vector<Frame> &swalk, unsigned frame, void *out_buffer, unsigned out_buffer_size)

Given a local variable and a stack frame from a call stack, this
function returns the value of the variable in that frame. The local
variable is specified by the SymtabAPI variable object, ``var``.
``swalk`` is a call stack that was collected via StackwalkerAPI, and
``frame`` specifies an index into that call stack that contains the
local variable. The value of the variable is stored in ``out_buffer``
and the size of ``out_buffer`` should be specified in
``out_buffer_size``.

A local variable only has a limited scope with-in a target process’
execution. StackwalkerAPI cannot guarantee that it can collect the
correct return value of a local variable from a call stack if the target
process is continued after the call stack is collected.

Finding and collecting the values of local variables is dependent on
debugging information being present in a target process’ binary. Not all
binaries contain debugging information, and in some cases, such as for
binaries built with high compiler optimization levels, that debugging
information may be incorrect.

``getLocalVariableValue`` will return on of the following values:

glvv_Success
   getLocalVariableValue was able to correctly read the value of the
   given variable.

glvv_EParam
   An error occurred, an incorrect parameter was specified (frame was
   larger than ``swalk.size()``, or var was not a variable in the
   function specified by frame).

glvv_EOutOfScope
   An error occurred, the specified variable exists in the function but
   isn’t live at the current execution point.

glvv_EBufferSize
   An error occurred, the variable’s value does not fit inside
   ``out_buffer``.

glvv_EUnknown
   An unknown error occurred. It is most likely that the local variable
   was optimized away or debugging information about the variable was
   incorrect.