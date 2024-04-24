.. _`sec-dev:Type.h`:

Type.h
######

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Type : public AnnotatableDense

  .. cpp:member:: static const std::size_t max_size

      Maximum memory needed to store any of the possible Types.

  .. cpp:member:: protected typeId_t ID_

      Unique ID of type.

  .. cpp:member:: protected std::string name_
  .. cpp:member:: protected unsigned int  size_

      Size of type in **bytes**.

  .. cpp:member:: protected dataClass   type_

  .. cpp:member:: protected bool updatingSize

      Set to ``true`` when we're in the process of
      calculating the size of container structures.  This helps avoid
      infinite recursion for self-referential types.  Getting a
      self-referential type probably signifies an error in another
      part of the type processing code (or an error in the binary).

  .. cpp:member:: private boost::weak_ptr<Type> self_;

      For carrying the reference count across the older pointer-based API.

  .. cpp:function:: Type(std::string name, typeId_t ID, dataClass dataTyp = dataNullType)
  .. cpp:function:: Type(std::string name, dataClass dataTyp = dataNullType)
  .. cpp:function:: Type()
  .. cpp:function:: virtual ~Type() = default
  .. cpp:function:: Type& operator=(const Type&) = default
  .. cpp:function:: template<class T, class... Args> static boost::shared_ptr<T> make_shared(Args&&... args)
  .. cpp:function:: boost::shared_ptr<Type> reshare()
  .. cpp:function:: void reshare(boost::shared_ptr<Type> const & sp)

  .. cpp:function:: bool setName(std::string n)

      This method sets the name of this type to name. Returns ``true`` on
      success and ``false`` on failure.

  .. cpp:function:: protected virtual void updateSize()
  .. cpp:function:: protected virtual void merge( Type *)
  .. cpp:function:: virtual bool operator==(const Type &) const
  .. cpp:function:: virtual void fixupUnknowns(Module *)
  .. cpp:function:: bool setSize(unsigned int size)
  .. cpp:function:: void updateUniqueTypeId(typeId_t)
  .. cpp:function:: typeId_t getUniqueTypeId()


  .. cpp:function:: static unique_ptr_Type createFake(std::string name)

      Placeholder for real type, to be filled in later.

  .. cpp:function:: static unique_ptr_Type createPlaceholder(typeId_t ID, std::string name = "")

      Placeholder for real type, to be filled in later.


.. cpp:class:: typeEnum : public derivedType

  .. cpp:member:: private bool is_scoped_

      C++11 scoped enum (i.e., 'enum class')?

  .. cpp:function:: typeEnum() = default
  .. cpp:function:: typeEnum(boost::shared_ptr<Type> underlying_type, std::string name)
  .. cpp:function:: typeEnum(boost::shared_ptr<Type> underlying_type, std::string name, typeId_t ID)
  .. cpp:function:: typeEnum(boost::shared_ptr<Type> underlying_type, std::string name, typeId_t ID, scoped_t)
  .. cpp:function:: static typeEnum* create(string &name, vector<pair<string, int>*>& consts, Symtab *obj = NULL)
  .. cpp:function:: static typeEnum *create(string &name, vector<string> &constNames, Symtab *obj)

      These factory methods create a new enumerated type. There are two
      variations to this function. ``consts`` supplies the names and IDs of
      the constants of the enum. The first variant is used when user-defined
      identifiers are required; the second variant is used when system-defined
      identifiers will be used. The newly created type is added to the
      ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
      to any object file, but it will be available for further queries.

  .. cpp:function:: bool setName(const char* name)

      This method sets the new name of the enum type to ``name``. Returns
      ``true`` if it succeeds, else returns ``false``.

  .. cpp:function:: bool addConstant(const std::string &fieldname,int value)

      This method adds a constant to an enum type with name ``constName`` and
      value ``value``. Returns ``true`` on success and ``false`` on failure.


