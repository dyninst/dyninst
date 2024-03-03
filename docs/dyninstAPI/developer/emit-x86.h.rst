.. _`sec:emit-x86.h`:

emit-x86.h
##########


.. cpp:class:: Emitterx86 : public Emitter

  .. cpp:function:: virtual ~Emitterx86()
  .. cpp:function:: virtual bool emitLoadRelativeSegReg(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitXorRegRM(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitXorRegReg(Dyninst::Register dest, Dyninst::Register base, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitXorRegImm(Dyninst::Register dest, int imm, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitXorRegSegReg(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLEA(Dyninst::Register base, Dyninst::Register index, unsigned int scale, int disp, Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitCallInstruction(codeGen &, func_instance *, Dyninst::Register) = 0


.. cpp:class:: EmitterIA32 : public Emitterx86

  32-bit class declared here since its implementation is in both inst-x86.C and emit-x86.C

  .. cpp:function:: virtual ~EmitterIA32()
  .. cpp:member:: static const int mt_offset
  .. cpp:function:: codeBufIndex_t emitIf(Dyninst::Register expr_reg, Dyninst::Register target, RegControl rc, codeGen &gen)
  .. cpp:function:: void emitOp(unsigned opcode, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen)
  .. cpp:function:: void emitRelOp(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s)
  .. cpp:function:: void emitDiv(Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s)
  .. cpp:function:: void emitOpImm(unsigned opcode1, unsigned opcode2, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen)
  .. cpp:function:: void emitRelOpImm(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s)
  .. cpp:function:: void emitTimesImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src1imm, codeGen &gen)
  .. cpp:function:: void emitDivImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src1imm, codeGen &gen, bool s)
  .. cpp:function:: void emitLoad(Dyninst::Register dest, Dyninst::Address addr, int size, codeGen &gen)
  .. cpp:function:: void emitLoadConst(Dyninst::Register dest, Dyninst::Address imm, codeGen &gen)
  .. cpp:function:: void emitLoadIndir(Dyninst::Register dest, Dyninst::Register addr_reg, int size, codeGen &gen)
  .. cpp:function:: bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &)
  .. cpp:function:: bool emitLoadRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen)
  .. cpp:function:: bool emitLoadRelativeSegReg(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen)
  .. cpp:function:: void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Dyninst::Address offset)
  .. cpp:function:: void emitLoadFrameAddr(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen)
  .. cpp:function:: void emitLoadOrigFrameRelative(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen)
  .. cpp:function:: void emitLoadOrigRegRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, codeGen &gen, bool store)
  .. cpp:function:: void emitLoadOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: void emitStoreOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: void emitStore(Dyninst::Address addr, Dyninst::Register src, int size, codeGen &gen)
  .. cpp:function:: void emitStoreIndir(Dyninst::Register addr_reg, Dyninst::Register src, int size, codeGen &gen)
  .. cpp:function:: void emitStoreFrameRelative(Dyninst::Address offset, Dyninst::Register src, Dyninst::Register scratch, int size, codeGen &gen)
  .. cpp:function:: void emitStoreRelative(Dyninst::Register source, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen)
  .. cpp:function:: void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen)
  .. cpp:function:: bool clobberAllFuncCall(registerSpace *rs, func_instance *callee)
  .. cpp:function:: void setFPSaveOrNot(const int *liveFPReg, bool saveOrNot)
  .. cpp:function:: virtual Dyninst::Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee)

    We can overload this for the statdyn case

  .. cpp:function:: int emitCallParams(codeGen &gen, const std::vector<AstNodePtr> &operands, func_instance *target, std::vector<Dyninst::Register> &extra_saves, bool noCost)
  .. cpp:function:: bool emitCallCleanup(codeGen &gen, func_instance *target, int frame_size, std::vector<Dyninst::Register> &extra_saves)
  .. cpp:function:: void emitGetRetVal(Dyninst::Register dest, bool addr_of, codeGen &gen)
  .. cpp:function:: void emitGetRetAddr(Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: void emitGetParam(Dyninst::Register dest, Dyninst::Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen)
  .. cpp:function:: void emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest, int stackShift, codeGen &gen)
  .. cpp:function:: void emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: void emitPushFlags(codeGen &gen)
  .. cpp:function:: void emitRestoreFlags(codeGen &gen, unsigned offset)
  .. cpp:function:: void emitRestoreFlagsFromStackSlot(codeGen &gen)
  .. cpp:function:: void emitStackAlign(int offset, codeGen &gen)

    Moves stack pointer by offset and aligns it to ``IA32_STACK_ALIGNMENT``
    with the following sequence:

    .. code:: console

         lea    -off(%esp) => %esp           # move %esp down
         mov    %eax => saveSlot1(%esp)      # save %eax onto stack
         lahf                                # save %eflags byte into %ah
         seto   %al                          # save overflow flag into %al
         mov    %eax => saveSlot2(%esp)      # save flags %eax onto stack
         lea    off(%esp) => %eax            # store original %esp in %eax
         and    -$IA32_STACK_ALIGNMENT,%esp  # align %esp
         mov    %eax => (%esp)               # store original %esp on stack
         mov    -off+saveSlot2(%eax) => %eax # restore flags %eax from stack
         add    $0x7f,%al                    # restore overflow flag from %al
         sahf                                # restore %eflags byte from %ah
         mov    (%esp) => %eax               # re-load old %esp into %eax to ...
         mov    -off+saveSlot1(%eax) => %eax # ... restore %eax from stack

    This sequence has three important properties:

     - It never *directly* writes to memory below %esp.  It always begins
       by moving %esp down, then writing to locations above it.  This way,
       if the kernel decides to interrupt, it won't stomp all over our
       values before we get a chance to use them.
     - It is designed to support easy de-allocation of this space by
       ending with %esp pointing to where we stored the original %esp.
     - Care has been taken to properly restore both %eax and %eflags
       by using "lea" instead of "add" or "sub," and saving the necessary
       flags around the "and" instruction.

    Saving of the flags register can be skipped if the register is not live.

  .. cpp:function:: bool emitBTSaves(baseTramp *bt, codeGen &gen)
  .. cpp:function:: bool emitBTRestores(baseTramp *bt, codeGen &gen)
  .. cpp:function:: void emitLoadEffectiveAddress(Dyninst::Register base, Dyninst::Register index, unsigned int scale, int disp, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: void emitStoreImm(Dyninst::Address addr, int imm, codeGen &gen, bool noCost)
  .. cpp:function:: void emitAddSignedImm(Dyninst::Address addr, int imm, codeGen &gen, bool noCost)
  .. cpp:function:: int Register_DWARFtoMachineEnc(int n)
  .. cpp:function:: bool emitPush(codeGen &gen, Dyninst::Register pushee)
  .. cpp:function:: bool emitPop(codeGen &gen, Dyninst::Register popee)
  .. cpp:function:: bool emitAdjustStackPointer(int index, codeGen &gen)
  .. cpp:function:: bool emitMoveRegToReg(Dyninst::Register src, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: bool emitMoveRegToReg(registerSlot* src, registerSlot* dest, codeGen& gen)
  .. cpp:function:: void emitLEA(Dyninst::Register base, Dyninst::Register index, unsigned int scale, int disp, Dyninst::Register dest, codeGen &gen)
  .. cpp:function:: bool emitXorRegRM(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen &gen)
  .. cpp:function:: bool emitXorRegReg(Dyninst::Register dest, Dyninst::Register base, codeGen &gen)
  .. cpp:function:: bool emitXorRegImm(Dyninst::Register dest, int imm, codeGen &gen)
  .. cpp:function:: bool emitXorRegSegReg(Dyninst::Register dest, Dyninst::Register base, int disp, codeGen &gen)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret) = 0


.. cpp:class:: EmitterIA32Dyn : public EmitterIA32

  .. cpp:function:: ~EmitterIA32Dyn()
  .. cpp:function:: protected bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret)


