.. _`sec:parseapi-ParseCallback.h.rst`:

ParseCallback.h
===============

.. cpp:namespace:: Dyninst::parseAPI

Class ParseCallback
-------------------

**Defined in:** ``ParseCallback.h``

The ParseCallback class allows ParseAPI users to be notified of various
events during parsing. For most users this notification is unnecessary,
and an instantiation of the default ParseCallback can be passed to the
CodeObject during initialization. Users who wish to be notified must
implement a class that inherits from ParseCallback, and implement one or
more of the methods described below to receive notification of those
events.

.. code-block:: cpp
    
    struct default_details default_details(unsigned char * b,size_t s, bool ib); unsigned char * ibuf; size_t isize; bool isbranch;

Details used in the ``unresolved_cf`` and ``abruptEnd_cf`` callbacks.

.. code-block:: cpp
    
    virtual void instruction_cb(Function *, Block *, Address, insn_details*)

Invoked for each instruction decoded during parsing. Implementing this
callback may incur significant overhead.

.. code-block:: cpp
    
    struct insn_details InsnAdapter::InstructionAdapter * insn;
    void interproc_cf(Function *, Address, interproc_details *)

Invoked for each interprocedural control flow instruction.

.. code-block:: cpp
    
    struct interproc_details typedef enum ret, call, branch_interproc, //
    tail calls, branches to plts syscall type_t; unsigned char * ibuf;
    size_t isize; type_t type; union struct Address target; bool
    absolute_address; bool dynamic_call; call; data;

Details used in the ``interproc_cf`` callback.

.. code-block:: cpp
    
    void overlapping_blocks(Block *, Block *)

Noification of inconsistent parse data (overlapping blocks).