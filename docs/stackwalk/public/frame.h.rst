.. _`sec:frame.h`:

frame.h
#######

.. cpp:namespace:: Dyninst::Stackwalker

The ``Walker`` class returns a call stack as a vector of ``Frame``
objects. As described in Section `3.1.1 <#subsec:definitions>`__, each
Frame object represents a stack frame, and contains a return address
(RA), stack pointer (SP) and frame pointer (FP). For each of these
values, optionally, it stores the location where the values were found.
Each Frame object may also be augmented with symbol information giving a
function name (or a symbolic name, in the case of non-functions) for the
object that created the stack frame.

The Frame class provides a set of functions (getRALocation,
getSPLocation and getFPLocation) that return the location in the target
process’ memory or registers where the RA, SP, or FP were found. These
functions may be used to modify the stack. For example, the DyninstAPI
uses these functions to change return addresses on the stack when it
relocates code. The RA, SP, and FP may be found in a register or in a
memory address on a call stack.

.. cpp:class:: Frame : public AnnotatableDense

  .. cpp:function:: Frame()
  .. cpp:function:: Frame(Walker *walker)

  .. cpp:function:: static Frame *newFrame(Dyninst::MachRegisterVal ra, Dyninst::MachRegisterVal sp, Dyninst::MachRegisterVal fp, Walker *walker)

      This method creates a new ``Frame`` object and sets the mandatory data
      members: RA, SP and FP. The new ``Frame`` object is associated with
      ``walker``.

      The optional location fields can be set by the methods below.

      The new ``Frame`` object is created with the ``new`` operator, and the
      user should be deallocate it with the ``delete`` operator when it is no
      longer needed.

  .. cpp:function:: Dyninst::MachRegisterVal getRA() const

      This method returns this ``Frame`` object’s return address.

  .. cpp:function:: void setRA(Dyninst::MachRegisterVal val)

      This method sets this ``Frame`` object’s return address to ``val``.

  .. cpp:function:: Dyninst::MachRegisterVal getSP() const

      This method returns this ``Frame`` object’s stack pointer.

  .. cpp:function:: void setSP(Dyninst::MachRegisterVal val)

      This method sets this ``Frame`` object’s stack pointer to ``val``.

  .. cpp:function:: Dyninst::MachRegisterVal getFP() const

      This method returns this ``Frame`` object’s frame pointer.

  .. cpp:function:: void setFP(Dyninst::MachRegisterVal val)

      This method sets this ``Frame`` object’s frame pointer to ``val``.

  .. cpp:function:: bool isTopFrame() const
  .. cpp:function:: bool isBottomFrame() const

      These methods return whether a ``Frame`` object is the top (e.g., most
      recently executing) or bottom of the stack walk.

  .. cpp:type:: enum loc_address, loc_register, loc_unknown storage_t;
          typedef struct union Dyninst::Address addr; Dyninst::MachRegister reg; val; storage_t location; location_t;

      The ``location_t`` structure is used by the ``getRALocation``,
      ``getSPLocation``, and ``getFPLocation`` methods to describe where in
      the process a ``Frame`` object’s RA, SP, or FP were found. When walking
      a call stack these values may be found in registers or memory. If they
      were found in memory, the ``location`` field of ``location_t`` will
      contain ``loc_address`` and the ``addr`` field will contain the address
      where it was found. If they were found in a register the ``location``
      field of ``location_t`` will contain ``loc_register`` and the ``reg``
      field will refer to the register where it was found. If this ``Frame``
      object was not created by a stackwalk (using the ``newframe`` factory
      method, for example), and has not had a set location method called, then
      location will contain ``loc_unknown``.

  .. cpp:function:: location_t getRALocation() const

      This method returns a ``location_t`` describing where the RA was found.

  .. cpp:function:: void setRALocation(location_t newval)

      This method sets the location of where the RA was found to newval.

  .. cpp:function:: location_t getSPLocation() const

      This method returns a ``location_t`` describing where the SP was found.

  .. cpp:function:: void setSPLocation(location_t newval)

      This method sets the location of where the SP was found to ``newval``.

  .. cpp:function:: location_t getFPLocation() const

      This method returns a ``location_t`` describing where the FP was found.

  .. cpp:function:: void setFPLocation(location_t newval)

      This method sets the location of where the FP was found to ``newval``.

  .. cpp:function:: bool getName(std::string &str) const

      This method returns a stack frame’s symbolic name. Most stack frames are
      created by functions, or function-like objects such as signal handlers
      or system calls. This method returns the name of the object that created
      this stack frame. For stack frames created by functions, this symbolic
      name will be the function name. A symbolic name may not always be
      available for all ``Frame`` objects, such as in cases of stripped
      binaries or special stack frames types.

      The function name is obtained by using this ``Frame`` object’s RA to
      call the ``SymbolLookup`` callback. By default StackwalkerAPI will
      attempt to use the ``SymtabAPI`` package to look up symbol names in
      binaries. If ``SymtabAPI`` is not found, and no alternative
      ``SymbolLookup`` object is present, then this method will return an
      error.

      This method returns ``true`` on success and ``false`` on error.

  .. cpp:function:: bool getObject(void* &obj) const

      In addition to returning a symbolic name (see ``getName``) the
      ``SymbolLookup`` interface allows for an opaque object, a ``void*``, to
      be associated with a ``Frame`` object. The contents of this ``void*`` is
      determined by the ``SymbolLookup`` implementation. Under the default
      implementation that uses SymtabAPI, the ``void*`` points to a Symbol
      object or NULL if no symbol is found.

      This method returns ``true`` on success and ``false`` on error.

  .. cpp:function:: Walker *getWalker() const;

      This method returns the ``Walker`` object that constructed this stack
      frame.

  .. cpp:function:: THR_ID getThread() const;

      This method returns the execution thread that the current ``Frame``
      represents.

  .. cpp:function:: FrameStepper* getStepper() const

      This method returns the ``FrameStepper`` object that was used to
      construct this ``Frame`` object in the ``stepper`` output parameter.
      This method returns ``true`` on success and ``false`` on error.

  .. cpp:function:: bool getLibOffset(std::string &lib, Dyninst::Offset &offset, void* &symtab) const

      This method returns the DSO (a library or executable) and an offset into
      that DSO that points to the location within that DSO where this frame
      was created. ``lib`` is the path to the library that was loaded, and
      ``offset`` is the offset into that library. The return value of the
      ``symtab`` parameter is dependent on the SymbolLookup implementation-by
      default it will contain a pointer to a Dyninst::Symtab object for this
      DSO. See the SymtabAPI Programmer’s Guide for more information on using
      Dyninst::Symtab objects.

  .. cpp:function:: bool nonCall() const

      This method returns whether a ``Frame`` object represents a function
      call; if ``false``, the ``Frame`` may represent instrumentation, a
      signal handler, or something else.

  .. cpp:function:: void setThread(THR_ID)
  .. cpp:function:: void setNonCall()
  .. cpp:function:: bool isTopFrame() const
  .. cpp:function:: bool isBottomFrame() const
  .. cpp:function:: bool isFrameComplete() const
  .. cpp:function:: const Frame *getPrevFrame() const
  .. cpp:function:: FrameStepper *getNextStepper() const


