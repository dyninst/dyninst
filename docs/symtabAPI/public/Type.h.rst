Type.h
======

.. cpp:namespace:: Dyninst::SymtabAPI

Class Type
----------

The class ``Type`` represents the types of variables, parameters, return
values, and functions. Instances of this class can represent language
predefined types (e.g. ``int``, ``float``), already defined types in the
Object File or binary (e.g., structures compiled into the binary), or
newly created types (created using the create factory methods of the
corresponding type classes described later in this section) that are
added to SymtabAPI by the user.

As described in Section `2.2 <#subsec:typeInterface>`__, this class
serves as a base class for all the other classes in this interface. An
object of this class is returned from type look up operations performed
through the Symtab class described in Section `6 <#sec:symtabAPI>`__.
The user can then obtain the specific type object from the generic Type
class object. The following example shows how to get the specific object
from a given ``Type`` object returned as part of a look up operation.

.. code-block:: cpp

   // Example shows how to retrieve a structure type object from a given Type object
   using namespace Dyninst;
   using namespace SymtabAPI;

   //Obj represents a handle to a parsed object file using symtabAPI
   //Find a structure type in the object file
   Type *structType = obj->findType("structType1");

   // Get the specific typeStruct object
   typeStruct *stType = structType->isStructType();


.. code-block:: cpp
    
    string &getName()

This method returns the name associated with this type. Each of the
types is represented by a symbolic name. This method retrieves the name
for the type. For example, in the example above "structType1" represents
the name for the ``structType`` object.

.. code-block:: cpp

    bool setName(string zname)

This method sets the name of this type to name. Returns ``true`` on
success and ``false`` on failure.

.. code-block:: cpp

    typedef enumdataEnum, dataPointer, dataFunction, dataSubrange,
    dataArray, dataStructure, dataUnion, dataCommon, dataScalar,
    dataTypedef, dataReference, dataUnknownType, dataNullType, dataTypeClass
    dataClass;


.. code-block:: cpp

    dataClass getDataClass()

This method returns the data class associated with the type. This value
should be used to convert this generic type object to a specific type
object which offers more functionality by using the corresponding query
function described later in this section. For example, if this method
returns ``dataStructure`` then the ``isStructureType()`` should be
called to dynamically cast the ``Type`` object to the ``typeStruct``
object.

.. code-block:: cpp

    typeId_t getID()

This method returns the ID associated with this type. Each type is
assigned a unique ID within the object file. For example an integer
scalar built-in type is assigned an ID -1.

.. code-block:: cpp

    unsigned getSize()

This method returns the total size in bytes occupied by the type.

.. code-block:: cpp

    typeEnum *getEnumType()

If this ``Type`` hobject represents an enum type, then return the object
casting the ``Type`` object to ``typeEnum`` otherwise return ``NULL``.

.. code-block:: cpp

    typePointer *getPointerType()

If this ``Type`` object represents an pointer type, then return the
object casting the ``Type`` object to ``typePointer`` otherwise return
``NULL``.

.. code-block:: cpp

    typeFunction *getFunctionType()

If this ``Type`` object represents an ``Function`` type, then return the
object casting the ``Type`` object to ``typeFunction`` otherwise return
``NULL``.

.. code-block:: cpp

    typeRange *getSubrangeType()

If this ``Type`` object represents a ``Subrange`` type, then return the
object casting the ``Type`` object to ``typeSubrange`` otherwise return
``NULL``.

.. code-block:: cpp

    typeArray *getArrayType()

If this ``Type`` object represents an ``Array`` type, then return the
object casting the ``Type`` object to ``typeArray`` otherwise return
``NULL``.

.. code-block:: cpp

    typeStruct *getStructType()

If this ``Type`` object represents a ``Structure`` type, then return the
object casting the ``Type`` object to ``typeStruct`` otherwise return
``NULL``.

.. code-block:: cpp

    typeUnion *getUnionType()

