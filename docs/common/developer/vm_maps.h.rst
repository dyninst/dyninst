.. _`sec:vm_maps.h`:

vm_maps.h
#########

Virtual Memory Map shared between platforms

.. c:macro:: PREMS_PRIVATE
.. c:macro:: PREMS_SHARED
.. c:macro:: PREMS_READ
.. c:macro:: PREMS_WRITE
.. c:macro:: PREMS_EXEC
.. c:macro:: MAPENTRIES_PATH_SIZE
.. c:macro:: MAPENTRIES_PATH_SIZE_STR


.. cpp:struct:: maps_entries

  .. cpp:type:: Address = Dyninst::Address

  .. cpp:member:: Address start
  .. cpp:member:: Address end
  .. cpp:member:: unsigned prems
  .. cpp:member:: Address offset
  .. cpp:member:: int dev_major
  .. cpp:member:: int dev_minor
  .. cpp:member:: unsigned long inode
  .. cpp:member:: char path[MAPENTRIES_PATH_SIZE]
