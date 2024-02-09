.. _`sec:BPatch_collections.h`:

BPatch_collections.h
####################


.. cpp:class:: BPatch_localVarCollection

  This class contains a collection of local variables. Each function will have one of these objects
   associated with it. This object will store all the local variables within this function.

   .. note:: This class is unaware of scope.

  .. cpp:function:: BPatch_localVarCollection()
  .. cpp:function:: ~BPatch_localVarCollection()
  .. cpp:function:: void addLocalVar(BPatch_localVar *var)
  .. cpp:function:: BPatch_localVar *findLocalVar(const char *name)
  .. cpp:function:: BPatch_Vector<BPatch_localVar *> *getAllVars()

.. cpp:class:: BPatch_typeCollection

  Due to DWARF weirdness, this can be shared between multiple BPatch_modules. We reference-count
  to make life easier.

  .. cpp:function:: static BPatch_typeCollection *getGlobalTypeCollection()

    DWARF: Cache type collections on a per-image basis. Since BPatch_functions are solitons, we don't
    have to cache them.

  .. cpp:function:: static BPatch_typeCollection *getModTypeCollection(BPatch_module *mod)
  .. cpp:function:: static void freeTypeCollection(BPatch_typeCollection *tc)
  .. cpp:function:: bool dwarfParsed()
  .. cpp:function:: void setDwarfParsed()
  .. cpp:function:: BPatch_type *findType(const char *name)
  .. cpp:function:: BPatch_type *findType(const int &ID)
  .. cpp:function:: BPatch_type *findTypeLocal(const char *name)
  .. cpp:function:: BPatch_type *findTypeLocal(const int &ID)
  .. cpp:function:: void addType(BPatch_type *type)
  .. cpp:function:: void addGlobalVariable(const char *name, BPatch_type *type)

    Some debug formats allow forward references. Rather than fill in forward in a second pass, generate placeholder
    types, and fill them in as we go. Because we require One True Pointer for each type, when updating a type,
    return that One True Pointer.

  .. cpp:function:: BPatch_type *findOrCreateType(const int &ID)
  .. cpp:function:: BPatch_type *addOrUpdateType(BPatch_type *type)
  .. cpp:function:: BPatch_type *findVariableType(const char *name)
  .. cpp:function:: void clearNumberedTypes()

.. cpp:class:: BPatch_builtInTypeCollection

  This class defines the collection for the built-in Types gnu use negative numbers to define other types in
  terms of these built-in types. This collection is global and built in the BPatch_image constructor. This means
  that only one collection of built-in types is made per image.  jdd 42199

  .. cpp:function:: BPatch_builtInTypeCollection()
  .. cpp:function:: ~BPatch_builtInTypeCollection()
  .. cpp:function:: BPatch_type *findBuiltInType(const char *name)
  .. cpp:function:: BPatch_type *findBuiltInType(const int &ID)
  .. cpp:function:: void addBuiltInType(BPatch_type *type)
