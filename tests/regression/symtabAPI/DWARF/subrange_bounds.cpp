#include "Symtab.h"
#include "Function.h"
#include "Type.h"
#include "Variable.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// See test_binaries/symtabAPI/DWARF/subrange_bounds/README.md
//
// Usage: subrange_bounds <binary> <function>:<variable>=<dims>...
//   <dims> is a comma-separated list of <low>:<high>, one per array
//   dimension, outermost first. A bound of '?' means the bound is only
//   known at run time and must be reported as unknown (LONG_MIN for a
//   lower bound, LONG_MAX for an upper bound). A bound of '*' is not
//   checked.

namespace st = Dyninst::SymtabAPI;

namespace {

struct bound {
  bool checked;
  long value;
};

struct dimension {
  bound low, high;
};

struct expectation {
  std::string function, variable, text;
  std::vector<dimension> dims;
};

bool parse_bound(std::string const &s, long unknown, bound &out) {
  out.checked = s != "*";
  if (!out.checked) return true;
  if (s == "?") {
    out.value = unknown;
    return true;
  }
  char *end{};
  out.value = std::strtol(s.c_str(), &end, 0);
  return !s.empty() && *end == '\0';
}

bool parse_expectation(std::string const &arg, expectation &e) {
  auto const colon = arg.find(':');
  auto const equals = arg.find('=');
  if (colon == std::string::npos || equals == std::string::npos || equals < colon)
    return false;
  e.text = arg;
  e.function = arg.substr(0, colon);
  e.variable = arg.substr(colon + 1, equals - colon - 1);

  std::stringstream dims{arg.substr(equals + 1)};
  std::string dim;
  while (std::getline(dims, dim, ',')) {
    auto const sep = dim.find(':');
    if (sep == std::string::npos)
      return false;
    dimension d{};
    if (!parse_bound(dim.substr(0, sep), LONG_MIN, d.low) ||
        !parse_bound(dim.substr(sep + 1), LONG_MAX, d.high))
      return false;
    e.dims.push_back(d);
  }
  return !e.dims.empty();
}

bool has_name(st::Function *f, std::string const &name) {
  for (auto n = f->mangled_names_begin(); n != f->mangled_names_end(); ++n)
    if (*n == name) return true;
  for (auto n = f->pretty_names_begin(); n != f->pretty_names_end(); ++n)
    if (*n == name) return true;
  return false;
}

st::localVar *find_variable(st::Symtab *symtab, expectation const &e) {
  std::vector<st::Function *> funcs;
  symtab->getAllFunctions(funcs);
  for (auto *f : funcs) {
    if (!has_name(f, e.function)) continue;
    std::vector<st::localVar *> vars;
    f->getParams(vars);
    f->getLocalVariables(vars);
    for (auto *v : vars)
      if (v->getName() == e.variable) return v;
  }
  return nullptr;
}

std::string bound_str(long b) {
  if (b == LONG_MIN || b == LONG_MAX) return "?";
  return std::to_string(b);
}

std::string bound_str(bound const &b) {
  return b.checked ? bound_str(b.value) : "*";
}

bool matches(bound const &b, long actual) {
  return !b.checked || b.value == actual;
}

bool check(st::Symtab *symtab, expectation const &e) {
  auto *var = find_variable(symtab, e);
  if (!var) {
    std::cerr << e.text << ": variable not found\n";
    return false;
  }

  // Each dimension of a multidimensional array is a nested typeArray whose
  // base type is the next dimension.
  st::Type *type = var->getType();
  for (std::size_t i = 0; i < e.dims.size(); ++i) {
    if (!type || !type->isArrayType()) {
      std::cerr << e.text << ": dimension " << i << " is "
                << (type ? type->getName() : "a null type")
                << ", not an array type\n";
      return false;
    }
    auto &arr = type->asArrayType();
    auto const low = static_cast<long>(arr.getLow());
    auto const high = static_cast<long>(arr.getHigh());
    if (!matches(e.dims[i].low, low) || !matches(e.dims[i].high, high)) {
      std::cerr << e.text << ": dimension " << i << " is [" << bound_str(low)
                << ":" << bound_str(high) << "], expected ["
                << bound_str(e.dims[i].low) << ":" << bound_str(e.dims[i].high)
                << "]\n";
      return false;
    }
    type = arr.getBaseType();
  }
  if (type && type->isArrayType()) {
    std::cerr << e.text << ": has more than " << e.dims.size()
              << " dimension(s)\n";
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <binary> <function>:<variable>=<low>:<high>[,...]...\n";
    return EXIT_FAILURE;
  }

  std::vector<expectation> expected;
  for (int i = 2; i < argc; ++i) {
    expectation e;
    if (!parse_expectation(argv[i], e)) {
      std::cerr << "Malformed expectation '" << argv[i] << "'\n";
      return EXIT_FAILURE;
    }
    expected.push_back(e);
  }

  st::Symtab *symtab{};
  if (!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  int failures = 0;
  for (auto const &e : expected)
    if (!check(symtab, e)) ++failures;

  if (failures) {
    std::cerr << failures << " of " << expected.size()
              << " expectation(s) failed for " << argv[1] << "\n";
    return EXIT_FAILURE;
  }
}
