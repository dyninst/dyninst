#include "dyntypes.h"
#include "find_main.h"

namespace Dyninst { namespace DyninstAPI { namespace aarch64 {

  main_addr_t find_main() {
    return main_addr_t{}.unknown();
  }

}}}
