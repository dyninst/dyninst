/* Plugin Interface */

#ifndef PATCHAPI_H_INSTRUMENTOR_H_
#define PATCHAPI_H_INSTRUMENTOR_H_

#include "common.h"
#include "Point.h"
#include "AddrSpace.h"

namespace Dyninst {
namespace PatchAPI {

/* Relocate the original code and generate snippet binary code in mutatee's
   address space. */
class Instrumenter {
  public:
    static InstrumenterPtr create(AddrSpacePtr as);
    explicit Instrumenter(AddrSpacePtr as) : as_(as) {}
    virtual ~Instrumenter() {}

    // Iterate all Objects and call their process method to do instrumentation
    virtual bool process(InstanceSet* /*insertion_set*/,
                         InstanceSet* /*deletion_set*/,
                         FuncRepMap*  /*func_rep*/,
                         CallRepMap*  /*call_rep*/,
                         CallRemoval* /*call_remove*/);

    // Getters
    AddrSpacePtr as() const { return as_; }

  protected:
    AddrSpacePtr as_;
};
}
}
#endif  // PATCHAPI_H_INSTRUMENTOR_H_
