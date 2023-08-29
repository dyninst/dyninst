#ifndef SYMTAB_IMPL_HPP
#define SYMTAB_IMPL_HPP

#include "Module.h"
#include "concurrent.h"
#include "indexed_symbols.hpp"

#include <string>
#include <mutex>

#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

namespace Dyninst { namespace SymtabAPI {

  struct symtab_impl final {
    indexed_symbols everyDefinedSymbol{};
    indexed_symbols undefDynSyms{};

    Dyninst::dyn_mutex im_lock{};
    boost::multi_index_container<
        Module *, boost::multi_index::indexed_by<
                      boost::multi_index::random_access<>,
                      boost::multi_index::ordered_unique<boost::multi_index::identity<Module *>>,
                      boost::multi_index::ordered_non_unique<boost::multi_index::const_mem_fun<
                          Module, const std::string &, &Module::fileName>>>>
        indexed_modules{};

    std::once_flag funcRangesAreParsed;
    std::once_flag types_parsed;
  };

}}

#endif
