#include "Symtab.h"
#include "Type.h"
#include "Variable.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// See test_binaries/symtabAPI/DWARF/type_units/README.md
//
// Usage: type_units <binary> <variable>=<size>[/<fields>]...
//
// Parses the types of <binary> and checks that each global <variable> has a
// type of <size> bytes and, if given, with <fields> components.
//
// With -fdebug-types-section a type is defined once, in a type unit, and the
// units that use it refer to it by signature. Variables of such types get the
// wrong size, or no type at all, unless the signatures resolve to the type
// units that define them.

namespace st = Dyninst::SymtabAPI;

namespace {

struct expectation {
  std::string text, variable;
  unsigned size{};
  long fields{-1};
};

bool parse_expectation(std::string const &arg, expectation &e) {
  auto const equals = arg.find('=');
  if (equals == std::string::npos || equals == 0) return false;
  e.text = arg;
  e.variable = arg.substr(0, equals);
  char *end{};
  e.size = static_cast<unsigned>(std::strtoul(arg.c_str() + equals + 1, &end, 0));
  if (end == arg.c_str() + equals + 1) return false;
  if (*end == '/') {
    char const *start = end + 1;
    e.fields = std::strtol(start, &end, 0);
    if (end == start) return false;
  }
  return *end == '\0';
}

bool check(st::Symtab *symtab, expectation const &e) {
  std::vector<st::Variable *> vars;
  if (!symtab->findVariablesByName(vars, e.variable) || vars.empty()) {
    std::cerr << "FAIL: " << e.text << ": no variable '" << e.variable << "'\n";
    return false;
  }
  auto type = vars.front()->getType(st::Type::share);
  if (!type) {
    std::cerr << "FAIL: " << e.text << ": '" << e.variable << "' has no type\n";
    return false;
  }

  unsigned const size = type->getSize();
  long fields = -1;
  if (type->isFieldListType()) fields = static_cast<long>(type->asFieldListType().getComponents()->size());

  std::cout << e.variable << ": " << type->getName() << ", " << size << " bytes";
  if (fields >= 0) std::cout << ", " << fields << " fields";
  std::cout << "\n";

  if (size != e.size || (e.fields >= 0 && fields != e.fields)) {
    std::cerr << "FAIL: " << e.text << ": found " << size << " bytes";
    if (e.fields >= 0) std::cerr << ", " << fields << " fields";
    std::cerr << "\n";
    return false;
  }
  return true;
}

}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <binary> <variable>=<size>[/<fields>]...\n";
    return EXIT_FAILURE;
  }

  std::vector<expectation> expected;
  for (int i = 2; i < argc; i++) {
    expectation e;
    if (!parse_expectation(argv[i], e)) {
      std::cerr << "FAIL: bad expectation '" << argv[i] << "'\n";
      return EXIT_FAILURE;
    }
    expected.push_back(e);
  }

  st::Symtab *symtab{};
  if (!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "FAIL: unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  symtab->parseTypesNow();

  bool failed = false;
  for (auto const &e : expected)
    if (!check(symtab, e)) failed = true;
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
