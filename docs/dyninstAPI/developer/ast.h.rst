.. _`sec:ast.h`:

ast.h
#####

.. cpp:type:: boost::shared_ptr<AstNode> AstNodePtr
.. cpp:type:: boost::shared_ptr<AstMiniTrampNode> AstMiniTrampNodePtr


.. cpp:class:: AstNode : public Dyninst::PatchAPI::Snippet

  .. cpp:member:: int referenceCount
  .. cpp:member:: int useCount

  .. cpp:member:: protected int lineNum
  .. cpp:member:: protected int columnNum
  .. cpp:member:: protected char *snippetName
  .. cpp:member:: protected bool lineInfoSet
  .. cpp:member:: protected bool columnInfoSet
  .. cpp:member:: protected bool snippetNameSet
  .. cpp:member:: protected BPatch_type *bptype
  .. cpp:member:: protected bool doTypeCheck
  .. cpp:member:: protected int size

  .. cpp:member:: private static AstNodePtr originalAddrNode_
  .. cpp:member:: private static AstNodePtr actualAddrNode_
  .. cpp:member:: private static AstNodePtr dynamicTargetNode_

  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: std::string convert(operandType type)
  .. cpp:function:: std::string convert(opCode op)

  ......

  .. rubric::
    These are for error reporting in dynC_API
  
  .. cpp:function:: int getLineNum()

    Returns the line number at which the ast was declared

  .. cpp:function:: int getColumnNum()

    Returns the column number at which the ast was declared

  .. cpp:function:: char *getSnippetName()
  .. cpp:function:: void setLineNum(int ln)

    Sets the line number at which the ast was declared

  .. cpp:function:: void setColumnNum(int cn)

    Sets the column number at which the ast was declared

  .. cpp:function:: void setSnippetName(char *n)
  .. cpp:function:: bool hasLineInfo()
  .. cpp:function:: bool hasColumnInfo()
  .. cpp:function:: bool hasNameInfo()

  ......

  .. cpp:function:: AstNode()
  .. cpp:function:: static AstNodePtr nullNode()
  .. cpp:function:: static AstNodePtr stackInsertNode(int size, MSpecialType type = GENERIC_AST)
  .. cpp:function:: static AstNodePtr stackRemoveNode(int size, MSpecialType type)
  .. cpp:function:: static AstNodePtr stackRemoveNode(int size, MSpecialType type, func_instance *func, bool canaryAfterPrologue, long canaryHeight)
  .. cpp:function:: static AstNodePtr stackGenericNode()
  .. cpp:function:: bool allocateCanaryRegister(codeGen &gen, bool noCost, Dyninst::Register &reg, bool &needSaveAndRestore)
  .. cpp:function:: static AstNodePtr labelNode(std::string &label)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, void *arg)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, AstNodePtr ast)

    TODO: this is an indirect load; should be an operator.

  .. cpp:function:: static AstNodePtr operandNode(operandType ot, const image_variable *iv)
  .. cpp:function:: static AstNodePtr memoryNode(memoryType ot, int which, int size = 8)
  .. cpp:function:: static AstNodePtr sequenceNode(std::vector<AstNodePtr> &sequence)
  .. cpp:function:: static AstNodePtr variableNode(std::vector<AstNodePtr> &ast_wrappers_, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset>> *ranges = NULL)
  .. cpp:function:: static AstNodePtr operatorNode(opCode ot, AstNodePtr l = AstNodePtr(), AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr())
  .. cpp:function:: static AstNodePtr funcCallNode(const std::string &func, std::vector<AstNodePtr> &args, AddressSpace *addrSpace = NULL)
  .. cpp:function:: static AstNodePtr funcCallNode(func_instance *func, std::vector<AstNodePtr> &args)
  .. cpp:function:: static AstNodePtr funcCallNode(func_instance *func)
  .. cpp:function:: static AstNodePtr funcCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args)
  .. cpp:function:: static AstNodePtr threadIndexNode()

      We use one of these across all platforms, since it devolves into a process-specific function node.
      However, this lets us delay that until code generation when we have the process pointer.

  .. cpp:function:: static AstNodePtr scrambleRegistersNode()
  .. cpp:function:: static AstNodePtr miniTrampNode(AstNodePtr tramp)
  .. cpp:function:: static AstNodePtr originalAddrNode()
  .. cpp:function:: static AstNodePtr actualAddrNode()
  .. cpp:function:: static AstNodePtr dynamicTargetNode()
  .. cpp:function:: static AstNodePtr snippetNode(Dyninst::PatchAPI::SnippetPtr snip)
  .. cpp:function:: AstNode(AstNodePtr src)
  .. cpp:function:: virtual ~AstNode()
  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

      This procedure generates code for an AST DAG. If there is a sub-graph being shared between more than
      1 node, then the code is generated only once for this sub-graph and the register where the return
      value of the sub-graph is stored, is kept allocated until the last node sharing the sub-graph has
      used it (freeing it afterwards). A count called "useCount" is used to determine whether a particular
      node or sub-graph is being shared. At the end of the call to generate code, this count must be 0 for
      every node. Another important issue to notice is that we have to make sure that if a node is not
      calling generate code recursively for either its left or right operands, we then need to make sure
      that we update the "useCount" for these nodes (otherwise we might be keeping registers allocated
      without reason).

      This code was modified in order to set the proper "useCount" for every node in
      the DAG before calling the original generateCode procedure (now generateCode_phase2). This means
      that we are traversing the DAG twice, but with the advantage of potencially generating more
      efficient code.

      Note: a complex Ast DAG might require more registers than the ones currently
      available. In order to fix this problem, we will need to implement a "virtual" register allocator -

  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost)
  .. cpp:function:: virtual bool generateCode(codeGen &gen, bool noCost, Dyninst::Register &retReg)
  .. cpp:function:: virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)

      For now, we only care if we should save everything. "Everything", of course, is platform dependent.
      This is the new location of the :cpp:func:`clobberAllFuncCalls` that had previously been in :cpp:func:`emitCall`.

  .. cpp:function:: virtual void setVariableAST(codeGen &)
  .. cpp:function:: unsigned getTreeSize()
  .. cpp:function:: bool decRefCount()
  .. cpp:function:: bool previousComputationValid(Dyninst::Register &reg, codeGen &gen)
  .. cpp:function:: void cleanRegTracker(regTracker_t *tracker, int level)
  .. cpp:function:: virtual AstNodePtr operand() const
  .. cpp:function:: virtual bool containsFuncCall() const = 0
  .. cpp:function:: virtual bool usesAppRegister() const = 0
  .. cpp:function:: int minCost() const
  .. cpp:function:: int avgCost() const
  .. cpp:function:: int maxCost() const
  .. cpp:function:: virtual int costHelper(enum CostStyleType) const
  .. cpp:function:: void setUseCount()

    This name is a bit of a misnomer. It's not the strict use count; it's the
    use count modified by whether a node can be kept or not. We can treat
    un-keepable nodes (AKA those that don't strictly depend on their AST inputs)
    as multiple different nodes that happen to have the same children; keepable
    nodes are the "same". If that makes any sense.

    In any case, we use the following algorithm to set use counts:

     - DFS through the AST graph.
     - If an AST can be kept:
       - Increase its use count;
       - Return.
     - If an AST cannot be kept:
       - Recurse to each child;
       - Return

    The result is all nodes having counts of 0, 1, or ``>1``.

     - ``0``: node cannot be kept, or is only reached via a keepable node.
     - ``1``: Node can be kept, but doesn't matter as it's only used once.
     - ``>1``: keep result in a register.

  .. cpp:function:: int getSize()
  .. cpp:function:: void cleanUseCount()
  .. cpp:function:: bool checkUseCount(registerSpace *, bool &)
  .. cpp:function:: void printUseCount()
  .. cpp:function:: virtual const std::vector<AstNodePtr> getArgs()
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: void decUseCount(codeGen &gen)

    Occasionally, we do not call :cpp:func:`generateCode_phase2` for the referenced node, but generate code by
    hand. This routine decrements its use count properly

  .. cpp:function:: void fixChildrenCounts()

    Our children may have incorrect useCounts (most likely they
    assume that we will not bother them again, which is wrong)

  .. cpp:function:: virtual bool canBeKept() const = 0
  .. cpp:function:: Dyninst::Register allocateAndKeep(codeGen &gen, bool noCost)

      Allocate a register and make it available for sharing if our node is shared

  .. cpp:function:: bool stealRegister(Dyninst::Register reg)
  .. cpp:function:: bool subpath(const std::vector<AstNode *> &path1, const std::vector<AstNode *> &path2) const
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual void setOValue(void *)
  .. cpp:function:: virtual const void *getOValue() const
  .. cpp:function:: virtual const image_variable *getOVar() const
  .. cpp:function:: virtual void emitVariableStore(opCode, Dyninst::Register, Dyninst::Register, codeGen &, bool, registerSpace *, int, const instPoint *, AddressSpace *)
  .. cpp:function:: virtual void emitVariableLoad(opCode, Dyninst::Register, Dyninst::Register, codeGen &, bool, registerSpace *, int, const instPoint *, AddressSpace *)
  .. cpp:function:: bool condMatch(AstNode *a, std::vector<dataReqNode *> &data_tuple1, std::vector<dataReqNode *> &data_tuple2, std::vector<dataReqNode *> datareqs1, std::vector<dataReqNode *> datareqs2)
  .. cpp:function:: virtual operandType getoType() const
  .. cpp:function:: virtual void setConstFunc(bool)
  .. cpp:function:: BPatch_type *getType()
  .. cpp:function:: void setType(BPatch_type *t)
  .. cpp:function:: void setTypeChecking(bool x)
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function *func = NULL)
  .. cpp:function:: virtual bool generate(Dyninst::PatchAPI::Point *, Dyninst::Buffer &)


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
  .. cpp:enumerator:: DataAddr
  .. cpp:enumerator:: FrameAddr
  .. cpp:enumerator:: RegOffset
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


