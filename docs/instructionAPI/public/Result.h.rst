Result.h
========

.. cpp:namespace:: Dyninst::instructionAPI

Result Class
------------

A ``Result`` object represents a value computed by an ``Expression``
AST.

The ``Result`` class is a tagged-union representation of the results
that Expressions can produce. It includes 8, 16, 32, 48, and 64 bit
integers (signed and unsigned), bit values, and single and double
precision floating point values. For each of these types, the value of a
Result may be undefined, or it may be a value within the range of the
type.

The ``type`` field is an enum that may contain any of the following
values:

-  ``u8:`` an unsigned 8-bit integer

-  ``s8:`` a signed 8-bit integer

-  ``u16:`` an unsigned 16-bit integer

-  ``s16:`` a signed 16-bit integer

-  ``u32:`` an unsigned 32-bit integer

-  ``s32:`` a signed 32-bit integer

-  ``u48:`` an unsigned 48-bit integer (IA32 pointers)

-  ``s48:`` a signed 48-bit integer (IA32 pointers)

-  ``u64:`` an unsigned 64-bit integer

-  ``s64:`` a signed 64-bit integer

-  ``sp_float:`` a single-precision float

-  ``dp_float:`` a double-precision float

-  ``bit_flag:`` a single bit (individual flags)

-  ``m512:`` a 512-bit memory value

-  ``dbl128:`` a 128-bit integer, which often contains packed floating
   point values - ``m14:`` a 14 byte memory value

.. code-block:: cpp

    Result (Result_Type t)

A ``Result`` may be constructed from a type without providing a value.
This constructor creates a ``Result`` of type ``t`` with undefined
contents.

.. code-block:: cpp

    Result (Result_Type t, T v)

A ``Result`` may be constructed from a type and any value convertible to
the type that the tag represents. This constructor creates a ``Result``
of type ``t`` and contents ``v`` for any ``v`` that is implicitly
convertible to type ``t``. Attempting to construct a ``Result`` with a
value that is incompatible with its type will result in a compile-time
error.

.. code-block:: cpp

    bool operator== (const Result & o) const

Two ``Result``\ s are equal if any of the following hold:

-  Both ``Result``\ s are of the same type and undefined

-  Both ``Result``\ s are of the same type, defined, and have the same
   value

Otherwise, they are unequal (due to having different types, an undefiend
``Result`` compared to a defined ``Result``, or different values).

.. code-block:: cpp

    std::string format () const

``Result``\ s are formatted as strings containing their contents,
represented as hexadecimal. The type of the ``Result`` is not included
in the output.

.. code-block:: cpp

    template <typename to_type> to_type convert() const

Converts the ``Result`` to the desired datatype. For example, to convert
a ``Result`` ``res`` to a signed char, use ``res.convert<char>()``; to
convert it to an unsigned long, use ``res.convert<unsigned long>()``.

.. code-block:: cpp

    int size () const

Returns the size of the contained type, in bytes.