.. _`sec:arch-x86.h`:

arch-x86.h
##########

.. cpp:namespace:: NS_x86

.. cpp:class:: ia32_instruction

  .. cpp:function:: ia32_instruction(ia32_memacc* _mac = NULL, ia32_condition* _cnd = NULL, ia32_locations *loc_ = NULL)
  .. cpp:function:: ia32_entry * getEntry()
  .. cpp:function:: unsigned int getSize() const
  .. cpp:function:: unsigned int getPrefixCount() const
  .. cpp:function:: ia32_prefixes * getPrefix()
  .. cpp:function:: unsigned int getLegacyType() const
  .. cpp:function:: bool hasRipRelativeData() const
  .. cpp:function:: const ia32_memacc& getMac(int which) const
  .. cpp:function:: const ia32_condition& getCond() const
  .. cpp:function:: const ia32_locations& getLocationInfo()
  .. cpp:function:: static dyn_hash_map<entryID, flagInfo> const& getFlagTable()
  .. cpp:function:: static void initFlagTable(dyn_hash_map<entryID, flagInfo>&)
