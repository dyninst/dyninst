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
#include <set>

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

    Module* getContainingModule(Offset offset) const {
      std::set<ModRange*> mods;
      mod_lookup_.find(offset, mods);

      if(mods.size() == 1) {
	auto const& m = *(mods.begin());
        return m->id();
      }

      /* Because the default module covers the entire PC range of the
         file, it will always be found so exclude it.

         It is assumed that the PC ranges of modules are disjoint and this
         function can only return a single Module.
      */
      for(auto *mr : mods) {
        if(mr->id() != default_module) {
          return mr->id();
	}
      }

      return nullptr;
    }
  };

}}

#endif
