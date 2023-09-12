.. _`sec:SymReader.h`:

SymReader.h
###########

.. cpp:namespace:: Dyninst

.. cpp:class:: Symbol_t

    Symbol_t is an anonymous struct that any SymReader can use for a symbol 
    handle.  Some symbol readers may not want to store the objects behind a 
    ``void*`` on the heap, so we're making Symbol_t big enough that it could 
    act as a full symbol handle.  Or a SymReader could just choose fill in one
    of the void pointers as a handle to a heap object, if it's comfortable
    doing so.

  .. cpp:member:: void *v1
  .. cpp:member:: void *v2
  .. cpp:member:: int i1
  .. cpp:member:: int i2

.. cpp:class:: Section_t

  .. cpp:member:: void *v1
  .. cpp:member:: void *v2
  .. cpp:member:: int i1
  .. cpp:member:: int i2

.. cpp:class:: SymSegment

  .. cpp:member:: Dyninst::Offset file_offset
  .. cpp:member:: Dyninst::Address mem_addr
  .. cpp:member:: size_t file_size
  .. cpp:member:: size_t mem_size
  .. cpp:member:: int type
  .. cpp:member:: int perms

.. cpp:class:: SymReader

  .. cpp:function:: virtual Symbol_t getSymbolByName(std::string symname) = 0
  .. cpp:function:: virtual Symbol_t getContainingSymbol(Dyninst::Offset offset) = 0
  .. cpp:function:: virtual std::string getInterpreterName() = 0
  .. cpp:function:: virtual unsigned getAddressWidth() = 0
  .. cpp:function:: virtual bool getABIVersion(int &major, int &minor) const = 0
  .. cpp:function:: virtual bool isBigEndianDataEncoding() const = 0
  .. cpp:function:: virtual Architecture getArchitecture() const = 0
  .. cpp:function:: virtual unsigned numSegments() = 0
  .. cpp:function:: virtual bool getSegment(unsigned num, SymSegment &reg) = 0
  .. cpp:function:: virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym) = 0
  .. cpp:function:: virtual Dyninst::Offset getSymbolTOC(const Symbol_t &sym) = 0
  .. cpp:function:: virtual std::string getSymbolName(const Symbol_t &sym) = 0
  .. cpp:function:: virtual std::string getDemangledName(const Symbol_t &sym) = 0
  .. cpp:function:: virtual unsigned long getSymbolSize(const Symbol_t &sym) = 0
  .. cpp:function:: virtual bool isValidSymbol(const Symbol_t &sym) = 0
  .. cpp:function:: virtual Section_t getSectionByName(std::string name) = 0
  .. cpp:function:: virtual Section_t getSectionByAddress(Dyninst::Address addr) = 0
  .. cpp:function:: virtual Dyninst::Address getSectionAddress(Section_t sec) = 0
  .. cpp:function:: virtual std::string getSectionName(Section_t sec) = 0
  .. cpp:function:: virtual bool isValidSection(Section_t sec) = 0
  .. cpp:function:: virtual Dyninst::Offset imageOffset() = 0
  .. cpp:function:: virtual Dyninst::Offset dataOffset() = 0
  .. cpp:function:: virtual void *getElfHandle()
  .. cpp:function:: virtual int getFD()

.. cpp:class:: SymbolReaderFactory

  .. cpp:function:: virtual SymReader *openSymbolReader(std::string pathname) = 0
  .. cpp:function:: virtual SymReader *openSymbolReader(const char *buffer, unsigned long size) = 0
  .. cpp:function:: virtual bool closeSymbolReader(SymReader *sr) = 0
  .. cpp:function:: Dyninst::SymbolReaderFactory *getSymReaderFactory()
