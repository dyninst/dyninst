.. _`sec-dev:Symbol.h`:

Symbol.h
########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Symbol : public AnnotatableSparse

  .. cpp:member:: private Offset ptr_offset_

      Function descriptor offset.  Not available on all platforms.

  .. cpp:member:: private unsigned size_

      Size of this symbol. This is NOT available on all platforms.

  .. cpp:member:: private Aggregate* aggregate_

      Pointer to Function or Variable container, if appropriate.

  .. cpp:function:: static Symbol *magicEmitElfSymbol()
  .. cpp:function:: bool setOffset(Offset newOffset)
  .. cpp:function:: bool setPtrOffset(Offset newOffset)
  .. cpp:function:: bool setLocalTOC(Offset localTOC)
  .. cpp:function:: bool setSize(unsigned ns)
  .. cpp:function:: bool setRegion(Region *r)
  .. cpp:function:: bool setModule(Module *mod)
  .. cpp:function:: bool setMangledName(std::string name)
  .. cpp:function:: bool setSymbolType(SymbolType sType)
  .. cpp:function:: bool setDynamic(bool d)
  .. cpp:function:: bool setAbsolute(bool a)
  .. cpp:function:: bool setDebug(bool dbg)
  .. cpp:function:: bool setCommonStorage(bool cs)
  .. cpp:function:: bool setVersionFileName(std::string &fileName)
  .. cpp:function:: bool setVersions(std::vector<std::string> &vers)
  .. cpp:function:: bool setVersionNum(unsigned verNum)
  .. cpp:function:: void setVersionHidden()
  .. cpp:function:: void setInternalType(int i)
  .. cpp:function:: bool setStrIndex(int strindex)
  .. cpp:function:: void setReferringSymbol(Symbol* referringSymbol)
  .. cpp:function:: bool setIndex(int index)
  .. cpp:function:: bool setVariable(Variable* var)
  .. cpp:function:: bool setFunction(Function* func)
  .. cpp:function:: Symtab *getSymtab() const
  .. cpp:function:: bool getVersionFileName(std::string &fileName) const
  .. cpp:function:: bool getVersions(std::vector<std::string> *&vers) const
  .. cpp:function:: bool getVersionNum(unsigned &verNum) const
  .. cpp:function:: bool getVersionHidden() const
  .. cpp:function:: int getInternalType() const
  .. cpp:function:: Symbol* getReferringSymbol() const

  .. cpp:function:: Offset getPtrOffset() const

      For binaries with an OPD section, the offset in the OPD that contains the function pointer data structure for this symbol.

  .. cpp:function:: Offset getLocalTOC() const

      For platforms with a TOC register, the expected TOC for this object referred to by this symbol.

Usage
*****

New symbols, functions, and variables can be created and added to the
library at any point using the handle returned by successful parsing of
the object file. When possible, add a function or variable rather than a
symbol directly.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/symtabAPI/addSymbol.cpp
  :language: cpp
  :linenos:
