.. _`sec:libstate.h`:

libstate.h
##########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: swkProcessReader : public ProcessReader

  .. cpp:function:: swkProcessReader(ProcessState *pstate, std::string executable_)
  .. cpp:function:: virtual bool start()
  .. cpp:function:: virtual bool ReadMem(Address inTraced, void *inSelf, unsigned amount)
  .. cpp:function:: virtual bool GetReg(Dyninst::MachRegister, Dyninst::MachRegisterVal&)
  .. cpp:function:: virtual bool done()
  .. cpp:function:: virtual ~swkProcessReader()


.. cpp:class:: TrackLibState : public LibraryState

  .. cpp:function:: TrackLibState(ProcessState *parent, std::string executable = "")
  .. cpp:function:: virtual bool getLibraryAtAddr(Address addr, LibAddrPair &olib)
  .. cpp:function:: virtual bool getLibraries(std::vector<LibAddrPair> &olibs, bool allow_refresh = true)
  .. cpp:function:: virtual bool getAOut(LibAddrPair &ao)
  .. cpp:function:: virtual void notifyOfUpdate()
  .. cpp:function:: virtual Address getLibTrapAddress()
  .. cpp:function:: virtual ~TrackLibState()


.. cpp:class:: StaticBinaryLibState : public LibraryState

  .. cpp:function:: StaticBinaryLibState(ProcessState *parent, std::string executable = "")
  .. cpp:function:: ~StaticBinaryLibState()
  .. cpp:function:: virtual bool getLibraryAtAddr(Address addr, LibAddrPair &olib)
  .. cpp:function:: virtual bool getLibraries(std::vector<LibAddrPair> &olibs, bool allow_refresh = true)
  .. cpp:function:: virtual bool getLibc(LibAddrPair &lc)
  .. cpp:function:: virtual bool getLibthread(LibAddrPair &lt)
  .. cpp:function:: virtual bool getAOut(LibAddrPair &ao)
  .. cpp:function:: virtual void notifyOfUpdate()
  .. cpp:function:: virtual Address getLibTrapAddress()


.. cpp:class:: LibraryWrapper

  .. cpp:function:: static SymReader *testLibrary(std::string filename)
  .. cpp:function:: static SymReader *getLibrary(std::string filename)
  .. cpp:function:: static void registerLibrary(SymReader *reader, std::string filename)
