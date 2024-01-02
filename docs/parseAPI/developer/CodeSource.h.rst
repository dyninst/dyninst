.. _`sec-dev:CodeSource.h`:

CodeSource.h
############

.. cpp:namespace:: Dyninst::ParseAPI::dev

.. cpp:class:: CodeSource

  Implementers of CodeSource can fill the following structures with available
  information. Some of this information is optional.

  .. cpp:member:: protected mutable std::map<Address, std::string> _linkage

      Named external linkage table (e.g. PLT on ELF). Optional.

  .. cpp:member:: protected Address _table_of_contents

      Table of Contents for position independent references. Optional.

  .. cpp:member:: protected std::vector<CodeRegion*> _regions

      Code regions in the binary. At least one region is required for parsing.

  .. cpp:member:: protected Dyninst::IBSTree<CodeRegion> _region_tree

      Code region lookup. Must be consistent with the _regions vector. Mandatory.

  .. cpp:member:: protected dyn_c_vector<Hint> _hints

      Hints for where to begin parsing.

      Required for the default parsing mode, but usage of one of the direct parsing
      modes (parsing particular locations or using speculative methods) is supported
      without hints.

  .. cpp:member:: protected static dyn_hash_map<std::string, bool> non_returning_funcs

      Lists of known non-returning functions

  .. cpp:member:: protected static dyn_hash_map<int, bool> non_returning_syscalls_x86

      Lists of known non-returning functions by syscall

  .. cpp:member:: protected static dyn_hash_map<int, bool> non_returning_syscalls_x86_64

      Lists of known non-returning functions by syscall number on x86_64

  .. cpp:function:: dyn_c_vector<Hint> const& hints() const
  .. cpp:function:: std::vector<CodeRegion*> const& regions() const
  .. cpp:function:: int findRegions(Address addr, std::set<CodeRegion*> & ret) const
  .. cpp:function:: bool regionsOverlap() const
  .. cpp:function:: Address getTOC() const
  .. cpp:function:: virtual Address getTOC(Address) const

      If the binary file type supplies per-function TOC's (e.g. ppc64 Linux), override.

  .. cpp:function:: virtual void print_stats() const
  .. cpp:function:: virtual bool have_stats() const
  .. cpp:function:: virtual void incrementCounter(const std::string& name) const
  .. cpp:function:: virtual void addCounter(const std::string& name, int num) const
  .. cpp:function:: virtual void decrementCounter(const std::string& name) const
  .. cpp:function:: virtual void startTimer(const std::string& name) const
  .. cpp:function:: virtual void stopTimer(const std::string& name) const
  .. cpp:function:: virtual bool findCatchBlockByTryRange(Address address, std::set<Address>&) const
  .. cpp:function:: void addRegion(CodeRegion*)
  .. cpp:function:: void removeRegion(CodeRegion*)

.. cpp:class:: SymtabCodeRegion : public CodeRegion

  .. cpp:function:: SymtabCodeRegion(SymtabAPI::Symtab*, SymtabAPI::Region*)
  .. cpp:function:: SymtabCodeRegion(SymtabAPI::Symtab*, SymtabAPI::Region*, std::vector<SymtabAPI::Symbol*> &symbols)
  .. cpp:function:: void names(Address, std::vector<std::string>&)
  .. cpp:function:: bool findCatchBlock(Address addr, Address& catchStart)
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
  .. cpp:function:: Address low() const
  .. cpp:function:: Address high() const
  .. cpp:function:: SymtabAPI::Region* symRegion() const

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
