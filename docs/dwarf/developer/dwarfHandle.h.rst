.. _`sec:dwarfHandle.h`:

dwarfHandle.h
#############

.. cpp:namespace:: Dyninst::DwarfDyninst


.. cpp:type:: boost::shared_ptr<DwarfFrameParser> DwarfFrameParserPtr

.. cpp:class:: DwarfHandle

  .. cpp:type:: DwarfHandle *ptr
  .. cpp:member:: private DwarfFrameParserPtr sw
  .. cpp:member:: private dwarf_status_t init_dwarf_status
  .. cpp:member:: private Dwarf *dbg_file_data
  .. cpp:member:: private Dwarf *file_data
  .. cpp:member:: private Dwarf **line_data
  .. cpp:member:: private Dwarf **type_data
  .. cpp:member:: private Dwarf **frame_data
  .. cpp:member:: private Elf_X *file
  .. cpp:member:: private Elf_X *dbg_file
  .. cpp:function:: private bool init_dbg()
  .. cpp:function:: private void locate_dbg_file()
  .. cpp:function:: private bool hasFrameData(Elf_X *elfx)
  .. cpp:member:: private std::string filename
  .. cpp:member:: private std::string debug_filename
  .. cpp:member:: private static std::map<std::string, DwarfHandle::ptr> all_dwarf_handles
  .. cpp:function:: private DwarfHandle(std::string filename_, Elf_X* file_, void*)
  .. cpp:function:: ~DwarfHandle()
  .. cpp:function:: static DwarfHandle::ptr createDwarfHandle(std::string filename_, Elf_X* file_, void* e = NULL)
  .. cpp:function:: Elf_X *origFile()
  .. cpp:function:: Elf_X *debugLinkFile()
  .. cpp:function:: Dwarf **line_dbg()
  .. cpp:function:: Dwarf **type_dbg()
  .. cpp:function:: Dwarf **frame_dbg()
  .. cpp:function:: DwarfFrameParserPtr frameParser()
  .. cpp:function:: const std::string &getDebugFilename()


.. cpp:enum:: DwarfHandle::dwarf_status_t

  .. cpp:enumerator:: dwarf_status_uninitialized
  .. cpp:enumerator:: dwarf_status_error
  .. cpp:enumerator:: dwarf_status_ok
