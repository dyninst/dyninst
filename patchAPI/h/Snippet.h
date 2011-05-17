/* Public Interface */

#ifndef PATCHAPI_H_SNIPPET_H_
#define PATCHAPI_H_SNIPPET_H_

#include "common.h"
#include "SnippetRep.h"

namespace Dyninst {
namespace PatchAPI {

// A sequence of code. See also: SnippetRep.h
class Snippet {
 public:
    explicit Snippet(void* snippet_rep) : snippet_rep_(snippet_rep) {}
    virtual ~Snippet() {}
    static SnippetPtr create(void* snippet_rep);

    void* rep() const { return snippet_rep_; }

  private:
    void* snippet_rep_;
};

}
}
#endif  // PATCHAPI_H_SNIPPET_H_
