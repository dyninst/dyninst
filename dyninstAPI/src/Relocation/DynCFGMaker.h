#ifndef PATCHAPI_CFGMAKER_H_
#define PATCHAPI_CFGMAKER_H_

#include "PatchCFG.h"

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchObject;

class DynCFGMaker : public Dyninst::PatchAPI::CFGMaker {
  public:
    DynCFGMaker() {}
    virtual ~DynCFGMaker() {}

    virtual PatchFunction* makeFunction(ParseAPI::Function*, PatchObject*);
    virtual PatchFunction* copyFunction(PatchFunction*, PatchObject*);
};
typedef dyn_detail::boost::shared_ptr<DynCFGMaker> DynCFGMakerPtr;

#endif /* PATCHAPI_CFGMAKER_H_ */
