.. _`sec:Type.h`:

Type.h
######

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:type:: int typeId_t

.. cpp:class:: Type : public AnnotatableDense

  **Language-level types for variables, parameters, return values, and functions**

  These can represent language-defined types (e.g. ``int``, ``float``), types defined in a
  binary (e.g., structures compiled into the binary), or user-defined types provided by extending
  this class.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`virtual bool isCompatible(Type *oType)`.

  .. cpp:function:: virtual bool isCompatible(Type *oType)

      Checks if this type is compatibility with ``oType``.

      Compatibility is determined by each type independently.

  .. cpp:function:: typeId_t getID() const

      Returns the ID associated with this type.

      Each type is assigned a unique ID within the object file.

  .. cpp:function:: unsigned int getSize()

      Returns the total size in **bytes** occupied by the type.

  .. cpp:function:: std::string &getName()

      Returns the name associated with this type.

      Each type is represented by a symbolic name. For builtin language types (e.g., 'int'),
      a default name is provided. For user-defined types (e.g., 'class myClass'), the name
      from the debugging information is used, if present.

  .. cpp:function:: std::string specificType()

      Returns a string representation of the class name (e.g., "typeEnum").

  .. cpp:function:: dataClass getDataClass() const

      Returns the data class associated with the type.

      This value can be used to convert a generic type to a specific type.

  ......

  .. rubric:: Down-cast conversions

  .. cpp:function:: typeArray* getArrayType()

      Returns the current type as an instance of :cpp:class:`typeArray`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline typeArray& asArrayType()

      The same as :cpp:func:`getArrayType`.

  .. cpp:function:: inline bool isArrayType()

      Checks if this is an instance of :cpp:class:`typeArray`.

  .. cpp:function:: typeCommon* getCommonType()

      Returns the current type as an instance of :cpp:class:`typeCommon`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline typeCommon& asCommonType()

      The same as :cpp:func:`getCommonType`.

  .. cpp:function:: inline bool isCommonType()

      Checks if this is an instance of :cpp:class:`typeCommon`.

  .. cpp:function:: inline derivedType& asDerivedType()

      Returns the current type as an instance of :cpp:class:`derivedType`.

      If the conversion fails, throws ``std::bad_cast``. Users should check
      :cpp:func:`isDerivedType` before calling.

  .. cpp:function:: inline bool isDerivedType()

      Checks if this is an instance of :cpp:class:`derivedType`.

  .. cpp:function:: typeEnum* getEnumType()

      Returns the current type as an instance of :cpp:class:`typeEnum`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline typeEnum& asEnumType()

      The same as :cpp:func:`getEnumType`.

  .. cpp:function:: inline bool isEnumType()

      Checks if this is an instance of :cpp:class:`typeEnum`.

  .. cpp:function:: inline fieldListType& asFieldListType()

      Returns the current type as an instance of :cpp:class:`fieldListType`.

      If the conversion fails, throws ``std::bad_cast``. Users should check
      :cpp:func:`isfieldListType` before calling.

  .. cpp:function:: inline bool isfieldListType()

      Checks if this is an instance of :cpp:class:`fieldListType`.

  .. cpp:function:: typeFunction* getFunctionType()

      Returns the current type as an instance of :cpp:class:`typeFunction`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline typeFunction& asFunctionType()

      The same as :cpp:func:`getFunctionType`.

  .. cpp:function:: typePointer* getPointerType()

      Returns the current type as an instance of :cpp:class:`typePointer`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline rangedType& asRangedType()

      Returns the current type as an instance of :cpp:class:`rangedType`.

      If the conversion fails, throws ``std::bad_cast``. Users should check
      :cpp:func:`isRangedType` before calling.

  .. cpp:function:: inline bool isRangedType()

      Checks if this is an instance of :cpp:class:`rangedType`.

  .. cpp:function:: typeRef* getRefType()

      Returns the current type as an instance of :cpp:class:`typeRef`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: typeScalar* getScalarType()

      Returns the current type as an instance of :cpp:class:`typeScalar`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: typeStruct* getStructType()

      Returns the current type as an instance of :cpp:class:`typeStruct`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: inline bool isStructType()

      Checks if this is an instance of :cpp:class:`typeStruct`.

  .. cpp:function:: typeSubrange* getSubrangeType()

      Returns the current type as an instance of :cpp:class:`typeSubrange`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: typeTypedef* getTypedefType()

      Returns the current type as an instance of :cpp:class:`typeTypedef`.

      Returns ``NULL`` if not convertible.

  .. cpp:function:: typeUnion* getUnionType()

      Returns the current type as an instance of :cpp:class:`typeUnion`.

      Returns ``NULL`` if not convertible.


