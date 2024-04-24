.. _`sec:CodeSource.h`:

CodeSource.h
############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: CodeSource : public Dyninst::InstructionSource

  **Retrieve binary code from an executable, library, or other binary code object**

  It also can provide hints of function entry points (such as those derived from debugging
  symbols) to seed the parser.

  .. cpp:type:: dyn_c_hash_map<void*, CodeRegion*> RegionMap

  .. cpp:function:: virtual bool nonReturning(Address func_entry)

      Checks if a function returns by location ``func_entry``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual bool nonReturning(std::string func_name)

      Checks if a function returns by name ``func_name``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual bool nonReturningSyscall(int number)

      Checks if a system call returns by system call number ``number``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual Address baseAddress()

      Returns the base address of the code covered by this source.

      If the binary file type supplies non-zero base or load addresses (e.g. Windows PE),
      implementations should override these functions.

  .. cpp:function:: virtual Address loadAddress()

      Returns the load address of the code covered by this source.

      If the binary file type supplies non-zero base or load addresses (e.g. Windows PE),
      implementations should override these functions.

  .. cpp:function:: std::map<Address, std::string>& linkage()

      Returns the external linkage map.

      This may be empty.

.. cpp:struct:: Hint

  **A starting point for parsing**

  .. note:: This class satisfies the C++ `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:member:: Address _addr
  .. cpp:member:: int _size
  .. cpp:member:: CodeRegion* _reg
  .. cpp:member:: std::string _name

  .. cpp:function:: Hint(Addr, CodeRegion*, std::string)
  .. cpp:function:: Hint(Address a, int size, CodeRegion * r, std::string s)

.. cpp:class:: SymtabCodeSource : public CodeSource, public boost::lockable_adapter<boost::recursive_mutex>

  .. cpp:function:: SymtabCodeSource(SymtabAPI::Symtab*, hint_filt, bool allLoadedRegions=false)
  .. cpp:function:: SymtabCodeSource(SymtabAPI::Symtab*)
  .. cpp:function:: SymtabCodeSource(const char*)
  .. cpp:function:: bool nonReturning(Address func_entry)
  .. cpp:function:: bool nonReturningSyscall(int num)
  .. cpp:function:: bool resizeRegion(SymtabAPI::Region*, Address newDiskSize)
  .. cpp:function:: Address baseAddress() const
  .. cpp:function:: Address loadAddress() const
  .. cpp:function:: Address getTOC(Address addr) const
  .. cpp:function:: SymtabAPI::Symtab* getSymtabObject()
  .. cpp:function:: bool isValidAddress(const Address) const
  .. cpp:function:: void* getPtrToInstruction(const Address) const
  .. cpp:function:: void* getPtrToData(const Address) const
  .. cpp:function:: unsigned int getAddressWidth() const
  .. cpp:function:: bool isCode(const Address) const
  .. cpp:function:: bool isData(const Address) const
  .. cpp:function:: bool isReadOnly(const Address) const
  .. cpp:function:: Address offset() const
  .. cpp:function:: Address length() const
  .. cpp:function:: Architecture getArch() const
  .. cpp:function:: void removeHint(Hint)
  .. cpp:function:: static void addNonReturning(std::string func_name)
  .. cpp:function:: void print_stats() const
  .. cpp:function:: bool have_stats() const
  .. cpp:function:: void incrementCounter(const std::string& name) const
  .. cpp:function:: void addCounter(const std::string& name, int num) const
  .. cpp:function:: void decrementCounter(const std::string& name) const
  .. cpp:function:: void startTimer(const std::string& name) const
  .. cpp:function:: void stopTimer(const std::string& name) const
  .. cpp:function:: bool findCatchBlockByTryRange(Address, std::set<Address>&) const

.. cpp:struct:: SymtabCodeSource::hint_filt

  .. cpp:function:: virtual bool operator()(SymtabAPI::Function* f)=0

.. cpp:struct:: SymtabCodeSource::try_block

  .. cpp:member:: Address tryStart
  .. cpp:member:: Address tryEnd
  .. cpp:member:: Address catchStart

  .. cpp:function:: try_block(Address ts, Address te, Address c)
