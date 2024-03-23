.. _`sec-dev:Register.h`:

Register.h
##########

.. cpp:namespace:: Dyninst::InstructionAPI::dev

.. cpp:class:: MaskRegisterAST : public RegisterAST

  **Mask register operands**

  This class is the same as RegisterAST except it handles the
  syntactial differences between register operands and mask register operands.

  .. cpp:function:: MaskRegisterAST(MachRegister r)
  .. cpp:function:: MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit)
  .. cpp:function:: MaskRegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType)
  .. cpp:function:: virtual std::string format(Architecture, formatStyle how = defaultStyle) const
  .. cpp:function:: virtual std::string format(formatStyle how = defaultStyle) const
