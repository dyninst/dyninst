/* Plugin Interface */

#ifndef PATCHAPI_H_LINKER_H_
#define PATCHAPI_H_LINKER_H_

#include "common.h"
#include "AddrSpace.h"
#include "Point.h"
#include "Object.h"

namespace Dyninst {
namespace PatchAPI {

/* Install instrumentation to the original code: link the patch area to the
   original code */
class Linker {
  public:
    static LinkerPtr create(AddrSpacePtr as);
    Linker(AddrSpacePtr as) : as_(as){}
    virtual ~Linker() {}

    // Iterate all Objects and call their link_preprocess to initialize
    virtual bool preprocess();

    // Iterate all Objects and call their link_process to install
    // instrumentation
    virtual bool process();

    // Getters
    AddrSpacePtr  as() const { return as_; }

  protected:
    AddrSpacePtr as_;
};

}
}

#endif /* end of include guard: _LINKER_H_ */
