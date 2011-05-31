#ifndef _DYNINSTRUMENTER_H_
#define _DYNINSTRUMENTER_H_


#include "Instrumenter.h"
#include "DynAddrSpace.h"

using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynAddrSpacePtr;

namespace Dyninst {
namespace PatchAPI {

class DynInstrumenter : public Dyninst::PatchAPI::Instrumenter {
  public:
    DynInstrumenter() {}
    virtual ~DynInstrumenter() {}
    virtual bool process();
  };
typedef dyn_detail::boost::shared_ptr<DynInstrumenter> DynInstrumenterPtr;

}
}

#endif /* _DYNINSTRUMENTER_H_ */
