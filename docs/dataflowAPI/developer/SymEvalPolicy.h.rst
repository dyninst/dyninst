SymEvalPolicy.h
###############

.. cpp:namespace:: Dyninst::dev::DataflowAPI

The ROSE symbolic evaluation engine wants a data type that
is template parametrized on the number of bits in the data
type. However, our ASTs don't have this, and a shared_ptr
to an AST _definitely_ doesn't have it. Instead, we use
a wrapper class (Handle) that is parametrized appropriately
and contains a shared pointer.

This uses a pointer to a shared pointer. This is ordinarily a really
bad idea, but stripping the pointer part makes the compiler allocate
all available memory and crash. No idea why.

.. cpp:struct:: template <size_t Len> Handle

  .. cpp:member:: AST::Ptr *v_

.. cpp:class:: SymEvalPolicy

  A "Policy" class specification for interfacing with the ROSE
  instruction semantics engine.

  Background:

  The ROSE compiler suite provides a description of x86 instruction
  semantics. This operates in terms of a "policy" that states how each
  ROSE operation should map to your particular use. This policy class
  builds an AST representation of the instruction.

  The instruction semantics are initialized with a copy of this class as
  a template parameter:

.. code:: cpp

  AST_Policy policy;
  X86InstructionSemantics semantics<policy>;

The engine also takes a datatype template parameter which itself is
parameterized on the bitsize of the data (e.g., 1, 16, 32, ...), so the
more proper version is:

.. code:: cpp

  X86InstructionSemantics semantics<policy, policy::Datatype>;

where ``Datatype`` must take an ``int`` template parameter.
