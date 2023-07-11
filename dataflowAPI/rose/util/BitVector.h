// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_BitVector_H
#define Sawyer_BitVector_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/cstdint.hpp>
#include "Assert.h"
#include "BitVectorSupport.h"
#include "Optional.h"
#include "Sawyer.h"
#include <vector>

namespace Sawyer {
namespace Container {

#ifdef BOOST_WINDOWS
/** Log base 2.
 *
 *  Returns the logorithm base 2 of n by the change-of-base formula. */
inline double log2(double n) {
    return log(n) / log(2.0);
}
#endif

/** Bit vectors.
 *
 *  This class implements bit vectors with run-time sizes and a rich set of operations that can be restricted to a contiguous
 *  subset of bits.  The primary goal of this class is not to provide the utmost performance, but rather a rich, easy-to-use
 *  interface. For example,
 *
 * @code
 *  BitVector bv(128);                                  // a 128-bit vector with all bits cleared
 *  bv.set(BitRange(6,27));                             // set (make true) bits 6 through 27, inclusive
 *  bv.fromHex(BitRange::baseSize(32,64), "deadbeef");  // initialize 64 bits beginning at bit 32
 *  bool carry = bv.add(BitRange(5,8), BitRange(5,8));  // double the integer represented by bits 5 through 8
 *  std::cout <<bv.toBinary(BitRange(5,8)) <<"\n";      // print bits 5-8 as a binary string
 * @endcode
 *
 *  In general, each method has a number of overloaded varieties: if a BitRange is not specified it generally means the entire
 *  bit vector; if a second bit vector is not specified for binary operations it generally means use this vector for both
 *  operands.  Non-const operations modify @p this vector in place, and most of them also return a reference so that they can
 *  be easily chained:
 *
 * @code
 *  bv.clear().set(BitRange(24,31));                    // clear all bits, then set bits 24-31
 * @endcode
 *
 *  When performing an operation that has two operands, the operands are generally permitted to both refer to the same vector,
 *  and the range arguments are permitted to overlap in that vector.  When this occurs, the semantics are as if a temporary
 *  copy was made, then the operation was performed on the temporary copy, then the result was written back to the
 *  destination.
 *
 *  BitVector objects manage their own data, but if one needs to operate on an array that is already allocated then the
 *  function templates in the @ref BitVectorSupport name space can be used. */
class BitVector {
public:
    typedef unsigned Word;                              /**< Base storage type. */
    typedef BitVectorSupport::BitRange BitRange;        /**< Describes an inclusive interval of bit indices. */

private:
    std::vector<Word> words_;
    size_t size_;

public:
    /** Default construct an empty vector. */
    BitVector(): size_(0) {}

    /** Copy constructor. */
    BitVector(const BitVector &other): words_(other.words_), size_(other.size_) {}

    /** Create a vector of specified size.
     *
     *  All bits in this vector will be set to the @p newBits value. */
    explicit BitVector(size_t nbits, bool newBits = false): size_(0) {
        resize(nbits, newBits);
    }

    /** Create a bit vector by reading a string.
     *
     *  Reads a bit vector from the string and returns the result. The input string has an optional suffix ("h" for
     *  hexadecimal) or optioanl prefix ("0x" for hexadecimal, "0b" or binary, or "0" for octal). It does not have both; if a
     *  suffix is present then the parser does not search for a prefix.  Lack of both prefix and suffix implies decimal
     *  format. Any recognized suffix or prefix is stripped from the value, and the number of valid digits is counted and used
     *  to calculate the width of the resulting bit vector. For instance, if three octal digits are found (not counting the "0"
     *  prefix) then the resulting bit vector will have nine bits -- three per digit.  The string is then passed to @ref
     *  fromBinary, @ref fromOctal, @ref fromDecimal, or @ref fromHex for parsing.
     *
     *  @todo Decimal parsing is not fully supported. */
    static BitVector parse(std::string str) {
        // Radix information
        size_t bitsPerDigit = 0;
        const char *digits = NULL;
        if (boost::starts_with(str, "0x")) {
            bitsPerDigit = 4;
            digits = "0123456789abcdefABCDEF";
            str = str.substr(2);
        } else if (boost::starts_with(str, "0b")) {
            bitsPerDigit = 1;
            digits = "01";
            str = str.substr(2);
        } else if (boost::ends_with(str, "h")) {
            bitsPerDigit = 4;
            digits = "0123456789abcdefABCDEF";
            str = str.substr(0, str.size()-1);
        } else if (boost::starts_with(str, "0")) {
            bitsPerDigit = 2;
            digits = "01234567";
            str = str.substr(1);
        } else {
            bitsPerDigit = 0;                           // special case
            digits = "0123456789";
        }

        // Count digits
        size_t nDigits = 0;
        for (const char *t=str.c_str(); *t; ++t) {
            if (strchr(digits, *t))
                ++nDigits;
        }
        if (0==nDigits)
            throw std::runtime_error("BitVector::parse: no valid digits");

        // Number of bits
        size_t nBits = 0;
        if (bitsPerDigit) {
            nBits = bitsPerDigit * nDigits;
        } else {
            nBits = ceil(log2(pow(10.0, (double)nDigits)));
        }

        // Parse the string
        BitVector result(nBits);
        switch (bitsPerDigit) {
            case 0:
                result.fromDecimal(str);
                break;
            case 2:
                result.fromBinary(str);
                break;
            case 3:
                result.fromOctal(str);
                break;
            case 4:
                result.fromHex(str);
                break;
            default:
                assert(!"invalid radix");
                break;
        }
        return result;
    }
    
    /** Assignment.
     *
     *  Makes this bit vector an exact copy of the @p other vector.
     *
     *  @sa The @ref copy method is similar but does not change the size of the destination vector. */
    BitVector& operator=(const BitVector &other) {
        words_ = other.words_;
        size_ = other.size_;
        return *this;
    }

    /** Determines if the vector is empty.
     *
     *  Returns true if this vector contains no data. */
    bool isEmpty() const { return 0 == size_; }

    /** Size of vector in bits.
     *
     *  Returns the size of this vector in bits. */
    size_t size() const { return size_; }

