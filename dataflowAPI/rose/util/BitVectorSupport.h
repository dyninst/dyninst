// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_BitVectorSupport_H
#define Sawyer_BitVectorSupport_H

#include <stddef.h>
#include <boost/cstdint.hpp>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <cstring>
#include "Assert.h"
#include "Interval.h"
#include "Optional.h"
#include "Sawyer.h"
#include <string>
#include <vector>

DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_VLA_GCC_PRAGMA_BUG

namespace Sawyer {
namespace Container {

/** Support functions for bit vectors. */
namespace BitVectorSupport {

/** For removing const qualifiers */
template<class T> struct RemoveConst { typedef T Base; };
template<class T> struct RemoveConst<const T> { typedef T Base; };

typedef Interval<size_t> BitRange;

/** Tags for traversal directions. */
struct LowToHigh {};
struct HighToLow {};

/** Number of bits per word. */
template<class Word>
struct bitsPerWord {
    enum { value = 8 * sizeof(Word) };
};

/** Index to bit vector word.
 *
 *  Returns the index to a bit vector word.  Bit zero of the bit vector is in word index zero. */
template<class Word>
size_t wordIndex(size_t idx) {
    return idx / bitsPerWord<Word>::value;
}

/** Index to bit within word.
 *
 *  Returns the index of a bit within a word. Bit zero is the least significant bit of the word. */
template<class Word>
size_t bitIndex(size_t idx) {
    return idx % bitsPerWord<Word>::value;
}

/** Number of words to hold indicated number of bits. */
template<class Word>
size_t numberOfWords(size_t nbits) {
    return (nbits + bitsPerWord<Word>::value - 1) / bitsPerWord<Word>::value;
}

/** Creates a mask.
 *
 *  Returns a mask which contains all zeros except for @p nbits consecutive bits set beginning at @p offset and continuing in a
 *  more-significant direction. */
template<class Word>
Word bitMask(size_t offset, size_t nbits) {
    ASSERT_require(offset + nbits <= bitsPerWord<Word>::value);
    Word mask = nbits == bitsPerWord<Word>::value ? Word(-1) : (Word(1) << nbits) - 1;
    return mask << offset;
}

/** Invoke the a processor for a vector traversal.
 *
 *  Returns true when the word is "found" and the traversal can abort.
 *
 *  @{ */
template<class Processor, class Word>
bool processWord(Processor &processor, const Word &word, size_t shift, size_t nbits) {
    ASSERT_require(shift < bitsPerWord<Word>::value);
    ASSERT_require(nbits <= bitsPerWord<Word>::value);
    const Word tmp = word >> shift;
    return processor(tmp, nbits);
}

template<class Processor, class Word>
bool processWord(Processor &processor, Word &word, size_t shift, size_t nbits) {
    ASSERT_require(shift < bitsPerWord<Word>::value);
    Word tmp = word >> shift;
    bool retval = processor(tmp, nbits);
    word &= ~bitMask<Word>(shift, nbits);
    word |= (tmp & bitMask<Word>(0, nbits)) << shift;
    return retval;
}

template<class Processor, class Word>
bool processWord(Processor &processor, const Word &src, Word &dst, size_t shift, size_t nbits) {
    ASSERT_require(shift < bitsPerWord<Word>::value);
    ASSERT_require(nbits <= bitsPerWord<Word>::value);
    const Word tmpSrc = src >> shift;
    Word tmpDst = dst >> shift;
    bool retval = processor(tmpSrc, tmpDst, nbits);
    dst &= ~bitMask<Word>(shift, nbits);
    dst |= (tmpDst & bitMask<Word>(0, nbits)) << shift;
    return retval;
}
template<class Processor, class Word>
bool processWord(Processor &processor, Word &w1, Word &w2, size_t shift, size_t nbits) {
    ASSERT_require(shift < bitsPerWord<Word>::value);
    ASSERT_require(nbits <= bitsPerWord<Word>::value);
    Word tmp1 = w1 >> shift;
    Word tmp2 = w2 >> shift;
    bool retval = processor(tmp1, tmp2, nbits);
    Word mask = bitMask<Word>(shift, nbits);
    w1 &= ~mask;
    w1 |= (tmp1 << shift) & mask;
    w2 &= ~mask;
    w2 |= (tmp2 << shift) & mask;
    return retval;
}
template<class Processor, class Word>
bool processWord(Processor &processor, const Word &w1, const Word &w2, size_t shift, size_t nbits) {
    ASSERT_require(shift < bitsPerWord<Word>::value);
    ASSERT_require(nbits <= bitsPerWord<Word>::value);
    Word tmp1 = w1 >> shift;
    Word tmp2 = w2 >> shift;
    return processor(tmp1, tmp2, nbits);
}
/** @} */

// Internal function to copy bits from one place to another. The two places must not overlap.
template<class Word>
void nonoverlappingCopy(const Word *src, const BitRange &srcRange, Word *dst, const BitRange &dstRange) {
    ASSERT_require(srcRange.size()==dstRange.size());
    ASSERT_require(src != dst);

    size_t dstFirstIdx = wordIndex<Word>(dstRange.least());
    size_t dstLastIdx  = wordIndex<Word>(dstRange.greatest());
    size_t srcFirstIdx = wordIndex<Word>(srcRange.least());
    size_t srcLastIdx  = wordIndex<Word>(srcRange.greatest());

    size_t srcOffset = srcFirstIdx - dstFirstIdx; // overflow is okay

    size_t srcBitIdx = bitIndex<Word>(srcRange.least());
    size_t dstBitIdx = bitIndex<Word>(dstRange.least());

    if (srcBitIdx < dstBitIdx) {
        // This is effectively a left shift, taking data from lower src elements
        const size_t leftShiftAmount = dstBitIdx - srcBitIdx;
        const size_t rightShiftAmount = bitsPerWord<Word>::value - leftShiftAmount;

        for (size_t i=dstFirstIdx; i<=dstLastIdx; ++i) {
            const Word tmp = (i+srcOffset > srcFirstIdx ? (src[i+srcOffset-1] >> rightShiftAmount) : Word(0)) |
                             (src[i+srcOffset] << leftShiftAmount);
            Word mask = bitMask<Word>(0, bitsPerWord<Word>::value);
            if (i==dstFirstIdx)
                mask &= ~bitMask<Word>(0, bitIndex<Word>(dstRange.least()));
            if (i==dstLastIdx)
                mask &= bitMask<Word>(0, bitIndex<Word>(dstRange.greatest())+1);
            dst[i] &= ~mask;
            dst[i] |= tmp & mask;
        }
    } else if (srcBitIdx > dstBitIdx) {
        // This is effectively a right shift, taking data from higher src elements
        const size_t rightShiftAmount = srcBitIdx - dstBitIdx;
        const size_t leftShiftAmount = bitsPerWord<Word>::value - rightShiftAmount;

        for (size_t i=dstFirstIdx; i<=dstLastIdx; ++i) {
            const Word tmp = (src[i+srcOffset] >> rightShiftAmount) |
                             (i+srcOffset+1 <= srcLastIdx ? (src[i+srcOffset+1] << leftShiftAmount) : Word(0));
            Word mask = bitMask<Word>(0, bitsPerWord<Word>::value);
            if (i==dstFirstIdx)
                mask &= ~bitMask<Word>(0, bitIndex<Word>(dstRange.least()));
            if (i==dstLastIdx)
                mask &= bitMask<Word>(0, bitIndex<Word>(dstRange.greatest())+1);
            dst[i] &= ~mask;
            dst[i] |= tmp & mask;
        }
    } else {
        // No shifting necessary
        for (size_t i=dstFirstIdx; i<=dstLastIdx; ++i) {
            const Word tmp = src[i+srcOffset];
            Word mask = bitMask<Word>(0, bitsPerWord<Word>::value);
            if (i==dstFirstIdx)
                mask &= ~bitMask<Word>(0, bitIndex<Word>(dstRange.least()));
            if (i==dstLastIdx)
                mask &= bitMask<Word>(0, bitIndex<Word>(dstRange.greatest())+1);
            dst[i] &= ~mask;
            dst[i] |= tmp & mask;
        }
    }
}

template<class Src, class Dst>
void conditionalCopy(const Src *src, const BitRange &srcRange, Dst *dst, const BitRange &dstRange) {
    nonoverlappingCopy(src, srcRange, dst, dstRange);
}
template<class Src, class Dst>
void conditionalCopy(const Src, const BitRange, const Dst, const BitRange) {
    // do not copy when dst is const
}

/** Traverses a range of bits.
 *
 *  Traverses a subset of the bits in the @p words array by invoking the @p processor function on each word.  The least
 *  significant traversed word will be right-shifted if necessary so that the least significant bit of the range will be at
 *  bit index zero when the @p processor is invoked.  The visit method must not modify the word. */
template<class Processor, class Word>
void traverse(Processor &processor, Word *words, const BitRange &range, LowToHigh) {
    if (range.isEmpty())
        return;
    size_t nRemaining = range.size();
    size_t offsetInWord = bitIndex<Word>(range.least());
    bool done = false;
    for (size_t wordIdx = wordIndex<Word>(range.least()); !done && nRemaining > 0; ++wordIdx) {
        size_t nbits = std::min(bitsPerWord<Word>::value - offsetInWord, nRemaining);
        ASSERT_require(nbits > 0);
        done = processWord(processor, words[wordIdx], offsetInWord, nbits);
        offsetInWord = 0;                               // only the first word has an internal bit offset
        nRemaining -= nbits;
    }
}

/** Traverse one range of bits starting with the most significant bit. */
template<class Processor, class Word>
void traverse(Processor &processor, Word *words, const BitRange &range, HighToLow) {
    if (range.isEmpty())
        return;
    size_t nRemaining = range.size();
    size_t offsetInWord = bitIndex<Word>(range.least());
    bool done = false;
    size_t firstWordIdx = wordIndex<Word>(range.least());
    size_t lastWordIdx = wordIndex<Word>(range.greatest());
    for (size_t wordIdx=lastWordIdx; !done && nRemaining > 0; --wordIdx) {
        size_t nbits;
        if (wordIdx == firstWordIdx) {
            ASSERT_require(nRemaining <= bitsPerWord<Word>::value);
            nbits = nRemaining;
        } else if (wordIdx < lastWordIdx) {
            ASSERT_require(nRemaining > bitsPerWord<Word>::value);
            nbits = bitsPerWord<Word>::value;
        } else {
            ASSERT_require(wordIdx==lastWordIdx);
            ASSERT_require(wordIdx>firstWordIdx);
            size_t nBitsToLeft = (bitsPerWord<Word>::value - offsetInWord) +
                                 (lastWordIdx-firstWordIdx-1) * bitsPerWord<Word>::value;
            ASSERT_require(nRemaining > nBitsToLeft);
            nbits = nRemaining - nBitsToLeft;
            ASSERT_require(nbits <= bitsPerWord<Word>::value);
        }
        done = processWord(processor, words[wordIdx], wordIdx==firstWordIdx ? offsetInWord : 0, nbits);
        nRemaining -= nbits;
    }
}

/** Traverse two ranges of bits from low to high. */
template<class Processor, class Word1, class Word2>
void traverse2(Processor &processor, Word1 *vec1, const BitRange &range1, Word2 *vec2, const BitRange &range2, LowToHigh) {
    ASSERT_require(sizeof(Word1)==sizeof(Word2));       // may differ in constness
    ASSERT_require((range1.isEmpty() && range2.isEmpty()) || (!range1.isEmpty() && !range2.isEmpty()));
    if (range1.isEmpty())
        return;
    ASSERT_require(range1.size() == range2.size());

    // Make a copy of the source and give it the same bit alignment as the destination.  This not only makes traversal easier
    // (since we can traverse whole words at a time) but it also makes it so we don't need to worry about traversal order when
    // the source and destination overlap.
    size_t offsetInWord = bitIndex<Word2>(range2.least());
    const size_t nWordsTmp = numberOfWords<Word2>(offsetInWord + range2.size());
    SAWYER_VARIABLE_LENGTH_ARRAY(typename RemoveConst<Word1>::Base, tmp, nWordsTmp);
    memset(tmp, 0, nWordsTmp*sizeof(*tmp));                         // only for making debugging easier
    BitRange tmpRange = BitRange::baseSize(offsetInWord, range1.size());
    nonoverlappingCopy(vec1, range1, tmp, tmpRange);

    // Do the traversal.  The first iteration's words are offset by offsetInWord bits, the remainder start at bit zero. All the
    // words except possibly the first and last are the full size.
    const size_t vec2WordOffset = wordIndex<Word2>(range2.least());
    size_t nRemaining = range2.size();
    bool done = false;
    for (size_t wordIdx=0; !done && nRemaining > 0; ++wordIdx) {
        ASSERT_require(wordIdx < nWordsTmp);
        size_t nbits = std::min(bitsPerWord<Word2>::value - offsetInWord, nRemaining);
        ASSERT_require(nbits > 0);
        done = processWord(processor, *const_cast<Word1*>(tmp+wordIdx), vec2[wordIdx+vec2WordOffset], offsetInWord, nbits);
        offsetInWord = 0;                               // only the first word has an internal bit offset
        nRemaining -= nbits;
    }

    // Copy tmp back into vec1, but only if vec1 is non-const
    conditionalCopy(tmp, tmpRange, vec1, range1);
}

/** Traverse two ranges of bits from high to low. */
template<class Processor, class Word1, class Word2>
void traverse2(Processor &processor, Word1 *vec1, const BitRange &range1, Word2 *vec2, const BitRange &range2, HighToLow) {
    ASSERT_require(sizeof(Word1)==sizeof(Word2));       // may differ in constness
    ASSERT_require((range1.isEmpty() && range2.isEmpty()) || (!range1.isEmpty() && !range2.isEmpty()));
    if (range1.isEmpty())
        return;
    ASSERT_require(range1.size() == range2.size());

    // Make a copy of the source and give it the same bit alignment as the destination.  This not only makes traversal easier
    // (since we can traverse whole words at a time) but it also makes it so we don't need to worry about traversal order when
    // the source and destination overlap.
    const size_t offsetInWord = bitIndex<Word2>(range2.least());
    const size_t nWordsTmp = numberOfWords<Word2>(offsetInWord + range2.size());
    SAWYER_VARIABLE_LENGTH_ARRAY(typename RemoveConst<Word1>::Base, tmp, nWordsTmp);
    BitRange tmpRange = BitRange::baseSize(offsetInWord, range1.size());
    nonoverlappingCopy(vec1, range1, tmp, tmpRange);

    // Traversal high-to-low.
    size_t nRemaining = range2.size();
    bool done = false;
    const size_t vec2WordOffset = wordIndex<Word2>(range2.least());
    for (size_t wordIdx=nWordsTmp-1; !done && nRemaining>0; --wordIdx) {
        size_t nbits;
        if (wordIdx == 0) {
            ASSERT_require(nRemaining <= bitsPerWord<Word2>::value);
            nbits = nRemaining;
        } else if (wordIdx < nWordsTmp-1) {
            ASSERT_require(nRemaining > bitsPerWord<Word2>::value);
            nbits = bitsPerWord<Word2>::value;
        } else {
            ASSERT_require(wordIdx==nWordsTmp-1);
            ASSERT_require(wordIdx>0);
            size_t nBitsToLeft = (bitsPerWord<Word2>::value - offsetInWord) + (nWordsTmp-2) * bitsPerWord<Word2>::value;
            ASSERT_require(nRemaining > nBitsToLeft);
            nbits = nRemaining - nBitsToLeft;
            ASSERT_require(nbits <= bitsPerWord<Word2>::value);
        }
        done = processWord(processor, *const_cast<Word1*>(tmp+wordIdx), vec2[wordIdx+vec2WordOffset],
                           wordIdx==0 ? offsetInWord : 0, nbits);
        nRemaining -= nbits;
    }

    // Copy tmp back into vec1, but only if vec1 is non-const
    conditionalCopy(tmp, tmpRange, vec1, range1);
}

template<class Processor, class Word>
void traverse(Processor &processor,
              const Word *vec1, const BitRange &range1,
              const Word *vec2, const BitRange &range2,
              HighToLow dir) {
    traverse2(processor, vec1, range1, vec2, range2, dir);
}

template<class Processor, class Word>
void traverse(Processor &processor,
              const Word *vec1, const BitRange &range1,
              const Word *vec2, const BitRange &range2,
              LowToHigh dir) {
    traverse2(processor, vec1, range1, vec2, range2, dir);
}
template<class Processor, class Word>
void traverse(Processor &processor,
              const Word *vec1, const BitRange &range1,
                    Word *vec2, const BitRange &range2,
              LowToHigh dir) {
    traverse2(processor, vec1, range1, vec2, range2, dir);
}
template<class Processor, class Word>
void traverse(Processor &processor,
              Word *vec1, const BitRange &range1,
              Word *vec2, const BitRange &range2,
              LowToHigh dir) {
    traverse2(processor, vec1, range1, vec2, range2, dir);
}
    


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Bit accessors
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Return a single bit.
 *
 *  Returns the bit at the specified index. */
template<class Word>
bool get(const Word *words, size_t idx) {
    return 0 != (words[wordIndex<Word>(idx)] & bitMask<Word>(bitIndex<Word>(idx), 1));
}

template<class Word>
struct ClearBits {
    bool operator()(Word &word, size_t nbits) {
        word &= ~bitMask<Word>(0, nbits);
        return false;
    }
};

/** Clear some bits.
 *
 *  Clears all bits in the specified index range. */
template<class Word>
void clear(Word *words, const BitRange &where) {
    ClearBits<Word> visitor;
    traverse(visitor, words, where, LowToHigh());
}

template<class Word>
struct SetBits {
    bool operator()(Word &word, size_t nbits) {
        word |= bitMask<Word>(0, nbits);
        return false;
    }
};

/** Set some bits.
 *
 *  Sets all bits in the specified index range. */
template<class Word>
void set(Word *words, const BitRange &where) {
    SetBits<Word> visitor;
    traverse(visitor, words, where, LowToHigh());
}

/** Set or clear some bits.
 *
 *  Sets or clears all bits in the specified index range. */
template<class Word>
void setValue(Word *words, const BitRange &where, bool value) {
    if (value) {
        set(words, where);
    } else {
        clear(words, where);
    }
}

template<class Word>
struct CopyBits {
    bool operator()(const Word &src, Word &dst, size_t nbits) {
        dst &= ~bitMask<Word>(0, nbits);
        dst |= src & bitMask<Word>(0, nbits);
        return false;
    }
};

/** Copy some bits.
 *
 *  Copies bits from @p src to @p dst according to the ranges @p srcRange and @p dstRange.  The size of the two address ranges
 *  must be the same. The ranges may overlap, and the @p src and @p dst may be the same pointer. */
template<class Word>
void copy(const Word *src, const BitRange &srcRange, Word *dst, const BitRange &dstRange) {
    CopyBits<Word> visitor;
    traverse(visitor, src, srcRange, dst, dstRange, LowToHigh());
}

template<class Word>
struct SwapBits {
    bool operator()(Word &w1, Word &w2, size_t nbits) {
        Word tmp = w1, mask = bitMask<Word>(0, nbits);
        w1 &= ~mask;
        w1 |= w2 & mask;
        w2 &= ~mask;
        w2 |= tmp & mask;
        return false;
    }
};

/** Swap some bits.
 *
 *  Swaps bits from part of one vector with bits from part of another (or the same).  The size of the two address ranges must
 *  be equal and they may not overlap within a single vector. */
template<class Word>
void swap(Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    SwapBits<Word> visitor;
    ASSERT_require(vec1!=vec2 || !range1.isOverlapping(range2));
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Counting/searching
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template<class Word>
struct LeastSignificantSetBit {
    size_t offset;
    Optional<size_t> result;
    LeastSignificantSetBit(): offset(0) {}
    bool operator()(const Word &word, size_t nbits) {
        if (0 != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=0; i<nbits; ++i) {
                if (0 != (word & bitMask<Word>(i, 1))) {
                    result = offset + i;
                    return true;
                }
            }
        }
        offset += nbits;
        return false;
    }
};

/** Index of least significant set bit.
 *
 *  Returns the index of the least significant set bit within the specified range of bits. The return value is the
 *  absolute bit number in the entire vector.  If none of the bits in the range are set then nothing is returned. */
template<class Word>
Optional<size_t> leastSignificantSetBit(const Word *words, const BitRange &range) {
    LeastSignificantSetBit<Word> visitor;
    traverse(visitor, words, range, LowToHigh());
    if (visitor.result)
        return range.least() + *visitor.result;
    return Nothing();
}

template<class Word>
struct LeastSignificantClearBit {
    size_t offset;
    Optional<size_t> result;
    LeastSignificantClearBit(): offset(0) {}
    bool operator()(const Word &word, size_t nbits) {
        if (bitMask<Word>(0, nbits) != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=0; i<nbits; ++i) {
                if (0 == (word & bitMask<Word>(i, 1))) {
                    result = offset + i;
                    return true;
                }
            }
        }
        offset += nbits;
        return false;
    }
};

/** Index of least significant zero bit.
 *
 *  Returns the index of the least significant clear bit within the specified range of bits.  The return value is the absolute
 *  bit number in the entire vector.  If none of the bits in the range are clear then nothing is returned. */
template<class Word>
Optional<size_t> leastSignificantClearBit(const Word *words, const BitRange &range) {
    LeastSignificantClearBit<Word> visitor;
    traverse(visitor, words, range, LowToHigh());
    if (visitor.result)
        return range.least() + *visitor.result;
    return Nothing();
}

template<class Word>
struct MostSignificantSetBit {
    size_t offset;
    Optional<size_t> result;
    MostSignificantSetBit(size_t nbits): offset(nbits) {}
    bool operator()(const Word &word, size_t nbits) {
        ASSERT_require(nbits <= offset);
        offset -= nbits;
        if (0 != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=nbits; i>0; --i) {
                if (0 != (word & bitMask<Word>(i-1, 1))) {
                    result = offset + i-1;
                    return true;
                }
            }
        }
        return false;
    }
};

/** Index of most significant set bit.
 *
 *  Returns the index of the most significant set bit within the specified range of bits. The return value is the
 *  absolute bit number in the entire vector.  If none of the bits in the range are set then nothing is returned. */
template<class Word>
Optional<size_t> mostSignificantSetBit(const Word *words, const BitRange &range) {
    MostSignificantSetBit<Word> visitor(range.size());
    traverse(visitor, words, range, HighToLow());
    if (visitor.result)
        return range.least() + *visitor.result;
    return Nothing();
}

template<class Word>
struct MostSignificantClearBit {
    size_t offset;
    Optional<size_t> result;
    MostSignificantClearBit(size_t nbits): offset(nbits) {}
    bool operator()(const Word &word, size_t nbits) {
        ASSERT_require(nbits <= offset);
        offset -= nbits;
        if (bitMask<Word>(0, nbits) != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=nbits; i>0; --i) {
                if (0 == (word & bitMask<Word>(i-1, 1))) {
                    result = offset + i-1;
                    return true;
                }
            }
        }
        return false;
    }
};