.. cpp:enum:: Type::do_share_t

  .. cpp:enumerator:: share

.. cpp:enum:: dataClass

  .. cpp:enumerator:: dataEnum
  .. cpp:enumerator:: dataPointer
  .. cpp:enumerator:: dataFunction
  .. cpp:enumerator:: dataSubrange
  .. cpp:enumerator:: dataArray
  .. cpp:enumerator:: dataStructure
  .. cpp:enumerator:: dataUnion
  .. cpp:enumerator:: dataCommon
  .. cpp:enumerator:: dataScalar
  .. cpp:enumerator:: dataTypedef
  .. cpp:enumerator:: dataReference
  .. cpp:enumerator:: dataUnknownType
  .. cpp:enumerator:: dataNullType
  .. cpp:enumerator:: dataTypeClass


.. cpp:enum:: visibility_t

  **C++ access specifier for class members**

  .. cpp:enumerator:: visPrivate
  .. cpp:enumerator:: visProtected
  .. cpp:enumerator:: visPublic
  .. cpp:enumerator:: visUnknown

    Unknown or doesn't apply (i.e., not C++)


.. cpp:class:: Type::unique_ptr_Type

  **Fake unique_ptr type**

  unique_ptr_Type(Type* p)
  operator boost::shared_ptr<Type>()
  operator Type*()


.. cpp:function:: const char *dataClass2Str(dataClass dc)
.. cpp:function:: const char *visibility2Str(visibility_t v)


.. cpp:class:: typeArray : public rangedType

  **A sequence of values contiguous in memory**

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two arrays are compatible if they have the same number of elements and
      have compatible base types.

  .. cpp:function:: boost::shared_ptr<Type> getBaseType(Type::do_share_t) const

      The same as :cpp:func:`Type* getBaseType() const`.

  .. cpp:function:: Type* getBaseType() const

      Returns the base type of this array.


.. cpp:class:: typeCommon : public fieldListType

  **A common block type in Fortran**

  .. cpp:function:: dyn_c_vector<CBlock*>* getCblocks() const

      Returns the common block objects.

  .. cpp:function:: void beginCommonBlock()
  .. cpp:function:: void endCommonBlock(Symbol *, void *baseAddr)


.. cpp:class:: CBlock : public AnnotatableSparse

  **An element of a common block in Fortran**

  .. cpp:function:: dyn_c_vector<Field*>* getComponents()

      Returns the variables of the common block.

  .. cpp:function:: dyn_c_vector<Symbol*>* getFunctions()

      Returns the functions that can see this common block.


.. cpp:class:: derivedInterface

  **Requirement for all derived types**

  .. cpp:function:: virtual boost::shared_ptr<Type> getConstituentType(Type::do_share_t) const = 0

      The same as :cpp:func:`Type* getConstituentType() const`.

  .. cpp:function:: Type* getConstituentType() const

      Returns the underlying type.


.. cpp:class:: derivedType : public Type, public derivedInterface

  **A reference to another type**

  Examples are pointers, references, and typedefs.

  .. cpp:function:: Type* getConstituentType() const

      Returns the type of the base type to which this type refers to.

  .. cpp:function:: boost::shared_ptr<Type> getConstituentType(Type::do_share_t) const

      The same as :cpp:func:`getConstituentType`.


.. cpp:class:: typeEnum : public derivedType

  **A list of named constants with values**.

  .. cpp:function:: bool is_scoped() const noexcept

      Checks if this is a C++ scoped enum (aka 'enum class').

  .. cpp:function:: dyn_c_vector<std::pair<std::string, int>>& getConstants()

      ThisReturns the named constants and their values.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two enums are compatibile if they have the same underlying type, the same
      number of elements, and each named constant and value is the same and in
      the same order.

.. cpp:struct:: typeEnum::scoped_t final

  **A marker class for C++11 scoped enums**


.. cpp:class:: typeFunction : public Type

  **A block of executable code with a return type and an optional list of parameters**

  .. cpp:function:: boost::shared_ptr<Type> getReturnType(Type::do_share_t) const

      The same as :cpp:func:`Type *getReturnType() const`.

  .. cpp:function:: Type *getReturnType() const

      Returns the return type for this function.

  .. cpp:function:: dyn_c_vector<boost::shared_ptr<Type>> &getParams()

      Returns the formal parameters.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two functions are compatible if their return types are compatible, they have the same
      number of parameters, and each parameter's type is compatible.


.. cpp:class:: fieldListInterface

  **Requirement for all fieldList types**

  .. cpp:function:: virtual dyn_c_vector<Field*>* getComponents() const = 0

      Returns all fields in the container type.


