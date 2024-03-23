.. _`sec:AddrLookup.h`:

AddrLookup.h
############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:struct:: LoadedLibrary

  .. cpp:member:: std::string name
  .. cpp:member:: Address codeAddr
  .. cpp:member:: Address dataAddr


.. cpp:class:: AddressLookup : public AnnotatableSparse

  **A mapping for determining the address in a process where a SymtabAPI object is loaded**

  A single dynamic library may load at different addresses in different processes.
  The ‘address’ fields in a dynamic library’s symbol tables will contain
  offsets rather than absolute addresses. These offsets can be added to
  the library’s load address, which is computed at runtime, to determine
  the absolute address where a symbol is loaded.

  It also examines a process and finds its dynamic
  libraries and executables and each one’s load address. This information
  can be used to map between SymtabAPI objects and absolute addresses.
  Each AddressLookup instance is associated with one process. An
  AddressLookup object can be created to work with the currently
  running process or a different process on the same system.

  .. cpp:function:: static AddressLookup* createAddressLookup(ProcessReader* reader = NULL)

    Creates a new AddressLookup object associated with the process that called this function.

    The caller is responsible for the lifetime of the returned object. If ``reader`` is non-NULL on Linux,
    then it will use ``reader`` to read from the target process.

    Returns a null pointer on error.

  .. cpp:function:: static AddressLookup* createAddressLookup(PID pid, ProcessReader* reader = NULL)

    Creates a new AddressLookup object associated with the process with ID, ``pid``.

    The caller is responsible for the lifetime of the returned object. If ``reader`` is non-NULL on Linux,
    then it will use ``reader`` to read from the target process.

    Returns a null pointer on error.

  .. cpp:function:: static AddressLookup* createAddressLookup(const std::vector<LoadedLibrary>& ll)

    Creates a new AddressLookup associated with the given collection of libraries from a process.

    The collection of libraries can initially be collected with :cpp:func:`getLoadAddresses`.
    The collection can then be used with this function to re-create the
    AddressLookup object, even if the original process no longer exists.
    This can be useful for off-line address lookups, where only the load
    addresses are collected while the process exists and then all address
    translation is done after the process has terminated.

    Returns a null pointer on error.

  .. cpp:function:: bool getAddress(Symtab* tab, Symbol* sym, Address& addr)

    Retrieve the address where the symbol, ``sym``, is located in the process.

    Returns `true` if an address was found.

  .. cpp:function:: bool getAddress(Symtab* tab, Offset off, Address& addr)

    Retrieve the address at the offset, ``off``, in the process.

    Returns `true` if an address was found.

  .. cpp:function:: bool getSymbol(Address addr, Symbol*& sym, Symtab*& tab, bool close = false)

    Retrieve the symbol and its :cpp:class:`Symtab` at the address, ``addr`` in the process.

    If the ``close`` is ``true``, returns the nearest symbol that comes before ``addr``.
    This can be useful when looking up the function that resides at an address.

    Returns `true` if a symbol was found.

  .. cpp:function:: bool getOffset(Address addr, Symtab*& tab, Offset& off)

    Retrieve the offset and its :cpp:class:`Symtab` at the address, ``addr`` in the process.

    Returns ``false`` on error.

  .. cpp:function:: bool getAllSymtabs(std::vector<Symtab*>& tabs)

    Retrieves all :cpp:class:`Symtab` objects contained in the process.

    This includes the process’s executable and all shared objects loaded into its address space.

    Returns ``false`` on error.

  .. cpp:function:: bool getLoadAddress(Symtab* sym, Address& load_address)

    Retrieve the address where :cpp:class:`Symtab` resides in the process.

    On systems where an object can have one load address for its code and one
    for its data, this function will return the code’s load address. Use
    :cpp:func:`getDataLoadAddress` to get the data load address.

    Returns ``false`` on error.

  .. cpp:function:: bool getDataLoadAddress(Symtab* sym, Address& load_addr)

    Retrieve the address where :cpp:class:`Symtab` resides in the process.

    Like :cpp:func:`getLoadAddress`, but returns the address of the data section.

    Returns ``false`` on error.

  .. cpp:function:: bool getLoadAddresses(std::vector<LoadedLibrary>& ll)

    Retrieves libraries loaded in the process.

    These libraries can then be used by :cpp:func:`createAddressLookup`
    to create a new AddressLookup object. This function is
    usually used as part of an off-line address lookup mechanism.

    Returns ``false`` on error.

  .. cpp:function:: bool getExecutable(LoadedLibrary &lib)

  .. cpp:function:: bool getOffset(Address addr, LoadedLibrary& lib, Offset& off)

    Retrieve the offset and its :cpp:class:`LoadedLibrary` at the address, ``addr`` in the process.

    Returns ``false`` on error.

  .. cpp:function:: bool refresh()

    Update the snapshot of the process.

    When a AddressLookup object is initially created it takes a snapshot
    of the libraries currently loaded in a process, which is then used to
    answer queries into this API. As the process runs more libraries may be
    loaded and unloaded, and this snapshot may become out of date. An
    AddressLookup’s view of a process can be updated by calling this
    function which causes it to examine the process for loaded and unloaded
    objects and update its data structures accordingly.

    Returns ``false`` on error.

  .. cpp:function:: Address getLibraryTrapAddrSysV()

  .. cpp:function:: virtual ~AddressLookup()


Notes
=====

On Linux, it needs to read from the
process’ address space to determine its shared objects and load
addresses. By default, it will attach to another process
using a debugger interface to read the necessary information, or simply
use ``memcpy`` if reading from the current process. This behavior
can be changed by implementing a custum reader derived from :cpp:class:`Dyninst::ProcessReader`.

When created for a running process, it takes a snapshot of the process’
loaded libraries and their addresses. These values can then be queried.
However, they are not automatically updated when the process loads or unloads libraries,
so users need to call :cpp:func:`refresh` to update them.
