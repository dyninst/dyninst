/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#ifndef DYNINST_COMMON_BITMATH_H
#define DYNINST_COMMON_BITMATH_H

#include <boost/optional.hpp>
#include <cstdint>
#include <type_traits>

namespace Dyninst {

  namespace detail {

    inline bool isPowerOf2(uint64_t x) noexcept {
      return x != 0UL && ((x & (x - 1ULL)) == 0ULL);
    }

    inline int popcnt(uint64_t x) noexcept {
    #ifdef __GNUC__
      return __builtin_popcountll(x);
    #else
      int n = 0;
      for (; x; ++n) {
        x &= x - 1U;
      }
      return n;
    #endif
    }

    // count leading zeros
    inline int clz(uint64_t x) noexcept {
    #ifdef __GNUC__
      return __builtin_clzll(x);
    #else
      x |= (x >> 1);
      x |= (x >> 2);
      x |= (x >> 4);
      x |= (x >> 8);
      x |= (x >> 16);
      return popcnt(~x);
    #endif
    }

    inline boost::optional<uint8_t> ilog2(uint64_t x) noexcept {
      if (!isPowerOf2(x)) {
        return boost::none;
      }
      return 63UL - clz(x);
    }
  }

  template<typename T, typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  bool isPowerOf2(T x) noexcept {
    if (x < 0LL) {
      return false;
    }
    return detail::isPowerOf2(static_cast<uint64_t>(x));
  }

  template<typename T, typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  bool isPowerOf2(T x) noexcept {
    return detail::isPowerOf2(static_cast<uint64_t>(x));
  }

  // Returns the log base2 of `x` or `boost::none` on failure
  template<typename T, typename std::enable_if<std::is_unsigned<T>::value, bool>::type = true>
  boost::optional<uint8_t> ilog2(T x) noexcept {
    return detail::ilog2(static_cast<uint64_t>(x));
  }

  template<typename T, typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
  inline boost::optional<uint8_t> ilog2(T x) noexcept {
    if(x <= 0LL) {
      return boost::none;
    }
    return detail::ilog2(static_cast<uint64_t>(x));
  }

}

#endif
