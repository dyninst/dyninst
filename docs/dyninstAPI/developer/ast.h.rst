.. _`sec:ast.h`:

ast.h
#####

AST class for use in generating primitive and pred calls

Dyninst::Register retention mechanism...
If we've already calculated a result, then we want to reuse it if it's
still available. This means it was calculated along a path that reaches the
current point (not inside a conditional) and the register hasn't been
reused. We handle this so:

  1. Iterate over the AST tree and see if any node is reached more than once; if so, mark it as potentially being
  worth keeping around. We can do this because we use pointers; a better approach would be a comparison operator.

  2. Start generation at "level 0".

  3. When a conditional AST is reached, generate each child at level+1.

  4. When the AST is reached during code generation, and doesn't have a register:

    4a. Allocate a register for it;
    4b. Enter that register, the AST node, and the current level in the global table.

  5. If it does have a register, reuse it.

  6. When the conditionally executed branch is finished, clean all entries in the table with that level value
  (undoing all kept registers along that path).

  7. If we need a register, the register allocator (registerSpace. can forcibly undo this optimization and grab
  a register. Grab the register from the AstNode with the lowest usage count.


.. cpp:type:: boost::shared_ptr<AstNode> AstNodePtr
.. cpp:type:: boost::shared_ptr<AstMiniTrampNode> AstMiniTrampNodePtr


.. cpp:enum:: cfjRet_t

  .. cpp:enumerator:: cfj_unset
  .. cpp:enumerator:: cfj_none
  .. cpp:enumerator:: cfj_jump
  .. cpp:enumerator:: cfj_call


.. cpp:class:: regTracker_t

  .. cpp:member:: int condLevel
  .. cpp:function:: static unsigned astHash(AstNode * const &ast)
  .. cpp:function:: regTracker_t()
  .. cpp:member:: std::unordered_map<AstNode *, commonExpressionTracker> tracker
  .. cpp:function:: void addKeptRegister(codeGen &gen, AstNode *n, Dyninst::Register reg)
  .. cpp:function:: void removeKeptRegister(codeGen &gen, AstNode *n)
  .. cpp:function:: Dyninst::Register hasKeptRegister(AstNode *n)
  .. cpp:function:: bool stealKeptRegister(Dyninst::Register reg)
  .. cpp:function:: void reset()
  .. cpp:function:: void increaseConditionalLevel()
  .. cpp:function:: void decreaseAndClean(codeGen &gen)
  .. cpp:function:: void cleanKeptRegisters(int level)
  .. cpp:function:: void debugPrint()


.. cpp:class:: regTracker_t::commonExpressionTracker

  .. cpp:member:: Dyninst::Register keptRegister
  .. cpp:member:: int keptLevel
  .. cpp:function:: commonExpressionTracker()

.. cpp:class:: AstNode : public Dyninst::PatchAPI::Snippet

  TODO: Needs some way of marking what to save and restore... should be a registerSpace, really

  .. cpp:member:: protected int lineNum
  .. cpp:member:: protected int columnNum
  .. cpp:member:: protected char *snippetName
  .. cpp:member:: protected bool lineInfoSet
  .. cpp:member:: protected bool columnInfoSet
  .. cpp:member:: protected bool snippetNameSet
  .. cpp:member:: int referenceCount

      Reference count for freeing memory

  .. cpp:member:: int useCount

      Reference count for generating code

  .. cpp:member:: private static AstNodePtr originalAddrNode_
  .. cpp:member:: private static AstNodePtr actualAddrNode_
  .. cpp:member:: private static AstNodePtr dynamicTargetNode_
  .. cpp:member:: protected BPatch_type *bptype

      type of corresponding BPatch_snippet

  .. cpp:member:: protected bool doTypeCheck

      should operands be type checked

  .. cpp:member:: protected int size

      size of the operations(in bytes)

  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: std::string convert(operandType type)
  .. cpp:function:: std::string convert(opCode op)
  .. cpp:function:: int getLineNum()
  .. cpp:function:: int getColumnNum()
  .. cpp:function:: char *getSnippetName()
  .. cpp:function:: void setLineNum(int ln)
  .. cpp:function:: void setColumnNum(int cn)
  .. cpp:function:: void setSnippetName(char *n)
  .. cpp:function:: bool hasLineInfo()
  .. cpp:function:: bool hasColumnInfo()
  .. cpp:function:: bool hasNameInfo()
  .. cpp:function:: AstNode()
  .. cpp:function:: static AstNodePtr nullNode()
  .. cpp:function:: static AstNodePtr stackInsertNode(int size, MSpecialType type = GENERIC_AST)
  .. cpp:function:: static AstNodePtr stackRemoveNode(int size, MSpecialType type)
  .. cpp:function:: static AstNodePtr stackRemoveNode(int size, MSpecialType type, func_instance* func, bool canaryAfterPrologue, long canaryHeight)
  .. cpp:function:: static AstNodePtr stackGenericNode()
  .. cpp:function:: bool allocateCanaryRegister(codeGen& gen, bool noCost, Dyninst::Register& reg, bool& needSaveAndRestore)
  .. cpp:function:: static AstNodePtr labelNode(std::string &label)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, void *arg)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, AstNodePtr ast)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, const image_variable* iv)
  .. cpp:function:: static AstNodePtr memoryNode(memoryType ot, int which, int size = 8)
  .. cpp:function:: static AstNodePtr sequenceNode(std::vector<AstNodePtr > &sequence)
  .. cpp:function:: static AstNodePtr variableNode(std::vector<AstNodePtr>&ast_wrappers_, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges = NULL)
  .. cpp:function:: static AstNodePtr operatorNode(opCode ot, AstNodePtr l = AstNodePtr(), AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr())
  .. cpp:function:: static AstNodePtr funcCallNode(const std::string &func, std::vector<AstNodePtr > &args, AddressSpace *addrSpace = NULL)
  .. cpp:function:: static AstNodePtr funcCallNode(func_instance *func, std::vector<AstNodePtr > &args)
  .. cpp:function:: static AstNodePtr funcCallNode(func_instance *func)

      Special case for function call replacement.

  .. cpp:function:: static AstNodePtr funcCallNode(Dyninst::Address addr, std::vector<AstNodePtr > &args)

      For when you absolutely need to jump somewhere.

  .. cpp:function:: static AstNodePtr threadIndexNode()

      Acquire the thread index value - a 0...n labelling of threads.

  .. cpp:function:: static AstNodePtr scrambleRegistersNode()
  .. cpp:function:: static AstNodePtr miniTrampNode(AstNodePtr tramp)
  .. cpp:function:: static AstNodePtr originalAddrNode()
  .. cpp:function:: static AstNodePtr actualAddrNode()
  .. cpp:function:: static AstNodePtr dynamicTargetNode()
  .. cpp:function:: static AstNodePtr snippetNode(Dyninst::PatchAPI::SnippetPtr snip)
  .. cpp:function:: AstNode(AstNodePtr src)
  .. cpp:function:: virtual ~AstNode()
  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost)
  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Register &retReg)
  .. cpp:function:: virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

      I don't know if there is an overload between address and register, so we'll toss in two different return types.

  .. cpp:function:: virtual bool initRegisters(codeGen &gen)

      Perform whatever pre-processing steps are necessary.

  .. cpp:function:: virtual void setVariableAST(codeGen &)

      Select the appropriate Variable AST as part of pre-processing steps before code generation.

  .. cpp:function:: unsigned getTreeSize()
  .. cpp:function:: bool decRefCount()
  .. cpp:function:: bool previousComputationValid(Dyninst::Register &reg, codeGen &gen)
  .. cpp:function:: void cleanRegTracker(regTracker_t *tracker, int level)

      Remove any kept register at a greater level than that provided (AKA that had been calculated
      within a conditional statement).

  .. cpp:function:: virtual AstNodePtr operand() const
  .. cpp:function:: virtual bool containsFuncCall() const = 0
  .. cpp:function:: virtual bool usesAppRegister() const = 0
  .. cpp:function:: int minCost() const
  .. cpp:function:: int avgCost() const
  .. cpp:function:: int maxCost() const
  .. cpp:function:: virtual int costHelper(enum CostStyleType) const

      Returns the number of instruction times in the ast.

  .. cpp:function:: void setUseCount()
  .. cpp:function:: int getSize()
  .. cpp:function:: void cleanUseCount(void)
  .. cpp:function:: bool checkUseCount(registerSpace*, bool&)
  .. cpp:function:: void printUseCount(void)
  .. cpp:function:: virtual const std::vector<AstNodePtr> getArgs()
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr > &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: void decUseCount(codeGen &gen)

    Occasionally, we do not call ``generateCode_phase2`` for the referenced node, but generate
    code by hand. This routine decrements its use count properly.

  .. cpp:function:: void fixChildrenCounts()

      Our children may have incorrect useCounts (most likely they assume that we will not bother
      them again, which is wrong)

  .. cpp:function:: virtual bool canBeKept() const = 0

      Check if the node can be kept at all. Some nodes (e.g., storeOp) can not be cached.

  .. cpp:function:: Dyninst::Register allocateAndKeep(codeGen &gen, bool noCost)

      Allocate a register and make it available for sharing if our node is shared

  .. cpp:function:: bool stealRegister(Dyninst::Register reg)

      If someone needs to take this guy away.

  .. cpp:function:: bool subpath(const std::vector<AstNode*> &path1, const std::vector<AstNode*> &path2) const

      Checks if path1 is a subpath of path2

  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &)

      Return all children of this node ([lre]operand, ..., operands[])

  .. cpp:function:: virtual bool accessesParam(void)
  .. cpp:function:: virtual void setOValue(void *)
  .. cpp:function:: virtual const void *getOValue() const
  .. cpp:function:: virtual const image_variable* getOVar() const
  .. cpp:function:: virtual void emitVariableStore(opCode, Dyninst::Register, Dyninst::Register, codeGen&, bool, registerSpace*, int, const instPoint*, AddressSpace*)
  .. cpp:function:: virtual void emitVariableLoad(opCode, Dyninst::Register, Dyninst::Register, codeGen&, bool, registerSpace*, int, const instPoint*, AddressSpace*)
  .. cpp:function:: bool condMatch(AstNode* a, std::vector<dataReqNode*> &data_tuple1, std::vector<dataReqNode*> &data_tuple2, std::vector<dataReqNode*> datareqs1, std::vector<dataReqNode*> datareqs2)
  .. cpp:function:: virtual operandType getoType() const
  .. cpp:function:: virtual void setConstFunc(bool)
  .. cpp:function:: BPatch_type *getType()
  .. cpp:function:: void setType(BPatch_type *t)
  .. cpp:function:: void setTypeChecking(bool x)
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* func = NULL)
  .. cpp:function:: virtual bool generate(Dyninst::PatchAPI::Point *, Dyninst::Buffer &)

    For PatchAPI compatibility

