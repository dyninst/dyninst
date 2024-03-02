.. _`sec:dyninstAPI:codegen.h`:

codegen.h
#########


.. cpp:class:: codeGen

  Code generation This class wraps the actual code generation mechanism: we keep a buffer and a pointer to where
  we are in the buffer. This may vary by platform, hence the typedef wrappers.

  .. cpp:function:: codeGen()

    Default constructor -- makes an empty generation area

  .. cpp:function:: codeGen(unsigned size)

    Make a generation buffer of ``size`` *bytes*.

  .. cpp:function:: codeGen(codeBuf_t *buf, int size)

    Use a preallocated buffer

  .. cpp:function:: ~codeGen()
  .. cpp:function:: bool valid()
  .. cpp:function:: bool verify()
  .. cpp:function:: codeGen(const codeGen &)

    Copy constructor. Deep-copy -- allocates a new buffer

  .. cpp:function:: bool operator==(void *ptr) const

    We consider our pointer to either be the start of the buffer, or NULL if the buffer is empty

  .. cpp:function:: bool operator!=(void *ptr) const
  .. cpp:function:: codeGen &operator=(const codeGen &param)
  .. cpp:function:: void applyTemplate(const codeGen &codeTemplate)

    Initialize the current using the argument as a "template"

  .. cpp:member:: static codeGen baseTemplate
  .. cpp:function:: void allocate(unsigned)

    Allocate a certain amount of space

  .. cpp:function:: void invalidate()

    And invalidate

  .. cpp:function:: void finalize()

    Finally, tighten down the memory usage. This frees the buffer if it's bigger than necessary and copies everything
    to a new fixed buffer.

  .. cpp:function:: void copy(const void *buf, const unsigned size)

    Copy a buffer into here and move the offset

  .. cpp:function:: void copy(const void *buf, const unsigned size, const codeBufIndex_t index)
  .. cpp:function:: void copy(const std::vector<unsigned char> &buf)
  .. cpp:function:: void insert(const void *buf, const unsigned size, const codeBufIndex_t index)

    Insert buffer into index, moving previous content

  .. cpp:function:: void copyAligned(const void *buf, const unsigned size)

    Workaround for copying strings on word-aligned platforms

  .. cpp:function:: void copy(codeGen &gen)

    Similar, but slurp from the start of the parameter

  .. cpp:function:: unsigned used() const

    How much space are we using?

  .. cpp:function:: unsigned size() const
  .. cpp:function:: unsigned max() const
  .. cpp:function:: void *start_ptr() const

    Blind pointer to the start of the code area

  .. cpp:function:: void *cur_ptr() const

    With ptr() and used() you can copy into the mutatee. Pointer to the current location...

  .. cpp:function:: void *get_ptr(unsigned offset) const

    And pointer to a given offset

  .. cpp:function:: void update(codeBuf_t *ptr)

    For things that make a copy of the current pointer and play around with it. This recalculates the current offset based on a new pointer

  .. cpp:function:: void setIndex(codeBufIndex_t offset)

    Set the offset at a particular location.

  .. cpp:function:: codeBufIndex_t getIndex() const
  .. cpp:function:: void moveIndex(int disp)

    Move up or down a certain amount

  .. cpp:function:: static long getDisplacement(codeBufIndex_t from, codeBufIndex_t to)

    To calculate a jump between the "from" and where we are

  .. cpp:function:: Dyninst::Address currAddr() const

    For code generation -- given the current state of generation and a base address in the mutatee, produce a "current" address.

  .. cpp:function:: Dyninst::Address currAddr(Dyninst::Address base) const
  .. cpp:function:: void fill(unsigned fillSize, int fillType)
  .. cpp:function:: void fillRemaining(int fillType)

    Since we have a known size

  .. cpp:function:: std::string format() const
  .. cpp:function:: void addPCRelRegion(pcRelRegion *reg)

    Add a new PCRelative region that should be generated after  addresses are fixed

  .. cpp:function:: void applyPCRels(Dyninst::Address addr)

    Have each region generate code with this codeGen object being  placed at addr

  .. cpp:function:: bool hasPCRels() const

    Return true if there are any active regions.

  .. cpp:function:: void addPatch(const relocPatch &p)

    Add a new patch point

  .. cpp:function:: void addPatch(codeBufIndex_t index, patchTarget *source, unsigned size = sizeof(Dyninst::Address), relocPatch::patch_type_t ptype = relocPatch::patch_type_t::abs, Dyninst::Offset off = 0)

    Create a patch into the codeRange

  .. cpp:function:: std::vector<relocPatch> &allPatches()
  .. cpp:function:: void applyPatches()

    Apply all patches that have been added

  .. cpp:function:: void setAddrSpace(AddressSpace *a)
  .. cpp:function:: void setThread(PCThread *t)
  .. cpp:function:: void setRegisterSpace(registerSpace *r)
  .. cpp:function:: void setAddr(Dyninst::Address a)
  .. cpp:function:: void setPoint(instPoint *i)
  .. cpp:function:: void setRegTracker(regTracker_t *t)
  .. cpp:function:: void setCodeEmitter(Emitter *emitter)
  .. cpp:function:: void setFunction(func_instance *f)
  .. cpp:function:: void setBT(baseTramp *i)
  .. cpp:function:: void setInInstrumentation(bool i)
  .. cpp:function:: unsigned width() const
  .. cpp:function:: AddressSpace *addrSpace() const
  .. cpp:function:: PCThread *thread()
  .. cpp:function:: Dyninst::Address startAddr() const
  .. cpp:function:: instPoint *point() const
  .. cpp:function:: baseTramp *bt() const
  .. cpp:function:: func_instance *func() const
  .. cpp:function:: registerSpace *rs() const
  .. cpp:function:: regTracker_t *tracker() const
  .. cpp:function:: Emitter *codeEmitter() const
  .. cpp:function:: Emitter *emitter() const
  .. cpp:function:: bool inInstrumentation() const
  .. cpp:function:: bool insertNaked() const
  .. cpp:function:: void setInsertNaked(bool i)
  .. cpp:function:: bool modifiedStackFrame() const
  .. cpp:function:: void setModifiedStackFrame(bool i)
  .. cpp:function:: Dyninst::Architecture getArch() const
  .. cpp:function:: void beginTrackRegDefs()
  .. cpp:function:: void endTrackRegDefs()
  .. cpp:function:: const bitArray &getRegsDefined()
  .. cpp:function:: void markRegDefined(Dyninst::Register r)
  .. cpp:function:: bool isRegDefined(Dyninst::Register r)
  .. cpp:function:: void setPCRelUseCount(int c)
  .. cpp:function:: int getPCRelUseCount() const
  .. cpp:type:: std::pair<Dyninst::Address, unsigned> Extent

    SD-DYNINST

  .. cpp:function:: void registerDefensivePad(block_instance *, Dyninst::Address, unsigned)
  .. cpp:function:: std::map<block_instance *, Extent> &getDefensivePads()
  .. cpp:function:: void registerInstrumentation(baseTramp *bt, Dyninst::Address loc)

    Immediate uninstrumentation

  .. cpp:function:: std::map<baseTramp *, Dyninst::Address> &getInstrumentation()
  .. cpp:function:: void registerRemovedInstrumentation(baseTramp *bt, Dyninst::Address loc)
  .. cpp:function:: std::map<baseTramp *, Dyninst::Address> &getRemovedInstrumentation()
  .. cpp:function:: private void realloc(unsigned newSize)
  .. cpp:member:: private codeBuf_t *buffer_
  .. cpp:member:: private codeBufIndex_t offset_
  .. cpp:member:: private unsigned size_
  .. cpp:member:: private unsigned max_
  .. cpp:member:: private int pc_rel_use_count
  .. cpp:member:: private Emitter *emitter_
  .. cpp:member:: private bool allocated_
  .. cpp:member:: private AddressSpace *aSpace_
  .. cpp:member:: private PCThread *thr_
  .. cpp:member:: private registerSpace *rs_
  .. cpp:member:: private regTracker_t *t_
  .. cpp:member:: private Dyninst::Address addr_
  .. cpp:member:: private instPoint *ip_
  .. cpp:member:: private func_instance *f_
  .. cpp:member:: private baseTramp *bt_
  .. cpp:member:: private bool isPadded_
  .. cpp:member:: private bitArray regsDefined_
  .. cpp:member:: private bool trackRegDefs_
  .. cpp:member:: private bool inInstrumentation_
  .. cpp:member:: private bool insertNaked_
  .. cpp:member:: private bool modifiedStackFrame_
  .. cpp:member:: private std::vector<relocPatch> patches_
  .. cpp:member:: private std::vector<pcRelRegion *> pcrels_
  .. cpp:member:: private std::map<block_instance *, Extent> defensivePads_
  .. cpp:member:: private std::map<baseTramp *, Dyninst::Address> instrumentation_
  .. cpp:member:: private std::map<baseTramp *, Dyninst::Address> removedInstrumentation_


.. cpp:enum:: codeGen::@type 

  .. cpp:enumerator:: cgNOP
  .. cpp:enumerator:: cgTrap
  .. cpp:enumerator:: cgIllegal
