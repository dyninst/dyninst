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
    DynObject(const DynObject *par_obj, AddressSpace* child, Address base);
    ~DynObject();

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

    virtual PatchEdge* makeEdge(ParseAPI::Edge*, PatchBlock*, PatchBlock*, PatchObject*);
    virtual PatchEdge* copyEdge(PatchEdge*, PatchObject*);
};
typedef boost::shared_ptr<DynCFGMaker> DynCFGMakerPtr;

}
}
#endif  // PATCHAPI_H_DYNINST_DYNMODULE_H_
