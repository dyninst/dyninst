.. _`sec:baseTramp.h`:

baseTramp.h
###########


.. cpp:class:: baseTramp

  .. cpp:function:: static baseTramp *create(instPoint *p)
  .. cpp:function:: static baseTramp *createForIRPC(AddressSpace *as)

    We use baseTramps to generate save and restore code for iRPCs iRPCs don't have a corresponding
    instPoint so the AddressSpace needs to be specified

  .. cpp:function:: static baseTramp *fork(baseTramp *parBT, AddressSpace *child)
  .. cpp:function:: func_instance *func() const
  .. cpp:function:: instPoint *point() const
  .. cpp:function:: instPoint *instP() const
  .. cpp:function:: AddressSpace *proc() const
  .. cpp:function:: void initializeFlags()
  .. cpp:function:: bool generateCode(codeGen &gen, Dyninst::Address baseInMutatee)
  .. cpp:function:: bool generateCodeInlined(codeGen &gen, Dyninst::Address baseInMutatee)

    We're generating something like so:

      .. code:: console

        <Save state>
        <If>
           <compare>
             <load>
               <add>
                 <tramp guard addr>
                 <multiply>
                   <thread index>
                   <sizeof (int)>
             <0>
           <sequence>
             <store>
               <... tramp guard addr>
               <1>
             <mini tramp sequence>
             <store>
               <... tramp guard addr>
               <0>
        <Cost section>
        <Load state>

        Break it down...
        <Save state>

    TODO: an AST for saves that knows how many registers we're using...

  .. cpp:function:: bool checkForFuncCalls()
  .. cpp:function:: ~baseTramp()
  .. cpp:function:: int numDefinedRegs()
  .. cpp:function:: void setIRPCAST(AstNodePtr ast)
  .. cpp:member:: private instPoint *point_
  .. cpp:member:: private AddressSpace *as_
  .. cpp:member:: private AstNodePtr ast_
  .. cpp:function:: private bool shouldRegenBaseTramp(registerSpace *rs)
  .. cpp:member:: private cfjRet_t funcJumpState_

    We keep two sets of flags. The first controls which features we enable in the base tramp,
    including: multithread support, which classes of registers are saved, etc. The second records
    (during code gen) what has been done so we can undo it later.

  .. cpp:member:: private bool needsStackFrame_
  .. cpp:member:: private bool threaded_
  .. cpp:member:: private bool optimizationInfo_
  .. cpp:member:: bool savedFPRs

    Tracking the results of code generation.

  .. cpp:member:: bool createdFrame
  .. cpp:member:: bool savedOrigAddr
  .. cpp:member:: bool createdLocalSpace
  .. cpp:member:: bool alignedStack
  .. cpp:member:: bool savedFlags
  .. cpp:member:: bool optimizedSavedRegs
  .. cpp:member:: bool suppressGuards
  .. cpp:member:: bool suppressThreads
  .. cpp:member:: bool spilledRegisters
  .. cpp:member:: int stackHeight
  .. cpp:member:: bool skippedRedZone
  .. cpp:member:: bool wasFullFPRSave
  .. cpp:function:: bool validOptimizationInfo()
  .. cpp:function:: bool generateSaves(codeGen &gen, registerSpace *)

    Code generation methods

  .. cpp:function:: bool generateRestores(codeGen &gen, registerSpace *)
  .. cpp:member:: bitArray definedRegs

    Generated state methods

  .. cpp:function:: int funcJumpSlotSize()
  .. cpp:function:: bool guarded() const
  .. cpp:function:: bool threaded() const
  .. cpp:function:: bool doOptimizations()
  .. cpp:function:: bool makesCall()
  .. cpp:function:: bool needsFrame()
  .. cpp:function:: bool madeFrame()
  .. cpp:function:: bool saveFPRs()
  .. cpp:function:: void setNeedsFrame(bool)


.. code:: cpp

  #define X86_REGS_SAVE_LIMIT 3

