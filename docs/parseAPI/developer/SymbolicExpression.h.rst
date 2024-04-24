.. _`sec:SymbolicExpression.h`:

SymbolicExpression.h
####################

.. cpp:class:: SymbolicExpression

  **Tracks the expanded assignments**

  Also provides several helper functions for manipulating ASTs.

  .. cpp:member:: Dyninst::ParseAPI::CodeSource* cs

  .. cpp:member:: Dyninst::ParseAPI::CodeRegion* cr

      For archive, there are overlapping regions. Need to use the right region.

  .. cpp:function:: Dyninst::AST::Ptr SimplifyRoot(Dyninst::AST::Ptr ast, Dyninst::Address addr, bool keepMultiOne = false)
  .. cpp:function:: Dyninst::AST::Ptr SimplifyAnAST(Dyninst::AST::Ptr ast, Dyninst::Address addr, bool keepMultiOne = false)
  .. cpp:function:: static Dyninst::AST::Ptr SubstituteAnAST(Dyninst::AST::Ptr ast, const std::map<Dyninst::AST::Ptr, Dyninst::AST::Ptr>& aliasMap)
  .. cpp:function:: static Dyninst::AST::Ptr DeepCopyAnAST(Dyninst::AST::Ptr ast)
  .. cpp:function:: static bool ContainAnAST(Dyninst::AST::Ptr root, Dyninst::AST::Ptr check)
  .. cpp:function:: bool ReadMemory(Dyninst::Address addr, uint64_t &val, int size)
  .. cpp:function:: std::pair<Dyninst::AST::Ptr, bool> ExpandAssignment(Dyninst::Assignment::Ptr, bool keepMultiOne = false)
  .. cpp:function:: static Dyninst::Address PCValue(Dyninst::Address cur, size_t insnSize, Dyninst::Architecture a)

      On x86 and x86-64, the value of PC is post-instruction which is the current address plus the length of the instruction.
      On ARMv8, the value of PC is pre-instruction, which is the current address.
