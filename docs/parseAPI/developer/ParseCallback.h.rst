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

    Allow examination of every instruction processed during parsing.

  .. cpp:function:: protected virtual void overlapping_blocks(Block*, Block*)

    Notify about inconsistent parse data (overlapping blocks).

    The blocks are *not* guaranteed to be finished parsing at the time the callback is fired.

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

      tail calls, branches to plts

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


.. cpp:class:: ParseCallbackManager

  .. cpp:function:: ParseCallbackManager(ParseCallback *b)
  .. cpp:function:: virtual ~ParseCallbackManager()
  .. cpp:type:: std::list<ParseCallback *> Callbacks
  .. cpp:type:: Callbacks::iterator iterator
  .. cpp:type:: Callbacks::const_iterator const_iterator
  .. cpp:function:: void registerCallback(ParseCallback *a)
  .. cpp:function:: void unregisterCallback(ParseCallback *a)
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: void batch_begin()
  .. cpp:function:: void batch_end(CFGFactory *fact)

    fact provided so we can safely delete

  .. cpp:function:: void destroy(Block *, CFGFactory *fact)
  .. cpp:function:: void destroy(Edge *, CFGFactory *fact)
  .. cpp:function:: void destroy(Function *, CFGFactory *fact)
  .. cpp:function:: void removeEdge(Block *, Edge *, ParseCallback::edge_type_t)
  .. cpp:function:: void addEdge(Block *, Edge *, ParseCallback::edge_type_t)
  .. cpp:function:: void removeBlock(Function *, Block *)
  .. cpp:function:: void addBlock(Function *, Block *)
  .. cpp:function:: void splitBlock(Block *, Block *)
  .. cpp:function:: void modifyEdge(Edge *, Block *, ParseCallback::edge_type_t)
  .. cpp:function:: void interproc_cf(Function*,Block *,Address,ParseCallback::interproc_details*)
  .. cpp:function:: void instruction_cb(Function*,Block *,Address,ParseCallback::insn_details*)
  .. cpp:function:: void overlapping_blocks(Block*,Block*)
  .. cpp:function:: void newfunction_retstatus(Function*)
  .. cpp:function:: void patch_nop_jump(Address)
  .. cpp:function:: bool updateCodeBytes(Address)
  .. cpp:function:: void abruptEnd_cf(Address, Block *,ParseCallback::default_details*)
  .. cpp:function:: bool absAddr(Address absolute, Address& loadAddr, CodeObject*& containerObject)
  .. cpp:function:: bool hasWeirdInsns(const Function*)
  .. cpp:function:: void foundWeirdInsns(Function*)
  .. cpp:function:: void split_block_cb(Block *, Block *)
  .. cpp:function:: void discover_function(Function*)
  .. cpp:function:: private void destroy_cb(Block *)
  .. cpp:function:: private void destroy_cb(Edge *)
  .. cpp:function:: private void destroy_cb(Function *)
  .. cpp:function:: private void remove_edge_cb(Block *, Edge *, ParseCallback::edge_type_t)
  .. cpp:function:: private void add_edge_cb(Block *, Edge *, ParseCallback::edge_type_t)
  .. cpp:function:: private void remove_block_cb(Function *, Block *)
  .. cpp:function:: private void add_block_cb(Function *, Block *)
  .. cpp:function:: private void modify_edge_cb(Edge *, Block *, ParseCallback::edge_type_t)
  .. cpp:member:: private Callbacks cbs_
  .. cpp:member:: private bool inBatch_
  .. cpp:type:: private std::pair<Block *, Block *> BlockSplit
  .. cpp:member:: private std::vector<Edge *> destroyedEdges_
  .. cpp:member:: private std::vector<Block *> destroyedBlocks_
  .. cpp:member:: private std::vector<Function *> destroyedFunctions_
  .. cpp:member:: private std::vector<BlockMod> blockMods_
  .. cpp:member:: private std::vector<EdgeMod> edgeMods_
  .. cpp:member:: private std::vector<FuncMod> funcMods_
  .. cpp:member:: private std::vector<BlockSplit> blockSplits_


.. cpp:enum:: ParseCallbackManager::mod_t

  .. cpp:enumerator:: removed
  .. cpp:enumerator:: added


.. cpp:struct:: ParseCallbackManager::BlockMod

  .. cpp:member:: private Block *block
  .. cpp:member:: private Edge *edge
  .. cpp:member:: private ParseCallback::edge_type_t type
  .. cpp:member:: private mod_t action
  .. cpp:function:: private BlockMod(Block *b, Edge *e, ParseCallback::edge_type_t t, mod_t m)


.. cpp:struct:: ParseCallbackManager::EdgeMod

  .. cpp:member:: private Edge *edge
  .. cpp:member:: private Block *block
  .. cpp:member:: private ParseCallback::edge_type_t action
  .. cpp:function:: private EdgeMod(Edge *e, Block *b, ParseCallback::edge_type_t t)


.. cpp:struct:: ParseCallbackManager::FuncMod

  .. cpp:member:: private Function *func
  .. cpp:member:: private Block *block
  .. cpp:member:: private mod_t action
  .. cpp:function:: private FuncMod(Function *f, Block *b, mod_t m)



Notes
=====

Defensive-mode notifications:

  - Notify when a function's parse is finalized so Dyninst can save its initial return status
  - Notify every time a block is split, after the initial parse of the function
  - Notify of the x86 obfuscation that performs a short jmp -1 (eb ff) so that dyninst can patch
    the opcode with a nop (0x90), which will keep code generation from doing bad things
