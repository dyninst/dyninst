.. _`sec-dev:BPatch_module.h`:

BPatch_module.h
###############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_module: public BPatch_sourceObj

  .. cpp:function:: mapped_module* lowlevel_mod()

    This function should go away when paradyn is on top of dyninst

  .. cpp:function:: BPatch_module(BPatch_addressSpace *_addSpace, AddressSpace *as, mapped_module *_mod, BPatch_image *img)
  .. cpp:function:: virtual ~BPatch_module()
  .. cpp:function:: bool getSourceObj(BPatch_Vector<BPatch_sourceObj *>&)
  .. cpp:function:: BPatch_sourceObj *getObjParent()
  .. cpp:function:: void parseTypes()
  .. cpp:function:: void setDefaultNamespacePrefix(char *name)
  .. cpp:function:: void handleUnload()
  .. cpp:function:: bool isExploratoryModeOn()

      true if exploratory or defensive mode is on

  .. cpp:function:: bool setAnalyzedCodeWriteable(bool writeable)

      sets write perm's analyzed code pages

  .. cpp:function:: bool isSystemLib()
  .. cpp:function:: bool remove(BPatch_function*)
  .. cpp:function:: bool remove(instPoint*)

  .. cpp:function:: bool dumpMangled(char *prefix)

    Prints all <mangled> function names in this module

  .. cpp:function:: private void parseDwarfTypes()

    We understand the type information in DWARF format.
