.. _sec:dyninstdomain:

The Dyninst Domain
==================

The domain has quite a few useful values and functions:

.. table:: Dyninst Domain Values

   ========== ==== ========================
   =======================================================================
   Identifier Type Where Valid              Description
   ========== ==== ========================
   =======================================================================
   \               Within a function        Evaluates to the name of the current function. Call to must specify a .
   \               Anywhere                 Evaluates to the name of the current module. Call to must specify a .
   \          int  At a memory operation    Evaluates to the number of bytes accessed by a memory operation.
   \               At a memory operation    Evaluates the effective address of a memory operation.
   \               Anywhere                 Evaluates to the original address where the snippet was inserted.
   \               Anywhere                 Evaluates to the actual address of the instrumentation.
   \               Function exit            Evaluates to the return value of a function.
   \          int  Anywhere                 Returns the index of the thread the snippet is executing on.
   \          int  Anywhere                 Returns the id of the thread the snippet is executing on.
   \               At calls, jumps, returns Calculates the target of a control flow instruction.
   \          void Anywhere                 Causes the mutatee to execute a breakpoint.
   \          void Anywhere                 Stops the thread on which the snippet is executing.
   ========== ==== ========================
   =======================================================================
