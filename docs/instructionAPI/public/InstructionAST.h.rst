.. _`sec:InstructionAST.h`:

InstructionAST.h
################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:enum:: formatStyle

  .. cpp:enumerator:: defaultStyle
  .. cpp:enumerator:: memoryAccessStyle

.. cpp:class:: InstructionAST : public boost::enable_shared_from_this<InstructionAST>

  **The base class for all nodes in the ASTs used by the Operand class**

  .. cpp:type:: boost::shared_ptr<InstructionAST> Ptr

      A reference-counted pointer to an ``InstructionAST``.

  .. cpp:function:: bool operator==(const InstructionAST &rhs) const

      Checks if this :cpp:class::`AST` node is equal to ``rhs``.

      Non-leaf nodes are equal if they are of the same type and their children are equal.

  .. cpp:function:: virtual void getChildren(vector<InstructionAST::Ptr> & children) const

      Children of this node are appended to the vector ``children``.

  .. cpp:function:: virtual void getUses(set<InstructionAST::Ptr> & uses)

      Appends the set of expressions used.

      See :ref:`sec:instructionast-use-sets` for details.

  .. cpp:function:: virtual bool isUsed(InstructionAST::Ptr i) const

      Checks if ``i`` is used by this node.

      Unlike :cpp:func:`getUses`, this looks for ``i`` as a subtree of the
      current tree. ``getUses`` is designed to return a minimal set of
      registers used in this tree, whereas ``isUsed`` is designed to allow
      searches for arbitrary subexpressions.

      See :ref:`sec:instructionast-use-sets` for details.

  .. cpp:function:: virtual std::string format(formatStyle how = defaultStyle) const

      Returns a string representation of the :cpp:class:`AST` node.

      Produces assembly language by default.

  virtual std::string format(Architecture arch, formatStyle how = defaultStyle) const = 0

  .. cpp:function:: protected virtual bool isStrictEqual(const InstructionAST& rhs) const= 0
  .. cpp:function:: protected virtual bool checkRegID(MachRegister, unsigned int = 0, unsigned int = 0) const
  .. cpp:function:: protected virtual const Result& eval() const = 0

.. _`sec:instructionast-notes`:

Notes
=====

InstructionAST defines the necessary interfaces for
traversing and searching an abstract syntax tree representing an
operand. For the purposes of searching an InstructionAST, we provide two
related interfaces. The first, ``getUses``, will return the registers
that appear in a given tree. The second, ``isUsed``, will take as input
another tree and return true if that tree is a (not necessarily proper)
subtree of this one. ``isUsed`` requires us to define an equality
relation on these abstract syntax trees, and the equality operator is
provided by the InstructionAST, with the details implemented by the
classes derived from InstructionAST. Two AST nodes are equal if the
following conditions hold:

  -  They are of the same type

  -  If leaf nodes, they represent the same immediate value or the same
     register

  -  If non-leaf nodes, they represent the same operation and their
     corresponding children are equal

.. _`sec:instructionast-use-sets`:

Use Sets
^^^^^^^^

The use set of an ``InstructionAST`` is defined as follows:

  -  A ``RegisterAST`` uses itself

  -  A ``BinaryFunction`` uses the use sets of its children

  -  A ``Immediate`` uses nothing

  -  A ``Dereference`` uses the use set of its child
