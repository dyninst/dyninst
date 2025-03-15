#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "Symtab.h"
#include "SymEval.h"
#include "CFG.h"
#include "dyntypes.h"
#include "Symbol.h"

#include <vector>

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;

  Dyninst::Address find_main(st::Symtab*);

  std::vector<st::Symbol*> get_missing_symbols(st::Symtab*, Dyninst::Address);

  namespace ppc {
    Dyninst::Address find_main_by_toc(st::Symtab*, pa::Function*, pa::Block*);
  }

  namespace x86 {
    Dyninst::Address find_main(pa::Function*, pa::Block*);
  }

}}

#endif
