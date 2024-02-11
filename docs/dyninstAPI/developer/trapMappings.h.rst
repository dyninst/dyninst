.. _`sec:trapMappings.h`:

trapMappings.h
##############

.. cpp:class:: trampTrapMappings

  .. cpp:member:: private dyn_hash_map<Dyninst::Address, tramp_mapping_t> mapping
  .. cpp:member:: private std::set<tramp_mapping_t *> updated_mappings
  .. cpp:function:: private static void arrange_mapping(tramp_mapping_t &m, bool should_sort, std::vector<tramp_mapping_t *> &mappings_to_add, std::vector<tramp_mapping_t *> &mappings_to_update)
  .. cpp:member:: private bool needs_updating
  .. cpp:member:: private AddressSpace *as
  .. cpp:member:: private const int_variable *trapTableUsed
  .. cpp:member:: private const int_variable *trapTableVersion
  .. cpp:member:: private const int_variable *trapTable
  .. cpp:member:: private const int_variable *trapTableSorted
  .. cpp:function:: private void writeToBuffer(unsigned char *buffer, unsigned long val, unsigned addr_width)
  .. cpp:function:: private void writeTrampVariable(const int_variable *var, unsigned long val)
  .. cpp:member:: private unsigned long table_version
  .. cpp:member:: private unsigned long table_used
  .. cpp:member:: private unsigned long table_allocated
  .. cpp:member:: private unsigned long table_mutatee_size
  .. cpp:member:: private Dyninst::Address current_table
  .. cpp:member:: private Dyninst::Address table_header
  .. cpp:member:: private bool blockFlushes
  .. cpp:function:: trampTrapMappings(AddressSpace *a)
  .. cpp:function:: void copyTrapMappings(trampTrapMappings *parent)
  .. cpp:function:: void clearTrapMappings()
  .. cpp:function:: void addTrapMapping(Dyninst::Address from, Dyninst::Address to, bool write_to_mutatee = false)
  .. cpp:function:: Dyninst::Address getTrapMapping(Dyninst::Address from)
  .. cpp:function:: bool definesTrapMapping(Dyninst::Address from)
  .. cpp:function:: bool needsUpdating()
  .. cpp:function:: void flush()
  .. cpp:function:: void allocateTable()
  .. cpp:function:: void shouldBlockFlushes(bool b)
  .. cpp:function:: bool empty()
  .. cpp:function:: AddressSpace *proc() const


.. cpp:struct:: trampTrapMappings::tramp_mapping_t

  .. cpp:member:: Dyninst::Address from_addr
  .. cpp:member:: Dyninst::Address to_addr
  .. cpp:member:: bool written
  .. cpp:member:: bool mutatee_side
  .. cpp:member:: unsigned cur_index

