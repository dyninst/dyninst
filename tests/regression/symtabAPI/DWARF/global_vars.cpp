#include "Symtab.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// See test_binaries/symtabAPI/DWARF/global_vars/README.md
//
// Usage: global_vars <binary> [<function>:]<variable>=<size>[/<fields>]...
//
// Parses the types of <binary> and checks that each <variable> has a type of
// <size> bytes and, if given, <fields> components. A <variable> is a global, or
// with <function>: a parameter or local variable of that function.
//
// A global gets its type only when the address in its DW_AT_location matches
// its symbol. DWARF 5 producers can give that address as DW_OP_addrx, an index
// into .debug_addr; if it isn't resolved, the variable keeps no type. In DWARF 5
// a DW_AT_decl_file of 0 names the unit's primary source file; if it is
// rejected, the variable is dropped.

namespace st = Dyninst::SymtabAPI;

namespace {

struct expectation {
  std::string text, function, variable;
  unsigned size{};
  long fields{-1};
};

bool parse_expectation(std::string const &arg, expectation &e) {
  auto const equals = arg.find('=');
  if (equals == std::string::npos || equals == 0) return false;
  e.text = arg;
  e.variable = arg.substr(0, equals);
  auto const colon = e.variable.find(':');
  if (colon != std::string::npos) {
    e.function = e.variable.substr(0, colon);
    e.variable = e.variable.substr(colon + 1);
    if (e.function.empty() || e.variable.empty()) return false;
  }
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

st::localVar *find_local(st::Symtab *symtab, expectation const &e) {
  std::vector<st::Function *> funcs;
  if (!symtab->findFunctionsByName(funcs, e.function) || funcs.empty()) return nullptr;
  std::vector<st::localVar *> params, locals;
  funcs.front()->getParams(params);
  funcs.front()->getLocalVariables(locals);
  for (auto const *vars : {&params, &locals})
    for (auto *v : *vars)
      if (v->getName() == e.variable) return v;
  return nullptr;
}

bool check(st::Symtab *symtab, expectation const &e) {
  boost::shared_ptr<st::Type> type;
  if (!e.function.empty()) {
    auto *v = find_local(symtab, e);
    if (!v) {
      std::cerr << "FAIL: " << e.text << ": no parameter or local '" << e.variable << "' in '" << e.function << "'\n";
      return false;
    }
    type = v->getType(st::Type::share);
  } else {
    std::vector<st::Variable *> vars;
    if (!symtab->findVariablesByName(vars, e.variable) || vars.empty()) {
      std::cerr << "FAIL: " << e.text << ": no variable '" << e.variable << "'\n";
      return false;
    }
    type = vars.front()->getType(st::Type::share);
  }
  if (!type) {
    std::cerr << "FAIL: " << e.text << ": '" << e.variable << "' has no type\n";
    return false;
  }

  unsigned const size = type->getSize();
  long fields = -1;
  if (type->isFieldListType()) fields = static_cast<long>(type->asFieldListType().getComponents()->size());

  std::cout << e.text.substr(0, e.text.find('=')) << ": " << type->getName() << ", " << size << " bytes";
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
    std::cerr << "Usage: " << argv[0] << " <binary> [<function>:]<variable>=<size>[/<fields>]...\n";
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