/** Index of most significant clear bit.
 *
 *  Returns the index of the most significant zero bit within the specified range of bits. The return value is the
 *  absolute bit number in the entire vector.  If none of the bits in the range are set then nothing is returned. */
template<class Word>
Optional<size_t> mostSignificantClearBit(const Word *words, const BitRange &range) {
    MostSignificantClearBit<Word> visitor(range.size());
    traverse(visitor, words, range, HighToLow());
    if (visitor.result)
        return range.least() + *visitor.result;
    return Nothing();
}

/** True if all bits are set.
 *
 *  Returns true if the indicated range does not contain a clear bit; an empty range returns true. */
template<class Word>
bool isAllSet(const Word *words, const BitRange &range) {
    return leastSignificantClearBit(words, range) ? false : true;
}

/** True if all bits are clear.
 *
 *  Returns true if the indicated range does not contain a set bit; an empty range returns true. */
template<class Word>
bool isAllClear(const Word *words, const BitRange &range) {
    return leastSignificantSetBit(words, range) ? false : true;
}

template<class Word>
struct CountSetBits {
    size_t result;
    CountSetBits(): result(0) {}
    bool operator()(const Word &word, size_t nbits) {
        if (0 != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=0; i<nbits; ++i) {
                if (0 != (word & bitMask<Word>(i, 1)))
                    ++result;
            }
        }
        return false;
    }
};

