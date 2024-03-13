.. _`sec:SymLiteCodeSource.h`:

SymLiteCodeSource.h
###################

.. cpp:namespace:: Dyninst::ParseAPI

.. Warning:: SymLite is not supported.


.. cpp:class:: SymReaderCodeRegion : public CodeRegion

  .. cpp:member:: private SymReader *_symtab
  .. cpp:member:: private SymSegment *_region
  .. cpp:member:: private void *rawData
  .. cpp:function:: SymReaderCodeRegion(SymReader *, SymSegment *)
  .. cpp:function:: ~SymReaderCodeRegion()
  .. cpp:function:: void names(Address, std::vector<std::string> &) override
  .. cpp:function:: bool findCatchBlock(Address addr, Address &catchStart) override

  ......

  .. rubric:: InstructionSource implementation

  .. cpp:function:: bool isValidAddress(const Address) const override
  .. cpp:function:: void *getPtrToInstruction(const Address) const override
  .. cpp:function:: void *getPtrToData(const Address) const override
  .. cpp:function:: unsigned int getAddressWidth() const override
  .. cpp:function:: bool isCode(const Address) const override
  .. cpp:function:: bool isData(const Address) const override
  .. cpp:function:: bool isReadOnly(const Address) const override
  .. cpp:function:: Address offset() const override
  .. cpp:function:: Address length() const override
  .. cpp:function:: Architecture getArch() const override

  ......

  .. rubric:: interval

  .. cpp:function:: Address low() const override
  .. cpp:function:: Address high() const override

  ......

  .. cpp:function:: SymSegment *symRegion() const


.. cpp:class:: SymReaderCodeSource : public CodeSource

  .. cpp:member:: private SymReader *_symtab
  .. cpp:member:: private bool owns_symtab
  .. cpp:member:: private mutable CodeRegion *_lookup_cache
  .. cpp:member:: private StatContainer *stats_parse

    Stats information

  .. cpp:member:: private bool _have_stats
  .. cpp:function:: SymReaderCodeSource(SymReader *)
  .. cpp:function:: SymReaderCodeSource(const char *)
  .. cpp:function:: ~SymReaderCodeSource()
  .. cpp:function:: bool nonReturning(Address func_entry)
  .. cpp:function:: bool nonReturningSyscall(int num)

  .. cpp:function:: bool resizeRegion(SymSegment *, Address newDiskSize)

    Fails and returns false if it can't find a CodeRegion to match the region has to remove the region
    before modifying the region's size,  otherwise the region can't be found

  .. cpp:function:: SymReader *getSymReaderObject()

  ......

  .. rubric:: InstructionSource implementation

  .. cpp:function:: bool isValidAddress(const Address) const
  .. cpp:function:: void *getPtrToInstruction(const Address) const
  .. cpp:function:: void *getPtrToData(const Address) const
  .. cpp:function:: unsigned int getAddressWidth() const
  .. cpp:function:: bool isCode(const Address) const
  .. cpp:function:: bool isData(const Address) const
  .. cpp:function:: bool isReadOnly(const Address) const
  .. cpp:function:: Address offset() const
  .. cpp:function:: Address length() const
  .. cpp:function:: Architecture getArch() const

  ......

  .. cpp:function:: void removeHint(Hint)
  .. cpp:function:: static void addNonReturning(std::string func_name)
  .. cpp:function:: void print_stats() const
  .. cpp:function:: bool have_stats() const
  .. cpp:function:: void incrementCounter(const std::string &name) const
  .. cpp:function:: void addCounter(const std::string &name, int num) const
  .. cpp:function:: void decrementCounter(const std::string &name) const
  .. cpp:function:: private CodeRegion *lookup_region(const Address addr) const
  .. cpp:function:: private void removeRegion(CodeRegion *) // removes from region tree
  .. cpp:function:: private void overlapping_warn(const char *file, unsigned line) const
  .. cpp:function:: private void init_regions()
  .. cpp:function:: private bool init_stats()