If this ``Type`` object represents a ``Union`` type, then return the
object casting the ``Type`` object to ``typeUnion`` otherwise return
``NULL``.

.. code-block:: cpp

    typeScalar *getScalarType()

If this ``Type`` object represents a ``Scalar`` type, then return the
object casting the ``Type`` object to ``typeScalar`` otherwise return
``NULL``.

.. code-block:: cpp

    typeCommon *getCommonType()

If this ``Type`` object represents a ``Common`` type, then return the
object casting the ``Type`` object to ``typeCommon`` otherwise return
``NULL``.

.. code-block:: cpp

    typeTypedef *getTypedefType()

If this ``Type`` object represents a ``TypeDef`` type, then return the
object casting the ``Type`` object to ``typeTypedef`` otherwise return
``NULL``.

.. code-block:: cpp

    typeRef *getRefType()

If this ``Type`` object represents a ``Reference`` type, then return the
object casting the ``Type`` object to ``typeRef`` otherwise return
``NULL``.

Class typeEnum
--------------

This class represents an enumeration type containing a list of constants
with values. This class is derived from ``Type``, so all those member
functions are applicable. ``typeEnum`` inherits from the ``Type`` class.

.. code-block:: cpp

    static typeEnum *create(string &name, vector<pair<string, int> *>
    &consts, Symtab *obj = NULL) static typeEnum *create(string &name,
    vector<string> &constNames, Symtab *obj)

These factory methods create a new enumerated type. There are two
variations to this function. ``consts`` supplies the names and Ids of
the constants of the enum. The first variant is used when user-defined
identifiers are required; the second variant is used when system-defined
identifiers will be used. The newly created type is added to the
``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
to any object file, but it will be available for further queries.

.. code-block:: cpp

    bool addConstant(const string &constname, int value)

This method adds a constant to an enum type with name ``constName`` and
value ``value``. Returns ``true`` on success and ``false`` on failure.

.. code-block:: cpp

    std::vector<std::pair<std::string, int> > &getConstants();

This method returns the vector containing the enum constants represented
by a (name, value) pair of the constant.

.. code-block:: cpp

    bool setName(const char* name)

This method sets the new name of the enum type to ``name``. Returns
``true`` if it succeeds, else returns ``false``.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the enum type is compatible with the
given type ``type`` or else returns ``false``.

Class typeFunction
------------------

This class represents a function type, containing a list of parameters
and a return type. This class is derived from ``Type``, so all the
member functions of class ``Type`` are applicable. ``typeFunction``
inherits from the ``Type`` class.

.. code-block:: cpp

    static typeFunction *create(string &name, Type *retType, vector<Type*> &paramTypes, Symtab *obj = NULL)


This factory method creates a new function type with name ``name``.
``retType`` represents the return type of the function and
``paramTypes`` is a vector of the types of the parameters in order. The
the newly created type is added to the ``Symtab`` object ``obj``. If
``obj`` is ``NULL`` the type is not added to any object file, but it
will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the function type is compatible with the
given type ``type`` or else returns ``false``.

.. code-block:: cpp

    bool addParam(Type *type)

This method adds a new function parameter with type ``type`` to the
function type. Returns ``true`` if it succeeds, else returns ``false``.

.. code-block:: cpp

    Type *getReturnType() const

This method returns the return type for this function type. Returns
``NULL`` if there is no return type associated with this function type.

.. code-block:: cpp

    bool setRetType(Type *rtype)

This method sets the return type of the function type to ``rtype``.
Returns ``true`` if it succeeds, else returns ``false``.

.. code-block:: cpp

    bool setName(string &name)

This method sets the new name of the function type to ``name``. Returns
``true`` if it succeeds, else returns ``false``.

.. code-block:: cpp

    vector< Type *> &getParams() const

This method returns the vector containing the individual parameters
represented by their types in order. Returns ``NULL`` if there are no
parameters to the function type.

