.. _`sec:dwarfFrameParser.h`:

dwarfFrameParser.h
##################

.. cpp:namespace:: Dyninst::DwarfDyninst


.. cpp:class:: DwarfFrameParser

  .. cpp:type:: boost::shared_ptr<DwarfFrameParser> Ptr
  .. cpp:function:: static Ptr create(Dwarf *dbg, Elf *eh_frame, Architecture arch)
  .. cpp:function:: DwarfFrameParser(Dwarf *dbg_, Elf *eh_frame, Architecture arch_)
  .. cpp:function:: ~DwarfFrameParser()
  .. cpp:function:: bool hasFrameDebugInfo()
  .. cpp:function:: bool getRegRepAtFrame(Address pc, MachRegister reg, VariableLocation &loc, FrameErrors_t &err_result)
  .. cpp:function:: bool getRegValueAtFrame(Address pc, MachRegister reg, MachRegisterVal &reg_result, ProcessReader *reader, FrameErrors_t &err_result)
  .. cpp:function:: bool getRegAtFrame(Address pc, MachRegister reg, DwarfResult &cons, FrameErrors_t &err_result)
  .. cpp:function:: bool getRegsForFunction(std::pair<Address, Address> range, MachRegister reg, std::vector<VariableLocation> &locs, FrameErrors_t &err_result)

    Returns whatever Dwarf claims the function covers. We use an entryPC (actually, can be any PC in the function)
    because common has no idea what a Function is and I don't want to move this to Symtab.

  .. cpp:function:: private void setupCFIData()
  .. cpp:member:: private static std::map<frameParser_key, Ptr> frameParsers

  ......

  .debug_frame and .eh_frame are sections that contain frame info.
  They might or might not be present, but we need at least one to create a
  DwarfFrameParser object, otherwise it wouldn't make sense to create a
  frame parser to no frame data.
  .debug_frame will be accessed by a Dwarf handle returned by libdw, while
  .eh_frame will be accessed by an Elf reference.
  Why? Although they can be in the same binary, for the case of separate debug
  info, in which we have two binaries, the .eh_frame remains in the stripped binary,
  while the .debug_frame (if generated) will be present in the debug info binary.

  .. note::
    ``dbg`` and ``dbg_eh_frame`` cannot both be NULL.
  
  .. note::
    ``dbg`` will be a handle to ``dbg_eh_frame`` for when we don't have separate debug info, which
    is a redundancy, but necessary.

  .. cpp:member:: private Dwarf *dbg

    to access .debug_frame, can be NULL.

  .. cpp:member:: private Elf *dbg_eh_frame

    to access .eh_frame, can be NULL.

  .. cpp:member:: private Architecture arch
  .. cpp:member:: private boost::once_flag fde_dwarf_once
  .. cpp:member:: private dwarf_status_t fde_dwarf_status
  .. cpp:member:: private dyn_mutex cfi_lock
  .. cpp:member:: private std::vector<Dwarf_CFI *> cfi_data


.. cpp:struct:: DwarfFrameParser::frameParser_key

  .. cpp:member:: Dwarf * dbg
  .. cpp:member:: Elf * eh_frame
  .. cpp:member:: Architecture arch
  .. cpp:function:: frameParser_key(Dwarf * d, Elf * eh_frame_, Architecture a)
  .. cpp:function:: bool operator<(const frameParser_key& rhs) const


.. cpp:enum:: DwarfFrameParser::dwarf_status_t

  .. cpp:enumerator:: dwarf_status_uninitialized
  .. cpp:enumerator:: dwarf_status_error
  .. cpp:enumerator:: dwarf_status_ok


.. cpp:enum:: FrameErrors_t

  .. cpp:enumerator:: FE_Bad_Frame_Data=15

    to coincide with equivalent SymtabError

  .. cpp:enumerator:: FE_No_Frame_Entry
  .. cpp:enumerator:: FE_Frame_Read_Error
  .. cpp:enumerator:: FE_Frame_Eval_Error
  .. cpp:enumerator:: FE_No_Error
