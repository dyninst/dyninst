#ifndef DYNINST_DYNCOMPAT_REGEX_H
#define DYNINST_DYNCOMPAT_REGEX_H

#include <regex>

namespace dyncompat {

using regex = std::regex;
using smatch = std::smatch;
using cmatch = std::cmatch;
using std::regex_match;
using std::regex_search;

namespace regex_constants = std::regex_constants;

} // namespace dyncompat

#endif