.. cpp:class:: AstNullNode : public AstNode

  .. cpp:function:: AstNullNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

.. cpp:class:: AstStackInsertNode : public AstNode

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
  .. cpp:function:: AstStackRemoveNode(int s, MSpecialType t, func_instance *func, bool canaryAfterPrologue, long canaryHeight)
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:member:: private int size
  .. cpp:member:: private MSpecialType type
  .. cpp:member:: private func_instance *func_{}
  .. cpp:member:: private bool canaryAfterPrologue_{}
  .. cpp:member:: private long canaryHeight_{}

.. cpp:class:: AstStackGenericNode : public AstNode

  .. cpp:function:: AstStackGenericNode()
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
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function *func = NULL)
  .. cpp:function:: virtual bool accessesParam()

    This is not the most efficient way to traverse a DAG

  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private bool generateOptimizedAssignment(codeGen &gen, int size, bool noCost)
  .. cpp:member:: private opCode op{}
  .. cpp:member:: private AstNodePtr loperand
  .. cpp:member:: private AstNodePtr roperand
  .. cpp:member:: private AstNodePtr eoperand

.. cpp:class:: AstOperandNode : public AstNode

  .. cpp:function:: AstOperandNode(operandType ot, void *arg)
  .. cpp:function:: AstOperandNode(operandType ot, AstNodePtr l)
  .. cpp:function:: AstOperandNode(operandType ot, const image_variable *iv)
  .. cpp:function:: ~AstOperandNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual operandType getoType() const
  .. cpp:function:: virtual void setOValue(void *o)
  .. cpp:function:: virtual const void *getOValue() const
  .. cpp:function:: virtual const image_variable *getOVar() const
  .. cpp:function:: virtual AstNodePtr operand() const
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function *func = NULL)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual bool canBeKept() const

    Check if the node can be kept at all. Some nodes (e.g., storeOp) can not be cached. In fact, there
    are fewer nodes that can be cached.

  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: virtual void emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool noCost, registerSpace *rs, int size, const instPoint *point, AddressSpace *as)
  .. cpp:function:: virtual void emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen &gen, bool noCost, registerSpace *rs, int size, const instPoint *point, AddressSpace *as)
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private int_variable *lookUpVar(AddressSpace *as)
  .. cpp:function:: private AstOperandNode()
  .. cpp:member:: private operandType oType
  .. cpp:member:: private void *oValue
  .. cpp:member:: private const image_variable *oVar
  .. cpp:member:: private AstNodePtr operand_

