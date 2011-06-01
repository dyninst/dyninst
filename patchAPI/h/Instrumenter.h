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
    PATCHAPI_EXPORT static InstrumenterPtr create(AddrSpacePtr as);
    PATCHAPI_EXPORT virtual ~Instrumenter() {}

    // Iterate all Objects and call their process method to do instrumentation
    PATCHAPI_EXPORT virtual bool process();

    // Code Modification interfaces

    // Function Replacement
    PATCHAPI_EXPORT virtual bool replaceFunction(PatchFunction* oldfunc,
                                                 PatchFunction* newfunc);
    PATCHAPI_EXPORT virtual bool revertReplacedFunction(PatchFunction* oldfunc);

    // Getters and setters
    PATCHAPI_EXPORT AddrSpacePtr as() const { return as_; }
    PATCHAPI_EXPORT void setAs(AddrSpacePtr as) { as_ = as; }
  protected:
    AddrSpacePtr as_;
    FuncModMap functionReplacements_;

    explicit Instrumenter(AddrSpacePtr as) : as_(as) {}
    Instrumenter() {}
};
}
}
#endif  // PATCHAPI_H_INSTRUMENTOR_H_
