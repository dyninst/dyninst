.. _`sec-dev:BPatch_frame.h`:

BPatch_frame.h
##############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_frame

  .. cpp:member:: private bool isSynthFrame

    BPatch defines that a trampoline is effectively a "function call" and puts an extra tramp on the stack.
    Various people (frex, Paradyn) really don't want to see this frame. To make life simpler for everyone,
    we add a "only call if you know what you're doing" flag.

  .. cpp:member:: private BPatch_point *point_

    This is _so_ much easier than looking it up later. If we're in instrumentation, stash the point

  .. cpp:function:: BPatch_function * findFunction()

    When we generate a BPatch_frame, we store the return address for the call as the PC. When we try to
    get the function associated with the frame, we use the PC to identify the function (via
    findFunctionByEntry). However, if the function made a non-returning call, the corresponding return
    address is never parsed, since it is unreachable code. This means that findFunctionByEntry will
    return NULL.

    To compensate, we'll instead look up the function by the address of the callsite
    itself, which is the instruction prior to the return address. Because we do not know the length of
    the call instruction, we'll simply use PC-1, which is within the call instruction.
