#ifndef DYNINST_DYNCOMPAT_ALGORITHM_STRING_CLASSIFICATION_HPP
#define DYNINST_DYNCOMPAT_ALGORITHM_STRING_CLASSIFICATION_HPP

#include <cctype>

namespace dyncompat {

struct is_graph_predicate {
  bool operator()(char ch) const {
    return std::isgraph(static_cast<unsigned char>(ch)) != 0;
  }
};

inline is_graph_predicate is_graph() {
  return {};
}

} // namespace dyncompat

#endif