.. cpp:class:: typeFunction : public Type

  .. cpp:member:: private boost::shared_ptr<Type> retType_

      Return type of the function

  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typeFunction()
  .. cpp:function:: typeFunction(typeId_t ID, boost::shared_ptr<Type> retType, std::string name = "")
  .. cpp:function:: typeFunction(typeId_t i, Type* r, std::string n = "")
  .. cpp:function:: typeFunction(boost::shared_ptr<Type> retType, std::string name = "")
  .. cpp:function:: typeFunction(Type* retType, std::string name = "")

  .. cpp:function:: static typeFunction *create(std::string &name, boost::shared_ptr<Type> retType, dyn_c_vector<boost::shared_ptr<Type>> &paramTypes, Symtab *obj = NULL)
  .. cpp:function:: static typeFunction* create(std::string &n, Type* rt, dyn_c_vector<Type*> &p, Symtab* o = NULL)

      This factory method creates a new function type with name ``name``.
      ``retType`` represents the return type of the function and
      ``paramTypes`` is a vector of the types of the parameters in order. The
      the newly created type is added to the ``Symtab`` object ``obj``. If
      ``obj`` is ``NULL`` the type is not added to any object file, but it
      will be available for further queries.

  .. cpp:function:: bool addParam(boost::shared_ptr<Type> type)
  .. cpp:function:: bool addParam(Type* t)

      This method adds a new function parameter with type ``type`` to the
      function type. Returns ``true`` if it succeeds, else returns ``false``.

  .. cpp:function:: bool setRetType(boost::shared_ptr<Type> rtype)
  .. cpp:function:: bool setRetType(Type* t)

      This method sets the return type of the function type to ``rtype``.
      Returns ``true`` if it succeeds, else returns ``false``.

  .. cpp:function:: bool setName(string &name)

      This method sets the new name of the function type to ``name``. Returns
      ``true`` if it succeeds, else returns ``false``.


.. cpp:class:: typeScalar : public Type

  .. cpp:function:: typeScalar() = default
  .. cpp:function:: typeScalar(typeId_t ID, unsigned int size, std::string name, properties_t p)
  .. cpp:function:: typeScalar(typeId_t ID, unsigned int size, std::string name = "", bool isSigned = false)
  .. cpp:function:: typeScalar(unsigned int size, std::string name = "", bool isSigned = false)
  .. cpp:function:: static typeScalar *create(std::string &name, int size, Symtab *obj = NULL)

      This factory method creates a new scalar type. The ``name`` field is
      used to specify the name of the type, and the ``size`` parameter is used
      to specify the size in bytes of each instance of the type. The newly
      created type is added to the ``Symtab`` object ``obj``. If ``obj`` is
      ``NULL`` the type is not added to any object file, but it will be
      available for further queries.


.. cpp:struct:: typeScalar::properties_t

  See DwarfWalker::parseBaseType for how these are computed


.. cpp:class:: Field : public AnnotatableDense

  .. cpp:function:: Field(std::string n, Type* t, int ov = -1, visibility_t v = visUnknown)

      This constructor creates a new field with name ``name``, type ``type``
      and visibility ``vis``. This newly created ``Field`` can be added to a
      container type.

  .. cpp:function:: protected void copy(Field &)
  .. cpp:function:: Field()
  .. cpp:function:: Field(std::string name, boost::shared_ptr<Type> typ, int offsetVal = -1, visibility_t vis = visUnknown)
  .. cpp:function:: Field(Field &f)
  .. cpp:function:: virtual ~Field()
  .. cpp:function:: void fixupUnknown(Module *)
  .. cpp:function:: virtual bool operator==(const Field &) const


.. cpp:class:: fieldListType : public Type, public fieldListInterface

  .. cpp:function:: virtual void postFieldInsert(int nsize) = 0

      Each subclass may need to update its size after adding a field.

  .. cpp:function:: void addField(std::string n, Type* t, int ov = -1, visibility_t v = visUnknown)

      This method adds a new field at the end to the container type with field
      name ``fieldname``, type ``type`` and type visibility ``vis``.

  .. cpp:function:: void addField(unsigned n, std::string f, Type* t, int o = -1, visibility_t v = visUnknown)

      This method adds a field after the field with number ``num`` with field
      name ``fieldname``, type ``type`` and type visibility ``vis``.

  .. cpp:function:: void addField(Field *fld)

      This method adds a new field ``fld`` to the container type.

  .. cpp:function:: void addField(unsigned num, Field *fld)

      This method adds a field ``fld`` after field ``num`` to the container type.

  .. cpp:member:: protected dyn_c_vector<Field *> fieldList
  .. cpp:member:: protected dyn_c_vector<Field *> *derivedFieldList
  .. cpp:function:: protected fieldListType(std::string &name, typeId_t ID, dataClass typeDes)
  .. cpp:function:: fieldListType()
  .. cpp:function:: ~fieldListType()
  .. cpp:function:: fieldListType& operator=(const fieldListType&) = default
  .. cpp:function:: bool operator==(const Type &) const
  .. cpp:function:: bool operator==(const fieldListType &otype) const
  .. cpp:function:: void addField(std::string fieldname, boost::shared_ptr<Type> type, int offsetVal = -1, visibility_t vis = visUnknown)
  .. cpp:function:: void addField(unsigned num, std::string fieldname, boost::shared_ptr<Type> type, int offsetVal = -1, visibility_t vis = visUnknown)


