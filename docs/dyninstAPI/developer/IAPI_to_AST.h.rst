.. _`sec:IAPI_to_AST.h`:

IAPI_to_AST.h
#############


.. cpp:class:: ASTFactory : public Dyninst::InstructionAPI::Visitor

  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::BinaryFunction* b)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::Dereference* d)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::Immediate* i)
  .. cpp:function:: virtual void visit(Dyninst::InstructionAPI::RegisterAST* r)
  .. cpp:member:: std::deque<AstNodePtr> m_stack
  .. cpp:function:: virtual ~ASTFactory()
  .. cpp:function:: ASTFactory()