/** Number of set bits.
 *
 *  Returns the number of bits that are set in the specified range. */
template<class Word>
size_t nSet(const Word *words, const BitRange &range) {
    CountSetBits<Word> visitor;
    traverse(visitor, words, range, LowToHigh());
    return visitor.result;
}

template<class Word>
struct CountClearBits {
    size_t result;
    CountClearBits(): result(0) {}
    bool operator()(const Word &word, size_t nbits) {
        if (bitMask<Word>(0, nbits) != (word & bitMask<Word>(0, nbits))) {
            for (size_t i=0; i<nbits; ++i) {
                if (0 == (word & bitMask<Word>(i, 1)))
                    ++result;
            }
        }
        return false;
    }
};

/** Number of clear bits.
 *
 *  Returns the number of bits that are clear in the specified range. */
template<class Word>
size_t nClear(const Word *words, const BitRange &range) {
    CountClearBits<Word> visitor;
    traverse(visitor, words, range, LowToHigh());
    return visitor.result;
}

template<class Word>
struct LeastSignificantDifference {
    size_t offset;
    Optional<size_t> result;
    LeastSignificantDifference(): offset(0) {}
    bool operator()(const Word &w1, const Word &w2, size_t nbits) {
        Word mask = bitMask<Word>(0, nbits);
        if ((w1 & mask) != (w2 & mask)) {
            for (size_t i=0; i<nbits; ++i) {
                if ((w1 ^ w2) & bitMask<Word>(i, 1)) {
                    result = offset + i;
                    return true;
                }
            }
        }
        offset += nbits;
        return false;
    }
};

