.. _`sec:ELF_Section.h`:

ELF_Section.h
#############

.. cpp:struct:: ELF_Section

  .. cpp:member:: unsigned int vaddr
  .. cpp:member:: void *data
  .. cpp:member:: unsigned int dataSize
  .. cpp:member:: Elf32_Shdr *shdr
  .. cpp:member:: char *name
  .. cpp:member:: int nameIndx
  .. cpp:member:: unsigned int align
  .. cpp:member:: unsigned int flags
  .. cpp:member:: unsigned int type
  .. cpp:member:: bool loadable


.. cpp:struct:: __Elf32_Dyn

  .. cpp:member:: Elf32_Sword d_tag


.. cpp:union:: __Elf32_Dyn::d_un
 
  .. cpp:member:: Elf32_Sword d_val
  .. cpp:member:: Elf32_Addr d_ptr


.. code:: cpp

  #define DT_NULL   0
  #define DT_NEEDED 1
  #define DT_STRTAB 5
  #define DT_PLTREL 20
  #define DT_PLTGOT 3
  #define DT_HASH 4
  #define DT_SYMTAB 6
  #define DT_RELA 7
  #define DT_INIT 12
  #define DT_FINI 13
  #define DT_REL 17
  #define DT_VERDEF 0x6ffffffc
  #define DT_VERDEFNUM 0x6ffffffd
  #define DT_VERNEED 0x6ffffffe
  #define DT_VERNEEDNUM 0x6fffffff
  #define DT_JMPREL 23
  #define DT_STRSZ 10
  #define DT_CHECKSUM 0x6ffffdf8
  #define DT_DEBUG 21
