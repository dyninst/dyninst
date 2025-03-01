#ifndef DYNINST_DYNINSTAPI_FINDMAIN_H
#define DYNINST_DYNINSTAPI_FINDMAIN_H

#include "Function.h"
#include "SymEval.h"
#include "Symtab.h"

namespace Dyninst { namespace DyninstAPI {

  namespace st = Dyninst::SymtabAPI;
  namespace pa = Dyninst::ParseAPI;

  namespace ppc {
    Dyninst::Address find_main(st::Symtab*, pa::Function*);
  }

  namespace x86 {
    Dyninst::Address find_main(pa::Function*);
  }

}}

#endif
