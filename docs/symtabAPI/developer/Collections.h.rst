.. _`sec:Collections.h`:

Collections.h
#############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: localVarCollection

  Each function will have one of these objects associated with it.
  This object will store all the local variables within this function.

  .. cpp:function:: localVarCollection()
  .. cpp:function:: void addLocalVar(localVarvar)
  .. cpp:function:: localVarfindLocalVar(std::string &name)
  .. cpp:function:: const dyn_c_vector<localVar *> &getAllVars() const

.. cpp:class:: typeCollection

  Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
  So we reference-count to make life easier.

  .. cpp:member:: private static dyn_c_hash_map< void *, typeCollection> fileToTypesMap

      Cache type collections on a per-image basis. (Since BPatch_functions are solitons,
      we don't have to cache them.)

  .. cpp:member:: static boost::mutex create_lock

  .. cpp:function:: typeCollection()
  .. cpp:function:: static void addDeferredLookup(int, dataClass, boost::shared_ptr<Type> *)
  .. cpp:function:: static typeCollection *getModTypeCollection(Module *mod)
  .. cpp:function:: bool dwarfParsed()
  .. cpp:function:: void setDwarfParsed()
  .. cpp:function:: boost::shared_ptr<Type> findType(std::string name, Type::do_share_t)
  .. cpp:function:: Type* findType(std::string n)
  .. cpp:function:: boost::shared_ptr<Type> findType(const int ID, Type::do_share_t)
  .. cpp:function:: Type* findType(const int i)
  .. cpp:function:: boost::shared_ptr<Type> findTypeLocal(std::string name, Type::do_share_t)
  .. cpp:function:: Type* findTypeLocal(std::string n)
  .. cpp:function:: boost::shared_ptr<Type> findTypeLocal(const int ID, Type::do_share_t)
  .. cpp:function:: Type* findTypeLocal(const int i)
  .. cpp:function:: void addType(boost::shared_ptr<Type> type)
  .. cpp:function:: void addType(Type* t)
  .. cpp:function:: void addType(boost::shared_ptr<Type> type, dyn_mutex::unique_lock&)
  .. cpp:function:: void addType(Type* t, dyn_mutex::unique_lock& g)

  .. cpp:function:: template <class T> typename boost::enable_if<boost::integral_constant<bool, \
                    !bool(boost::is_same<Type, T>::value)>, boost::shared_ptr<Type>>::type \
                    addOrUpdateType(boost::shared_ptr<T> type)

  .. cpp:function:: template <class T> T* addOrUpdateType(T* t)


.. cpp:class:: builtInTypeCollection

  This class defines the collection for the built-in Types gnu use negative numbers to define other types
  in terms of these built-in types.
  This collection is global and built in the BPatch_image constructor.
  This means that only one collection of built-in types is made per image.

  .. cpp:function:: builtInTypeCollection()
  .. cpp:function:: boost::shared_ptr<Type> findBuiltInType(std::string &name, Type::do_share_t)
  .. cpp:function:: Type* findBuiltInType(std::string& n)
  .. cpp:function:: boost::shared_ptr<Type> findBuiltInType(const int ID, Type::do_share_t)
  .. cpp:function:: Type* findBuiltInType(const int i)
  .. cpp:function:: void addBuiltInType(boost::shared_ptr<Type>)
  .. cpp:function:: void addBuiltInType(Type* t)
  .. cpp:function:: void getAllBuiltInTypes(std::vector<boost::shared_ptr<Type>>&)
  .. cpp:function:: std::vector<Type*>* getAllBuiltInTypes()


Notes
=====

Some debug formats allow forward references.  Rather than fill in forward in a second
pass, generate placeholder types, and fill them in as we go.  Because we require One
True Pointer for each type, when updating a type, return that One True Pointer.

