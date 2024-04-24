.. _`sec:symlookup.h`:

symlookup.h
###########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: SymbolLookup

  **An abstract interface for associating a symbolic name with a stack frame**

  Each frame contains a return address pointing into the function (or function-like object)
  that created its stack frame. However, users do not always want to deal
  with addresses when symbolic names are more convenient. This class can also associate an opaque
  object with a frame.

  .. cpp:member:: protected Walker *walker

  .. cpp:function:: SymbolLookup(std::string exec_path = "")

      Constructor for a ``SymbolLookup`` object.

  .. cpp:function:: virtual ~SymbolLookup()

  .. cpp:function:: virtual bool lookupAtAddr(Dyninst::Address addr, std::string &out_name, \
                                              void* &out_value) = 0

      Returns in ``out_name`` the name of the function and the opaque data in ``out_value`` located
      at the address ``addr``. 
      
      ``out_value`` can be any opaque value determined by ``SymbolLookup``. The
      values returned are used by :cpp:func:`Frame::getName` and :cpp:func:`Frame::getObject`.

      This method returns ``true`` on success and ``false`` on error.

  .. cpp:function:: virtual Walker *getWalker()

      Returns the associated walker.

  .. cpp:function:: virtual ProcessState *getProcessState()

      Returns the associated process state.

.. cpp:class:: SwkSymtab : public SymbolLookup

  .. cpp:function:: SwkSymtab(std::string exec_name)
  .. cpp:function:: virtual bool lookupAtAddr(Dyninst::Address addr, std::string &out_name, void* &out_value)
  .. cpp:function:: virtual ~SwkSymtab()


.. cpp:class:: SymDefaultLookup : public SymbolLookup

  .. cpp:function:: SymDefaultLookup(std::string exec_name)
  .. cpp:function:: virtual bool lookupAtAddr(Dyninst::Address addr, std::string &out_name, void* &out_value)
  .. cpp:function:: virtual ~SymDefaultLookup()
