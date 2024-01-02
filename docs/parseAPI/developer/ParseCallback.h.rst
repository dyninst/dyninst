.. _`sec:ParseCallback.h`:

ParseCallback.h
###############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: ParseCallback

  **Generic callback used throughout ParseAPI**

  The ParseCallback class allows ParseAPI users to be notified of various
  events during parsing. For most users this notification is unnecessary,
  and an instantiation of the default ParseCallback can be passed to the
  CodeObject during initialization. Users who wish to be notified must
  implement a class that inherits from ParseCallback, and implement one or
  more of the methods described below to receive notification of those
  events.

  .. cpp:function:: protected virtual void interproc_cf(Function*, Block *, Address, interproc_details*)
  .. cpp:function:: protected virtual void instruction_cb(Function*, Block *, Address, insn_details*)
  .. cpp:function:: protected virtual void overlapping_blocks(Block*, Block*)
  .. cpp:function:: protected virtual void newfunction_retstatus(Function*)
  .. cpp:function:: protected virtual void patch_nop_jump(Address)
  .. cpp:function:: protected virtual bool updateCodeBytes(Address)
  .. cpp:function:: protected virtual void abruptEnd_cf(Address, Block*, default_details*)
  .. cpp:function:: protected virtual bool absAddr(Address, Address&, CodeObject*& )

      Returns the load address of the code object containing an absolute address.

  .. cpp:function:: protected virtual bool hasWeirdInsns(const Function*) const
  .. cpp:function:: protected virtual void foundWeirdInsns(Function*)
  .. cpp:function:: protected virtual void split_block_cb(Block*, Block)
  .. cpp:function:: protected virtual void destroy_cb(Block*)
  .. cpp:function:: protected virtual void destroy_cb(Edge*)
  .. cpp:function:: protected virtual void destroy_cb(Function*)
  .. cpp:function:: protected virtual void remove_edge_cb(Block*, Edge*, edge_type_t)
  .. cpp:function:: protected virtual void add_edge_cb(Block*, Edge*, edge_type_t)
  .. cpp:function:: protected virtual void remove_block_cb(Function*, Block*)
  .. cpp:function:: protected virtual void add_block_cb(Function*, Block*)
  .. cpp:function:: protected virtual void modify_edge_cb(Edge*, Block*, edge_type_t)
  .. cpp:function:: protected virtual void function_discovery_cb(Function*)

.. cpp:struct:: ParseCallback::default_details

  **Notify when control transfers have run `off the rails'**

  .. cpp:member:: unsigned char * ibuf
  .. cpp:member:: size_t isize
  .. cpp:member:: bool isbranch

  .. cpp:function:: default_details(unsigned char * b,size_t s, bool ib)

  .. cpp:function:: virtual void instruction_cb(Function *, Block *, Address, insn_details*)

      Invoked for each instruction decoded during parsing. Implementing this
      callback may incur significant overhead.

  .. cpp:function:: void overlapping_blocks(Block *, Block *)

      Notification of inconsistent parse data (overlapping blocks).


.. cpp:struct:: ParseCallback::interproc_details

  **Notify for interprocedural control transfers**

  .. cpp:enum:: type_t

    .. cpp:enumerator:: ret
    .. cpp:enumerator:: call
    .. cpp:enumerator:: branch_interproc
    .. cpp:enumerator:: syscall
    .. cpp:enumerator:: unresolved

  .. cpp:member:: size_t isize
  .. cpp:member:: type_t type
  .. cpp:member:: interproc_details::@ipd_data data

.. cpp:union:: ParseCallback::interproc_details::@ipd_data

  .. cpp:member:: interproc_details::@ipd_call call
  .. cpp:member:: interproc_details::@ipd_unres unres

.. cpp:struct:: ParseCallback::interproc_details::@ipd_call

  .. cpp:member:: Address target
  .. cpp:member:: bool absolute_address
  .. cpp:member:: bool dynamic_call

.. cpp:struct:: ParseCallback::interproc_details::@ipd_unres

  .. cpp:member:: Address target
  .. cpp:member:: bool absolute_address
  .. cpp:member:: bool dynamic

.. cpp:struct:: ParseCallback::insn_details

  **Invoked for each interprocedural control flow instruction**

  .. cpp:member:: InsnAdapter::InstructionAdapter * insn

.. cpp:enum:: ParseCallback::edge_type_t

  .. cpp:enumerator:: source
  .. cpp:enumerator:: target

Notes
=====

Defensive-mode notifications:

  - Notify when a function's parse is finalized so Dyninst can save its initial return status
  - Notify every time a block is split, after the initial parse of the function
  - Notify of the x86 obfuscation that performs a short jmp -1 (eb ff) so that dyninst can patch
    the opcode with a nop (0x90), which will keep code generation from doing bad things