.. cpp:class:: typeStruct : public fieldListType

  .. cpp:function:: static typeStruct *create(string &name, vector<pair<string, Type *>*> &flds, Symtab *obj = NULL)

      This factory method creates a new struct type. The name of the structure
      is specified in the ``name`` parameter. The ``flds`` vector specifies
      the names and types of the fields of the structure type. The newly
      created type is added to the ``Symtab`` object ``obj``. If ``obj`` is
      ``NULL`` the type is not added to any object file, but it will be
      available for further queries.

  .. cpp:function:: static typeStruct *create(string &name, vector<Field *> &fields, Symtab *obj = NULL)

      This factory method creates a new struct type. The name of the structure
      is specified in the ``name`` parameter. The ``fields`` vector specifies
      the fields of the type. The newly created type is added to the
      ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
      to any object file, but it will be available for further queries.

  .. cpp:function:: protected void updateSize()
  .. cpp:function:: protected void postFieldInsert(int nsize)
  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: protected void merge(Type *other)
  .. cpp:function:: typeStruct()
  .. cpp:function:: typeStruct(typeId_t ID, std::string name = "")
  .. cpp:function:: typeStruct(std::string name)
  .. cpp:function:: static typeStruct *create(std::string &name, dyn_c_vector< std::pair<std::string, boost::shared_ptr<Type> > *> &flds, Symtab *obj = NULL)
  .. cpp:function:: static typeStruct *create(std::string &n, dyn_c_vector<std::pair<std::string, Type*>*> &f, Symtab *o = NULL)
  .. cpp:function:: static typeStruct *create(std::string &name, dyn_c_vector<Field *> &fields, Symtab *obj = NULL)


.. cpp:class:: typeUnion : public fieldListType

  .. cpp:function:: static typeUnion *create(string &name, vector<pair<string, Type *>*> &flds, Symtab *obj = NULL)

      This factory method creates a new union type. The name of the union is
      specified in the ``name`` parameter. The ``flds`` vector specifies the
      names and types of the fields of the union type. The newly created type
      is added to the ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the
      type is not added to any object file, but it will be available for
      further queries.

  .. cpp:function:: static typeUnion *create(string &name, vector<Field *> &fields, Symtab *obj = NULL)

      This factory method creates a new union type. The name of the structure
      is specified in the ``name`` parameter. The ``fields`` vector specifies
      the fields of the type. The newly created type is added to the
      ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
      to any object file, but it will be available for further queries.

  .. cpp:function:: protected void updateSize()
  .. cpp:function:: protected void postFieldInsert(int nsize)
  .. cpp:function:: protected void merge(Type *other)
  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typeUnion()
  .. cpp:function:: typeUnion(typeId_t ID, std::string name = "")
  .. cpp:function:: typeUnion(std::string name)
  .. cpp:function:: static typeUnion *create(std::string &name, dyn_c_vector<std::pair<std::string, boost::shared_ptr<Type>> *> &fieldNames, Symtab *obj = NULL)
  .. cpp:function:: static typeUnion *create(std::string &n, dyn_c_vector<std::pair<std::string, Type*>*> &f, Symtab *o = NULL)
  .. cpp:function:: static typeUnion *create(std::string &name, dyn_c_vector<Field *> &fields,  Symtab *obj = NULL)


.. cpp:class:: typeCommon : public fieldListType

  .. cpp:function:: protected void postFieldInsert(int nsize)
  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typeCommon()
  .. cpp:function:: typeCommon(typeId_t ID, std::string name = "")
  .. cpp:function:: typeCommon(std::string name)
  .. cpp:function:: static typeCommon *create(std::string &name, Symtab *obj = NULL)


