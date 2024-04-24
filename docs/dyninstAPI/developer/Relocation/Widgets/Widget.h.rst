.. _`sec:Widget.h`:

Widget.h
########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: Widget

  Widget code generation class

  .. cpp:type:: boost::shared_ptr<Widget> Ptr
  .. cpp:type:: boost::shared_ptr<RelocBlock> RelocBlockPtr
  .. cpp:function:: Widget()
  .. cpp:function:: virtual Address addr() const

    A default value to make sure things don't go wonky.

  .. cpp:function:: virtual unsigned size() const
  .. cpp:function:: virtual InstructionAPI::Instruction insn() const
  .. cpp:function:: virtual bool generate(const codeGen &templ, const RelocBlock *trace, CodeBuffer &buffer) = 0

    Make binary from the thing Current address (if we need it) is in the codeGen object.

  .. cpp:function:: virtual std::string format() const = 0
  .. cpp:function:: virtual ~Widget()


.. cpp:struct:: Patch

  A generic code patching mechanism

  .. cpp:function:: virtual bool apply(codeGen &gen, CodeBuffer *buf) = 0
  .. cpp:function:: virtual unsigned estimate(codeGen &templ) = 0
  .. cpp:function:: virtual ~Patch()