Class typeScalar
----------------

This class represents a scalar type. This class is derived from
``Type``, so all the member functions of class ``Type`` are applicable.
``typeScalar`` inherits from the Type class.

.. code-block:: cpp

    static typeScalar *create(string &name, int size, Symtab *obj = NULL)

This factory method creates a new scalar type. The ``name`` field is
used to specify the name of the type, and the ``size`` parameter is used
to specify the size in bytes of each instance of the type. The newly
created type is added to the ``Symtab`` object ``obj``. If ``obj`` is
``NULL`` the type is not added to any object file, but it will be
available for further queries.

.. code-block:: cpp

    bool isSigned()

This method returns ``true`` if the scalar type is signed or else
returns ``false``.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the scalar type is compatible with the
given type ``type`` or else returns ``false``.

Class Field
-----------

This class represents a field in a container. For e.g. a field in a
structure/union type.

.. code-block:: cpp

    typedef enum visPrivate, visProtected, visPublic, visUnknown visibility_t;

A handle for identifying the visibility of a certain ``Field`` in a
container type. This can represent private, public, protected or
unknown(default) visibility.

.. code-block:: cpp

    Field(string &name, Type *type, visibility_t vis = visUnknown)

This constructor creates a new field with name ``name``, type ``type``
and visibility ``vis``. This newly created ``Field`` can be added to a
container type.

.. code-block:: cpp

    const string &getName()

This method returns the name associated with the field in the container.

.. code-block:: cpp

    Type *getType()

This method returns the type associated with the field in the container.

.. code-block:: cpp

    int getOffset()

This method returns the offset associated with the field in the
container.

.. code-block:: cpp

    visibility_t getVisibility()

This method returns the visibility associated with a field in a
container. This returns ``visPublic`` for the variables within a common
block.

Class fieldListType
-------------------

This class represents a container type. It is one of the three
categories of types as described in Section
`2.2 <#subsec:typeInterface>`__. The structure and the union types fall
under this category. This class is derived from ``Type``, so all the
member functions of class ``Type`` are applicable. ``fieldListType``
inherits from the ``Type`` class.

.. code-block:: cpp

    vector<Field *> *getComponents()

This method returns the list of all fields present in the container.
This gives information about the name, type and visibility of each of
the fields. Returns ``NULL`` of there are no fields.

.. code-block:: cpp

    void addField(std::string fieldname, Type *type, int offsetVal = -1,
    visibility_t vis = visUnknown)

This method adds a new field at the end to the container type with field
name ``fieldname``, type ``type`` and type visibility ``vis``.

.. code-block:: cpp

    void addField(unsigned num, std::string fieldname, Type *type, int
    offsetVal = -1, visibility_t vis = visUnknown)

This method adds a field after the field with number ``num`` with field
name ``fieldname``, type ``type`` and type visibility ``vis``.

.. code-block:: cpp

    void addField(Field *fld)

This method adds a new field ``fld`` to the container type.

.. code-block:: cpp

    void addField(unsigned num, Field *fld)

This method adds a field ``fld`` after field ``num`` to the container
type.

Class typeStruct : public fieldListType
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

|  
| This class represents a structure type. The structure type is a
  special case of the container type. The fields of the structure
  represent the fields in this case. As a subclass of class
  ``fieldListType``, all methods in ``fieldListType`` are applicable.

.. code-block:: cpp

    static typeStruct *create(string &name, vector<pair<string, Type *>*> &flds, Symtab *obj = NULL)

This factory method creates a new struct type. The name of the structure
is specified in the ``name`` parameter. The ``flds`` vector specifies
the names and types of the fields of the structure type. The newly
created type is added to the ``Symtab`` object ``obj``. If ``obj`` is
``NULL`` the type is not added to any object file, but it will be
available for further queries.

.. code-block:: cpp

    static typeStruct *create(string &name, vector<Field *> &fields, Symtab *obj = NULL)

