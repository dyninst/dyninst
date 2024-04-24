.. _`sec:CodeBuffer.h`:

CodeBuffer.h
############

Infrastructure for the code generating Widgets; try to automate
as much of the handling as we can.

Background
  An Widget can generate two types of code: PIC and non-PIC. PIC code can be treated
  as a sequence of bytes that can be freely copied around, whereas non-PIC code must
  be tweaked whenever its address changes as part of code generation. Note, it is safe
  treat PIC code as non-PIC (though less than optimal); the reverse is not true. For
  efficiency we want to bundle PIC code together and avoid re-generating it in every pass,
  while keeping callbacks for non-PIC code.

  Another requirement of this system is to build a tracking data structure that maps from
  address in the generated code to who generated that code, so we can maintain a mapping
  between original addresses and moved (relocated) code. We also wish to do this automatically.

  Thus, we provide an interface that automates these two functions as much as possible. We provide
  two ways of specifying code: copy and patch. Copy handles PIC code, and pulls in a buffer of bytes
  (e.g., memcpy). Patch registers a callback for later that will provide some code.

  With these two methods the user can specify a tracking data structure that specifies what kind
  of code they have provided.

  The input grammar to the CodeBuffer is ``[PIC|nonPIC]*``. However, it is safe to accumulate
  between adjacent PICs, so we can simplify this to ``((PIC*)nonPIC)*``. This drives our internal
  storage, which is a list of ``(buffer, patch)`` pairs; the buffer contains ``(PIC*)`` and the
  patch represents a nonPIC entry. For each pair, we also associate a list of code trackers,
  since there is a 1:1 relationship between each element ``(PIC, nonPIC)`` and a tracker.


.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: CodeBuffer

  .. cpp:type:: std::vector<unsigned char> Buffer
  .. cpp:function:: CodeBuffer()
  .. cpp:function:: ~CodeBuffer()
  .. cpp:function:: void initialize(const codeGen &templ, unsigned numBlocks)
  .. cpp:function:: unsigned getLabel()
  .. cpp:function:: unsigned defineLabel(Address addr)
  .. cpp:function:: void addPIC(const unsigned char *input, unsigned size, TrackerElement *tracker)
  .. cpp:function:: void addPIC(const void *input, unsigned size, TrackerElement *tracker)
  .. cpp:function:: void addPIC(const codeGen &gen, TrackerElement *tracker)
  .. cpp:function:: void addPIC(const Buffer buf, TrackerElement *tracker)
  .. cpp:function:: void addPatch(Patch *patch, TrackerElement *tracker)
  .. cpp:function:: bool extractTrackers(CodeTracker *t)
  .. cpp:function:: unsigned size() const
  .. cpp:function:: void *ptr() const
  .. cpp:function:: bool generate(Address baseAddr)
  .. cpp:function:: void updateLabel(unsigned id, Address addr, bool &regenerate)
  .. cpp:function:: Address predictedAddr(unsigned labelID)
  .. cpp:function:: Address getLabelAddr(unsigned labelID)
  .. cpp:function:: void disassemble() const
  .. cpp:function:: codeGen &gen()
  .. cpp:function:: private BufferElement &current()
  .. cpp:type:: private std::list<BufferElement> Buffers
  .. cpp:member:: private Buffers buffers_
  .. cpp:member:: private unsigned size_
  .. cpp:member:: private codeGen gen_
  .. cpp:member:: private int curIteration_
  .. cpp:type:: private std::vector<Label> Labels
  .. cpp:member:: private Labels labels_
  .. cpp:member:: private int curLabelID_
  .. cpp:member:: private int shift_
  .. cpp:member:: private bool generated_


.. cpp:struct:: CodeBuffer::Label

  .. cpp:type:: private unsigned Id
  .. cpp:member:: private Type type
  .. cpp:member:: private Id id

  .. cpp:member:: private int iteration

    This is a bit of a complication. We want to estimate where a label will go
    pre-generation to try and reduce how many iterations we need to go through.
    Thus, addr may either be an absolute address, an offset, or an estimated address.

  .. cpp:member:: private Address addr
  .. cpp:member:: private static const unsigned INVALID
  .. cpp:function:: private Label() noexcept
  .. cpp:function:: private Label(Type a, Id b, Address c)
  .. cpp:function:: private bool valid()


.. cpp:enum:: CodeBuffer::Label::Type

  .. cpp:enumerator:: Invalid
  .. cpp:enumerator:: Absolute
  .. cpp:enumerator:: Relative
  .. cpp:enumerator:: Estimate


.. cpp:class:: CodeBuffer::BufferElement

  .. cpp:function:: BufferElement()
  .. cpp:function:: BufferElement(const BufferElement&) = delete
  .. cpp:function:: BufferElement(BufferElement&&)
  .. cpp:function:: ~BufferElement()
  .. cpp:function:: void setLabelID(unsigned id)
  .. cpp:function:: void addPIC(const unsigned char *input, unsigned size, TrackerElement *tracker)
  .. cpp:function:: void addPIC(const Buffer &buffer, TrackerElement *tracker)
  .. cpp:function:: void setPatch(Patch *patch, TrackerElement *tracker)
  .. cpp:function:: bool full()
  .. cpp:function:: bool empty()
  .. cpp:function:: bool generate(CodeBuffer *buf, codeGen &gen, int &shift, bool &regenerate)
  .. cpp:function:: bool extractTrackers(CodeTracker *t)
  .. cpp:function:: private BufferElement& operator=(BufferElement&) = default
  .. cpp:function:: private void addTracker(TrackerElement *tracker)
  .. cpp:member:: private Address addr_{}
  .. cpp:member:: private unsigned size_{}
  .. cpp:member:: private Buffer buffer_
  .. cpp:member:: private Patch *patch_{}
  .. cpp:member:: private unsigned labelID_{Label::INVALID}
  .. cpp:type:: private std::map<Offset, TrackerElement *> Trackers

    Here the Offset is an offset within the buffer, starting at 0.

  .. cpp:member:: private Trackers trackers_