.. cpp:class:: CBlock : public AnnotatableSparse

  .. cpp:member:: private dyn_c_vector<Symbol*> functions

      Which functions use this list. Should probably be updated to use aggregates.

  .. cpp:function:: void fixupUnknowns(Module *)


.. cpp:class:: derivedType : public Type, public derivedInterface

  .. cpp:member:: protected boost::shared_ptr<Type> baseType_
  .. cpp:function:: protected derivedType(std::string &name, typeId_t id, int size, dataClass typeDes)
  .. cpp:function:: protected derivedType(std::string &name, int size, dataClass typeDes)
  .. cpp:function:: derivedType()
  .. cpp:function:: bool operator==(const Type &) const
  .. cpp:function:: bool operator==(const derivedType &otype) const


.. cpp:class:: typePointer : public derivedType

  .. cpp:function:: static typePointer *create(string &name, Type *ptr, Symtab *obj = NULL)
  .. cpp:function:: static typePointer *create(string &name, Type *ptr, int size, Symtab *obj = NULL)

      These factory methods create a new type, named ``name``, which points to
      objects of type ``ptr``. The first form creates a pointer whose size is
      equal to sizeof(void*) on the target platform where the application is
      running. In the second form, the size of the pointer is the value passed
      in the ``size`` parameter. The newly created type is added to the
      ``Symtab`` object ``obj``. If obj is ``NULL`` the type is not added to
      any object file, but it will be available for further queries.

  .. cpp:function:: bool setPtr(boost::shared_ptr<Type> ptr)
  .. cpp:function:: bool setPtr(Type* ptr)

      This method sets the pointer type to point to the type in ``ptr``.
      Returns ``true`` if it succeeds, else returns ``false``.

  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typePointer()
  .. cpp:function:: typePointer(typeId_t ID, boost::shared_ptr<Type> ptr, std::string name = "")
  .. cpp:function:: typePointer(typeId_t i, Type* p, std::string n = "")
  .. cpp:function:: typePointer(boost::shared_ptr<Type> ptr, std::string name = "")
  .. cpp:function:: typePointer(Type* p, std::string n = "")
  .. cpp:function:: static typePointer *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab *obj = NULL)
  .. cpp:function:: static typePointer *create(std::string &n, Type* p, Symtab *o = NULL)
  .. cpp:function:: static typePointer *create(std::string &name, boost::shared_ptr<Type> ptr, int size, Symtab *obj = NULL)
  .. cpp:function:: static typePointer *create(std::string &n, Type* p, int s, Symtab *o = NULL)


.. cpp:class:: typeTypedef: public derivedType

  .. cpp:function:: static typeTypedef *create(string &name, Type *ptr, Symtab *obj = NULL)

      This factory method creates a new type called ``name`` and having the
      type ``ptr``. The newly created type is added to the ``Symtab`` object
      ``obj``. If ``obj`` is ``NULL`` the type is not added to any object
      file, but it will be available for further queries.

  .. cpp:function:: protected void updateSize()
  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typeTypedef()
  .. cpp:function:: typeTypedef(typeId_t ID, boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint = 0)
  .. cpp:function:: typeTypedef(typeId_t i, Type* b, std::string n, unsigned int s = 0)
  .. cpp:function:: typeTypedef(boost::shared_ptr<Type> base, std::string name, unsigned int sizeHint = 0)
  .. cpp:function:: typeTypedef(Type* b, std::string n, unsigned int s = 0)
  .. cpp:function:: static typeTypedef *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab *obj = NULL)
  .. cpp:function:: static typeTypedef *create(std::string &n, Type* p, Symtab *o = NULL)
  .. cpp:function:: bool operator==(const Type &otype) const
  .. cpp:function:: bool operator==(const typeTypedef &otype) const