/** Find least significant different bits.
 *
 *  Finds the least significant bit of two sub-vectors where the value is different.  The return value is the offset relative
 *  to the beginning of the sub-vectors where the first difference is found; it is not a bit index within the either
 *  vector-as-a-whole (unless the ranges start at zero). If no difference is found then nothing is returned. */
template<class Word>
Optional<size_t> leastSignificantDifference(const Word *vec1, const BitRange &range1,
                                            const Word *vec2, const BitRange &range2) {
    LeastSignificantDifference<Word> visitor;
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
    return visitor.result;
}
    
template<class Word>
struct MostSignificantDifference {
    size_t offset;
    Optional<size_t> result;
    MostSignificantDifference(size_t nbits): offset(nbits) {}
    bool operator()(const Word &w1, const Word &w2, size_t nbits) {
        ASSERT_require(nbits <= offset);
        offset -= nbits;
        Word mask = bitMask<Word>(0, nbits);
        if ((w1 & mask) != (w2 & mask)) {
            for (size_t i=nbits; i>0; --i) {
                if ((w1 ^ w2) & bitMask<Word>(i-1, 1)) {
                    result = offset + i-1;
                    return true;
                }
            }
        }
        return false;
    }
};

/** Find most significant different bits.
 *
 *  Finds the most significant bit of two sub-vectors where the value is different.  The return value is the offset relative to
 *  the beginning of the sub-vectors where the first difference is found; it is not a bit index within the either
 *  vector-as-a-whole (unless the ranges start at zero). If no difference is found then nothing is returned. */
