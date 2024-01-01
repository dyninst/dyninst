.. _`sec:Register.h`:

Register.h
##########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: RegisterAST : public Expression

  **A register contained in an operand**

  .. Note:: This class satisfies the `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:type:: boost::shared_ptr<RegisterAST> Ptr

  .. cpp:function:: RegisterAST(MachRegister r)

      Creates a register based on the machine register ``r``.

  .. cpp:function:: RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, uint32_t num_elements = 1)

  .. cpp:function:: RegisterAST(MachRegister r, unsigned int lowbit, unsigned int highbit, Result_Type regType, uint32_t num_elements = 1)

  .. cpp:function:: void getChildren(vector<InstructionAST::Ptr>& children) const

      Does nothing because a register has no children.

  .. cpp:function:: virtual void getChildren(vector<Expression::Ptr>& children) const

      Does nothing because a register has no children.

  .. cpp:function:: void getUses(set<InstructionAST::Ptr>& uses)

      Appends the set of used registers to ``uses``.

      By definition, the use set of a register is only itself.

  .. cpp:function:: bool isUsed(InstructionAST::Ptr findMe) const

      Checks if ``i`` represents the same register.

  .. cpp:function:: std::string format(formatStyle how = defaultStyle) const

      Returns a string representation of this expression using the style ``formatStyle``.

  .. cpp:function:: static RegisterAST makePC(Dyninst::Architecture arch)

      Returns the program counter (PC) for the architecture ``arch``.

  .. cpp:function:: MachRegister getID() const

      Returns the underlying register represented by this AST.

  .. cpp:function:: unsigned int lowBit() const
  .. cpp:function:: unsigned int highBit() const

  .. cpp:function:: static RegisterAST::Ptr promote(const InstructionAST::Ptr reg)

      Hides aliasing complexity on platforms that allow addressing part or all of a register.

  .. cpp:function:: static RegisterAST::Ptr promote(const RegisterAST* reg)

    Hides aliasing complexity on platforms that allow addressing part or all of a register.

  .. cpp:function:: virtual void apply(Visitor* v)

    Applies ``v``.

  .. cpp:function:: virtual bool bind(Expression* e, const Result& val)

    Forwards to :cpp:func:`Expression::bind`.

