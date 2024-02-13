.. _`sec-dev:liveness.h`:

liveness.h
##########

.. cpp:namespace:: dev


.. cpp:class:: LivenessAnalyzer

  .. cpp:member:: private std::map<ParseAPI::Block*, livenessData> blockLiveInfo
  .. cpp:member:: private std::map<ParseAPI::Function*, bool> liveFuncCalculated
  .. cpp:member:: private std::map<ParseAPI::Function*, bitArray> funcRegsDefined
  .. cpp:member:: private InstructionCache cachedLivenessInfo
  .. cpp:function:: private const bitArray& getLivenessIn(ParseAPI::Block *block)
  .. cpp:function:: private const bitArray& getLivenessOut(ParseAPI::Block *block, bitArray &allRegsDefined)
  .. cpp:function:: private void processEdgeLiveness(ParseAPI::Edge* e, livenessData& data, ParseAPI::Block* block, const bitArray& allRegsDefined)
  .. cpp:function:: private void summarizeBlockLivenessInfo(ParseAPI::Function* func, ParseAPI::Block *block, bitArray &allRegsDefined)
  .. cpp:function:: private bool updateBlockLivenessInfo(ParseAPI::Block *block, bitArray &allRegsDefined)

      This is used to do fixed point iteration until the in and out don't change anymore.

  .. cpp:function:: private ReadWriteInfo calcRWSets(Instruction curInsn, ParseAPI::Block *blk, Address a)
  .. cpp:function:: private void* getPtrToInstruction(ParseAPI::Block *block, Address addr) const
  .. cpp:function:: private bool isExitBlock(ParseAPI::Block *block)
  .. cpp:function:: private bool isMMX(MachRegister machReg)
  .. cpp:function:: private MachRegister changeIfMMX(MachRegister machReg)
  .. cpp:member:: private int width
  .. cpp:member:: private ABI* abi

  .. cpp:member:: private ErrorType errorno

