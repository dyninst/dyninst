framestepper.h
==============

.. cpp:namespace:: Dyninst::stackwalk

FrameSteppers
-------------

StackwalkerAPI ships with numerous default implementations of the
``FrameStepper`` class. Each of these ``FrameStepper`` implementations
allow StackwalkerAPI to walk a type of call frames.
Section `3.6.1 <#subsec:defaults>`__ describes which ``FrameStepper``
implementations are available on which platforms. This sections gives a
brief description of what each ``FrameStepper`` implementation does.
Each of the following classes implements the ``FrameStepper`` interface
described in Section `3.6.2 <#subsec:framestepper>`__, so we do not
repeat the API description for the classes here.

Several of the ``FrameStepper``\ s use helper classes (see
``FrameFuncStepper`` as an example). Users can further customize the
behavior of a ``FrameStepper`` by providing their own implementation of
these helper classes.

Class FrameFuncStepper
~~~~~~~~~~~~~~~~~~~~~~

This class implements stack walking through a call frame that is setup
with the architectures standard stack frame. For example, on x86 this
``FrameStepper`` will be used to walk through stack frames that are
setup with a ``push %ebp/mov %esp,%ebp`` prologue.

Class FrameFuncHelper
~~~~~~~~~~~~~~~~~~~~~

``FrameFuncStepper`` uses a helper class, ``FrameFuncHelper``, to get
information on what kind of stack frame it’s walking through. The
``FrameFuncHelper`` will generally use techniques such as binary
analysis to determine what type of stack frame the ``FrameFuncStepper``
is walking through. Users can have StackwalkerAPI use their own binary
analysis mechanisms by providing an implementation of this
``FrameFuncHelper``.

There are two important types used by ``FrameFuncHelper`` and one
important function: typedef enum unknown_t=0, no_frame, standard_frame,
savefp_only_frame, frame_type;

The ``frame_type`` describes what kind of stack frame a function uses.
If it does not set up a stack frame then ``frame_type`` should be
``no_frame``. If it sets up a standard frame then ``frame_type`` should
be ``standard_frame``. The ``savefp_only_frame`` value currently only
has meaning on the x86 family of systems, and means that a function
saves the old frame pointer, but does not setup a new frame pointer (it
has a ``push %ebp`` instruction, but no ``mov %esp,%ebp``). If the
``FrameFuncHelper`` cannot determine the ``frame_type``, then it should
be assigned the value ``unknown_t``.

.. code-block:: cpp

    typedef enum unknown_s=0, unset_frame, halfset_frame, set_frame frame_state;

The ``frame_state`` type determines the current state of function with a
stack frame at some point of execution. For example, a function may set
up a standard stack frame and have a ``frame_type`` of
``standard_frame``, but execution may be at the first instruction in the
function and the frame is not yet setup, in which case the
``frame_state`` will be ``unset_frame``.

If the function sets up a standard stack frame and the execution point
is someplace where the frame is completely setup, then the
``frame_state`` should be ``set_frame``. If the function sets up a
standard frame and the execution point is at a point where the frame
does not yet exist or has been torn down, then ``frame_state`` should be
``unset_frame``. The ``halfset_frame`` value of ``frame_state`` is
currently only meaningful on the x86 family of architecture, and should
if the function has saved the old frame pointer, but not yet set up a
new frame pointer.

.. code-block:: cpp

    typedef std::pair<frame_type, frame_state> alloc_frame_t; virtual alloc_frame_t allocatesFrame(Address addr) = 0;

The ``allocatesFrame`` function of ``FrameFuncHelper`` returns a
``alloc_frame_t`` that describes the frame_type of the function at
``addr`` and the ``frame_state`` of the function when execution reached
``addr``.

If ``addr`` is invalid or an error occurs, allocatedFrame should return
``alloc_frame_t(unknown_t, unknown_s)``.

Class SigHandlerStepper
~~~~~~~~~~~~~~~~~~~~~~~

The ``SigHandlerStepper`` is used to walk through UNIX signal handlers
as found on the call stack. On some systems a signal handler generates a
special kind of stack frame that cannot be walked through using normal
stack walking techniques.

Class DebugStepper
~~~~~~~~~~~~~~~~~~

This class uses debug information found in a binary to walk through a
stack frame. It depends on SymtabAPI to read debug information from a
binary, then uses that debug information to walk through a call frame.

Most binaries must be built with debug information (``-g`` with ``gcc``)
in order to include debug information that this ``FrameStepper`` uses.
Some languages, such as C++, automatically include stackwalking debug
information for use by exceptions. The ``DebugStepper`` class will also
make use of this kind of exception information if it is available.

Class AnalysisStepper
~~~~~~~~~~~~~~~~~~~~~

This class uses dataflow analysis to determine possible stack sizes at
all locations in a function as well as the location of the frame
pointer. It is able to handle optimized code with omitted frame pointers
and overlapping code sequences.

Class StepperWanderer
~~~~~~~~~~~~~~~~~~~~~

This class uses a heuristic approach to find possible return addresses
in the stack frame. If a return address is found that matches a valid
caller of the current function, we conclude it is the actual return
address and construct a matching stack frame. Since this approach is
heuristic it can make mistakes leading to incorrect stack information.
It has primarily been replaced by the ``AnalysisStepper`` described
above.

Class BottomOfStackStepper
~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``BottomOfStackStepper`` doesn’t actually walk through any type of
call frame. Instead it attempts to detect whether the bottom of the call
stack has been reached. If so, ``BottomOfStackStepper`` will report
``gcf_stackbottom`` from its ``getCallerFrame`` method. Otherwise it
will report ``gcf_not_me``. ``BottomOfStackStepper`` runs with a higher
priority than any other ``FrameStepper`` class.