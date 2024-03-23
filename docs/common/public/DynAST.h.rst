.. _`sec:DynAST.h`:

DynAST.h
########

.. cpp:namespace:: Dyninst

.. cpp:class:: AST

  **A generic framework for representing Abstract Syntax Trees (AST)**

  One example use case is to represent instruction semantics with symbolic
  expressions. The AST framework includes the base class definitions for
  tree nodes and visitors.

  Users can inherit tree node classes to create
  their own AST structure and AST visitors to write their own analyses for
  the AST. All AST node classes should be derived from the AST class.

  .. cpp:enum:: ID

    .. cpp:enumerator:: V_AST
    .. cpp:enumerator:: V_BottomAST
    .. cpp:enumerator:: V_ConstantAST
    .. cpp:enumerator:: V_VariableAST
    .. cpp:enumerator:: V_RoseAST
    .. cpp:enumerator:: V_StackAST
    .. cpp:enumerator:: V_InputVariableAST
    .. cpp:enumerator:: V_ReferenceAST
    .. cpp:enumerator:: V_StpAST
    .. cpp:enumerator:: V_YicesAST
    .. cpp:enumerator:: V_SemanticsAST

  .. cpp:type:: boost::shared_ptr<AST> Ptr

    Shared pointer for class AST.

  .. cpp:type:: std::vector<Ptr> Children

    The container type for the children of this AST.

  .. cpp:function:: bool operator==(const AST &rhs) const
  .. cpp:function:: bool equals(Ptr rhs)

    Checks if two AST nodes are equal.

    Return ``true`` when two nodes
    are in the same type and are equal according to the ``==`` operator of
    that type.

  .. cpp:function:: virtual unsigned numChildren() const

    Returns the number of children of this node.

  .. cpp:function:: virtual Ptr child(unsigned i) const

    Returns the ``i``\ th child.

  .. cpp:function:: virtual const std::string format() const = 0

    Returns the string representation of the node.

  .. cpp:function:: static void hasCycle(Ptr in,std::map<Ptr, int> &visited)

    Detects if ``in`` exists in the set of ``visited`` nodes.

    This works recursively to detect visitation of ``in`` multiple times which would
    signal the presence of a cycle in the tree.

  .. cpp:function:: static Ptr substitute(Ptr in, Ptr a, Ptr b)

    Substitutes every occurrence of ``a`` with ``b`` in AST ``in``.

    Returns a new AST after the substitution.

  .. cpp:function:: virtual ID getID() const

    Returns the class type ID of this node.

  .. cpp:function:: virtual Ptr accept(ASTVisitor *v)

    Applies visitor ``v`` to this node.

    .. Note:: This method will not automatically apply the visitor to its children.

  .. cpp:function:: Ptr ptr()

    Returns the current instance as a ``Ptr``.

  .. cpp:function:: virtual void setChild(int i, Ptr c)

    Sets the ``i``\ th child of this node to ``c``.

.. cpp:class:: ASTVisitor

  **An AST visitor for each AST node type**

  Users can inherit from this class to write customized analyses for ASTs.

  .. cpp:type:: boost::shared_ptr<AST> ASTVisitor::ASTPtr
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(AST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::BottomAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::ConstantAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::VariableAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::RoseAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(StackAST *)

 Callback functions for visiting each type of AST node. The default
 behavior is to return the input parameter.
