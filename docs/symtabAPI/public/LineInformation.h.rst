LineInformation.h
=================

.. cpp:namespace:: Dyninst::SymtabAPI

Class LineInformation
---------------------

This class represents an entire line map for a module. This contains
mappings from a line number within a source to the address ranges.

.. code-block:: cpp

    bool getAddressRanges(const char * lineSource, unsigned int LineNo,
    std::vector<AddressRange> & ranges)

This methos returns the address ranges in ``ranges`` corresponding to
the line with line number ``lineNo`` in the source file ``lineSource``.
Searches within this line map. Return ``true`` if at least one address
range corresponding to the line number was found and returns ``false``
if none found.

.. code-block:: cpp

    bool getSourceLines(Offset addressInRange, std::vector<Statement *> & lines) bool getSourceLines(Offset addressInRange,
    std::vector<LineNoTuple> & lines)

These methods returns the source file names and line numbers
corresponding to the given address ``addressInRange``. Searches within
this line map. Return ``true`` if at least one tuple corresponding to
the offset was found and returns ``false`` if none found. Note that the
order of arguments is reversed from the corresponding interfaces in
``Module`` and ``Symtab``.

.. code-block:: cpp

    bool addLine(const char * lineSource, unsigned int lineNo, unsigned int
    lineOffset, Offset lowInclusiveAddr, Offset highExclusiveAddr)

This method adds a new line to the line Map. ``lineSource`` represents
the source file name. ``lineNo`` represents the line number.

.. code-block:: cpp

    bool addAddressRange(Offset lowInclusiveAddr, Offset highExclusiveAddr,
    const char* lineSource, unsigned int lineNo, unsigned int lineOffset = 0);

This method adds an address range
``[lowInclusiveAddr, highExclusiveAddr)`` for the line with line number
``lineNo`` in source file ``lineSource``.

.. code-block:: cpp

    LineInformation::const_iterator begin() const

This method returns an iterator pointing to the beginning of the line
information for the module. This is useful for iterating over the entire
line information present in a module. An example described in Section
`7.3 <#subsec:LineNoIterating>`__ gives more information on how to use
``begin()`` for iterating over the line information.

.. code-block:: cpp

    LineInformation::const_iterator end() const

This method returns an iterator pointing to the end of the line
information for the module. This is useful for iterating over the entire
line information present in a module. An example described in Section
`7.3 <#subsec:LineNoIterating>`__ gives more information on how to use
``end()`` for iterating over the line information.