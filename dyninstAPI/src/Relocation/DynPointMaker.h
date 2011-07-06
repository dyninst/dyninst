#ifndef PATCHAPI_DYNPOINTMAKER_H_
#define PATCHAPI_DYNPOINTMAKER_H_

#include "Point.h"

class instPoint;

using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchEdge;
using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::PointMaker;

class DynPointMaker : public Dyninst::PatchAPI::PointMaker {
  public:
    DynPointMaker() {}
    virtual ~DynPointMaker() {}
  protected:
    virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *);
    virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *, PatchBlock *);
    virtual Point *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, PatchFunction *context);
    virtual Point *mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, Address, 
                               InstructionAPI::Instruction::Ptr, PatchFunction *context);
    virtual Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *, PatchFunction *f);

};
typedef dyn_detail::boost::shared_ptr<DynPointMaker> DynPointMakerPtr;

#endif /* PATCHAPI_DYNPOINTMAKER_H_ */
