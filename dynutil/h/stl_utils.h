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