    /** Change vector size.
     *
     *  Changes the size of a vector, measured in bits, by either adding or removing bits from the most-significant side of
     *  this vector.  If new bits are added they are each given the value @p newBits.  Increasing the size of a vector may
     *  cause it to reallocate and copy its internal data structures. */
    BitVector& resize(size_t newSize, bool newBits=false) {
        if (0==newSize) {
            words_.clear();
            size_ = 0;
        } else if (newSize > size_) {
            size_t nwords = BitVectorSupport::numberOfWords<Word>(newSize);
            words_.resize(nwords, Word(0));
            BitVectorSupport::setValue(data(), BitRange::hull(size_, newSize-1), newBits);
            size_ = newSize;
        } else {
            size_t nwords = BitVectorSupport::numberOfWords<Word>(newSize);
            words_.resize(nwords);
            size_ = newSize;
        }
        return *this;
    }

    /** Maximum size before reallocation.
     *
     *  Returns the maximum number of bits to which this vector could be resized via @ref resize before it becomes necessary to
     *  reallocate its internal data structures. */
    size_t capacity() const {
        return BitVectorSupport::bitsPerWord<Word>::value * words_.capacity();
    }

    /** Interval representing the entire vector.
     *
     *  Returns the smallest index interval that includes all bits. */
    BitRange hull() const {
        return 0==size_ ? BitRange() : BitRange::hull(0, size_-1);
    }

    /** Create a bit range from a starting offset and size.
     *
     *  This is just a convenience wrapper around BitRange::baseSize() so that name qualification can be avoided when "using
     *  namespace" directives are not employed. */
    static BitRange baseSize(size_t base, size_t size) {
        return BitRange::baseSize(base, size);
    }

