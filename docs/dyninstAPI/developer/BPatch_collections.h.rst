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

    Adds the local variable ``var`` to the set of local variables for function.

  .. cpp:function:: BPatch_localVar *findLocalVar(const char *name)

    This function finds a local variable by name and returns a pointer to it or NULL if the local
    variable does not exist in the set of function local variables.

  .. cpp:function:: BPatch_Vector<BPatch_localVar *> *getAllVars()

    Returns all the local variables in the collection.

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

    Returns the the type with name ``name``.

    Searches both builtin and user-defined types.

  .. cpp:function:: BPatch_type *findType(const int &ID)

    Returns the the type with id ``ID``.

    Searches both builtin and user-defined types.

  .. cpp:function:: BPatch_type *findTypeLocal(const char *name)
  .. cpp:function:: BPatch_type *findTypeLocal(const int &ID)
  .. cpp:function:: void addType(BPatch_type *type)

    Add a new type to the type collection.

    When a type is added to the collection, it becomes the collection's responsibility to delete it
    when it is no longer needed. This means that a type allocated on the stack should *NEVER* be
    passed here.

  .. cpp:function:: void addGlobalVariable(const char *name, BPatch_type *type)

    Some debug formats allow forward references. Rather than fill in forward in a second pass, generate placeholder
    types, and fill them in as we go. Because we require One True Pointer for each type, when updating a type,
    return that One True Pointer.

  .. cpp:function:: BPatch_type *findOrCreateType(const int &ID)
  .. cpp:function:: BPatch_type *addOrUpdateType(BPatch_type *type)

  .. cpp:function:: BPatch_type *findVariableType(const char *name)

    The same as :cpp:func:`findType`, but for global variables.

  .. cpp:function:: void clearNumberedTypes()

  .. cpp:member:: private std::unordered_map<std::string, BPatch_type *> typesByName
  .. cpp:member:: private std::unordered_map<std::string, BPatch_type *> globalVarsByName
  .. cpp:member:: private std::unordered_map<int, BPatch_type *> typesByID
  .. cpp:member:: private unsigned refcount
  .. cpp:member:: private static std::unordered_map< std::string, BPatch_typeCollection * > fileToTypesMap
  .. cpp:member:: private bool dwarfParsed_

  .. cpp:function:: private ~BPatch_typeCollection()
  .. cpp:function:: private BPatch_typeCollection()


.. cpp:class:: BPatch_builtInTypeCollection

  This class defines the collection for the built-in Types gnu use negative numbers to define other types in
  terms of these built-in types. This collection is global and built in the BPatch_image constructor. This means
  that only one collection of built-in types is made per image.  jdd 42199

  .. cpp:function:: BPatch_builtInTypeCollection()
  .. cpp:function:: ~BPatch_builtInTypeCollection()
  .. cpp:function:: BPatch_type *findBuiltInType(const char *name)

    Returns the the type with name ``name``

    Searches only builtin types.

  .. cpp:function:: BPatch_type *findBuiltInType(const int &ID)

    Returns the the type with id ``ID``.

    Searches only builtin types.

  .. cpp:function:: void addBuiltInType(BPatch_type *type)
