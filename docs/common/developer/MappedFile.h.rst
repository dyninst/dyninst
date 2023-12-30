.. _`sec:MappedFile.h`:

MappedFile.h
############

.. cpp:class:: MappedFile

  .. cpp:function:: static MappedFile *createMappedFile(std::string fullpath_)
  .. cpp:function:: static MappedFile *createMappedFile(void *map_loc, unsigned long size_, const std::string &name)
  .. cpp:function:: static void closeMappedFile(MappedFile *&mf)
  .. cpp:function:: std::string filename()
  .. cpp:function:: void *base_addr()
  .. cpp:function:: int getFD()
  .. cpp:function:: unsigned long size()
  .. cpp:function:: MappedFile *clone()
  .. cpp:function:: void setSharing(bool s)
  .. cpp:function:: bool canBeShared()