.. cpp:class:: AstCallNode : public AstNode

  .. cpp:function:: AstCallNode(func_instance *func, std::vector<AstNodePtr> &args)
  .. cpp:function:: AstCallNode(const std::string &str, std::vector<AstNodePtr> &args)
  .. cpp:function:: AstCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args)
  .. cpp:function:: AstCallNode(func_instance *func)
  .. cpp:function:: ~AstCallNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function *func = NULL)
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
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private AstCallNode()
  .. cpp:member:: private const std::string func_name_
  .. cpp:member:: private Dyninst::Address func_addr_
  .. cpp:member:: private func_instance *func_
  .. cpp:member:: private std::vector<AstNodePtr> args_
  .. cpp:member:: private bool callReplace_
  .. cpp:member:: private bool constFunc_

.. cpp:class:: AstSequenceNode : public AstNode

  .. cpp:function:: AstSequenceNode(std::vector<AstNodePtr> &sequence)
  .. cpp:function:: ~AstSequenceNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function *func = NULL)
  .. cpp:function:: virtual bool accessesParam()
  .. cpp:function:: virtual bool canBeKept() const

    Theoretically we could keep the entire thing, but... not sure
    that's a terrific idea. For now, don't keep a sequence node around.

  .. cpp:function:: virtual void getChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual void setChildren(std::vector<AstNodePtr> &children)
  .. cpp:function:: virtual AstNodePtr deepCopy()
  .. cpp:function:: virtual void setVariableAST(codeGen &gen)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private AstSequenceNode()
  .. cpp:member:: private std::vector<AstNodePtr> sequence_

