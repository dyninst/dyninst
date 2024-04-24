.. _`sec:dev-stackanalysis.h`:

stackanalysis.h
###############

Concepts
********

Stack height
  The size of the stack: ``cur_stack_ptr - func_start_stack_ptr``

Stack delta
  The difference in the size of the stack over the execution of
  a region of code (basic block here):
  ``block_end_stack_ptr - block_start_stack_ptr``

Stack clean:
  The amount the callee function shifts the stack. This is an x86
  idiom. On x86 the caller pushes arguments onto the stack (and thus shifts
  the stack). Normally the caller also cleans the stack (that is, removes
  arguments). However, some callee functions perform this cleaning
  themselves. We need to account for this in our analysis, which otherwise
  assumes that callee functions don't alter the stack.

......

.. cpp:namespace:: Dyninst::dev

.. cpp:class:: StackAnalysis

  .. cpp:function:: bool canGetFunctionSummary()

    Returns ``true`` if the function associated with this StackAnalysis object
    returns on some execution path. Looks for return edges in the function, following
    tail calls if necessary. Returns true if any return edges are found.

  .. cpp:function:: bool getFunctionSummary(TransferSet &summary)

    Returns in ``summary`` a summary for the function associated with this
    StackAnalysis object. Function summaries can then be passed to the
    constructors for other StackAnalysis objects to enable interprocedural
    analysis.

    Returns ``true`` on success.

  .. cpp:member:: private static const unsigned DEF_LIMIT=2

    This constant limits the number of definitions we track per register. If
    more than this many definitions are found, the register is considered to
    be BOTTOM, and the definitions tracked so far are dropped.

  .. cpp:member:: private ParseAPI::Function *func
  .. cpp:member:: private std::map<Address, Address> callResolutionMap

    Map from call sites to PLT-resolved addresses

  .. cpp:member:: private std::map<Address, TransferSet> functionSummaries

    Function summaries to utilize during analysis

  .. cpp:member:: private std::set<Address> toppableFunctions

    Functions whose return values should be topped rather than bottomed.  This
    is used when evaluating a cycle in the call graph via fixed-point
    analysis.  Note that if functionSummaries contains a summary for the
    function, it will be used instead.

  ......

  .. rubric::
    SP effect tracking
    
  .. cpp:member:: private BlockEffects *blockEffects

    Pointer so we can make it an annotation

  .. cpp:member:: private InstructionEffects *insnEffects

    Pointer so we can make it an annotation

  .. cpp:member:: private CallEffects *callEffects

    Pointer so we can make it an annotation

  ......

  .. cpp:member:: private BlockState blockInputs
  .. cpp:member:: private BlockState blockOutputs
  .. cpp:member:: private BlockSummaryState blockSummaryInputs

    Like blockInputs and blockOutputs, but used for function summaries. Instead of tracking Heights, we track transfer functions.

  .. cpp:member:: private BlockSummaryState blockSummaryOutputs
  .. cpp:member:: private Intervals *intervals_

    Pointer so we can make it an annotation

  .. cpp:member:: private FuncCleanAmounts funcCleanAmounts
  .. cpp:member:: private int word_size
  .. cpp:member:: private ExpressionPtr theStackPtr
  .. cpp:member:: private ExpressionPtr thePC

  .. cpp:function:: private std::string format(const AbslocState &input) const
  .. cpp:function:: private std::string format(const TransferSet &input) const
  .. cpp:function:: private MachRegister sp()
  .. cpp:function:: private MachRegister fp()
  .. cpp:function:: private bool analyze()
  .. cpp:function:: private bool genInsnEffects()
  .. cpp:function:: private void summarizeBlocks(bool verbose = false)

      We want to create a transfer function for the block as a whole. This will
      allow us to perform our fixpoint calculation over blocks (thus, ``O(B^2)``)
      rather than instructions (thus, ``O(I^2)``).

  .. cpp:function:: private void summarize()
  .. cpp:function:: private void fixpoint(bool verbose = false)
  .. cpp:function:: private void summaryFixpoint()
  .. cpp:function:: private void createIntervals()
  .. cpp:function:: private void createEntryInput(AbslocState &input)
  .. cpp:function:: private void createSummaryEntryInput(TransferSet &input)
  .. cpp:function:: private void meetInputs(ParseAPI::Block *b, AbslocState &blockInput, AbslocState &input)
  .. cpp:function:: private void meetSummaryInputs(ParseAPI::Block *b, TransferSet &blockInput, TransferSet &input)
  .. cpp:function:: private DefHeight meetDefHeight(const DefHeight &dh1, const DefHeight &dh2)
  .. cpp:function:: private DefHeightSet meetDefHeights(const DefHeightSet &s1, const DefHeightSet &s2)

    Keep track of up to DEF_LIMIT multiple definitions/heights, then bottom

  .. cpp:function:: private void meet(const AbslocState &source, AbslocState &accum)
  .. cpp:function:: private void meetSummary(const TransferSet &source, TransferSet &accum)
  .. cpp:function:: private AbslocState getSrcOutputLocs(ParseAPI::Edge *e)
  .. cpp:function:: private TransferSet getSummarySrcOutputLocs(ParseAPI::Edge *e)
  .. cpp:function:: private void computeInsnEffects(ParseAPI::Block *block, InstructionAPI::Instruction insn, const Offset off, TransferFuncs &xferFunc, TransferSet &funcSummary)
  .. cpp:function:: private bool isCall(InstructionAPI::Instruction insn)
  .. cpp:function:: private bool isJump(InstructionAPI::Instruction insn)
  .. cpp:function:: private bool handleNormalCall(InstructionAPI::Instruction insn, ParseAPI::Block *block, Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary)
  .. cpp:function:: private bool handleThunkCall(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private bool handleJump(InstructionAPI::Instruction insn, ParseAPI::Block *block, Offset off, TransferFuncs &xferFuncs, TransferSet &funcSummary)

    Create transfer functions for tail calls

  .. cpp:function:: private void handlePushPop(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, int sign, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleReturn(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleAddSub(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, int sign, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleLEA(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleLeave(ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handlePushPopFlags(int sign, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handlePushPopRegs(int sign, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handlePowerAddSub(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, int sign, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handlePowerStoreUpdate(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleMov(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleZeroExtend(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleSignExtend(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleSpecialSignExtend(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleXor(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleDiv(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleMul(InstructionAPI::Instruction insn, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleSyscall(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)
  .. cpp:function:: private void handleDefault(InstructionAPI::Instruction insn, ParseAPI::Block *block, const Offset off, TransferFuncs &xferFuncs)

      Handle instructions for which we have no special handling implemented. Be conservative for safety.

  .. cpp:function:: private long extractDelta(InstructionAPI::Result deltaRes)

    Converts a delta in a Result to a long

  .. cpp:function:: private bool getSubReg(const MachRegister &reg, MachRegister &subreg)
  .. cpp:function:: private void retopBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs)

      If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
      retops the subregister.  If reg is a 4 byte register (eax, ebx, ecx, etc.)
      with an 8 byte base register, retops the base register.  The appropriate
      retop transfer function is added to xferFuncs.

  .. cpp:function:: private void copyBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs)

      If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
      copies the subregister into the base register.  If reg is a 4 byte register
      (eax, ebx, ecx, etc.) with an 8 byte base register, copies the base register
      into the subregister. The appropriate copy transfer function is added to
      xferFuncs.

  .. cpp:function:: private void bottomBaseSubReg(const MachRegister &reg, TransferFuncs &xferFuncs)

      If reg is an 8 byte register (rax, rbx, rcx, etc.) with a 4 byte subregister,
      bottoms the subregister.  If reg is a 4 byte register (eax, ebx, ecx, etc.)
      with an 8 byte base register, bottoms the base register.  The appropriate
      bottom transfer function is added to xferFuncs.

  .. cpp:function:: private Height getStackCleanAmount(ParseAPI::Function *func)


.. cpp:class:: StackAnalysis::Definition

  This class represents a stack pointer definition by recording the block
  and address of the definition, as well as the original absloc that was
  defined by the definition.


.. cpp:class:: Definition

  .. cpp:member:: Address addr
  .. cpp:member:: ParseAPI::Block *block
  .. cpp:member:: Absloc origLoc
  .. cpp:member:: Type type
  .. cpp:function:: Definition(ParseAPI::Block *b, Address a, Absloc l)
  .. cpp:function:: Definition(ParseAPI::Block *b, Absloc l)
  .. cpp:function:: Definition(Address a, Absloc l)
  .. cpp:function:: Definition()
  .. cpp:function:: bool operator==(const Definition &other) const

      .. error::
        **FIXME**: To pass checks in StackAnalysis::summarize(), we consider
        definitions equivalent as long as one is not BOTTOM and the other
        something else. This is not proper.
        return type == other.type && block == other.block;

  .. cpp:function:: bool operator<(const Definition &rhs) const
  .. cpp:function:: std::string format() const
  .. cpp:function:: static Definition meet(const Definition &lhs, const Definition &rhs)


.. cpp:enum:: StackAnalysis::Definition::Type

  .. cpp:enumerator:: TOP
  .. cpp:enumerator:: BOTTOM
  .. cpp:enumerator:: DEF


.. cpp:class:: Height

  This class represents offsets on the stack, which we call heights.

  .. cpp:type:: signed long Height_t
  .. cpp:member:: static const Height_t uninitialized = MAXLONG
  .. cpp:member:: static const Height_t notUnique = MINLONG
  .. cpp:member:: static const Height bottom
  .. cpp:member:: static const Height top
  .. cpp:function:: Height(const Height_t h, const Type t = HEIGHT)
  .. cpp:function:: Height()
  .. cpp:function:: Height_t height() const
  .. cpp:function:: bool operator<(const Height &rhs) const noexcept

    FIXME if we stop using TOP == MAXINT and BOT == MININT...

  .. cpp:function:: bool operator>(const Height &rhs) const noexcept
  .. cpp:function:: bool operator<=(const Height &rhs) const noexcept
  .. cpp:function:: bool operator>=(const Height &rhs) const noexcept
  .. cpp:function:: Height &operator+=(const Height &other)
  .. cpp:function:: Height &operator+=(const signed long &rhs)
  .. cpp:function:: const Height operator+(const Height &rhs) const
  .. cpp:function:: const Height operator+(const signed long &rhs) const
  .. cpp:function:: const Height operator-(const Height &rhs) const
  .. cpp:function:: bool operator==(const Height &rhs) const
  .. cpp:function:: bool operator!=(const Height &rhs) const
  .. cpp:function:: std::string format() const
  .. cpp:function:: bool isBottom() const
  .. cpp:function:: bool isTop() const
  .. cpp:function:: static Height meet(const Height &lhs, const Height &rhs)
  .. cpp:function:: static Height meet(std::set<Height> &ins)
  .. cpp:member:: private Height_t height_
  .. cpp:member:: private Type type_

.. cpp:enum:: StackAnalysis::Height::Type

  .. cpp:enumerator:: TOP
  .. cpp:enumerator:: BOTTOM
  .. cpp:enumerator:: HEIGHT


.. cpp:class:: StackAnalysis::DefHeight

  During stack pointer analysis, we keep track of any stack pointers in registers or
  memory, as well as the instruction addresses at which those pointers were
  defined. This is useful for :cpp:class:`StackMod`, where we sometimes want to modify
  pointer definitions to adjust the locations of variables on the stack.
  Thus, it makes sense to associate each stack pointer (:cpp:class:`StackAnalysis::Height`) to
  the point at which it was defined (:cpp:class:`StackAnalysis::Definition`).

  .. cpp:function:: DefHeight(const Definition &d, const Height &h)
  .. cpp:function:: bool operator==(const DefHeight &other) const
  .. cpp:function:: bool operator<(const DefHeight &other) const
  .. cpp:member:: Definition def
  .. cpp:member:: Height height

.. cpp:class:: StackAnalysis::DefHeightSet

  In some programs, it is possible for a register or memory location to
  contain different stack pointers depending on the path taken to the
  current instruction.  When this happens, our stack pointer analysis tries
  to keep track of the different possible stack pointers, up to a maximum
  number per instruction (specified by the DEF_LIMIT constant).  As a
  result, we need a structure to hold sets of DefHeights.

  .. cpp:function:: bool operator==(const DefHeightSet &other) const
  .. cpp:function:: std::set<DefHeight>::iterator begin()

    Returns an iterator to the set of DefHeights

  .. cpp:function:: std::set<DefHeight>::const_iterator begin() const

    Returns a constant iterator to the set of DefHeights

  .. cpp:function:: std::set<DefHeight>::iterator end()

    Returns an iterator to the end of the set of DefHeights

  .. cpp:function:: std::set<DefHeight>::const_iterator end() const

    Returns a constant iterator to the end of the set of DefHeights

  .. cpp:function:: std::set<DefHeight>::size_type size() const

    Returns the size of this set

  .. cpp:function:: void insert(const DefHeight &dh)

    Inserts a DefHeight into this set

  .. cpp:function:: bool isTopSet() const

    Returns true if this DefHeightSet is TOP

  .. cpp:function:: bool isBottomSet() const

    Returns true if this DefHeightSet is BOTTOM

  .. cpp:function:: void makeTopSet()

    Sets this DefHeightSet to TOP

  .. cpp:function:: void makeBottomSet()

    Sets this DefHeightSet to BOTTOM

  .. cpp:function:: void makeNewSet(ParseAPI::Block *b, Address addr, const Absloc &origLoc, const Height &h)

    Populates this DefHeightSet with the corresponding information

  .. cpp:function:: void addInitSet(const Height &h)

    Adds to this DefHeightSet a new definition with height h

  .. cpp:function:: void addDeltaSet(long delta)

    Updates all Heights in this set by the delta amount

  .. cpp:function:: Height getHeightSet() const

    Returns the result of computing a meet on all Heights in this set

  .. cpp:function:: Definition getDefSet() const

    Returns the result of computing a meet on all Definitions in this set

  .. cpp:member:: private std::set<DefHeight> defHeights


.. cpp:class:: StackAnalysis::TransferFunc

  We need to represent the effects of instructions. We do this in terms of
  transfer functions. We recognize the following effects on the stack.

    1) Offset by known amount: push/pop/etc.
    2) Set to known value: leave
    3) Copy the stack pointer to/from some Absloc.

  There are also:

    1) Offset by unknown amount expressible in a range [l, h]
    2) Set to unknown value expressible in a range [l, h] which we don't handle yet.

  This gives us the following transfer functions.

    1) Delta(RV, f, t, v) -> RV[f] += v;
    2) Abs(RV, f, t, v) -> RV[f] = v;
    3) Copy(RV, f, t, v) -> RV[t] = RV[f];

  In the implementations below, we provide f, t, v at construction time (as
  they are fixed) and RV as a parameter. Note that a transfer function is a
  function T : (RegisterVector, RegisterID, RegisterID, value) -> (RegisterVector).

  .. cpp:member:: static const long uninitialized = MAXLONG
  .. cpp:member:: static const long notUnique = MINLONG
  .. cpp:member:: static const TransferFunc top
  .. cpp:member:: static const TransferFunc bottom

  .. cpp:function:: TransferFunc()
  .. cpp:function:: TransferFunc(long a, long d, Absloc f, Absloc t, bool i = false, bool rt = false, Type type = OTHER)
  .. cpp:function:: TransferFunc(std::map<Absloc, std::pair<long, bool>> f, long d, Absloc t)
  .. cpp:function:: static TransferFunc identityFunc(Absloc r)
  .. cpp:function:: static TransferFunc deltaFunc(Absloc r, long d)
  .. cpp:function:: static TransferFunc absFunc(Absloc r, long a, bool i = false)
  .. cpp:function:: static TransferFunc copyFunc(Absloc f, Absloc t, bool i = false)
  .. cpp:function:: static TransferFunc bottomFunc(Absloc r)
  .. cpp:function:: static TransferFunc retopFunc(Absloc r)
  .. cpp:function:: static TransferFunc sibFunc(std::map<Absloc, std::pair<long, bool>> f, long d, Absloc t)
  .. cpp:function:: static TransferFunc meet(const TransferFunc &lhs, const TransferFunc &rhs)
  .. cpp:function:: bool isBaseRegCopy() const
  .. cpp:function:: bool isBaseRegSIB() const
  .. cpp:function:: bool isIdentity() const
  .. cpp:function:: bool isBottom() const
  .. cpp:function:: bool isTop() const
  .. cpp:function:: bool isRetop() const
  .. cpp:function:: bool isAbs() const
  .. cpp:function:: bool isCopy() const
  .. cpp:function:: bool isDelta() const
  .. cpp:function:: bool isSIB() const
  .. cpp:function:: bool isTopBottom() const
  .. cpp:function:: bool operator==(const TransferFunc &rhs) const
  .. cpp:function:: bool operator!=(const TransferFunc &rhs) const
  .. cpp:function:: DefHeightSet apply(const AbslocState &inputs) const

    Destructive update of the input map. Assumes inputs are absolute, uninitialized, or bottom; no deltas.

  .. cpp:function:: void accumulate(std::map<Absloc, TransferFunc> &inputs)

    Accumulation to the input map. This is intended to create a summary, so we
    create something that can take further input.

  .. cpp:function:: TransferFunc summaryAccumulate(const std::map<Absloc, TransferFunc> &inputs) const

    Returns accumulated transfer function without modifying inputs

  .. cpp:function:: std::string format() const
  .. cpp:function:: Type type() const
  .. cpp:member:: Absloc from
  .. cpp:member:: Absloc target
  .. cpp:member:: long delta
  .. cpp:member:: long abs
  .. cpp:member:: private bool retop

      Distinguish between default-constructed transfer functions and explicitly-retopped transfer
      functions.

  .. cpp:member:: private bool topBottom

    Annotate transfer functions that have the following characteristic:

    if target is TOP, keep as TOP
    else, target must be set to BOTTOM
    e.g., sign-extending a register:

      if the register had an uninitialized stack height (TOP), the sign-extension has no effect
      if the register had a valid or notunique (BOTTOM) stack height, the sign-extension must result in a BOTTOM stack height

  .. cpp:member:: std::map<Absloc, std::pair<long, bool>> fromRegs

    Handle complex math from SIB functions

  .. cpp:member:: private Type type_


.. cpp:enum:: StackAnalysis::TransferFunc::Type

  .. cpp:enumerator:: TOP
  .. cpp:enumerator:: BOTTOM
  .. cpp:enumerator:: OTHER


.. cpp:class:: StackAnalysis::SummaryFunc

  Summarize the effects of a series (list!) of transfer functions.
  Intended to summarize a block. We may want to do a better job of
  summarizing, but this works...

  .. cpp:member:: static const long uninitialized = MAXLONG
  .. cpp:member:: static const long notUnique = MINLONG
  .. cpp:function:: SummaryFunc()
  .. cpp:function:: void apply(ParseAPI::Block *block, const AbslocState &in, AbslocState &out) const
  .. cpp:function:: void accumulate(const TransferSet &in, TransferSet &out) const
  .. cpp:function:: std::string format() const
  .. cpp:function:: void validate() const
  .. cpp:function:: void add(TransferFuncs &f)
  .. cpp:function:: void addSummary(const TransferSet &summary)
  .. cpp:member:: TransferSet accumFuncs


.. cpp:type:: std::map<Absloc, DefHeightSet> AbslocState
.. cpp:type:: std::list<TransferFunc> TransferFuncs
.. cpp:type:: std::map<Absloc, TransferFunc> TransferSet
.. cpp:type:: std::map<Offset, AbslocState> StateIntervals
.. cpp:type:: std::map<ParseAPI::Block *, StateIntervals> Intervals
.. cpp:type:: std::map<ParseAPI::Function *, Height> FuncCleanAmounts
.. cpp:type:: std::map<ParseAPI::Block *, SummaryFunc> BlockEffects
.. cpp:type:: std::map<ParseAPI::Block *, AbslocState> BlockState
.. cpp:type:: std::map<ParseAPI::Block *, TransferSet> BlockSummaryState
.. cpp:type:: std::map<ParseAPI::Block *, std::map<Offset, TransferFuncs>> InstructionEffects

    To build intervals, we must replay the effect of each instruction. To avoid sucking
    enormous time, we keep those transfer functions around.

.. cpp:type:: std::map<ParseAPI::Block *, std::map<Offset, TransferSet>> CallEffects


.. cpp:class:: StateEvalVisitor : public Visitor

  Visitor class to evaluate stack heights and PC-relative addresses

  .. cpp:function:: StateEvalVisitor(Address addr, Instruction insn, StackAnalysis::AbslocState *s)

    addr is the starting address of instruction insn. insn is the instruction containing the expression to evaluate.

  .. cpp:function:: StateEvalVisitor()
  .. cpp:function:: bool isDefined()
  .. cpp:function:: std::pair<Address, bool> getResult()
  .. cpp:function:: virtual void visit(BinaryFunction *bf)
  .. cpp:function:: virtual void visit(Immediate *imm)
  .. cpp:function:: virtual void visit(RegisterAST *rast)
  .. cpp:function:: virtual void visit(Dereference *)
  .. cpp:member:: private bool defined
  .. cpp:member:: private StackAnalysis::AbslocState *state
  .. cpp:member:: private Address rip
  .. cpp:member:: private std::deque<std::pair<Address, bool>> results

    Stack for calculations bool is true if the value in Address is a stack height



Intervals
*********

The results of the stack analysis is a series of intervals. For each interval we have the
following information:

  1. Whether the function has a well-defined stack frame.
  2. The "depth" of the stack; the distance between the stack pointer and the caller's stack pointer.
  3. The "depth" of any copies of the stack pointer.

A well-defined stack frame is defined as follows:

  1. x86/AMD-64: a frame pointer
  2. POWER: an allocated frame pointed to by GPR1
