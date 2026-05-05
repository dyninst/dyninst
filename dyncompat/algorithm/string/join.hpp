#ifndef DYNINST_DYNCOMPAT_ALGORITHM_STRING_JOIN_HPP
#define DYNINST_DYNCOMPAT_ALGORITHM_STRING_JOIN_HPP

#include <sstream>
#include <string>

namespace dyncompat {
namespace algorithm {

template <typename Range>
std::string join(const Range& parts, const std::string& separator) {
  std::ostringstream os;
  auto it = parts.begin();
  if(it == parts.end()) {
    return os.str();
  }
  os << *it;
  ++it;
  for(; it != parts.end(); ++it) {
    os << separator << *it;
  }
  return os.str();
}

} // namespace algorithm
} // namespace dyncompat

#endif
