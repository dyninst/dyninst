#ifndef DYNINST_DYNCOMPAT_ALGORITHM_STRING_PREDICATE_HPP
#define DYNINST_DYNCOMPAT_ALGORITHM_STRING_PREDICATE_HPP

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace dyncompat {

inline bool starts_with(std::string_view value, std::string_view prefix) {
  return value.substr(0, prefix.size()) == prefix;
}

inline bool ends_with(std::string_view value, std::string_view suffix) {
  return value.size() >= suffix.size() &&
         value.substr(value.size() - suffix.size()) == suffix;
}

inline bool iequals(std::string_view lhs, std::string_view rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char a, char b) {
           return std::tolower(static_cast<unsigned char>(a)) ==
                  std::tolower(static_cast<unsigned char>(b));
         });
}

} // namespace dyncompat

#endif
