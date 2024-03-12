.. _`sec:registerSpace.h`:

registerSpace.h
###############

A class to retain information about where the original register can be found. It can be in one of the following states:

1. Unsaved, and available via the register itself;
2. Saved in a frame, e.g., a base tramp;
3. Pushed on the stack at a relative offset from the current stack pointer.
4. TODO: we could subclass this and make "get me the current value" a member function; not sure it's really worth it for the minimal amount of memory multiple types will use.

We also need a better way of tracking what state a register is in. Here's some possibilities, not at all mutually independent:

1. Live at the start of instrumentation, or dead;
2. Used during the generation of a subexpression
3. Currently reserved by another AST, but we could recalculate if necessary At a function call node, is it carrying a value?

**Terminology**

Live
  contains a value outside of instrumentation, and so must be saved before use

Used
  used by instrumentation code.


.. cpp:class:: RealRegister

  This is currently only used on x86_32 to represent the
  virtual/real register difference. :cpp:type:`Dyninst::Register` still refers
  to virtual registers on this platform.  Contained in a struct
  so that no one can accidently cast a :cpp:type:`Dyninst::Register` into a
  ``RealRegister``.

  .. cpp:member:: private signed int r
  .. cpp:function:: RealRegister()
  .. cpp:function:: explicit RealRegister(int reg)
  .. cpp:function:: int reg() const


.. cpp:class:: registerSlot

  .. cpp:member:: int alloc_num

    MATT TODO: Remove

  .. cpp:member::  const Dyninst::Register number
  .. cpp:member:: const std::string name
  .. cpp:member:: const initialLiveness_t initialState
  .. cpp:member:: bool offLimits

    Are we off limits for allocation in this particular instance?

  .. cpp:member:: const regType_t type

  ......

  .. rubric::
    Code generation
    
  .. cpp:member:: int refCount

    zero, if free

  .. cpp:member:: livenessState_t liveState
  .. cpp:member:: bool keptValue

    Are we keeping this (as long as we can) to save the pre-calculated value?

    Note: refCount can be 0 and this still set.

  .. cpp:member:: bool beenUsed

    Has this register been used by generated code?

  .. cpp:member:: spillReference_t spilledState

    New version of "if we were saved, then where?" It's a pair - truefalse, then offset from the "zeroed" stack pointer.

  .. cpp:member:: int saveOffset

    Offset where this register can be retrieved.

  ......

  .. cpp:function:: unsigned encoding() const

    AMD-64: this is the number of words
    POWER: this is the number of bytes I know it's inconsistent, but it's easier this way since POWER has some funky math.

  .. cpp:function:: void cleanSlot()
  .. cpp:function:: void markUsed(bool incRefCount)
  .. cpp:function:: void debugPrint(const char *str = NULL)
  .. cpp:function:: registerSlot()

    Don't want to use this...

  .. cpp:function:: registerSlot(Dyninst::Register num, std::string name_, bool offLimits_, initialLiveness_t initial, regType_t type_)


.. cpp:enum:: registerSlot::regType_tc

  .. cpp:enumerator:: invalid
  .. cpp:enumerator:: GPR
  .. cpp:enumerator:: FPR
  .. cpp:enumerator:: SPR
  .. cpp:enumerator:: realReg


.. cpp:enum:: registerSlot::livenessState_t

  .. cpp:enumerator:: live
  .. cpp:enumerator:: spilled
  .. cpp:enumerator:: dead


.. cpp:enum:: registerSlot::spillReference_t

  .. cpp:enumerator:: unspilled
  .. cpp:enumerator:: framePointer


.. cpp:struct:: RealRegsState

  .. cpp:member:: bool is_allocatable
  .. cpp:member:: bool been_used
  .. cpp:member:: int last_used
  .. cpp:member:: registerSlot *contains


.. cpp:class:: regState_t

  .. cpp:function:: regState_t()
  .. cpp:member:: int pc_rel_offset
  .. cpp:member:: int timeline
  .. cpp:member:: int stack_height
  .. cpp:member:: std::vector<RealRegsState> registerStates


