#ifndef ROSE_INTEGEROPS_H
#define ROSE_INTEGEROPS_H

#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <limits>
#include <boost/static_assert.hpp>
#include <boost/optional.hpp>

namespace IntegerOpsPrivate {

    template <typename T>
    struct NumBits {
        BOOST_STATIC_ASSERT (std::numeric_limits<T>::radix == 2);
        BOOST_STATIC_ASSERT (std::numeric_limits<T>::is_integer);
        static const size_t value = std::numeric_limits<T>::digits;
    };

    template <typename T, size_t Count, bool TooBig> struct SHL1Helper;
    template <typename T, size_t Count>
    struct SHL1Helper<T, Count, true> {
        static const T value = 0;
    };
    template <typename T, size_t Count>
    struct SHL1Helper<T, Count, false> {
        static const T value = T(1) << Count;
    };

}

/** Bit-wise operations on integers.
 *
 *  Many of these are function templates for the best optimization, but we also provide non-template versions for those cases
 *  when the arguments are not known at compile time.  The non-template versions typically have "2" appended to their names to
 *  indicate that they should be the second choice--used only when the template version cannot be used. */
namespace IntegerOps {

/** Bitmask constant with bit @p n set.  Handles the case where @p n is greater than the width of type @p T. */
    template <typename T, size_t n>
    struct SHL1: public IntegerOpsPrivate::SHL1Helper<T, n, (n >= IntegerOpsPrivate::NumBits<T>::value)> {};

/** Bitmask with bit @p n set.  Handles the case where @p n is greater than the width of type @p T. */
    template <typename T>
    inline T shl1(size_t n) {
      return (n >= IntegerOpsPrivate::NumBits<T>::value) ? T(0) : (T(1) << n);
    }

/** Bit mask constant with bits 0 through @p n-1 set. */
    template <typename T, size_t n>
    struct GenMask {
        static const T value = SHL1<T, n>::value - T(1);
    };

/** Bitmask with bits 0 through @p N-1 set. */
    template <typename T>
    inline T genMask(size_t n) {
      return shl1<T>(n) - 1;
    }

/** Generate a bitmask. The return value has bits @p lobit (inclusive) through @p hibit (inclusive) set, and all other bits
 *  are clear. */
    template <typename T>
    inline T genMask(size_t lobit, size_t hibit)
    {
      assert(hibit<8*sizeof(T));
      assert(hibit>=lobit);
      return genMask<T>(1+hibit-lobit) << lobit;
    }

/** Returns true if the sign bit is set, false if clear.
 * @{ */
    template <size_t NBits, typename T>
    inline bool signBit(T value) {
      return (value & SHL1<T, NBits - 1>::value) != T(0);
    }

    template <typename T>
    inline bool signBit2(T value, size_t width=8*sizeof(T)) {
      assert(width>0 && width<=8*sizeof(T));
      T sign_mask = shl1<T>(width-1);
      return 0 != (value & sign_mask);
    }
/** @} */

/** Sign extend value.  If the bit @p FromBits-1 is set set for @p value, then the result will have bits @p FromBits through @p
*  ToBits-1 also set (other bits are unchanged).  If @p ToBits is less than or equal to @p FromBits then nothing happens.
* @{ */
    template <size_t FromBits, size_t ToBits, typename T>
    inline T signExtend(T value) {
      return value | (signBit<FromBits>(value) ? (GenMask<T, ToBits>::value ^ GenMask<T, FromBits>::value) : T(0));
    }

    template <typename T>
    inline T signExtend2(T value, size_t from_width, size_t to_width) {
      assert(from_width<=8*sizeof(T));
      assert(to_width<=8*sizeof(T));
      return value | (signBit2(value, from_width) ? (genMask<T>(to_width) ^ genMask<T>(from_width)) : T(0));
    }
/** @} */

/** Shifts bits of @p value left by @p count bits.
 * @{ */
    template <size_t NBits, typename T>
    inline T shiftLeft(T value, size_t count) {
      return (value * shl1<T>(count)) & GenMask<T, NBits>::value;
    }

    template <typename T>
    inline T shiftLeft2(T value, size_t count, size_t width=8*sizeof(T)) {
      assert(width>0 && width<=8*sizeof(T));
      return (value * shl1<T>(count)) & genMask<T>(width);
    }
/** @} */

/** Shifts bits of @p value right by @p count bits without sign extension.
 * @{ */
    template <size_t NBits, typename T>
    inline T shiftRightLogical(T value, size_t count) {
      return (count >= NBits) ? T(0) : (value >> count);
    }

    template <typename T>
    inline T shiftRightLogical2(T value, size_t count, size_t width=8*sizeof(T)) {
      assert(width>0 && width<=8*sizeof(T));
      return (count >= width) ? T(0) : (value >> count);
    }
/** @} */

/** Shifts bits of @p value right by @p count bits with sign extension.
 * @{ */
    template <size_t NBits, typename T>
    inline T shiftRightArithmetic(T value, size_t count) {
      if (count >= NBits) {
        return signBit<NBits>(value) ? GenMask<T, NBits>::value : T(0);
      } else {
        return (shiftRightLogical<NBits>(value, count) |
                (signBit<NBits>(value) ? (GenMask<T, NBits>::value ^ genMask<T>(NBits - count)) : T(0)));
      }
    }

    template <typename T>
    inline T shiftRightArithmetic2(T value, size_t count, size_t width=8*sizeof(T)) {
      if (count >= width) {
        return signBit2(value, width) ? genMask<T>(width) : T(0);
      } else {
        return (shiftRightLogical2(value, count, width) |
                (signBit2(value, width) ? (genMask<T>(width) ^ genMask<T>(width-count)) : T(0)));
      }
    }
/** @} */

/** Rotate the bits of the value left by count bits.
 * @{ */
    template <size_t NBits, typename T>
    inline T rotateLeft(T value, size_t count) {
      count %= NBits;
      return ((value << count) | (value >> (NBits - count))) & GenMask<T, NBits>::value;
    }

