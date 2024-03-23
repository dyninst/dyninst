.. _`sec-dev:SymtabReader.h`:

SymtabReader.h
##############

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: SymtabReader : public SymReader

  .. cpp:member:: protected Symtab *symtab
  .. cpp:member:: protected int ref_count
  .. cpp:member:: protected std::vector<SymSegment> segments
  .. cpp:member:: protected bool ownsSymtab

  .. cpp:function:: virtual ~SymtabReader()
  .. cpp:function:: virtual bool getABIVersion(int &major, int &minor) const
  .. cpp:function:: virtual void *getElfHandle()

.. cpp:function:: SymbolReaderFactory *getSymtabReaderFactory()


Notes
=====

Some components (StackwalkerAPI, ProcControlAPI) use a SymReader
to read symbols rather than a straight dependency on SymtabAPI.  A component can
either define its own SymReader (as ProcControlAPI does) or it can use SymtabAPI as
its symbol reader.  These SymtabReader and SymtabReaderFactory implement the SymReader
interface with a SymtabAPI implementation.
