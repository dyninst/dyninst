/* Public Interface */

#ifndef PATCHAPI_H_SNIPPET_H_
#define PATCHAPI_H_SNIPPET_H_

#include "PatchCommon.h"
#include "Point.h"
#include "Buffer.h"


namespace Dyninst {
namespace PatchAPI {

/* Interface for snippet representation. */

/*
  To extend Snippet:

    - Prepare a constructor and pass in whatever structures that are needed to
      generate code, e.g., codeGen in dyninst.
    - Implement generateCode().
 */
class Snippet {
  public:
    typedef dyn_detail::boost::shared_ptr<Snippet> Ptr;
    Snippet();
    virtual ~Snippet();
    static Snippet::Ptr create(Snippet *a) { return Ptr(a); }

    // Returns false if code generation failed catastrophically
    // Point is an in-param that identifies where the snippet is
    // being generated.
    // Buffer is an out-param that holds the generated code. 
    virtual bool generate(Point *, Buffer &) = 0;
};


}
}
#endif  // PATCHAPI_H_SNIPPET_H_
