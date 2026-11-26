#include "Symtab.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// See test_binaries/symtabAPI/DWARF/global_vars/README.md
//
// Usage: global_vars <binary>
//
// Parses the types of <binary>, one of the builds of global_vars.c, and checks
// the type of each variable below: its size and, for a struct, its number of
// fields. A variable is a global, or with a function: a parameter or local
// variable of that function. Every build gives the same answers.
//
// A global gets its type only when the address in its DW_AT_location matches
// its symbol. DWARF 5 producers can give that address as DW_OP_addrx, an index
// into .debug_addr; if it isn't resolved, the variable keeps no type. In DWARF 5
// a DW_AT_decl_file of 0 names the unit's primary source file; if it is
// rejected, the variable is dropped.

namespace st = Dyninst::SymtabAPI;

namespace {

struct expectation {
  char const *function, *variable;
  unsigned size;
  long fields;
};

constexpr long no_fields = -1;

constexpr std::array<expectation, 12> expected = {{
  {"", "g_i8", 1, no_fields},
  {"", "g_i16", 2, no_fields},
  {"", "g_i32", 4, no_fields},
  {"", "g_i64", 8, no_fields},
  {"", "g_vlong", 8, no_fields},
  {"", "g_pair", 16, 2},
  {"", "g_array", 40, no_fields},
  {"", "g_bss", 8, no_fields},
  {"", "g_static", 4, no_fields},
  {"counter", "calls", 4, no_fields},
  {"twice", "value", 4, no_fields},
  {"twice", "doubled", 4, no_fields},
}};

std::string name(expectation const &e) {
  return *e.function ? std::string(e.function) + ":" + e.variable : e.variable;
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
  if (*e.function) {
    auto *v = find_local(symtab, e);
    if (!v) {
      std::cerr << "FAIL: " << name(e) << ": no parameter or local '" << e.variable << "' in '" << e.function << "'\n";
      return false;
    }
    type = v->getType(st::Type::share);
  } else {
    std::vector<st::Variable *> vars;
    if (!symtab->findVariablesByName(vars, e.variable) || vars.empty()) {
      std::cerr << "FAIL: " << name(e) << ": no variable '" << e.variable << "'\n";
      return false;
    }
    type = vars.front()->getType(st::Type::share);
  }
  if (!type) {
    std::cerr << "FAIL: " << name(e) << ": no type\n";
    return false;
  }

  unsigned const size = type->getSize();
  long fields = no_fields;
  if (type->isFieldListType()) fields = static_cast<long>(type->asFieldListType().getComponents()->size());

  std::cout << name(e) << ": " << type->getName() << ", " << size << " bytes";
  if (fields != no_fields) std::cout << ", " << fields << " fields";
  std::cout << "\n";

  if (size != e.size || (e.fields != no_fields && fields != e.fields)) {
    std::cerr << "FAIL: " << name(e) << ": expected " << e.size << " bytes";
    if (e.fields != no_fields) std::cerr << ", " << e.fields << " fields";
    std::cerr << ", found " << size << " bytes";
    if (e.fields != no_fields) std::cerr << ", " << fields << " fields";
    std::cerr << "\n";
    return false;
  }
  return true;
}

}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <binary>\n";
    return EXIT_FAILURE;
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