.. cpp:enum:: AstNode::nodeType

  .. cpp:enumerator:: sequenceNode_t
  .. cpp:enumerator:: opCodeNode_t
  .. cpp:enumerator:: operandNode_t
  .. cpp:enumerator:: callNode_t
  .. cpp:enumerator:: scrambleRegisters_t


.. cpp:enum:: AstNode::operandType

  .. cpp:enumerator:: Constant
  .. cpp:enumerator:: ConstantString
  .. cpp:enumerator:: DataReg
  .. cpp:enumerator:: DataIndir
  .. cpp:enumerator:: Param
  .. cpp:enumerator:: ParamAtCall
  .. cpp:enumerator:: ParamAtEntry
  .. cpp:enumerator:: ReturnVal
  .. cpp:enumerator:: ReturnAddr

    address of a return instruction

  .. cpp:enumerator:: DataAddr

    Used to represent a variable in memory

  .. cpp:enumerator:: FrameAddr

    Calculate FP

  .. cpp:enumerator:: RegOffset

    Calculate ``*reg + offset`` oValue is reg loperand->oValue is offset.

  .. cpp:enumerator:: origRegister
  .. cpp:enumerator:: variableAddr
  .. cpp:enumerator:: variableValue
  .. cpp:enumerator:: undefOperandType


