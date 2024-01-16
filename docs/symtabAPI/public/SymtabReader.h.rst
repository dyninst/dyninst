.. _`sec:SymtabReader.h`:

SymtabReader.h
##############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: SymtabReaderFactory : public SymbolReaderFactory

  .. cpp:function:: SymtabReaderFactory()
  .. cpp:function:: virtual ~SymtabReaderFactory()
  .. cpp:function:: virtual SymReader *openSymbolReader(std::string pathname)
  .. cpp:function:: virtual SymReader *openSymbolReader(const char *buffer, unsigned long size)
  .. cpp:function:: virtual bool closeSymbolReader(SymReader *sr)


.. cpp:class:: SymtabReader : public SymReader

  .. cpp:function:: SymtabReader(std::string file_)
  .. cpp:function:: SymtabReader(const char *buffer, unsigned long size)
  .. cpp:function:: SymtabReader(Symtab *s)
  .. cpp:function:: virtual Symbol_t getSymbolByName(std::string symname)
  .. cpp:function:: virtual unsigned long getSymbolSize(const Symbol_t &sym)
  .. cpp:function:: virtual Symbol_t getContainingSymbol(Dyninst::Offset offset)
  .. cpp:function:: virtual std::string getInterpreterName()
  .. cpp:function:: virtual unsigned getAddressWidth()
  .. cpp:function:: virtual bool isBigEndianDataEncoding() const
  .. cpp:function:: virtual Architecture getArchitecture() const
  .. cpp:function:: virtual unsigned numSegments()
  .. cpp:function:: virtual bool getSegment(unsigned num, SymSegment &seg)
  .. cpp:function:: virtual Dyninst::Offset getSymbolOffset(const Symbol_t &sym)
  .. cpp:function:: virtual Dyninst::Offset getSymbolTOC(const Symbol_t &sym)
  .. cpp:function:: virtual std::string getSymbolName(const Symbol_t &sym)
  .. cpp:function:: virtual std::string getDemangledName(const Symbol_t &sym)
  .. cpp:function:: virtual bool isValidSymbol(const Symbol_t &sym)
  .. cpp:function:: virtual Section_t getSectionByName(std::string name)
  .. cpp:function:: virtual Section_t getSectionByAddress(Dyninst::Address addr)
  .. cpp:function:: virtual Dyninst::Address getSectionAddress(Section_t sec)
  .. cpp:function:: virtual std::string getSectionName(Section_t sec)
  .. cpp:function:: virtual bool isValidSection(Section_t sec)
  .. cpp:function:: virtual Dyninst::Offset imageOffset()
  .. cpp:function:: virtual Dyninst::Offset dataOffset()

