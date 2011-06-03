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
    virtual ~Instrumenter() {}

    // Iterate all Objects and call their process method to do instrumentation
    PATCHAPI_EXPORT virtual bool process();

    // Code Modification interfaces

    // Function Replacement
    PATCHAPI_EXPORT virtual bool replaceFunction(PatchFunction* oldfunc,
                                                 PatchFunction* newfunc);
    PATCHAPI_EXPORT virtual bool revertReplacedFunction(PatchFunction* oldfunc);
    virtual FuncModMap& funcRepMap() { return functionReplacements_; }

    // Function Wrapping
    PATCHAPI_EXPORT virtual bool wrapFunction(PatchFunction* oldfunc,
                                              PatchFunction* newfunc);
    PATCHAPI_EXPORT virtual bool revertWrappedFunction(PatchFunction* oldfunc);
    virtual FuncModMap& funcWrapMap() { return functionWraps_; }

    // Call Modification
    PATCHAPI_EXPORT bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee,
                                    PatchFunction *context = NULL);
    PATCHAPI_EXPORT bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    PATCHAPI_EXPORT bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    CallModMap& callModMap() { return callModifications_; }

    // Getters and setters
    AddrSpacePtr as() const { return as_; }
    void setAs(AddrSpacePtr as) { as_ = as; }
  protected:
    AddrSpacePtr as_;
    FuncModMap functionReplacements_;
    FuncModMap functionWraps_;
    CallModMap callModifications_;

    explicit Instrumenter(AddrSpacePtr as) : as_(as) {}
    Instrumenter() {}
};
}
}
#endif  // PATCHAPI_H_INSTRUMENTOR_H_
