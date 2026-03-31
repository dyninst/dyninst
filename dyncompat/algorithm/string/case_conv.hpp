#ifndef DYNINST_DYNCOMPAT_ALGORITHM_STRING_CASE_CONV_HPP
#define DYNINST_DYNCOMPAT_ALGORITHM_STRING_CASE_CONV_HPP

#include <algorithm>
#include <cctype>
#include <string>

namespace dyncompat {
namespace algorithm {

inline void trim_right(std::string& value) {
  while(!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
    value.pop_back();
  }
}

} // namespace algorithm
} // namespace dyncompat

#endif
