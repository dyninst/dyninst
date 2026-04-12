#include "addressSpace.h"
#include "arch-regs-x86.h"
#include "arch-x86.h"
#include "Architecture.h"
#include "binaryEdit.h"
#include "codegen/emitters/x86/Emitterx86.h"
#include "codegen/RegControl.h"
#include "debug.h"
#include "function.h"
#include "image.h"
#include "inst-x86.h"
#include "parse_func.h"
#include "registerSpace/registerSpace.h"
#include "Symbol.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace Dyninst { namespace DyninstAPI {

  Address Emitterx86::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    const unsigned int jump_slot_size = getArchAddressWidth(gen.getArch());

    if(!binEdit || !func) {
      assert(!"Invalid function call (function info is missing)");
    }

    SymtabAPI::Symbol *referring = func->getRelocSymbol();

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if(!relocation_address) {
      // inferiorMalloc addr location and initialize to zero
      relocation_address = binEdit->inferiorMalloc(jump_slot_size);
      unsigned char *dat = (unsigned char *)malloc(jump_slot_size);
      memset(dat, 0, jump_slot_size);
      binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);
      free(dat);

      // add write new relocation symbol/entry
      binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
  }

  Address Emitterx86::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    const unsigned int jump_slot_size = getArchAddressWidth(gen.getArch());

    if(!binEdit || !var) {
      assert(!"Invalid variable load (variable info is missing)");
    }

    // find the Symbol corresponding to the int_variable
    std::vector<SymtabAPI::Symbol *> syms;
    var->svar()->getSymbols(syms);

    if(syms.size() == 0) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s", __FILE__, __LINE__,
              var->symTabName().c_str());
      showErrorCallback(80, msg);
      assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for(unsigned k = 0; k < syms.size(); k++) {
      if(syms[k]->isInDynSymtab()) {
        referring = syms[k];
        break;
      }
    }

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if(!relocation_address) {
      // inferiorMalloc addr location and initialize to zero
      relocation_address = binEdit->inferiorMalloc(jump_slot_size);
      unsigned int dat = 0;
      binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, &dat);

      // add write new relocation symbol/entry
      binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
  }

}}