.. cpp:enum:: AstNode::memoryType

  .. cpp:enumerator:: EffectiveAddr
  .. cpp:enumerator:: BytesAccessed


.. cpp:enum:: AstNode::MSpecialType

  .. cpp:enumerator:: GENERIC_AST
  .. cpp:enumerator:: CANARY_AST


.. cpp:enum:: AstNode::CostStyleType

  .. cpp:enumerator:: Min
  .. cpp:enumerator:: Avg
  .. cpp:enumerator:: Max


.. cpp:class:: AstStackInsertNode : public AstNode

  **Stack Frame Modification**

  .. cpp:function:: AstStackInsertNode(int s, MSpecialType t)
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:member:: private int size
  .. cpp:member:: private MSpecialType type


.. cpp:class:: AstStackRemoveNode : public AstNode

  .. cpp:function:: AstStackRemoveNode(int s, MSpecialType t = GENERIC_AST)
  .. cpp:function:: AstStackRemoveNode(int s, MSpecialType t, func_instance* func, bool canaryAfterPrologue, long canaryHeight)
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:member:: private int size
  .. cpp:member:: private MSpecialType type
  .. cpp:member:: private func_instance* func_{}
  .. cpp:member:: private bool canaryAfterPrologue_{}
  .. cpp:member:: private long canaryHeight_{}


