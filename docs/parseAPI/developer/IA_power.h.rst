.. _`sec:IA_power.h`:

IA_power.h
##########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: IA_power : public IA_IAPI

  .. cpp:function:: IA_power(Dyninst::InstructionAPI::InstructionDecoder dec_, Address start_, Dyninst::ParseAPI::CodeObject* o, Dyninst::ParseAPI::CodeRegion* r, Dyninst::InstructionSource *isrc, Dyninst::ParseAPI::Block * curBlk_)
  .. cpp:function:: IA_power(const IA_power &)
  .. cpp:function:: virtual IA_power* clone() const
  .. cpp:function:: virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction) const
  .. cpp:function:: virtual bool isNop() const
  .. cpp:function:: virtual bool isThunk() const
  .. cpp:function:: virtual bool isTailCall(const ParseAPI::Function* context, ParseAPI::EdgeTypeEnum type, unsigned int, const set<Address>& knownTargets) const
  .. cpp:function:: virtual bool savesFP() const
  .. cpp:function:: virtual bool isStackFramePreamble() const
  .. cpp:function:: virtual bool cleansStack() const
  .. cpp:function:: virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const
  .. cpp:function:: virtual bool isReturnAddrSave(Address& retAddr) const
  .. cpp:function:: virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
  .. cpp:function:: virtual bool isFakeCall() const
  .. cpp:function:: virtual bool isIATcall(std::string &) const
  .. cpp:function:: virtual bool isLinkerStub() const
  .. cpp:function:: virtual bool isNopJump() const

.. cpp:class:: PPC_BLR_Visitor: public ASTVisitor

  .. cpp:function:: PPC_BLR_Visitor(Address ret)
  .. cpp:function:: virtual AST::Ptr visit(AST*)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::BottomAST*)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::ConstantAST*)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::VariableAST*)
  .. cpp:function:: virtual AST::Ptr visit(DataflowAPI::RoseAST*)
  .. cpp:function:: virtual ASTPtr visit(InputVariableAST*)
  .. cpp:function:: virtual ASTPtr visit(ReferenceAST*)
  .. cpp:function:: virtual ASTPtr visit(StpAST*)
  .. cpp:function:: virtual ASTPtr visit(YicesAST*)
  .. cpp:function:: virtual ASTPtr visit(SemanticsAST*)
  .. cpp:function:: ReturnState returnState() const

.. cpp:enum:: PPC_BLR_Visitor::ReturnState

  .. cpp:enumerator:: PPC_BLR_UNSET
  .. cpp:enumerator:: PPC_BLR_UNKNOWN
  .. cpp:enumerator:: PPC_BLR_RETURN
  .. cpp:enumerator:: PPC_BLR_NOTRETURN
