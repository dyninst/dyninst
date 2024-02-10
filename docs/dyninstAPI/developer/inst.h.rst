.. _`sec:inst.h`:

inst.h
######

.. cpp:class:: instMapping

  Container class for "instrument this point with this function".

  .. cpp:function:: instMapping(const std::string f, const std::string i, const int w, callWhen wn, callOrder o, AstNodePtr a = AstNodePtr(), std::string l = "")
  .. cpp:function:: instMapping(const std::string f, const std::string i, const int w, AstNodePtr a = AstNodePtr(), std::string l = "")
  .. cpp:function:: instMapping(const std::string f, const std::string i, const int w, std::vector<AstNodePtr> &aList, std::string l = "")
  .. cpp:function:: instMapping(const instMapping *parMapping, AddressSpace *child)

    Fork

  .. cpp:function:: ~instMapping()
  .. cpp:function:: void dontUseTrampGuard()
  .. cpp:function:: void markAs_MTonly()
  .. cpp:function:: void canUseTrap(bool t)
  .. cpp:function:: bool is_MTonly()
  .. cpp:member:: std::string func

    function to instrument

  .. cpp:member:: std::string inst

      inst. function to place at func

  .. cpp:member:: std::string lib

      library name

  .. cpp:member:: int where
  .. cpp:member:: callWhen when
  .. cpp:member:: callOrder order
  .. cpp:member:: std::vector<AstNodePtr> args

    what to pass as arg0 ... n

  .. cpp:member:: bool useTrampGuard
  .. cpp:member:: bool mt_only
  .. cpp:member:: bool allow_trap
  .. cpp:member:: std::vector<Dyninst::PatchAPI::InstancePtr> instances


.. cpp:enum:: RegControl

  .. cpp:enumerator:: rc_before_jump
  .. cpp:enumerator:: rc_after_jump
  .. cpp:enumerator:: rc_no_control


.. cpp:function:: void initPrimitiveCost()

  get information about the cost of primitives.

.. cpp:function:: void initDefaultPointFrequencyTable()

.. cpp:function:: unsigned getPrimitiveCost(const std::string &name)

  Return the expected runtime of the passed function in instruction times.

.. cpp:function:: func_instance *getFunction(instPoint *point)

  return the function asociated with a point.

.. code:: cpp

  #define FUNC_ENTRY    0x1     /* entry to the function */
  #define FUNC_EXIT     0x2     /* exit from function */
  #define FUNC_CALL     0x4     /* subroutines called from func */
  #define FUNC_ARG      0x8     /* use arg as argument */

......

**Generate an instruction**

Previously this was handled by the polymorphic "emit" function, which
took a variety of argument types and variously returned either an
:cpp:type:`Dyninst::Address` or a :cpp:type:`Dyninst::Register` or nothing of value.
The following family of functions replace "emit" with more strongly typed versions.

.. cpp:function:: codeBufIndex_t emitA(opCode op, Dyninst::Register src1, Dyninst::Register src2, long dst, codeGen &gen, RegControl rc, bool noCost)

  The return value is a magic "hand this in when we update" black box emitA handles emission of things like ifs that need to be updated later.

.. cpp:function:: Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, bool noCost, const instPoint *location, bool for_multithreaded)

  for operations requiring a Dyninst::Register to be returned (e.g., getRetValOp, getRetAddrOp, getParamOp, getSysRetValOp, getSysParamOp)

.. cpp:function:: void emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, int size = 4, const instPoint *location = NULL, AddressSpace *proc = NULL, bool s = true)

  for general arithmetic and logic operations which return nothing

.. cpp:function:: void emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, int size = 4, const instPoint *location = NULL, AddressSpace *proc = NULL)

  for loadOp and loadConstOp (reading from an Dyninst::Address)

.. cpp:function:: void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, int size = 4, const instPoint *location = NULL, AddressSpace *proc = NULL)

  for storeOp (writing to an Dyninst::Address)

.. cpp:function:: void emitVload(opCode op, const image_variable *src1, Dyninst::Register src2, Dyninst::Register dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, int size = 4, const instPoint *location = NULL, AddressSpace *proc = NULL)

  for loadOp and loadConstOp (reading from an Dyninst::Address)

.. cpp:function:: void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, const image_variable *dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, int size = 4, const instPoint *location = NULL, AddressSpace *proc = NULL)

  for storeOp (writing to an Dyninst::Address)

.. cpp:function:: void emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst, codeGen &gen, bool noCost, registerSpace *rs = NULL, bool s = true)

  and the retyped original emitImm companion

.. cpp:type:: BPatch_addrSpec_NP BPatch_countSpec_NP
.. cpp:function:: void emitJmpMC(int condition, int offset, codeGen &gen)
.. cpp:function:: void emitASload(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen, bool noCost)
.. cpp:function:: void emitCSload(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen, bool noCost)
.. cpp:function:: Dyninst::Register emitFuncCall(opCode op, codeGen &gen, std::vector<AstNodePtr> &operands, bool noCost, func_instance *func)

  VG(110601): moved here and added location

.. cpp:function:: Dyninst::Register emitFuncCall(opCode op, codeGen &gen, std::vector<AstNodePtr> &operands, bool noCost, Dyninst::Address callee_addr_)

  Obsolete version that uses an address. DON'T USE THIS or expect it to survive.

.. cpp:function:: int getInsnCost(opCode t)
.. cpp:function:: Dyninst::Register getParameter(Dyninst::Register dest, int param)

  get the requested parameter into a register.

.. cpp:function:: extern std::string getProcessStatus(const AddressSpace *p)
.. cpp:function:: extern unsigned findTags(const std::string funcName)

  TODO - what about mangled names ? expects the symbol name advanced past the underscore

.. cpp:function:: extern Dyninst::Address getMaxBranch()

.. cpp:var:: extern std::map<std::string, unsigned> primitiveCosts
.. cpp:function:: bool writeFunctionPtr(AddressSpace *p, Dyninst::Address addr, func_instance *f)


**A set of optimized emiters for common idioms**

Return false if the platform can't perform any optimizations.

.. cpp:function:: bool emitStoreConst(Dyninst::Address addr, int imm, codeGen &gen, bool noCost)

  Store constant in memory at address

.. cpp:function:: bool emitAddSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost)

  Add constant to memory at address

.. cpp:function:: bool emitSubSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost)

  Subtract constant from memory at address
