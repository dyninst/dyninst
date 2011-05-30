#ifndef _DYNINSTRUMENTER_H_
#define _DYNINSTRUMENTER_H_


#include "Instrumenter.h"
#include "DynAddrSpace.h"

using Dyninst::PatchAPI::DynAddrSpace;
using Dyninst::PatchAPI::DynAddrSpacePtr;
using Dyninst::PatchAPI::InstanceSet;
using Dyninst::PatchAPI::FuncRepMap;
using Dyninst::PatchAPI::CallRepMap;
using Dyninst::PatchAPI::CallRemoval;

namespace Dyninst {
namespace PatchAPI {

class DynInstrumenter : public Dyninst::PatchAPI::Instrumenter {
  public:
    DynInstrumenter() {}
    virtual ~DynInstrumenter() {}
    virtual bool process(InstanceSet* /*insertion_set*/,
                         InstanceSet* /*deletion_set*/,
                         FuncRepMap*  /*func_rep*/,
                         CallRepMap*  /*call_rep*/,
                         CallRemoval* /*call_removal*/);
  };
typedef dyn_detail::boost::shared_ptr<DynInstrumenter> DynInstrumenterPtr;

}
}

#endif /* _DYNINSTRUMENTER_H_ */
