.. _`sec:basetypes.h`:

basetypes.h
###########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:struct:: location_t

  .. cpp:member:: storage_t location
  .. cpp:member:: @location_t_val val
  .. cpp:function:: std::string format() const

.. cpp:struct:: location_t::@location_t_val

  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: Dyninst::MachRegister reg


.. cpp:enum:: storage_t

  .. cpp:enumerator:: loc_address
  .. cpp:enumerator:: loc_register
  .. cpp:enumerator:: loc_unknown

