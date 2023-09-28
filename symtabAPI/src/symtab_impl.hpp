#ifndef SYMTAB_IMPL_HPP
#define SYMTAB_IMPL_HPP

#include "IBSTree.h"
#include "Module.h"
#include "Variable.h"
#include "concurrent.h"
#include "indexed_symbols.hpp"
#include "indexed_modules.h"

#include <mutex>
#include <string>

namespace Dyninst { namespace SymtabAPI {

  struct symtab_impl final {
    indexed_symbols everyDefinedSymbol{};
    indexed_symbols undefDynSyms{};

    indexed_modules modules{};

    std::once_flag funcRangesAreParsed{};
    std::once_flag types_parsed{};

    // Since Functions are unique by address, we require this structure to
    // efficiently track them.
    dyn_c_hash_map<Offset, Function *> funcsByOffset{};

    using VarsByOffsetMap = dyn_c_hash_map<Offset, std::vector<Variable *>>;
    VarsByOffsetMap varsByOffset{};

    using ModRangeLookup = IBSTree<ModRange>;
    ModRangeLookup mod_lookup_{};

    using FuncRangeLookup = IBSTree<FuncRange>;
    FuncRangeLookup func_lookup{};

    Module* default_module{};
  };

}}

#endif
