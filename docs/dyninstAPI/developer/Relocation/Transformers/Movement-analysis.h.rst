.. _`sec:Movement-analysis.h`:

Movement-analysis.h
###################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: ExtPCSensVisitor : public ASTVisitor

  .. cpp:function:: ExtPCSensVisitor(const AbsRegion &a)
  .. cpp:function:: virtual AST::Ptr visit(AST *)
  .. cpp:function:: virtual AST::Ptr visit(Dyninst::DataflowAPI::BottomAST *)
  .. cpp:function:: virtual AST::Ptr visit(Dyninst::DataflowAPI::ConstantAST *)
  .. cpp:function:: virtual AST::Ptr visit(Dyninst::DataflowAPI::VariableAST *)
  .. cpp:function:: virtual AST::Ptr visit(Dyninst::DataflowAPI::RoseAST *)
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
  .. cpp:type:: private linVar<Dyninst::Dyninst::DataflowAPI::Variable> DiffVar
  .. cpp:member:: private std::stack<DiffVar > diffs_


.. cpp:class:: PCSensitiveTransformer : public Transformer

  .. cpp:function:: virtual bool process(RelocBlock *, RelocGraph *)
  .. cpp:function:: PCSensitiveTransformer(AddressSpace *as, PriorityMap &)
  .. cpp:function:: virtual ~PCSensitiveTransformer()
  .. cpp:function:: static void invalidateCache(func_instance *)
  .. cpp:function:: static void invalidateCache(const block_instance *)
  .. cpp:function:: private bool analysisRequired(RelocBlock *)
  .. cpp:function:: private bool isPCSensitive(Dyninst::InstructionAPI::Instruction insn, Dyninst::Address addr, const func_instance *func, const block_instance *block, AssignList &sensitiveAssignment)
  .. cpp:function:: private Graph::Ptr forwardSlice(Assignment::Ptr ptr, parse_block *block, parse_func *func)
  .. cpp:function:: private bool determineSensitivity(Graph::Ptr slice, bool &intSens, bool &extSens)

    Examine a slice to determine whether any of its terminal nodes will cause the program to produce a different value.
    As a secondary, divide terminal nodes into the set that will produce a different value (pos) and those that will
    not (neg).

  .. cpp:function:: private bool insnIsThunkCall(Dyninst::InstructionAPI::Instruction insn, Dyninst::Address addr, Absloc &destination)

    An example of a group transformation. If this is a call to a thunk function then record both that (as in return
    true) and where the return address gets put.

  .. cpp:function:: private void handleThunkCall(RelocBlock *b_iter, RelocGraph *cfg, WidgetList::iterator &iter, Absloc &destination)
  .. cpp:function:: private void emulateInsn(RelocBlock *b_iter, RelocGraph *cfg, WidgetList::iterator &iter, Dyninst::InstructionAPI::Instruction insn, Dyninst::Address addr)
  .. cpp:function:: private bool exceptionSensitive(Dyninst::Address addr, const block_instance *bbl)

    Checks if `addr` is exception sensitive. That is, if the address is in a function that contains a catch block.

  .. cpp:function:: private bool isSyscall(Dyninst::InstructionAPI::Instruction insn, Dyninst::Address addr)
  .. cpp:function:: private static void cacheAnalysis(const block_instance *bbl, Dyninst::Address addr, bool intSens, bool extSens)
  .. cpp:function:: private static bool queryCache(const block_instance *bbl, Dyninst::Address addr, bool &intSens, bool &extSens)
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
  .. cpp:type:: private std::map<Dyninst::Address, CacheData> CacheEntry
  .. cpp:type:: private std::map<const block_instance *, CacheEntry > AnalysisCache
  .. cpp:member:: private static AnalysisCache analysisCache_
