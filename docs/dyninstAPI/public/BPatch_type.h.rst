.. _`sec:BPatch_type.h`:

BPatch_type.h
#############

These are the DyninstAPI versions of the SymtabAPI :ref:`types <sec:Type.h>`.

The Dyninst type system is based on the notion of structural
equivalence. Structural equivalence was selected to allow the system the
greatest flexibility in allowing users to write mutators that work with
applications compiled both with and without debugging symbols enabled.
Using the create* methods of the BPatch class, a mutator can construct
type definitions for existing mutatee structures. This information
allows a mutator to read and write complex types even if the application
program has been compiled without debugging information. However, if the
application has been compiled with debugging information, Dyninst will
verify the type compatibility of the operations performed by the
mutator.

The rules for type computability are that two types must be of the same
storage class (i.e. arrays are only compatible with other arrays) to be
type compatible. In addition, if either of the types is the type
:cpp:enumerator:`BPatch_dataClass::BPatch_dataUnknownType`, then
the two types are compatible. Variables in mutatee programs that have
not been compiled with debugging symbols (or in the symbols are in a
format that the Dyninst library does not recognize) will be of type
``BPatch_dataUnknownType``.


.. cpp:class:: BPatch_type

  **Language-level type**

  Used to describe ypes of variables, parameters, return values, and functions. Instances of the class can
  represent language predefined types (e.g. int, float), mutatee defined types (e.g., structures compiled
  into the mutatee application), or mutator defined types.

  .. cpp:function:: BPatch_type(const char *name = NULL, int _ID = 0, BPatch_dataClass c = BPatch_dataNullType)
  .. cpp:function:: BPatch_type(boost::shared_ptr<Dyninst::SymtabAPI::Type> typ_)
  .. cpp:function:: BPatch_type(Dyninst::SymtabAPI::Type* t)
  .. cpp:function:: virtual bool operator==(const BPatch_type &) const
  .. cpp:function:: int getID() const
  .. cpp:function:: unsigned int getSize()
  .. cpp:function:: boost::shared_ptr<Dyninst::SymtabAPI::Type> getSymtabType(Dyninst::SymtabAPI::Type::do_share_t) const
  .. cpp:function:: Dyninst::SymtabAPI::Type* getSymtabType() const
  .. cpp:function:: const char *getName() const

    Returns the name of the type.

  .. cpp:function:: BPatch_dataClass getDataClass() const

    Return one of the above data classes for this type.

  .. cpp:function:: unsigned long getLow() const

    Returns the lower bound of an array.

    Calling this on non-array types produces an undefined result.

  .. cpp:function:: unsigned long getHigh() const

    Returns the upper bound of an array.

    Calling this on non-array types produces an undefined result.

  .. cpp:function:: BPatch_Vector<BPatch_field *> * getComponents() const

    Return a vector of the types of the fields in a BPatch_struct or
    BPatch_union. If this method is invoked on a type whose BPatch_dataClass
    is not BPatch_struct or BPatch_union, NULL is returned.

  .. cpp:function:: bool isCompatible(BPatch_type* otype)

    Return true if otype is type compatible with this type. The rules for
    type compatibility are given in Section 4.28. If the two types are not
    type compatible, the error reporting callback function will be invoked
    one or more times with additional information about why the types are
    not compatible.

  .. cpp:function:: BPatch_type *getConstituentType() const

    Return the type of the base type. For a BPatch_array this is the type of
    each element, for a BPatch_pointer this is the type of the object the
    pointer points to. For BPatch_typedef types, this is the original type.
    For all other types, NULL is returned.

  .. cpp:function:: BPatch_Vector<BPatch_cblock *> *getCblocks() const

    Return the common block classes for the type. The methods of the
    BPatch_cblock can be used to access information about the member of a
    common block. Since the same named (or anonymous) common block can be
    defined with different members in different functions, a given common
    block may have multiple definitions. The vector returned by this
    function contains one instance of BPatch_cblock for each unique
    definition of the common block. If this method is invoked on a type
    whose BPatch_dataClass is not BPatch_common, NULL will be returned.


.. cpp:class:: BPatch_field

  **A field in a enum, struct, or union**

  A field can be an atomic type, i.e, int  char, or more complex like a
  union or struct.

  .. cpp:function:: BPatch_field(BPatch_field &f)
  .. cpp:function:: BPatch_field(Dyninst::SymtabAPI::Field *fld_ = NULL,\
                                 BPatch_dataClass typeDescriptor = BPatch_dataUnknownType,\
                                 int value_ = 0, int size_ = 0)

  .. cpp:function:: ~BPatch_field()
  .. cpp:function:: BPatch_field & operator=(BPatch_field &src)
  .. cpp:function:: const char* getName()
  .. cpp:function:: BPatch_type* getType()
  .. cpp:function:: int getValue()
  .. cpp:function:: BPatch_visibility getVisibility()
  .. cpp:function:: BPatch_dataClass getTypeDesc()
  .. cpp:function:: int getSize()
  .. cpp:function:: int getOffset()


.. cpp:class:: BPatch_cblock
   
  **A common block in Fortran**

  .. cpp:function:: BPatch_cblock(Dyninst::SymtabAPI::CBlock *cBlk_)
  .. cpp:function:: BPatch_cblock()

  .. cpp:function:: BPatch_Vector<BPatch_field *> * getComponents()

    Return a vector containing the individual variables of the common block.

  .. cpp:function:: BPatch_Vector<BPatch_function *> * getFunctions()

    Return a vector of the functions that can see this common block with the
    set of fields described in getComponents. However, other functions that
    define this common block with a different set of variables (or sizes of
    any variable) will not be returned.


