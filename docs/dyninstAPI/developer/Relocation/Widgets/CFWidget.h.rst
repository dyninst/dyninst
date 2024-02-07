.. _`sec:CFWidget.h`:

CFWidget.h
##########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: CFWidget : public Widget

  .. cpp:member:: static const Address Fallthrough
  .. cpp:member:: static const Address Taken
  .. cpp:type:: boost::shared_ptr<CFWidget> Ptr
  .. cpp:type:: std::map<Address, TargetInt *> DestinationMap
  .. cpp:function:: static Ptr create(Address addr)
  .. cpp:function:: static Ptr create(const Widget::Ptr info)
  .. cpp:function:: bool generate(const codeGen &templ, const RelocBlock *, CodeBuffer &buffer)
  .. cpp:function:: virtual ~CFWidget()
  .. cpp:function:: void addDestination(Address index, TargetInt *dest)

    Owns the provided dest parameter

  .. cpp:function:: TargetInt *getDestination(Address dest) const
  .. cpp:function:: const DestinationMap &destinations() const
  .. cpp:function:: virtual std::string format() const
  .. cpp:function:: virtual Address addr() const
  .. cpp:function:: virtual InstructionAPI::Instruction insn() const
  .. cpp:function:: void setGap(unsigned gap)
  .. cpp:function:: void setOrigTarget(Address a)
  .. cpp:function:: unsigned gap() const
  .. cpp:function:: void clearIsCall()
  .. cpp:function:: void clearIsIndirect()
  .. cpp:function:: void clearIsConditional()
  .. cpp:function:: bool isCall() const
  .. cpp:function:: bool isIndirect() const
  .. cpp:function:: bool isConditional() const
  .. cpp:function:: private CFWidget(Address a)
  .. cpp:function:: private CFWidget(InstructionAPI::Instruction insn, Address addr)
  .. cpp:function:: private TrackerElement *tracker(const RelocBlock *) const
  .. cpp:function:: private TrackerElement *destTracker(TargetInt *dest, const RelocBlock *) const
  .. cpp:function:: private TrackerElement *addrTracker(Address addr, const RelocBlock *) const
  .. cpp:function:: private TrackerElement *padTracker(Address addr, unsigned size, const RelocBlock *) const
  .. cpp:member:: private bool isCall_

    These are not necessarily mutually exclusive. See also: PPC conditional linking indirect branch, oy.

  .. cpp:member:: private bool isConditional_
  .. cpp:member:: private bool isIndirect_
  .. cpp:member:: private unsigned gap_
  .. cpp:member:: private InstructionAPI::Instruction insn_
  .. cpp:member:: private Address addr_
  .. cpp:member:: private Address origTarget_

    If we were a PC-relative indirect store that data here

  .. cpp:member:: private DestinationMap destMap_

    A map from input values (for some representation of input values) to Targets Used during code generation
    to determine whether we require some form of address translation. We currently have two cases: conditional
    and indirect control flow.  Conditional: <true> -> taken target <false> -> fallthrough target
    Indirect: <original address> -> corresponding target TBD: PPC has conditional indirect control flow, so we
    may want to split these up.

  .. cpp:function:: private bool generateBranch(CodeBuffer &gens, TargetInt *to, InstructionAPI::Instruction insn,\
                                                const RelocBlock *trace, bool fallthrough)

    These should move to a CodeGenerator class or something... But for now they can go here.

    The Instruction input allows pulling out ancillary data (e.g., conditions, prediction, etc.

  .. cpp:function:: private bool generateCall(CodeBuffer &gens, TargetInt *to, const RelocBlock *trace,\
                                              InstructionAPI::Instruction insn)

  .. cpp:function:: private bool generateConditionalBranch(CodeBuffer &gens, TargetInt *to, const RelocBlock *trace,\
                                                           InstructionAPI::Instruction insn)

  .. cpp:type:: private unsigned Register

    The Register holds the translated destination (if any)

  .. cpp:function:: private bool generateIndirect(CodeBuffer &gens, Register reg, const RelocBlock *trace, InstructionAPI::Instruction insn)
  .. cpp:function:: private bool generateIndirectCall(CodeBuffer &gens, Register reg, InstructionAPI::Instruction insn, const RelocBlock *trace, Address origAddr)


.. cpp:struct:: CFPatch : public Patch

  .. cpp:function:: private CFPatch(Type a, InstructionAPI::Instruction b, TargetInt *c, const func_instance *d,\
                                    Address e = 0)

  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~CFPatch()
  .. cpp:member:: private Type type
  .. cpp:member:: private InstructionAPI::Instruction orig_insn
  .. cpp:member:: private TargetInt *target
  .. cpp:member:: private const func_instance *func
  .. cpp:member:: private Address origAddr_
  .. cpp:member:: private arch_insn *ugly_insn
  .. cpp:member:: private unsigned char* insn_ptr
  .. cpp:function:: private bool needsTOCUpdate()

    64-bit PPCLinux has a TOC register we need to maintain. That puts it in "special case" territory...

  .. cpp:function:: private bool handleTOCUpdate(codeGen &gen)
  .. cpp:function:: private bool isPLT(codeGen &gen)
  .. cpp:function:: private bool applyPLT(codeGen &gen, CodeBuffer *buf)


.. cpp:enum:: CFPatch::Type

  .. cpp:enumerator:: Jump
  .. cpp:enumerator:: JCC
  .. cpp:enumerator:: Call
  .. cpp:enumerator:: Data

    RIP-relative expression for the destination


.. cpp:struct:: PaddingPatch : public Patch

  .. cpp:function:: private PaddingPatch(unsigned size, bool registerDefensive, bool noop, block_instance *b)

    For Kevin's defensive Dyninst, we want to append a padding area past the return point of calls that don't
    necessarily return to the normal places. This requires both a) an empty space in code gen and b) tracking
    that address in the process. The first is easy enough to do statically, but the second requires a patch so
    that we get notified of address finickiness.

  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~PaddingPatch()
  .. cpp:member:: private unsigned size_
  .. cpp:member:: private bool registerDefensive_
  .. cpp:member:: private bool noop_
  .. cpp:member:: private block_instance *block_