.. cpp:class:: EmitterIA32Stat : public EmitterIA32

  .. cpp:function:: ~EmitterIA32Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: protected bool emitCallInstruction(codeGen &gen, func_instance *target, Dyninst::Register ret)


.. cpp:var:: extern EmitterIA32Dyn emitterIA32Dyn
.. cpp:var:: extern EmitterIA32Stat emitterIA32Stat

.. cpp:function:: void emitMovRegToReg64(Register dest, Register src, bool is_64, codeGen &gen)
.. cpp:function:: void emitMovPCRMToReg64(Register dest, int offset, int size, codeGen &gen)
.. cpp:function:: void emitMovImmToReg64(Register dest, long imm, bool is_64, codeGen &gen)
.. cpp:function:: void emitPushReg64(Register src, codeGen &gen)
.. cpp:function:: void emitPopReg64(Register dest, codeGen &gen)
.. cpp:function:: void emitMovImmToRM64(Register base, int disp, int imm, codeGen &gen)
.. cpp:function:: void emitAddMem64(Address addr, int imm, codeGen &gen)
.. cpp:function:: void emitAddRM64(Address addr, int imm, codeGen &gen)
.. cpp:function:: void emitOpRegImm64(unsigned opcode, unsigned opcode_ext, Register rm_reg, int imm, bool is_64, codeGen &gen)


