#ifndef PATCHAPI_H_DYNINST_DYNCOMMON_H_
#define PATCHAPI_H_DYNINST_DYNCOMMON_H_

// PatchAPI public interface
#include "common.h"
#include "Instrumenter.h"

// Dyninst Internal
#include "addressSpace.h"

#define DYN_CAST(type, obj)  dyn_detail::boost::dynamic_pointer_cast<type>(obj)

// Shortcuts for type casting
#define SCAST_MO(o) static_cast<mapped_object*>(o)
#define SCAST_EI(e) static_cast<edge_instance*>(e)
#define SCAST_BI(b) static_cast<block_instance*>(b)
#define SCAST_PB(b) static_cast<parse_block*>(b)
#define SCAST_PF(f) static_cast<parse_func*>(f)
#define SCAST_FI(f) static_cast<func_instance*>(f)


namespace Dyninst {
namespace PatchAPI {
  class DynAddrSpace;
  typedef dyn_detail::boost::shared_ptr<DynAddrSpace> DynAddrSpacePtr;

  class DynObject;
  typedef dyn_detail::boost::shared_ptr<DynObject> DynObjectPtr;

}
}


#endif  // PATCHAPI_H_DYNINST_DYNCOMMON_H_