This factory method creates a new struct type. The name of the structure
is specified in the ``name`` parameter. The ``fields`` vector specifies
the fields of the type. The newly created type is added to the
``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
to any object file, but it will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the struct type is compatible with the
given type ``type`` or else returns ``false``.

Class typeUnion
~~~~~~~~~~~~~~~

|  
| This class represents a union type, a special case of the container
  type. The fields of the union type represent the fields in this case.
  As a subclass of class ``fieldListType``, all methods in
  ``fieldListType`` are applicable. ``typeUnion`` inherits from the
  ``fieldListType`` class.

.. code-block:: cpp

    static typeUnion *create(string &name, vector<pair<string, Type *>*> &flds, Symtab *obj = NULL)

This factory method creates a new union type. The name of the union is
specified in the ``name`` parameter. The ``flds`` vector specifies the
names and types of the fields of the union type. The newly created type
is added to the ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the
type is not added to any object file, but it will be available for
further queries.

.. code-block:: cpp

    static typeUnion *create(string &name, vector<Field *> &fields, Symtab *obj = NULL)

This factory method creates a new union type. The name of the structure
is specified in the ``name`` parameter. The ``fields`` vector specifies
the fields of the type. The newly created type is added to the
``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not added
to any object file, but it will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the union type is compatible with the
given type ``type`` or else returns ``false``.

Class typeCommon
~~~~~~~~~~~~~~~~

|  
| This class represents a common block type in fortran, a special case
  of the container type. The variables of the common block represent the
  fields in this case. As a subclass of class ``fieldListType``, all
  methods in ``fieldListType`` are applicable. ``typeCommon`` inherits
  from the ``Type`` class.

.. code-block:: cpp

    vector<CBlocks *> *getCBlocks()

This method returns the common block objects for the type. The methods
of the ``CBlock`` can be used to access information about the members of
a common block. The vector returned by this function contains one
instance of ``CBlock`` for each unique definition of the common block.

Class CBlock
~~~~~~~~~~~~

|  
| This class represents a common block in Fortran. Multiple functions
  can share a common block.

.. code-block:: cpp

    bool getComponents(vector<Field *> *vars)

This method returns the vector containing the individual variables of
the common block. Returns ``true`` if there is at least one variable,
else returns ``false``.

.. code-block:: cpp

    bool getFunctions(vector<Symbol *> *funcs)

This method returns the functions that can see this common block with
the set of variables described in ``getComponents`` method above.
Returns ``true`` if there is at least one function, else returns
``false``.

Class derivedType
-----------------

This class represents a derived type which is a reference to another
type. It is one of the three categories of types as described in Section
`2.2 <#subsec:typeInterface>`__. The pointer, reference and the typedef
types fall under this category. This class is derived from ``Type``, so
all the member functions of class ``Type`` are applicable.

.. code-block:: cpp

    Type *getConstituentType() const

This method returns the type of the base type to which this type refers
to.

Class typePointer
~~~~~~~~~~~~~~~~~

|  
| This class represents a pointer type, a special case of the derived
  type. The base type in this case is the type this particular type
  points to. As a subclass of class ``derivedType``, all methods in
  ``derivedType`` are also applicable.

.. code-block:: cpp

    static typePointer *create(string &name, Type *ptr, Symtab *obj = NULL) static typePointer *create(string &name, Type *ptr, int size, Symtab *obj = NULL)

These factory methods create a new type, named ``name``, which points to
objects of type ``ptr``. The first form creates a pointer whose size is
equal to sizeof(void*) on the target platform where the application is
running. In the second form, the size of the pointer is the value passed
in the ``size`` parameter. The newly created type is added to the
``Symtab`` object ``obj``. If obj is ``NULL`` the type is not added to
any object file, but it will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the Pointer type is compatible with the
given type ``type`` or else returns ``false``.

