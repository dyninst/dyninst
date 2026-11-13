#ifndef DYNINST_TOOLS_NORETURN_NAME_FILE_H
#define DYNINST_TOOLS_NORETURN_NAME_FILE_H

#include <cstdio>
#include <fstream>
#include <string>

namespace noreturn_tools {

// Calls add(name) for every name in the file at path: one name per line, with
// surrounding white space removed; blank lines and lines starting with '#' are
// skipped. Stops at the first name add() refuses.
// Returns the number of names added, or -1 on an unreadable file or a refusal.
template <typename AddFn>
int readNames(char const *path, AddFn add) {
  std::ifstream in{path};
  if(!in) {
    std::fprintf(stderr, "cannot read %s\n", path);
    return -1;
  }
  int count{0};
  std::string line;
  while(std::getline(in, line)) {
    auto const first = line.find_first_not_of(" \t\r");
    if(first == std::string::npos || line[first] == '#') {
      continue;
    }
    auto const last = line.find_last_not_of(" \t\r");
    std::string const name = line.substr(first, last - first + 1);
    if(!add(name)) {
      std::fprintf(stderr, "refused: %s\n", name.c_str());
      return -1;
    }
    ++count;
  }
  return count;
}

}

#endif
