.. _`sec:StringTable.h`:

StringTable.h
#############

.. cpp:namespace:: Dyninst::SymtabAPI

.. code:: cpp

    namespace bmi = boost::multi_index

.. cpp:type:: boost::multi_index_container<StringTableEntry, bmi::indexed_by< \
                bmi::random_access<>, bmi::ordered_non_unique< \
                bmi::member<StringTableEntry, const std::string, &StringTableEntry::str>>, \
                bmi::ordered_non_unique< \
                bmi::member<StringTableEntry, const std::string, &StringTableEntry::filename>>>> \
              StringTableBase

.. cpp:type:: boost::shared_ptr<StringTable> StringTablePtr

.. cpp:struct:: StringTableEntry

  This is being used for storing filenames.

  .. cpp:function:: StringTableEntry(std::string s, std::string f)

  .. cpp:member:: std::string str

      Usually the full filename, it depends on what was the filename stored

  .. cpp:member:: std::string filename

      To be only the filename and extension without path. Ex: "foo.txt"

  .. cpp:function:: bool operator==(std::string s) const


.. cpp:struct:: StringTable : public StringTableBase

  .. cpp:member:: dyn_mutex lock
