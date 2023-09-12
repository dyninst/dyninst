.. _`sec:DynAST.h`:

DynAST.h
########

.. cpp:namespace:: Dyninst

.. cpp:class:: AST

   We provide a generic AST framework to represent tree structures. One
   example use case is to represent instruction semantics with symbolic
   expressions. The AST framework includes the base class definitions for
   tree nodes and visitors. Users can inherit tree node classes to create
   their own AST structure and AST visitors to write their own analyses for
   the AST.

   All AST node classes should be derived from the AST class. Currently we
   have the following types of AST nodes.

   .. table::

     ============= ======================
     AST::ID       Meaning
     ============= ======================
     V_AST         Base class type
     V_BottomAST   Bottom AST node
     V_ConstantAST Constant AST node
     V_VariableAST Variable AST node
     V_RoseAST     ROSEOperation AST node
     V_StackAST    Stack AST node
     ============= ======================

   .. cpp:type:: boost::shared_ptr<AST> Ptr;

     Shared pointer for class AST.

   .. cpp:type:: std::vector<AST::Ptr> Children;

     The container type for the children of this AST.

   .. cpp:function:: bool operator==(const AST &rhs) const
   .. cpp:function:: bool equals(AST::Ptr rhs)

     Check whether two AST nodes are equal. Return ``true`` when two nodes
     are in the same type and are equal according to the ``==`` operator of
     that type.

   .. cpp:function:: virtual unsigned numChildren() const

     Return the number of children of this node.

   .. cpp:function:: virtual AST::Ptr child(unsigned i) const

     Return the ``i``\ th child.

   .. cpp:function:: virtual const std::string format() const = 0;

     Return the string representation of the node.

   .. cpp:function:: static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b)

     Substitute every occurrence of ``a`` with ``b`` in AST ``in``. Return a
     new AST after the substitution.

   .. cpp:function:: virtual AST::ID AST::getID() const

     Return the class type ID of this node.

   .. cpp:function:: virtual Ptr accept(ASTVisitor *v)

     Apply visitor ``v`` to this node. Note that this method will not
     automatically apply the visitor to its children.

   .. cpp:function:: virtual void AST::setChild(int i, AST::Ptr c)

     Set the ``i``\ th child of this node to ``c``.

.. cpp:class:: ASTVisitor

  An AST for each AST node type. Users can inherit from this class to
  write customized analyses for ASTs.

  .. cpp:type:: boost::shared_ptr<AST> ASTVisitor::ASTPtr
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(AST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::BottomAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::ConstantAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::VariableAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(DataflowAPI::RoseAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr ASTVisitor::visit(StackAST *)

    Callback functions for visiting each type of AST node. The default
    behavior is to return the input parameter.
