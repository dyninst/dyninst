IndirectASTVisitor.h
====================

.. cpp:namespace:: Dyninst::parseAPI

.. code:: c

  #define SIGNEX_64_32 0xffffffff00000000LL
  #define SIGNEX_64_16 0xffffffffffff0000LL
  #define SIGNEX_64_8  0xffffffffffffff00LL
  #define SIGNEX_32_16 0xffff0000
  #define SIGNEX_32_8 0xffffff00

.. cpp:class:: SimplifyVisitor: public ASTVisitor

  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST* ast)
  .. cpp:function:: SimplifyVisitor(Address a, bool k, SymbolicExpression& sym)

.. cpp:class:: BoundCalcVisitor: public ASTVisitor

  .. cpp:member:: map<AST*, StridedInterval*> bound
  .. cpp:member:: BoundFact &boundFact
  .. cpp:member:: ParseAPI::Block *block
  .. cpp:member:: bool handleOneByteRead
  .. cpp:member:: int derefSize

  .. cpp:function:: BoundCalcVisitor(BoundFact &bf, ParseAPI::Block* b, bool handle, int size)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST *ast)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::ConstantAST *ast)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::VariableAST *ast)
  .. cpp:function:: bool IsResultBounded(AST::Ptr ast)
  .. cpp:function:: StridedInterval* GetResultBound(AST::Ptr ast)

.. cpp:class:: JumpCondVisitor: public ASTVisitor

  .. cpp:member:: bool invertFlag

  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST *ast)
  .. cpp:function:: JumpCondVisitor()

.. cpp:class:: ComparisonVisitor: public ASTVisitor

  .. cpp:member:: AST::Ptr subtrahend
  .. cpp:member:: AST::Ptr minuend

  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST *ast)
  .. cpp:function:: ComparisonVisitor()

.. cpp:class:: JumpTableFormatVisitor: public ASTVisitor
  
  .. cpp:member:: AbsRegion index
  .. cpp:member:: int numOfVar
  .. cpp:member:: int memoryReadLayer
  .. cpp:member:: ParseAPI::Block *b
  .. cpp:member:: bool findIncorrectFormat
  .. cpp:member:: bool findTableBase
  .. cpp:member:: bool findIndex
  .. cpp:member:: bool firstAdd

  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST *ast)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::VariableAST *ast)
  .. cpp:function:: JumpTableFormatVisitor(ParseAPI::Block *bl)

.. cpp:class:: JumpTableReadVisitor: public ASTVisitor

  .. cpp:member:: AbsRegion index
  .. cpp:member:: int64_t indexValue
  .. cpp:member:: CodeSource* cs
  .. cpp:member:: CodeRegion* cr
  .. cpp:member:: Address targetAddress
  .. cpp:member:: Address readAddress
  .. cpp:member:: int memoryReadSize
  .. cpp:member:: bool valid
  .. cpp:member:: bool isZeroExtend
  .. cpp:member:: map<AST*, int64_t> results

  .. cpp:function:: JumpTableReadVisitor(AbsRegion i, int v, CodeSource *c, CodeRegion *r, bool ze, int m)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::RoseAST *ast)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::ConstantAST *ast)
  .. cpp:function:: virtual ASTPtr visit(DataflowAPI::VariableAST *ast)
  .. cpp:function:: bool PerformMemoryRead(Address addr, int64_t &v)
