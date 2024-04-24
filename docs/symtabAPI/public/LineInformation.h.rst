.. _`sec:LineInformation.h`:

LineInformation.h
#################

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: LineInformation final : private RangeLookupTypes<Statement>::type

  **Mappings from a line number within a source to the address ranges**

  .. cpp:type:: RangeLookupTypes< Statement> traits
  .. cpp:type:: RangeLookupTypes< Statement >::type impl_t
  .. cpp:type:: impl_t::index<Statement::addr_range>::type::const_iterator const_iterator
  .. cpp:type:: impl_t::index<Statement::line_info>::type::const_iterator const_line_info_iterator
  .. cpp:type:: traits::value_type Statement_t

  .. cpp:function:: LineInformation()
  .. cpp:function:: ~LineInformation() = default

  .. cpp:function:: bool getSourceLines(Offset addressInRange, std::vector<Statement_t>& lines)

      Returns in ``lines`` the source file names and line numbers corresponding to the address, ``addressInRange``.

      Returns ``true`` if at least one source was found.

  .. cpp:function:: bool getSourceLines(Offset addressInRange, std::vector<Statement>& lines)

      Returns in ``lines`` the source file names and line numbers corresponding to the address, ``addressInRange``.

      Returns ``true`` if at least one source was found.

  .. cpp:function:: bool getAddressRanges(const char* lineSource, unsigned int LineNo, std::vector<AddressRange>& ranges)

      Returns the address ranges in ``ranges`` corresponding to the line with line number ``lineNo`` in the source file ``lineSource``.

      Searches within this line map.

      Returns ``true`` if at least one range was found.

  .. cpp:function:: const_line_info_iterator begin_by_source() const

      Returns an iterator pointing to the beginning of the underlying source files for the module.

  .. cpp:function:: const_line_info_iterator end_by_source() const

      Returns an iterator marking the end of the sequence provided by :cpp:func:`begin_by_source`.

  .. cpp:function:: LineInformation::const_iterator begin() const

      Returns an iterator pointing to the beginning of the line information for the module.

      This is useful for iterating over the entire line information present in a module.

  .. cpp:function:: LineInformation::const_iterator end() const

      Returns an iterator marking the end of the sequence provided by :cpp:func:`begin`.

  .. cpp:function:: const_iterator find(Offset addressInRange) const

      Returns an iterator to the line map entry starting at the address, ``addressInRange``,
      covered by this line map.

      Returns :cpp:func:`end` if not found.

  .. cpp:function:: const_iterator find(Offset addressInRange, const_iterator hint) const

      Returns an iterator to the line map entry starting at the address, ``addressInRange``,
      covered by this line map starting at the position in ``hint``.

      Returns :cpp:func:`end` if not found.

  .. cpp:function:: unsigned getSize() const

      Returns the number of entries in this map.
