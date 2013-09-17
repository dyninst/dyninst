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
#ifndef STL_UTILS
#define STL_UTILS
#include <utility>

///
/// This file contains some utility functions for dealing with the STL.
/// In particular, this mostly contains functors and adaptors for dealing
/// with std::pair inside STL algorithms.
///

namespace Dyninst {

/// Functor to get the first element of a pair.  Use with STL functions like transform().
struct get_first {
  template <typename P>
  typename P::first_type operator()(const P& pair) {
    return pair.first;
  }
};

/// Functor to get the second element of a pair.  Use with STL functions like transform().
struct get_second {
  template <typename P>
  typename P::second_type operator()(const P& pair) {
    return pair.second;
  }
};

/// Applies a ftor to the first element of a pair
template <typename Functor>
struct do_to_first_ftor {
  Functor ftor;
  do_to_first_ftor(const Functor& f) : ftor(f) { }
  template <typename P>
  void operator()(const P& pair) {
    ftor(pair.first);
  }
};

/// Type-inferring adapter function for do_to_first_ftor
template <typename Functor>
inline do_to_first_ftor<Functor> do_to_first(const Functor& f) {
  return do_to_first_ftor<Functor>(f);
}

/// Applies a ftor to the second element of a pair
template <typename Functor>
struct do_to_second_ftor {
  Functor ftor;
  do_to_second_ftor(const Functor& f) : ftor(f) { }
  template <typename P>
  void operator()(const P& pair) {
    ftor(pair.second);
  }
};

/// Type-inferring adapter function for do_to_second_ftor
template <typename Functor>
inline do_to_second_ftor<Functor> do_to_second(const Functor& f) {
  return do_to_second_ftor<Functor>(f);
}

} // namespace Dyninst

#endif // STL_UTILS