.. code-block:: cpp

    bool setPtr(Type *ptr)

This method sets the pointer type to point to the type in ``ptr``.
Returns ``true`` if it succeeds, else returns ``false``.

Class typeTypedef
~~~~~~~~~~~~~~~~~

|  
| This class represents a ``typedef`` type, a special case of the
  derived type. The base type in this case is the ``Type``. This
  particular type is typedefed to. As a subclass of class
  ``derivedType``, all methods in ``derivedType`` are also applicable.

.. code-block:: cpp

    static typeTypedef *create(string &name, Type *ptr, Symtab *obj = NULL)

This factory method creates a new type called ``name`` and having the
type ``ptr``. The newly created type is added to the ``Symtab`` object
``obj``. If ``obj`` is ``NULL`` the type is not added to any object
file, but it will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the typedef type is compatible with the
given type ``type`` or else returns ``false``.

Class typeRef
~~~~~~~~~~~~~

|  
| This class represents a reference type, a special case of the derived
  type. The base type in this case is the ``Type`` this particular type
  refers to. As a subclass of class ``derivedType``, all methods in
  ``derivedType`` are also applicable here.


.. code-block:: cpp

    static typeRef *create(string &name, Type *ptr, Symtab * obj = NULL)

This factory method creates a new type, named ``name``, which is a
reference to objects of type ``ptr``. The newly created type is added to
the ``Symtab`` object ``obj``. If ``obj`` is ``NULL`` the type is not
added to any object file, but it will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the ref type is compatible with the
given type ``type`` or else returns ``false``.

Class rangedType
----------------

This class represents a range type with a lower and an upper bound. It
is one of the three categories of types as described in section
`2.2 <#subsec:typeInterface>`__. The sub-range and the array types fall
under this category. This class is derived from ``Type``, so all the
member functions of class ``Type`` are applicable.

.. code-block:: cpp

    unsigned long getLow() const

This method returns the lower bound of the range. This can be the lower
bound of the range type or the lowest index for an array type.

.. code-block:: cpp

    unsigned long getHigh() const

This method returns the higher bound of the range. This can be the
higher bound of the range type or the highest index for an array type.

Class typeSubrange
~~~~~~~~~~~~~~~~~~

|  
| This class represents a sub-range type. As a subclass of class
  ``rangedType``, all methods in ``rangedType`` are applicable here.
  This type is usually used to represent a sub-range of another type.
  For example, a ``typeSubrange`` can represent a sub-range of the array
  type or a new integer type can be declared as a sub range of the
  integer using this type.

.. code-block:: cpp

    static typeSubrange *create(string &name, int size, int low, int hi, symtab *obj = NULL)

This factory method creates a new sub-range type. The name of the type
is ``name``, and the size is ``size``. The lower bound of the type is
represented by ``low``, and the upper bound is represented by ``high``.
The newly created type is added to the ``Symtab`` object ``obj``. If
``obj`` is ``NULL`` the type is not added to any object file, but it
will be available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if this sub range type is compatible with
the given type ``type`` or else returns ``false``.

Class typeArray
~~~~~~~~~~~~~~~

|  
| This class represents an ``Array`` type. As a subclass of class
  ``rangedType``, all methods in ``rangedType`` are applicable.

.. code-block:: cpp

    static typeArray *create(string &name, Type *type, int low, int hi, Symtab *obj = NULL)

This factory method creates a new array type. The name of the type is
``name``, and the type of each element is ``type``. The index of the
first element of the array is ``low``, and the last is ``high``. The
newly created type is added to the ``Symtab`` object ``obj``. If ``obj``
is ``NULL`` the type is not added to any object file, but it will be
available for further queries.

.. code-block:: cpp

    bool isCompatible(Type *type)

This method returns ``true`` if the array type is compatible with the
given type ``type`` or else returns ``false``.

.. code-block:: cpp

    Type *getBaseType() const

This method returns the base type of this array type.