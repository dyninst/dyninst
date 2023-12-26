BPatch_type.h
=============

.. _`sec:BPatch_Type_System`:

Type System
-----------

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
type compatible. For each storage class, the following additional
requirements must be met for two types to be compatible:
   
Bpatch_dataScalar
~~~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataScalar
   
   Scalars are compatible if their names are the same (as defined by
   strcmp) and their sizes are the same.
   
BPatch_dataPointer
~~~~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataPointer
   
   Pointers are compatible if the types they point to are compatible.
   
BPatch_dataFunc
~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataFunc
   
   Functions are compatible if their return types are compatible, they have
   same number of parameters, and position by position each element of the
   parameter list is type compatible.
   
BPatch_dataArray
~~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataArray
   
   Arrays are compatible if they have the same number of elements
   (regardless of their lower and upper bounds) and the base element types
   are type compatible.
   
BPatch_dataEnumerated
~~~~~~~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataEnumerated
   
   Enumerated types are compatible if they have the same number of elements
   and the identifiers of the elements are the same.
   
BPatch_dataUnion
~~~~~~~~~~~~~~~~
.. cpp:class:: BPatch_dataUnion
   
   Structures and unions are compatible if they have the same number of
   constituent parts (fields) and item by item each field is type
   compatible with the corresponds field of the other type.
   
   In addition, if either of the types is the type BPatch_unknownType, then
   the two types are compatible. Variables in mutatee programs that have
   not been compiled with debugging symbols (or in the symbols are in a
   format that the Dyninst library does not recognize) will be of type
   BPatch_unknownType.
   
``BPatch_type``
---------------
.. cpp:namespace:: BPatch_type

.. cpp:class:: BPatch_type
   
   The class BPatch_type is used to describe the types of variables,
   parameters, return values, and functions. Instances of the class can
   represent language predefined types (e.g. int, float), mutatee defined
   types (e.g., structures compiled into the mutatee application), or
   mutator defined types (created using the create* methods of the BPatch
   class).
   
   .. cpp:function:: std::vector<BPatch_field *> *getComponents()
      
      Return a vector of the types of the fields in a BPatch_struct or
      BPatch_union. If this method is invoked on a type whose BPatch_dataClass
      is not BPatch_struct or BPatch_union, NULL is returned.
      
   .. cpp:function:: std::vector<BPatch_cblock *> *getCblocks()
      
      Return the common block classes for the type. The methods of the
      BPatch_cblock can be used to access information about the member of a
      common block. Since the same named (or anonymous) common block can be
      defined with different members in different functions, a given common
      block may have multiple definitions. The vector returned by this
      function contains one instance of BPatch_cblock for each unique
      definition of the common block. If this method is invoked on a type
      whose BPatch_dataClass is not BPatch_common, NULL will be returned.
      
   .. cpp:function:: BPatch_type *getConstituentType()
      
      Return the type of the base type. For a BPatch_array this is the type of
      each element, for a BPatch_pointer this is the type of the object the
      pointer points to. For BPatch_typedef types, this is the original type.
      For all other types, NULL is returned.
      
   .. cpp:enum:: BPatch_dataClass
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataScalar
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataEnumerated
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataTypeClass
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataStructure
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataUnion
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataArray
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataPointer
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataReference
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataFunction
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataTypeAttrib
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataUnknownType
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataMethod
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataCommon
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataPrimitive
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataTypeNumber
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataTypeDefine
   .. cpp:enumerator:: BPatch_dataClass::BPatch_dataNullType
      
   .. cpp:function:: BPatch_dataClass getDataClass()
      
      Return one of the above data classes for this type.
      
   .. cpp:function:: unsigned long getLow()
      
   .. cpp:function:: unsigned long getHigh()
      
      Return the upper and lower bound of an array. Calling these two methods
      on non-array types produces an undefined result.
      
   .. cpp:function:: const char *getName()
      
      Return the name of the type.
      
   .. cpp:function:: bool isCompatible(const BPatch_type &otype)
      
      Return true if otype is type compatible with this type. The rules for
      type compatibility are given in Section 4.28. If the two types are not
      type compatible, the error reporting callback function will be invoked
      one or more times with additional information about why the types are
      not compatible.

``BPatch_cblock``
-----------------
.. cpp:namespace:: BPatch_cblock

.. cpp:class:: BPatch_cblock
   
   This class is used to access information about a common block.
   
   .. cpp:function:: std::vector<BPatch_field *> *getComponents()
      
      Return a vector containing the individual variables of the common block.
      
   .. cpp:function:: std::vector<BPatch_function *> *getFunctions()
      
      Return a vector of the functions that can see this common block with the
      set of fields described in getComponents. However, other functions that
      define this common block with a different set of variables (or sizes of
      any variable) will not be returned.