.. cpp:class:: AstStackGenericNode : public AstNode

  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)


.. cpp:class:: AstLabelNode : public AstNode

  .. cpp:function:: AstLabelNode(std::string &label)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:member:: private std::string label_
  .. cpp:member:: private Dyninst::Address generatedAddr_


.. cpp:class:: AstOperatorNode : public AstNode

  .. cpp:function:: AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr())
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* func = NULL)
  .. cpp:function:: virtual bool accessesParam(void)

     Does this AST access "Param"

  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)

      We override initRegisters in the case of writing to an original register.

  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private bool generateOptimizedAssignment(codeGen &gen, int size, bool noCost)
  .. cpp:member:: private opCode op{}
  .. cpp:member:: private AstNodePtr loperand
  .. cpp:member:: private AstNodePtr roperand
  .. cpp:member:: private AstNodePtr eoperand


.. cpp:class:: AstOperandNode : public AstNode

  .. cpp:function:: AstOperandNode(operandType ot, void *arg)

      Direct operand

  .. cpp:function:: AstOperandNode(operandType ot, AstNodePtr l)

      And an indirect (say, a load)

  .. cpp:function:: AstOperandNode(operandType ot, const image_variable* iv)
  .. cpp:function:: ~AstOperandNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual operandType getoType() const
  .. cpp:function:: virtual void setOValue(void *o)
  .. cpp:function:: virtual const void *getOValue() const
  .. cpp:function:: virtual const image_variable* getOVar() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:function:: private int_variable* lookUpVar(AddressSpace* as)
  .. cpp:function:: private AstOperandNode()
  .. cpp:member:: private operandType oType
  .. cpp:member:: private void *oValue
  .. cpp:member:: private const image_variable* oVar
  .. cpp:member:: private AstNodePtr operand_


.. cpp:class:: AstCallNode : public AstNode

  .. cpp:function:: AstCallNode(func_instance *func, std::vector<AstNodePtr>&args)
  .. cpp:function:: AstCallNode(const std::string &str, std::vector<AstNodePtr>&args)
  .. cpp:function:: AstCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args)
  .. cpp:function:: AstCallNode(func_instance *func)
  .. cpp:function:: ~AstCallNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* func = NULL)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: void setConstFunc(bool val)
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:function:: private AstCallNode()

      Sometimes we just don't have enough information.

  .. cpp:member:: private const std::string func_name_
  .. cpp:member:: private Dyninst::Address func_addr_
  .. cpp:member:: private func_instance *func_
  .. cpp:member:: private std::vector<AstNodePtr> args_
  .. cpp:member:: private bool callReplace_

      Node is intended for function call replacement

  .. cpp:member:: private bool constFunc_

      True if the output depends solely on input parameters, or can otherwise be guaranteed
      to not change if executed multiple times in the same sequence - AKA "can be kept".