.. cpp:class:: EmitterAMD64 : public Emitterx86

  .. cpp:function:: virtual ~EmitterAMD64()
  .. cpp:member:: static const int mt_offset
  .. cpp:function:: codeBufIndex_t emitIf(Register expr_reg, Register target, RegControl rc, codeGen &gen)
  .. cpp:function:: void emitOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen)
  .. cpp:function:: void emitRelOp(unsigned op, Register dest, Register src1, Register src2, codeGen &gen, bool s)
  .. cpp:function:: void emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s)
  .. cpp:function:: void emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1, RegValue src2imm, codeGen &gen)
  .. cpp:function:: void emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm, codeGen &gen, bool s)
  .. cpp:function:: void emitTimesImm(Register dest, Register src1, RegValue src1imm, codeGen &gen)
  .. cpp:function:: void emitDivImm(Register dest, Register src1, RegValue src1imm, codeGen &gen, bool s)
  .. cpp:function:: void emitLoad(Register dest, Address addr, int size, codeGen &gen)
  .. cpp:function:: void emitLoadConst(Register dest, Address imm, codeGen &gen)
  .. cpp:function:: void emitLoadIndir(Register dest, Register addr_reg, int size, codeGen &gen)
  .. cpp:function:: bool emitCallRelative(Register, Address, Register, codeGen &)
  .. cpp:function:: bool emitLoadRelative(Register dest, Address offset, Register base, int size, codeGen &gen)
  .. cpp:function:: bool emitLoadRelativeSegReg(Register dest, Address offset, Register base, int size, codeGen &gen)
  .. cpp:function:: void emitLoadFrameAddr(Register dest, Address offset, codeGen &gen)
  .. cpp:function:: void emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen)
  .. cpp:function:: void emitLoadOrigRegRelative(Register dest, Address offset, Register base, codeGen &gen, bool store)
  .. cpp:function:: void emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen)
  .. cpp:function:: void emitLoadShared(opCode op, Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Address offset)
  .. cpp:function:: void emitStoreOrigRegister(Address register_num, Register dest, codeGen &gen)
  .. cpp:function:: void emitStore(Address addr, Register src, int size, codeGen &gen)
  .. cpp:function:: void emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen)
  .. cpp:function:: void emitStoreFrameRelative(Address offset, Register src, Register scratch, int size, codeGen &gen)
  .. cpp:function:: void emitStoreRelative(Register source, Address offset, Register base, int size, codeGen &gen)
  .. cpp:function:: void emitStoreShared(Register source, const image_variable *var, bool is_local, int size, codeGen &gen)
  .. cpp:function:: bool clobberAllFuncCall(registerSpace *rs, func_instance *callee)

    Recursive function that goes to where our instrumentation is calling to figure out what registers
    are clobbered there, and in any function that it calls, to a certain depth ... at which point we
    clobber everything

    Update-12/06, njr, since we're going to a cached system we are just going to
    look at the first level and not do recursive, since we would have to also store and reexamine every
    call out instead of doing it on the fly like before.

  .. cpp:function:: void setFPSaveOrNot(const int *liveFPReg, bool saveOrNot)
  .. cpp:function:: virtual Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee)

    See comment on 32-bit emitCall

  .. cpp:function:: void emitGetRetVal(Register dest, bool addr_of, codeGen &gen)

    FIXME: comment here on the stack layout

  .. cpp:function:: void emitGetRetAddr(Register dest, codeGen &gen)
  .. cpp:function:: void emitGetParam(Register dest, Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen)
  .. cpp:function:: void emitASload(int ra, int rb, int sc, long imm, Register dest, int stackShift, codeGen &gen)
  .. cpp:function:: void emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
  .. cpp:function:: void emitPushFlags(codeGen &gen)
  .. cpp:function:: void emitRestoreFlags(codeGen &gen, unsigned offset)
  .. cpp:function:: void emitRestoreFlagsFromStackSlot(codeGen &gen)
  .. cpp:function:: void emitStackAlign(int offset, codeGen &gen)

    Moves stack pointer by offset and aligns it to AMD64_STACK_ALIGNMENT
    with the following sequence:

    .. code:: console

        lea    -off(%rsp) => %rsp           # move %rsp down
        mov    %rax => saveSlot1(%rsp)      # save %rax onto stack
        lahf                                # save %rflags byte into %ah
        seto   %al                          # save overflow flag into %al
        mov    %rax => saveSlot2(%rsp)      # save flags %rax onto stack
        lea    off(%rsp) => %rax            # store original %rsp in %rax
        and    -$AMD64_STACK_ALIGNMENT,%rsp # align %rsp
        mov    %rax => (%rsp)               # store original %rsp on stack
        mov    -off+saveSlot2(%rax) => %rax # restore flags %rax from stack
        add    $0x7f,%al                    # restore overflow flag from %al
        sahf                                # restore %rflags byte from %ah
        mov    (%rsp) => %rax               # re-load old %rsp into %rax to ...
        mov    -off+saveSlot1(%rax) => %rax # ... restore %rax from stack

    This sequence has four important properties:

    - It never writes to memory within offset bytes below the original
      %rsp.  This is to make it compatible with red zone skips.
    - It never *directly* writes to memory below %rsp.  It always begins
      by moving %rsp down, then writing to locations above it.  This way,
      if the kernel decides to interrupt, it won't stomp all over our
      values before we get a chance to use them.
    - It is designed to support easy de-allocation of this space by
      ending with %rsp pointing to where we stored the original %rsp.
    - Care has been taken to properly restore both %eax and %eflags
      by using "lea" instead of "add" or "sub," and saving the necessary
      flags around the "and" instruction.

    Saving of the flags register can be skipped if the register is not live.

  .. cpp:function:: bool emitBTSaves(baseTramp *bt, codeGen &gen)
  .. cpp:function:: bool emitBTRestores(baseTramp *bt, codeGen &gen)
  .. cpp:function:: void emitStoreImm(Address addr, int imm, codeGen &gen, bool noCost)
  .. cpp:function:: void emitAddSignedImm(Address addr, int imm, codeGen &gen, bool noCost)
  .. cpp:function:: int Register_DWARFtoMachineEnc(int n)

    The DWARF register numbering does not correspond to the architecture's register encoding for 64-bit target binaries only.
    This method maps the number that DWARF reports for a register to the actualregister number.

  .. cpp:function:: bool emitPush(codeGen &gen, Register pushee)
  .. cpp:function:: bool emitPop(codeGen &gen, Register popee)
  .. cpp:function:: bool emitAdjustStackPointer(int index, codeGen &gen)
  .. cpp:function:: bool emitMoveRegToReg(Register src, Register dest, codeGen &gen)
  .. cpp:function:: bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen)
  .. cpp:function:: void emitLEA(Register base, Register index, unsigned int scale, int disp, Register dest, codeGen &gen)
  .. cpp:function:: bool emitXorRegRM(Register dest, Register base, int disp, codeGen &gen)
  .. cpp:function:: bool emitXorRegReg(Register dest, Register base, codeGen &gen)
  .. cpp:function:: bool emitXorRegImm(Register dest, int imm, codeGen &gen)
  .. cpp:function:: bool emitXorRegSegReg(Register dest, Register base, int disp, codeGen &gen)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret) = 0