.. cpp:class:: BPatch_localVar

  **Information about local variables**

  It is desgined store information about a variable in a function.

  .. cpp:function:: const char * getName()
  .. cpp:function:: BPatch_type * getType()
  .. cpp:function:: int getLineNum()
  .. cpp:function:: long getFrameOffset()
  .. cpp:function:: int getRegister()
  .. cpp:function:: BPatch_storageClass getStorageClass()


.. cpp:enum:: symDescr_t
  
  .. cpp:enumerator:: BPatchSymLocalVar

    local variable- gnu sun-(empty)

  .. cpp:enumerator:: BPatchSymGlobalVar

    global variable- gnu sun-'G'

  .. cpp:enumerator:: BPatchSymRegisterVar

    register variable- gnu sun-'r'

  .. cpp:enumerator:: BPatchSymStaticLocalVar

    static local variable- gnu sun-'V'

  .. cpp:enumerator:: BPatchSymStaticGlobal

    static global variable- gnu- sun'S'

  .. cpp:enumerator:: BPatchSymLocalFunc

    local function- gnu sun-'f'

  .. cpp:enumerator:: BPatchSymGlobalFunc

    global function- gnu- sun'F'

  .. cpp:enumerator:: BPatchSymFuncParam

    function paramater - gnu- sun'p'

  .. cpp:enumerator:: BPatchSymTypeName

    type name- gnu sun-'t'

  .. cpp:enumerator:: BPatchSymAggType

    aggregate type-struct,union, enum- gnu sun-'T'

  .. cpp:enumerator:: BPatchSymTypeTag

    C++ type name and tag combination


.. cpp:enum:: BPatch_dataClass

  .. cpp:enumerator:: BPatch_dataScalar

    Scalars are compatible if their names are the same (as defined by
    strcmp) and their sizes are the same.

  .. cpp:enumerator:: BPatch_dataEnumerated

    Enumerated types are compatible if they have the same number of elements
    and the identifiers of the elements are the same.

  .. cpp:enumerator:: BPatch_dataTypeClass
  .. cpp:enumerator:: BPatch_dataStructure

  .. cpp:enumerator:: BPatch_dataUnion

    Structures and unions are compatible if they have the same number of
    constituent parts (fields) and item by item each field is type
    compatible with the corresponds field of the other type.

  .. cpp:enumerator:: BPatch_dataArray

    Arrays are compatible if they have the same number of elements
    (regardless of their lower and upper bounds) and the base element types
    are type compatible.

  .. cpp:enumerator:: BPatch_dataPointer

    Pointers are compatible if the types they point to are compatible.

  .. cpp:enumerator:: BPatch_dataReference

  .. cpp:enumerator:: BPatch_dataFunction

    Functions are compatible if their return types are compatible, they have
    same number of parameters, and position by position each element of the
    parameter list is type compatible.

  .. cpp:enumerator:: BPatch_dataTypeAttrib
  .. cpp:enumerator:: BPatch_dataUnknownType
  .. cpp:enumerator:: BPatch_dataMethod
  .. cpp:enumerator:: BPatch_dataCommon
  .. cpp:enumerator:: BPatch_dataPrimitive
  .. cpp:enumerator:: BPatch_dataTypeNumber
  .. cpp:enumerator:: BPatch_dataTypeDefine
  .. cpp:enumerator:: BPatch_dataNullType



.. cpp:enum:: BPatch_visibility

  .. cpp:enumerator:: BPatch_private

    gnu Sun -- private

  .. cpp:enumerator:: BPatch_protected

    gnu Sun -- protected

  .. cpp:enumerator:: BPatch_public

    gnu Sun -- public

  .. cpp:enumerator:: BPatch_optimized

    gnu Sun -- field optimized out and is public

  .. cpp:enumerator:: BPatch_visUnknown

    visibility not known or doesn't apply(ANSIC), the default


.. cpp:enum:: BPatch_storageClass

  **Encodes how a variable is stored**

  .. cpp:enumerator:: BPatch_storageAddr

    Absolute address of variable.

  .. cpp:enumerator:: BPatch_storageAddrRef

    Address of pointer to variable.

  .. cpp:enumerator:: BPatch_storageReg

    Register which holds variable value.

  .. cpp:enumerator:: BPatch_storageRegRef

    Register which holds pointer to variable.

  .. cpp:enumerator:: BPatch_storageRegOffset

    Address of variable ``$reg + address``.

  .. cpp:enumerator:: BPatch_storageFrameOffset

    Address of variable ``$fp  + address``.


.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:function:: boost::shared_ptr<Type> convert(const BPatch_type *, Type::do_share_t)
.. cpp:function:: Type* convert(const BPatch_type* t)


Notes
*****

These aliases are provided for backwards compatibility only. Do not use.

.. code:: cpp

  #define BPatch_scalar BPatch_dataScalar
  #define BPatch_enumerated BPatch_dataEnumerated
  #define BPatch_typeClass  BPatch_dataTypeClass
  #define BPatch_structure  BPatch_dataStructure
  #define BPatch_union  BPatch_dataUnion
  #define BPatch_array  BPatch_dataArray
  #define BPatch_pointer  BPatch_dataPointer
  #define BPatch_reference  BPatch_dataReferance
  #define BPatch_typeAttrib BPatch_dataTypeAttrib
  #define BPatch_unknownType  BPatch_dataUnknownType
  #define BPatch_typeDefine BPatch_dataTypeDefine

