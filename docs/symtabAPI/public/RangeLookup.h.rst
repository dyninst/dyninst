.. _`sec:RangeLookup.h`:

RangeLookup.h
#############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:struct:: AddressRange : std::pair<Offset, Offset>

  **An ordered, open interval of address values**

  .. cpp:function:: template <typename T> AddressRange(Dyninst::SimpleInterval<T> i)

      Creates a range with the same bounds as ``i``.

  .. cpp:function:: AddressRange(Offset t)

      Creates a range with the upper and lower bound set to ``t``.

  .. cpp:function:: AddressRange(Offset start, Offset end)

      Creates a range with an upper bound ``start`` and lower bound ``end``.

  .. cpp:function:: bool operator==(const AddressRange& rhs) const

      Checks if this range is equal to ``rhs``.

      Two ranges are equal if they have the same lower and upper bounds.

  .. cpp:function:: bool operator<(const AddressRange& rhs) const

      Compares this range with ``rhs``.

      Ranges are compared by their pairwise lower and upper bounds.

  .. cpp:function:: bool contains(Offset off) const

      Checks if ``off`` is contained in this range.


.. cpp:struct:: template <typename Value> RangeLookupTypes

  .. cpp:type:: typename boost::multi_index::composite_key<Value, \
        boost::multi_index::const_mem_fun<Value, Offset, &Value::startAddr>, \
        boost::multi_index::const_mem_fun<Value, Offset, &Value::endAddr>> \
        addr_range_key

  .. cpp:type:: typename boost::multi_index::composite_key<Value, \
        boost::multi_index::const_mem_fun<Value, Offset, &Value::endAddr>, \
        boost::multi_index::const_mem_fun<Value, Offset, &Value::startAddr>> \
        upper_bound_key

  .. cpp:type:: typename boost::multi_index::composite_key<Value, \
        boost::multi_index::const_mem_fun<Value, unsigned int, &Value::getFileIndex>, \
        boost::multi_index::const_mem_fun<Value, unsigned int, &Value::getLine>> \
        line_info_key

  .. cpp:type:: typename boost::multi_index_container< \
        typename Value::Ptr, \
        boost::multi_index::indexed_by< \
          boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::addr_range>, addr_range_key>, \
          boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::upper_bound>, upper_bound_key>, \
          boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::line_info>, line_info_key>>> \
          type

  .. cpp:type:: typename boost::multi_index::index<type, typename Value::addr_range>::type addr_range_index

  .. cpp:type:: typename boost::multi_index::index<type, typename Value::upper_bound>::type upper_bound_index

  .. cpp:type:: typename boost::multi_index::index<type, typename Value::line_info>::type line_info_index

  .. cpp:type:: typename type::value_type value_type
