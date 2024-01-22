symlookup.h
===========

.. cpp:namespace:: Dyninst::stackwalk

Class SymbolLookup
~~~~~~~~~~~~~~~~~~

**Defined in:** ``symlookup.h``

The ``SymbolLookup`` virtual class is an interface for associating a
symbolic name with a stack frame. Each ``Frame`` object contains an
address (the RA) pointing into the function (or function-like object)
that created its stack frame. However, users do not always want to deal
with addresses when symbolic names are more convenient. This class is an
interface for mapping a ``Frame`` object’s RA into a name.

In addition to getting a name, this class can also associate an opaque
object (via a ``void*``) with a Frame object. It is up to the
``SymbolLookup`` implementation what to return in this opaque object.

The default implementation of ``SymbolLookup`` provided by
StackwalkerAPI uses the ``SymLite`` tool to lookup symbol names. It
returns a Symbol object in the anonymous ``void*``.

.. code-block:: cpp

    SymbolLookup(std::string exec_path = "");

Constructor for a ``SymbolLookup`` object.

.. code-block:: cpp

    virtual bool lookupAtAddr(Address addr, string &out_name, void* &out_value) = 0

This method takes an address, ``addr``, as input and returns the
function name, ``out_name``, and an opaque value, ``out_value``, at that
address. Output parameter ``out_name`` should be the name of the
function that contains ``addr``. Output parameter ``out_value`` can be
any opaque value determined by the ``SymbolLookup`` implementation. The
values returned are used by the ``Frame::getName`` and
``Frame::getObject`` functions.

This method returns ``true`` on success and ``false`` on error.

.. code-block:: cpp

    virtual Walker *getWalker()

This method returns the ``Walker`` object associated with this
``SymbolLookup``.

.. code-block:: cpp

    virtual ProcessState *getProcessSate()

This method returns the ``ProcessState`` object associated with this
``SymbolLookup``.