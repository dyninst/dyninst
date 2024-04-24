.. _`sec:addrtranslate-sysv.h`:

addrtranslate-sysv.h
####################

.. cpp:namespace:: Dyninst

.. cpp:class:: FCNode

  .. cpp:function:: FCNode(string f, dev_t d, ino_t i, SymbolReaderFactory *factory_)
  .. cpp:function:: string getFilename()
  .. cpp:function:: string getInterpreter()
  .. cpp:function:: void markInterpreter()
  .. cpp:function:: void getSegments(vector<SymSegment> &segs)
  .. cpp:function:: unsigned getAddrSize()
  .. cpp:function:: Offset get_r_debug()
  .. cpp:function:: Offset get_r_trap()

  .. cpp:member:: protected string filename
  .. cpp:member:: protected dev_t device
  .. cpp:member:: protected ino_t inode
  .. cpp:member:: protected bool parsed_file
  .. cpp:member:: protected bool parsed_file_fast
  .. cpp:member:: protected bool parse_error
  .. cpp:member:: protected bool is_interpreter
  .. cpp:member:: protected string interpreter_name
  .. cpp:member:: protected vector<SymSegment> segments
  .. cpp:member:: protected unsigned addr_size
  .. cpp:member:: protected Offset r_debug_offset
  .. cpp:member:: protected Offset r_trap_offset
  .. cpp:member:: protected SymReader *symreader
  .. cpp:member:: protected SymbolReaderFactory *factory
  .. cpp:function:: protected void parsefile()

.. cpp:class:: FileCache

  .. cpp:function:: FileCache()
  .. cpp:function:: FCNode *getNode(const string &filename, Dyninst::SymbolReaderFactory *factory_)

.. cpp:class:: AddressTranslateSysV : public AddressTranslate

  .. cpp:function:: bool init()
  .. cpp:function:: virtual bool refresh()
  .. cpp:function:: virtual Address getLibraryTrapAddrSysV()
  .. cpp:function:: AddressTranslateSysV(int pid, ProcessReader *reader_,  SymbolReaderFactory *reader_fact, std::string exe_name, Address interp_base)
  .. cpp:function:: AddressTranslateSysV()
  .. cpp:function:: virtual ~AddressTranslateSysV()
