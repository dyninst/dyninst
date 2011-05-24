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
    static DynObject* create(ParseAPI::CodeObject* co,
                             AddressSpace* as,
                             Address base) {
      return (new DynObject(co, as, base));
    }
    DynObject(ParseAPI::CodeObject* co, AddressSpace* as, Address base);
    DynObject(const DynObject *par_obj, process *child, Address base);
    ~DynObject();

    virtual bool instrument(InstanceSet* insertion_set,
                         InstanceSet* deletion_set,
                         FuncRepMap*  func_rep,
                         CallRepMap*  call_rep,
                         CallRemoval* call_remove);

    // Getters
    AddressSpace* as() const { return as_; }

  private:
    AddressSpace* as_;
};

class DynCFGMaker : public Dyninst::PatchAPI::CFGMaker {
  public:
    DynCFGMaker() {}
    virtual ~DynCFGMaker() {}

    virtual PatchFunction* makeFunction(ParseAPI::Function*, PatchObject*);
    virtual PatchFunction* copyFunction(PatchFunction*, PatchObject*);

    virtual PatchBlock* makeBlock(ParseAPI::Block*, PatchObject*);
    virtual PatchBlock* copyBlock(PatchBlock*, PatchObject*);

    virtual void initCopiedObject(const PatchObject* parObj, PatchObject* obj);
};
typedef dyn_detail::boost::shared_ptr<DynCFGMaker> DynCFGMakerPtr;

}
}
#endif  // PATCHAPI_H_DYNINST_DYNMODULE_H_