.. cpp:function:: static void emitOpMemImm64(unsigned opcode, unsigned opcode_ext, Register base, int imm, bool is_64, codeGen &gen)

  operation on memory location specified with a base register (does not work for RSP, RBP, R12, R13)


.. cpp:class:: EmitterAMD64Dyn : public EmitterAMD64

  .. cpp:function:: ~EmitterAMD64Dyn()
  .. cpp:function:: bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret)

.. cpp:class:: EmitterAMD64Stat : public EmitterAMD64

  .. cpp:function:: ~EmitterAMD64Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: bool emitCallInstruction(codeGen &gen, func_instance *target, Register ret)


.. cpp:var:: extern EmitterAMD64Dyn emitterAMD64Dyn
.. cpp:var:: extern EmitterAMD64Stat emitterAMD64Stat

......

.. rubric::
  This is the distance on the basetramp stack frame from the start of the GPR save region
  to where the base pointer is, in 8-byte quadwords.

.. code:: c

  #define GPR_SAVE_REGION_OFFSET 18

.. rubric::
  This is the distance in 8-byte quadwords from the frame pointer in our basetramp's stack frame to
  the saved value of RFLAGS (1 qword for our false return address, 16 for the saved registers, 1 more
  for the flags).

.. code:: c

  #define SAVED_RFLAGS_OFFSET 18



Notes on DWARF mappings
***********************

On 64-bit x86_64 targets, the DWARF register number does not correspond to the machine encoding. See
the AMD-64 ABI. We can only safely map the general purpose registers (0-7 on ia-32, 0-15 on amd-64)
This is incomplete. The x86_64 ABI specifies a mapping from dwarf numbers (0-66) to ("architecture
number"). Without a corresponding mapping for the SVR4 dwarf-machine encoding for IA-32, however, it
is not meaningful to provide this mapping.

See :ref:`sec-dev:MachRegister-DWARF-encodings` for more details.


.. code:: c

  #define IA32_MAX_MAP 7
  #define AMD64_MAX_MAP 15
  static int const amd64_register_map[] =
  {
      0,  // RAX
      2,  // RDX
      1,  // RCX
      3,  // RBX
      6,  // RSI
      7,  // RDI
      5,  // RBP
      4,  // RSP
      8, 9, 10, 11, 12, 13, 14, 15    // gp 8 - 15
  };
