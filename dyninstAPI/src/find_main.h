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

  namespace ppc {
    Dyninst::Address find_main(st::Symtab*, pa::Function*);
  }

  namespace x86 {
    Dyninst::Address find_main(pa::Function*);
  }

}}

#endif
