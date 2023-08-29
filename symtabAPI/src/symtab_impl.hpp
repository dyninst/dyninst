#ifndef SYMTAB_IMPL_HPP
#define SYMTAB_IMPL_HPP

#include "indexed_symbols.hpp"

namespace Dyninst { namespace SymtabAPI {

  struct symtab_impl final {
    indexed_symbols everyDefinedSymbol{};
    indexed_symbols undefDynSyms{};
  };

}}

#endif
