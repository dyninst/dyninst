SymEvalVisitors.h
#################

.. cpp:namespace:: Dyninst::dev::DataflowAPI

A collection of visitors for SymEval AST classes.

.. cpp:class:: StackVisitor : public ASTVisitor

  For evaluating stack variables.

.. cpp:class:: BooleanVisitor : public ASTVisitor

  Simplify boolean expressions for PPC