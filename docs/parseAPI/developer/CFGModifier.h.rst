.. _`sec:CFGModifier.h`:

CFGModifier.h
#############

.. cpp:namespace:: Dyninst::ParseAPI

A collection of methods for user-triggered modification of a ParseAPI CFG.
Implemented here so that we can make it a friend class of the CFG it also
needs internal information about CFG objects.

.. cpp:class:: CFGModifier
   
  These are all static methods as this class has no state so really, it's just a namespace.

  .. cpp:function:: static bool redirect(Edge *edge, Block *target)

      Redirect the target of an existing edge.

      If target is NULL, user is requesting a redirect to the sink block (create edge only if source block
      doesn't have a sink edge of the same type already).

  .. cpp:function:: static Block *split(Block *, Address, bool trust = false, Address newlast = -1)

      Split a block at a provided point. we double-check whether the address
      is a valid instruction boundary unless trust is true.
      Newlast is the new "last insn" of the original block provide it if
      you don't want to waste time disassembling to figure it out.

  .. cpp:function:: static InsertedRegion *insert(CodeObject *obj, Address base, void *data, unsigned size)

      Parse and add a new region of code to a CodeObject
      The void * becomes "owned" by the CodeObject, as it's used
      as a backing store it cannot be ephemeral.
      Returns the new entry block.

  .. cpp:function:: static bool remove(std::vector<Block *> &, bool force = false)

      Remove blocks from the CFG the block must be unreachable
      (that is, have no in-edges) unless force is true.

  .. cpp:function:: static bool remove(Function *)

      As the above, but for functions.

  .. cpp:function:: static Function *makeEntry(Block *)

      Label a block as the entry of a new function. If the block is already an
      entry that function is returned otherwise we create a new function and
      return it.

.. cpp:class:: InsertedRegion : public CodeRegion

  .. cpp:function:: InsertedRegion(Address base, void *data, unsigned size, Architecture arch)

  .. cpp:function:: Address low() const
  .. cpp:function:: Address high() const

      Addresses are provided by the user, as Dyninst etc. have well-known ways of allocating
      additional code by extending the binary or allocating memory, etc.

  .. cpp:function:: bool isValidAddress(const Address a) const
  .. cpp:function:: void* getPtrToInstruction(const Address a) const
  .. cpp:function:: void* getPtrToData(const Address) const
  .. cpp:function:: bool isCode(const Address a) const
  .. cpp:function:: bool isData(const Address) const
  .. cpp:function:: bool isReadOnly(const Address) const
  .. cpp:function:: Address offset() const
  .. cpp:function:: Address length() const
  .. cpp:function:: unsigned int getAddressWidth() const
  .. cpp:function:: Architecture getArch() const
  .. cpp:function:: bool wasUserAdded() const
