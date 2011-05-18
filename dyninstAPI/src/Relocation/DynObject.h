/* Plugin Implementation */

#ifndef PATCHAPI_H_DYNINST_DYNMODULE_H_
#define PATCHAPI_H_DYNINST_DYNMODULE_H_

#include "DynCommon.h"
#include "CodeMover.h"
#include "PatchObject.h"

namespace Dyninst {
namespace PatchAPI {

class DynObject : public PatchObject {

  public:
    static DynObjectPtr create(ParseAPI::CodeObject* co, Address base);
    DynObject(ParseAPI::CodeObject* co, Address base);
    ~DynObject();

    virtual bool process(InstanceSet* insertion_set,
                         InstanceSet* deletion_set,
                         FuncRepMap*  func_rep,
                         CallRepMap*  call_rep,
                         CallRemoval* call_remove);

    // Getters and Setters
    AddressSpace* as() const { return as_; }
    void setAs(AddressSpace* as) { as_ = as; }

  private:
    AddressSpace* as_;
};

}
}
#endif  // PATCHAPI_H_DYNINST_DYNMODULE_H_
