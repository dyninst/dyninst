/* Plugin Interface */

#ifndef PATCHAPI_H_MODIFICATOR_H_
#define PATCHAPI_H_MODIFICATOR_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Modify binary code: delete function call, replace function call, replace
   function */
class Modificator {
  public:
    Modificator(ObjectPtr obj): obj_(obj) {}
    virtual ~Modificator() {}

    // Put the main logic inside
    virtual bool process() { return false; }

  protected:
    ObjectPtr obj_;
};

}
}
#endif  // PATCHAPI_H_MODIFICATOR_H_
