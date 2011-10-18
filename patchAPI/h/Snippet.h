/* Public Interface */

#ifndef PATCHAPI_H_SNIPPET_H_
#define PATCHAPI_H_SNIPPET_H_

#include "common.h"

namespace Dyninst {
namespace PatchAPI {

/* Interface for snippet representation. */
/*
template <class T>
class Snippet {
  public:
    typedef dyn_detail::boost::shared_ptr<Snippet<T> > Ptr;
    static Ptr create(T rep) { return Ptr(new Snippet<T>(rep)); }
    static Ptr get(SnippetPtr s) {
      return dyn_detail::boost::static_pointer_cast<Snippet<T> >(s);
    }
    explicit Snippet(T rep) : rep_(rep) {}
    ~Snippet() {}

    T rep() { return rep_; }

  protected:
    T rep_;
};
*/

/*
  To extend Snippet:

    - Prepare a constructor and pass in whatever structures that are needed to
      generate code, e.g., codeGen in dyninst.
    - Implement generateCode().
 */
class Snippet {
  public:
    typedef dyn_detail::boost::shared_ptr<Snippet> Ptr;

    virtual bool generateCode();
};


}
}
#endif  // PATCHAPI_H_SNIPPET_H_
