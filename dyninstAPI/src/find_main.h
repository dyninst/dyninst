#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "Symtab.h"
#include "SymEval.h"
#include "Symbol.h"

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;

  Dyninst::Address find_main(st::Symtab*);

  std::vector<st::Symbol*> get_missing_symbols(st::Symtab*, Dyninst::Address);

  namespace ppc {
    Dyninst::Address find_main(st::Symtab*, pa::Function*);
  }

  namespace x86_64 {
    Dyninst::Address find_main(pa::Function*);
  }

  namespace x86 {
    Dyninst::Address find_main(pa::Function*);
  }

}}

#endif
