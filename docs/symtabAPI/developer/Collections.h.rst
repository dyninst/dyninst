.. _`sec:Collections.h`:

Collections.h
#############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: localVarCollection

  Each function will have one of these objects associated with it.
  This object will store all the local variables within this function.

  .. cpp:member:: private dyn_c_vector<localVar *> localVars
  .. cpp:function:: private bool addItem_impl(localVar *)
  .. cpp:function:: localVarCollection()
  .. cpp:function:: ~localVarCollection()
  .. cpp:function:: void addLocalVar(localVar *var)

  .. cpp:function:: localVar *findLocalVar(std::string &name)

    Finds a local variable with name ``name``.

    Returns ``NULL`` if the variable does not exist.

  .. cpp:function:: const dyn_c_vector<localVar *> &getAllVars() const

    Returns all the local variables in the collection.

.. cpp:class:: typeCollection

  Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
  So we reference-count to make life easier.

  .. cpp:member:: static boost::mutex create_lock
  .. cpp:member:: private dyn_c_hash_map<std::string, boost::shared_ptr<Type>> typesByName
  .. cpp:member:: private dyn_c_hash_map<std::string, boost::shared_ptr<Type>> globalVarsByName
  .. cpp:member:: private dyn_c_hash_map<int, boost::shared_ptr<Type>> typesByID

  .. cpp:member:: private static dyn_c_hash_map<void *, typeCollection *> fileToTypesMap

      Cache type collections on a per-image basis. (Since BPatch_functions are solitons,
      we don't have to cache them.)

  .. cpp:function:: private static bool doDeferredLookups(typeCollection *)
  .. cpp:member:: private bool dwarfParsed_

  .. cpp:function:: typeCollection()
  .. cpp:function:: ~typeCollection()
  .. cpp:function:: static void addDeferredLookup(int, dataClass, boost::shared_ptr<Type> *)
  .. cpp:function:: static typeCollection *getModTypeCollection(Module *mod)
  .. cpp:function:: bool dwarfParsed()
  .. cpp:function:: void setDwarfParsed()

  .. cpp:function:: boost::shared_ptr<Type> findType(std::string name, Type::do_share_t)

    Retrieves the type with name ``n``.

    Returns ``NULL`` if not found.

  .. cpp:function:: Type* findType(std::string n)

    Retrieves the type with name ``n``.

    Returns ``NULL`` if not found.

  .. cpp:function:: boost::shared_ptr<Type> findType(const int ID, Type::do_share_t)

    Retrieves the type with ID ``ID``.

    Returns ``NULL`` if not found.

  .. cpp:function:: Type* findType(const int i)

    Retrieves the type with ID ``ID``.

    Returns ``NULL`` if not found.

  .. cpp:function:: boost::shared_ptr<Type> findTypeLocal(std::string name, Type::do_share_t)

    Retrieves the type with name ``n``.

    Returns ``NULL`` if not found.

  .. cpp:function:: Type* findTypeLocal(std::string n)

    Retrieves the type with name ``n``.

    Returns ``NULL`` if not found.

  .. cpp:function:: boost::shared_ptr<Type> findTypeLocal(const int ID, Type::do_share_t)

    Retrieves the type with ID ``ID``.

    Returns ``NULL`` if not found.

  .. cpp:function:: Type* findTypeLocal(const int i)

    Retrieves the type with ID ``ID``.

    Returns ``NULL`` if not found.

  .. cpp:function:: void addType(boost::shared_ptr<Type> type)
  .. cpp:function:: void addType(Type *t)
  .. cpp:function:: void addType(boost::shared_ptr<Type> type, dyn_mutex::unique_lock &)
  .. cpp:function:: void addType(Type *t, dyn_mutex::unique_lock &g)
  .. cpp:function:: void addGlobalVariable(boost::shared_ptr<Type> type)
  .. cpp:function:: void addGlobalVariable(Type *t)
  .. cpp:function:: boost::shared_ptr<Type> findOrCreateType(const int ID, Type::do_share_t)
  .. cpp:function:: Type *findOrCreateType(const int i)

  .. cpp:function:: template <class T> typename boost::enable_if<boost::integral_constant<bool, \
                    !bool(boost::is_same<Type, T>::value)>, boost::shared_ptr<Type>>::type \
                    addOrUpdateType(boost::shared_ptr<T> type)

  .. cpp:function:: template <class T> T *addOrUpdateType(T *t)
  .. cpp:function:: boost::shared_ptr<Type> findVariableType(std::string &name, Type::do_share_t)
  .. cpp:function:: Type *findVariableType(std::string &n)
  .. cpp:function:: void getAllTypes(std::vector<boost::shared_ptr<Type>> &)
  .. cpp:function:: std::vector<Type *> *getAllTypes()
  .. cpp:function:: void getAllGlobalVariables(std::vector<std::pair<std::string, boost::shared_ptr<Type>>> &)
  .. cpp:function:: std::vector<std::pair<std::string, Type *>> *getAllGlobalVariables()
  .. cpp:function:: void clearNumberedTypes()


.. cpp:class:: builtInTypeCollection

  This class defines the collection for the built-in Types gnu use negative numbers to define other types
  in terms of these built-in types.
  This collection is global and built in the BPatch_image constructor.
  This means that only one collection of built-in types is made per image.

  .. cpp:function:: builtInTypeCollection()
  .. cpp:function:: ~builtInTypeCollection()
  .. cpp:function:: boost::shared_ptr<Type> findBuiltInType(std::string &name, Type::do_share_t)
  .. cpp:function:: Type *findBuiltInType(std::string &n)
  .. cpp:function:: boost::shared_ptr<Type> findBuiltInType(const int ID, Type::do_share_t)
  .. cpp:function:: Type *findBuiltInType(const int i)
  .. cpp:function:: void addBuiltInType(boost::shared_ptr<Type>)
  .. cpp:function:: void addBuiltInType(Type *t)
  .. cpp:function:: void getAllBuiltInTypes(std::vector<boost::shared_ptr<Type>> &)
  .. cpp:function:: std::vector<Type *> *getAllBuiltInTypes()

Notes
=====

Some debug formats allow forward references.  Rather than fill in forward in a second
pass, generate placeholder types, and fill them in as we go.  Because we require One
True Pointer for each type, when updating a type, return that One True Pointer.

