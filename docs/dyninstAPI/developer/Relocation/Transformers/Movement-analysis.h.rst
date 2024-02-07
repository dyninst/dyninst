.. _`sec:Movement-analysis.h`:

Movement-analysis.h
###################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: ExtPCSensVisitor : public ASTVisitor

  .. cpp:function:: ExtPCSensVisitor(const AbsRegion &a)
  .. cpp:function:: virtual AST::Ptr visit(AST *)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::BottomAST *)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::ConstantAST *)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::VariableAST *)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::RoseAST *)
  .. cpp:function:: virtual AST::Ptr visit(StackAST *)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(InputVariableAST *x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(ReferenceAST *x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(StpAST *x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(YicesAST *x)
  .. cpp:function:: virtual ASTVisitor::ASTPtr visit(SemanticsAST *x)
  .. cpp:function:: virtual ~ExtPCSensVisitor()
  .. cpp:function:: bool isExtSens(AST::Ptr a)
  .. cpp:member:: private bool assignPC_
  .. cpp:member:: private bool isExtSens_
  .. cpp:type:: private linVar<Dyninst::DataflowAPI::Variable> DiffVar
  .. cpp:member:: private std::stack<DiffVar > diffs_


.. cpp:class:: PCSensitiveTransformer : public Transformer

  .. cpp:function:: virtual bool process(RelocBlock *, RelocGraph *)
  .. cpp:function:: PCSensitiveTransformer(AddressSpace *as, PriorityMap &)
  .. cpp:function:: virtual ~PCSensitiveTransformer()
  .. cpp:function:: static void invalidateCache(func_instance *)
  .. cpp:function:: static void invalidateCache(const block_instance *)
  .. cpp:function:: private bool analysisRequired(RelocBlock *)
  .. cpp:function:: private bool isPCSensitive(InstructionAPI::Instruction insn, Address addr, const func_instance *func, const block_instance *block, AssignList &sensitiveAssignment)
  .. cpp:function:: private Graph::Ptr forwardSlice(Assignment::Ptr ptr, parse_block *block, parse_func *func)
  .. cpp:function:: private bool determineSensitivity(Graph::Ptr slice, bool &intSens, bool &extSens)
  .. cpp:function:: private bool insnIsThunkCall(InstructionAPI::Instruction insn, Address addr, Absloc &destination)
  .. cpp:function:: private void handleThunkCall(RelocBlock *b_iter, RelocGraph *cfg, WidgetList::iterator &iter, Absloc &destination)
  .. cpp:function:: private void emulateInsn(RelocBlock *b_iter, RelocGraph *cfg, WidgetList::iterator &iter, InstructionAPI::Instruction insn, Address addr)
  .. cpp:function:: private bool exceptionSensitive(Address addr, const block_instance *bbl)
  .. cpp:function:: private bool isSyscall(InstructionAPI::Instruction insn, Address addr)
  .. cpp:function:: private static void cacheAnalysis(const block_instance *bbl, Address addr, bool intSens, bool extSens)
  .. cpp:function:: private static bool queryCache(const block_instance *bbl, Address addr, bool &intSens, bool &extSens)
  .. cpp:member:: private AssignmentConverter aConverter
  .. cpp:member:: private AddressSpace *addrSpace
  .. cpp:member:: private long Sens_
  .. cpp:member:: private long extSens_
  .. cpp:member:: private long intSens_
  .. cpp:member:: private long thunk_
  .. cpp:member:: private long overApprox_
  .. cpp:member:: private adhocMovementTransformer adhoc

    And for times we don't want the overhead - if non-defensive or system libraries

  .. cpp:type:: private std::pair<bool, bool> CacheData
  .. cpp:type:: private std::map<Address, CacheData> CacheEntry
  .. cpp:type:: private std::map<const block_instance *, CacheEntry > AnalysisCache
  .. cpp:member:: private static AnalysisCache analysisCache_
