.. _`sec:ast.h`:

ast.h
#####

.. cpp:type:: boost::shared_ptr<AstNode> AstNodePtr
.. cpp:type:: boost::shared_ptr<AstMiniTrampNode> AstMiniTrampNodePtr


.. cpp:class:: AstNode : public Dyninst::PatchAPI::Snippet

  .. cpp:member:: protected int lineNum
  .. cpp:member:: protected int columnNum
  .. cpp:member:: protected char *snippetName
  .. cpp:member:: protected bool lineInfoSet
  .. cpp:member:: protected bool columnInfoSet
  .. cpp:member:: protected bool snippetNameSet
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
  .. cpp:function:: static AstNodePtr stackRemoveNode(int size, MSpecialType type, func_instance *func, bool canaryAfterPrologue, long canaryHeight)
  .. cpp:function:: static AstNodePtr stackGenericNode()
  .. cpp:function:: bool allocateCanaryRegister(codeGen &gen, bool noCost, Dyninst::Register &reg, bool &needSaveAndRestore)
  .. cpp:function:: static AstNodePtr labelNode(std::string &label)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, void *arg)
  .. cpp:function:: static AstNodePtr operandNode(operandType ot, AstNodePtr ast)
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
  .. cpp:function:: virtual bool initRegisters(codeGen &gen)
  .. cpp:function:: virtual void setVariableAST(codeGen &)
  .. cpp:function:: unsigned getTreeSize()
  .. cpp:function:: bool decRefCount()
  .. cpp:function:: bool previousComputationValid(Dyninst::Register &reg, codeGen &gen)
  .. cpp:function:: void cleanRegTracker(regTracker_t *tracker, int level)
  .. cpp:function:: virtual AstNodePtr operand() const
  .. cpp:function:: virtual bool containsFuncCall() const = 0
  .. cpp:function:: virtual bool usesAppRegister() const = 0



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


.. code:: cpp

  #define SCAST_AST(ast) boost::static_pointer_cast<AstNode>(ast)
  #define DCAST_AST(ast) boost::dynamic_pointer_cast<AstNode>(ast)