.. cpp:class:: AstVariableNode : public AstNode

  .. cpp:function:: AstVariableNode(std::vector<AstNodePtr> &ast_wrappers, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset>> *ranges)
  .. cpp:function:: ~AstVariableNode()
  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual int costHelper(enum CostStyleType costStyle) const
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function * = NULL)
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
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private AstVariableNode()
  .. cpp:member:: private std::vector<AstNodePtr> ast_wrappers_
  .. cpp:member:: private std::vector<std::pair<Dyninst::Offset, Dyninst::Offset>> *ranges_
  .. cpp:member:: private unsigned index

.. cpp:class:: AstMiniTrampNode : public AstNode

  .. cpp:function:: AstMiniTrampNode(AstNodePtr ast)
  .. cpp:function:: Dyninst::Address generateTramp(codeGen &gen, int &trampCost, bool noCost)
  .. cpp:function:: virtual ~AstMiniTrampNode()
  .. cpp:function:: virtual bool accessesParam()
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

    Despite our memory loads, we can be kept; we're loading off process state, which is defined
    to be invariant during the instrumentation phase.

  .. cpp:function:: virtual std::string format(std::string indent)
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:function:: private AstMemoryNode()
  .. cpp:member:: private memoryType mem_{}
  .. cpp:member:: private unsigned which_{}

.. cpp:class:: AstOriginalAddrNode : public AstNode

  .. cpp:function:: AstOriginalAddrNode()
  .. cpp:function:: virtual ~AstOriginalAddrNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function * = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

.. cpp:class:: AstActualAddrNode : public AstNode

  .. cpp:function:: AstActualAddrNode()
  .. cpp:function:: virtual ~AstActualAddrNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function * = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

.. cpp:class:: AstDynamicTargetNode : public AstNode

  .. cpp:function:: AstDynamicTargetNode()
  .. cpp:function:: virtual ~AstDynamicTargetNode()
  .. cpp:function:: virtual BPatch_type *checkType(BPatch_function * = NULL)
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

.. cpp:class:: AstScrambleRegistersNode : public AstNode

  .. cpp:function:: AstScrambleRegistersNode()
  .. cpp:function:: virtual ~AstScrambleRegistersNode()
  .. cpp:function:: virtual bool canBeKept() const
  .. cpp:function:: virtual bool containsFuncCall() const
  .. cpp:function:: virtual bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)

.. cpp:class:: AstSnippetNode : public AstNode

  .. cpp:function:: AstSnippetNode(Dyninst::PatchAPI::SnippetPtr snip)
  .. cpp:function:: bool canBeKept() const
  .. cpp:function:: bool containsFuncCall() const
  .. cpp:function:: bool usesAppRegister() const
  .. cpp:function:: private virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr, Dyninst::Register &retReg)
  .. cpp:member:: private Dyninst::PatchAPI::SnippetPtr snip_


.. code:: cpp

  #define SCAST_AST(ast) boost::static_pointer_cast<AstNode>(ast)
  #define DCAST_AST(ast) boost::dynamic_pointer_cast<AstNode>(ast)

