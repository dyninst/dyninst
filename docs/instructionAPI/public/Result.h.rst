.. _`sec:Result.h`:

Result.h
########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Result

  **A value computed by an expression AST**

  .. Note:: This class satisfies the `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:member:: Result_Value val

      The value of the result.

  .. cpp:member:: Result_Type type

      The type of the result.

  .. cpp:member:: bool defined

      ``true`` if the result is defined.

  .. cpp:function:: Result()

      Creates a undefined result with type :cpp:enumerator:`Result_Type::u32`.

  .. cpp:function:: Result(Result_Type t)

      Creates a result with type ``t``.

  .. cpp:function:: Result(Result_Type t, T v)

      Creates a result of type ``t`` and contents ``v``.

      ``T`` must be implicitly convertible to ``t``.

  .. cpp:function:: Result(Result_Type t, float v)

      Creates a result of type ``t`` and contents ``v``.

      ``t`` must be one of :cpp:enumerator:`Result_Type::sp_float`
      or :cpp:enumerator:`Result_Type::dp_float`.

  .. cpp:function:: Result(Result_Type t, double v)

      Creates a result of type ``t`` and contents ``v``.

      ``t`` must be one of :cpp:enumerator:`Result_Type::sp_float`
      or :cpp:enumerator:`Result_Type::dp_float`.

  .. cpp:function:: bool operator==(const Result & o) const

      Checks if this result is equal to ``o``.

      Two results are equal if:
        -  Both are of the same type and undefined
        -  Both are of the same type, defined, and have the same value

  .. cpp:function:: std::string format() const

      Returns a string representation of the result as hexadecimal.

  .. cpp:function:: template <typename to_type> to_type convert() const

      Converts this result to the desired type.

      For example, to convert to a signed char, use ``convert<char>()``.

  .. cpp:function:: int size() const

      Returns the size of the contained type in **bytes**.

  .. cpp:function:: Result operator+(const Result& arg1, const Result& arg2)

      Adds the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.

  .. cpp:function:: Result operator*(const Result& arg1, const Result& arg2)

      Multiplies the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.

  .. cpp:function:: Result operator<<(const Result& arg1, const Result& arg2)

      Left shifts the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.

  .. cpp:function:: Result operator>>(const Result& arg1, const Result& arg2)

      Right arithmetic shifts the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.

  .. cpp:function:: Result operator&(const Result& arg1, const Result& arg2)

      Bitwise ands the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.

  .. cpp:function:: Result operator|(const Result& arg1, const Result& arg2)

      Bitwise ors the underlying values of two results.

      .. Attention:: Only defined when the types of both results are int-like.


.. cpp:struct:: template <Result_Type t> Result::Result_type2type

  Converts a result to type ``t``.

  An overload is provided for all supported :cpp:enum:`Result_Type`\ s.


.. cpp:union:: Result::Result_Value

  .. cpp:member:: unsigned char bitval : 1
  .. cpp:member:: unsigned char u8val
  .. cpp:member:: signed char s8val

    char can be signed or unsigned, must be signed for s8val

  .. cpp:member:: uint16_t u16val
  .. cpp:member:: int16_t s16val
  .. cpp:member:: uint32_t u24val:24
  .. cpp:member:: uint32_t u32val
  .. cpp:member:: int32_t s32val
  .. cpp:member:: uint64_t u48val : 48
  .. cpp:member:: int64_t s48val : 48
  .. cpp:member:: uint64_t u64val
  .. cpp:member:: int64_t s64val
  .. cpp:member:: float floatval
  .. cpp:member:: double dblval
  .. cpp:member:: void * dbl128val
  .. cpp:member:: void * m14val
  .. cpp:member:: void * m32val
  .. cpp:member:: void * m64val
  .. cpp:member:: void * m80val
  .. cpp:member:: void * m96val
  .. cpp:member:: void * m128val
  .. cpp:member:: void * m160val
  .. cpp:member:: void * m192val
  .. cpp:member:: void * m224val
  .. cpp:member:: void * m256val
  .. cpp:member:: void * m288val
  .. cpp:member:: void * m320val
  .. cpp:member:: void * m352val
  .. cpp:member:: void * m384val
  .. cpp:member:: void * m416val
  .. cpp:member:: void * m448val
  .. cpp:member:: void * m480val
  .. cpp:member:: void * m512val

.. cpp:enum:: Result::Result_Type

  .. warning::
    The order of these enumerations is important. See 'operator==' and arithmetic operators.

  .. cpp:enumerator:: bit_flag

       a single bit (individual flags)

  .. cpp:enumerator:: u8

       an unsigned 8-bit integer

  .. cpp:enumerator:: s8

       a signed 8-bit integer

  .. cpp:enumerator:: u16

       an unsigned 16-bit integer

  .. cpp:enumerator:: s16

       a signed 16-bit integer

  .. cpp:enumerator:: u24

       an unsigned 24-bit integer

  .. cpp:enumerator:: u32

       an unsigned 32-bit integer

  .. cpp:enumerator:: s32

       a signed 32-bit integer

  .. cpp:enumerator:: u48

       an unsigned 48-bit integer

  .. cpp:enumerator:: s48

       a signed 48-bit integer

  .. cpp:enumerator:: u64

       an unsigned 64-bit integer

  .. cpp:enumerator:: s64

       a signed 64-bit integer

  .. cpp:enumerator:: sp_float

       a single-precision float

  .. cpp:enumerator:: dp_float

       a double-precision float

  .. cpp:enumerator:: dbl128

       a 128-bit integer, which often contains packed floating point values

  .. cpp:enumerator:: m14

       a 14 byte memory value

       For historical reason m14 means 14 bytes. All other ``mX`` means ``X`` bits.

  .. cpp:enumerator:: m32

       a 32-bit memory value

  .. cpp:enumerator:: m64

       a 64-bit memory value

  .. cpp:enumerator:: m80

       an 80-bit memory value

  .. cpp:enumerator:: m96

       a 96-bit memory value

  .. cpp:enumerator:: m128

       a 128-bit memory value

  .. cpp:enumerator:: m160

       a 160-bit memory value

  .. cpp:enumerator:: m192

       a 192-bit memory value

  .. cpp:enumerator:: m224

       a 224-bit memory value

  .. cpp:enumerator:: m256

       a 256-bit memory value

  .. cpp:enumerator:: m288

       a 288-bit memory value

  .. cpp:enumerator:: m320

       a 320-bit memory value

  .. cpp:enumerator:: m352

       a 352-bit memory value

  .. cpp:enumerator:: m384

       a 384-bit memory value

  .. cpp:enumerator:: m416

       a 416-bit memory value

  .. cpp:enumerator:: m448

       a 448-bit memory value

  .. cpp:enumerator:: m480

       a 480-bit memory value

  .. cpp:enumerator:: m512

       a 512-bit memory value


.. _`sec:result-notes`:

Notes
=====

The ``Result`` class is a tagged-union representation of the results
that Expressions can produce. It includes 8, 16, 32, 48, and 64 bit
integers(signed and unsigned), bit values, and single and double
precision floating point values. For each of these types, the value of a
Result may be undefined, or it may be a value within the range of the
type.

The %Instruction API's model of %Results is a simple one, and may seem overly aggressive about
making an %Expression's %Result undefined.  It follows the same basic rule as the rest of the API:
a decoded %Instruction object represents only the information that may be obtained from the machine
instruction that was decoded.  As discussed in the Expression section, the \c setValue
and \c eval interface allows you to determine the possible %Results of an %Expression when evaluated over various
machine states.  From this, you may construct abstractions to represent the set of possible results.
Alternately, you may use instrumentation to determine the exact machine state at the time an
instruction executes, which will allow you to evaluate the %Result of an %Expression in its actual context.
