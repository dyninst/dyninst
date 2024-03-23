.. _`sec:frame.h`:

frame.h
#######

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: Frame : public AnnotatableDense

  **A stack frame containing a return address (RA), stack pointer (SP), and frame pointer (FP)**

  Frames may be augmented with symbol information giving a function name (or a symbolic name,
  in the case of non-functions) for the object that created the stack frame.

  .. cpp:function:: Frame()
  .. cpp:function:: Frame(Walker *walker)

  .. cpp:function:: static Frame *newFrame(Dyninst::MachRegisterVal ra, Dyninst::MachRegisterVal sp, \
                                           Dyninst::MachRegisterVal fp, Walker *walker)

      Creates a frame with return address ``ra``, stack pointer ``sp``, and frame pointer ``fp`` and stores
      it in ``walker``.

      .. attention:: A valid ``walker`` is required.

      .. danger:: The user is responsible for deallocating the returned value.

  .. cpp:function:: Dyninst::MachRegisterVal getRA() const

      Returns the return address stored in the frame.

  .. cpp:function:: void setRA(Dyninst::MachRegisterVal val)

      Sets the return address stored in the frame to ``val``.

  .. cpp:function:: Dyninst::MachRegisterVal getSP() const

      Returns the stack pointer stored in the frame.

  .. cpp:function:: void setSP(Dyninst::MachRegisterVal val)

      Sets the stack pointer stored in the frame to ``val``.

  .. cpp:function:: Dyninst::MachRegisterVal getFP() const

      Returns the frame pointer stored in the frame.

  .. cpp:function:: void setFP(Dyninst::MachRegisterVal val)

      Sets the frame pointer stored in the frame to ``val``.

  .. cpp:function:: bool isTopFrame() const

      Checks if this is the most-recently executed frame in a stack walk.

  .. cpp:function:: bool isBottomFrame() const

      Checks if this is the least-recently executed frame in a stack walk.

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

      Returns the stack frame’s symbolic name.

      Most stack frames are created by functions or function-like objects such as signal handlers
      or system calls. This method returns the name of the object that created this stack frame.
      For stack frames created by functions, this symbolic name will be the function name. A
      symbolic name may not always be available (e.g., stripped binaries or special stack frame).

      .. note:: The function name is obtained by invoking the :cpp:class:`StackwalkSymLookup` callback.

      Returns ``false`` on error.

  .. cpp:function:: bool getObject(void* &obj) const

      Returns an opaque handle associated with this frame.

      The contents are determined by the :cpp:class:`StackwalkSymLookup` implementation. Under
      the default implementation that uses :ref:`sec:symtab-intro`, the handle points to a
      :cpp:class:`Symbol` or ``NULL`` if no symbol is found.

      Returns ``false`` on error.

  .. cpp:function:: Walker *getWalker() const;

      Returns the walker that constructed this stack frame.

  .. cpp:function:: THR_ID getThread() const;

      Returns the execution thread to which the current frame belongs.

  .. cpp:function:: FrameStepper* getStepper() const

      Returns the stepper that was used to construct this frame.

  .. cpp:function:: bool getLibOffset(std::string &lib, Dyninst::Offset &offset, void* &symtab) const

      Returns the library and an offset into that library that points to the location where this frame
      was created.

      ``lib`` is the path to the library that was loaded and ``offset`` is the offset into that library.
      The return of ``symtab`` depends on the :cpp:class:`StackwalkSymLookup` implementation. By
      default, it will be the :cpp:class:`Symtab` for the library.

  .. cpp:function:: bool nonCall() const

      Checks if this frame represents a function call.

      If it returns ``false``, this frame may represent instrumentation, a signal handler, or something else.

  .. cpp:function:: void setNonCall()

      Indicates this frame is **not** for a function call.

  .. cpp:function:: bool isFrameComplete() const

      Checks if this frame processing is complete.

      Completion is indicated when the frame's return address is explicitly set via :cpp:func:`setRA`.

  .. cpp:function:: const Frame *getPrevFrame() const

      Returns the frame immediately preceding this one in the stack walk.

      If there is no such frame, returns ``NULL``.


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
