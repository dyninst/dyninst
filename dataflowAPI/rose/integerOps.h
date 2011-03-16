/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef ROSE_INTEGEROPS_H
#define ROSE_INTEGEROPS_H

#include <limits>
#include <boost/static_assert.hpp>

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

namespace IntegerOps {

// 1 << x, handling the case where x is >= the number of bits in T
template <typename T, size_t Count>
struct SHL1: public IntegerOpsPrivate::SHL1Helper<T, Count, (Count >= IntegerOpsPrivate::NumBits<T>::value)> {};

// 1 << x, handling the case where x is >= the number of bits in T
template <typename T>
inline T shl1(size_t count) {
  return (count >= IntegerOpsPrivate::NumBits<T>::value) ? T(0) : (T(1) << count);
}

// Set rightmost (from LSB) count bits of result, clear others
template <typename T, size_t Count>
struct GenMask {
  static const T value = SHL1<T, Count>::value - T(1);
};

template <typename T>
inline T genMask(size_t count) {
  return shl1<T>(count) - 1;
}

template <size_t NBits, typename T>
inline bool signBit(T value) {
  return (value & SHL1<T, NBits - 1>::value) != T(0);
}

template <size_t FromBits, size_t ToBits, typename T>
inline T signExtend(T value) {
  return value | (signBit<FromBits>(value) ? (GenMask<T, ToBits>::value ^ GenMask<T, FromBits>::value) : T(0));
}

template <size_t NBits, typename T>
inline T shiftLeft(T value, size_t count) {
  return (value * shl1<T>(count)) & GenMask<T, NBits>::value;
};

template <size_t NBits, typename T>
inline T shiftRightLogical(T value, size_t count) {
  return (count >= NBits) ? T(0) : (value >> count);
}

template <size_t NBits, typename T>
inline T shiftRightArithmetic(T value, size_t count) {
  if (count >= NBits) {
    return signBit<NBits>(value) ? GenMask<T, NBits>::value : T(0);
  } else {
    return shiftRightLogical<NBits>(value, count) |
           (signBit<NBits>(value) ? (GenMask<T, NBits>::value ^ genMask<T>(NBits - count)) : T(0));
  }
}

template <size_t NBits, typename T>
inline T rotateLeft(T value, size_t count) {
  count %= NBits;
  return ((value << count) | (value >> (NBits - count))) & GenMask<T, NBits>::value;
}

template <size_t NBits, typename T>
inline T rotateRight(T value, size_t count) {
  count %= NBits;
  return ((value >> count) | (value << (NBits - count))) & GenMask<T, NBits>::value;
}

template <typename T>
inline T log2(T a) {
  T n = T(1);
  T i = 0;
  while (n != 0 && n < a) {n <<= 1; ++i;}
  return i;
}

} // namespace IntegerOps

#endif // ROSE_INTEGEROPS_H
