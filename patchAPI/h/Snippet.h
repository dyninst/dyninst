/* Public Interface */

#ifndef PATCHAPI_H_SNIPPET_H_
#define PATCHAPI_H_SNIPPET_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface for snippet representation. */
template <class T>
class Snippet {
  public:
    typedef boost::shared_ptr<Snippet<T> > Ptr;
    static Ptr create(T rep) { return Ptr(new Snippet<T>(rep)); }
    static Ptr get(SnippetPtr s) {
      return Ptr(s);
    }
    explicit Snippet(T rep) : rep_(rep) {}
    ~Snippet() {}

    T rep() { return rep_; }

  protected:
    T rep_;
};

}
}
#endif  // PATCHAPI_H_SNIPPET_H_
