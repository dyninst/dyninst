.. _`sec-dev:LineInformation.h`:

LineInformation.h
#################

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: LineInformation final : private RangeLookupTypes<Statement>::type

  .. cpp:member:: StringTablePtr strings_

  .. cpp:function:: void dump()

      Dumps a debug representation of the line map.

  .. cpp:function:: StringTablePtr getStrings()
  .. cpp:function:: void setStrings(StringTablePtr strings_)


  .. cpp:function:: bool addLine(unsigned int fileIndex, unsigned int lineNo, unsigned int lineOffset, \
                                 Offset lowInclusiveAddr, Offset highExclusiveAddr)

  .. cpp:function:: bool addLine(const std::string &lineSource, unsigned int lineNo, unsigned int lineOffset, \
                                 Offset lowInclusiveAddr, Offset highExclusiveAddr)

      This method adds a new line to the line Map. ``lineSource`` represents the source file name.
      ``lineNo`` represents the line number.

  .. cpp:function:: bool addAddressRange(Offset lowInclusiveAddr, Offset highExclusiveAddr, const char* lineSource, \
                                         unsigned int lineNo, unsigned int lineOffset = 0);

      This method adds an address range ``[lowInclusiveAddr, highExclusiveAddr)`` for the line with line number ``lineNo``
      in source file ``lineSource``.

  .. cpp:function:: void addLineInfo(LineInformation *lineInfo)

  .. cpp:function:: std::pair<const_line_info_iterator, const_line_info_iterator> range(std::string const& file, const unsigned int lineNo) const
  .. cpp:function:: std::pair<const_line_info_iterator, const_line_info_iterator> equal_range(std::string const& file) const
