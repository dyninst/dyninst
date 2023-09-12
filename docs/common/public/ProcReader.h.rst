.. _`sec:ProcReader.h`:

ProcReader.h
############

.. cpp:namespace:: Dyninst

.. cpp:class:: ProcessReader

The implementation of the :cpp:class:`AddressLookup` on Linux requires it to be
able to read from the target process’s address space.

.. note:: The ``ProcessReader`` is defined, but not used, on non-Linux systems.

By default, reading from another process on the same system this is done through the
operating system debugger interface. A user can provide their own
process reading mechanism by implementing a child of the
``ProcessReader`` class and passing it to the ``AddressLookup`` constructors.

.. warning:: These functions should not be directly called by user code.

  .. cpp:function:: virtual bool ReadMem(Address traced, void *inSelf, unsigned size) = 0

    This function should read ``size`` bytes from the address at ``traced``
    into the buffer pointed to by ``inSelf``. This function must return
    ``true`` on success and ``false`` on error.

  .. cpp:function:: virtual bool GetReg(MachRegister reg, MachRegisterVal &val) = 0

    This function reads from the register specified by ``reg`` and places
    the result in ``val``. It must return ``true`` on success and ``false``
    on failure.

  .. cpp:function::virtual bool start() = 0;
  .. cpp:function::virtual bool isAsync()
  .. cpp:function::virtual bool done() = 0
