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

    virtual Point* createPoint(Address     addr, Point::Type type,
                               PatchMgrPtr mgr,  Address*         scope);
    virtual Point* createPoint(Address     addr, Point::Type type,
                               PatchMgrPtr mgr,  PatchBlock*      scope);
    virtual Point* createPoint(Address     addr, Point::Type type,
                               PatchMgrPtr mgr,  PatchEdge*       scope);
    virtual Point* createPoint(Address     addr, Point::Type type,
                               PatchMgrPtr mgr,  PatchFunction*   scope);
};
typedef dyn_detail::boost::shared_ptr<DynPointMaker> DynPointMakerPtr;

#endif /* PATCHAPI_DYNPOINTMAKER_H_ */