    template <typename T>
    inline T rotateLeft2(T value, size_t count, size_t width=8*sizeof(T)) {
      assert(width>0 && width<=8*sizeof(T));
      count %= width;
      return ((value << count) | (value >> (width-count))) & genMask<T>(width);
    }
/** @} */

/** Rotate bits of the value right by @p count bits.
 * @{ */
    template <size_t NBits, typename T>
    inline T rotateRight(T value, size_t count) {
      count %= NBits;
      return ((value >> count) | (value << (NBits - count))) & GenMask<T, NBits>::value;
    }

    template <typename T>
    inline T rotateRight2(T value, size_t count, size_t width=8*sizeof(T)) {
      assert(width>0 && width<=8*sizeof(T));
      return ((value >> count) | (value << (width - count))) & genMask<T>(width);
    }
/** @} */

/** Returns true if the value is a power of two.  Zero is considered a power of two. */
    template <typename T>
    inline bool isPowerOfTwo(T value)
    {
      assert(sizeof(T) <= sizeof(uint64_t));
      if (0 != (value & SHL1<T, 8*sizeof(T)-1>::value))
        value = ~value + 1;
      uint64_t uval = value;
      while (uval) {
        if (uval & 1)
          return 1==uval;
        uval >>= 1u;
      }
      return true; // treat zero as a power of two
    }

/** Returns the base-2 logorithm of @p value.  If @p value is not a power of two then the return value is rounded up to the
 *  next integer. The @p value is treated as an unsigned value. Returns zero if @p value is zero. */
    template <typename T>
    inline T log2max(T value)
    {
      assert(sizeof(T) <= sizeof(uint64_t));
      uint64_t uval = value;
      bool low_bits_set = false;
      T retval = 0;
      while (uval) {
        if (1==uval)
          return retval + (low_bits_set ? 1 : 0);
        if (uval & 1)
          low_bits_set = true;
        ++retval;
        uval >>= 1;
      }
      return retval;
    }


    template <typename T>
    inline T log2(T a) {
      T n = T(1);
      T i = 0;
      while (n != 0 && n < a) {
        n <<= 1;
        ++i;
      }
      return i;
    }

/** Create a shifted value. The return value is created by shifting @p value to the specified position in the result. Other
 *  bits of the return value are clear. The @p hibit is specified so that we can check at run-time that a valid value was
 *  specified (i.e., the value isn't too wide).
 * @{ */
    template<size_t lobit, size_t hibit, typename T>
    inline T shift_to(T value) {
      assert(hibit<8*sizeof(T));
      assert(hibit>=lobit);
      assert(0==(value & ~GenMask<T, 1+hibit-lobit>::value));
      return shiftLeft<8*sizeof(T)>(value & GenMask<T, 1+hibit-lobit>::value, lobit);
    }
    template<typename T>
    inline T shift_to2(size_t lobit, size_t hibit, T value)
    {
      assert(hibit<8*sizeof(T));
      assert(hibit>=lobit);
      assert(0==(value & ~genMask<T>(1+hibit-lobit)));
      return shiftLeft2<T>(value & genMask<T>(1+hibit-lobit), lobit);
    }
/** @} */

/** Extract bits from a value.  Bits @p lobit through @p hibit, inclusive, are right shifted into the result and higher-order
 *  bits of the result are cleared.
 * @{ */
    template<size_t lobit, size_t hibit, typename T>
    inline T extract(T bits) {
      assert(hibit<8*sizeof(T));
      assert(hibit>=lobit);
      return shiftRightLogical<8*sizeof(T)>(bits, lobit) & GenMask<T, 1+hibit-lobit>::value;
    }
    template<typename T>
    inline T extract2(size_t lobit, size_t hibit, T bits)
    {
      assert(hibit<8*sizeof(T));
      assert(hibit>=lobit);
      return shiftRightLogical<8*sizeof(T)>(bits, lobit) & genMask<T>(1+hibit-lobit);
    }
/** @} */

/** Determines if one bitmask is a subset of another.  Returns true if the bits set in the first argument form a subset of the
 *  bits set in the second argument. */
    template<typename T>
    inline bool bitmask_subset(T m1, T m2)
    {
      return 0 == (~m1 & m2); // m2 must not contain bits that are not in m1
    }

/** Counts how many bits are set (one). */
    template<typename T>
    inline size_t countSet(T val)
    {
      size_t retval = 0;
      for (size_t i=0; i<8*sizeof(T); ++i) {
        if (0 != (val & shl1<T>(i)))
          ++retval;
      }
      return retval;
    }

/** Counts how many bits are clear (zero). */
    template<typename T>
    inline size_t countClear(T val)
    {
      size_t retval = 0;
      for (size_t i=0; i<8*sizeof(T); ++i) {
        if (0 == (val & shl1<T>(i)))
          ++retval;
      }
      return retval;
    }

/** Optionally returns the zero-origin position of the most significant set bit.  Returns nothing if no bits are set. */
    template<typename T>
    inline boost::optional<size_t> msb_set(T val)
    {
      if (val!=0) {
        for (size_t bitno = 8*sizeof(T); bitno>0; --bitno) {
          if (0 != (val & shl1<T>(bitno-1)))
            return boost::optional<size_t>(bitno-1);
        }
      }
      return boost::optional<size_t>();
    }

} // namespace
#endif // ROSE_INTEGEROPS_H
