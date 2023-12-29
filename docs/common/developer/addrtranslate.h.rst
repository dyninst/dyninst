.. _`sec:addrtranslate.h`:

addrtranslate.h
###############

.. cpp:namespace:: Dyninst

.. cpp:class:: LoadedLib

  .. cpp:member:: protected string name
  .. cpp:member:: protected Address load_addr
  .. cpp:member:: protected Address data_load_addr
  .. cpp:member:: protected Address dynamic_addr
  .. cpp:member:: protected Address map_addr
  .. cpp:member:: protected bool should_clean
  .. cpp:member:: protected vector< pair<Address, unsigned long> > mapped_regions
  .. cpp:member:: protected SymReader *symreader
  .. cpp:member:: protected SymbolReaderFactory *symreader_factory
  .. cpp:member:: protected void *up_ptr
  .. cpp:function:: LoadedLib(string name, Address load_addr)
  .. cpp:function:: virtual ~LoadedLib()
  .. cpp:function:: void add_mapped_region(Address addr, unsigned long size)
  .. cpp:function:: string getName() const
  .. cpp:function:: void setDataLoadAddr(Address a)
  .. cpp:function:: vector< pair<Address, unsigned long> > *getMappedRegions()
  .. cpp:function:: virtual Address offToAddress(Offset off)
  .. cpp:function:: virtual Offset addrToOffset(Address addr)
  .. cpp:function:: virtual Address getCodeLoadAddr() const
  .. cpp:function:: virtual Address getDataLoadAddr() const
  .. cpp:function:: virtual Address getDynamicAddr() const
  .. cpp:function:: virtual void getOutputs(string &filename, Address &code, Address &data)
  .. cpp:function:: Address getMapAddr() const
  .. cpp:function:: void* getUpPtr()
  .. cpp:function:: void setUpPtr(void *v)
  .. cpp:function:: void setShouldClean(bool b)
  .. cpp:function:: bool shouldClean()
  .. cpp:function:: void setFactory(SymbolReaderFactory *factory)

.. cpp:class:: AddressTranslate

  .. cpp:member:: protected PID pid
  .. cpp:member:: protected PROC_HANDLE phandle
  .. cpp:member:: protected bool creation_error
  .. cpp:member:: protected vector<LoadedLib *> libs
  .. cpp:member:: protected std::string exec_name
  .. cpp:member:: protected LoadedLib *exec
  .. cpp:member:: protected SymbolReaderFactory *symfactory
  .. cpp:member:: protected bool read_abort
  .. cpp:function:: protected AddressTranslate(PID pid, PROC_HANDLE phand = INVALID_HANDLE_VALUE, std::string exename = std::string(""))
  .. cpp:function:: virtual bool refresh() = 0
  .. cpp:function:: virtual ~AddressTranslate()
  .. cpp:function:: PID getPid()
  .. cpp:function:: bool getLibAtAddress(Address addr, LoadedLib* &lib)
  .. cpp:function:: bool getLibs(vector<LoadedLib *> &libs_)
  .. cpp:function:: bool getArchLibs(vector<LoadedLib *> &olibs)
  .. cpp:function:: LoadedLib *getLoadedLib(std::string name)
  .. cpp:function:: LoadedLib *getLoadedLib(SymReader *sym)
  .. cpp:function:: LoadedLib *getExecutable()
  .. cpp:function:: virtual Address getLibraryTrapAddrSysV()
  .. cpp:function:: void setReadAbort(bool b)
  .. cpp:function:: static AddressTranslate *createAddressTranslator(PID pid_, ProcessReader *reader_ = NULL, SymbolReaderFactory *symfactory_ = NULL, PROC_HANDLE phand = INVALID_HANDLE_VALUE, std::string exename = std::string(""), Address interp_base = (Address) -1);
  .. cpp:function:: static AddressTranslate *createAddressTranslator(ProcessReader *reader_ = NULL, SymbolReaderFactory *symfactory_ = NULL, std::string exename = std::string(""), Address interp_base = (Address) -1);
