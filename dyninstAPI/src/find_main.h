#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "Function.h"
#include "SymEval.h"
#include "Symtab.h"

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;

  namespace ppc {
    Dyninst::Address find_main_by_toc(st::Symtab*, pa::Function*, pa::Block*);
  }

  namespace x86 {
    Dyninst::Address find_main(pa::Function*, pa::Block*);
  }

}}

#endif