.. cpp:class:: typeRef : public derivedType

  .. cpp:function:: static typeRef *create(string &name, Type *ptr, Symtab * obj = NULL)

      This factory method creates a new type, named ``name``, which is a
      reference to objects of type ``ptr``. The newly created type is added to
      the ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not
      added to any object file, but it will be available for further queries.

  .. cpp:member:: struct rvalue_t final{}
  .. cpp:function:: protected void fixupUnknowns(Module *)
  .. cpp:function:: typeRef()
  .. cpp:function:: typeRef(typeId_t ID, boost::shared_ptr<Type> refType, std::string name)
  .. cpp:function:: typeRef(typeId_t ID, boost::shared_ptr<Type> refType, std::string name, rvalue_t)
  .. cpp:function:: typeRef(typeId_t i, Type* r, std::string n)
  .. cpp:function:: typeRef(boost::shared_ptr<Type> refType, std::string name)
  .. cpp:function:: typeRef(Type* r, std::string n)
  .. cpp:function:: static typeRef *create(std::string &name, boost::shared_ptr<Type> ptr, Symtab * obj = NULL)
  .. cpp:function:: static typeRef *create(std::string &n, Type* p, Symtab * o = NULL)
  .. cpp:function:: bool operator==(const Type &otype) const
  .. cpp:function:: bool operator==(const typeRef &otype) const


.. cpp:class:: rangedType : public Type, public rangedInterface

  .. cpp:member:: protected unsigned long low_
  .. cpp:member:: protected unsigned long hi_
  .. cpp:function:: protected rangedType(std::string &name, typeId_t ID, dataClass typeDes, int size, unsigned long low, unsigned long hi)
  .. cpp:function:: protected rangedType(std::string &name, dataClass typeDes, int size, unsigned long low, unsigned long hi)
  .. cpp:function:: rangedType()
  .. cpp:function:: bool operator==(const Type &) const
  .. cpp:function:: bool operator==(const rangedType &otype) const


.. cpp:class:: typeSubrange : public rangedType

  .. cpp:function:: static typeSubrange *create(string &name, int size, int low, int hi, symtab *obj = NULL)

      This factory method creates a new sub-range type. The name of the type
      is ``name``, and the size is ``size``. The lower bound of the type is
      represented by ``low``, and the upper bound is represented by ``high``.
      The newly created type is added to the ``Symtab`` object ``obj``. If
      ``obj`` is ``NULL`` the type is not added to any object file, but it
      will be available for further queries.

  .. cpp:function:: typeSubrange()
  .. cpp:function:: typeSubrange(typeId_t ID, int size, long low, long hi, std::string name)
  .. cpp:function:: typeSubrange( int size, long low, long hi, std::string name)
  .. cpp:function:: static typeSubrange *create(std::string &name, int size, long low, long hi, Symtab *obj = NULL)


.. cpp:class:: typeArray : public rangedType

  .. cpp:function:: static typeArray *create(string &name, Type *type, int low, int hi, Symtab *obj = NULL)

      This factory method creates a new array type. The name of the type is
      ``name``, and the type of each element is ``type``. The index of the
      first element of the array is ``low``, and the last is ``high``. The
      newly created type is added to the ``Symtab`` object ``obj``. If ``obj``
      is ``NULL`` the type is not added to any object file, but it will be
      available for further queries.

  .. cpp:function:: protected void updateSize()
  .. cpp:function:: protected void merge(Type *other)
  .. cpp:function:: typeArray()
  .. cpp:function:: typeArray(typeId_t ID, boost::shared_ptr<Type> base, long low, long hi, std::string name, unsigned int sizeHint = 0)
  .. cpp:function:: typeArray(typeId_t i, Type* b, long l, long h, std::string n, unsigned int s = 0)
  .. cpp:function:: typeArray(boost::shared_ptr<Type> base, long low, long hi, std::string name, unsigned int sizeHint = 0)
  .. cpp:function:: typeArray(Type* b, long l, long h, std::string n, unsigned int s = 0)
  .. cpp:function:: static typeArray *create(std::string &name, boost::shared_ptr<Type> typ,  long low, long hi, Symtab *obj = NULL)
  .. cpp:function:: static typeArray *create(std::string &n, Type* t,  long l, long h, Symtab *o = NULL)
  .. cpp:function:: bool operator==(const Type &otype) const
  .. cpp:function:: bool operator==(const typeArray &otype) const
  .. cpp:function:: void fixupUnknowns(Module *)

.. cpp:var:: static boost::atomic<typeId_t> user_type_id

  This is the ID that is decremented for each type a user defines. It is
  global so that every type that the user defines has a unique ID.