.. cpp:class:: registerSpace

  .. cpp:member:: private static registerSpace *globalRegSpace_

    A global mapping of register names to slots

  .. cpp:member:: private static registerSpace *globalRegSpace64_
  .. cpp:function:: private static void createRegSpaceInt(std::vector<registerSlot *> &regs, registerSpace *regSpace)
  .. cpp:function:: static registerSpace *conservativeRegSpace(AddressSpace *proc)

    Pre-set unknown register state: Everything is live...

  .. cpp:function:: static registerSpace *optimisticRegSpace(AddressSpace *proc)

    Everything is dead...

  .. cpp:function:: static registerSpace *irpcRegSpace(AddressSpace *proc)

    IRPC-specific - everything live for now

  .. cpp:function:: static registerSpace *actualRegSpace(instPoint *iP)

    Aaand instPoint-specific

  .. cpp:function:: static registerSpace *savedRegSpace(AddressSpace *proc)

    DO NOT DELETE THESE.

  .. cpp:function:: static registerSpace *getRegisterSpace(AddressSpace *proc)
  .. cpp:function:: static registerSpace *getRegisterSpace(unsigned addr_width)
  .. cpp:function:: registerSpace()
  .. cpp:function:: static void createRegisterSpace(std::vector<registerSlot *> &registers)
  .. cpp:function:: static void createRegisterSpace64(std::vector<registerSlot *> &registers)
  .. cpp:function:: ~registerSpace()
  .. cpp:function:: bool readProgramRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register destination, unsigned size)

    Read the value in register souce from wherever we've stored it in memory (including the register itself), and stick it in actual
    register destination. So the source is the label, and destination is an actual. Size is a legacy parameter for places where we
    don't have register information.

  .. cpp:function:: bool writeProgramRegister(codeGen &gen, Dyninst::Register destination, Dyninst::Register source, unsigned size)

    And the reverse

  .. cpp:function:: Dyninst::Register allocateRegister(codeGen &gen, bool noCost, bool realReg = false)
  .. cpp:function:: bool allocateSpecificRegister(codeGen &gen, Dyninst::Register r, bool noCost = true)
  .. cpp:function:: Dyninst::Register getScratchRegister(codeGen &gen, bool noCost = true, bool realReg = false)

    Like allocate, but don't keep it around if someone else tries to allocate they might get this one.

  .. cpp:function:: Dyninst::Register getScratchRegister(codeGen &gen, std::vector<Dyninst::Register> &excluded, bool noCost = true, bool realReg = false)

    Like the above, but excluding a set of registers (that we don't want to touch)

  .. cpp:function:: bool trySpecificRegister(codeGen &gen, Dyninst::Register reg, bool noCost = true)
  .. cpp:function:: bool saveAllRegisters(codeGen &gen, bool noCost)
  .. cpp:function:: bool restoreAllRegisters(codeGen &gen, bool noCost)
  .. cpp:function:: bool markSavedRegister(Dyninst::Register num, int offsetFromFP)

    For now, we save registers elsewhere and mark them here.

  .. cpp:function:: bool markSavedRegister(RealRegister num, int offsetFromFP)
  .. cpp:function:: bool markKeptRegister(Dyninst::Register num)
  .. cpp:function:: bool checkVolatileRegisters(codeGen &gen, registerSlot::livenessState_t)

    Things that will be modified implicitly by anything else we generate - condition registers, etc.

    This might mean something different later, but for now means "Save special purpose registers". We
    may want to define a "volatile" later - something like "can be unintentionally nuked". For example,
    x86 flags register.

    .. warning:: Only implemented on x86 and x86_64

  .. cpp:function:: bool saveVolatileRegisters(codeGen &gen)
  .. cpp:function:: bool restoreVolatileRegisters(codeGen &gen)
  .. cpp:function:: void freeRegister(Dyninst::Register k)

    Free the specified register (decrement its refCount)

  .. cpp:function:: void forceFreeRegister(Dyninst::Register k)

    Free the register even if its refCount is greater than 1

  .. cpp:function:: void unKeepRegister(Dyninst::Register k)

    And mark a register as not being kept any more

  .. cpp:function:: void cleanSpace()

    Mark all registers as unallocated, but keep livedead info

  .. cpp:function:: bool isFreeRegister(Dyninst::Register k)

    Check to see if the register is free

    DO NOT USE THIS!!!! to tell if you can use a register as a scratch register do that with trySpecificRegister or
    allocateSpecificRegister. This is _ONLY_ to determine if a register should be saved (e.g., over a call).

  .. cpp:function:: bool isRegStartsLive(Dyninst::Register reg)

    Checks to see if register starts live

  .. cpp:function:: int fillDeadRegs(Dyninst::Register *deadRegs, int num)
  .. cpp:function:: void incRefCount(Dyninst::Register k)

    Bump up the reference count. Occasionally, we underestimate it and call this routine to correct this.

  .. cpp:function:: bool markReadOnly(Dyninst::Register k)

    Reset when the regSpace is reset - marked offlimits for allocation.

  .. cpp:function:: bool readOnlyRegister(Dyninst::Register k)
  .. cpp:function:: void checkLeaks(Dyninst::Register to_exclude)

    Make sure that no registers remain allocated, except "to_exclude" Used for assertion checking.

  .. cpp:function:: int getAddressWidth()
  .. cpp:function:: void debugPrint()
  .. cpp:function:: void printAllocedRegisters()
  .. cpp:function:: int numGPRs() const
  .. cpp:function:: int numFPRs() const
  .. cpp:function:: int numSPRs() const
  .. cpp:function:: int numRegisters() const
  .. cpp:function:: std::vector<registerSlot *> &GPRs()
  .. cpp:function:: std::vector<registerSlot *> &FPRs()
  .. cpp:function:: std::vector<registerSlot *> &SPRs()
  .. cpp:function:: std::vector<registerSlot *> &realRegs()

    If we have defined ``realRegisters_`` (IA-32 and 32-bit mode AMD-64) return that. Otherwise return GPRs.

  .. cpp:function:: std::vector<registerSlot *> &trampRegs()

    realRegs() on x86-32, GPRs on all others

  .. cpp:function:: registerSlot *physicalRegs(Dyninst::Register reg)
  .. cpp:function:: registerSlot *operator[](Dyninst::Register)
  .. cpp:function:: bool anyLiveGPRsAtEntry() const

    For platforms with "save all" semantics...

  .. cpp:function:: bool anyLiveFPRsAtEntry() const
  .. cpp:function:: bool anyLiveSPRsAtEntry() const

  ......

  .. rubric::
    The following set of methods and data deal with virtual registers, currently used only on x86.
    The above 'Dyninst::Register' class allocates and uses virtual registers, these methods provide mappings
    from virtual registers to real registers.
 
  .. cpp:function:: RealRegister loadVirtual(registerSlot *virt_r, codeGen &gen)
  .. cpp:function:: RealRegister loadVirtual(Dyninst::Register virt_r, codeGen &gen)

    Put VReg into RReg

  .. cpp:function:: void loadVirtualToSpecific(registerSlot *virt_r, RealRegister real_r, codeGen &gen)
  .. cpp:function:: void loadVirtualToSpecific(Dyninst::Register virt_r, RealRegister real_r, codeGen &gen)

    Put VReg into specific real register

  .. cpp:function:: void makeRegisterAvail(RealRegister r, codeGen &gen)

    Spill away any virtual register in a real so that the real  can be used freely.  Careful with this, no guarentee it won't
    be reallocated in the next step.

  .. cpp:function:: void noteVirtualInReal(Dyninst::Register v_r, RealRegister r_r)

    Tell the tracker that we've manually put some virtual into a real

  .. cpp:function:: void noteVirtualInReal(registerSlot *v_r, RealRegister r_r)
  .. cpp:function:: RealRegister loadVirtualForWrite(Dyninst::Register virt_r, codeGen &gen)

    Like loadVirtual, but don't load orig value first

  .. cpp:function:: RealRegister loadVirtualForWrite(registerSlot *virt_r, codeGen &gen)
  .. cpp:function:: void markVirtualDead(Dyninst::Register num)
  .. cpp:function:: bool spilledAnything()
  .. cpp:member:: Dyninst::Register pc_rel_reg
  .. cpp:member:: int pc_rel_use_count
  .. cpp:function:: int &pc_rel_offset()
  .. cpp:function:: void incStack(int val)
  .. cpp:function:: int getInstFrameSize()
  .. cpp:function:: void setInstFrameSize(int val)
  .. cpp:function:: int getStackHeight()
  .. cpp:function:: void setStackHeight(int val)
  .. cpp:function:: void unifyTopRegStates(codeGen &gen)

    This handles merging register states at merges in the generated code CFG. Used for things like 'if'
    statements. Takes the top level registerState (e.g, the code that was generated in an 'if') and
    emits the saves/restores such to takes us back to the preceding registerState (e.g, the code we
    would be in if the 'if' hadn't executed).

  .. cpp:function:: void pushNewRegState()
  .. cpp:member:: private int instFrameSize_

    How much stack space we allocate for instrumentation before a frame is set up.

  .. cpp:member:: private std::vector<regState_t *> regStateStack
  .. cpp:function:: private std::vector<RealRegsState> &regState()
  .. cpp:function:: private int &timeline()
  .. cpp:member:: private std::set<registerSlot *> regs_been_spilled
  .. cpp:function:: private void initRealRegSpace()

  ......

  .. rubric::
    High-level functions that track data structures and call code gen

  .. cpp:function:: private RealRegister findReal(registerSlot *virt_r, bool &already_setup)
  .. cpp:function:: private void spillReal(RealRegister r, codeGen &gen)
  .. cpp:function:: private void loadReal(RealRegister r, registerSlot *v_r, codeGen &gen)
  .. cpp:function:: private void freeReal(RealRegister r)

  ......

  .. rubric::
    low-level functions for code gen

  .. cpp:function:: private void spillToVReg(RealRegister reg, registerSlot *v_reg, codeGen &gen)
  .. cpp:function:: private void movVRegToReal(registerSlot *v_reg, RealRegister r, codeGen &gen)
  .. cpp:function:: private void movRegToReg(RealRegister dest, RealRegister src, codeGen &gen)

  ......

  .. cpp:member:: private unsigned savedFlagSize
  .. cpp:function:: private registerSpace(const registerSpace &)
  .. cpp:function:: private registerSlot &getRegisterSlot(Dyninst::Register reg)
  .. cpp:function:: private registerSlot *findRegister(Dyninst::Register reg)
  .. cpp:function:: private registerSlot *findRegister(RealRegister reg)
  .. cpp:function:: private bool spillRegister(Dyninst::Register reg, codeGen &gen, bool noCost)
  .. cpp:function:: private bool stealRegister(Dyninst::Register reg, codeGen &gen, bool noCost)
  .. cpp:function:: private bool restoreRegister(Dyninst::Register reg, codeGen &gen, bool noCost)
  .. cpp:function:: private bool popRegister(Dyninst::Register reg, codeGen &gen, bool noCost)
  .. cpp:function:: private bool markSavedRegister(registerSlot *num, int offsetFromFP)
  .. cpp:member:: private int currStackPointer
  .. cpp:member:: private std::unordered_map<Dyninst::Register, registerSlot *> registers_

    This structure is permanently tainted by its association with virtual registers...

  .. cpp:member:: private std::map<Dyninst::Register, registerSlot *> physicalRegisters_
  .. cpp:member:: private std::vector<registerSlot *> GPRs_
  .. cpp:member:: private std::vector<registerSlot *> FPRs_
  .. cpp:member:: private std::vector<registerSlot *> SPRs_
  .. cpp:member:: private std::vector<registerSlot *> realRegisters_

    Used on platforms that have "virtual" registers to provide a mapping for real (e.g., architectural) registers

  .. cpp:function:: private static void initialize()
  .. cpp:function:: private static void initialize32()
  .. cpp:function:: private static void initialize64()
  .. cpp:function:: private registerSpace &operator=(const registerSpace &src)
  .. cpp:function:: private void specializeSpace(rs_location_t state)

    Specialize liveness as represented by a bit array

  .. cpp:function:: private void specializeSpace(const bitArray &)
  .. cpp:function:: private bool checkLive(Dyninst::Register reg, const bitArray &liveRegs)
  .. cpp:member:: private unsigned addr_width
  .. cpp:member:: static bool hasXMM

    for Intel architectures, XMM registers

  .. cpp:function:: int framePointer()
  .. cpp:function:: static unsigned GPR(Dyninst::Register x)
  .. cpp:function:: static unsigned FPR(Dyninst::Register x)
  .. cpp:member:: std::map<std::string, Dyninst::Register> registersByName

    Create a map of register names to register numbers

  .. cpp:function:: Dyninst::Register getRegByName(const std::string name)

    The reverse map can be handled by doing a ``rs[x]->name``

  .. cpp:function:: std::string getRegByNumber(Dyninst::Register num)
  .. cpp:function:: void getAllRegisterNames(std::vector<std::string> &ret)


.. cpp:enum:: registerSpace::rs_location_t

  .. cpp:enumerator:: arbitrary
  .. cpp:enumerator:: ABI_boundary
  .. cpp:enumerator:: allSaved