    /** Create a bit range from min and max positions.
     *
     *  This is just a convenience wrapper around BitRange::hull(size_t,size_t) so that name qualification can be avoided when
     *  "using namespace" directives are not employed. */
    static BitRange hull(size_t minOffset, size_t maxOffset) {
        return BitRange::hull(minOffset, maxOffset);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Value access
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Retrieve one bit.
     *
     *  Returns the value of the bit at the specified index in constant time.  The index must be a valid index for this
     *  vector. */
    bool get(size_t idx) const {
        checkRange(idx);
        return BitVectorSupport::get(data(), idx);
    }

    /** Assign zero to some bits.
     *
     *  Clears bits by assigning false to each bit in the specified range.  The convention is that "clear" means to assign
     *  false to a bit and should not be confused with the STL usage of the word, namely to erase all values from a
     *  container. To erase all bits from a vector, use <code>resize(0)</code>. */
    BitVector& clear(const BitRange &range) {
        checkRange(range);
        BitVectorSupport::clear(data(), range);
        return *this;
    }

    /** Assign zero to all bits.
     *
     *  Clears bits by assigning false to all bits in this vector.  The convention is that "clear" means to assign false
     *  to to a bit and should not be confused with the STL usage of the word, namely to erase all values from a
     *  container.  To erase all bits from a vector, use <code>resize(0)</code>. */
    BitVector& clear() {
        BitVectorSupport::clear(data(), hull());
        return *this;
    }

    /** Assign true to some bits.
     *
     *  Sets bits by assigning true (or @p newBits) to each bit in the specified range.  The convention is that "set" means to
     *  assign true to a bit; to assign a specific value use @ref setValue. */
    BitVector& set(const BitRange &range) {
        checkRange(range);
        BitVectorSupport::set(data(), range);
        return *this;
    }

    /** Assign true to all bits.
     *
     *  Sets bits by assigning true (or @p newBits) to all bits in this vector.  The convention is that "set" means to
     *  assign true to a bit; to assign a specific value use @ref setValue. */
    BitVector& set() {
        BitVectorSupport::set(data(), hull());
        return *this;
    }

    /** Assign true/false to some bits.
     *
     *  Sets the bits in the specified range to the specified value. */
    BitVector& setValue(const BitRange &range, bool value) {
        checkRange(range);
        BitVectorSupport::setValue(data(), range, value);
        return *this;
    }

    /** Assign true/false to all bits.
     *
     *  Sets all bits to the specified value. */
    BitVector& setValue(bool value) {
        BitVectorSupport::setValue(data(), hull(), value);
        return *this;
    }
    
    /** Copy some bits.
     *
     *  Copies bits from @p other specified by @p from into this vector specified by @p to.  The ranges must be the same size
     *  and must be valid for their respective vectors.  The @p other vector is permitted to be the same vector as
     *  <code>this</code>, in which case @p from is also permitted to overlap with @p to.
     *
     *  @sa Copy constructor and assignment operator. */
    BitVector& copy(const BitRange &to, const BitVector &other, const BitRange &from) {
        checkRange(to);
        other.checkRange(from);
        BitVectorSupport::copy(other.data(), from, data(), to);
        return *this;
    }

    /** Copy some bits.
     *
     *  Copies bits from the range @p from to the range @p to.  Both ranges must be the same size, and they may overlap.
     *
     *  @sa Copy constructor and assignment operator. */
    BitVector& copy(const BitRange &to, const BitRange &from) {
        checkRange(to);
        checkRange(from);
        BitVectorSupport::copy(data(), from, data(), to);
        return *this;
    }

    /** Swap some bits.
     *
     *  Swaps bits between @p range1 of this vector and @p range2 of the @p other vector.  Both ranges must be the same size
     *  and must be valid for their respective vectors.  The @p other vector is permitted to be the same vector as
     *  <code>this</code>, but @p range1 and @p range2 are not permitted to overlap. */
    BitVector& swap(const BitRange &range1, BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        BitVectorSupport::swap(data(), range1, other.data(), range2);
        return *this;
    }

    /** Swap some bits.
     *
     *  Swaps bits between @p range1 and @p range2 of this vector.  Both ranges must be the same size, and must be valid for
     *  this vector, and must not overlap. */
    BitVector& swap(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        BitVectorSupport::swap(data(), range1, data(), range2);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Counting/searching
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Find the least significant set bit.
     *
     *  Returns the index for the least significant bit that has the value true in the specified range.  The range must be
     *  valid for this vector. If the range has no such bits, including the case when the range is empty, then nothing is
     *  returned. */
    Optional<size_t> leastSignificantSetBit(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::leastSignificantSetBit(data(), range);
    }

    /** Find the least significant set bit.
     *
     *  Returns the index for the least significant bit that has the value true. If no bit is true, including the case when the
     *  vector is empty, then nothing is returned. */
    Optional<size_t> leastSignificantSetBit() const {
        return BitVectorSupport::leastSignificantSetBit(data(), hull());
    }

    /** Find the least significant clear bit.
     *
     *  Returns the index for the least significant bit that has the value false in the specified range.  The range must be
     *  valid for this vector. If the range has no such bits, including the case when the range is empty, then nothing is
     *  returned. */
    Optional<size_t> leastSignificantClearBit(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::leastSignificantClearBit(data(), range);
    }

    /** Find the least significant clear bit.
     *
     *  Returns the index for the least significant bit that has the value false. If no bit is false, including the case when
     *  the vector is empty, then nothing is returned. */
    Optional<size_t> leastSignificantClearBit() const {
        return BitVectorSupport::leastSignificantClearBit(data(), hull());
    }
    
    /** Find the most significant set bit.
     *
     *  Returns the index for the most significant bit that has the value true in the specified range.  The range must be
     *  valid for this vector. If the range has no such bits, including the case when the range is empty, then nothing is
     *  returned. */
    Optional<size_t> mostSignificantSetBit(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::mostSignificantSetBit(data(), range);
    }

    /** Find the most significant set bit.
     *
     *  Returns the index for the most significant bit that has the value true. If no bit is true, including the case when the
     *  vector is empty, then nothing is returned. */
    Optional<size_t> mostSignificantSetBit() const {
        return BitVectorSupport::mostSignificantSetBit(data(), hull());
    }

    /** Find the most significant clear bit.
     *
     *  Returns the index for the most significant bit that has the value false in the specified range.  The range must be
     *  valid for this vector. If the range has no such bits, including the case when the range is empty, then nothing is
     *  returned. */
    Optional<size_t> mostSignificantClearBit(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::mostSignificantClearBit(data(), range);
    }

    /** Find the most significant clear bit.
     *
     *  Returns the index for the most significant bit that has the value false. If no bit is false, including the case when
     *  the vector is empty, then nothing is returned. */
    Optional<size_t> mostSignificantClearBit() const {
        return BitVectorSupport::mostSignificantClearBit(data(), hull());
    }

    /** True if all bits are set.
     *
     *  Returns true if all bits are set within the specified range, or if the range is empty. The range must be valid for this
     *  vector. */
    bool isAllSet(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::isAllSet(data(), range);
    }
    
    /** True if all bits are set.
     *
     *  Returns true if all bits are set, or if the vector is empty. */
    bool isAllSet() const {
        return BitVectorSupport::isAllSet(data(), hull());
    }
    
    /** True if all bits are clear.
     *
     *  Returns true if all bits are clear within the specified range, or if the range is empty. The range must be valid for
     *  this vector.
     *
     *  @sa isEqualToZero */
    bool isAllClear(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::isAllClear(data(), range);
    }
    
    /** True if all bits are clear.
     *
     *  Returns true if all bits are clear, or if the vector is empty.
     *
     *  @sa isEqualToZero */
    bool isAllClear() const {
        return BitVectorSupport::isAllClear(data(), hull());
    }

    /** Number of set bits.
     *
     *  Returns the number of bits that are set in the specified range. The range must be valid for this vector. */
    size_t nSet(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::nSet(data(), range);
    }

    /** Number of set bits.
     *
     *  Returns the number of bits that are set. */
    size_t nSet() const {
        return BitVectorSupport::nSet(data(), hull());
    }

    /** Number of clear bits.
     *
     *  Returns the number of bits that are clear in the specified range. The range must be valid for this vector. */
    size_t nClear(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::nClear(data(), range);
    }

    /** Number of clear bits.
     *
     *  Returns the number of bits that are clear. */
    size_t nClear() const {
        return BitVectorSupport::nClear(data(), hull());
    }

    /** Find most significant difference.
     *
     *  Finds the most significant bit that differs between @p range1 of this vector and @p range2 of the @p other vector and
     *  returns its offset from the beginning of the ranges.  Both ranges must be the same size and must be valid for their
     *  respective vectors. If no bits differ, including the case when both ranges are empty, then nothing is returned.  The @p
     *  other vector is permitted to be the same as <code>this</code> vector, in which case the ranges are also permitted to
     *  overlap.
     *
     *  Note that the return value is not a vector index, but rather an offset with respect to the starting index in each of
     *  the ranges. */
    Optional<size_t> mostSignificantDifference(const BitRange &range1, const BitVector &other,
                                               const BitRange &range2) const {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::mostSignificantDifference(data(), range1, other.data(), range2);
    }
    
    /** Find most significant difference.
     *
     *  Finds the most significant bit that differs between the two specified ranges of this vector and returns its offset from
     *  the beginning of the ranges.  Both ranges must be the same size and must be valid for this vector. If no bits differ,
     *  including the case when both ranges are empty, then nothing is returned.  The ranges are permitted to overlap.
     *
     *  Note that the return value is not a vector index, but rather an offset with respect to the starting index in each of
     *  the ranges. */
    Optional<size_t> mostSignificantDifference(const BitRange &range1, const BitRange &range2) const {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::mostSignificantDifference(data(), range1, data(), range2);
    }
    
    /** Find most significant difference.
     *
     *  Finds the most significant bit that differs between this vector and the @p other vector and return its index.  Both
     *  vectors must be the same size.  If no bits differ, including the case when this vector is empty, then nothing is
     *  returned. */
    Optional<size_t> mostSignificantDifference(const BitVector &other) const {
        return BitVectorSupport::mostSignificantDifference(data(), hull(), other.data(), other.hull());
    }
        
    /** Find least significant difference.
     *
     *  Finds the least significant bit that differs between @p range1 of this vector and @p range2 of the @p other vector and
     *  returns its offset from the beginning of the ranges.  Both ranges must be the same size and must be valid for their
     *  respective vectors. If no bits differ, including the case when both ranges are empty, then nothing is returned.  The @p
     *  other vector is permitted to be the same as <code>this</code> vector, in which case the ranges are also permitted to
     *  overlap.
     *
     *  Note that the return value is not a vector index, but rather an offset with respect to the starting index in each of
     *  the ranges. */
    Optional<size_t> leastSignificantDifference(const BitRange &range1, const BitVector &other,
                                                const BitRange &range2) const {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::leastSignificantDifference(data(), range1, other.data(), range2);
    }
    
    /** Find least significant difference.
     *
     *  Finds the least significant bit that differs between the two specified ranges of this vector and returns its offset from
     *  the beginning of the ranges.  Both ranges must be the same size and must be valid for this vector. If no bits differ,
     *  including the case when both ranges are empty, then nothing is returned.  The ranges are permitted to overlap.
     *
     *  Note that the return value is not a vector index, but rather an offset with respect to the starting index in each of
     *  the ranges. */
    Optional<size_t> leastSignificantDifference(const BitRange &range1, const BitRange &range2) const {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::leastSignificantDifference(data(), range1, data(), range2);
    }
    
    /** Find least significant difference.
     *
     *  Finds the least significant bit that differs between this vector and the @p other vector and return its index.  Both
     *  vectors must be the same size.  If no bits differ, including the case when this vector is empty, then nothing is
     *  returned. */
    Optional<size_t> leastSignificantDifference(const BitVector &other) const {
        return BitVectorSupport::leastSignificantDifference(data(), hull(), other.data(), other.hull());
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Shift/rotate
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Shift bits left.
     *
     *  Shifts the bits in the specified range of this vector left (to more significant positions) by @p nShift bits.  Bits
     *  shifted off the left of the range are discarded; new bits shifted into the right of the range are introduced with the
     *  value @p newBits. The range must be valid for this vector.  If @p nShift is zero or the range is empty then no
     *  operation is performed. Specifying an @p nShift value equal to or greater than the size of the range has the same
     *  effect as filling the range with @p newBits (see also, @ref set and @ref clear, which are probably more efficient). */
    BitVector& shiftLeft(const BitRange &range, size_t nShift, bool newBits = 0) {
        checkRange(range);
        BitVectorSupport::shiftLeft(data(), range, nShift, newBits);
        return *this;
    }

    /** Shift bits left.
     *
     *  Shifts all bits of this vector left (to more significant positions) by @p nShift bits.  Bits shifted off the left of
     *  this vector are discarded; new bits shifted into the right of this vector are introduced with the value @p newBits. If
     *  @p nShift is zero or the vector is empty then no operation is performed. Specifying an @p nShift value equal to or
     *  greater than the size of this vector has the same effect as filling the vector with @p newBits (see also, @ref set and
     *  @ref clear, which are probably more efficient). */
    BitVector& shiftLeft(size_t nShift, bool newBits = 0) {
        BitVectorSupport::shiftLeft(data(), hull(), nShift, newBits);
        return *this;
    }

    /** Shift bits right.
     *
     *  Shifts the bits in the specified range of this vector right (to less significant positions) by @p nShift bits.  Bits
     *  shifted off the right of the range are discarded; new bits shifted into the left of the range are introduced with the
     *  value @p newBits. The range must be valid for this vector.  If @p nShift is zero or the range is empty then no
     *  operation is performed. Specifying an @p nShift value equal to or greater than the size of the range has the same
     *  effect as filling the range with @p newBits (see also, @ref set and @ref clear, which are probably more efficient). */
    BitVector& shiftRight(const BitRange &range, size_t nShift, bool newBits = 0) {
        checkRange(range);
        BitVectorSupport::shiftRight(data(), range, nShift, newBits);
        return *this;
    }

    /** Shift bits right.
     *
     *  Shifts all bits of this vector right (to less significant positions) by @p nShift bits.  Bits shifted off the right of
     *  this vector are discarded; new bits shifted into the left of this vector are introduced with the value @p newBits. If
     *  @p nShift is zero or the vector is empty then no operation is performed. Specifying an @p nShift value equal to or
     *  greater than the size of this vector has the same effect as filling the vector with @p newBits (see also, @ref set and
     *  @ref clear, which are probably more efficient). */
    BitVector& shiftRight(size_t nShift, bool newBits = 0) {
        BitVectorSupport::shiftRight(data(), hull(), nShift, newBits);
        return *this;
    }

    /** Shift bits right.
     *
     *  Shifts the bits in the specified range of this vector right (to less significant positions) by @p nShift bits.  Bits
     *  shifted off the right of the range are discarded; new bits shifted into the left of the range are introduced with the
     *  same value as the original most-significant bit of the range. The range must be valid for this vector.  If @p nShift is
     *  zero or the range is empty or a singleton then no operation is performed. Specifying an @p nShift value equal to or
     *  greater than the size of the range has the same effect as filling the range with its original most-significant bit (see
     *  also, @ref set and @ref clear, which are probably more efficient). */
    BitVector& shiftRightArithmetic(const BitRange &range, size_t nShift) {
        checkRange(range);
        BitVectorSupport::shiftRightArithmetic(data(), range, nShift);
        return *this;
    }

    /** Shift bits right.
     *
     *  Shifts the bits in this vector right (to less significant positions) by @p nShift bits.  Bits shifted off the right of
     *  this vector are discarded; new bits shifted into the left of this vector are introduced with the same value as the
     *  original most-significant bit of this vector. If @p nShift is zero or the vector is empty or only a single bit then no
     *  operation is performed. Specifying an @p nShift value equal to or greater than the size of this vector has the same
     *  effect as filling the vector with its original most-significant bit (see also, @ref set and @ref clear, which are
     *  probably more efficient). */
    BitVector& shiftRightArithmetic(size_t nShift) {
        BitVectorSupport::shiftRightArithmetic(data(), hull(), nShift);
        return *this;
    }

    /** Rotate bits right.
     *
     *  Rotates the bits in the specified range to the right (to less significant positions) by shifting right and
     *  reintroducing the bits shifted off the right end into the left end. The range must be valid for this vector.  If @p
     *  nShift is zero modulo the range size, or the range is empty, then no operation is performed. */
    BitVector& rotateRight(const BitRange &range, size_t nShift) {
        checkRange(range);
        BitVectorSupport::rotateRight(data(), range, nShift);
        return *this;
    }

    /** Rotate bits right.
     *
     *  Rotates all bits in this vector to the right (to less significant positions) by shifting right and reintroducing the
     *  bits shifted off the right end into the left end. If @p nShift is zero modulo this vector size, or this vector is
     *  empty, then no operation is performed. */
    BitVector& rotateRight(size_t nShift) {
        BitVectorSupport::rotateRight(data(), hull(), nShift);
        return *this;
    }
    
    /** Rotate bits left.
     *
     *  Rotates the bits in the specified range to the left (to more significant positions) by shifting left and
     *  reintroducing the bits shifted off the left end into the right end. The range must be valid for this vector.  If @p
     *  nShift is zero modulo the range size, or the range is empty, then no operation is performed. */
    BitVector& rotateLeft(const BitRange &range, size_t nShift) {
        checkRange(range);
        BitVectorSupport::rotateLeft(data(), range, nShift);
        return *this;
    }

    /** Rotate bits left.
     *
     *  Rotates all bits in this vector to the left (to more significant positions) by shifting left and reintroducing the
     *  bits shifted off the left end into the right end. If @p nShift is zero modulo this vector size, or this vector is
     *  empty, then no operation is performed. */
    BitVector& rotateLeft(size_t nShift) {
        BitVectorSupport::rotateLeft(data(), hull(), nShift);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Arithmetic
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Negates bits as integer.
     *
     *  Treats @p range1 of this vector as a two's complement integer and negates it, storing the result back into the same
     *  range of bits.  The range must be valid for this vector. */
    BitVector& negate(const BitRange &range1) {
        checkRange(range1);
        BitVectorSupport::negate(data(), range1);
        return *this;
    }
    
    /** Negates bits as integer.
     *
     *  Treats all bits of this vector as a two's complement integer and negates it, storing the result back into this
     *  vector. */
    BitVector& negate() {
        BitVectorSupport::negate(data(), hull());
        return *this;
    }

    /** Increment bits as integer.
     *
     *  Treats @p range1 of this vector as an integer and adds one to it, storing the result back into this vector. The range
     *  must be valid for this vector. Returns true if all bits were originally set and the result is all clear (i.e., returns
     *  the carry-out value) . */
    bool increment(const BitRange &range1) {
        checkRange(range1);
        return BitVectorSupport::increment(data(), range1);
    }

    /** Increment bits as integer.
     *
     *  Treats the entire vector as an integer and adds one to it, storing the result back into the vector.  Returns true if
     *  all bits were originally set and the result is all clear (i.e., returns the carry-out value). */
    bool increment() {
        return BitVectorSupport::increment(data(), hull());
    }

    /** Decrement bits as integer.
     *
     *  Treats @p range1 of this vector as an integer and subtracts one from it, storing the result back into this vector. The
     *  range must be valid for this vector. Returns true if all bits were originally clear and the result is all set (i.e.,
     *  returns the overflow bit) . */
    bool decrement(const BitRange &range1) {
        checkRange(range1);
        return BitVectorSupport::decrement(data(), range1);
    }

    /** Decrement bits as integer.
     *
     *  Treats the entire vector as an integer and subtracts one from it, storing the result back into the vector.  Returns
     *  true if all bits were originally clear and the result is all set (i.e., returns the overflow bit). */
    bool decrement() {
        return BitVectorSupport::decrement(data(), hull());
    }
    
    /** Add bits as integers.
     *
     *  Treats @p range1 of this vector and @p range2 of the @p other vector as integers, sums them, and stores the result in
     *  @p range1 of this vector.  The ranges must be valid for their respective vectors, and both ranges must be the same
     *  size.  The @p other vector is permitted to be the same vector as <code>this</code>, in which case the ranges are also
     *  permitted to overlap.  Returns the final carry-out value which is not stored in the result. */
    bool add(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::add(other.data(), range2, data(), range1, false);
    }

    /** Add bits as integers.
     *
     *  Treats @p range1 and @p range2 of this vector as integers, sums them, and stores the result in @p range1.  The ranges
     *  must be valid for this vector and both ranges must be the same size.  The are permitted to overlap.  Returns the final
     *  carry-out value which is not stored in the result. */
    bool add(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::add(data(), range2, data(), range1, false);
    }

    /** Add bits as integers.
     *
     *  Treats this vector and the @p other vector as integers, sums them, and stores the result in this vector. Both vectors
     *  must be the same size. The @p other vector is permitted to be the same as <code>this</code> vector, in which case it
     *  numerically doubles the value (like a left shift by one bit).  Returns the final carry-out value which is not stored in
     *  the result. */
    bool add(const BitVector &other) {
        return BitVectorSupport::add(other.data(), other.hull(), data(), hull(), false);
    }

    /** Subtract bits as integers.
     *  
     *  Treats @p range1 of this vector and @p range2 of the @p other vector as integers, subtracts @p other from @p this, and
     *  stores the result in @p range1 of this vector.  The ranges must be valid for their respective vectors, and both ranges
     *  must be the same size.  The @p other vector is permitted to be the same vector as <code>this</code>, in which case the
     *  ranges are also permitted to overlap.  Returns false only when an overflow occurs (i.e., the integer interpretation of
     *  this vector is unsigned-greater-than the integer from the @p other vector). If the vectors are interpreted as two's
     *  complement signed integers then an overflow is indicated when both operands have the same sign and the result has a
     *  different sign. */
    bool subtract(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::subtract(other.data(), range2, data(), range1);
    }

    /** Subtract bits as integers.
     *  
     *  Treats @p range1 and @p range2 of this vector as integers, subtracts the integer in @p range2 from the integer in @p
     *  range1, and stores the result in @p range1 of this vector.  The ranges must be valid for this vector, and both ranges
     *  must be the same size.  The ranges are permitted to overlap.  Returns false only when an overflow occurs (i.e., the
     *  integer interpretation of @p range1 is unsigned-greater-than the integer from @p range2). If the ranges are interpreted
     *  as containing two's complement signed integers then an overflow is indicated when both operands have the same sign and
     *  the result has a different sign. */
    bool subtract(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::subtract(data(), range2, data(), range1);
    }

    /** Subtract bits as integers.
     *
     *  Treats this vector and the @p other vector as integers, subtracts @p other from @p this, and stores the result in this
     *  vector. Both vectors must be the same size. The @p other vector is permitted to be the same as <code>this</code>
     *  vector, in which case this vector is filled with zero. */
    bool subtract(const BitVector &other) {
        return BitVectorSupport::subtract(other.data(), other.hull(), data(), hull());
    }

    /** Copy bits and sign extend.
     *
     *  Copies bits from @p range2 of the @p other vector to @p range1 of this vector while sign extending. That is, if the
     *  destination is larger than the source, the most significant bit of the source is repeated to fill the high order bits
     *  of the destination.  Both ranges must be valid for their respective vectors.  The @p other vector is permitted to be
     *  the same as <code>this</code> vector, in which case the ranges are also permitted to overlap. */
    BitVector& signExtend(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        BitVectorSupport::signExtend(other.data(), range2, data(), range1);
        return *this;
    }

    /** Copy bits and sign extend.
     *
     *  Copies bits from @p range2 of this vector to @p range1 of this vector while sign extending. That is, if the
     *  destination is larger than the source, the most significant bit of the source is repeated to fill the high order bits
     *  of the destination.  Both ranges must be valid for this vector, and are permitted to overlap. */
    BitVector& signExtend(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        BitVectorSupport::signExtend(data(), range2, data(), range1);
        return *this;
    }

    /** Copy bits and sign extend.
     *
     *  Copies bits from the @p other vector to this vector while sign extending. That is, if the destination is larger than
     *  the source, the most significant bit of the source is repeated to fill the high order bits of the destination. */
    BitVector& signExtend(const BitVector &other) {
        BitVectorSupport::signExtend(other.data(), other.hull(), data(), hull());
        return *this;
    }

    // FIXME[Robb Matzke 2014-05-01]: we should also have zeroExtend, which is like copy but allows the source and destination
    // to be different sizes.
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Bit-wise Boolean logic
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Invert bits.
     *
     *  Each bit in the specified range is inverted.  The range must be valid for this vector. */
    BitVector& invert(const BitRange &range) {
        checkRange(range);
        BitVectorSupport::invert(data(), range);
        return *this;
    }

    /** Invert bits.
     *
     *  Each bit in this vector is inverted. */
    BitVector& invert() {
        BitVectorSupport::invert(data(), hull());
        return *this;
    }

    /** Bit-wise AND.
     *
     *  Computes the bit-wise AND of @p range1 from this vector and @p range2 of the @p other vector, storing the result in @p
     *  range1.  The ranges must be valid for their respective vectors and must be the same size.  The @p other vector is
     *  permitted to refer to @p this vector, in which case the ranges are also permitted to overlap. */
    BitVector& bitwiseAnd(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        BitVectorSupport::bitwiseAnd(other.data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise AND.
     *
     *  Computes the bit-wise AND of @p range1 and @p range2 of this vector, storing the result in @p range1.  The ranges must
     *  be valid for this vector and must be the same size. They are permitted to overlap. */
    BitVector& bitwiseAnd(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        BitVectorSupport::bitwiseAnd(data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise AND.
     *
     *  Computes the bit-wise AND of this vector and the @p other vector, storing the result in this vector.  The vectors
     *  must be the same size. */
    BitVector& bitwiseAnd(const BitVector &other) {
        BitVectorSupport::bitwiseAnd(other.data(), other.hull(), data(), hull());
        return *this;
    }
    
    /** Bit-wise OR.
     *
     *  Computes the bit-wise OR of @p range1 from this vector and @p range2 of the @p other vector, storing the result in @p
     *  range1.  The ranges must be valid for their respective vectors and must be the same size.  The @p other vector is
     *  permitted to refer to @p this vector, in which case the ranges are also permitted to overlap. */
    BitVector& bitwiseOr(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        BitVectorSupport::bitwiseOr(other.data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise OR.
     *
     *  Computes the bit-wise OR of @p range1 and @p range2 of this vector, storing the result in @p range1.  The ranges must
     *  be valid for this vector and must be the same size. They are permitted to overlap. */
    BitVector& bitwiseOr(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        BitVectorSupport::bitwiseOr(data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise OR.
     *
     *  Computes the bit-wise OR of this vector and the @p other vector, storing the result in this vector.  The vectors
     *  must be the same size. */
    BitVector& bitwiseOr(const BitVector &other) {
        BitVectorSupport::bitwiseOr(other.data(), other.hull(), data(), hull());
        return *this;
    }

    /** Bit-wise XOR.
     *
     *  Computes the bit-wise XOR of @p range1 from this vector and @p range2 of the @p other vector, storing the result in @p
     *  range1.  The ranges must be valid for their respective vectors and must be the same size.  The @p other vector is
     *  permitted to refer to @p this vector, in which case the ranges are also permitted to overlap. */
    BitVector& bitwiseXor(const BitRange &range1, const BitVector &other, const BitRange &range2) {
        checkRange(range1);
        other.checkRange(range2);
        BitVectorSupport::bitwiseXor(other.data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise XOR.
     *
     *  Computes the bit-wise XOR of @p range1 and @p range2 of this vector, storing the result in @p range1.  The ranges must
     *  be valid for this vector and must be the same size. They are permitted to overlap. */
    BitVector& bitwiseXor(const BitRange &range1, const BitRange &range2) {
        checkRange(range1);
        checkRange(range2);
        BitVectorSupport::bitwiseXor(data(), range2, data(), range1);
        return *this;
    }

    /** Bit-wise XOR.
     *
     *  Computes the bit-wise XOR of this vector and the @p other vector, storing the result in this vector.  The vectors
     *  must be the same size. */
    BitVector& bitwiseXor(const BitVector &other) {
        BitVectorSupport::bitwiseXor(other.data(), other.hull(), data(), hull());
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Numeric comparisons
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Compare to zero.
     *
     *  Compares the integer value referred to by the specified range with zero.  Returns true if the value is equal to zero
     *  the range is empty.
     *
     *  @sa isAllClear */
    bool isEqualToZero(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::isEqualToZero(data(), range);
    }

    /** Compare to zero.
     *
     *  Returns true if this vector is empty all bits are false.
     *
     *  @sa isAllClear */
    bool isEqualToZero() const {
        return BitVectorSupport::isEqualToZero(data(), hull());
    }

    /** Compare bits as integers.
     *
     *  Compares @p range1 from this vector with @p range2 from the @p other vector as integers and returns a value whose sign
     *  indicates the ordering relationship between the two ranges. Returns negative if the @p range1 value is less than the @p
     *  range2 value, returns zero if they are equal, and returns positive if the @p range1 value is greater than the @p range2
     *  value. The ranges must be valid for their respective vectors, and need not be the same size (the smaller range will be
     *  temporarily zero extended on its most significant end). An empty range is treated as zero. The @p other vector is
     *  permitted to refer to @p this vector, in which case the ranges are also permitted to overlap. */
    int compare(const BitRange &range1, const BitVector &other, const BitRange &range2) const {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::compare(data(), range1, other.data(), range2);
    }

    /** Compare bits as integers.
     *
     *  Compares @p range1 and @p range2 from this vector as integers and returns a value whose sign indicates the ordering
     *  relationship between the two ranges. Returns negative if the @p range1 value is less than the @p range2 value, returns
     *  zero if they are equal, and returns positive if the @p range1 value is greater than the @p range2 value. The ranges
     *  must be valid for this vector, and need not be the same size (the smaller range will be temporarily zero extended on
     *  its most significant end).  An empty range is interpreted as zero. The ranges are permitted to overlap. */
    int compare(const BitRange &range1, const BitRange &range2) const {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::compare(data(), range1, data(), range2);
    }

    /** Compare bits as integers.
     *
     *  Compares the bits of this vector with the bits of @p other as integers and returns a value whose sign indicates the
     *  ordering relationship between the two ranges. Returns negative if @p this value is less than the @p other value,
     *  returns zero if they are equal, and returns positive if @p this value is greater than the @p other value. The vectors
     *  need not be the same size (the smaller vector will be temporarily zero extended on its most significant end). An empty
     *  vector is treated as zero. */
    int compare(const BitVector &other) const {
        return BitVectorSupport::compare(data(), hull(), other.data(), other.hull());
    }
    
    /** Compare bits as signed integers.
     *
     *  Compares @p range1 from this vector with @p range2 from the @p other vector as signed, two's complement integers and
     *  returns a value whose sign indicates the ordering relationship between the two ranges. Returns negative if the @p
     *  range1 value is less than the @p range2 value, returns zero if they are equal, and returns positive if the @p range1
     *  value is greater than the @p range2 value. The ranges must be valid for their respective vectors, and need not be the
     *  same size (the smaller range will be temporarily zero extended on its most significant end). An empty range is treated
     *  as zero. The @p other vector is permitted to refer to @p this vector, in which case the ranges are also permitted to
     *  overlap. */
    int compareSigned(const BitRange &range1, const BitVector &other, const BitRange &range2) const {
        checkRange(range1);
        other.checkRange(range2);
        return BitVectorSupport::compareSigned(data(), range1, other.data(), range2);
    }

    /** Compare bits as signed integers.
     *
     *  Compares @p range1 and @p range2 from this vector as signed, two's complement integers and returns a value whose sign
     *  indicates the ordering relationship between the two ranges. Returns negative if the @p range1 value is less than the @p
     *  range2 value, returns zero if they are equal, and returns positive if the @p range1 value is greater than the @p range2
     *  value. The ranges must be valid for this vector, and need not be the same size (the smaller range will be temporarily
     *  zero extended on its most significant end).  An empty range is interpreted as zero. The ranges are permitted to
     *  overlap. */
    int compareSigned(const BitRange &range1, const BitRange &range2) const {
        checkRange(range1);
        checkRange(range2);
        return BitVectorSupport::compareSigned(data(), range1, data(), range2);
    }

    /** Compare bits as signed integers.
     *
     *  Compares the bits of this vector with the bits of @p other as signed, two's complement integers and returns a value
     *  whose sign indicates the ordering relationship between the two ranges. Returns negative if @p this value is less than
     *  the @p other value, returns zero if they are equal, and returns positive if @p this value is greater than the @p other
     *  value. The vectors need not be the same size (the smaller vector will be temporarily zero extended on its most
     *  significant end). An empty vector is treated as zero. */
    int compareSigned(const BitVector &other) const {
        return BitVectorSupport::compareSigned(data(), hull(), other.data(), other.hull());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Conversion
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Interpret bits as an unsigned integer.
     *
     *  Returns the bits of the specified range by interpreting them as an unsigned integer.  The range must be valid for this
     *  vector. If the range contains more than 64 bits then only the low-order 64 bits are considered. */
    boost::uint64_t toInteger(const BitRange &range) const {
        checkRange(range);
        return BitVectorSupport::toInteger(data(), range);
    }
    
    /** Interpret bits as an unsigned integer.
     *
     *  Returns the bits of this vector by interpreting them as an unsigned integer.  If this vector contains more than 64 bits
     *  then only the low-order 64 bits are considered. */
    boost::uint64_t toInteger() const {
        if (size() <= 64)
            return BitVectorSupport::toInteger(data(), size());
        return BitVectorSupport::toInteger(data(), hull());
    }

    /** Convert to a hexadecimal string.
     *
     *  Returns a string which is the hexadecimal representation of the bits in the specified range.  The range must be valid
     *  for this vector. No prefix or suffix is added (e.g., no leading "0x" or trailing "h").  The number of digits in the
     *  return value is the minimum required to explicitly represent each bit of the range, including leading zeros; an empty
     *  range will return an empty string. The returned string is lower case. */
    std::string toHex(const BitRange &range) const {
        return BitVectorSupport::toHex(data(), range);
    }
    
    /** Convert to a hexadecimal string.
     *
     *  Returns a string which is the hexadecimal representation of the bits in this vector.  No prefix or suffix is added
     *  (e.g., no leading "0x" or trailing "h").  The number of digits in the return value is the minimum required to
     *  explicitly represent each bit of the vector, including leading zeros; an empty vector will return an empty string. The
     *  returned string is lower case. */
    std::string toHex() const {
        return BitVectorSupport::toHex(data(), hull());
    }

    /** Convert to an octal string.
     *
     *  Returns a string which is the octal representation of the bits in the specified range.  The range must be valid for
     *  this vector. No prefix or suffix is added (e.g., no extra leading "0" or trailing "o").  The number of digits in the
     *  return value is the minimum required to explicitly represent each bit of the range, including leading zeros; an empty
     *  range will return an empty string. */
    std::string toOctal(const BitRange &range) const {
        return BitVectorSupport::toOctal(data(), range);
    }
    
    /** Convert to an octal string.
     *
     *  Returns a string which is the octal representation of the bits in this vector.  No prefix or suffix is added
     *  (e.g., no leading "0" or trailing "o").  The number of digits in the return value is the minimum required to
     *  explicitly represent each bit of the vector, including leading zeros; an empty vector will return an empty string. */
    std::string toOctal() const {
        return BitVectorSupport::toOctal(data(), hull());
    }

    /** Convert to a binary string.
     *
     *  Returns a string which is the binary representation of the bits in the specified range.  The range must be valid for
     *  this vector. No prefix or suffix is added (e.g., no extra leading or trailing "b").  The number of digits in the
     *  return value is the minimum required to explicitly represent each bit of the range, including leading zeros; an empty
     *  range will return an empty string. */
    std::string toBinary(const BitRange &range) const {
        return BitVectorSupport::toBinary(data(), range);
    }
    
    /** Convert to an binary string.
     *
     *  Returns a string which is the binary representation of the bits in this vector.  No prefix or suffix is added
     *  (e.g., no leading or trailing "b").  The number of digits in the return value is the minimum required to
     *  explicitly represent each bit of the vector, including leading zeros; an empty vector will return an empty string. */
    std::string toBinary() const {
        return BitVectorSupport::toBinary(data(), hull());
    }

    /** Obtain bits from an integer.
     *
     *  Assigns the specified value to the bits indicated by @p range of this vector.  If the range contains fewer than 64 bits
     *  then only the low order bits of @p value are used; if the range contains more than 64 bits then the high-order bits are
     *  cleared. The range must be a valid range for this vector. */
    BitVector& fromInteger(const BitRange &range, boost::uint64_t value) {
        checkRange(range);
        BitVectorSupport::fromInteger(data(), range, value);
        return *this;
    }

    /** Obtain bits from an integer.
     *
     *  Assigns the specified value to this vector.  If this vector contains fewer than 64 bits then only the low order bits of
     *  @p value are used; if this vector contains more than 64 bits then the high-order bits are cleared. The size of this
     *  vector is not changed by this operation.
     *
     *  @sa The assignment operator. */
    BitVector& fromInteger(boost::uint64_t value) {
        BitVectorSupport::fromInteger(data(), hull(), value);
        return *this;
    }

    /** Obtains bits from a decimal representation.
     *
     *  
     *  Assigns the specified value, represented in decimal, to the specified range of this vector. The @p input string must
     *  contain only valid decimal digits '0' through '9' or the underscore character (to make long strings more readable), or
     *  else an <code>std::runtime_error</code> is thrown. The range must be valid for this vector. If the number of supplied
     *  digits is larger than what is required to initialize the specified range then the extra data is discarded.  On the
     *  other hand, if the length of the string is insufficient to initialize the entire range then the high order bits of the
     *  range are cleared. */
    BitVector& fromDecimal(const BitRange &range, const std::string &input) {
        checkRange(range);
        BitVectorSupport::fromDecimal(data(), range, input);
        return *this;
    }

    /** Obtain bits from a decimal representation.
     *
     *  Assigns the specified value, represented in decimal, to this vector. The @p input string must contain only valid
     *  decimal digits '0' through '9' or the underscore character (to make long strings more readable), or else an
     *  <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger than what is required to
     *  initialize this vector then the extra data is discarded.  On the other hand, if the length of the string is insufficient
     *  to initialize the entire vector then the high order bits of the vector are cleared. The size of this vector is not
     *  changed by this operation. */
    BitVector& fromDecimal(const std::string &input) {
        BitVectorSupport::fromDecimal(data(), hull(), input);
        return *this;
    }

    /** Obtain bits from a hexadecimal representation.
     *
     *  Assigns the specified value, represented in hexadecimal, to the specified range of this vector. The @p input string
     *  must contain only valid hexadecimal digits '0' through '9', 'a' through 'f', and 'A' through 'F', or the underscore
     *  character (to make long strings more readable), or else an <code>std::runtime_error</code> is thrown. The range must be
     *  valid for this vector. If the number of supplied digits is larger than what is required to initialize the specified
     *  range then the extra data is discarded.  On the other hand, if the length of the string is insufficient to initialize
     *  the entire range then the high order bits of the range are cleared. */
    BitVector& fromHex(const BitRange &range, const std::string &input) {
        checkRange(range);
        BitVectorSupport::fromHex(data(), range, input);
        return *this;
    }

    /** Obtain bits from a hexadecimal representation.
     *
     *  Assigns the specified value, represented in hexadecimal, to this vector. The @p input string must contain only valid
     *  hexadecimal digits '0' through '9', 'a' through 'f', and 'A' through 'F', or the underscore character (to make long
     *  strings more readable), or else an <code>std::runtime_error</code> is thrown. If the number of supplied digits is
     *  larger than what is required to initialize this vector then the extra data is discarded.  On the other hand, if the
     *  length of the string is insufficient to initialize the entire vector then the high order bits of the vector are
     *  cleared. The size of this vector is not changed by this operation. */
    BitVector& fromHex(const std::string &input) {
        BitVectorSupport::fromHex(data(), hull(), input);
        return *this;
    }

    /** Obtain bits from an octal representation.
     *
     *  Assigns the specified value, represented in octal, to the specified range of this vector. The @p input string must
     *  contain only valid octal digits '0' through '7' or the underscore character (to make long strings more readable), or
     *  else an <code>std::runtime_error</code> is thrown. The range must be valid for this vector. If the number of supplied
     *  digits is larger than what is required to initialize the specified range then the extra data is discarded.  On the
     *  other hand, if the length of the string is insufficient to initialize the entire range then the high order bits of the
     *  range are cleared. */
    BitVector& fromOctal(const BitRange &range, const std::string &input) {
        checkRange(range);
        BitVectorSupport::fromOctal(data(), range, input);
        return *this;
    }

    /** Obtain bits from an octal representation.
     *
     *  Assigns the specified value, represented in octal, to this vector. The @p input string must contain only valid octal
     *  digits '0' through '7' or the underscore character (to make long strings more readable), or else an
     *  <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger than what is required to
     *  initialize this vector then the extra data is discarded.  On the other hand, if the length of the string is insufficient
     *  to initialize the entire vector then the high order bits of the vector are cleared. The size of this vector is not
     *  changed by this operation. */
    BitVector& fromOctal(const std::string &input) {
        BitVectorSupport::fromOctal(data(), hull(), input);
        return *this;
    }

    /** Obtain bits from a binary representation.
     *
     *  Assigns the specified value, represented in binary, to the specified range of this vector. The @p input string must
     *  contain only valid binary digits '0' and '1' or the underscore character (to make long strings more readable), or else
     *  an <code>std::runtime_error</code> is thrown. The range must be valid for this vector. If the number of supplied digits
     *  is larger than what is required to initialize the specified range then the extra data is discarded.  On the other hand,
     *  if the length of the string is insufficient to initialize the entire range then the high order bits of the range are
     *  cleared. */
    BitVector& fromBinary(const BitRange &range, const std::string &input) {
        checkRange(range);
        BitVectorSupport::fromBinary(data(), range, input);
        return *this;
    }

    /** Obtain bits from a binary representation.
     *
     *  Assigns the specified value, represented in binary, to this vector. The @p input string must contain only valid binary
     *  digits '0' and '1' or the underscore character (to make long strings more readable), or else an
     *  <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger than what is required to
     *  initialize this vector then the extra data is discarded.  On the other hand, if the length of the string is insufficient
     *  to initialize the entire vector then the high order bits of the vector are cleared. The size of this vector is not
     *  changed by this operation. */
    BitVector& fromBinary(const std::string &input) {
        BitVectorSupport::fromBinary(data(), hull(), input);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Utility
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** Assert valid range.
     *
     *  Asserts that the specified range is valid for this vector. This is intended mostly for internal use and does nothing
     *  when assertions are disabled. */
    void checkRange(const BitRange &range) const {
        ASSERT_require(hull().isContaining(range));
    }

    /** Raw data for vector.
     *
     *  Returns a pointer to the raw data for the vector.  This is mostly for internal use so that the raw data can be passed
     *  to the BitVectorSupport functions.
     *
     *  @{ */
    Word* data() {
        return words_.empty() ? NULL : &words_[0];
    }

    const Word* data() const {
        return words_.empty() ? NULL : &words_[0];
    }
    /** @} */

    /** Raw data size.
     *
     *  Returns the number of elements of type Word in the array returned by the @ref data method. */
    size_t dataSize() const {
        return words_.size();
    }
};

} // namespace
} // namespace
#endif
