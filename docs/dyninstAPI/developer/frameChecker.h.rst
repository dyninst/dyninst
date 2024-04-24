.. _`sec:frameChecker.h`:

frameChecker.h
##############


.. cpp:class:: frameChecker

  .. cpp:function:: frameChecker(const unsigned char* addr, size_t max_length, Dyninst::Architecture arch)
  .. cpp:function:: virtual ~frameChecker()
  .. cpp:function:: bool isReturn() const
  .. cpp:function:: bool isStackPreamble() const
  .. cpp:function:: bool isStackFrameSetup() const
  .. cpp:function:: private bool isMovStackToBase(unsigned index_to_check) const
  .. cpp:member:: private std::vector<Dyninst::InstructionAPI::Instruction> m_Insns
  .. cpp:member:: private Dyninst::Architecture arch
