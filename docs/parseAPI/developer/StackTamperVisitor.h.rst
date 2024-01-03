.. _`sec:StackTamperVisitor.h`:

StackTamperVisitor.h
####################

.. cpp:namespace:: Dyninst

.. cpp:class:: StackTamperVisitor : public ASTVisitor

  .. cpp:function:: StackTamperVisitor(const Absloc&)
  .. cpp:function:: virtual AST::Ptr visit(AST*)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::BottomAST* )
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::ConstantAST* )
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::VariableAST* )
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::RoseAST* )
  .. cpp:function:: virtual AST::Ptr visit(StackAST*)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(InputVariableAST* x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(ReferenceAST* x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(StpAST* x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(YicesAST* x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(SemanticsAST* x)
  .. cpp:function:: ParseAPI::StackTamper tampersStack(AST::Ptr a, Address& modAddr)

.. cpp:enum:: Priority

  .. cpp:enumerator:: NotRequired
  .. cpp:enumerator:: Suggested
  .. cpp:enumerator:: Required
  .. cpp:enumerator:: OffLimits
  .. cpp:enumerator:: MaxPriority

.. cpp:struct:: template <typename T>  linVar

  .. cpp:function:: linVar<T> &operator+=(const linVar<T> &rhs)
  .. cpp:function:: const linVar<T> operator+(const linVar<T> &rhs) const
  .. cpp:function:: const linVar<T> operator*=(const int &rhs)
  .. cpp:function:: const linVar<T> operator*=(const linVar<T> &rhs)
  .. cpp:function:: const linVar<T> operator*(const linVar<T> &rhs) const
  .. cpp:function:: linVar()
  .. cpp:function:: linVar(T x, T y)
  .. cpp:function:: linVar(int x, int y)
  .. cpp:function:: linVar(T x, int y)
  .. cpp:function:: linVar(Var<T> x, Var<T> y)

.. cpp:struct:: template <typename T> Var

  A representation of a variable x = x + var1 + var2 + var3 + ...
  where int is an integer and var1...varN are unknown variables.

  .. cpp:type:: typename std::map<T, int> Unknowns

  .. cpp:member:: int x
  .. cpp:member:: Unknowns unknowns

  .. cpp:function:: Var<T> &operator+=(const Var<T> &rhs)
  .. cpp:function:: Var<T> &operator+=(const int &rhs)
  .. cpp:function:: Var<T> operator+(const Var<T> &rhs) const
  .. cpp:function:: Var<T> operator+(const int &rhs) const
  .. cpp:function:: Var<T> operator*=(const int &rhs)
  .. cpp:function:: Var<T> operator*(const int &rhs) const
  .. cpp:function:: bool operator==(const Var<T> &rhs) const
  .. cpp:function:: bool operator!=(const Var<T> &rhs) const
  .. cpp:function:: bool operator!=(const int &rhs) const
  .. cpp:function:: Var()
  .. cpp:function:: Var(int a)
  .. cpp:function:: Var(T a)
