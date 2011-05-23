#ifndef PATCHAPI_H_DYNINST_DYNCOMMON_H_
#define PATCHAPI_H_DYNINST_DYNCOMMON_H_

// PatchAPI public interface
#include "common.h"
#include "Instrumenter.h"

// Dyninst Internal
#include "addressSpace.h"

#define DYN_CAST(type, obj)  dyn_detail::boost::dynamic_pointer_cast<type>(obj)

namespace Dyninst {
namespace PatchAPI {
  class DynAddrSpace;
  typedef dyn_detail::boost::shared_ptr<DynAddrSpace> DynAddrSpacePtr;

  class DynObject;
  typedef dyn_detail::boost::shared_ptr<DynObject> DynObjectPtr;

}
}


#endif  // PATCHAPI_H_DYNINST_DYNCOMMON_H_
