#ifndef DYNINST_DYNCOMPAT_ALGORITHM_STRING_FIND_HPP
#define DYNINST_DYNCOMPAT_ALGORITHM_STRING_FIND_HPP

#include <algorithm>

namespace dyncompat {

template <typename Range, typename Predicate>
auto find_token(Range&& range, Predicate predicate) {
  using std::begin;
  using std::end;
  return std::find_if(begin(range), end(range), predicate);
}

} // namespace dyncompat

#endif