.. cpp:class:: AstSequenceNode : public AstNode

  .. cpp:function:: AstSequenceNode(std::vector<AstNodePtr> &sequence)
  .. cpp:function:: ~AstSequenceNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* func = NULL)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:function:: private AstSequenceNode()
  .. cpp:member:: private std::vector<AstNodePtr> sequence_


.. cpp:class:: AstVariableNode : public AstNode

  .. cpp:function:: AstVariableNode(std::vector<AstNodePtr>&ast_wrappers, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> >*ranges)
  .. cpp:function:: ~AstVariableNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* = NULL)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual operandType getoType() const
  .. cpp:function:: virtual AstNodePtr operand() const
  .. cpp:function:: virtual const void *getOValue() const
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:function:: private AstVariableNode()
  .. cpp:member:: private std::vector<AstNodePtr>ast_wrappers_
  .. cpp:member:: private std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges_
  .. cpp:member:: private unsigned index


.. cpp:class:: AstMiniTrampNode : public AstNode

  .. cpp:function:: AstMiniTrampNode(AstNodePtr ast)
  .. cpp:function:: Dyninst::Address generateTramp(codeGen &gen, int &trampCost, bool noCost)
  .. cpp:function:: virtual ~AstMiniTrampNode()
  .. cpp:function:: virtual bool accessesParam(void)
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: AstNodePtr getAST()
  .. cpp:function:: private AstMiniTrampNode()
  .. cpp:member:: private bool inline_
  .. cpp:member:: private AstNodePtr ast_


.. cpp:class:: AstMemoryNode : public AstNode

  .. cpp:function:: AstMemoryNode(memoryType mem, unsigned which, int size)
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:function:: private AstMemoryNode()
  .. cpp:member:: private memoryType mem_{}
  .. cpp:member:: private unsigned which_{}


.. cpp:class:: AstOriginalAddrNode : public AstNode

  .. cpp:function:: AstOriginalAddrNode()
  .. cpp:function:: virtual ~AstOriginalAddrNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)


.. cpp:class:: AstActualAddrNode : public AstNode

  .. cpp:function:: AstActualAddrNode()
  .. cpp:function:: virtual ~AstActualAddrNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)


.. cpp:class:: AstDynamicTargetNode : public AstNode

  .. cpp:function:: AstDynamicTargetNode()
  .. cpp:function:: virtual ~AstDynamicTargetNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function* = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)


.. cpp:class:: AstScrambleRegistersNode : public AstNode

  .. cpp:function:: AstScrambleRegistersNode()
  .. cpp:function:: virtual ~AstScrambleRegistersNode()
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)


.. cpp:class:: AstSnippetNode : public AstNode

  This is a little odd, since an AstNode *is* a Snippet. It's a compatibility interface to
  allow generic PatchAPI snippets to play nice in our world.

  .. cpp:function:: AstSnippetNode(Dyninst::PatchAPI::SnippetPtr snip)
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: bool containsFuncCall() const
  .. cpp:function:: bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, \
                                                             Dyninst::Address &retAddr, \
                                                             Dyninst::Register &retReg)
  .. cpp:member:: private Dyninst::PatchAPI::SnippetPtr snip_


.. cpp:function:: void emitLoadPreviousStackFrameRegister(Dyninst::Address register_num, Dyninst::Register dest,\
                                                          codeGen &gen, int size, bool noCost)
.. cpp:function:: void emitStorePreviousStackFrameRegister(Dyninst::Address register_num, Dyninst::Register src, \
                                                           codeGen &gen, int size, bool noCost)

.. code:: cpp

  #define SCAST_AST(ast) boost::static_pointer_cast<AstNode>(ast)
  #define DCAST_AST(ast) boost::dynamic_pointer_cast<AstNode>(ast)

