InstructionAST.h
================

.. cpp:namespace:: Dyninst::instructionAPI

InstructionAST Class
--------------------

The InstructionAST class is the base class for all nodes in the ASTs
used by the Operand class. It defines the necessary interfaces for
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

.. code-block:: cpp

    typedef boost::shared_ptr<InstructionAST> Ptr

A type definition for a reference-counted pointer to an
``InstructionAST``.

.. code-block:: cpp
 
    bool operator==(const InstructionAST &rhs) const

Compare two AST nodes for equality.

Non-leaf nodes are equal if they are of the same type and their children
are equal. ``RegisterAST``\ s are equal if they represent the same
register. ``Immediate``\ s are equal if they represent the same value.
Note that it is not safe to compare two ``InstructionAST::Ptr``
variables, as those are pointers. Instead, test the underlying
``InstructionAST`` objects.

.. code-block:: cpp

    virtual void getChildren(vector<InstructionAPI::Ptr> & children) const

Children of this node are appended to the vector ``children``.

.. code-block:: cpp

    virtual void getUses(set<InstructionAPI::Ptr> & uses)

The use set of an ``InstructionAST`` is defined as follows:

-  A ``RegisterAST`` uses itself

-  A ``BinaryFunction`` uses the use sets of its children

-  A ``Immediate`` uses nothing

-  A ``Dereference uses the use set of its child``

The use set oft his node is appended to the vector ``uses``.

.. code-block:: cpp

    virtual bool isUsed(InstructionAPI::Ptr findMe) const

Unlike ``getUses``, ``isUsed`` looks for ``findMe`` as a subtree of the
current tree. ``getUses`` is designed to return a minimal set of
registers used in this tree, whereas ``isUsed`` is designed to allow
searches for arbitrary subexpressions. ``findMe`` is the AST node to
find in the use set of this node.

Returns ``true`` if ``findMe`` is used by this AST node.

.. code-block:: cpp

    virtual std::string format(formatStyle how == defaultStyle) const

The ``format`` interface returns the contents of an ``InstructionAPI``
object as a string. By default, ``format`` produces assembly language.