.. cpp:type:: bool (*frame_cmp_t)(const Frame &a, const Frame &b)

  Return true if a < b, by some comparison


.. cpp:function:: bool frame_addr_cmp(const Frame &a, const Frame &b)
.. cpp:function:: bool frame_lib_offset_cmp(const Frame &a, const Frame &b)
.. cpp:function:: bool frame_symname_cmp(const Frame &a, const Frame &b)
.. cpp:function:: bool frame_lineno_cmp(const Frame &a, const Frame &b)


.. cpp:struct:: frame_cmp_wrapper

  .. cpp:member:: frame_cmp_t f
  .. cpp:function:: bool operator()(const FrameNode *a, const FrameNode *b) const

.. cpp:type:: std::set<FrameNode *, frame_cmp_wrapper> frame_set_t


.. cpp:class:: FrameNode

  .. cpp:function:: FrameNode(frame_cmp_wrapper f, std::string s)
  .. cpp:function:: bool isFrame() const
  .. cpp:function:: bool isThread() const
  .. cpp:function:: bool isHead() const
  .. cpp:function:: bool isString() const
  .. cpp:function:: const Frame *getFrame() const
  .. cpp:function:: Frame *getFrame()
  .. cpp:function:: THR_ID getThread() const
  .. cpp:function:: std::string frameString() const
  .. cpp:function:: bool hadError() const
  .. cpp:function:: const frame_set_t &getChildren() const
  .. cpp:function:: frame_set_t &getChildren()
  .. cpp:function:: const FrameNode *getParent() const
  .. cpp:function:: FrameNode *getParent()
  .. cpp:function:: void addChild(FrameNode *fn)
  .. cpp:function:: Walker *getWalker()
  .. cpp:function:: const Walker *getWalker() const


.. cpp:class:: CallTree

  .. cpp:function:: CallTree(frame_cmp_t cmpf = frame_addr_cmp)
  .. cpp:function:: FrameNode *getHead() const
  .. cpp:function:: FrameNode *addFrame(const Frame &f, FrameNode *parent)
  .. cpp:function:: FrameNode *addThread(THR_ID thrd, FrameNode *parent, Walker *walker, bool err_stack)
  .. cpp:function:: frame_cmp_t getComparator()
  .. cpp:function:: frame_cmp_wrapper getCompareWrapper()
  .. cpp:function:: void addCallStack(const std::vector<Frame> &stk, THR_ID thrd, Walker *walker, bool err_stack)