template<class Word>
Optional<size_t> mostSignificantDifference(const Word *vec1, const BitRange &range1,
                                           const Word *vec2, const BitRange &range2) {
    MostSignificantDifference<Word> visitor(range1.size());
    traverse(visitor, vec1, range1, vec2, range2, HighToLow());
    return visitor.result;
}

/** Equality predicate.
 *
 *  Returns true if two vectors are equal.  An empty vector is equal to another empty vector, but not equal to any non-empty
 *  vector.  See also, @ref compare, which treats vectors numerically and can be used in cases when they are different sizes. */
template<class Word>
bool areEqual(const Word *vec1, const BitRange &range1, const Word *vec2, const BitRange &range2) {
    if (range1.size()!=range2.size())
        return false;
    return leastSignificantDifference(vec1, range1, vec2, range2) ? false : true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Shift/rotate
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Shift bits left.
 *
 *  Shifts the bits in the specified range left by @p nShift places, discarding the high-order bits and introducing low-order
 *  bits with the value @p newBits. */
template<class Word>
void shiftLeft(Word *words, const BitRange &range, size_t nShift, bool newBits=0) {
    if (nShift < range.size()) {
        size_t nbits = range.size() - nShift;           // number of bits to move
        BitRange shiftedFrom = BitRange::baseSize(range.least(), nbits);
        BitRange shiftedTo = BitRange::baseSize(range.least() + nShift, nbits);
        BitRange introduced = BitRange::baseSize(range.least(), nShift);
        copy(words, shiftedFrom, words, shiftedTo);
        setValue(words, introduced, newBits);
    } else {
        setValue(words, range, newBits);
    }
}

/** Shift bits right.
 *
 *  Shifts the bits in the specified range right by @p nShift places, discarding the low-order bits and introducing high-order
 *  bits with the value @p newBits. */
template<class Word>
void shiftRight(Word *words, const BitRange &range, size_t nShift, bool newBits=0) {
    if (nShift < range.size()) {
        size_t nbits = range.size() - nShift;           // number of bits to move
        BitRange shiftedFrom = BitRange::baseSize(range.least() + nShift, nbits);
        BitRange shiftedTo = BitRange::baseSize(range.least(), nbits);
        BitRange introduced = BitRange::baseSize(range.least() + nbits, nShift);
        copy(words, shiftedFrom, words, shiftedTo);
        setValue(words, introduced, newBits);
    } else {
        setValue(words, range, newBits);
    }
}

/** Shift bits right arithmetically.
 *
 *  Shifts the bits in the specified range right by @p nShift places, discarding the low-order bits and introducing high-order
 *  bits with the same value as the original high-order bit within the range. */
template<class Word>
void shiftRightArithmetic(Word *words, const BitRange &range, size_t nShift) {
    if (!range.isEmpty()) {
        bool newBits = get(words, range.greatest());
        shiftRight(words, range, nShift, newBits);
    }
}

/** Rotate bits to the right.
 *
 *  Rotates the bits of the specified range to the right by @p nShift places, introducing high-order bits that are the same as
 *  the low-order bits that were removed. */
template<class Word>
void rotateRight(Word *words, const BitRange &range, size_t nShift) {
    if (range.isEmpty())
        return;
    nShift %= range.size();
    if (0==nShift)
        return;

    // Save the low-order bits that will be shifted off
    size_t nSavedWords = numberOfWords<Word>(nShift);
    SAWYER_VARIABLE_LENGTH_ARRAY(Word, saved, nSavedWords);
    BitRange savedRange = BitRange::baseSize(0, nShift);
    copy(words, BitRange::baseSize(range.least(), nShift), saved, savedRange);

    shiftRight(words, range, nShift);

    BitRange introduced = BitRange::baseSize(range.least()+range.size()-nShift, nShift);
    copy(saved, savedRange, words, introduced);
}

/** Rotate bits to the left.
 *
 *  Rotates the bits of the specified range to the left by @p nShift places, introducing low-order bits that are the same as
 *  the high-order bits that were removed. */
template<class Word>
void rotateLeft(Word *words, const BitRange &range, size_t nShift) {
    if (range.isEmpty())
        return;
    nShift %= range.size();
    rotateRight(words, range, range.size()-nShift);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Bit-wise Boolean logic
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Word>
struct InvertBits {
    bool operator()(Word &word, size_t nbits) {
        word ^= bitMask<Word>(0, nbits);
        return false;
    }
};

/** Invert bits.
 *
 *  Inverts all bits in the specified range. */
template<class Word>
void invert(Word *words, const BitRange &range) {
    InvertBits<Word> visitor;
    traverse(visitor, words, range, LowToHigh());
}

template<class Word>
struct AndBits {
    bool operator()(const Word &w1, Word &w2, size_t nbits) {
        w2 &= w1 | ~bitMask<Word>(0, nbits);
        return false;
    }
};

/** Bit-wise AND.
 *
 *  Computes the bitwise AND of equal-size sub-vectors @p vec1 and @p vec2 and stores the result in @p vec2.  The sub-vectors
 *  may overlap. */
template<class Word>
void bitwiseAnd(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    AndBits<Word> visitor;
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
}

template<class Word>
struct OrBits {
    bool operator()(const Word &w1, Word &w2, size_t nbits) {
        w2 |= w1 & bitMask<Word>(0, nbits);
        return false;
    }
};

/** Bit-wise OR.
 *
 *  Computes the bitwise OR of equal-size sub-vectors @p vec1 and @p vec2 and stores the result in @p vec2.  The sub-vectors
 *  may overlap. */
template<class Word>
void bitwiseOr(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    OrBits<Word> visitor;
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
}

template<class Word>
struct XorBits {
    bool operator()(const Word &w1, Word &w2, size_t nbits) {
        w2 ^= w1 & bitMask<Word>(0, nbits);
        return false;
    }
};

/** Bit-wise XOR.
 *
 *  Computes the bitwise exclusive-OR of equal-size sub-vectors @p vec1 and @p vec2 and stores the result in @p vec2.  The
 *  sub-vectors may overlap. */
template<class Word>
void bitwiseXor(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    XorBits<Word> visitor;
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Arithmetic
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Assign an unsigned value to a bit range.
 *
 *  Zero extends or truncates @p value to the same width as @p range and copies it into the bit vector. */
template<class Word>
void fromInteger(Word *words, const BitRange &range, boost::uint64_t value) {
    ASSERT_require(8==sizeof value);
    if (range.isEmpty())
        return;

    // Create a bit vector that is just the value
    size_t nbits = std::min(range.size(), (size_t)64);  // number of significant bits to copy, not fill
    Word wordMask = bitMask<Word>(0, bitsPerWord<Word>::value);
    size_t nTmpWords = numberOfWords<Word>(nbits);
    SAWYER_VARIABLE_LENGTH_ARRAY(Word, tmp, nTmpWords);
    for (size_t i=0; i<nTmpWords; ++i)
        tmp[i] = (value >> (i * bitsPerWord<Word>::value)) & wordMask;

    // Copy the value's bit vector to the destination and zero-fill
    copy(tmp, BitRange::baseSize(0, nbits), words, BitRange::baseSize(range.least(), nbits));
    if (range.size() >= 64) {
        BitRange hi = BitRange::baseSize(range.least() + 64, range.size() - 64);
        clear(words, hi);
    }
}

/** Convert a bit vector to an integer.
 *
 *  Converts the specified range to an unsigned 64-bit value and returns that value. Returns zero if the range is empty.  If
 *  the range is wider than 64 bits then all of its high-order bits are ignored and only the lowest 64 bits are used. */
template<class Word>
boost::uint64_t toInteger(const Word *words, const BitRange &range) {
    boost::uint64_t result = 0;
    ASSERT_require(8==sizeof result);

    // Copy the bits into the low bits of a temporary bit vector
    size_t nbits = std::min(range.size(), (size_t)64);
    size_t nTmpWords = numberOfWords<Word>(nbits);
    SAWYER_VARIABLE_LENGTH_ARRAY(typename RemoveConst<Word>::Base, tmp, nTmpWords);
    memset(tmp, 0, nTmpWords*sizeof(*tmp));
    BitRange lo = BitRange::baseSize(range.least(), nbits);
    copy(words, lo, tmp, BitRange::baseSize(0, nbits));

    // Convert the temporary bit vector to an integer
    for (size_t i=0; i<nTmpWords; ++i)
        result |= (boost::uint64_t)tmp[i] << (i * bitsPerWord<Word>::value);
    return result;
}

/** Convert a small bit vector to an integer.
 *
 *  Faster version of @ref toInteger for instances where the range offset is zero, and the size is not greater than 64 bits. */
template<class Word>
boost::uint64_t toInteger(const Word *words, size_t nbits) {
    boost::uint64_t result = 0;
    ASSERT_require(nbits <= 64);
    size_t nTmpWords = numberOfWords<Word>(nbits);
    for (size_t i=0; i<nTmpWords; ++i)
        result |= (boost::uint64_t)words[i] << (i * bitsPerWord<Word>::value);
    if (nbits < 64)
        result &= ~((~/*UINT64_C(0)*/0ULL) << nbits);
    return result;
}

template<class Word>
struct Increment {
    bool carry;
    Increment(): carry(true) {}
    bool operator()(Word &word, size_t nbits) {
        ASSERT_require(carry);
        Word mask = bitMask<Word>(0, nbits);
        Word arg1 = word & mask;
        Word sum = arg1 + 1;
        word &= ~mask;
        word |= sum & mask;
        carry = (sum & mask) < arg1;
        return !carry;                                  // terminate traversal when carry is false
    }
};

/** Increment.
 *
 *  Interprets @p range of @p vec1 as an integer and increments the value by one.  The return value is the carry-out bit. */
template<class Word>
bool increment(Word *vec1, const BitRange &range1) {
    Increment<Word> visitor;
    traverse(visitor, vec1, range1, LowToHigh());
    return visitor.carry;
}

template<class Word>
struct Decrement {
    bool borrowed;
    Decrement(): borrowed(true) {}
    bool operator()(Word &word, size_t nbits) {
        ASSERT_require(borrowed);
        Word mask = bitMask<Word>(0, nbits);
        Word arg1 = word & mask;
        borrowed = 0==arg1;
        Word difference = (arg1 - 1) & mask;
        word &= ~mask;
        word |= difference;
        return !borrowed;                               // terminate traversal unless we borrowed
    }
};

/** Decrement.
 *
 *  Interprets @p range of @p vec1 as an integer and decrements the value by one. Returns true if the original value was zero
 *  or empty. */
template<class Word>
bool decrement(Word *vec1, const BitRange &range1) {
    Decrement<Word> visitor;
    traverse(visitor, vec1, range1, LowToHigh());
    return visitor.borrowed;
}

/** Negate bits as an integer.
 *
 *  Interprets @p range of @p vec1 as a two's complement integer and negates its value. */
template<class Word>
void negate(Word *vec1, const BitRange &range) {
    invert(vec1, range);
    increment(vec1, range);
}

template<class Word>
struct AddBits {
    bool carry;
    AddBits(bool carry_): carry(carry_) {}
    bool operator()(const Word &w1, Word &w2, size_t nbits) {
        Word mask = bitMask<Word>(0, nbits);
        Word addend1(carry ? 1 : 0);
        Word addend2(w1 & mask);
        Word addend3(w2 & mask);
        Word sum = addend1 + addend2 + addend3;
        w2 &= ~mask;
        w2 |= sum & mask;
        if (nbits == bitsPerWord<Word>::value) {
            carry = sum < addend2 || (sum==addend2 && carry);
        } else {
            carry = 0 != (sum & bitMask<Word>(nbits, 1));
        }
        return false;
    }
};

/** Add bits.
 *
 *  Treats two sub-vectors as unsigned values and add them together, storing the result in the second vector. Both ranges must
 *  be the same size, and the sub-vectors are permitted to overlap. The return value is the carry-out bit. */
template<class Word>
bool add(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2, bool carryIn=false) {
    AddBits<Word> visitor(carryIn);
    traverse(visitor, vec1, range1, vec2, range2, LowToHigh());
    return visitor.carry;
}

/** Subtract bits.
 *
 *  Treats two sub-vectors as unsigned values and subtracts @p vec1 from @p vec2 and stores the result in @p vec2.  Both ranges
 *  must be the same size, and the sub-vectors are permitted to overlap. The return value is false only when an overflow occurs
 *  (i.e., vec2 is unsigned-greater-than vec1). If the vectors are interpreted as 2's complement signed integers then an
 *  overflow is indicated whan both operands ahve the same sign and the result has a different sign. */
template<class Word>
bool subtract(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    size_t nTempWords = numberOfWords<Word>(range1.size());
    SAWYER_VARIABLE_LENGTH_ARRAY(Word, temp, nTempWords);
    BitRange tempRange = BitRange::baseSize(0, range1.size());
    copy(vec1, range1, temp, tempRange);
    invert(temp, tempRange);
    return add(temp, tempRange, vec2, range2, true);
}

/** Sign extend.
 *
 *  Performs sign extension while copying bits from range1 to range2, which may overlap.  If range1 is empty then range2 is
 *  cleared. If range2 is smaller than range1 then the not all bits are copied (high-order bits in range1 are omitted). If
 *  range1 is non-empty and range2 is empty then nothing is copied. The return value is the new high-order (sign) bit of
 *  range2, or false if range2 is empty. */
template<class Word>
bool signExtend(const Word *vec1, const BitRange &range1, Word *vec2, const BitRange &range2) {
    if (range2.isEmpty()) {
        return false;
    } else if (range1.isEmpty()) {
        clear(vec2, range2);
        return false;
    } else if (range1.size() < range2.size()) {         // sign extension
        bool oldSign = get(vec1, range1.greatest());
        copy(vec1, range1, vec2, BitRange::baseSize(range2.least(), range1.size()));
        setValue(vec2, BitRange::hull(range2.least()+range1.size(), range2.greatest()), oldSign);
        return oldSign;
    } else {                                            // truncation or plain copy
        ASSERT_require(range1.size() >= range2.size());
        BitRange copyFrom = BitRange::baseSize(range1.least(), range2.size());
        copy(vec1, copyFrom, vec2, range2);
        return get(vec2, range2.greatest());
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Numeric comparison
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Compares with zero.
 *
 *  Returns true if the vector is equal to zero. */
template<class Word>
bool isEqualToZero(const Word *vec1, const BitRange &range1) {
    return isAllClear(vec1, range1);
}

template<class Word>
struct CompareBits {
    int result;
    CompareBits(): result(0) {}
    bool operator()(const Word &w1, const Word &w2, size_t nbits) {
        Word mask = bitMask<Word>(0, nbits);
        Word tmp1 = w1 & mask;
        Word tmp2 = w2 & mask;
        if (tmp1 < tmp2) {
            result = -1;
            return true;
        } else if (tmp1 > tmp2) {
            result = 1;
            return true;
        } else {
            return false;
        }
    }
};

/** Unsigned comparison.
 *
 *  Interprests two sub-vectors as unsigned integers and returns an integer whose sign indicates the relationship between those
 *  two integers.  Returns negative if @p vec1 is less than @p vec2; returns zero if @p vec1 is equal to @p vec2; returns
 *  positive if @p vec1 is greater than @p vec2.  The ranges need not be the same size--the smaller of the two ranges will act
 *  as if it were zero padded on its most-significant side.  Two empty vectors are interpreted as zero. */
template<class Word>
int compare(const Word *vec1, const BitRange &range1, const Word *vec2, const BitRange &range2) {
    if (range1.isEmpty() && range2.isEmpty()) {
        return 0;
    } else if (range1.isEmpty()) {
        return isEqualToZero(vec2, range2) ? 0 : -1;
    } else if (range2.isEmpty()) {
        return isEqualToZero(vec1, range1) ? 0 : 1;
    } else if (range1.size() < range2.size()) {
        BitRange hi = BitRange::hull(range2.least() + range1.size(), range2.greatest());
        if (!isEqualToZero(vec2, hi))
            return -1;
        BitRange lo = BitRange::baseSize(range2.least(), range1.size());
        return compare(vec1, range1, vec2, lo);
    } else if (range1.size() > range2.size()) {
        BitRange hi = BitRange::hull(range1.least() + range2.size(), range1.greatest());
        if (!isEqualToZero(vec1, hi))
            return 1;
        BitRange lo = BitRange::baseSize(range1.least(), range2.size());
        return compare(vec1, lo, vec2, range2);
    } else {
        ASSERT_require(!range1.isEmpty() && !range2.isEmpty());
        ASSERT_require(range1.size() == range2.size());
        CompareBits<Word> visitor;
        traverse(visitor, vec1, range1, vec2, range2, HighToLow());
        return visitor.result;
    }
}

/** Signed comparison.
 *
 *  Interprests two sub-vectors as 2's complement signed integers and returns an integer whose sign indicates the relationship
 *  between those two integers.  Returns negative if @p vec1 is less than @p vec2; returns zero if @p vec1 is equal to @p vec2;
 *  returns positive if @p vec1 is greater than @p vec2.  The ranges need not be the same size--the smaller of the two ranges
 *  will act as if it were sign-extended to the same size as the other.  Empty vectors are interpreted as zero. */
template<class Word>
int compareSigned(const Word *vec1, const BitRange &range1, const Word *vec2, const BitRange &range2) {
    if (range1.isEmpty() && range2.isEmpty()) {
        return 0;
    } else if (range1.isEmpty()) {
        if (Optional<size_t> mssb = mostSignificantSetBit(vec2, range2)) {
            return *mssb == range2.greatest() ? 1 : -1;
        } else {
            return 0;
        }
    } else if (range2.isEmpty()) {
        if (Optional<size_t> mssb = mostSignificantSetBit(vec1, range1)) {
            return *mssb == range1.greatest() ? -1 : 1;
        } else {
            return 0;
        }
    } else if (range1.size() < range2.size()) {
        BitRange hi = BitRange::hull(range2.least()+range1.size(), range2.greatest());
        bool neg1 = get(vec1, range1.greatest());
        bool neg2 = get(vec2, range2.greatest());
        if (!neg1 && !neg2) {                           // both are non-negative, so leverage unsigned comparison
            return compare(vec1, range1, vec2, range2);
        } else if (neg1 && neg2) {                      // both are negative
            if (nSet(vec2, hi)==hi.size() && get(vec2, hi.least()-1)) {
                return compareSigned(vec1, range1, vec2, BitRange::baseSize(range2.least(), range1.size()));
            } else {
                return 1;                               // vec2 is less than vec1
            }
        } else if (neg1) {                              // vec1 is negative but vec2 isn't
            return -1;
        } else {                                        // vec2 is negative but vec1 isn't
            return 1;
        }
    } else if (range1.size() > range2.size()) {         // call recursively to trigger previous case
        return -1 * compareSigned(vec2, range2, vec1, range1);
    } else {
        ASSERT_require(range1.size()==range2.size());
        bool neg1 = get(vec1, range1.greatest());
        bool neg2 = get(vec2, range2.greatest());
        if (!neg1 && !neg2) {
            return compare(vec1, range1, vec2, range2);
        } else if (neg1 && neg2) {
            if (Optional<size_t> diff = mostSignificantDifference(vec1, range1, vec2, range2)) {
                return get(vec1, range1.least() + *diff) ? 1 : -1;
            } else {
                return 0;
            }
        } else if (neg1) {
            return -1;
        } else {
            return 1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      String representation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Can handle binary, base-4, octal, or hexadecimal
template<class Word, size_t bitsPerDigit>
struct ToString {
    Word remaining;                                     // left over bits from a previous iteration
    size_t nremaining;                                  // number valid low-order bits in "remaining" data member
    std::string reversed;                               // resulting string with the least significant digit first

    ToString(): remaining(0), nremaining(0) {
        ASSERT_require(bitsPerDigit >= 1 && bitsPerDigit <= 4);
        ASSERT_require(bitsPerDigit <= bitsPerWord<Word>::value);
    }
    
    bool operator()(const Word &word, size_t nbits) {
        static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        Word tmp = word & bitMask<Word>(0, nbits);
        ASSERT_require(nremaining < bitsPerDigit);
        while (nremaining+nbits >= bitsPerDigit) {
            size_t nrem = std::min(nremaining, bitsPerDigit); // number left-over bits to use
            size_t nnew = bitsPerDigit - nrem;                // number of new bits to use
            Word digit = (remaining & bitMask<Word>(0, nrem)) | ((tmp & bitMask<Word>(0, nnew)) << nrem);
            reversed += digits[digit];
            nremaining = 0;
            nbits -= nnew;
            tmp >>= nnew;
        }
        nremaining = nbits;
        remaining = tmp;
        return false;
    }

    std::string result() {
        if (nremaining > 0) {
            ASSERT_require(nremaining <= 3);            // digits '0' through '7'
            reversed += '0' + (remaining & bitMask<Word>(0, nremaining));
            nremaining = 0;
        }
        std::string s;
        std::swap(s, reversed);
        std::reverse(s.begin(), s.end());
        return s;
    }
};

/** Hexadecimal representation.
 *
 *  Returns a string which is the hexadecimal representation of the bits in the specified range.  No prefix or suffix is added
 *  (i.e., no leading "0x" or trailing "h").  The number of digits in the return value is the minimum required to explicitly
 *  represent the value, including leading zeros; an empty range will return an empty string. The returned string is lower
 *  case. */
template<class Word>
std::string toHex(const Word *vec, const BitRange &range) {
    ToString<Word, 4> visitor;
    traverse(visitor, vec, range, LowToHigh());
    return visitor.result();
}

/** Octal representation.
 *
 *  Returns a string which is the octal representation of the bits in the specified range.  No prefix or suffix is added (i.e.,
 *  no leading "0" or trailing "o").  The number of digits in the return value is the minimum required to explicitly represent
 *  the value, including leading zeros.  An empty range will return an empty string. */
template<class Word>
std::string toOctal(const Word *vec, const BitRange &range) {
    ToString<Word, 3> visitor;
    traverse(visitor, vec, range, LowToHigh());
    return visitor.result();
}

/** Binary representation.
 *
 *  Returns a string which is the binary representation of the bits in the specified range.  No prefix or suffix is added.  The
 *  number of digits in the return value is the minimum required to explicitly represent the value, including leading zeros.
 *  An empty range will return an empty string. */
template<class Word>
std::string toBinary(const Word *vec, const BitRange &range) {
    ToString<Word, 1> visitor;
    traverse(visitor, vec, range, LowToHigh());
    return visitor.result();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Word>
Word charToDigit(char ch) {
    Word digit;
    if (ch >= '0' && ch <= '9') {
        digit = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        digit = ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        digit = ch - 'A' + 10;
    } else {
        digit = 16;                                     // signals an invalid character
    }
    return digit;
}

template<class Word, size_t bitsPerDigit>
void fromString(Word *vec, const BitRange &range, const std::string &input) {
    ASSERT_require(bitsPerDigit >= 1 && bitsPerDigit <= 4);
    ASSERT_require(bitsPerDigit <= bitsPerWord<Word>::value);

    // Check that all characters of the string are valid.
    for (size_t idx=0; idx<input.size(); ++idx) {
        if ('_'!=input[idx] && charToDigit<Word>(input[idx]) >= Word(1) << bitsPerDigit) {
            throw std::runtime_error(std::string("invalid character '") + input[idx] + "' at index " +
                                     boost::lexical_cast<std::string>(idx) + " in base-" +
                                     boost::lexical_cast<std::string>(1 << bitsPerDigit) + " input string");
        }
    }

    // Copy digits into the vector
    size_t offset = 0;                                  // bit offset from range.least()
    for (size_t idx=input.size(); idx>0 && offset<range.size(); --idx) {
        if ('_'!=input[idx-1]) {
            Word digit = charToDigit<Word>(input[idx-1]);
            size_t nbits = std::min(bitsPerDigit, range.size()-offset);// number of bits of digit to copy into vector
            copy(&digit, BitRange::baseSize(0, std::min(bitsPerDigit, nbits)),
                 vec, BitRange::baseSize(range.least()+offset, nbits));
            offset += bitsPerDigit;
        }
    }

    // Zero fill high order stuff that we didn't already initialize
    if (offset < range.size())
        clear(vec, BitRange::hull(range.least()+offset, range.greatest()));
}

/** Obtain bits from a decimal representation.
 *
 *  The @p input string must contain only valid decimal digits '0' through '9' or the underscore character (to make long
 *  strings more readable), or else an <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger
 *  than what is required to initialize the specified sub-vector, then extra data is discarded.  On the other hand, if the
 *  length of the string is insufficient to initialize the entire sub-vector then the high order bits of the sub-vector are
 *  cleared.
 *
 *  @todo Conversion from a decimal string to a bit vector is not fully implemented. At this time, the decimal string must not
 *  parse to more than 64 bits. */
template<class Word>
void fromDecimal(Word *vec, const BitRange &range, const std::string &input) {
    boost::uint64_t v = 0;
    BOOST_FOREACH (char ch, input) {
        if (isdigit(ch)) {
            boost::uint64_t tmp = v * 10 + (ch - '0');
            if (tmp < v)
                throw std::runtime_error("overflow parsing decimal string");
            v = tmp;
        } else if (ch != '_') {
            throw std::runtime_error("invalid decimal digit \"" + std::string(1, ch) + "\"");
        }
    }
    fromInteger(vec, range, v);
}

/** Obtain bits from a hexadecimal representation.
 *
 *  The @p input string must contain only valid hexadecimal digits '0' through '9', 'a' through 'f', and 'A' through 'F', or
 *  the underscore character (to make long strings more readable), or else an <code>std::runtime_error</code> is thrown. If the
 *  number of supplied digits is larger than what is required to initialize the specified sub-vector, then extra data is
 *  discarded.  On the other hand, if the length of the string is insufficient to initialize the entire sub-vector then the
 *  high order bits of the sub-vector are cleared. */
template<class Word>
void fromHex(Word *vec, const BitRange &range, const std::string &input) {
    fromString<Word, 4>(vec, range, input);
}

/** Obtain bits from an octal representation.
 *
 *  The @p input string must contain only valid octal digits '0' through '7' or the underscore (to make long strings more
 *  readable), or else an <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger than what is
 *  required to initialize the specified sub-vector, then extra data is discarded.  On the other hand, if the length of the
 *  string is insufficient to initialize the entire sub-vector then the high order bits of the sub-vector are cleared. */
template<class Word>
void fromOctal(Word *vec, const BitRange &range, const std::string &input) {
    fromString<Word, 3>(vec, range, input);
}

/** Obtain bits from a binary representation.
 *
 *  The @p input string must contain only valid binary digits '0' and '1' or the underscore (to make long strings more
 *  readable), or else an <code>std::runtime_error</code> is thrown. If the number of supplied digits is larger than what is
 *  required to initialize the specified sub-vector, then extra data is discarded.  On the other hand, if the length of the
 *  string is insufficient to initialize the entire sub-vector then the high order bits of the sub-vector are cleared. */
template<class Word>
void fromBinary(Word *vec, const BitRange &range, const std::string &input) {
    fromString<Word, 1>(vec, range, input);
}


} // namespace
} // namespace
} // namespace

DYNINST_DIAGNOSTIC_END_SUPPRESS_VLA_GCC_PRAGMA_BUG

#endif
