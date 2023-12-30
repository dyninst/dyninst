.. _`sec:util.h`:

util.h
######

This is a public header, but it is not intended for use by users.

Visibility
==========

These macros expand to compiler-specific visibility attributes. For example on gcc, you get

.. code:: c

  __attribute__((visibility ("default")))

.. c:macro:: SYMTAB_EXPORT
.. c:macro:: SYMLITE_EXPORT
.. c:macro:: DYNELF_EXPORT
.. c:macro:: DYNDWARF_EXPORT
.. c:macro:: COMMON_EXPORT
.. c:macro:: COMMON_TEMPLATE_EXPORT
.. c:macro:: INSTRUCTION_EXPORT
.. c:macro:: PARSER_EXPORT
.. c:macro:: PATCHAPI_EXPORT
.. c:macro:: DATAFLOW_EXPORT
.. c:macro:: PC_EXPORT
.. c:macro:: SW_EXPORT
.. c:macro:: INJECTOR_EXPORT
.. c:macro:: SYMEVAL_EXPORT

.. cpp:namespace:: Dyninst

.. cpp:function:: unsigned addrHashCommon(const Address &addr)
.. cpp:function:: unsigned ptrHash(const void * addr)
.. cpp:function:: unsigned ptrHash(void * addr)
.. cpp:function:: unsigned addrHash(const Address &addr)
.. cpp:function:: unsigned addrHash4(const Address &addr)
.. cpp:function:: unsigned addrHash16(const Address &addr)
.. cpp:function:: unsigned stringhash(const std::string &s)
.. cpp:function:: std::string itos(int)
.. cpp:function:: std::string utos(unsigned)

.. code:: c

  #define WILDCARD_CHAR '?'
  #define MULTIPLE_WILDCARD_CHAR '*'

.. cpp:function:: bool wildcardEquiv(const std::string &us, const std::string &them, bool checkCase = false )
