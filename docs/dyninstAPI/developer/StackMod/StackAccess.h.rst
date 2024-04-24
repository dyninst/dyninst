.. _`sec:StackAccess.h`:

StackAccess.h
#############

.. cpp:class:: StackAccess

  .. cpp:function:: static std::string printStackAccessType(StackAccessType t)
  .. cpp:function:: StackAccess()
  .. cpp:function:: ~StackAccess()
  .. cpp:function:: MachRegister reg() const
  .. cpp:function:: void setReg(MachRegister r)
  .. cpp:function:: StackAnalysis::Height regHeight() const
  .. cpp:function:: void setRegHeight(const StackAnalysis::Height &h)
  .. cpp:function:: StackAnalysis::Definition regDef() const
  .. cpp:function:: void setRegDef(const StackAnalysis::Definition &d)
  .. cpp:function:: StackAnalysis::Height readHeight() const
  .. cpp:function:: void setReadHeight(const StackAnalysis::Height &h)
  .. cpp:function:: StackAccessType type() const
  .. cpp:function:: void setType(StackAccessType t)
  .. cpp:function:: signed long disp() const
  .. cpp:function:: void setDisp(signed long d)
  .. cpp:function:: bool skipReg() const
  .. cpp:function:: void setSkipReg(bool s)
  .. cpp:function:: std::string format()
  .. cpp:member:: private MachRegister _reg
  .. cpp:member:: private StackAnalysis::Height _regHeight
  .. cpp:member:: private StackAnalysis::Definition _regDef
  .. cpp:member:: private StackAnalysis::Height _readHeight
  .. cpp:member:: private StackAccessType _type
  .. cpp:member:: private signed long _disp
  .. cpp:member:: private bool _skipReg


.. cpp:enum:: StackAccess::StackAccessType

  .. cpp:enumerator:: DEBUGINFO_LOCAL
  .. cpp:enumerator:: DEBUGINFO_PARAM
  .. cpp:enumerator:: SAVED
  .. cpp:enumerator:: WRITE
  .. cpp:enumerator:: UNKNOWN
  .. cpp:enumerator:: READ
  .. cpp:enumerator:: READWRITE
  .. cpp:enumerator:: REGHEIGHT
  .. cpp:enumerator:: DEFINITION
  .. cpp:enumerator:: MISUNDERSTOOD

.. cpp:type:: std::map<MachRegister, std::set<StackAccess*> > Accesses

.. cpp:function:: bool isDebugType(StackAccess::StackAccessType t)
.. cpp:function:: int getAccessSize(InstructionAPI::Instruction insn)
.. cpp:function:: bool getAccesses(ParseAPI::Function *func, ParseAPI::Block *block, Address addr, InstructionAPI::Instruction insn, Accesses *accesses, std::set<Address> &defPointsToMod, bool analyzeDefinition = false)
.. cpp:function:: bool getMemoryOffset(ParseAPI::Function *func, ParseAPI::Block *block, InstructionAPI::Instruction insn, Address addr, const MachRegister &reg, const StackAnalysis::Height &height, const StackAnalysis::Definition &def, StackAccess *&ret, Architecture arch, bool analyzeDefintion = false)
