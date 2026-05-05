#ifndef DYNINST_DYNCOMPAT_LEXICAL_CAST_HPP
#define DYNINST_DYNCOMPAT_LEXICAL_CAST_HPP

#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace dyncompat {

class bad_lexical_cast : public std::bad_cast {
public:
  const char* what() const noexcept override { return "bad lexical cast"; }
};

template <typename Target, typename Source>
Target lexical_cast(const Source& source) {
  if constexpr(std::is_same_v<Target, std::string>) {
    std::ostringstream os;
    os << source;
    if(!os) {
      throw bad_lexical_cast();
    }
    return os.str();
  } else {
    std::istringstream is(lexical_cast<std::string>(source));
    Target target{};
    is >> target;
    if(!is || !is.eof()) {
      throw bad_lexical_cast();
    }
    return target;
  }
}

} // namespace dyncompat

#endif
