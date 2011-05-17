/* Plugin Interface */

#ifndef PATCHAPI_H_RELOCATOR_H_
#define PATCHAPI_H_RELOCATOR_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Relocate original binary code. */
class Relocator {
  public:
    Relocator(ObjectPtr obj) : obj_(obj) {}
    virtual ~Relocator() {}
    virtual bool preprocess() { return false; }
    virtual bool postprocess() { return false; }

  protected:
    ObjectPtr obj_;
};

}
}
#endif  // PATCHAPI_H_RELOCATOR_H_
