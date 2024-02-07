.. _`sec-dev:BPatch_type.h`:

BPatch_type.h
#############

.. cpp:namespace:: dev

.. cpp:class:: BPatch_type

  .. cpp:member:: protected int ID

      unique ID of type

  .. cpp:member:: protected static std::map<Dyninst::SymtabAPI::Type*, BPatch_type *> type_map
  .. cpp:member:: protected BPatch_dataClass type_
  .. cpp:member:: protected boost::shared_ptr<Dyninst::SymtabAPI::Type> typ

    Symtab type

  .. cpp:member:: protected static int USER_BPATCH_TYPE_ID

    For common blocks

  .. cpp:member:: protected unsigned int refCount

  .. cpp:function:: protected virtual ~BPatch_type()
  .. cpp:function:: protected static BPatch_type *findOrCreateType(boost::shared_ptr<Dyninst::SymtabAPI::Type> type)
  .. cpp:function:: protected static BPatch_type *findOrCreateType(Dyninst::SymtabAPI::Type* ty)
  .. cpp:function:: protected BPatch_dataClass convertToBPatchdataClass(Dyninst::SymtabAPI::dataClass type)

    A few convenience functions

  .. cpp:function:: protected Dyninst::SymtabAPI::dataClass convertToSymtabType(BPatch_dataClass type)
  .. cpp:function:: protected static BPatch_type *createFake(const char *_name)
  .. cpp:function:: protected static BPatch_type *createPlaceholder(int _ID, const char *_name = NULL)

    Placeholder for real type, to be filled in later

  .. cpp:function:: void incrRefCount()
  .. cpp:function:: void decrRefCount()
  .. cpp:function:: void fixupUnknowns(BPatch_module *)


.. cpp:class:: BPatch_field

  .. cpp:member:: private int value

    For Enums

  .. cpp:member:: private int size

    For structs and unions

  .. cpp:member:: private Dyninst::SymtabAPI::Field *fld

  .. cpp:function:: protected void copy(BPatch_field &)
  .. cpp:function:: protected void fixupUnknown(BPatch_module *)


.. cpp:class:: BPatch_cblock

  .. cpp:member:: private BPatch_Vector<BPatch_field *> fieldList

    the list of fields

  .. cpp:member:: private BPatch_Vector<BPatch_function *> functions

    which functions use this list

  .. cpp:member:: private Dyninst::SymtabAPI::CBlock *cBlk{}

  .. cpp:function:: private void fixupUnknowns(BPatch_module *)


.. cpp::class BPatch_localvar

  .. cpp:function:: BPatch_localVar(Dyninst::SymtabAPI::localVar *lVar_)
  .. cpp:function:: ~BPatch_localVar()
  .. cpp:function:: BPatch_localVar()
  .. cpp:function:: void fixupUnknown(BPatch_module *)
  .. cpp:function:: Dyninst::SymtabAPI::localVar *getSymtabVar()
  .. cpp:function:: BPatch_storageClass convertToBPatchStorage(Dyninst::VariableLocation *loc)

