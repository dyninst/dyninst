/* Plugin Interface */

#ifndef PATCHAPI_H_INSTRUMENTOR_H_
#define PATCHAPI_H_INSTRUMENTOR_H_

#include "common.h"
#include "Point.h"
#include "AddrSpace.h"
#include "Command.h"

namespace Dyninst {
namespace PatchAPI {

/* Relocate the original code and generate snippet binary code in mutatee's
   address space. */

class Instrumenter : public BatchCommand {
  public:
    friend class Patcher;
    PATCHAPI_EXPORT static InstrumenterPtr create(AddrSpacePtr as);
    virtual ~Instrumenter() {}
    /*
    // Iterate all Objects and call their process method to do instrumentation
    PATCHAPI_EXPORT virtual bool process();
    */
    // Code Modification interfaces

    // Function Replacement
    PATCHAPI_EXPORT virtual bool replaceFunction(PatchFunction* oldfunc,
                                                 PatchFunction* newfunc);
    PATCHAPI_EXPORT virtual bool revertReplacedFunction(PatchFunction* oldfunc);
    virtual FuncModMap& funcRepMap() { return functionReplacements_; }

    // Function Wrapping
    PATCHAPI_EXPORT virtual bool wrapFunction(PatchFunction* oldfunc,
                                              PatchFunction* newfunc,
                                              std::string name);
    PATCHAPI_EXPORT virtual bool revertWrappedFunction(PatchFunction* oldfunc);
    virtual FuncWrapMap& funcWrapMap() { return functionWraps_; }

    // Call Modification
    PATCHAPI_EXPORT virtual bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee,
                                    PatchFunction *context = NULL);
    PATCHAPI_EXPORT virtual bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    PATCHAPI_EXPORT virtual bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL);
    virtual CallModMap& callModMap() { return callModifications_; }

    // Getters and setters
    AddrSpacePtr as() const { return as_; }
    void setAs(AddrSpacePtr as) { as_ = as; }

  protected:
    AddrSpacePtr as_;
    CommandList user_commands_;
    FuncModMap functionReplacements_;
    FuncWrapMap functionWraps_;
    CallModMap callModifications_;

    explicit Instrumenter(AddrSpacePtr as) : as_(as) {}
    Instrumenter() {}
};
}
}
#endif  // PATCHAPI_H_INSTRUMENTOR_H_
