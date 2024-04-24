.. _`sec:ArchSpecificFormatters.h`:

ArchSpecificFormatters.h
########################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: ArchSpecificFormatter

  .. cpp:function:: virtual std::string getInstructionString(const std::vector <std::string>&) const
  .. cpp:function:: virtual std::string formatImmediate(const std::string&) const = 0
  .. cpp:function:: virtual std::string formatDeref(const std::string&)  const= 0
  .. cpp:function:: virtual std::string formatRegister(const std::string&)  const= 0
  .. cpp:function:: virtual std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const
  .. cpp:function:: virtual bool operandPrintOrderReversed() const
  .. cpp:function:: virtual ~ArchSpecificFormatter() = default
  .. cpp:function:: ArchSpecificFormatter& operator=(const ArchSpecificFormatter&) = default
  .. cpp:function:: static ArchSpecificFormatter& getFormatter(Dyninst::Architecture a)

.. cpp:class:: PPCFormatter : public ArchSpecificFormatter

  .. cpp:function:: PPCFormatter()
  .. cpp:function:: std::string formatImmediate(const std::string&) const override
  .. cpp:function:: std::string formatDeref(const std::string&) const override
  .. cpp:function:: std::string formatRegister(const std::string&) const override
  .. cpp:function:: std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override

.. cpp:class:: ArmFormatter : public ArchSpecificFormatter

  .. cpp:function:: ArmFormatter()
  .. cpp:function:: std::string formatImmediate(const std::string&) const override
  .. cpp:function:: std::string formatDeref(const std::string&) const override
  .. cpp:function:: std::string formatRegister(const std::string&) const override
  .. cpp:function:: std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override

.. cpp:class:: AmdgpuFormatter : public ArchSpecificFormatter

  .. cpp:function:: AmdgpuFormatter()
  .. cpp:function:: std::string formatImmediate(const std::string&) const override
  .. cpp:function:: std::string formatDeref(const std::string&) const override
  .. cpp:function:: std::string formatRegister(const std::string&) const override
  .. cpp:function:: std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override
  .. cpp:function:: static std::string formatRegister(MachRegister m_Reg, uint32_t num_elements, unsigned m_Low , unsigned m_High )

      Helper function for formatting consecutive registers that are displayed as a single operand
      Called when architecture is passed to Instruction.format.

.. cpp:class:: x86Formatter : public ArchSpecificFormatter

  .. cpp:function:: x86Formatter()
  .. cpp:function:: std::string getInstructionString(const std::vector <std::string>&) const override
  .. cpp:function:: std::string formatImmediate(const std::string&) const override
  .. cpp:function:: std::string formatDeref(const std::string&) const override
  .. cpp:function:: std::string formatRegister(const std::string&) const override
  .. cpp:function:: std::string formatBinaryFunc(const std::string&, const std::string&, const std::string&) const override
  .. cpp:function:: bool operandPrintOrderReversed() const override