.. cpp:class:: fieldListType : public Type, public fieldListInterface

  **A container type**

  Examples of container types are ``struct``, ``union``, and ``class``.

  .. cpp:function:: dyn_c_vector<Field*>* getComponents() const

      Returns all fields in the container.

  .. cpp:function:: dyn_c_vector<Field*> *getFields() const

      The same as :cpp:func:`getComponents`.


.. cpp:class:: Field : public AnnotatableDense

  **A field in a container**

  For example, a data member of a struct or union type.

  .. cpp:function:: std::string &getName()

      Returns the field's name as it appears in the source code.

  .. cpp:function:: Type* getType()

      Returns the field's type.

  .. cpp:function:: int getOffset()

      Returns the offset relative to the beginning of the container.

  .. cpp:function:: visibility_t getVisibility()

      Returns the field's visibility.

      Note:: ``visPublic`` is used for variables within a common block.

  .. cpp:function:: boost::shared_ptr<Type> getType(Type::do_share_t)

      Returns this type in a sharable pointer.

  .. cpp:function:: unsigned int getSize()

      Returns the size in **bytes** of this type, as defined by the compiled source.

      This is taken directly from the debugging information and may or may not take into
      account the architecture-specific sizes and padding requirements.


.. cpp:class:: typePointer : public derivedType

  **A pointer**

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two pointers are compatible if the types of the pointed-to objects are compatible.


.. cpp:class:: rangedInterface

  **Requirement for all ranged types**

  .. cpp:function:: virtual unsigned long getLow() const = 0

      Returns the upper bound of the range.

  .. cpp:function:: virtual unsigned long getHigh() const  = 0

      Returns the lower bound of the range.


.. cpp:class:: rangedType : public Type, public rangedInterface

  **A range with a lower and upper bound**

  .. cpp:function:: unsigned long getLow() const

      Returns the lower bound of the range.

      This can be the lower bound of the range type or the lowest index for an array type.

  .. cpp:function:: unsigned long getHigh() const

      Returns the higher bound of the range.

      This can be the higher bound of the range type or the highest index for an array type.


.. cpp:class:: typeRef : public derivedType

  **A C++ reference**

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two references are compatible if the referred-to types are compatible.

  .. cpp:function:: bool is_rvalue() const noexcept

      Checks if this reference is a C++ r-value reference.


.. cpp:class:: typeScalar : public Type

  **Integral and floating-point types**

  .. cpp:function:: bool isSigned() const

      Checks if this is a signed type.

      The definition of signedness depends on the source language.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two scalars are compatible if their language-level types are compatible.

  .. cpp:function:: properties_t const& properties() const

      Returns the detailed properties of the scalar.

      This can be used to differentiate the various integral and floating-point types supported
      by the source language.


.. cpp:struct:: typeScalar::properties_t

  .. rubric:: Summary properties

  .. cpp:member:: bool is_integral
  .. cpp:member:: bool is_floating_point
  .. cpp:member:: bool is_string

      This is used for Pascal-style strings, not C-style null-terminated string.

  .. rubric:: Detailed properties

  .. cpp:member:: bool is_address
  .. cpp:member:: bool is_boolean
  .. cpp:member:: bool is_complex_float
  .. cpp:member:: bool is_float
  .. cpp:member:: bool is_imaginary_float
  .. cpp:member:: bool is_decimal_float
  .. cpp:member:: bool is_signed
  .. cpp:member:: bool is_signed_char
  .. cpp:member:: bool is_unsigned
  .. cpp:member:: bool is_unsigned_char
  .. cpp:member:: bool is_UTF


.. cpp:class:: typeStruct : public fieldListType

  **An algebraic product type**

  The C ``struct`` is a common example.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two structs are compatible if they have the same number of fields and the
      types of the fields are compatible.


.. cpp:class:: typeSubrange : public rangedType

  **A subsequence of a range**

  This could be a subrange of an array.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two subranges are compatible if they have the same number of elements.


.. cpp:class:: typeTypedef: public derivedType

  **An alias name for a language-level type**

  Represents a C ``typedef`` or C++ ``using`` alias.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two typedefs are compatible if their aliased types are compatible.


.. cpp:class:: typeUnion : public fieldListType

  **An algebraic sum type**

  The C ``union`` is a common example.

  .. cpp:function:: bool isCompatible(boost::shared_ptr<Type> x)

      The same as :cpp:func:`bool isCompatible(Type *otype)`.

  .. cpp:function:: bool isCompatible(Type *otype)

      Checks if ``otype`` is compatible with this type.

      Two unions are compatible if they have the same number of fields and the
      types of the fields are compatible.

